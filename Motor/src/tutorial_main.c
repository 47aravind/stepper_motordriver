/*******************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only 
* intended for use with Renesas products. No other uses are authorized. This 
* software is owned by Renesas Electronics Corporation and is protected under
* all applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT
* LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE 
* AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS 
* ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE 
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR
* ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE
* BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software
* and to discontinue the availability of this software. By using this software,
* you agree to the additional terms and conditions found by accessing the 
* following link:
* http://www.renesas.com/disclaimer *
* Copyright (C) 2012 Renesas Electronics Corporation. All rights reserved.    
*******************************************************************************/
/*******************************************************************************
* File Name     : tutorial_main.c
* Version       : 1.0
* Device(s)     : RX63N
* Tool-Chain    : Renesas RX Standard Toolchain 1.0.0
* H/W Platform  : YRDKRX63N
* Description   : This tutorial sample will demonstrate basic use of the YRDK
*                 hardware and the J-Link debugger. 
* Operation     : 1.  Build and download the tutorial project to the YRDK.
*
*                 2.  Click 'Reset Go' to start the software.
*         
*                 3.  "Renesas RX63N" will be displayed on the debug LCD,
*                     and the user LEDs will flash.
*          
*                 4.  The user LEDs will flash at a fixed rate until either a
*                     switch is pressed, or the LEDs have flashed 200 times.
*          
*                 5.  The software will then vary the rate in which the LEDs flash
*                     by the position of the potentiometer, VR1. Turn the pot in
*                     both directions, and observe the change in flash rate.
*          
*                 6.  While the LEDs flash at a varying rate, the second line of
*                     the debug LCD will show " STATIC ". The second LCD line will
*                     slowly be replaced, letter by letter, with the string 
*                     "TEST TEST".
*          
*                 7.  Once the second line of the debug LCD shows "TEST TEST"
*                     fully, it will return back to showing "RX63N". The LEDs will
*                     continue to flash at a varying rate until the tutorial is
*                     stopped.
*          
*                 8.  In order to repeat the tutorial, click 'Reset Go'.
*******************************************************************************/
/*******************************************************************************
* History : DD.MM.YYYY     Version     Description
*         : 24.02.2012     1.00        First release              
*******************************************************************************/

/*******************************************************************************
Includes   <System Includes> , "Project Includes"
*******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <machine.h>
#include "platform.h" 
#include "r_switches.h"
#include "cmt_periodic_multi.h"
#include "timer_adc.h"
#include "flash_led.h"
#include "customADC.h"
#include "r_riic_rx600.h"
#include "r_riic_rx600_master.h"
#include "riic_master_main.h"
#include "thermal_sensor_demo.h"
/*******************************************************************************
Macro Definitions
*******************************************************************************/
//#define TIMER_COUNT_300MS 28125  /* 600mS with 48MHz pclk/512 */
#define TIMER_COUNT 9375  /* Should give 100mS with 48MHz pclk/512 */

#define MCU_NAME "   RX63N    "
#define CHANNEL_0   0


#pragma interrupt(Drive_isr(vect = VECT(ICU,IRQ11)))            // Interrupt declaration for Drive pin
#pragma interrupt(Direction_isr(vect = VECT(ICU,IRQ14)))		// Interrupt declaration for Direction pin		

#pragma interrupt(SW1_isr(vect = VECT(ICU,IRQ8)))				// Interrupt declaration for SW1
#pragma interrupt(SW2_isr(vect = VECT(ICU,IRQ9)))				// Interrupt declaration for SW2
#pragma interrupt(SW3_isr(vect = VECT(ICU,IRQ12)))				// Interrupt declaration for SW3

/********************************************************************************
LED Macro Definition
********************************************************************************/

#define  LED4                   PORTD.PODR.BIT.B5
#define  LED5                   PORTE.PODR.BIT.B3
#define  LED6                   PORTD.PODR.BIT.B2
#define  LED7                   PORTE.PODR.BIT.B0
#define  LED8                   PORTD.PODR.BIT.B4
#define  LED9                   PORTE.PODR.BIT.B2
#define  LED10                  PORTD.PODR.BIT.B1
#define  LED11                  PORTD.PODR.BIT.B7
#define  LED12                  PORTD.PODR.BIT.B3
#define  LED13                  PORTE.PODR.BIT.B1
#define  LED14                  PORTD.PODR.BIT.B0
#define  LED15                  PORTD.PODR.BIT.B6

#define  LED_ON                 0
#define  LED_OFF                1

/*******************************************************************************
Switch Macro Definition
*******************************************************************************/
#define  SW1 					PORT4.PIDR.BIT.B0
#define  SW2 					PORT4.PIDR.BIT.B1
#define  SW3 					PORT4.PIDR.BIT.B4

/*******************************************************************************
External Pin Macro Definition
*******************************************************************************/
#define Direction_Pin 			PORT4.PIDR.BIT.B6
#define Drive_Pin				PORT4.PIDR.BIT.B3

/*******************************************************************************
External functions and public global variables. 
*******************************************************************************/
/* Statics test replacement variable */
uint8_t g_working_string[13] = "   STATIC   "; /* 12 characters plus NULL terminator. */

/* Statics test const variable */
const uint8_t g_replacement_str[] = "TEST TEST"; /* Must be 12 chars or less. */

volatile bool g_sw1_press = false;
volatile bool g_sw2_press = false;
volatile bool g_sw3_press = false;

/* Switch Press condition check variables */
volatile bool sw1_press = false;
volatile bool sw2_press = false;
volatile bool sw3_press = false;

/*Status Flags for Direction and system Overheat */
unsigned int Direction_Flag=0;
unsigned int Overheat_Flag=0;

/* Not all boards have a thermal sensor, so this will be tested. */
static bool g_thermal_sensor_good = false;

/*******************************************************************************
Private function prototypes and global variables
*******************************************************************************/
static void timer_callback(void);
static void LED_Init(void);
static void Switches_Init(void);
static void Drive_Init(void);
static void Direction_Init(void);
static void Drive_isr(void);
static void Direction_isr(void);
static void SW1_isr(void);
static void SW2_isr(void);
static void SW3_isr(void);
static void clockwise(void);
static void counter_clockwise(void);
static void Temp_Sensor(void);
static riic_ret_t riic_master_init(void);

volatile static bool g_delay_complete = false;
unsigned int wave_count1	= 4;
unsigned int full_count1	= 4;
unsigned int half_count1	= 8;
unsigned int wave_count2	= 4;
unsigned int full_count2	= 4;
unsigned int half_count2	= 8;

unsigned int i,j,k,l,m,n;
int CURR_TEMP;
int THRESHOLD_TEMP = 400;
/******************************************************************************
* Function name: main
* Description  : Main program function. The function first initialises the debug
*                LCD, and displays the splash screen on the LCD. The function
*                then calls the flash_led, timer_adc and statics_test functions. 
*                These start by flashing the LEDs at a fixed rate until a switch is
*                pressed, then the flash rate varies with the position of the   
*                pot, RV1. Finally, statics_test displays " STATICS " on the    
*                debug LCD, which is replaced with "TEST TEST" a letter at a 
*                time.
* Arguments    : none
* Return value : none
******************************************************************************/
void main(void)
{
   riic_ret_t iic_ret = RIIC_OK; 
    /* Initialize the debug LCD on the RSPI bus. */
    lcd_initialize();
    /* Clear LCD */
    lcd_clear();
    
    /* Display message on LCD */
    lcd_display(LCD_LINE1, " MOTOR-COIL ");
    lcd_display(LCD_LINE2, " SIMULATION ");

    cmt_init(); 						/* Initialize the CMT unit for application timing tasks. */ 

    R_SWITCHES_Init(); 					/* Prepare the board switches for use. */
    
    LED_Init();							// Initializes LEDs
	Drive_Init();						// Initializes Drive pin with interrupts
	Direction_Init();					// Initializes Direction pin with interrupts
	iic_ret |= riic_master_init();		/* Prepare an RIIC channel for master mode communications. */
    
    while (RIIC_OK != iic_ret)
    {
        nop(); 							/* Failure to initialize here means demo can not proceed. */    
    }

    /* Some boards may not have the thermal sensor present. This sequence 
       demonstrates an example of how to recover from some bus error conditions. */
    iic_ret |= thermal_sensor_init();    /* Initialize IIC thermal sensor */
   
    if (RIIC_OK == iic_ret)
    {
        g_thermal_sensor_good = true;
    }
    /* Thermal sensor not present or malfunctioning. Remove it from the demo. */ 
	else  
    {   /* Got a NACK. */
        g_thermal_sensor_good = false;  
          
        iic_ret = R_RIIC_Reset(RIIC_CHANNEL); /* Do soft reset to clean up bus states. */
        
        if (RIIC_RESET_ERR & iic_ret) /* Check for successful IIC soft-reset. */
        {   /* Soft-reset failed. Need to do complete re-initialization. */
            /* Need to release the channel first berfore it can be re-initialized. */
            R_RIIC_ReleaseChannel(RIIC_CHANNEL);    
            iic_ret = riic_master_init(); 
        }
        
        while (RIIC_OK != iic_ret)
        {
            nop(); /* Failure to initialize here means demo can not proceed. */
        }               
    }
    cmt_callback_set(CHANNEL_0,&Temp_Sensor); 	// Temp_Sensor is called   
	cmt_start(CHANNEL_0,TIMER_COUNT);			// Calls the callback function every 100 mS
    while (1)
    {
	 
	    /* All done. Loop here forever. LEDs will continue to flash as 
           at a variable rate as the timer ISR executes. */
    }
} /* End of function main(). */


/*******************************************************************************
* Function name: riic_master_init
* Description  : Prepare an RIIC channel for master mode communications used in the demo
* Arguments    : none
* Return value : riic_ret_t -
*                   RIIC result code
*******************************************************************************/
static riic_ret_t riic_master_init(void) 
{ 
    /* Initialize a data structure used by the R_RIIC_Init() API function. */
    /* See r_riic_rx600.h for definitions of this structure. */
    riic_config_t   riic_master_config = {RIIC_CHANNEL, RIIC_MASTER_CONFIG, 
                                          0, 
                                          0, 
                                          0, 
                                          0, 
                                          MASTER_IIC_ADDRESS_LO, 
                                          MASTER_IIC_ADDRESS_HI}; 

    /* Initialize the RIIC channel for communication with the accelerometer. */
    return R_RIIC_Init(&riic_master_config);
}



/*******************************************************************************
* Function name: statics_test
* Description  : Static variable test routine. The function replaces the
*                contents of the string g_working_string with that of g_replacement_str, one
*                element at a time. Right-click the variable g_working_string, and select
*                instant watch - click add in the subsequent dialog. If you step
*                through the function, you can watch the string elements being
*                overwritten with the new data.
* Arguments    : none
* Return value : none
*******************************************************************************/
void statics_test(void)
{
    /* Declare loop count variable */
    uint8_t loop_counter;

    /* Write the initial contents of g_working_string to the debug LCD */
    lcd_display(LCD_LINE2, g_working_string);

    /* Set up the callback function on cmt channel 0 */   
    cmt_callback_set(CHANNEL_0, &timer_callback);
    
    /* Replace each element of g_working_string in each loop iteration with the contents 
       of g_replacement_str */
    for (loop_counter = 0; loop_counter < strlen((char*)g_replacement_str); loop_counter++)
    {
        /* Replace character from g_working_string with characer from g_replacement_str */
        g_working_string[loop_counter] = g_replacement_str[loop_counter];

        /* Write current contents of g_working_string to the debug LCD */
        lcd_display(LCD_LINE2, g_working_string);
   
        g_delay_complete = false; /* Clear this flag variable. */
        
        /* Use a timer to create the delay between each loop iteration */         
        
        while(!g_delay_complete)
        {
            /* wait until the delay has completed. */
        }               
    }

    /* Revert the debug LCD back to displaying the MCU_NAME */    
    lcd_display(LCD_LINE2, MCU_NAME);
    
} /* End of function statics_test(). */


/******************************************************************************
* Function name: timer_callback
* Description  : This function is called from the cmt_timer compare-match ISR.
*              : It sets a global flag that is polled for delay loop control.
* Arguments    : None
* Return value : None
******************************************************************************/
void timer_callback(void)
{
    cmt_stop(CHANNEL_0);
    g_delay_complete = true;
    
} /* End of function timer_callback() */



/******************************************************************************
* Function name: sw1_callback
* Description  : Callback function that is executed when SW1 is pressed.
*                Called by sw1_isr in r_switches.c
* Arguments    : none
* Return value : none
******************************************************************************/
void sw1_callback(void)
{
    g_sw1_press = true; /* The switch was pressed. */
} /* End of function sw1_callback() */


/******************************************************************************
* Function name: sw2_callback
* Description  : Callback function that is executed when SW2 is pressed.
*                Called by sw2_isr in r_switches.c
* Arguments    : none
* Return value : none
******************************************************************************/
void sw2_callback(void)
{
    g_sw2_press = true; /* The switch was pressed. */
} /* End of function sw2_callback() */


/******************************************************************************
* Function name: sw3_callback
* Description  : Callback function that is executed when SW3 is pressed.
*                Called by sw3_isr in r_switches.c
* Arguments    : none
* Return value : none
******************************************************************************/
void sw3_callback(void)
{
    g_sw3_press = true; /* The switch was pressed. */
} /* End of function sw3_callback() */

/******************************************************************************
* Function name: LED_Init
* Description  : Sets all LEDs as outputs. 
* Arguments    : none
* Return value : none
******************************************************************************/

static  void  LED_Init (void)		
{
    PORTD.PDR.BIT.B0 = 1;                                       
    PORTD.PDR.BIT.B1 = 1;
    PORTD.PDR.BIT.B2 = 1;
    PORTD.PDR.BIT.B3 = 1;
    PORTD.PDR.BIT.B4 = 1;
    PORTD.PDR.BIT.B5 = 1;
    PORTD.PDR.BIT.B6 = 1;
    PORTD.PDR.BIT.B7 = 1;
    PORTE.PDR.BIT.B0 = 1;
    PORTE.PDR.BIT.B1 = 1;
    PORTE.PDR.BIT.B2 = 1;
    PORTE.PDR.BIT.B3 = 1;
}

/******************************************************************************
* Function name: Drive_Init
* Description  : Register settings for the drive pin interrupt 
* Arguments    : none
* Return value : none
******************************************************************************/
static void Drive_Init()
{
	IEN(ICU,IRQ11)			= 0;		// Disbles interrupt for the initial setting
	/*Port Pin setting */
	PORT4.PDR.BIT.B3		= 0;
	PORT4.PMR.BIT.B3		= 0;
	/*Multi-Pin configurations */
	MPC.PWPR.BIT.B0WI		= 0;
	MPC.PWPR.BIT.PFSWE		= 1;
	MPC.P43PFS.BIT.ISEL		= 1;	   
	ICU.IRQCR[11].BIT.IRQMD	= 0x01;    	// Selects type of interrupt
	IPR(ICU,IRQ11) 			= 3;       	// Sets interrupt priority
	IR(ICU,IRQ11) 			= 0;		// Clears the interrupt buffer
	MPC.PWPR.BIT.PFSWE		= 0;
	IEN(ICU,IRQ11) 			= 1;		// Enables interrupt after all the setting is done
}

/******************************************************************************
* Function name: Direction_Init
* Description  : Register settings for the direction pin interrupt 
* Arguments    : none
* Return value : none
******************************************************************************/
static void Direction_Init()
{
	IEN(ICU,IRQ14)			= 0;		// Disbles interrupt for the initial setting
	/*Port Pin setting */
	PORT4.PDR.BIT.B6		= 0;
	PORT4.PMR.BIT.B6		= 0;
	/*Multi-Pin configurations */
	MPC.PWPR.BIT.B0WI 		= 0;
	MPC.PWPR.BIT.PFSWE		= 1;
	MPC.P46PFS.BIT.ISEL		= 1;
	ICU.IRQCR[14].BIT.IRQMD	= 0x10;     // Selects type of interrupt
	IPR(ICU,IRQ14) 			= 4;		// Sets interrupt priority
	IR(ICU,IRQ14) 			= 0;		// Clears the interrupt buffer 
	MPC.PWPR.BIT.PFSWE		= 0;
	IEN(ICU,IRQ14) 			= 1;		// Enables interrupt after all the setting is done
}

/******************************************************************************
* Function name: Drive_isr
* Description  : Interrupt Service Routine for Drive Pin 
* Arguments    : none
* Return value : none
******************************************************************************/
static void Drive_isr()
{
if (Direction_Flag == 0)					// Clock-wise rotation
 {					
   lcd_display(LCD_LINE6, " Clock ");
   clockwise();
 }
else										// Counter clock-wise rotation
 {
	lcd_display(LCD_LINE6, " Anti-Clock ");
	counter_clockwise();
 }
}

/******************************************************************************
* Function name: Direction_isr
* Description  : Interrupt Service Routine for Direction Pin 
* Arguments    : none
* Return value : none
******************************************************************************/
static void Direction_isr()
{
	if(Direction_Pin == 0)			// clockwise rotation
	
		Direction_Flag = 0;       

	else if(Direction_Pin == 1)		// Counter clockwise rotation
	                      
		Direction_Flag = 1;
}

/******************************************************************************
* Function name: SW1_isr
* Description  : Switch 1 Interrupt Service Routine 
* Arguments    : none
* Return value : none
******************************************************************************/
static void SW1_isr()
{
		lcd_clear();
		lcd_display(LCD_LINE1, " WAVE-DRIVE ");
		sw1_press = true;
		sw2_press = false;
		sw3_press = false;
		LED4=LED5=LED6=LED7=LED8=LED9=LED10=LED11=LED12=LED13=LED14=LED15=LED_OFF;
} 

/******************************************************************************
* Function name: SW2_isr
* Description  : Switch 2 Interrupt Service Routine 
* Arguments    : none
* Return value : none
******************************************************************************/
static void SW2_isr()
{
		lcd_clear();
		lcd_display(LCD_LINE1, " HALF-STEP ");
		sw1_press = false;
		sw2_press = true;
		sw3_press = false;
		LED4=LED5=LED6=LED7=LED8=LED9=LED10=LED11=LED12=LED13=LED14=LED15=LED_OFF;
}

/******************************************************************************
* Function name: SW3_isr
* Description  : Switch 3 Interrupt Service Routine 
* Arguments    : none
* Return value : none
******************************************************************************/
static void	SW3_isr()
{
		lcd_clear();
		lcd_display(LCD_LINE1, " FULL-STEP "); 
		sw1_press = false;
		sw2_press = false;
		sw3_press = true;
		LED4=LED5=LED6=LED7=LED8=LED9=LED10=LED11=LED12=LED13=LED14=LED15=LED_OFF;
}


/******************************************************************************
* Function name: Clockwise
* Description  : LED function that rotates clock-wise direction
* Arguments    : none
* Return value : none
******************************************************************************/
static void clockwise()
{
	if(sw1_press)							//wave-drive rotation
	{
	  wave_count1 = wave_count1 - 1;
	  i = wave_count1 % 4;
	  switch(i)
	  {
		  case 3: LED15=LED4=LED5 = LED_ON;
		          LED12=LED13=LED14 = LED_OFF;		  		  	
				  break;
		  case 2: LED15=LED4=LED5 = LED_OFF;
		          LED6=LED7=LED8 = LED_ON;
				  break;
		  case 1: LED6=LED7=LED8 = LED_OFF;
		  		  LED9=LED10=LED11 = LED_ON;
				  break;
		  case 0: LED9=LED10=LED11 = LED_OFF;
		  		  LED12=LED13=LED14 = LED_ON;
				  wave_count1=4;
		          break;
		  default:break;
		}
	}
  if(sw2_press)						//Half-Step rotation
  {
	 half_count1 = half_count1 - 1;
	 j = half_count1 % 8;
	 switch(j)
	  {
		case 7: LED15=LED5=LED4=LED6=LED7=LED8 = LED_ON;
				 break;
		case 6: LED15=LED5=LED4 = LED_OFF;
				 LED6=LED7=LED8 = LED_ON;
				 break;
		case 5:  LED7=LED6=LED8=LED9=LED10=LED11 = LED_ON;
				 break;
		case 4:  LED6=LED7=LED8 = LED_OFF;
				 LED9=LED10=LED11 = LED_ON;
		         break;
		case 3:  LED9=LED10=LED11=LED12=LED13=LED14 = LED_ON;
				 break;
		case 2:  LED9=LED10=LED11 = LED_OFF;
				 LED12=LED13=LED14 = LED_ON;
				 break;
		case 1:  LED12=LED13=LED14=LED15=LED4=LED5 = LED_ON;
				 break;
		case 0:  LED12=LED13=LED14 = LED_OFF;
				 LED15=LED4=LED5 = LED_ON;
				 half_count1=8;
				 break;		
		default: break;   
	  }
  }
  if(sw3_press) 						// Full step rotation
  {
	  full_count1 = full_count1 - 1;
	  k = full_count1 % 4;
	  switch(k)
	  {
		case 3: LED12=LED13=LED14 = LED_OFF;
		        LED4=LED5=LED15=LED6=LED7=LED8 = LED_ON;
				break;
		case 2: LED4=LED5=LED15 = LED_OFF;
				LED6=LED7=LED8=LED9=LED10=LED11 = LED_ON;
				break;
		case 1: LED6=LED7=LED8 = LED_OFF;
				LED9=LED10=LED11=LED12=LED13=LED14 = LED_ON;
				break;
		case 0: LED9=LED10=LED11 = LED_OFF;
				LED12=LED13=LED14=LED15=LED4=LED5 = LED_ON;
				full_count1=4;
				break;
		default:break;
	  }
  } 
}

/******************************************************************************
* Function name: Counter_clockwise
* Description  : LED function that rotates counter clock-wise direction
* Arguments    : none
* Return value : none
******************************************************************************/
static void counter_clockwise()
{
  if(sw1_press)							//wave-drive
	{
	  wave_count2 = wave_count2 - 1;
	  k = wave_count2 % 4;
	  switch(k)
		 {
		  case 3: LED15=LED4=LED5 = LED_ON;
		          LED8=LED7=LED6 = LED_OFF;		  		  	
				  break;
		  case 2: LED15=LED4=LED5 = LED_OFF;
		          LED14=LED13=LED12 = LED_ON;
				  break;
		  case 1: LED14=LED13=LED12 = LED_OFF;
		  		  LED9=LED10=LED11 = LED_ON;
				  break;
		  case 0: LED9=LED10=LED11 = LED_OFF;
		  		  LED8=LED7=LED6 = LED_ON;
				  wave_count2=4;
		          break;
		  default:break;
	   }
	}
  if(sw2_press)							// Half-step
  {
	 half_count2 = half_count2 - 1;
	 l = half_count2 % 8;
	 printf("%d\n",l);
	 switch(l)
	  {
		case 7: LED15=LED5=LED4=LED12=LED13=LED14 = LED_ON;
				 break;
		case 6: LED15=LED5=LED4 = LED_OFF;
				 LED12=LED13=LED14 = LED_ON;
				 break;
		case 5:  LED12=LED13=LED14=LED9=LED10=LED11 = LED_ON;
				 break;
		case 4:  LED12=LED13=LED14 = LED_OFF;
				 LED9=LED10=LED11 = LED_ON;
		         break;
		case 3:  LED9=LED10=LED11=LED8=LED7=LED6 = LED_ON;
				 break;
		case 2:  LED9=LED10=LED11 = LED_OFF;
				 LED8=LED7=LED6 = LED_ON;
				 break;
		case 1:  LED8=LED7=LED6=LED15=LED4=LED5 = LED_ON;
				 break;
		case 0:  LED8=LED7=LED6 = LED_OFF;
				 LED15=LED4=LED5 = LED_ON;
				 half_count2=8;
				 break;		
		default: break;   
	  }
  }
  if(sw3_press)						// Full-step
  {
	  full_count2 = full_count2 - 1;
	  m = full_count2 % 4;
	  switch(m)
	  {
		case 3: LED8=LED7=LED6 = LED_OFF;
		        LED4=LED5=LED15=LED14=LED13=LED12 = LED_ON;
				break;
		case 2: LED4=LED5=LED15 = LED_OFF;
				LED12=LED13=LED14=LED9=LED10=LED11 = LED_ON;
				break;
		case 1: LED12=LED13=LED14 = LED_OFF;
				LED9=LED10=LED11=LED8=LED7=LED6 = LED_ON;
				break;
		case 0: LED9=LED10=LED11 = LED_OFF;
				LED8=LED7=LED6=LED15=LED4=LED5 = LED_ON;
				full_count2=4;
				break;
		default:break;
	  }
  }     
}

/******************************************************************************
* Function name: Clockwise
* Description  : Routine that checks the temperature continuously using the Temperature Sensor
* Arguments    : none
* Return value : none
******************************************************************************/
static void Temp_Sensor(void)
{
	if (g_thermal_sensor_good) /* Only run thermal sensor demo if it is present. */
    {
        CURR_TEMP=temperature_display();
    }
	  printf("%d\n",CURR_TEMP);      // For debug console
      if(THRESHOLD_TEMP < CURR_TEMP)
	  {	
		  lcd_display(LCD_LINE1,"ALERT!");
		  lcd_display(LCD_LINE2,"System is");
		  lcd_display(LCD_LINE3,"SWITCHED-OFF");
		  lcd_display(LCD_LINE4,"due to");
		  lcd_display(LCD_LINE5,"OVERHEAT");
		  /*Disables all interrupts when temperature rises */
		  IEN(ICU,IRQ11) 			= 0;		
		  IEN(ICU,IRQ14) 			= 0;
		  IEN(ICU,IRQ8) 			= 0;
		  IEN(ICU,IRQ9) 			= 0;
		  IEN(ICU,IRQ11) 			= 0;
		  IEN(ICU,IRQ12) 			= 0;
		  cmt_stop(0);
		  Overheat_Flag=1;
	  }
	  else if((Overheat_Flag==1) && (THRESHOLD_TEMP > CURR_TEMP))
	  {
		  lcd_display(LCD_LINE1,"WELCOME!");
		  lcd_display(LCD_LINE2,"System is");
		  lcd_display(LCD_LINE3,"SWITCHED-ON");
		  /* Enables all interrupts when temperature falls below the threshold */
		  IEN(ICU,IRQ11) 			= 1;
		  IEN(ICU,IRQ14) 			= 1;
		  IEN(ICU,IRQ9) 			= 1;
		  IEN(ICU,IRQ11) 			= 1;
		  IEN(ICU,IRQ12) 			= 1;
		  cmt_start(0,TIMER_COUNT);
		  Overheat_Flag=0;
	  }
}