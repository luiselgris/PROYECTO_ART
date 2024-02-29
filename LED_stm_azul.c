#include "LED_stm_azul.h"
#include "stm32f4xx_hal.h"


/*----- Threads -----*/
void ThLEDstm_azul(void *argument);
osThreadId_t id_ThLED_stm_azul;

/*---- FLAGS ----*/
#define LED_AZUL_ON 0x10

/*----- Variables de control -----*/
uint32_t flag_led_2;

/*----- Init Thread -----*/
int Init_ThLEDstm_azul(void){
  id_ThLED_stm_azul = osThreadNew(ThLEDstm_azul, NULL, NULL);
  if(id_ThLED_stm_azul == NULL)
    return(-1);
  return 0;
}

void ThLEDstm_azul(void *argument) {
	
	while(1) {
		flag_led_2 = osThreadFlagsWait(LED_AZUL_ON, osFlagsWaitAny, osWaitForever);
		 if(flag_led_2 == LED_AZUL_ON){
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
			osDelay(350);
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
		}
		
	}
	
}
