#include <machine.h>
/* Standard string manipulation & formatting functions */
#include <stdio.h>
#include <string.h>
/* Defines standard variable types used in this function */
#include <stdint.h>
/* Board includes. */
#include "platform.h"

#include <customADC.h>
/* Graphics library support */
#include "glyph.h"
/* RSPI package. */
#include "r_rspi_rx600.h"


void adc_Init(void)
{
#ifdef PLATFORM_BOARD_RDKRX63N
	SYSTEM.PRCR.WORD = 0xA50B; /* Protect off */
#endif

    /* Power up the S12ADC */
    MSTP(S12AD) = 0;
	MSTP(TEMPS) = 0;
#ifdef PLATFORM_BOARD_RDKRX63N
SYSTEM.MSTPCRB.BIT.MSTPB8 = 0;	
#endif    
    /* Set up the I/O pin that will be used as analog input source. In this 
       demo the potentiometer will be used and is connected to port 42. */
    //PORT4.PODR.BIT.B2 = 0;    /* Clear I/O pin data register to low output. */
    //PORT4.PDR.BIT.B2  = 0;    /* Set I/O pin direction to input. */
    //PORT4.PMR.BIT.B2  = 0;    /* First set I/O pin mode register to GPIO mode. */    
    //MPC.P42PFS.BYTE = 0x80;   /* Set port function register to analog input, no interrupt. */    
        
#ifdef PLATFORM_BOARD_RDKRX63N
	SYSTEM.PRCR.WORD = 0xA500; /* Protect on  */
#endif 

    /* ADCSR: A/D Control Register  
    b7    ADST     0 a/d conversion start, Stop a scan conversion process
    b6    ADCS     1 Scan mode select, Continuous scan mode
    b5    Reserved 0 This bit is always read as 0. The write value should always be 0.
    b4    ADIE     0 Disables conversion complete IRQ to ICU
    b3:b2 CKS      0 A/D conversion clock select = PCLK/8
    b1    TRGE     0 Disables conversion to start w/ trigger
    b0    EXTRG    0 Trigger select, Scan conversion start by a timer source or software
	*/
    //S12AD.ADCSR.BYTE = 0x00;
	
	
	/* ADANS1: A/D Channel Select Register 1
	b15:b5  Reserved: These bits are always read as 0. 
                      The write value should always be 0.
    b4:b0   ANS1:     Selects analog inputs of the channels AN016 to AN020 
                      that are subjected to A/D conversion
    */
    
    S12AD.ADANS1.WORD = 0x0000;
    S12AD.ADANS0.WORD = 0x0000;
	
	/* ADADS0: A/D-converted Value Addition Mode Select Register 0
    b15:b0  ADS0: A/D-Converted Value Addition Channel Select for AN000 to AN015.
    */
    S12AD.ADADS0.WORD = 0x0000;
	
	/* ADADS1: A/D-converted Value Addition Mode Select Register 1
	b15:b5  Reserved: These bits are always read as 0. The write value should always be 0.
    b4:b0   ADS1: A/D-Converted Value Addition Channel Select for AN016 to AN020.
    */
    S12AD.ADADS1.WORD = 0x0000;
    
    /* ADADC: A/D-Converted Value Addition Count Select Register
    b1:b0   ADC: 00 = 1 time conversion (same as normal conversion)
    */
	S12AD.ADADC.BYTE = 0x00;  		 /* 1-time conversion */

    /* ADCER: A/D Control Extended Register
    b15     ADRFMT:0  Right align the data in the result registers
    b5      ACE:0 Disables automatic clearing of ADDRn after it is read
    */
	
	S12AD.ADCER.WORD = 0x0020;  	 /* Right align data, automatic clearing on. */

    /* ADSTRGR: A/D Start Triggger Select Register
    b7:b4   Reserved. Always read/write 0.
    b3:b0   ADSTRS: 0, Software trigger or ADTRG0#
    */
	
	S12AD.ADSTRGR.BYTE = 0x00;    //Disables trigger
	
	//S12AD.ADCSR.BIT.ADST = 0 ;
	S12AD.ADCSR.BYTE= 0x0C ;   	 // Single scan mode, PCLK. 
	S12AD.ADEXICR.BIT.TSS = 1;   // Starts Temp. Sensor
	S12AD.ADEXICR.BIT.TSSAD = 1; // TEMP. ADDITION MODE IS SELECTED
	S12AD.ADEXICR.BIT.OCS = 0;   // Disables ref. voltage output
	
	TEMPS.TSCR.BIT.TSEN=1;
	TEMPS.TSCR.BIT.TSOE=1;
	//S12AD.ADCSR.BIT.ADST=1;

} /* End of function S12ADC_init() */

int adc_Read()
{
	uint16_t adc_result;
	
	/* ADANS0: A/D Channel Select Register 0
    b15:b0  ANS0: Selects analog inputs of the channels AN000 to AN015 that are 
    subjected to A/D conversion
    */
    //S12AD.ADANS0.WORD = channel; /* Read AN002, which is connected to the potentiometer */
	
	//TEMPS.TSCR.BIT.TSEN = 1;
	//TEMPS.TSCR.BIT.TSOE = 1;
	
	S12AD.ADCSR.BIT.ADST = 1;		 /* Start the A/D converter */
    
    for(int i=0;i<1000000;i++);
	adc_result = S12AD.ADTSDR;        /* Read the result register for AN2 */
    adc_result = adc_result & 0x0FFF;		
    return adc_result;
}
