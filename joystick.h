#ifndef _JOYSTICK_H
#define _JOYSTICK_H

#include "cmsis_os2.h"

#define MSGQUEUE_OBJECTS_JOY 4

//Tiempo para pulsacion Largo (ms multiplo de 50)
#define PULSACION_TIEMPO 1000

typedef enum{
	UP, 
	DOWN
}direccion;

typedef enum{
	Corto,
	Largo
}pulso;

typedef struct{
  direccion dir;
	pulso pul;
}MSGQUEUE_JOY_TRANSFER;

int Init_ThJoystick(void);
osMessageQueueId_t get_id_MsgQueue_joystick(void);

#endif
