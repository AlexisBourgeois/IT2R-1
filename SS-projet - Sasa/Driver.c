#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include <LPC17xx.h>
#include "stdio.h"
#include "GPIO.h"
#include "osObjects.h"                      // RTOS object definitions



void init_PWM6(void)
{
	int rapport_cyclique = 0.5 * 999; // varie de 1 � 100
	
	// initialisation de timer 1
	LPC_SC->PCONP = LPC_SC->PCONP | (3<<1);   // enable PWM1 et PWM 0	pour plusieurs PWM
	
	// prescaler+1 = 12 cela donne une horloge de base de p�riode 480 ns
	// TC s'incr�mente toutes les 480 ns
	LPC_PWM1->PR = 0;  // prescaler
	
	// valeur de MR0  + 1 = 100 cela fait 48 us, p�riode de la PWM
  // valeur proche des 20 KHz de la gamelle !
  // ceci permet de r�gler facilement le rapport cyclique entre 1 et 100	

  LPC_PWM1->MR0 = 999;    // Ceci ajuste la p�riode de la PWM � 40ms
	LPC_PWM1->MR6 = rapport_cyclique-1;    // ceci ajuste la duree de l'�tat haut PWM 6
	
	LPC_PINCON->PINSEL4  |=  (1 << 10); //  P2.5 est la sortie PWM Channel 6 de Timer 1
	
	LPC_PWM1->MCR = LPC_PWM1->MCR | 0x00000002; // Timer relanc� quand MR0 repasse � 0
	LPC_PWM1->LER = LPC_PWM1->LER | 0x00000009;  // ceci donne le droit de modifier dynamiquement la valeur du rapport cyclique
	                                             // bit 0 = MR0    bit3 = MR3
	LPC_PWM1->PCR = LPC_PWM1->PCR | 0x00007e00;  // autorise la sortie PWM [1 � 6] Ne pas oublier
	                                
  LPC_PWM1->TCR = 1;  /* 0 pour arret du pwm */
}
