/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions
#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include <LPC17xx.h>
#include "stdio.h"
#include "GPIO.h"
#include "Driver.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"

#define ENTETE 0xA5											//Definition du code des messages à envoyer au LIDAR
#define STOP 0x25
#define RESET 0x40
#define SCAN 0x20
#define GET_INFO 0x50
#define GET_HEALTH 0x52
#define RECUE 1
#define NB_TOUR 2 
#define CENTRE_X 159
#define CENTRE_Y 119
#define CM_TO_PIXEL 2
#define PI 3.14159

extern ARM_DRIVER_USART Driver_USART0;
extern ARM_DRIVER_USART Driver_USART1;
extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;

char reponseLidarScan[6] = {0x5A, 0x05, 0x00, 0x00, 0x40, 0x81};		//Definition de la réponse du LIDAR pour le Scan
char reponseLidarHealth[7] = {0xA5, 0x5A, 0x03, 0x00, 0x00, 0x00, 0x06};		//Definition de la réponse du LIDAR pour le GET_Health

char etatReponse = 0;			
char tabDonnee[2000*NB_TOUR];
float env[360];

extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;

void LectureLidar(void const * argument);
void UARTReceive(void const * argument);
void myUART_callback(uint32_t event); //ARM_USART_SignalEvent_t
void sendLidar(char data);
void affichage_cartesien(void);

osThreadId ID_UARTReceive;
osThreadId ID_Lecture;

osThreadDef (UARTReceive, osPriorityNormal, 1, 0);
osThreadDef (LectureLidar, osPriorityNormal, 1, 0);

void Init_UART(void){
	Driver_USART0.Initialize(myUART_callback);
	Driver_USART0.PowerControl(ARM_POWER_FULL);
	Driver_USART0.Control(	ARM_USART_MODE_ASYNCHRONOUS |
							ARM_USART_DATA_BITS_8		|
							ARM_USART_STOP_BITS_1		|
							ARM_USART_PARITY_NONE		|
							ARM_USART_FLOW_CONTROL_NONE,
							115200);
	Driver_USART0.Control(ARM_USART_CONTROL_TX,1);
	Driver_USART0.Control(ARM_USART_CONTROL_RX,1);
	
	Driver_USART1.Initialize(NULL);
	Driver_USART1.PowerControl(ARM_POWER_FULL);
	Driver_USART1.Control( ARM_USART_MODE_ASYNCHRONOUS |
							ARM_USART_DATA_BITS_8 |
							ARM_USART_STOP_BITS_1 |
							ARM_USART_PARITY_NONE |
							ARM_USART_FLOW_CONTROL_NONE,
							115200);
	Driver_USART1.Control(ARM_USART_CONTROL_TX,1);
	Driver_USART1.Control(ARM_USART_CONTROL_RX,1);
}

/*
 * main: initialize and start the system
 */
 
int main (void) {
	
  osKernelInitialize ();                    // initialize CMSIS-RTOS
		
	init_PWM6();
	Init_UART(); 
	Initialise_GPIO();  
	
	GLCD_Initialize();
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	GLCD_SetForegroundColor(GLCD_COLOR_BLACK);
  // initialize peripherals here
	
  // create 'thread' functions that start executing,
  // example: tid_name = osThreadCreate (osThread(name), NULL);
	
	ID_UARTReceive = osThreadCreate(osThread(UARTReceive), NULL);
	ID_Lecture     = osThreadCreate(osThread(LectureLidar), NULL);
	
  osKernelStart ();                         			// start thread execution 
	
	Driver_USART1.Send("\n\r---------------Debut-----------------\n\r",100);
	while(Driver_USART1.GetStatus().tx_busy == 1); 	// attente buffer TX vide
	

	
	osSignalSet(ID_UARTReceive, 0x01);
	
	osDelay(osWaitForever);
}

void LectureLidar(void const * argument){
	char tabPC[100];
	int i;
	GLCD_SetForegroundColor(GLCD_COLOR_BLACK);
	while(1){		
		osSignalWait(0x01, osWaitForever);
		
		Driver_USART0.Receive(tabDonnee, NB_TOUR*2000);
		osSignalWait(0x02,osWaitForever);
		
		sendLidar(STOP);
		osDelay(2);
		etatReponse = 0;
		
		//Envoi des données récupérées au PC
		for(i=0;i<400*NB_TOUR;i++){
			env[((tabDonnee[5*i+1]>>1)|(tabDonnee[5*i+2]<<7))>>6] = (tabDonnee[5*i+3]|(tabDonnee[5*i+4]<<8))/40.0;
		}
		for(i=0;i<360;i++){
			sprintf(tabPC,"Angle:%3d° , Distance:%6.1f cm\r\n",i,env[i]);
			Driver_USART1.Send(tabPC,50);
			while(Driver_USART1.GetStatus().tx_busy == 1);
		}
					

			affichage_cartesien();
			osSignalSet(ID_UARTReceive, 0x01);
	}
}

void UARTReceive(void const * argument){//fonction pour lecture
	char reponse[6],count=0,trash;
	osEvent result;
	while(1){
		osSignalWait(0x01, osWaitForever);
		do{
			sendLidar(GET_HEALTH);
			Driver_USART0.Receive(reponse, 7);
			result = osSignalWait(0x02, 500);

			if(result.status == osEventTimeout){
				Driver_USART1.Send("Communication timeout\n\r",23);
				while(Driver_USART1.GetStatus().tx_busy == 1); // attente buffer TX vide
			} 
			
			Driver_USART0.Receive(reponse, 3);
			result = osSignalWait(0x02, osWaitForever);
			
			count++;
			if(count > 10){
				Driver_USART1.Send("RPLIDAR hardware failure\n\r",26);
				while(Driver_USART1.GetStatus().tx_busy == 1); // attente buffer TX vide
				break;
			}
		} while(reponse[0] == 2);
		
		count = 0;
		osDelay(10);
		
		do{
			sendLidar(SCAN);
			do{
				Driver_USART0.Receive(&trash, 1);
				result = osSignalWait(0x02, 50);
			}while(trash != (char)0xA5);
		
			Driver_USART0.Receive(reponse,6);
			osSignalWait(0x02, 50);
			
			if(result.status == osEventTimeout){
				sendLidar(STOP);
				osDelay(2);
			} 
			
		}while(strcmp(reponse,reponseLidarScan) != 0);
		
		etatReponse = RECUE;
		osSignalSet(ID_Lecture, 0x01);
	}
}

void myUART_callback(uint32_t event){
	switch(event){
		case ARM_USART_EVENT_RECEIVE_COMPLETE: 
		
			if(etatReponse == 0){
			osSignalSet(ID_UARTReceive, 0x02);
			}
			else osSignalSet(ID_Lecture, 0x02);
			
		break;

		
		case ARM_USART_EVENT_SEND_COMPLETE:
			break;
		
		default: 
	  	break;
	}	
}

/**************************
	Fonction d'envoi d'instructions au lidar

	Valeur des instructions :

	ENTETE 0xA5											
	STOP 0x25
	RESET 0x40
	SCAN 0x20
	GET_INFO 0x50
	GET_HEALTH 0x52
**************************/

void sendLidar(char data){
	char tab[2];
	tab[0] = ENTETE;
	tab[1] = data;
	Driver_USART0.Send(tab,2);
	while(Driver_USART0.GetStatus().tx_busy == 1); // attente buffer TX vide
}

void affichage_cartesien(void){
	short x,y,i;
	
	GLCD_ClearScreen();
	GLCD_SetForegroundColor(GLCD_COLOR_BLACK);
	GLCD_DrawRectangle(CENTRE_X-4,CENTRE_Y-1,8,2);					//Dessin de la voiture
	
	for(i=0;i<360;i++){
		x=env[i]*cos(2*PI*i/360)/CM_TO_PIXEL;									//Conversion des coordonnées polaires en cartésiennes
		y=env[i]*sin(2*PI*i/360)/CM_TO_PIXEL;
		GLCD_DrawPixel(CENTRE_X+x,CENTRE_Y+y);								//Dessin à partir du centre de l'écran
	}

}
