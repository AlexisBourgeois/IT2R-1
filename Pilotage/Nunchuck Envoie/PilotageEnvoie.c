#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions

#include "LPC17xx.h"                    // Device header
#include "Driver_CAN.h"                 // ::CMSIS Driver:CAN
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include "stdio.h"
#include "cmsis_os.h"
#include "GPIO.h"
#include "Driver_USART.h"               // ::CMSIS Driver:USART

extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;
extern ARM_DRIVER_USART Driver_USART1;

double MR1=1250, ETAT=0;
int cpt=0;

void Init_UART(void){
	Driver_USART1.Initialize(NULL);
	Driver_USART1.PowerControl(ARM_POWER_FULL);
	Driver_USART1.Control(	ARM_USART_MODE_ASYNCHRONOUS |
							ARM_USART_DATA_BITS_8		|
							ARM_USART_STOP_BITS_1		|
							ARM_USART_PARITY_NONE		|
							ARM_USART_FLOW_CONTROL_NONE,
							115200);
	Driver_USART1.Control(ARM_USART_CONTROL_TX,1);
	Driver_USART1.Control(ARM_USART_CONTROL_RX,1);
}

void SendUARTCommande(char PositionX, char PositionY){
	char tab[2],aff[10];
	cpt++;
	tab[0] = PositionX; // rangement de la valeur du joystickX dans un tableau
	tab[1] = PositionY; // rangement de la valeur du joystick dans un tableau
	while(Driver_USART1.GetStatus().tx_busy == 1); // attente buffer TX vide
	Driver_USART1.Send(tab, 2);	//Envoie de positionX et position Y qui ont etait rangé dans le tabelau
	sprintf(aff, "Nb: %d", cpt);
	GLCD_DrawString(1,80,(unsigned char*)aff); 
}