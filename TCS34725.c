#include "TCS34725.h"

extern ARM_DRIVER_I2C Driver_I2C1;
ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C1;

/*----- Variables de control -----*/
uint32_t flag_RGB;

/*----- VARIABLES RGBC -----*/
uint16_t r, g, b, c, colorTemp, lux = 0;
uint16_t range = 254;
uint8_t normalized_r, normalized_g, normalized_b = 0;

/*----- QUEUE RGB -----*/
osMessageQueueId_t TCS_MsgQueue_tx;
int Init_TCS_message_queue(void);
typedef struct{
	uint16_t r;
	uint16_t g;
	uint16_t b;
}MSGQUEUE_OBJ_TCS;
MSGQUEUE_OBJ_TCS msg_TCS_RGB_tx;

uint8_t dataT[3]= {0x13,1,0};
uint8_t dataR[3]= {0,0,0};
uint16_t timeOut = 1;

int _tcs34725Initialised;
tcs34725Gain_t _tcs34725Gain;
tcs34725IntegrationTime_t _tcs34725IntegrationTime; 

osThreadId_t id_ThTCS34725;

static void I2C1_Init(void);
void callback_i2c(uint32_t event);
void tcs3272_init(void);

int Init_Th_TCS (void) {

  id_ThTCS34725 = osThreadNew(Th_TCS, NULL, NULL);
  if (id_ThTCS34725 == NULL) {
    return(-1);
  }
	return(Init_TCS_message_queue());
}

int Init_TCS_message_queue(void){
	TCS_MsgQueue_tx = osMessageQueueNew(MSGQUEUE_OBJECTS_TCS, sizeof(MSGQUEUE_OBJ_TCS), NULL);
	if(TCS_MsgQueue_tx ==NULL)return -1;
	return 0;
}
 

void Th_TCS (void *argument) {

	I2C1_Init();
	tcs3272_init();

		flag_RGB = osThreadFlagsWait(0x100, osFlagsWaitAny, osWaitForever);
	while(1){
		//osDelay(1000);
		if(flag_RGB	== osThreadFlagsWait(0x100, osFlagsWaitAny, 0)){
			tcs3472_test();
		}
		
		
	}
	
}

osThreadId_t get_id_Th_TCS34725(void){
    return id_ThTCS34725;
}

static void I2C1_Init(void){
  // Inicializar el controlador I2C
  I2Cdrv->Initialize(callback_i2c);
  // Configurar la alimentación del controlador I2C
  I2Cdrv->PowerControl(ARM_POWER_FULL);
  // Configurar la velocidad del bus I2C
  I2Cdrv->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
  // Realizar una limpieza del bus I2C
  I2Cdrv->Control(ARM_I2C_BUS_CLEAR, 0);
}

void callback_i2c(uint32_t event){
    osThreadFlagsSet(id_ThTCS34725, event);
}

void tcs3272_init( void )
{
	int testSensor= 0;
	testSensor=begin();
}

void tcs3472_test( void ){
	
	
	getRawData(&r, &g, &b, &c);
	uint16_t max_value = c;
	
	// Normalizar los valores de R, G y B al rango de 0 a 256
  normalized_r = (r * range) / max_value;
  normalized_g = (g * range) / max_value;
  normalized_b = (b * range) / max_value;
	
////	if(normalized_b >= 150 && normalized_r <= 80 && normalized_g <= 80){
////		normalized_b = normalized_b + 100;
////		normalized_r = 0;
////		normalized_g = 0;
////	}
//	
//	if((normalized_b > normalized_r) && (normalized_b > normalized_g) && (normalized_b >= 150)){ // AZUL 
//		normalized_b = normalized_b + 100;
//		normalized_r = 0;
//		normalized_g = 0;
//		
//	}else if((normalized_b > normalized_g) && (normalized_b > normalized_r) && (normalized_g <= 80) && (normalized_b >= 130) && (normalized_r <= 40)){ //AZUL OSCURO
//		normalized_g = 0;
//		normalized_b = normalized_b + 100;
//		normalized_r = normalized_r + 20;
//	}else if((normalized_b > normalized_g) && (normalized_b > normalized_r) && (normalized_g <= 150) && (normalized_b >= 130) && (normalized_r <= 30)){ //AZUL CIAN
//		normalized_g = normalized_g + 50;
//		normalized_b = normalized_b + 50;
//		normalized_r = 0;
//	}else if((normalized_b > normalized_g) && (normalized_g > normalized_r) && (normalized_g <= 130) && (normalized_b <= 130)){ //VERDE OSCURO
//		normalized_g = normalized_g + 100;
//		normalized_b = 70;
//		normalized_r = 0;
//	}else if((normalized_g > normalized_b) && (normalized_g > normalized_r) && (normalized_g <= 130) && (normalized_b <= 130)&& (normalized_r <= 60)){ //VERDE CLARITO
//		normalized_g = normalized_g + 100;
//		normalized_b = 0;
//		normalized_r = normalized_r + 10;
//	}else if((normalized_g > normalized_b) && (normalized_g > normalized_r) && (normalized_g >= 150)){ //VERDE
//		normalized_g = normalized_g + 100;
//		normalized_b = 0;
//		normalized_r = 0;
//	}else if((normalized_b > normalized_g) && (normalized_b <= 150) && (normalized_b >= 90) && (normalized_r) >= 65){ //MORADO CLARITO
//		normalized_b = normalized_b + 50;
//		normalized_r = normalized_r + 50;
//		normalized_g = 0;
//	}else if((normalized_b > normalized_g) && (normalized_b <= 150) && (normalized_r <= 65) && (normalized_g <=80)  ){ //MORADO OSCURO
//		normalized_b = normalized_b + 110;
//		normalized_r = normalized_r + 20;
//		normalized_g = 0;
//	}else if((normalized_r > normalized_b) && (normalized_g > normalized_b) && (normalized_r >= 70) && (normalized_g >= 70) && (normalized_b <= 70)){ //AMARILLO
//		normalized_g = normalized_g + 50;
//		normalized_r = normalized_r + 50;
//		normalized_b = 0;
//	}else if((normalized_r > normalized_b) && (normalized_r > normalized_g) && (normalized_r < 120) && (normalized_g <= 70)){ // ROJO
//		normalized_r = normalized_r + 100;
//		normalized_b = 0;
//		normalized_g = 0;
//	}else if((normalized_r > normalized_b) && (normalized_r > normalized_g) && (normalized_r >= 120) && (normalized_g > 70) && (normalized_g <= 80)) { //NARANJA ROJIZO
//		normalized_g = normalized_g - 50;
//		normalized_r = normalized_r + 100;
//		normalized_b = 0;
//	}else if((normalized_r > normalized_b) && (normalized_r > normalized_g) && (normalized_r >= 120) && (normalized_g >= 30) && (normalized_g <= 70)) { //NARANJA
//		normalized_g = normalized_g + 20;
//		normalized_r = normalized_r + 100;
//		normalized_b = 0;
//	}
	
	msg_TCS_RGB_tx.r = normalized_r;
	msg_TCS_RGB_tx.g = normalized_g;
	msg_TCS_RGB_tx.b = normalized_b;
	osMessageQueuePut(TCS_MsgQueue_tx, &msg_TCS_RGB_tx, 0U, 0U);
	
	colorTemp = calculateColorTemperature(r, g, b);
	lux = calculateLux(r, g, b);
	
	

}

void write8 (uint8_t reg, uint32_t value)
{
	uint8_t pkt[2];
	
  pkt[0] = (TCS34725_COMMAND_BIT | reg);
  pkt[1] = (value & 0xFF);
	/////////////////
	//HAL_I2C_Master_Transmit(&hi2c1, TCS34725_ADDRESS, pkt, 2, timeOut);
	I2Cdrv->MasterTransmit(TCS34725_ADDRESS, pkt, 2, false);
	while (I2Cdrv->GetStatus().busy){ 																	//Espera hasta bus libre
		}
}

/**************************************************************************/
/*!
    @brief  Reads an 8 bit value over I2C
*/
/**************************************************************************/
uint8_t read8(uint8_t reg){
	
	uint8_t pkt[2];
	
  pkt[0] = (TCS34725_COMMAND_BIT | reg);
	
	//HAL_I2C_Master_Transmit(&hi2c1, TCS34725_ADDRESS, pkt, 1, timeOut);
  I2Cdrv->MasterTransmit(TCS34725_ADDRESS, pkt, 1, false);
	while (I2Cdrv->GetStatus().busy){ 																	//Espera hasta bus libre
		}
	//HAL_I2C_Master_Receive(&hi2c1, TCS34725_ADDRESS, pkt, 1, timeOut);
	I2Cdrv->MasterReceive(TCS34725_ADDRESS, pkt, 1, false);
	while (I2Cdrv->GetStatus().busy){ 																	//Espera hasta bus libre
		}
	return pkt[0];
}

/**************************************************************************/
/*!
    @brief  Reads a 16 bit values over I2C
*/
/**************************************************************************/
uint16_t read16(uint8_t reg)
{
  uint16_t x;
	uint8_t Tpkt[2],Rpkt[2];

  Tpkt[0] = (TCS34725_COMMAND_BIT | reg);
	//HAL_I2C_Master_Transmit(&hi2c1, TCS34725_ADDRESS, Tpkt, 1, timeOut);
  I2Cdrv->MasterTransmit(TCS34725_ADDRESS, Tpkt, 1, false);
//	while (I2Cdrv->GetStatus().busy){ 																	//Espera hasta bus libre
//		}
	//HAL_I2C_Master_Receive(&hi2c1, TCS34725_ADDRESS, Rpkt, 2, timeOut);
	I2Cdrv->MasterReceive(TCS34725_ADDRESS, Rpkt, 2, false);
//	while (I2Cdrv->GetStatus().busy){ 																	//Espera hasta bus libre
//		}
	x = Rpkt[0];
  x <<= 8;
  x |= Rpkt[1];
	
  return x;
}

void enable(void)
{
  write8(TCS34725_ENABLE, TCS34725_ENABLE_PON);
  write8(TCS34725_ENABLE, TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN);  
}

void disable(void)
{
  /* Turn the device off to save power */
  uint8_t reg = 0;
  reg = read8(TCS34725_ENABLE);
  write8(TCS34725_ENABLE, reg & ~(TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN));
}


int begin(void) 
{
  /* Make sure we're actually connected */
  uint8_t x = read8(TCS34725_ID);
  if ((x != 0x44) && (x != 0x10)){
    return false;
  }
  _tcs34725Initialised = true;

  /* Set default integration time and gain */
  setIntegrationTime(TCS34725_INTEGRATIONTIME_700MS);
  setGain(TCS34725_GAIN_16X);

  /* Note: by default, the device is in power down mode on bootup */
  enable();

  return true;
}

void setIntegrationTime(tcs34725IntegrationTime_t it)
{
  if (!_tcs34725Initialised) begin();

  /* Update the timing register */
  write8(TCS34725_ATIME, it);

  /* Update value placeholders */
  _tcs34725IntegrationTime = it;
}

void setGain(tcs34725Gain_t gain)
{
  if (!_tcs34725Initialised) begin();

  /* Update the timing register */
  write8(TCS34725_CONTROL, gain);

  /* Update value placeholders */
  _tcs34725Gain = gain;
}

void getRawData (uint16_t *r, uint16_t *g, uint16_t *b, uint16_t *c)
{
  if (!_tcs34725Initialised) begin();

  *c = (read8(TCS34725_CDATAH) << 8) | read8(TCS34725_CDATAL);
	*r = (read8(TCS34725_RDATAH) << 8) | read8(TCS34725_RDATAL);
	*g = (read8(TCS34725_GDATAH) << 8) | read8(TCS34725_GDATAL);
	*b = (read8(TCS34725_BDATAH) << 8) | read8(TCS34725_BDATAL);
  
  /* Set a delay for the integration time */
  switch (_tcs34725IntegrationTime)
  {
    case TCS34725_INTEGRATIONTIME_2_4MS:
      HAL_Delay(3);
      break;
    case TCS34725_INTEGRATIONTIME_24MS:
      HAL_Delay(24);
      break;
    case TCS34725_INTEGRATIONTIME_50MS:
      HAL_Delay(50);
      break;
    case TCS34725_INTEGRATIONTIME_101MS:
      HAL_Delay(101);
      break;
    case TCS34725_INTEGRATIONTIME_154MS:
      HAL_Delay(154);
      break;
    case TCS34725_INTEGRATIONTIME_700MS:
      HAL_Delay(700);
      break;
  }
}

uint16_t calculateColorTemperature(uint16_t r, uint16_t g, uint16_t b)
{
  float X, Y, Z;      /* RGB to XYZ correlation      */
  float xc, yc;       /* Chromaticity co-ordinates   */
  float n;            /* McCamy's formula            */
  float cct;

  /* 1. Map RGB values to their XYZ counterparts.    */
  /* Based on 6500K fluorescent, 3000K fluorescent   */
  /* and 60W incandescent values for a wide range.   */
  /* Note: Y = Illuminance or lux                    */
  X = (-0.14282F * r) + (1.54924F * g) + (-0.95641F * b);
  Y = (-0.32466F * r) + (1.57837F * g) + (-0.73191F * b);
  Z = (-0.68202F * r) + (0.77073F * g) + ( 0.56332F * b);

  /* 2. Calculate the chromaticity co-ordinates      */
  xc = (X) / (X + Y + Z);
  yc = (Y) / (X + Y + Z);

  /* 3. Use McCamy's formula to determine the CCT    */
  n = (xc - 0.3320F) / (0.1858F - yc);

  /* Calculate the final CCT */
  cct = (449.0F * powf(n, 3)) + (3525.0F * powf(n, 2)) + (6823.3F * n) + 5520.33F;

  /* Return the results in degrees Kelvin */
  return (uint16_t)cct;
}



/**************************************************************************/
/*!
    @brief  Converts the raw R/G/B values to lux
*/
/**************************************************************************/
uint16_t calculateLux(uint16_t r, uint16_t g, uint16_t b)
{
  float illuminance;

  /* This only uses RGB ... how can we integrate clear or calculate lux */
  /* based exclusively on clear since this might be more reliable?      */
  illuminance = (-0.32466F * r) + (1.57837F * g) + (-0.73191F * b);

  return (uint16_t)illuminance;
}



void setInterrupt(int i) {
  uint8_t r = read8(TCS34725_ENABLE);
  if (i) {
    r |= TCS34725_ENABLE_AIEN;
  } else {
    r &= ~TCS34725_ENABLE_AIEN;
  }
  write8(TCS34725_ENABLE, r);
}

void clearInterrupt(void) {
  
	uint8_t pkt[2];
	
  pkt[0] = (TCS34725_COMMAND_BIT | 0x66);
 
	/////////////////
	I2Cdrv->MasterTransmit(TCS34725_ADDRESS, pkt, 2, false);
	
	while (I2Cdrv->GetStatus().busy){ 																	//Espera hasta bus libre
		}
	
}

void setIntLimits(uint16_t low, uint16_t high) {
   write8(0x04, low & 0xFF);
   write8(0x05, low >> 8);
   write8(0x06, high & 0xFF);
   write8(0x07, high >> 8);
}

osMessageQueueId_t get_id_TCS_MsgQueue_tx(void){
	return TCS_MsgQueue_tx;
}


