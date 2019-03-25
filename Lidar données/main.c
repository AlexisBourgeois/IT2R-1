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

#define ENTETE 0xA5											//Definition du code des messages à envoyer au LIDAR
#define STOP 0x25
#define RESET 0x40
#define SCAN 0x20
#define RECUE 1

extern ARM_DRIVER_USART Driver_USART0;
extern ARM_DRIVER_USART Driver_USART1;

char reponseLidarScan[7] = {0xA5, 0x5A, 0x05, 0x00, 0x00, 0x40, 0x81};		//Definition de la réponse du LIDAR
char etatReponse = 0;			
char tabDonnee[2000];
short env[360];

extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;

void LectureLidar(void const * argument);
void UARTReceive(void const * argument);
//void UARTSend(void const * argument);
void myUART_callback(uint32_t event); //ARM_USART_SignalEvent_t
void stopLidar(void);

osThreadId ID_UARTReceive;
//osThreadId ID_UARTSend;

osThreadId ID_Lecture;

osThreadDef (UARTReceive, osPriorityNormal, 1, 0);
//osThreadDef (UARTSend, osPriorityAboveNormal, 1, 0);
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
	
	
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
  // initialize peripherals here

  // create 'thread' functions that start executing,
  // example: tid_name = osThreadCreate (osThread(name), NULL);
	
	ID_UARTReceive = osThreadCreate(osThread(UARTReceive), NULL);
	//ID_UARTSend    = osThreadCreate(osThread(UARTSend), NULL);
	ID_Lecture     = osThreadCreate(osThread(LectureLidar), NULL);
	
	Driver_USART1.Send("\n\r---------------Debut-----------------\n\r",100);
	while(Driver_USART1.GetStatus().tx_busy == 1); 	// attente buffer TX vide
	
  osKernelStart ();                         			// start thread execution 
	
	
	
	
	osSignalSet(ID_UARTReceive, 0x01);
	
	osDelay(osWaitForever);
}

void LectureLidar(void const * argument){
	char tabPC[100];

	//char erreur;
	int i;
	
	osSignalWait(0x01, osWaitForever);
	
	while(1){		

		Driver_USART0.Receive(tabDonnee, 2000);
		osSignalWait(0x02,osWaitForever);
		while(Driver_USART0.GetRxCount()<2000);
		
		stopLidar();
		etatReponse = 0;
		
		//Envoi des données récupérées au PC
		for(i=0;i<400;i++){
			/*sprintf(tabPC,"1: 0x%0.2x\n\r2: 0x%0.2x\n\r3: 0x%0.2x\n\r4: 0x%0.2x\n\r5: 0x%0.2x \r\n",tabDonnee[5*i],tabDonnee[5*i+1],tabDonnee[5*i+2],tabDonnee[5*i+3],tabDonnee[5*i+4]);
			Driver_USART1.Send(tabPC,100);
			while(Driver_USART1.GetStatus().tx_busy == 1); // attente buffer TX vide*/
			
			env[((tabDonnee[5*i+1]>>1)|(tabDonnee[5*i+2]<<7))>>6] = (tabDonnee[5*i+3]|(tabDonnee[5*i+4]<<8))/40;
		}
		for(i=0;i<360;i++){
			sprintf(tabPC,"Angle:%3d° , Distance:%5d cm\r\n",i,env[i]);
			Driver_USART1.Send(tabPC,100);
			while(Driver_USART1.GetStatus().tx_busy == 1);
		}
		
		//Vérification des données
		/*erreur = 0;
		for(i=0;i<400;i++){
			if((tabDonnee[5*i+1]&0x01) == 0x00) erreur = 1;
			if(((tabDonnee[5*i]&0x03) == 0x00)||(tabDonnee[5*i]&0x03) == 0x03) erreur = 1;
		}
			if(erreur == 1)
			{
				sprintf(tabPC,"Problème!\n\r");
				Driver_USART1.Send(tabPC,100);
				while(Driver_USART1.GetStatus().tx_busy == 1); // attente buffer TX vide
			}
			else
			{
				sprintf(tabPC,"RAS !\n\r\0");
				Driver_USART1.Send(tabPC,8);
				while(Driver_USART1.GetStatus().tx_busy == 1); // attente buffer TX vide
			}*/
			
		//Traitement des données récupérées par le Lidar
			
			
	}
}

void UARTReceive(void const * argument){//fonction pour lecture
	char reponse[7],tabTx[2],carac1=0,i;
	tabTx[0] = ENTETE;
	while(1){
		osSignalWait(0x01, osWaitForever);
		do{
			stopLidar();
			
			Driver_USART1.Send("STOP\n\r", 10);
			while(Driver_USART1.GetStatus().tx_busy == 1);
			
			osDelay(20);
			
			Driver_USART1.Send("SCAN\r\n", 10);
			while(Driver_USART1.GetStatus().tx_busy == 1);
			
			tabTx[1] = SCAN;
			Driver_USART0.Send(tabTx,2);
			while(Driver_USART0.GetStatus().tx_busy == 1); // attente buffer TX vide
			
			/*
			Driver_USART1.Send("\n\rDEBUT\n\r",10);
			while(Driver_USART1.GetStatus().tx_busy == 1); // attente buffer TX vide
			*/			
			
			Driver_USART0.Receive(&carac1, 1);
			while(Driver_USART0.GetRxCount()<1);
			
			if(carac1 == (char) 0xA5){
			Driver_USART0.Receive(reponse, 6);
			while(Driver_USART0.GetRxCount()<6);
			
			for(i=7;i>0;i--){
					reponse[i] = reponse[i-1];
			}
			reponse[0] = carac1;
			}
			
			osSignalWait(0x02, osWaitForever);

		} while(strcmp(reponse,reponseLidarScan) != 0);
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

void stopLidar(void){
	char tab[2];
	tab[0] = 0xA5;
	tab[1] = 0x25;
	Driver_USART1.Send("STOP du Lidar",15);
	while(Driver_USART1.GetStatus().tx_busy == 1); // attente buffer TX vide
	Driver_USART0.Send(tab,2);
	while(Driver_USART0.GetStatus().tx_busy == 1); // attente buffer TX vide
}
