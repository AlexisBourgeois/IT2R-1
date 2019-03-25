#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include <LPC17xx.h>
#include "stdio.h"
#include "GPIO.h"

#define STOP 0x25
#define RESET 0x40
#define SCAN 0x20


extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;

extern ARM_DRIVER_USART Driver_USART0;

void init_PWM6(void)
{
	int rapport_cyclique = 0.5 * 999; // varie de 1 à 100
	
	// initialisation de timer 1
	LPC_SC->PCONP = LPC_SC->PCONP | (3<<1);   // enable PWM1 et PWM 0	pour plusieurs PWM
	
	// prescaler+1 = 12 cela donne une horloge de base de période 480 ns
	// TC s'incrémente toutes les 480 ns
	LPC_PWM1->PR = 0;  // prescaler
	
	// valeur de MR0  + 1 = 100 cela fait 48 us, période de la PWM
  // valeur proche des 20 KHz de la gamelle !
  // ceci permet de régler facilement le rapport cyclique entre 1 et 100	

  LPC_PWM1->MR0 = 999;    // Ceci ajuste la période de la PWM à 40ms
	LPC_PWM1->MR6 = rapport_cyclique-1;    // ceci ajuste la duree de l'état haut PWM 6
	
	LPC_PINCON->PINSEL4  |=  (1 << 10); //  P2.5 est la sortie PWM Channel 6 de Timer 1
	
	LPC_PWM1->MCR = LPC_PWM1->MCR | 0x00000002; // Timer relancé quand MR0 repasse à 0
	LPC_PWM1->LER = LPC_PWM1->LER | 0x00000009;  // ceci donne le droit de modifier dynamiquement la valeur du rapport cyclique
	                                             // bit 0 = MR0    bit3 = MR3
	LPC_PWM1->PCR = LPC_PWM1->PCR | 0x00007e00;  // autorise la sortie PWM [1 à 6] Ne pas oublier
	                                
  LPC_PWM1->TCR = 1;  /*arret du pwm */
}
void Init_UART(void){
	Driver_USART0.Initialize(NULL);
	Driver_USART0.PowerControl(ARM_POWER_FULL);
	Driver_USART0.Control(	ARM_USART_MODE_ASYNCHRONOUS |
							ARM_USART_DATA_BITS_8		|
							ARM_USART_STOP_BITS_1		|
							ARM_USART_PARITY_NONE		|
							ARM_USART_FLOW_CONTROL_NONE,
							115200);
	Driver_USART0.Control(ARM_USART_CONTROL_TX,1);
	Driver_USART0.Control(ARM_USART_CONTROL_RX,1);
}

int main (void){
	uint8_t tabRx[7],tabTx[2], donnee[20];
	char d[8];
	int i,rx=0;
	
	tabTx[0] = 0xA5;
	
	GLCD_Initialize();
	init_PWM6();
	Init_UART();
	Initialise_GPIO(); 
	

	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	GLCD_SetForegroundColor  (GLCD_COLOR_BLACK);
	GLCD_SetBackgroundColor  (GLCD_COLOR_WHITE);
	
	GLCD_DrawString(0,50,"INIT!");
	
	tabTx[1] = STOP;
	
	while(Driver_USART0.GetStatus().tx_busy == 1); // attente buffer TX vide
	Driver_USART0.Send(tabTx,2);
	
	while(Valeur_BP() != 1);
		
	tabTx[1] = SCAN;
	
	while(Driver_USART0.GetStatus().tx_busy == 1); // attente buffer TX vide
	Driver_USART0.Send(tabTx,2);
	
	GLCD_DrawString(0,50,"SCAN!");
	
	//do{
	Driver_USART0.Receive(tabRx,7);
	while(Driver_USART0.GetRxCount()<1);
	GLCD_DrawString(0,50,"REPONSE!");
	while(tabRx[0] != 0xA5 || tabRx[1] != 0x5A || tabRx[2] != 0x05 || tabRx[3] != 0x00 || tabRx[4] != 0x00 || tabRx[5] != 0x40 || tabRx[6] != 0x81);
		
	GLCD_DrawString(0,50,"Fait!");
	
	while (1){
		i=0;
		if(Valeur_Joystick_haut() == 1){
			tabTx[1] = STOP;
			
			while(Driver_USART0.GetStatus().tx_busy == 1); // attente buffer TX vide
			Driver_USART0.Send(tabTx,2);
			GLCD_DrawString(0,50,"Reset!");
		} 
		
		if(Valeur_Joystick_bas() == 1){
			tabTx[1] = SCAN;
			
			while(Driver_USART0.GetStatus().tx_busy == 1); // attente buffer TX vide
			Driver_USART0.Send(tabTx,2);
			
			do{
			Driver_USART0.Receive(tabRx,7);
			while(Driver_USART0.GetRxCount()<1);
			i++;
			}while((tabRx[0] != 0xA5 || tabRx[1] != 0x5A || tabRx[2] != 0x05 || tabRx[3] != 0x00 || tabRx[4] != 0x00 || tabRx[5] != 0x40 || tabRx[6] != 0x81) && i <= 2000*16667);
				
			GLCD_DrawString(0,50,"Start!");
		}
	}	
	return 0;
}