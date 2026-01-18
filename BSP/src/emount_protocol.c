#include "emount_protocol.h"
#include <string.h>

static UART_HandleTypeDef *emount_uart;
static EMountState current_state = EMOUNT_STATE_INIT;
static uint8_t current_seq = 0;
static uint8_t rx_buffer[64];
static uint8_t tx_buffer[64];

static EMount_ErrorCallback_t error_cb = NULL;		//

// 镜头参数结构
typedef struct {
    uint16_t focal_length;    // 焦距
    uint16_t aperture;        // 光圈
    uint8_t focus_mode;       // 对焦模式
    uint8_t aperture_mode;    // 光圈模式
    uint8_t lens_id;          // 镜头ID
} LensParams;

static LensParams lens_params = {
    .focal_length = 50,    // 50mm
    .aperture = 280,       // f/2.8
    .focus_mode = 0,       // 手动对焦
    .aperture_mode = 0,    // 手动光圈
    .lens_id = 0x1234
};

void EMount_Init(UART_HandleTypeDef *huart) {
    emount_uart = huart;
    
    // 配置UART为750k波特率
    huart->Init.BaudRate = 750000;
    huart->Init.WordLength = UART_WORDLENGTH_8B;
    huart->Init.StopBits = UART_STOPBITS_1;
    huart->Init.Parity = UART_PARITY_NONE;
    huart->Init.Mode = UART_MODE_TX_RX;
    huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    
    if (HAL_UART_Init(huart) != HAL_OK) {
        Error_Handler();
    }
    
    current_state = EMOUNT_STATE_INIT;
}

void EMount_ProcessMessage(uint8_t *rx_data, uint16_t len) {
    if (len < 8) return;  // 最小消息长度
    
    // 检查同步字节
    if (rx_data[0] != 0xF0) return;
    
    // 检查结束字节
    if (rx_data[len-1] != 0x55) return;
    
    uint8_t msg_id = rx_data[1];
    uint8_t seq_num = rx_data[4];
    
    // 更新序列号
    current_seq = seq_num;
    
    // 处理不同消息类型
    switch (msg_id) {
        case 0x01:  // 初始化请求
            EMount_HandleInit();
            break;
            
        case 0x0B:  // 状态查询
            EMount_HandleStatusQuery();
            break;
            
        case 0x09:  // 参数查询
            EMount_HandleParamQuery();
            break;
            
        case 0x29:  // 镜头信息
            EMount_HandleLensInfo();
            break;
            
        default:
            // 发送默认响应
            EMount_SendDefaultResponse(msg_id);
            break;
    }
}

void EMount_HandleInit(void) {
    uint8_t response_data[] = {
        0x01, 0x00, 0x02, 0x00,  // 基础响应
        0x00, 0x00, 0x00, 0x00   // 保留
    };
    
    EMount_SendResponse(0x01, response_data, 8);
    current_state = EMOUNT_STATE_NORMAL;
}

void EMount_HandleStatusQuery(void) {
    uint8_t response_data[] = {
        0x0B, 0x00, 0x01, 0x00,  // 状态
        0x00, 0x00, 0x00, 0x00   // 保留
    };
    
    EMount_SendResponse(0x0B, response_data, 8);
}

void EMount_HandleParamQuery(void) {
    uint8_t response_data[] = {
        0x09, 0x00, 0x01, 0x00,  // 参数响应
        0x00, 0x00, 0x00, 0x00   // 保留
    };
    
    EMount_SendResponse(0x09, response_data, 8);
}

void EMount_HandleLensInfo(void) {
    uint8_t response_data[16];
    
    // 镜头信息数据结构
    response_data[0] = lens_params.lens_id & 0xFF;
    response_data[1] = (lens_params.lens_id >> 8) & 0xFF;
    response_data[2] = lens_params.focal_length & 0xFF;
    response_data[3] = (lens_params.focal_length >> 8) & 0xFF;
    response_data[4] = lens_params.aperture & 0xFF;
    response_data[5] = (lens_params.aperture >> 8) & 0xFF;
    response_data[6] = lens_params.focus_mode;
    response_data[7] = lens_params.aperture_mode;
    
    // 填充剩余字节
    for (int i = 8; i < 16; i++) {
        response_data[i] = 0x00;
    }
    
    EMount_SendResponse(0x29, response_data, 16);
}

void EMount_SendResponse(uint8_t msg_id, uint8_t *data, uint8_t len) {
    uint8_t tx_len = 0;
    
    // 构建消息
    tx_buffer[tx_len++] = 0xF0;  // 同步
    tx_buffer[tx_len++] = msg_id; // 消息ID
    tx_buffer[tx_len++] = 0x00;   // 未知字节
    tx_buffer[tx_len++] = (current_state == EMOUNT_STATE_NORMAL) ? 0x01 : 0x02;
    tx_buffer[tx_len++] = current_seq; // 序列号
    
    // 复制数据
    memcpy(&tx_buffer[tx_len], data, len);
    tx_len += len;
    
    // 计算校验和
    uint16_t checksum = EMount_CalculateChecksum(tx_buffer + 1, tx_len - 1);
    tx_buffer[tx_len++] = checksum & 0xFF;
    tx_buffer[tx_len++] = (checksum >> 8) & 0xFF;
    
    // 结束字节
    tx_buffer[tx_len++] = 0x55;
    
    // 发送消息
    HAL_UART_Transmit(emount_uart, tx_buffer, tx_len, 100);
}

uint16_t EMount_CalculateChecksum(uint8_t *data, uint16_t len) {
    uint16_t checksum = 0;
    for (uint16_t i = 0; i < len; i++) {
        checksum += data[i];
    }
    return checksum;
}



/* ----------------   未知消息默认回复  ---------------- */
void EMount_SendDefaultResponse(uint8_t msg_id)
{
    /* 回送相同 msg_id，数据段全 0，告诉机身“我不认识这条命令” */
    uint8_t dummy[8] = {0};
    EMount_SendResponse(msg_id, dummy, 8);
}

/* 弱定义：如果没人注册，就进空循环 */
__weak void Error_Handler(void)
{
    if (error_cb) error_cb();
    else          while (1);
}

/* 供 main 调用 */
void EMount_RegisterErrorCallback(EMount_ErrorCallback_t cb)
{
    error_cb = cb;
}
