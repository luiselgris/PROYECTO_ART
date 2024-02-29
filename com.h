#ifndef COM_H
#define COM_H

#define MSGQUEUE_OBJECTS_COM 16

#define USART_NEW_LINE		//Definir para contestar en nueva linea

#include "Driver_USART.h"
#include "cmsis_os2.h"

typedef enum{
	SET_HORA = 0x20,
	CUENTA_ATRAS = 0x25,
	LECTURA_C_ATRAS = 0x35,
	CICLO_MEDIDAS =0x70,
	NUM_M_ALMACENADAS = 0x40,
	ULTIMA_MEDIDA = 0x50,
	TODAS_MEDIDAS = 0x55,
	TODAS_MEDIDAS_SEND = 0xAF,
	BORRAR_MEDIDAS = 0x60
}Operaciones_t;

//typedef enum{
//	UNLOCK,
//	LOCK
//}Padlock_t;

//typedef struct{
//	Padlock_t padlock;
//}MSGQUEUE_OBJ_COM_PAD;

typedef struct{
	Operaciones_t op;
	uint8_t size;
	char data[32];
	uint8_t num_med_alm;
}MSGQUEUE_OBJ_COM;

int Init_Th_com(void);
osMessageQueueId_t get_id_MsgQueue_com_rx(void);
osMessageQueueId_t get_id_MsgQueue_com_tx(void);
//osMessageQueueId_t get_id_MsgQueue_com_pad(void);

#endif
