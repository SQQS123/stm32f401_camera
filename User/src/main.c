/**
  ******************************************************************************
  * @file    GPIO/GPIO_EXTI/Src/main.c 
  * @author  MCD Application Team
  * @brief   This example describes how to configure and use GPIOs through 
  *          the STM32F4xx HAL API.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "emount_protocol.h"
#include "gpio_config.h"
#include "delay.h"
#include "key.h"


UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;

uint8_t rx_buffer[64];
uint8_t rx_complete = 0;

static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_DMA_Init(void);
static void SystemClock_Config(void);
static void EXTILine0_Config(void);

void Main_Error_Handler(void);
void MX_DMA_Init(void);


/**
  * @brief  Main program
  * @param  None
  * @retval None
	*/
/*
int main(void)
{

  HAL_Init();
  SystemClock_Config();
	EMount_RegisterErrorCallback(Main_Error_Handler);
	
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_USART1_UART_Init();
	
	// 初始化E卡口协议
	EMount_Init(&huart1);
	
	// 启动UART接收
	HAL_UART_Receive_DMA(&huart1, rx_buffer, 64);
	
 Infinite loop */
/*
  while (1)
  {
		// 检查是否接收到完整消息
		if (rx_complete){
			// 处理接收到的消息
			EMount_ProcessMessage(rx_buffer, 64);
			rx_complete = 0;
			
			// 重新启动接收
			HAL_UART_Receive_DMA(&huart1, rx_buffer, 64);
		}
  }
}

*/

//测试消抖的主函数
int main(void){
	HAL_Init();
	SystemClock_Config();
	delay_init(84);
	key_init();
	
	if(key_scan()){
		HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_5);
	}
	else{
		delay_ms(10);
	}
}


/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 84000000
  *            HCLK(Hz)                       = 84000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 2
  *            APB2 Prescaler                 = 1
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 336
  *            PLL_P                          = 4
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale2 mode
  *            Flash Latency(WS)              = 2
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;   //要填写时钟树的结构体
  RCC_OscInitTypeDef RCC_OscInitStruct;   //要填写振荡器/PLL的结构体

  /* Enable Power Control clock */
  // 打开PWR外设时钟,“STM32 的电源控制器（PWR）默认没时钟，先给它时钟，否则后面调压寄存器写不进去。”
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  // 选择电压等级2，
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;      //告诉 HAL：我要配置的是高速外部晶振（HSE），CubeMX里相当于最左面选择HSE
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;                        //把 HSE 打开（如果是 Proteus 仿真，这里相当于让软件模型知道‘外部 8 MHz 源已使能’）
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;                    //打开 PLL
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;            //PLL 的时钟源选择 HSE

  RCC_OscInitStruct.PLL.PLLM = 8;                                 //把 8 MHz 先除以8分频 ，得到 1 MHz 的 VCO 输入频率
  RCC_OscInitStruct.PLL.PLLN = 336;                               //再乘以 336 倍，得到 336 MHz 的 VCO 输出频率 
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;                     //最后除以4分频，得到 84 MHz 的系统时钟 SYSCLK  
  RCC_OscInitStruct.PLL.PLLQ = 7;                                 //USB OTG FS, SDIO 和 RNG 的时钟 = 336 MHz / 7 = 48 MHz
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  } 

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  //下面填写总线分频
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);  // 高速HAL，我要配置哪些，这些或一下
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;               // 系统时钟选择 PLL 输出的 84 MHz
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;                      // AHB 总线不分频，还是 84 MHz
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;                       // APB1 总线2分频，得到 42 MHz
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;                       // APB2 总线不分频，还是 84 MHz  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)  
  {
    Error_Handler();
  }
}

/**
  * @brief  Configures EXTI Line0 (connected to PA0 pin) in interrupt mode
  * @param  None
  * @retval None
  */
static void EXTILine0_Config(void)
{
  GPIO_InitTypeDef   GPIO_InitStructure;

  /* Enable GPIOA clock */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  
  /* Configure PA0 pin as input floating */
  GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Pin = GPIO_PIN_0;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Enable and set EXTI Line0 Interrupt to the lowest priority */
  HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

/**
  * @brief EXTI line detection callbacks
  * @param GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Main_Error_Handler(void)
{
  /* Turn LED5 on */
  while(1)
  {
  }
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        rx_complete = 1;
    }
}

static void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 750000;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
}

static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // 配置握手信号引脚
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;  // LENS_CS_BODY, BODY_CS_LENS
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // 配置检测引脚
    GPIO_InitStruct.Pin = GPIO_PIN_2;  // LENS_XDETECT
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // 设置检测引脚为低电平（镜头存在）
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
}

void MX_DMA_Init(void)
{
    /* DMA 控制器时钟使能 */
    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();

    /* 如果后面还要用 HAL_UART_Receive_DMA / HAL_UART_Transmit_DMA
       就在这里加 HAL_NVIC_SetPriority / HAL_NVIC_EnableIRQ 也行 */
}

// 电源控制函数
void EMount_PowerControl(void)
{
    // 检测镜头是否被识别
    if (EMount_GetSyncSignal() == 0) {
        // 同步信号为低，相机正在通信
        // 可以启用UART通信
        HAL_UART_Receive_IT(&huart1, rx_buffer, 1);
    }
}

// 在主循环中处理握手信号
void EMount_HandleHandshaking(void)
{
    // 当机身片选为高时，准备发送数据
    if (EMount_GetBodyCS()) {
        // 机身准备接收数据
        EMount_SetLensCS(1);  // 拉高镜头片选
        // 发送数据...
        EMount_SetLensCS(0);  // 发送完成后拉低
    }
}

// 波特率切换函数
void EMount_SwitchBaudRate(uint32_t baudrate)
{
    huart1.Init.BaudRate = baudrate;
    if (HAL_UART_Init(&huart1) != HAL_OK) {
        Error_Handler();
    }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */

/**
  * @}
  */
