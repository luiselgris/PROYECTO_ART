#include "rgb.h"

TIM_HandleTypeDef htim4;

osThreadId_t tid_ThRGB;
osMessageQueueId_t mid_MsgQueueRGB;

osMessageQueueId_t getModLedQueueID(void);
int Init_ThRGB (void);
int Init_MsgQueue_RGB (void);
void Th_RGB(void *argument);

/*----- QUEUE RGB -----*/
typedef struct{
	uint16_t r;
	uint16_t g;
	uint16_t b;
}MSGQUEUE_OBJ_RGB;
MSGQUEUE_OBJ_RGB msg_RGB_rx;

MSGQUEUE_OBJ_RGB local_rgb;


/*---------------------------------------------------
 *    				Inicializacion de las Queue
 *---------------------------------------------------*/
int Init_MsgQueue_RGB (void) {
  mid_MsgQueueRGB = osMessageQueueNew(MSGQUEUE_OBJECTS_RGB , sizeof(MSGQUEUE_OBJ_RGB), NULL);
  if (mid_MsgQueueRGB == NULL) {
    return (-1);
  }
  return(0);
}

osMessageQueueId_t get_mid_MsgQueueRGB(void){
	return mid_MsgQueueRGB;
}



int Init_ThRGB(void){
  tid_ThRGB = osThreadNew(Th_RGB, NULL, NULL);
  if(tid_ThRGB == NULL)
    return(-1);
  return(Init_MsgQueue_RGB());

}
void Th_RGB(void *argument){
	LED_RGB_Init();
	
	
	while(1){
		if(osOK == osMessageQueueGet(mid_MsgQueueRGB, &local_rgb, NULL, 0U)){
			
			LED_RGB_SetColor(local_rgb.r, local_rgb.g, local_rgb.b);
			
		}
		osThreadYield();
	}
}




void LED_RGB_Init(void)
{
  /* Inicializar los puertos GPIO para los LEDs R, G y B */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  
  GPIO_InitTypeDef GPIO_InitStruct;
  
  /* Configurar el pin del LED verde (PD12) como salida */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
  
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  
  /* Configurar el pin del LED rojo (PD13) como salida */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  
  /* Configurar el pin del LED azul (PD14) como salida */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  
  /* Inicializar el Timer 4 para el control PWM */
  __HAL_RCC_TIM4_CLK_ENABLE();
  
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 255;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  
  HAL_TIM_PWM_Init(&htim4);
  
  /* Configurar el canal 1 del Timer 4 para el LED verde */
  TIM_OC_InitTypeDef sConfigOC;
  
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  
  HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1);
  
  /* Configurar el canal 2 del Timer 4 para el LED rojo */
  HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2);
  
  /* Configurar el canal 3 del Timer 4 para el LED azul */
  HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4);
  
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_15, GPIO_PIN_SET);
}


void LED_RGB_SetColor(uint8_t red, uint8_t green, uint8_t blue){

	red = 255 - red;
	green = 255 - green;
	blue = 255 - blue;
	

	
	if (red>0){																			//ROJO
		HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
		/* Establecer la intensidad del LED rojo mediante PWM */
		htim4.Instance->CCR2 = red;
	}
	if (green>0){																		//VERDE
		HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
		/* Establecer la intensidad del LED verde mediante PWM */
		htim4.Instance->CCR1 = green;
	}
	if (blue>0){																			//AZUL
		HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);
		/* Establecer la intensidad del LED rojo mediante PWM */
		htim4.Instance->CCR4 = blue;
	}
 
}





void Th_RGB_Test(void *argument); 
 
osThreadId_t tid_Th_RGB_Test;

int Init_Th_RGB_Test(void) {
 
  tid_Th_RGB_Test = osThreadNew(Th_RGB_Test, NULL, NULL);
  if (tid_Th_RGB_Test == NULL) {
    return(-1);
  }	
  return(0);
}
 
void Th_RGB_Test (void *argument) {
	MSGQUEUE_OBJ_RGB extern_rgb;
	
  while (1) {
		
//		//Color sable de luke antiguo
//		extern_rgb.r = 254;
//		extern_rgb.g = 0;
//		extern_rgb.b = 0;
//		osMessageQueuePut(mid_MsgQueueRGB, &extern_rgb, 0U, 0U);
//		osDelay(1000);

		
  }
}
