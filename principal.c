#include "string.h"
#include "stdio.h"
#include "LCD.h"
#include "clock.h"
#include "joystick.h"
#include "com.h"
#include "TCS34725.h"
#include "principal.h"
#include "LED_stm.h"
#include "LED_stm_azul.h"
#include "rgb.h"

/*-----BUFFER CIRCULAR -----*/

#define SUBARRAY_SIZE 12
#define ARRAY_SIZE 50

int circularArray[ARRAY_SIZE][SUBARRAY_SIZE];
int head = 0;
int lastPosition = 0;

int rPercent = 0;
int gPercent = 0;
int	bPercent = 0;

/*----- RGB -----*/
typedef struct{
	uint16_t red;
	uint16_t green;
	uint16_t blue;
}MSGQUEUE_OBJ_TCS;
MSGQUEUE_OBJ_TCS msg_TCS_RGB_rx;

MSGQUEUE_OBJ_TCS msg_RGB_enviar;

/*----- JoyStick -----*/

typedef struct{
  direccion dir;
    pulso pul;
}MSGQUEUE_JOY;
MSGQUEUE_JOY JOY_recep;

/*-----Variables y Metodos-----*/
void LCD_INACTIVO(void);
void LCD_ACTIVO(void);
void LCD_MEDIDA(void);
void shift_array(void);
static uint8_t cambio_hora(MSGQUEUE_OBJ_COM msg, int hour, int minute, int second);
static uint8_t get_ciclo(MSGQUEUE_OBJ_COM msg_rx);
static void lastArrayPosition(void);
static void todasLasMedidas(void);

uint8_t valor_manual = 6;
uint8_t contador_manual;
int num_ciclo = 0;
int time_ciclo = 0;
uint8_t lcd_ciclo = 0;
int num_medidas = 0;
int valor_array[SUBARRAY_SIZE] = {0}; 
int num_med_alm = 0;

char ascii_valor = 0;

typedef enum{
	UNLOCK,
	LOCK
}Padlock_t;
Padlock_t candado;


/*----- BUFFER 12 -----*/
int digits[12];

/*----- Estados -----*/
typedef enum{
	INACTIVO,
	ACTIVO,
	MEDIDA
}Estados;
Estados estado;

/*----- Cuenta atras -----*/
typedef enum{
	CICLO,
	MANUAL,
}Estado_cuenta;
Estado_cuenta modo;

void convertToPorcentage(int red, int green, int blue, int* rPercent, int* gPercent, int* bPercent);


 
 /*---------------------------------------------------
	*      							Reception 
	*---------------------------------------------------*/

/*----- Hora -----*/
int hour=0;
int minute=0;
int second=0;
int sec_prev=61;

int hora;
int minuto;
int segundo;

/*----- JoyStick -----*/
MSGQUEUE_JOY JOY_recep;

/*----- ComPC -----*/
MSGQUEUE_OBJ_COM msg_com_rx;


 /*---------------------------------------------------
	*      								Send 
	*---------------------------------------------------*/

/*---------- LCD ----------*/
MSGQUEUE_LCD LCD_msg_send;

/*---------- COM_PC ----------*/
MSGQUEUE_OBJ_COM msg_com_tx;

/*----- LEDSTM -----*/
extern osThreadId_t id_ThLED_stm;
extern osThreadId_t id_ThLED_stm_azul;

/*---- TCS -----*/
extern osThreadId_t id_ThTCS34725;

/*---- FLAGS ----*/
#define LED_VERDE_ON 0x01
#define LED_AZUL_ON 0x10

/*---- PERIODIC TIMER ----*/
osTimerId_t tim_id;
static uint32_t exec;

/*---- FUNCIONES ----*/
void Medida_Manual(void);

 osStatus_t status_msg;
 
/*---------------------------------------------------
 *      					Thread Principal
 *---------------------------------------------------*/
 
osThreadId_t tid_ThPrincipal;                        // thread id
 
void ThPrincipal (void *argument);                   // thread function
 
int Init_ThPrincipal (void) {
 
  tid_ThPrincipal = osThreadNew(ThPrincipal, NULL, NULL);
  if (tid_ThPrincipal == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThPrincipal (void *argument) {
	estado = INACTIVO;
	modo = MANUAL;
	
	Init_Timer_CNT_DOWN();
	contador_manual = valor_manual;
	
  while (1) {
		switch (estado){
			case INACTIVO:
				//LCD_INACTIVO();				//Llama a la funcion que pinta SOLO la hora
				LCD_msg_send.clean = 0;
				if (osOK == osMessageQueueGet(get_id_MsgQueue_joystick(), &JOY_recep, NULL, 0U)){					
					if (JOY_recep.dir==UP && JOY_recep.pul==Largo){
						
						
						estado = ACTIVO;
						osThreadFlagsSet(id_ThLED_stm, LED_VERDE_ON);  //Parpadeo del led verde
						
						LCD_msg_send.clean = 1;
						osMessageQueueReset(get_id_MsgQueue_com_rx());
					}else if (JOY_recep.dir==DOWN){
				
				}
			}
			break;
			case ACTIVO:
				//LCD_ACTIVO();
				LCD_msg_send.clean = 0;
			
				if (osOK == osMessageQueueGet(get_id_MsgQueue_com_rx(), &msg_com_rx, NULL, 0U)){					//Recibe Configuracion Hora
					if(status_msg == osOK){
						switch (msg_com_rx.op){
							case SET_HORA:
								
								if (cambio_hora(msg_com_rx, hora, minuto, segundo)==0){
									msg_com_tx.op = CMD_PUESTA_EN_HORA_SEND;
									msg_com_tx.size = msg_com_rx.size;	
									memcpy(msg_com_tx.data, msg_com_rx.data, msg_com_rx.size);
									osMessageQueuePut(get_id_MsgQueue_com_tx(), &msg_com_tx, 0U, 0U);
								
									osThreadFlagsSet(id_ThLED_stm, LED_VERDE_ON);  //Parpadeo del led verde
									estado = MEDIDA;
									LCD_msg_send.clean = 1;
									osMessageQueueReset(get_id_MsgQueue_com_rx());
								}
								
								
								

							break;

							default:
								
							break;
						}
					}	
				}				
				if (osOK == osMessageQueueGet(get_id_MsgQueue_joystick(), &JOY_recep, NULL, 0U)){				//Cambio de modo inactivo	
					if (JOY_recep.dir==UP && JOY_recep.pul==Largo){
						osThreadFlagsSet(id_ThLED_stm, LED_VERDE_ON);  //Para el parpadeo del led verde						
						estado = INACTIVO;
						LCD_msg_send.clean = 1;
						osMessageQueueReset(get_id_MsgQueue_com_rx());
					}
				}
			
			break;
			case MEDIDA:
				//LCD_MEDIDA();
				LCD_msg_send.clean = 0;
				if (osOK == osMessageQueueGet(get_id_MsgQueue_joystick(), &JOY_recep, NULL, 0U)){					
					if (JOY_recep.dir==UP && JOY_recep.pul==Largo){							//Volver a INACTIVO
						
						
						estado = INACTIVO;
						LCD_msg_send.clean = 1;
					}
					if (JOY_recep.dir==DOWN && JOY_recep.pul==Corto){						//Realizar medida CACHAU
						
						if(num_ciclo > 0){
							
						}else{
							modo = MANUAL;
							candado = LOCK;
							osTimerStart(tim_id, 1000U);
						}
					}
				}
				if (osOK == osMessageQueueGet(get_id_MsgQueue_com_rx(), &msg_com_rx, NULL, 0U)){
					if(status_msg == osOK){
						if (candado == LOCK){
							osMessageQueueReset(get_id_MsgQueue_com_rx());
						}else{
							switch (msg_com_rx.op){
								case SET_HORA:
									if (cambio_hora(msg_com_rx, hora, minuto, segundo)==0){
										msg_com_tx.op = CMD_PUESTA_EN_HORA_SEND;
										msg_com_tx.size = msg_com_rx.size;	
										memcpy(msg_com_tx.data, msg_com_rx.data, msg_com_rx.size);
										osMessageQueuePut(get_id_MsgQueue_com_tx(), &msg_com_tx, 0U, 0U);
									
									}
								
								break;
								case CUENTA_ATRAS:									
									valor_manual = msg_com_rx.data[0] - '0';
									if (valor_manual!= 0){
										contador_manual = valor_manual;
								
										msg_com_tx.op = CMD_CUENTA_ATRAS_SEND;
										msg_com_tx.size = msg_com_rx.size;	
										memcpy(msg_com_tx.data, msg_com_rx.data, msg_com_rx.size);
										osMessageQueuePut(get_id_MsgQueue_com_tx(), &msg_com_tx, 0U, 0U);
									}						
									
								break;
								case LECTURA_C_ATRAS:
									
									msg_com_tx.op = CMD_RESPU_CUENTA_ATRAS_SEND;
									msg_com_tx.size = 0x01;		//La ponemmos a 1 ya que no recibe data pero si envia data
									sprintf(msg_com_tx.data, "%X", valor_manual);
									osMessageQueuePut(get_id_MsgQueue_com_tx(), &msg_com_tx, 0U, 0U);
									
								break;
								case CICLO_MEDIDAS:
								
									modo = CICLO;
									get_ciclo(msg_com_rx);
									lcd_ciclo = 1;
									candado = LOCK;
									osTimerStart(tim_id, time_ciclo*1000U);
									
								//Nuevo timer periodico que te cada x tiempo tome captura.
								//No devuelve nada
								
								break;
								case NUM_M_ALMACENADAS:
									msg_com_tx.op = CMD_MED_ALMACENADAS_SEND;
									msg_com_tx.size = 0x01;		//La ponemmos a 1 ya que no recibe data pero si envia data
									sprintf(msg_com_tx.data, "%X", num_medidas);
									osMessageQueuePut(get_id_MsgQueue_com_tx(), &msg_com_tx, 0U, 0U);
									
								break;
								case ULTIMA_MEDIDA:
									lastArrayPosition();
									msg_com_tx.num_med_alm = num_medidas;
									msg_com_tx.op = CMD_MEDIDA_SEND;
									msg_com_tx.size = 17;
									sprintf(msg_com_tx.data, "%c%c:%c%c:%c%c-%c%c-%c%c-%c%c",
									valor_array[0] + '0', valor_array[1] + '0',
									valor_array[2] + '0', valor_array[3] + '0',
									valor_array[4] + '0', valor_array[5] + '0',
									valor_array[6] + '0', valor_array[7] + '0',
									valor_array[8] + '0', valor_array[9] + '0',
									valor_array[10] + '0', valor_array[11] + '0');
									osMessageQueuePut(get_id_MsgQueue_com_tx(), &msg_com_tx, 0U, 0U);
								
								break;
								case TODAS_MEDIDAS:
									todasLasMedidas();
								
								break;
								case BORRAR_MEDIDAS:
									msg_com_tx.op = CMD_RESP_BORRAR_MEDIDAS_SEND;
									msg_com_tx.size = msg_com_rx.size;	
									memcpy(msg_com_tx.data, msg_com_rx.data, msg_com_rx.size);
									osMessageQueuePut(get_id_MsgQueue_com_tx(), &msg_com_tx, 0U, 0U);
									// Borrar el contenido del array circularArray
									memset(circularArray, 0, sizeof(circularArray));
									num_medidas = 0;
									num_med_alm = 0;
									head = 0;

								break;

								default:
									
								break;
						
							}
						}
					}
				}

			break;
		}	
		osThreadYield();
  }
}

/*----- TODAS LAS POSICIONES DEL ARRAY -----*/
static void todasLasMedidas(void){
	msg_com_tx.num_med_alm = num_medidas;
	
	for (int i = 0; i < num_medidas; i++) {
    for (int j = 0; j < SUBARRAY_SIZE; j++) {
        ascii_valor = circularArray[i][j] + '0';  // Conversión a ASCII
    

    msg_com_tx.op = CMD_MEDIDA_SEND;
    msg_com_tx.size = 17;
    sprintf(msg_com_tx.data, "%c%c:%c%c:%c%c-%c%c-%c%c-%c%c",
            circularArray[i][0] + '0', circularArray[i][1] + '0',
            circularArray[i][2] + '0', circularArray[i][3] + '0',
            circularArray[i][4] + '0', circularArray[i][5] + '0',
            circularArray[i][6] + '0', circularArray[i][7] + '0',
            circularArray[i][8] + '0', circularArray[i][9] + '0',
            circularArray[i][10] + '0', circularArray[i][11] + '0');
    

			}
		
		osMessageQueuePut(get_id_MsgQueue_com_tx(), &msg_com_tx, 0U, 0U);	

	}
	
}
/*----- ULTIMA POSICION ARRAY -----*/

static void lastArrayPosition(void){
	
	// Obtener la última posición del array circular
	lastPosition = (head == 0) ? (ARRAY_SIZE - 1) : (head - 1);

	// Acceder al subarray en la última posición
	int* lastSubArray = circularArray[lastPosition];
	
		for( int i = 0; i < SUBARRAY_SIZE; i++){
		
			valor_array[i]=lastSubArray[i];
		
	}
	
}

/*----- CONVERTIR A DIGITOS -----*/

void extractDigits(int value, int* digits) {
  digits[0] = (value / 10) % 10;
  digits[1] = value % 10;
}

/*----- CONVERTIR RGB A % -----*/

void convertToPorcentage(int red, int green, int blue, int* rPercent, int* gPercent, int* bPercent) {
  // Verificar si los valores de entrada están en el rango válido
  if (red < 0 || red > 255 || green < 0 || green > 255 || blue < 0 || blue > 255) {
    return;
  }

  // Convertir los valores RGB a porcentajes
  *rPercent = (red * 100) / 255;
  *gPercent = (green * 100) / 255;
  *bPercent = (blue * 100) / 255;
	if(*rPercent == 100){*rPercent = 99;}
	if(*gPercent == 100){*gPercent = 99;}
	if(*bPercent == 100){*bPercent = 99;}
}

/*-----Cambiar Hora-----*/
static uint8_t cambio_hora(MSGQUEUE_OBJ_COM msg_rx, int hora, int minuto, int segundo){ 
	
	sscanf(msg_rx.data, "%d:%d:%d", &hora, &minuto, &segundo);

	if (hora>=0 && hora<24){
		if (minuto>=0 && minuto<60){
			if (segundo>=0 && segundo<60){
					hour = hora;
					minute = minuto;
					second = segundo;
					return 0;
			}else{
				return -1;
			}			
		}else{
			return -1;
		}
	}else {
		return -1;
	}	
}

static uint8_t get_ciclo(MSGQUEUE_OBJ_COM msg_rx){ 
	
	sscanf(msg_rx.data, "N%dT%d", &num_ciclo, &time_ciclo);

	if (num_ciclo>0 && num_ciclo<100){
		if (time_ciclo>0 && time_ciclo<100){
			return 0;
			//Arrancar el timer periodico con time_ciclo como tiempo hasta que num_ciclo sea 0 (restar desde la callback)
		}else{
			return -1;
		}
	}else {
		return -1;
	}	
}



/*---------------------------------------------------
 *      							Metodos TIMER
 *---------------------------------------------------*/

static void Timer_Callback (void const *arg) {
	switch(modo){
		case MANUAL:
				contador_manual--;
				sprintf(LCD_msg_send.data_2, "%d", contador_manual);
				osMessageQueuePut(get_id_MsgQueue_LCD(), &LCD_msg_send, 0U, 0U);
				if(contador_manual == 0){
					osTimerStop(tim_id);	
					osThreadFlagsSet(id_ThLED_stm_azul, LED_AZUL_ON);
					osThreadFlagsSet(id_ThTCS34725, 0x100);
					contador_manual = valor_manual;
					candado = UNLOCK;
				if (osOK == osMessageQueueGet(get_id_TCS_MsgQueue_tx(), &msg_TCS_RGB_rx, NULL, osWaitForever)){
					if(status_msg == osOK){
						msg_RGB_enviar.red = msg_TCS_RGB_rx.red;
						msg_RGB_enviar.green = msg_TCS_RGB_rx.green;
						msg_RGB_enviar.blue = msg_TCS_RGB_rx.blue;
						convertToPorcentage(msg_RGB_enviar.red, msg_RGB_enviar.green,msg_RGB_enviar.blue, &rPercent, &gPercent, &bPercent);
						sprintf(LCD_msg_send.data_2, "   R:%02d%sG:%02d%sB:%02d%s", rPercent,"%", gPercent,"%", bPercent,"%");
						osMessageQueuePut(get_id_MsgQueue_LCD(), &LCD_msg_send, 0U, 0U);
						
						// Llenar el subarray con los valores de horas, minutos, segundos y RGB
						extractDigits(hour, &circularArray[head][0]);
						extractDigits(minute, &circularArray[head][2]);
						extractDigits(second, &circularArray[head][4]);

						extractDigits(rPercent, &circularArray[head][6]);
						extractDigits(gPercent, &circularArray[head][8]);
						extractDigits(bPercent, &circularArray[head][10]);

						// Actualizar el puntero "head" para la próxima inserción
						head = (head + 1) % ARRAY_SIZE;
						
						osMessageQueuePut(get_mid_MsgQueueRGB(), &msg_RGB_enviar, 0U, 0U);
						
						num_medidas ++; // valor que almacena el numero de medidas realizadas
						}
					}
				}
				
		break;
		case CICLO:
			num_ciclo--;
			osThreadFlagsSet(id_ThLED_stm_azul, LED_AZUL_ON);
			osThreadFlagsSet(id_ThTCS34725, 0x100);
			if (osOK == osMessageQueueGet(get_id_TCS_MsgQueue_tx(), &msg_TCS_RGB_rx, NULL, osWaitForever)){
				if(status_msg == osOK){
					msg_RGB_enviar.red = msg_TCS_RGB_rx.red;
					msg_RGB_enviar.green = msg_TCS_RGB_rx.green;
					msg_RGB_enviar.blue = msg_TCS_RGB_rx.blue;
					convertToPorcentage(msg_RGB_enviar.red, msg_RGB_enviar.green,msg_RGB_enviar.blue, &rPercent, &gPercent, &bPercent);
					sprintf(LCD_msg_send.data_2, "   R:%02d%sG:%02d%sB:%02d%s", rPercent,"%", gPercent,"%", bPercent,"%");
					osMessageQueuePut(get_id_MsgQueue_LCD(), &LCD_msg_send, 0U, 0U);
						
					// Llenar el subarray con los valores de horas, minutos, segundos y RGB
					extractDigits(hour, &circularArray[head][0]);
					extractDigits(minute, &circularArray[head][2]);
					extractDigits(second, &circularArray[head][4]);

					extractDigits(rPercent, &circularArray[head][6]);
					extractDigits(gPercent, &circularArray[head][8]);
					extractDigits(bPercent, &circularArray[head][10]);

					// Actualizar el puntero "head" para la próxima inserción
					head = (head + 1) % ARRAY_SIZE;
						
					osMessageQueuePut(get_mid_MsgQueueRGB(), &msg_RGB_enviar, 0U, 0U);
					
					num_medidas ++; // valor que almacena el numero de medidas realizadas
				}
				if (num_ciclo == 0){
					osTimerStop(tim_id);
					lcd_ciclo = 0;
					candado = UNLOCK;
					modo = MANUAL;
				}
			
		break;
		default:
		break;
	}
 }

	
 
}
/*---- INIT TIMER ----*/
int Init_Timer_CNT_DOWN(void) {
  osStatus_t status;                            // function return status
  exec = 2U;
  tim_id = osTimerNew((osTimerFunc_t)&Timer_Callback, osTimerPeriodic, &exec, NULL);
  if (tim_id != NULL) {  // Periodic timer created
    // start timer with periodic 1000ms interval
           
    if (status != osOK) {
      return -1;
    }
  }
  return NULL;
}

/*---------------------------------------------------
 *      							Metodos LCD
 *---------------------------------------------------*/

void LCD_hora(void){
	switch(estado){
		case INACTIVO:
			sprintf(LCD_msg_send.data_1, "%02d:%02d:%02d _ Inactivo", hour, minute, second);
			sprintf(LCD_msg_send.data_2, "          ");
			sec_prev = second;
			osMessageQueuePut(get_id_MsgQueue_LCD(), &LCD_msg_send, 0U, 0U);
		break;
		case ACTIVO:
			sprintf(LCD_msg_send.data_1, "%02d:%02d:%02d _ Activo    ", hour, minute, second);
			sec_prev = second;
			osMessageQueuePut(get_id_MsgQueue_LCD(), &LCD_msg_send, 0U, 0U);
		break;
		case MEDIDA:
			if (lcd_ciclo == 1){
				sprintf(LCD_msg_send.data_1, "%02d:%02d:%02d _ C      ", hour, minute, second);
				sprintf(LCD_msg_send.data_2, "%02d", num_ciclo);
				sec_prev = second;
				osMessageQueuePut(get_id_MsgQueue_LCD(), &LCD_msg_send, 0U, 0U);
			}else{
				sprintf(LCD_msg_send.data_1, "%02d:%02d:%02d _ Medida ", hour, minute, second);
				sec_prev = second;
				if (candado == UNLOCK){
					sprintf(LCD_msg_send.data_2, "%d", valor_manual);
				}
				osMessageQueuePut(get_id_MsgQueue_LCD(), &LCD_msg_send, 0U, 0U);
				
			}
		break;
	}
}


void LCD_ACTIVO(void){
	//Linea 2 del LCD
	sprintf(LCD_msg_send.data_2, "Mantener UP: salir");
	osMessageQueuePut(get_id_MsgQueue_LCD(), &LCD_msg_send, 0U, 0U);
	osDelay(1000);
}

void LCD_MEDIDA(void){
	

	//Linea 2 del LCD
	//sprintf(LCD_msg_send.data_2, "C:%02d  R:%.1f  G:%02d B:%02d",contador_medida, 0, 0, 0);
	
	switch(modo){
		case MANUAL:
			//Linea 2 del LCD
			sprintf(LCD_msg_send.data_2, "   R:%02d%sG:%02d%sB:%02d%s", rPercent,"%", gPercent,"%", bPercent,"%");	
			//sprintf(LCD_msg_send.data_2, "C:%02d R:%02d%sG:%02d%sB:%02d%s" ,contador_manual, digits[0],"%", digits[1],"%", digits[2],"%");	
			osMessageQueuePut(get_id_MsgQueue_LCD(), &LCD_msg_send, 0U, 0U);
		break;
		case CICLO:
			//Linea 2 del LCD
			//sprintf(LCD_msg_send.data_2, "       C:%02d",contador_manual);	
			sprintf(LCD_msg_send.data_2, " R:%02d G:%02d B:%02d", rPercent, gPercent, bPercent);
			osMessageQueuePut(get_id_MsgQueue_LCD(), &LCD_msg_send, 0U, 0U);
		break;
		default:
		break;
	}
	

	osDelay(1000);
}


osThreadId_t tid_ThLCD_Clock;                        // thread id
 
void ThLCD_Clock (void *argument);                   // thread function
 
int Init_ThLCD_Clock (void) {
 
  tid_ThLCD_Clock = osThreadNew(ThLCD_Clock, NULL, NULL);
  if (tid_ThLCD_Clock == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThLCD_Clock (void *argument) {
 
  while (1) {
		if (second != sec_prev){
			LCD_hora();
		}
    osThreadYield();                            // suspend thread
  }
}
