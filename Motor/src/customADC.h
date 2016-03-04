#include <platform.h>
#include <stdint.h>


void adc_Init(void); 						// This function initializes the ADC
int adc_Read(); 
void adc_stop(void);							// This function will read an analog value from the specified channel 
