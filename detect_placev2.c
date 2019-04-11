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

char tab1[50], tab2[50];
int data;

void tache1(void const *argument);
void tache2(void const *argument);
void tache3(void const *argument);
void affichage(void const *argument);

/*Identification des taches*/
osThreadId ID_tache1;
osThreadId ID_tache2;
osThreadId ID_tache3;
osThreadId ID_affichage;

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


void tache1(void const *argument){												//Données du Capteur lateral droit
	
	uint8_t retour[2];

	while(1){
		
		write1byte(SLAVE_SRF10_ADDR1,0x00,0x51);							//Configuration en cm et Lancement de l'acquisition
		osDelay(65);																					//Attente acquisition des données
		
		readNbyte(SLAVE_SRF10_ADDR1, 0x02, 2, retour);				//Lecture de 2 octets à partir du bit de poid fort
		data = retour[1] + (retour[0]<<8);										//Regroupement des valeurs

		sprintf(tab1,"valeur: %4d cm",data);									//Affichage de la distance
		
		if (data > 15){
			osSignalSet(ID_tache2, 0x0001);											//Active l'event 1 si un espace
			}
	}
}

void tache2(void const *argument){												//Detection place libre
	osEvent place;
	while (1){
		place = osSignalWait(0x0001,100);											//Attend l'event 1
		if(place.status != osEventTimeout){
			osSignalSet(ID_tache3, 0x0002);											//Active l'event 2
		}
		else{
			sprintf(tab2,"Espace Libre : NON");
		}
	}
}

void tache3(void const *argument){												//Detection places libre
	int wait=0;
	osEvent verif;	
	while (1){
		osDelay(10000);																				//Etre sur d'avoir la bonne place
		verif = osSignalWait(0x0002,osWaitForever);										//Attente de l'event 2
		if(verif.status != osEventTimeout){
			if(data > 15) wait++;
		}
		else{
			wait = 0;
		}
		if(wait == 10){ 
			sprintf(tab2,"Espace Libre : OUI");
			wait=0;
		}
	}
}

void affichage(void const *argument){											//Affichage sur ecran LCD
	while (1){
		GLCD_DrawString(0,1*24,tab1);
		GLCD_DrawString(0,2*24,tab2);
	}
}

/*Definition des taches*/
osThreadDef(tache1, osPriorityNormal, 1, 0);
osThreadDef(tache2, osPriorityNormal, 1, 0);
osThreadDef(tache3, osPriorityHigh, 1, 0);
osThreadDef(affichage, osPriorityNormal, 1, 0);

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
	ID_tache2 = osThreadCreate (osThread(tache2), NULL);
	ID_tache3 = osThreadCreate (osThread(tache3), NULL);
	ID_affichage = osThreadCreate (osThread(affichage), NULL);
	
	write1byte(SLAVE_SRF10_ADDR1,0x01,7);							//gain analogique à 6
	write1byte(SLAVE_SRF10_ADDR1,0x02,65);						//range numérique à 65 <=> 175cm max dans nos conditions, en théorie d'après le fabriquant 280cm max
	osKernelStart ();                         // start thread execution 
	osDelay(osWaitForever);										// Belle au bois dormant
	return 0;
}