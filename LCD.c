#include "LCD.h"
#include "Arial12x12.h"
#include "Driver_SPI.h"

void LCD_init(void);
void LCD_update(void);
void LCD_reset(void);
void LCD_Clean(void);
void LCD_write(uint8_t line, char str[]);

/*---------------------------------------------------
 *      			Cola Escritura (Recibida)
 *---------------------------------------------------*/

osMessageQueueId_t LCD_MsgQueue_send;

int Init_MsgQueue_LCD_send (void) { 
  LCD_MsgQueue_send = osMessageQueueNew(16, sizeof(MSGQUEUE_LCD), NULL);
  if (LCD_MsgQueue_send == NULL) {
    
  }
  return(0);
}

osMessageQueueId_t get_id_MsgQueue_LCD(void){
	return LCD_MsgQueue_send;
}


MSGQUEUE_LCD LCD_recibida;

/*---------------------------------------------------
 *      							Thread LCD
 *---------------------------------------------------*/
 
osThreadId_t tid_ThLCD;                        // thread id
void ThLCD (void *argument);                   // thread function

int Init_ThLCD (void) {
 
  tid_ThLCD = osThreadNew(ThLCD, NULL, NULL);
  if (tid_ThLCD == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThLCD (void *argument) {
	
	Init_MsgQueue_LCD_send();
	LCD_reset();
	LCD_init();
	LCD_Clean();

	
  while (1) {
		
		osMessageQueueGet(get_id_MsgQueue_LCD(), &LCD_recibida, NULL, osWaitForever);
		
		if (LCD_recibida.clean == 1)
			LCD_Clean();
		
		LCD_write(1, LCD_recibida.data_1);
		LCD_write(2, LCD_recibida.data_2);

 }
}


extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;
TIM_HandleTypeDef tim7;

unsigned char buffer[512];
static uint32_t positionL1, positionL2 = 0;

static void LCD_wr_data(unsigned char data);
static void LCD_wr_cmd(unsigned char cmd);
static void delay(uint32_t n_microsegundos);
static void symbolToLocalBuffer(uint8_t line, uint8_t symbol);
static void symbolToLocalBuffer_L1(uint8_t symbol);
static void symbolToLocalBuffer_L2(uint8_t symbol);

/*---------------------------------------------------
 *      			Inicialización del LCD
 *---------------------------------------------------*/
void LCD_init(void){
	LCD_wr_cmd(0xAE);//display off
	LCD_wr_cmd(0xA2);//Fija el valor de la tensión de polarización del LCD a 1/9
	LCD_wr_cmd(0xA0);//El direccionamiento de la RAM de datos del display es la normal
	LCD_wr_cmd(0xC8);//El scan en las salidas COM es el normal
	LCD_wr_cmd(0x22);//Fija la relación de resistencias interna a 2
	LCD_wr_cmd(0x2F);//Power on
	LCD_wr_cmd(0x40);//Display empieza en la línea 0
	LCD_wr_cmd(0xAF);//Display ON
	LCD_wr_cmd(0x81);//Contraste
	LCD_wr_cmd(0x13);//Valor de contraste
	LCD_wr_cmd(0xA4);//Display all points normal
	LCD_wr_cmd(0xA6);//LCD Display Normal 0xA7->Reverse
}

/*---------------------------------------------------
 *      			Reset del LCD
 *---------------------------------------------------*/
void LCD_reset(void){
	static GPIO_InitTypeDef GPIO_InitStruct_LCD;
	/*CS*/
	__HAL_RCC_GPIOD_CLK_ENABLE();
	GPIO_InitStruct_LCD.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct_LCD.Pull = GPIO_PULLUP;
	GPIO_InitStruct_LCD.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct_LCD.Pin = GPIO_PIN_14;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct_LCD);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
	
	/*A0*/
	__HAL_RCC_GPIOF_CLK_ENABLE();
	GPIO_InitStruct_LCD.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct_LCD.Pull = GPIO_PULLUP;
	GPIO_InitStruct_LCD.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct_LCD.Pin = GPIO_PIN_13;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct_LCD);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_SET);
	
	/*Reset*/
	__HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitStruct_LCD.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct_LCD.Pull = GPIO_PULLUP;
	GPIO_InitStruct_LCD.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct_LCD.Pin = GPIO_PIN_6;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct_LCD);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
	
	/*SPI*/
	SPIdrv->Initialize(NULL);
  SPIdrv-> PowerControl(ARM_POWER_FULL);
  SPIdrv-> Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS (8), 20000000);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
	delay(1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
	delay(1000);
}

/*---------------------------------------------------
 *      			Update del LCD
 *---------------------------------------------------*/
void LCD_update(void){
	int i;	
	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB0);
	for(i = 0; i < 128; i++){
			LCD_wr_data(buffer[i]);
	}

	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB1);
	for(i = 128; i < 256; i++){
			LCD_wr_data(buffer[i]);
	}

	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB2);
	for(i = 256; i < 384; i++){
		LCD_wr_data(buffer[i]);
	}
	
	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB3);
	for(i = 384; i < 512; i++){
		LCD_wr_data(buffer[i]);
	}
}

/*---------------------------------------------------
 *      			Limpieza del LCD
 *---------------------------------------------------*/
void LCD_Clean(void){
	for(int i=0; i<512; i++)
		buffer[i]=0x00;
	LCD_update();
}

/*---------------------------------------------------
 *      			Escritura y comandos del LCD
 *---------------------------------------------------*/
static void LCD_wr_data(unsigned char data){
	static ARM_SPI_STATUS stat;
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_SET);
	SPIdrv->Send(&data, sizeof(data));
	do{
		stat = SPIdrv->GetStatus();
	}while(stat.busy);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
}

static void LCD_wr_cmd(unsigned char cmd){
	static ARM_SPI_STATUS stat;
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_RESET);
	SPIdrv->Send(&cmd, sizeof(cmd));
	do{
		stat = SPIdrv->GetStatus();
	}	while(stat.busy);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
}

/*---------------------------------------------------
 *      			Delay del LCD
 *---------------------------------------------------*/
static void delay(uint32_t n_microsegundos){
	__HAL_RCC_TIM7_CLK_ENABLE();
	tim7.Instance = TIM7;
	tim7.Init.Prescaler = 83;
	tim7.Init.Period = n_microsegundos - 1;

	HAL_TIM_Base_Init(&tim7);
	HAL_TIM_Base_Start(&tim7);
	
	while(!__HAL_TIM_GET_FLAG(&tim7, TIM_FLAG_UPDATE)){} //Bloqueante
	__HAL_TIM_CLEAR_FLAG(&tim7, TIM_FLAG_UPDATE);
		
	HAL_TIM_Base_Stop(&tim7);
	HAL_TIM_Base_DeInit(&tim7);
		
	__HAL_RCC_TIM7_CLK_DISABLE();
}

/*---------------------------------------------------
 *      			Escrituras lineas 1 y 2 del LCD
 *---------------------------------------------------*/
static void symbolToLocalBuffer(uint8_t line, uint8_t symbol){
	if(line == 1)
		symbolToLocalBuffer_L1(symbol);
	if(line == 2)
		symbolToLocalBuffer_L2(symbol);
}

static void symbolToLocalBuffer_L1(uint8_t symbol){
	uint8_t i, value1, value2;
	uint16_t offset = 0;
	
	offset = 25 * (symbol - ' ');
	
	for(i=0 ; i<12 ; i++){
		
		value1 = Arial12x12[offset + i * 2 + 1];
		value2 = Arial12x12[offset + i * 2 + 2];
		buffer[i+positionL1] = value1;
		buffer[i+128+positionL1] = value2;
	}
	positionL1 = positionL1+Arial12x12[offset];
}

static void symbolToLocalBuffer_L2(uint8_t symbol){
	uint8_t i, value1, value2;
	uint16_t offset = 0;
	
	offset = 25 * (symbol - ' ');
	
	for(i=0 ; i<12 ; i++){
		
		value1 = Arial12x12[offset + i * 2 + 1];
		value2 = Arial12x12[offset + i * 2 + 2];
		buffer[i+256+positionL2] = value1;
		buffer[i+384+positionL2] = value2;
	}
	positionL2 = positionL2+Arial12x12[offset];
}


/*---------------------------------------------------
 *    			  Metodo de escritura general
 *---------------------------------------------------*/
void LCD_write(uint8_t line, char str[]){
	int i;
	for(i=0; i<strlen(str); i++){
		symbolToLocalBuffer(line, str[i]);
	}
	positionL1 = 0;
	positionL2 = 0;
	LCD_update();
}
