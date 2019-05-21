#include "LPC17xx.h"


void Init_I2C(void);
void write1byte(unsigned char composant, unsigned char registre, unsigned char valeur);
unsigned char read1byte(unsigned char composant, unsigned char registre);
void zero_data_request(void);
unsigned char JoystickX(void);
unsigned char JoystickY(void);