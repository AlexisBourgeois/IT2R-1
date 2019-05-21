///*---------------------------------------------------
//* CAN 2 uniquement en TX 
//* + réception CAN1 
//* avec RTOS et utilisation des fonction CB
//* pour test sur 1 carte -> relier CAN1 et CAN2
//* 2017-04-02 - XM
//---------------------------------------------------*/

//#define osObjectsPublic                     // define objects in main module
//#include "osObjects.h"                      // RTOS object definitions

//#include "LPC17xx.h"                    // Device header
//#include "Driver_CAN.h"                 // ::CMSIS Driver:CAN
//#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
//#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
//#include "stdio.h"
//#include "cmsis_os.h"
//#include "Driver_I2C.h" 
//#include "GPIO.h"
//#include "Driver_USART.h"               // ::CMSIS Driver:USART
//#include "PilotageReception.h"
//#include "Nunchuk.h"

//extern GLCD_FONT GLCD_Font_6x8;
//extern GLCD_FONT GLCD_Font_16x24;
//extern ARM_DRIVER_I2C Driver_I2C0;
//extern ARM_DRIVER_USART Driver_USART1;

//osThreadId id_CANthreadR;
//osThreadId id_CANthread;


//int main (void) {
//	

//		char PositionX, PositionY;
//	  char charX[30], charY[30], Ini[30], CtrlX[30], CtrlY[30], tab[2], cpt=1;
//		double PosCtrlX, PosCtrlY;
//		Initialisation_Pilotage();
//	  Init_I2C();
//		Init_UART();
//		///sprintf(Ini, "Initialisation OK");
//		//GLCD_DrawString(1,80,(unsigned char*)Ini);
//	while(1){
//		GLCD_DrawString(1,160,"Je suis la");

//		Driver_USART1.Receive(tab,2);
//		while (Driver_USART1.GetRxCount() <2);
//		sprintf(Ini, "Je suis la %d", cpt++);
//		GLCD_DrawString(1,160,Ini);
//		PositionX = tab[0];
//		PositionY = tab[1];
//		PosCtrlX = (double) PositionX * 0.0037878787879;
//		PosCtrlY = (double) PositionY * 0.0038167938931;
//		
//		sprintf(charX, "PositionX: 0x%x ", PositionX);
//		GLCD_DrawString(1,0,(unsigned char*)charX);
//		
//		sprintf(charY, "PositionY: 0x%x ", PositionY);
//		GLCD_DrawString(1,40,(unsigned char*)charY);
//		
//		sprintf(CtrlX, "CtrlX: %lf ", PosCtrlX);
//		GLCD_DrawString(1,80,(unsigned char*)CtrlX);
//		
//		sprintf(CtrlY, "CtrlY: %lf ", PosCtrlY);
//		GLCD_DrawString(1,120,(unsigned char*)CtrlY);
//		Position_Roues(PosCtrlX);
//		Avancer(PosCtrlY);

//		
////		osDelay(5000);
////		Position_Roues(0);  // droite
////		osDelay(5000);
////		Position_Roues(0.5); // centre
////		osDelay(5000);
////		Position_Roues(1);  // gauche
////		osDelay(5000);
////		Position_Roues(0.5);
////			osDelay(5000);
//	}
//	osKernelStart ();                         // start thread execution 
//	osDelay(osWaitForever);

//	return 0;

//  
//}

///*
//osDelay(5000);
//		LPC_PWM1->MR3 = 0.062*(499999+1)-1;  // droite
//		osDelay(5000);
//		LPC_PWM1->MR3 = 0.0725*(499999+1)-1; // centre
//		osDelay(5000);
//		LPC_PWM1->MR3 = 0.083*(499999+1)-1;  // gauche
//		osDelay(5000);
//		LPC_PWM1->MR3 = 0.0725*(499999+1)-1;
//*/

