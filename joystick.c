#include "joystick.h"
#include "stm32f4xx_hal.h"

#define REBOTE_FLAG 0X01
#define CICLOS_FLAG 0X02
#define IRQ_FLAG    0x04

/*----- Threads -----*/
osThreadId_t id_ThJoystick;
/*----- Queue -----*/
osMessageQueueId_t id_MsgQueue_joystick;
MSGQUEUE_JOY_TRANSFER msg;
/*----- Timers -----*/
osTimerId_t Tmr_rebote;
osTimerId_t Tmr_ciclos;
/*----- Variables de control -----*/
uint32_t flags;
uint8_t control = 0;
uint8_t cnt = 0;
uint8_t max_cnt = ((PULSACION_TIEMPO / 50) - 1);


void ThJoystick(void *argument);
void conf_pins(void);
void led_init(void);
uint8_t check_pins(void);

/*---------------------------------------------------
 *    				Inicializacion de las Queue
 *---------------------------------------------------*/
osMessageQueueId_t get_id_MsgQueue_joystick(void){
	return id_MsgQueue_joystick;
}

static int Init_MsgQueue_joy(void){
  id_MsgQueue_joystick = osMessageQueueNew(MSGQUEUE_OBJECTS_JOY, sizeof(MSGQUEUE_JOY_TRANSFER), NULL);
  if(id_MsgQueue_joystick == NULL)
    return (-1); 
  return(0);
}
/*----- Init Thread -----*/
int Init_ThJoystick(void){
  id_ThJoystick = osThreadNew(ThJoystick, NULL, NULL);
  if(id_ThJoystick == NULL)
    return(-1);
  return(Init_MsgQueue_joy());
}

/*----- Callbacks Timers -----*/
static void tmr_rebote_Callback (void* argument) {
	osThreadFlagsSet(id_ThJoystick, REBOTE_FLAG);
}

static void tmr_ciclos_Callback (void* argument) {
  osThreadFlagsSet(id_ThJoystick, CICLOS_FLAG);
}

/*---------------------------------------------------
 *    									Thread
 *---------------------------------------------------*/
void ThJoystick(void *argument) {
	conf_pins();
	Tmr_rebote = osTimerNew(tmr_rebote_Callback, osTimerOnce, (void *)0, NULL);
	Tmr_ciclos = osTimerNew(tmr_ciclos_Callback, osTimerPeriodic, (void *)0, NULL);
	while(1) {
		flags = osThreadFlagsWait((CICLOS_FLAG | REBOTE_FLAG | IRQ_FLAG), osFlagsWaitAny, osWaitForever);
		if(flags & IRQ_FLAG){
			osTimerStart(Tmr_rebote, 50U);
		}
		if(flags & REBOTE_FLAG){
			if(check_pins()){
				control = 1;
				cnt = 0;
				osTimerStart(Tmr_ciclos, 50U);
			} 
		}
		if(flags & CICLOS_FLAG){
			if(cnt == max_cnt){
				cnt = 0;
				msg.pul = Largo;
				osTimerStop(Tmr_ciclos);
				osMessageQueuePut(id_MsgQueue_joystick, &msg, 0U, 0U);
			}else if(check_pins()){
				cnt ++;
			} 
			else if(control == 1){
				msg.pul = Corto;
				osTimerStop(Tmr_ciclos);
				osMessageQueuePut(id_MsgQueue_joystick, &msg, 0U, 0U);
			}
		}
  }
}


/*---------------------------------------------------
 *    							Interruptions
 *---------------------------------------------------*/
void EXTI15_10_IRQHandler(void){
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	control = 0;
	switch(GPIO_Pin){
		case GPIO_PIN_10:
			msg.dir = UP;
		break;
		
		case GPIO_PIN_12:
			msg.dir = DOWN;
		break;
	}
	osThreadFlagsSet(id_ThJoystick, IRQ_FLAG);
}

static uint8_t check_pins(void){
	if((HAL_GPIO_ReadPin (GPIOB, GPIO_PIN_10) == GPIO_PIN_SET)|
		 (HAL_GPIO_ReadPin (GPIOE, GPIO_PIN_12) == GPIO_PIN_SET)){
		return(1);
	}
	return(0);
}

static void conf_pins(void) {
	GPIO_InitTypeDef GPIO_InitStruct;
  __HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	
 	GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct); 
	
	GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	

	
	
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}


