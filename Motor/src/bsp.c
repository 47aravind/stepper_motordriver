/*
*********************************************************************************************************
*                                         BOARD SUPPORT PACKAGE
*
*                            (c) Copyright 2011; Micrium, Inc.; Weston, FL
*
*               All rights reserved. Protected by international copyright laws.
*
*               BSP is provided in source form to registered licensees ONLY.  It is
*               illegal to distribute this source code to any third party unless you receive
*               written permission by an authorized Micrium representative.  Knowledge of
*               the source code may NOT be used to develop a similar product.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                         BOARD SUPPORT PACKAGE
*
*                                              Renesas RX63N
*                                                on the
*                                               YRDKRX63N
*
* Filename      : bsp.c
* Version       : V1.00
* Programmer(s) : PC
*********************************************************************************************************
*/

//#include  <includes.h>
//#include  "iodefine.h"

/*
*********************************************************************************************************
*                                              CONSTANTS
*********************************************************************************************************
*/

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


/*$PAGE*/
/*
*********************************************************************************************************
*                                          LOCAL PROTOTYPES
*********************************************************************************************************
*/

static  void  LED_Init(void);

/*$PAGE*/
/*
*********************************************************************************************************
*                                         BSP INITIALIZATION
*
* Description: This function should be called by the application code before using any functions in this 
*              module.
*
* Arguments  : none
*********************************************************************************************************
*/

void  BSP_Init (void)
{
  CPU_INT32U  i;

    SYSTEM.PRCR.WORD     = 0xA503;                              /* protect off                                          */
    SYSTEM.SOSCCR.BYTE   = 0x01;                                /* Dis Sub-Clock oscillation                            */
    SYSTEM.MOSCWTCR.BYTE = 0x0D;                                /* 131072 state                                         */
                                                                /* wait over 10ms  @12.5MHz                             */
    SYSTEM.PLLWTCR.BYTE  = 0x0F;                                /* 4194304 state (default)                              */
                                                                /* wait over 12ms  @PLL=200MHz(12.5MHz*16)              */
    
                                                                /* ----------------- PLL CONFIGURATION ---------------- */
                                                                /* ... PLIDIV = 1                                       */
                                                                /* ... STC    = 16                                      */
                                                                /* ... PLL    = (XTAL_FREQ * PLIDIV) * STC              */
                                                                /* ...        = (12000000 * 1) * 16                     */
                                                                /* ...        = 192Mhz                                  */
    SYSTEM.PLLCR.WORD = 0x0F00;
    SYSTEM.MOSCCR.BYTE = 0x00;                                  /* En main clock oscillation                            */    
    SYSTEM.PLLCR2.BYTE = 0x00;                                  /* En PLL oscillation                                   */
  
    for (i = 0; i < 0x168; i++) {                               /* wait over 12ms                                       */
    }
                                                                /* ---------------- SYSTEM CONFIGURATION -------------- */
                                                                /* ... ICK  = PLL * 1/2 = 96Mhz                         */
                                                                /* ... PCKB = PLL * 1/2 = 48Mhz                         */
                                                                /* ... BCK  = PLL * 1/4 = 24Mhz                         */
                                                                /* ... FCK  = PLL * 1/4 = 24Mhz                         */
    SYSTEM.SCKCR.LONG = 0x21020111;
    SYSTEM.SCKCR2.WORD = 0x0031;                                /* ... UCK  = PLL * 1/4 = 48Mhz                         */
    SYSTEM.SCKCR3.WORD = 0x0400;                                /* Switch to PLL clock                                  */

                                                                /* ---------------- USB PIN CONFIGURATION ------------- */
    PORT1.PDR.BIT.B4 = 0;                                       /* Clear USB0_DPUPE pin                                 */
    PORT1.PMR.BIT.B4 = 0;
    
    MPC.PWPR.BIT.B0WI  = 0;                                     /* En writing to PFS registers                          */
    MPC.PWPR.BIT.PFSWE = 1;
    MPC.P14PFS.BYTE = 0x11;                                     /* Select USB0_DPUPE as peripheral function             */
    MPC.PWPR.BIT.PFSWE = 0;                                     /* Dis writing to PFS registers                         */
    MPC.PWPR.BIT.B0WI  = 1;
    
    PORT1.PMR.BIT.B4 = 1;                                       /* Set pin as I/O port for peripheral function          */

    
    LED_Init();                                                 /* Initialize LEDs                                      */

}


/*$PAGE*/
/*
*********************************************************************************************************
*                                     PERIPHERAL CLOCK FREQUENCY
*
* Description: This function is used to retrieve peripheral clock frequency.
*
* Arguments  : none
*
* Return     : Peripheral clock frequency in cycles.
*********************************************************************************************************
*/

CPU_INT32U  BSP_CPU_PerClkFreq (void)
{
    CPU_INT32U  sys_clk;
    CPU_INT32U  per_clk;
    CPU_INT32U  per_div;


    sys_clk =  SYSTEM.SCKCR.LONG;
    per_div = (sys_clk >> 8u) & 0xFu;
    if (per_div > 3u) {
        return (0u);
    }

    per_clk =  BSP_CPU_EXT_CLK_FREQ << (3u - per_div);
    return (per_clk);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                       LED INITIALIZATION
*
* Description: This function is used to initialize the LEDs on the board.
*
* Arguments  : none
*********************************************************************************************************
*/

static  void  LED_Init (void)
{
    PORTD.PDR.BIT.B0 = 1;                                       /* Select port function.                                */
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

    LED_Off(0);                                                 /* Turn OFF all LEDs.                                   */
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                               LED ON
*
* Description: This function is used to control any or all the LEDs on the board.
*
* Arguments  : led    is the number of the LED to control
*                     0    indicates that you want ALL the LEDs to be ON
*                     1    turns ON LED1 on the board
*                     .
*                     .
*                     3    turns ON LED3 on the board
*********************************************************************************************************
*/

void  LED_On (CPU_INT08U  led)
{
    switch (led) {
        case 0:
            LED4  = LED_ON;
            LED5  = LED_ON;
            LED6  = LED_ON;
            LED7  = LED_ON;
            LED8  = LED_ON;
            LED9  = LED_ON;
            LED10 = LED_ON;
            LED11 = LED_ON;
            LED12 = LED_ON;
            LED13 = LED_ON;
            LED14 = LED_ON;
            LED15 = LED_ON;
            break;

        case 4:
            LED4  = LED_ON;
            break;

        case 5:
            LED5  = LED_ON;
            break;

        case 6:
            LED6  = LED_ON;
            break;

        case 7:
            LED7  = LED_ON;
            break;

        case 8:
            LED8  = LED_ON;
            break;

        case 9:
            LED9  = LED_ON;
            break;

        case 10:
            LED10 = LED_ON;
            break;

        case 11:
            LED11 = LED_ON;
            break;

        case 12:
            LED12 = LED_ON;
            break;

       case 13:
            LED13 = LED_ON;
            break;

       case 14:
            LED14 = LED_ON;
            break;

       case 15:
            LED15 = LED_ON;
            break;

       default:
            break;
    }
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                               LED OFF
*
* Description: This function is used to control any or all the LEDs on the board.
*
* Arguments  : led    is the number of the LED to turn OFF
*                     0    indicates that you want ALL the LEDs to be OFF
*                     1    turns OFF LED1 on the board
*                     .
*                     .
*                     3    turns OFF LED3 on the board
*********************************************************************************************************
*/

void  LED_Off (CPU_INT08U  led)
{
    switch (led) {
        case 0:
            LED4  = LED_OFF;
            LED5  = LED_OFF;
            LED6  = LED_OFF;
            LED7  = LED_OFF;
            LED8  = LED_OFF;
            LED9  = LED_OFF;
            LED10 = LED_OFF;
            LED11 = LED_OFF;
            LED12 = LED_OFF;
            LED13 = LED_OFF;
            LED14 = LED_OFF;
            LED15 = LED_OFF;
            break;

        case 4:
            LED4  = LED_OFF;
            break;

        case 5:
            LED5  = LED_OFF;
            break;

        case 6:
            LED6  = LED_OFF;
            break;

        case 7:
            LED7  = LED_OFF;
            break;

        case 8:
            LED8  = LED_OFF;
            break;

        case 9:
            LED9  = LED_OFF;
            break;

        case 10:
            LED10 = LED_OFF;
            break;

        case 11:
            LED11 = LED_OFF;
            break;

        case 12:
            LED12 = LED_OFF;
            break;

       case 13:
            LED13 = LED_OFF;
            break;

       case 14:
            LED14 = LED_OFF;
            break;

       case 15:
            LED15 = LED_OFF;
            break;

       default:
            break;
    }
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                             LED TOGGLE
*
* Description: This function is used to toggle the state of any or all the LEDs on the board.
*
* Arguments  : led    is the number of the LED to toggle
*                     0    indicates that you want ALL the LEDs to toggle
*                     1    Toggle LED1 on the board
*                     .
*                     .
*                     3    Toggle LED3 on the board
*********************************************************************************************************
*/

void  LED_Toggle (CPU_INT08U  led)
{
    switch (led) {
        case 0:
            LED4  = ~LED4;
            LED5  = ~LED5;
            LED6  = ~LED6;
            LED7  = ~LED7;
            LED8  = ~LED8;
            LED9  = ~LED9;
            LED10 = ~LED10;
            LED11 = ~LED11;
            LED12 = ~LED12;
            LED13 = ~LED13;
            LED14 = ~LED14;
            LED15 = ~LED15;
            break;

        case 4:
            LED4  = ~LED4;
            break;

        case 5:
            LED5  = ~LED5;
            break;

        case 6:
            LED6  = ~LED6;
            break;

        case 7:
            LED7  = ~LED7;
            break;

        case 8:
            LED8  = ~LED8;
            break;

        case 9:
            LED9  = ~LED9;
            break;

        case 10:
            LED10 = ~LED10;
            break;

        case 11:
            LED11 = ~LED11;
            break;

        case 12:
            LED12 = ~LED12;
            break;

       case 13:
            LED13 = ~LED13;
            break;

       case 14:
            LED14 = ~LED14;
            break;

       case 15:
            LED15 = ~LED15;
            break;

       default:
            break;
    }
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                          CPU_TS_TmrInit()
*
* Description : Initialize & start CPU timestamp timer.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : CPU_TS_Init().
*
*               This function is an INTERNAL CPU module function & MUST be implemented by application/
*               BSP function(s) [see Note #1] but MUST NOT be called by application function(s).
*
* Note(s)     : (1) CPU_TS_TmrInit() is an application/BSP function that MUST be defined by the developer 
*                   if either of the following CPU features is enabled :
*
*                   (a) CPU timestamps
*                   (b) CPU interrupts disabled time measurements
*
*                   See 'cpu_cfg.h  CPU TIMESTAMP CONFIGURATION  Note #1'
*                     & 'cpu_cfg.h  CPU INTERRUPTS DISABLED TIME MEASUREMENT CONFIGURATION  Note #1a'.
*
*               (2) (a) Timer count values MUST be returned via word-size-configurable 'CPU_TS_TMR' 
*                       data type.
*
*                       (1) If timer has more bits, truncate timer values' higher-order bits greater 
*                           than the configured 'CPU_TS_TMR' timestamp timer data type word size.
*
*                       (2) Since the timer MUST NOT have less bits than the configured 'CPU_TS_TMR' 
*                           timestamp timer data type word size; 'CPU_CFG_TS_TMR_SIZE' MUST be 
*                           configured so that ALL bits in 'CPU_TS_TMR' data type are significant.
*
*                           In other words, if timer size is not a binary-multiple of 8-bit octets 
*                           (e.g. 20-bits or even 24-bits), then the next lower, binary-multiple 
*                           octet word size SHOULD be configured (e.g. to 16-bits).  However, the 
*                           minimum supported word size for CPU timestamp timers is 8-bits.
*
*                       See also 'cpu_cfg.h   CPU TIMESTAMP CONFIGURATION  Note #2'
*                              & 'cpu_core.h  CPU TIMESTAMP DATA TYPES     Note #1'.
*
*                   (b) Timer SHOULD be an 'up'  counter whose values increase with each time count.
*
*                   (c) When applicable, timer period SHOULD be less than the typical measured time 
*                       but MUST be less than the maximum measured time; otherwise, timer resolution 
*                       inadequate to measure desired times.
*
*                   See also 'CPU_TS_TmrRd()  Note #2'.
*********************************************************************************************************
*/

#if (CPU_CFG_TS_TMR_EN == DEF_ENABLED)
void  CPU_TS_TmrInit (void)
{
    CPU_INT08U  cks;
    CPU_INT16U  n;
    CPU_INT32U  freq;


    MSTP(CMT1)          = 0;           /* enable the timer in the module stop register     */
    CMT.CMSTR0.BIT.STR1 = 0;           /* stop timer                                       */
    cks                 = 1;           /* set clock source select as follows:              */
    CMT1.CMCR.BIT.CKS   = cks;         /*    0  sets divider by   8                        */
                                       /*    1  sets divider by  32                        */
                                       /*    2  sets divider by 128                        */   
                                       /*    3  sets divider by 512                        */
    CMT1.CMCOR          = 0xFFFF;      /* compare match not used                           */
    CMT1.CMCNT          = 0;           /* clear counter register                           */
    CMT1.CMCR.BIT.CMIE  = 0;           /* disable compare match interrupt                  */
    CMT.CMSTR0.BIT.STR1 = 1;           /* start timer                                      */

    n    = 3 + ((cks & 3) << 1);       /* Set the count rate of the timestamp timer        */
    freq = BSP_CPU_PerClkFreq() >> n;

    CPU_TS_TmrFreqSet(freq);
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                           CPU_TS_TmrRd()
*
* Description : Get current CPU timestamp timer count value.
*
* Argument(s) : none.
*
* Return(s)   : Timestamp timer count (see Notes #2a & #2b).
*
* Caller(s)   : CPU_TS_Init(),
*               CPU_TS_Get32(),
*               CPU_TS_Get64(),
*               CPU_IntDisMeasStart(),
*               CPU_IntDisMeasStop().
*
*               This function is an INTERNAL CPU module function & MUST be implemented by application/
*               BSP function(s) [see Note #1] but SHOULD NOT be called by application function(s).
*
* Note(s)     : (1) CPU_TS_TmrRd() is an application/BSP function that MUST be defined by the developer 
*                   if either of the following CPU features is enabled :
*
*                   (a) CPU timestamps
*                   (b) CPU interrupts disabled time measurements
*
*                   See 'cpu_cfg.h  CPU TIMESTAMP CONFIGURATION  Note #1'
*                     & 'cpu_cfg.h  CPU INTERRUPTS DISABLED TIME MEASUREMENT CONFIGURATION  Note #1a'.
*
*               (2) (a) Timer count values MUST be returned via word-size-configurable 'CPU_TS_TMR' 
*                       data type.
*
*                       (1) If timer has more bits, truncate timer values' higher-order bits greater 
*                           than the configured 'CPU_TS_TMR' timestamp timer data type word size.
*
*                       (2) Since the timer MUST NOT have less bits than the configured 'CPU_TS_TMR' 
*                           timestamp timer data type word size; 'CPU_CFG_TS_TMR_SIZE' MUST be 
*                           configured so that ALL bits in 'CPU_TS_TMR' data type are significant.
*
*                           In other words, if timer size is not a binary-multiple of 8-bit octets 
*                           (e.g. 20-bits or even 24-bits), then the next lower, binary-multiple 
*                           octet word size SHOULD be configured (e.g. to 16-bits).  However, the 
*                           minimum supported word size for CPU timestamp timers is 8-bits.
*
*                       See also 'cpu_cfg.h   CPU TIMESTAMP CONFIGURATION  Note #2'
*                              & 'cpu_core.h  CPU TIMESTAMP DATA TYPES     Note #1'.
*
*                   (b) Timer SHOULD be an 'up'  counter whose values increase with each time count.
*
*                       (1) If timer is a 'down' counter whose values decrease with each time count,
*                           then the returned timer value MUST be ones-complemented.
*
*                   (c) (1) When applicable, the amount of time measured by CPU timestamps is 
*                           calculated by either of the following equations :
*
*                           (A) Time measured  =  Number timer counts  *  Timer period
*
*                                   where
*
*                                       Number timer counts     Number of timer counts measured 
*                                       Timer period            Timer's period in some units of 
*                                                                   (fractional) seconds
*                                       Time measured           Amount of time measured, in same 
*                                                                   units of (fractional) seconds 
*                                                                   as the Timer period
*
*                                                  Number timer counts
*                           (B) Time measured  =  ---------------------
*                                                    Timer frequency
*
*                                   where
*
*                                       Number timer counts     Number of timer counts measured
*                                       Timer frequency         Timer's frequency in some units 
*                                                                   of counts per second
*                                       Time measured           Amount of time measured, in seconds
*
*                       (2) Timer period SHOULD be less than the typical measured time but MUST be less 
*                           than the maximum measured time; otherwise, timer resolution inadequate to 
*                           measure desired times.
*********************************************************************************************************
*/

#if (CPU_CFG_TS_TMR_EN == DEF_ENABLED)
CPU_TS_TMR  CPU_TS_TmrRd (void)
{
    CPU_TS_TMR  ts_tmr_cnts;


    ts_tmr_cnts = (CPU_TS_TMR)CMT1.CMCNT;
    return (ts_tmr_cnts);
}
#endif
