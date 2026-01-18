#include "gpio_config.h"

void EMount_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // 使能GPIOA时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    // 配置VD信号输入(PA0)
    GPIO_InitStruct.Pin = E_MOUNT_VD_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(E_MOUNT_VD_PORT, &GPIO_InitStruct);
    
    // 配置CS输出(PA1)
    GPIO_InitStruct.Pin = E_MOUNT_CS_OUT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(E_MOUNT_CS_OUT_PORT, &GPIO_InitStruct);
    
    // 配置CS输入(PA2)
    GPIO_InitStruct.Pin = E_MOUNT_CS_IN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(E_MOUNT_CS_IN_PORT, &GPIO_InitStruct);
    
    // 配置检测输出(PA3)
    GPIO_InitStruct.Pin = E_MOUNT_DETECT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(E_MOUNT_DETECT_PORT, &GPIO_InitStruct);
    
    // 初始化状态
    EMount_SetLensCS(0);    // CS输出低电平
    EMount_SetDetect(0);    // 检测引脚接地(镜头存在)
}

uint8_t EMount_GetSyncSignal(void)
{
    return HAL_GPIO_ReadPin(E_MOUNT_VD_PORT, E_MOUNT_VD_PIN);
}

uint8_t EMount_GetBodyCS(void)
{
    return HAL_GPIO_ReadPin(E_MOUNT_CS_IN_PORT, E_MOUNT_CS_IN_PIN);
}

void EMount_SetLensCS(uint8_t state)
{
    HAL_GPIO_WritePin(E_MOUNT_CS_OUT_PORT, E_MOUNT_CS_OUT_PIN, 
                      state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void EMount_SetDetect(uint8_t state)
{
    HAL_GPIO_WritePin(E_MOUNT_DETECT_PORT, E_MOUNT_DETECT_PIN, 
                      state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}