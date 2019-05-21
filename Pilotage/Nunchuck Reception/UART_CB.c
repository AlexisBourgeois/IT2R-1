// Utilisation Event UART en emission-reception

#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions

#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include "Board_ADC.h"                  // ::Board Support:A/D Converter
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include "LPC17xx.h"                    // Device header
#include "cmsis_os.h"                   // CMSIS RTOS header file
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "def.h"
#include "PilotageReception.h"

extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;

osMailQId maBAL;
osMailQDef(maBAL, 200, char);

//void Thread_T (void const *argument);                             // thread function Transmit
//osThreadId tid_Thread_T;                                          // thread id
//osThreadDef (Thread_T, osPriorityNormal, 1, 0);                   // thread object

void Thread_R (void const *argument);                             // thread function Receive
osThreadId tid_Thread_R;                                          // thread id
osThreadDef (Thread_R, osPriorityNormal, 1, 0);                   // thread object

void Thread_PWM(void const *argument);                             // thread function Display
osThreadId tid_Thread_PWM;                                          // thread id
osThreadDef (Thread_PWM, osPriorityNormal, 1, 0);                   // thread object

// prototypes fonctions
void Init_UART(void);
extern ARM_DRIVER_USART Driver_USART1;

//fonction de CallBack lancee si Event T ou R
void event_UART(uint32_t event)
{
	switch (event) {
		
		case ARM_USART_EVENT_RECEIVE_COMPLETE : 	
			osSignalSet(tid_Thread_R, 0x01);
			LPC_GPIO2->FIOPIN0 |= (1<<6);
			
			break;
		
		case ARM_USART_EVENT_SEND_COMPLETE  : 	
			break;
		
		default : 
			break;
	}
}

int main (void){

	osKernelInitialize ();                    // initialize CMSIS-RTOS
	Initialisation_Pilotage();
	Init_UART();
	// initialize peripherals here 
	LPC_GPIO2->FIODIR0 &= ~(1<<6);
	LPC_GPIO2->FIOPIN0 &= ~(1<<6);
	//DisplayTitre();

//		
	maBAL=osMailCreate(osMailQ(maBAL), NULL);
	NVIC_SetPriority(UART1_IRQn,2);
	
	//Creation of 3 tasks Thread_T, Thread_R & Thread_D
//		tid_Thread_T = osThreadCreate (osThread(Thread_T), NULL); //Transmit commands
		tid_Thread_R = osThreadCreate (osThread(Thread_R), NULL); //Receive responses
		tid_Thread_PWM = osThreadCreate (osThread(Thread_PWM), NULL); //Display responses
	
	osKernelStart ();                         // start thread execution 
	
	osDelay(osWaitForever);
	
	return 0;
}


//// Tache de réception
void Thread_R (void const *argument) {

	char Recep[2];
	ValeurPos *ptr;
	
  while (1) {

		Driver_USART1.Receive(Recep,2);		// A mettre ds boucle pour recevoir 
		osSignalWait(0x01, osWaitForever);	// sommeil attente reception
		ptr = osMailAlloc(maBAL, osWaitForever);
		ptr -> PositionX = Recep[0];
		ptr -> PositionY = Recep[1];
		osMailPut (maBAL, ptr);
			
  }
}

void Thread_PWM (void const *argument) {

	char PositionXv, PositionYv;
	double PosCtrlX, PosCtrlY;
	char CtrlX[30], CtrlY[30];
	ValeurPos *ptr;
	osEvent RxEv;
	
  while (1) {
		
		RxEv = osMailGet (maBAL, osWaitForever);
		ptr=RxEv.value.p;
		PositionXv = ptr -> PositionX;
		PositionYv = ptr -> PositionY;
		osMailFree(maBAL, ptr);
		PosCtrlX = (double) PositionXv * 0.0037878787879;
		PosCtrlY = (double) PositionYv * 0.0038167938931;
		
//		Position_Roues(PosCtrlX);
//		Avancer(PosCtrlY);
		Position_Roues(0.5);
		Avancer(0.2);
	}
}

void Init_UART(void){
	Driver_USART1.Initialize(event_UART);
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



