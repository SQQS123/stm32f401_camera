#include "key.h"
#include "delay.h"

// 用于学习
void key_init(void){
	GPIO_InitTypeDef gpio_init_struct = {0};
	
	__HAL_RCC_GPIOE_CLK_ENABLE();
	
	gpio_init_struct.Pin = GPIO_PIN_2;
	gpio_init_struct.Mode =GPIO_MODE_INPUT;
	gpio_init_struct.Pull = GPIO_PULLUP;
	gpio_init_struct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOE,&gpio_init_struct);
	
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_SET);
}


// 按键扫描，消抖
uint8_t key_scan(void){
	
	// 如果读取到的状态为0，则按键按下
	if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_2) == 0){
		delay_ms(10);  //10ms消抖,就是等待足够长的时间判断这个信号持续了至少10ms以上
		if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_2) == 0){
			while(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_2) == 0)
			return 1;		//按键按下
		}
	}
	return 0;				//按键没有按下
}