/*---------------------------------------------------
* CAN 2 uniquement en TX 
* + réception CAN1 
* avec RTOS et utilisation des fonction CB
* pour test sur 1 carte -> relier CAN1 et CAN2
* 2017-04-02 - XM
---------------------------------------------------*/

#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions

#include "LPC17xx.h"                    // Device header
#include "Driver_CAN.h"                 // ::CMSIS Driver:CAN
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include "stdio.h"
#include "cmsis_os.h"
#include "Driver_I2C.h" 
#include "GPIO.h"

#include "PilotageEnvoie.h"
#include "Nunchuk.h"

extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;
extern ARM_DRIVER_I2C Driver_I2C0;



int main (void) {
	

		char PositionX, PositionY;
	  char charX[30], charY[30], Ini[30], CtrlX[30], CtrlY[30];
		double PosCtrlX, PosCtrlY;

  // initialize peripherals here
		GLCD_Initialize();
		GLCD_ClearScreen();
		GLCD_SetFont(&GLCD_Font_16x24);
	  Init_I2C();
		Init_UART();
		///sprintf(Ini, "Initialisation OK");
		//GLCD_DrawString(1,80,(unsigned char*)Ini);
	
	while(1){
		PositionX = JoystickX();
		PositionY = JoystickY();
		
		sprintf(charX, "PositionX: 0x%x ", PositionX);
		GLCD_DrawString(1,0,(unsigned char*)charX);
		
		sprintf(charY, "PositionY: 0x%x ", PositionY);
		GLCD_DrawString(1,40,(unsigned char*)charY);
		SendUARTCommande(PositionX, PositionY);
		
		
	}


	return 0;

  
}

/*
osDelay(5000);
		LPC_PWM1->MR3 = 0.062*(499999+1)-1;  // droite
		osDelay(5000);
		LPC_PWM1->MR3 = 0.0725*(499999+1)-1; // centre
		osDelay(5000);
		LPC_PWM1->MR3 = 0.083*(499999+1)-1;  // gauche
		osDelay(5000);
		LPC_PWM1->MR3 = 0.0725*(499999+1)-1;
*/

