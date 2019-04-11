#define osObjectsPublic                 // define objects in main module
#include "osObjects.h"                  // RTOS object definitions
#include "Driver_I2C.h"                 // ::CMSIS Driver:I2C
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include "LPC17xx.h"                    // Device header
#include "RTE_Device.h"                 // Keil::Device:Startup
#include "stdio.h"

#define SLAVE_SRF10_ADDR1    	0xE0/2			// Adresse esclave n°1 sur 7 bits

extern ARM_DRIVER_I2C Driver_I2C2;
extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;
void tache1(void const *argument);

/*Identification des taches*/
osThreadId ID_tache1;

/*Definition des taches*/
osThreadDef(tache1, osPriorityNormal, 1, 0);

/*Initialisation du peripherique I2C*/
void Init_I2C(void){
	Driver_I2C2.Initialize(NULL);
	Driver_I2C2.PowerControl(ARM_POWER_FULL);
	Driver_I2C2.Control(	ARM_I2C_BUS_SPEED,							// 2nd argument = débit
												ARM_I2C_BUS_SPEED_STANDARD  );	// 100 kHz
	Driver_I2C2.Control(	ARM_I2C_BUS_CLEAR,0 );
}


/*Ecrire un octect sur un esclave -> Esclave + Sous-adresse + Commande*/
void write1byte(char Saddress, char SubAddress, char data){
	uint8_t tabwrite[2];
	tabwrite[0] = SubAddress;
	tabwrite[1] = data;
	// Ecriture vers registre esclave : START + ADDR(W) + 1W_DATA + 1W_DATA + STOP
	Driver_I2C2.MasterTransmit (Saddress, tabwrite, 2, false);		// false = avec stop
	while (Driver_I2C2.GetStatus().busy == 1);										// attente fin transmission
}

/*Lire un octet venant d'un esclave*/
char read1byte(char Saddress, uint8_t SubAddress){
	uint8_t retour;
	// Ecriture vers registre esclave : START + ADDR(W) + 1W_DATA + 1W_DATA + STOP
	Driver_I2C2.MasterTransmit (Saddress, &SubAddress, 1, true);	// true = sans stop
	while (Driver_I2C2.GetStatus().busy == 1);										// attente fin transmission
	//Lecture de data esclave : START + ADDR(R) + 1R_DATA + STOP
	Driver_I2C2.MasterReceive (Saddress, &retour, 1, false);			// false = avec stop
	while (Driver_I2C2.GetStatus().busy == 1);										// attente fin transmission
	return retour;
}

/*Lire N octets venant d'un esclave*/
void readNbyte(uint8_t Saddress, uint8_t SubAddress, uint8_t N, uint8_t *retour){
	
	/* Ecriture vers registre esclave : START + ADDR(W) + 1W_DATA + 1W_DATA + STOP */
	Driver_I2C2.MasterTransmit (Saddress, &SubAddress, 1, false);	// true = sans stop
	while (Driver_I2C2.GetStatus().busy == 1);										// attente fin transmission

	/* Lecture des data esclave : START + ADDR(R) + NR_DATA + STOP */
	Driver_I2C2.MasterReceive (Saddress, retour, N, false);				// false = avec stop
	while (Driver_I2C2.GetStatus().busy == 1);										// attente fin transmission
}


void tache1(void const *argument){																//Données du Capteur lateral droit
	char tab[50];
	uint8_t datalow, datahigh, retour[2];
	int data;

	while(1){
		
		write1byte(SLAVE_SRF10_ADDR1,0x00,0x51);											//Configuration en cm et Lancement de l'acquisition
		osDelay(70);																									//Attente acquisition des données
		
		readNbyte(SLAVE_SRF10_ADDR1, 0x02, 2, retour);								//Lecture de 2 octets à partir du bit de poid fort
		data = retour[1] + (retour[0]<<8);														//Regroupement des valeurs
		
		
//		datahigh = read1byte(SLAVE_SRF10_ADDR1, 0x02);							//Reccuperation de l'octet de poid fort
//		datalow	= read1byte(SLAVE_SRF10_ADDR1, 0x03);								//Reccuperation de l'octet de poid faible
//		data = datalow + (datahigh<<8);															//Regroupement des data

		sprintf(tab,"valeur: %4d cm",data);															//Affichage de la distance
		GLCD_DrawString(0,1*24,tab);											
	}
}

/*
 * main: initialize and start the system
 */
int main (void){

	osKernelInitialize ();                    // initialize CMSIS-RTOS
	
	GLCD_Initialize();
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	Init_I2C();

/*Création des Taches*/
	ID_tache1 = osThreadCreate (osThread(tache1), NULL);
	
	osKernelStart ();                         // start thread execution 
	osDelay(osWaitForever);										// Belle au bois dormant
	return 0;
}
