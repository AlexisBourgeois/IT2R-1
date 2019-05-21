#include "LPC17xx.h"
#include "Driver_I2C.h"                 // ::CMSIS Driver:I2C
#include "Board_GLCD.h"
#include "GLCD_Config.h"

#include "stdio.h"

#define SLAVE_I2C_ADDR       0x52			// Adresse esclave sur 7 bits

extern ARM_DRIVER_I2C Driver_I2C0;

uint8_t DeviceAddr;

void Init_I2C(void);
void write1byte(unsigned char composant, unsigned char registre, unsigned char valeur);
unsigned char read1byte(unsigned char composant, unsigned char registre);
void zero_data_request(void);
unsigned char JoystickX(void);
unsigned char JoystickY(void);
	
void Init_I2C(void){

	Driver_I2C0.Initialize(NULL);
	Driver_I2C0.PowerControl(ARM_POWER_FULL); 
	Driver_I2C0.Control(	ARM_I2C_BUS_SPEED,				// 2nd argument = d?bit
							ARM_I2C_BUS_SPEED_STANDARD  );	// 100 kHz
	Driver_I2C0.Control(	ARM_I2C_BUS_CLEAR,
							0 );
	NVIC_SetPriority(I2C0_IRQn,0);
	NVIC_SetPriority(SSP1_IRQn,1);
	
	write1byte(SLAVE_I2C_ADDR, 0xF0, 0x55);
	write1byte(SLAVE_I2C_ADDR, 0xFB, 0x00);
	//write1byte(SLAVE_I2C_ADDR, 0x40, 0x00);
}

void write1byte(unsigned char composant, unsigned char registre, unsigned char valeur){
	unsigned char tab[2];
	tab[0] = registre;
	tab[1] = valeur;
	
	Driver_I2C0.MasterTransmit(composant, tab, 2, false); // Ecriture vers registre esclave : START + ADDR(W) + 1W_SUB + 1W_DATA + STOP
	while(Driver_I2C0.GetStatus().busy == 1); // Attente fin transmission
}

unsigned char read1byte(unsigned char composant, unsigned char registre){
	unsigned char tab[1];
	char read;
	tab[0] = registre;
	
	Driver_I2C0.MasterTransmit(composant, tab, 1, true); // Ecriture vers registre esclave : START + ADDR(W) + 1W_SUB
	while(Driver_I2C0.GetStatus().busy == 1); // Attente fin transmission
	
	Driver_I2C0.MasterReceive(composant, &read, 1, false); // Lecture de data esclave : START + ADDR(R) + 1R_DATA + STOP
	while(Driver_I2C0.GetStatus().busy == 1); // Attente fin transmission
	
	return read;
}



//unsigned char JoystickX(void){
//	 char read[6];
//	 char byte1;
//	read[0] = 0xDC;
//	zero_data_request();
//	 
//	Driver_I2C0.MasterReceive(SLAVE_I2C_ADDR, read, 1, false); // Lecture de data esclave : START + ADDR(R) + 1R_DATA + STOP
//	while(Driver_I2C0.GetStatus().busy == 1); // Attente fin transmission
//	byte1=read[0];
//	
//	return byte1;

//}

//unsigned char JoystickY(void){
//	 char read[6];
//	 char byte2;
//	read[1] = 0xCD;
//	zero_data_request();
//	 
//	Driver_I2C0.MasterReceive(SLAVE_I2C_ADDR, read, 1, false); // Lecture de data esclave : START + ADDR(R) + 1R_DATA + STOP
//	while(Driver_I2C0.GetStatus().busy == 1); // Attente fin transmission
//	byte2=read[1];
//	
//	return byte2;

//}


void zero_data_request(void){
	unsigned char tab[1];
	tab[0] = 0x00;
	Driver_I2C0.MasterTransmit(SLAVE_I2C_ADDR, tab, 1, false); // Ecriture vers registre esclave : START + ADDR(W) + 1W_SUB
	while(Driver_I2C0.GetStatus().busy == 1); // Attente fin transmission
}

unsigned char JoystickX(void){
	 char read[6];
	unsigned char tab;
	 char byte1;
	//read[0] = 0xDC;
	tab = 0x00;
	
	Driver_I2C0.MasterTransmit(SLAVE_I2C_ADDR, &tab, 1, false); // Ecriture vers registre esclave : START + ADDR(W) + 1W_SUB
	while(Driver_I2C0.GetStatus().busy == 1); // Attente fin transmission
	 
	Driver_I2C0.MasterReceive(SLAVE_I2C_ADDR, read, 6, false); // Lecture de data esclave : START + ADDR(R) + 1R_DATA + STOP
	while(Driver_I2C0.GetStatus().busy == 1); // Attente fin transmission
	
	
	byte1= 0xff-read[0];
	
	return byte1;

}

unsigned char JoystickY(void){
	 char read[6];
	unsigned char tab;
	 char byte2;
	tab = 0x00;
	//read[1] = 0xCD;
	
	Driver_I2C0.MasterTransmit(SLAVE_I2C_ADDR, &tab, 1, false); // Ecriture vers registre esclave : START + ADDR(W) + 1W_SUB
	while(Driver_I2C0.GetStatus().busy == 1); // Attente fin transmission
	 
	Driver_I2C0.MasterReceive(SLAVE_I2C_ADDR, read, 6, false); // Lecture de data esclave : START + ADDR(R) + 1R_DATA + STOP
	while(Driver_I2C0.GetStatus().busy == 1); // Attente fin transmission
	byte2=read[1];
	
	return byte2;

}
