#include "com.h"
#include "stdio.h"
#include "string.h"

#define INIT_DATAGRAM 0x01
#define END_DATAGRAM 0xFE
#define BLOCK_TX_BUFFER 1<<31 

void Th_com (void *argument); 
void USART3_callback(uint32_t event);

osThreadId_t tid_pc_com_Thread;
osMessageQueueId_t id_MsgQueue_com_rx;			//Hilo para recibir del PC al micro
osMessageQueueId_t id_MsgQueue_com_tx;			//Hilo para transmitir del micro al pc
osMessageQueueId_t id_MsgQueue_com_pad;			//Hilo para recibir del PC al micro

extern ARM_DRIVER_USART Driver_USART3;
static ARM_DRIVER_USART *USARTdrv = &Driver_USART3;
void USART3_callback(uint32_t event){
	if(event & ARM_USART_EVENT_RECEIVE_COMPLETE){
		osThreadFlagsSet(tid_pc_com_Thread, ARM_USART_EVENT_RECEIVE_COMPLETE); //Le paso lo que ha sucedido
	}
	if(event & ARM_USART_EVENT_TX_COMPLETE) {
		//Esta seccion es para evitar que se produzca un bottleneck en las transmisiones, cuando se intenta escribir
		//el buufer de tx mientras que se esta produciendo otra tx
		//v1
		osThreadFlagsSet(tid_pc_com_Thread, ARM_USART_EVENT_TX_COMPLETE);
		//V2
		//osThreadFlagsSet(tid_pc_com_Thread, ARM_USART_EVENT_TX_COMPLETE);
		//osThreadFlagsSet(tid_pc_com_Thread, BLOCK_TX_BUFFER);
	}
}


void convertToHex(const char* input, char* output, int length) {
	int i;
	for (i = 0; i < length; i++) {
			sprintf(&output[i * 3], "%02X ", input[i]);
	}
}


static int conf_USART3(void){
	USARTdrv->Initialize(USART3_callback);
	USARTdrv->PowerControl(ARM_POWER_FULL);
	USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
										ARM_USART_DATA_BITS_8				|
										ARM_USART_PARITY_NONE				|
										ARM_USART_STOP_BITS_1				|
										ARM_USART_FLOW_CONTROL_NONE,
										115200);
	USARTdrv->Control(ARM_USART_CONTROL_RX, 1);
	USARTdrv->Control(ARM_USART_CONTROL_TX, 1);
	//writeUsart("\n\rHola mundo\n\r",14); //No podemos usar flags si el OS no a arrancado
	USARTdrv->Send("\n\rIniciando\n\r", 13);
	return 0;
}

/*----- Getter id Queue -----*/
osMessageQueueId_t get_id_MsgQueue_com_rx(void){
	return id_MsgQueue_com_rx;
}
osMessageQueueId_t get_id_MsgQueue_com_tx(void){
	return id_MsgQueue_com_tx;
}
osMessageQueueId_t get_id_MsgQueue_com_pad(void){
	return id_MsgQueue_com_pad;
}

int Init_MsgQueue_com(void){
	id_MsgQueue_com_rx = osMessageQueueNew(MSGQUEUE_OBJECTS_COM, sizeof(MSGQUEUE_OBJ_COM), NULL);
	id_MsgQueue_com_tx = osMessageQueueNew(MSGQUEUE_OBJECTS_COM, sizeof(MSGQUEUE_OBJ_COM), NULL);
	id_MsgQueue_com_pad = osMessageQueueNew(MSGQUEUE_OBJECTS_COM, sizeof(MSGQUEUE_OBJ_COM), NULL);
	if((id_MsgQueue_com_rx == NULL) && (id_MsgQueue_com_tx == NULL) && (id_MsgQueue_com_pad == NULL)){
		return -1;
	}
	return 0;
}
int Init_Th_com (void) {
	conf_USART3();
  tid_pc_com_Thread = osThreadNew(Th_com, NULL, NULL);
	osThreadSetPriority(tid_pc_com_Thread, osPriorityRealtime7);
  if (tid_pc_com_Thread == NULL) {
    return(-1);
  }
  return(Init_MsgQueue_com());
}
 
void Th_com (void *argument) {
	uint8_t cmd[64];
	char char_tx[64] = {0};
	char char_tx_data[64];
	uint8_t operation;
	uint8_t size;
	static MSGQUEUE_OBJ_COM msg_rx;
	static MSGQUEUE_OBJ_COM msg_tx;
//	static MSGQUEUE_OBJ_COM_PAD msg_pad_rx;
	static osStatus_t queue_status_pad;
	static osStatus_t queue_status;
//	Padlock_t candado = UNLOCK;
	
	
	while(1){
	
		USARTdrv->Receive(&cmd, 3);	
		
		//Recibimos 3 bytes.
			//1º Indica inicio de trama 0x01
			//2º Indica tipo de operacion , opciones: 0x20, 0x25, 0x35, 0x70, 0x40, 0x50, 0x55, 0x60
			//3º Indica tamaño del resto de la trama
		osThreadFlagsWait(ARM_USART_EVENT_RECEIVE_COMPLETE, osFlagsWaitAny, osWaitForever);
		if(cmd[0] == INIT_DATAGRAM){	//&&((proc_flags & ARM_USART_EVENT_RECEIVE_COMPLETE)!= 0)
			//Solo entramos si el evento de recepcion se ha producido correctamente. 
			//Esto evita que si se produzca otra condicion se quede bloqueado
			operation = cmd[1];
			size = (cmd[2]-4);
			USARTdrv->Receive(&cmd, size+1);
			
			osThreadFlagsWait(ARM_USART_EVENT_RECEIVE_COMPLETE, osFlagsWaitAny, 500U);//osWaitForever);	
			//Esperamos la condicion de haber llenado el buffer. 
			
			if(cmd[size] == END_DATAGRAM){
				//writeUsartln("Hemos entrado en 1", 18);
//					queue_status_pad=osMessageQueueGet(id_MsgQueue_com_pad, &msg_pad_rx, NULL, 1000U);
//					candado = msg_pad_rx.padlock;
				
						//candado = msg_rx.padlock;
					msg_rx.op = operation;
					memcpy(msg_rx.data, cmd, size);
					msg_rx.size = size;	//Guardamos el tamaño del mensaje
//					if (candado == UNLOCK){
						queue_status=osMessageQueuePut(id_MsgQueue_com_rx, &msg_rx, 0U, 0U);
					
					queue_status=osMessageQueueGet(id_MsgQueue_com_tx, &msg_tx, NULL, 1000U);
					
							if(msg_tx.op != TODAS_MEDIDAS_SEND){
								if(queue_status == osOK){
								char_tx[0] = INIT_DATAGRAM;							//SOH
								char_tx[1] = msg_tx.op;									//CMD
								char_tx[2] = msg_tx.size+4;							//LEN			
									for(int i=0; i<=msg_tx.size; i++){
										char_tx[i+3] = msg_tx.data[i];					//Data
									}
								char_tx[msg_tx.size+3] = END_DATAGRAM;		//EOT
						
						
								convertToHex(char_tx, char_tx_data, sizeof(char_tx) - 1);
						

								USARTdrv->Send(char_tx_data, (msg_tx.size+4)*3);
						
								for (int i = 0; i < 32; i++){
									msg_rx.data[i] = 0;
								}

						
								osDelay(100);
								USARTdrv->Send("\n\r", 2);
							}
							}
						else{
						while (msg_tx.num_med_alm != 0){			

							if(queue_status == osOK){
								char_tx[0] = INIT_DATAGRAM;							//SOH
								char_tx[1] = msg_tx.op;									//CMD
								char_tx[2] = msg_tx.size+4;							//LEN			
									for(int i=0; i<=msg_tx.size; i++){
										char_tx[i+3] = msg_tx.data[i];					//Data
									}
								char_tx[msg_tx.size+3] = END_DATAGRAM;		//EOT
						
						
								convertToHex(char_tx, char_tx_data, sizeof(char_tx) - 1);
						

								USARTdrv->Send(char_tx_data, (msg_tx.size+4)*3);
						
								for (int i = 0; i < 32; i++){
									msg_rx.data[i] = 0;
								}

						
								osDelay(100);
								USARTdrv->Send("\n\r", 2);
							}
						queue_status=osMessageQueueGet(id_MsgQueue_com_tx, &msg_tx, NULL, 1000U);
						msg_tx.num_med_alm = msg_tx.num_med_alm--;
					}
						
						}
//					}else{
//						USARTdrv->Send("Denied: measure in progress\n\r", 29);
//					}

			}

		}
	
		osMessageQueueReset(get_id_MsgQueue_com_rx());
	}
	
}
