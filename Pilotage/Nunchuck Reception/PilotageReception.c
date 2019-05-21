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
	
void TIMER0_IRQHandler(void)
{
	LPC_TIM0->IR = LPC_TIM0->IR | (1<<0); //baisse le drapeau dû à MR0

	if(!ETAT) // Si l'ancien état était haut ou bas
		{
			LPC_TIM0->MR0 = MR1;  // On change la durée du timer qui correspondra à l'état haut
			ETAT=1; // On mémorise l'état actuel
		}
		else
		{
			LPC_TIM0->MR0 = (2499-MR1); // On change la durée du timer qui correspondra à l'état bas
			ETAT=0; // On mémorise l'état actuel
		}
			
}

void Initialisation_Pilotage(void){

  // initialize peripherals here
	GLCD_Initialize();
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	
	Initialise_GPIO (); // init GPIO
	// initialisation de timer 1
	LPC_SC->PCONP = LPC_SC->PCONP | 0x00000040;   // enable PWM1
	// prescaler+1 = 12 cela donne une horloge de base de période 480 ns
	// TC s'incrémente toutes les 480 ns
	LPC_PWM1->PR = 0;  // prescaler
	
	// valeur de MR0  + 1 = 100 cela fait 48 us, période de la PWM
  // valeur proche des 20 KHz de la gamelle !
  // ceci permet de régler facilement le rapport cyclique entre 1 et 100	

  LPC_PWM1->MR0 = 499999;    // Ceci ajuste la période de la PWM à 48 us
	LPC_PWM1->MR3 = 0.5*(499999+1)-1;    // ceci ajuste la duree de l'état haut
	
	LPC_PINCON->PINSEL7 = LPC_PINCON->PINSEL7 | 0x00300000; //  P3.26 est la sortie PWM Channel 3 de Timer 1
	
	LPC_PWM1->MCR = LPC_PWM1->MCR | 0x00000002; // Timer relancé quand MR0 repasse à 0
	LPC_PWM1->LER = LPC_PWM1->LER | 0x00000009;  // ceci donne le droit de modifier dynamiquement la valeur du rapport cyclique
	                                             // bit 0 = MR0    bit3 = MR3
	LPC_PWM1->PCR = LPC_PWM1->PCR | 0x00000e00;  // autorise la sortie PWM
	                                
  LPC_PWM1->TCR = 1;  /*validation de timer 1 et reset counter */
	LPC_GPIO2->FIODIR = LPC_GPIO2->FIODIR | (1<<0); //P2.0 configuré en sortie
	//
	LPC_SC->PCONP = LPC_SC->PCONP | (1<<1); //active timer0
	
	LPC_PINCON->PINSEL7 = LPC_PINCON->PINSEL7 | (2<<18);
	
	LPC_TIM0->CTCR = 0; //mode Timer (signaux carrés)
	LPC_TIM0->PR = 0; //Prescaler à 0
	LPC_TIM0->MR0 = 2499; //valeur de N : 20kHz
//	LPC_TIM0->MCR = LPC_TIM0->MCR | (1<<1); // RAZ du compteur si correspondance avec MR0
//	LPC_TIM0->EMR = LPC_TIM0->EMR | (3<<4); // inverse la sortie MAT2.0 à chaque évènement
	LPC_TIM0->MCR = LPC_TIM0->MCR | (3<<0); 
	LPC_TIM0->EMR = LPC_TIM0->EMR | (3<<4);
			//RAZ du compteur si correspondance avec MR0, et interruption
	NVIC_SetPriority(1,0); //TIMER0 : interruption de priorité 0
	NVIC_EnableIRQ(1); //active les interruption TIMER0
	LPC_TIM0->TCR = 1;
	//
}

char Position_Roues(double direction){
	if ((direction<=1)&&(direction>=0)){
		LPC_PWM1->MR3 = (0.06*(499999+1)-1) + direction*(0.025*(499999+1)-1); 
		//LPC_PWM1->MR3 = (0.062*(499999+1)-1) + direction*(0.023*(499999+1)-1);
		return 0;
	}
	else{
		return 1;
	}
}

char Avancer(double Puissance){

		MR1= Puissance * 500 + 100; //Conversion de la variable puissance
		//LPC_PWM1->MR3 = (0.062*(499999+1)-1) + direction*(0.023*(499999+1)-1);
		return 0;

}
