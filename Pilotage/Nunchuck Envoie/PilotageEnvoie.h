#include "LPC17xx.h"

void Initialisation_Pilotage(void);
char Position_Roues(double direction);
char Avancer(double Puissance);
void Init_UART(void);
void SendUARTCommande(char PositionX, char PositionY);