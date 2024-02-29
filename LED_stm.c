#include "LED_stm.h"
#include "stm32f4xx_hal.h"


/*----- Threads -----*/
void ThLEDstm(void *argument);
osThreadId_t id_ThLED_stm;

/*---- FLAGS ----*/
#define LED_VERDE 0x01

/*----- Variables de control -----*/
uint32_t flag_led;
uint8_t estado_led = 1; //acitivo = 1 inactivo = 0

/*----- Init Thread -----*/
int Init_ThLEDstm(void){
  id_ThLED_stm = osThreadNew(ThLEDstm, NULL, NULL);
  if(id_ThLED_stm == NULL)
    return(-1);
  return 0;
}

void ThLEDstm(void *argument) {
	flag_led = osThreadFlagsWait(LED_VERDE, osFlagsWaitAny, osWaitForever);
	while(1) {
		
		if(flag_led == osThreadFlagsWait(LED_VERDE, osFlagsWaitAny, 0)){
				estado_led ++;
		}
		if(estado_led == 2){
			estado_led = 0;
		}

		if(estado_led == 1){
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
			HAL_Delay(500);
		}else if(estado_led == 0){
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
		}
	}
	
}
