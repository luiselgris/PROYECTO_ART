#ifndef LED_RGB_H
#define LED_RGB_H

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"

void LED_RGB_Init(void);
void LED_RGB_SetColor(uint8_t red, uint8_t green, uint8_t blue);
osMessageQueueId_t get_mid_MsgQueueRGB(void);
int Init_ThRGB (void);

int Init_Th_RGB_Test(void);

#define MSGQUEUE_OBJECTS_RGB 16


#endif
