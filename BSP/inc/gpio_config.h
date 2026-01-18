#ifndef GPIO_CONFIG_H
#define GPIO_CONFIG_H

#include "stm32f4xx_hal.h"

// E卡口引脚定义
#define E_MOUNT_VD_PIN          GPIO_PIN_0    // PA0 - 同步信号输入
#define E_MOUNT_CS_OUT_PIN      GPIO_PIN_1    // PA1 - 片选输出
#define E_MOUNT_CS_IN_PIN       GPIO_PIN_2    // PA2 - 片选输入  
#define E_MOUNT_DETECT_PIN      GPIO_PIN_3    // PA3 - 检测输出

#define E_MOUNT_VD_PORT         GPIOA
#define E_MOUNT_CS_OUT_PORT     GPIOA
#define E_MOUNT_CS_IN_PORT      GPIOA
#define E_MOUNT_DETECT_PORT     GPIOA

// 函数声明
void EMount_GPIO_Init(void);
uint8_t EMount_GetSyncSignal(void);
uint8_t EMount_GetBodyCS(void);
void EMount_SetLensCS(uint8_t state);
void EMount_SetDetect(uint8_t state);

#endif