#ifndef EMOUNT_PROTOCOL_H
#define EMOUNT_PROTOCOL_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

typedef void (*EMount_ErrorCallback_t)(void);			// 错误回调原型

void EMount_RegisterErrorCallback(EMount_ErrorCallback_t cb);		//注册函数

// E卡口消息结构
typedef struct {
    uint8_t sync;      // 0xF0
    uint8_t msg_id;    // 消息ID
    uint8_t unknown;   // 通常为0
    uint8_t status;    // 01:正常, 02:初始化
    uint8_t seq_num;   // 序列号
    uint8_t data[16];  // 数据载荷
    uint16_t checksum; // 校验和
    uint8_t end;       // 0x55
} EMountMessage;

// 协议状态
typedef enum {
    EMOUNT_STATE_INIT,
    EMOUNT_STATE_NORMAL,
    EMOUNT_STATE_ERROR
} EMountState;

// 函数声明
void EMount_Init(UART_HandleTypeDef *huart);
void EMount_ProcessMessage(uint8_t *rx_data, uint16_t len);
void EMount_SendResponse(uint8_t msg_id, uint8_t *data, uint8_t len);
uint16_t EMount_CalculateChecksum(uint8_t *data, uint16_t len);

void EMount_HandleInit(void);
void EMount_HandleStatusQuery(void);
void EMount_HandleParamQuery(void);
void EMount_HandleLensInfo(void);

extern void Error_Handler(void);                       /* 系统错误钩子  */
void EMount_SendDefaultResponse(uint8_t msg_id);/* 未知消息默认回复 */

#endif
