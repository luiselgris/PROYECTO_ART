#ifndef __LCD_H
#define __LCD_H

	#include "cmsis_os2.h"
	#include "stm32f4xx_hal.h"
	#include "string.h"
	#include "stdio.h"

	int Init_ThLCD(void);
	int Init_Timer (void);
	
	typedef struct{
		char data_1[20];
		char data_2[20];
		uint8_t clean;
	}MSGQUEUE_LCD;

	osMessageQueueId_t get_id_MsgQueue_LCD(void);

#endif
