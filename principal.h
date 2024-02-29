#ifndef _PRINCIPAL_H
#define _PRINCIPAL_H

	#include "RTE_Device.h"
	#include "stm32f4xx_hal.h"
	#include "cmsis_os2.h"  
	
	//#define BUFFER_SIZE 16
	
	int Init_ThPrincipal(void);
	int Init_ThLCD_Clock (void);
	int Init_Timer_CNT_DOWN(void);
	osMessageQueueId_t get_id_MsgQueue_LCD(void);
	
	/*---- COMANDOS DEL PC AL SISTEMA ----*/
	#define CMD_PUESTA_EN_HORA_REC 		0x20
	#define CMD_CUENTA_ATRAS_REC 			0x25
	#define CMD_LEER_CUENTA_ATRAS_REC 0x35
	#define	CMD_CICLO_MEDIDAS_REC 		0x70
	#define	CMD_NUM_MEDIDAS_REC 			0x40
	#define	CMD_ULTIMA_MEDIDA_REC 		0x50
	#define	CMD_TODAS_MEDIDAS_REC			0x55
	#define	CMD_BORRAR_MEDIDAS_REC		0x60
	/*---- COMANDOS DEL SISTEMA AL PC ----*/
	#define	CMD_PUESTA_EN_HORA_SEND   0xDF
	#define	CMD_CUENTA_ATRAS_SEND     0xDA
	#define	CMD_RESPU_CUENTA_ATRAS_SEND  0xCA
	#define	CMD_MED_ALMACENADAS_SEND	0xBF
	#define	CMD_MEDIDA_SEND						0xAF
	#define	CMD_RESP_BORRAR_MEDIDAS_SEND 0x9F
	
	

#endif
