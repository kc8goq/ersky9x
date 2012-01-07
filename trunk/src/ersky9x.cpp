/****************************************************************************
*  Copyright (c) 2011 by Michael Blandford. All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*  1. Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*  2. Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
*  3. Neither the name of the author nor the names of its contributors may
*     be used to endorse or promote products derived from this software
*     without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
*  THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
*  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
*  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
*  THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
*  SUCH DAMAGE.
*
****************************************************************************
*  History:
*
****************************************************************************/
#define __ERSKY9X_CPP__

#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "AT91SAM3S2.h"
#include "core_cm3.h"


#include "ersky9x.h"
#include "sound.h"
#include "lcd.h"
#include "myeeprom.h"
#include "s9xsplash.lbm"
#include "drivers.h"
#include "file.h"
#include "menus.h"


#include "debug.h"


#define TRUE	1
#define FALSE	0

//#define bool uint32_t

/*=========================================================================*/
/*  DEFINE: All Structures and Common Constants                            */
/*=========================================================================*/

/*=========================================================================*/
/*  DEFINE: Prototypes                                                     */
/*=========================================================================*/


extern uint32_t read_keys( void ) ;
extern uint32_t read_trims( void ) ;



void mainSequence( void ) ;
void perMain( void ) ;
void generalDefault( void ) ;
void modelDefault( uint8_t id ) ;
void UART_Configure( uint32_t baudrate, uint32_t masterClock) ;
void txmit( uint8_t c ) ;
void uputs( char *string ) ;
uint16_t rxuart( void ) ;
void start_timer2( void ) ;
void start_timer0( void ) ;
extern "C" void TC2_IRQHandler( void ) ;
extern "C" void sam_boot( void ) ;
void crlf( void ) ;
void p8hex( uint32_t value ) ;
void p4hex( uint16_t value ) ;
void p2hex( unsigned char c ) ;
void hex_digit_send( unsigned char c ) ;
void handle_serial( void ) ;
uint16_t anaIn( uint8_t chan ) ;
void getADC_single( void ) ;
void getADC_osmp( void ) ;
void getADC_filt( void ) ;
void read_8_adc( void ) ;
void init_adc( void ) ;
void init_pwm( void ) ;
//void poll_pwm( void ) ;
uint32_t hextoi( uint8_t *string ) ;
//uint32_t gets( uint8_t *string, uint32_t maxcount ) ;
void setup_switches( void ) ;
uint32_t read_trims( void ) ;
uint32_t read_switch( enum EnumKeys enuk ) ;
extern uint32_t read_keys( void ) ;
void hello( void ) ;
void dbl9x( void ) ;
//uint32_t get_switches( void ) ;
void config_free_pins( void ) ;

uint8_t checkTrim(uint8_t event) ;
void screen0( void ) ;

void putsTime(uint8_t x,uint8_t y,int16_t tme,uint8_t att,uint8_t att2) ;
void putsVolts(uint8_t x,uint8_t y, uint8_t volts, uint8_t att) ;
void putsVBat(uint8_t x,uint8_t y,uint8_t att) ;
void putsVBat(uint8_t x,uint8_t y,uint8_t att) ;
void putsChnRaw(uint8_t x,uint8_t y,uint8_t idx,uint8_t att) ;
void putsChn(uint8_t x,uint8_t y,uint8_t idx1,uint8_t att) ;
void putsDrSwitches(uint8_t x,uint8_t y,int8_t idx1,uint8_t att) ;//, bool nc) ;
const char *get_switches_string( void ) ;
bool getSwitch(int8_t swtch, bool nc, uint8_t level) ;
void perOut( int16_t *chanOut, uint8_t att ) ;



/*=========================================================================*/
/*  DEFINE: Definition of all local Data                                   */
/*=========================================================================*/

uint16_t Pulses[12] = {	2000, 2200, 2400, 2600, 2800, 3000, 3200, 3400, 9000, 0, 0, 0 } ;
volatile uint32_t Pulses_index = 0 ;		// Modified in interrupt routine



uint32_t Master_frequency ;
//uint16_t Adc_data[32] ;
volatile uint32_t Timer2_count ;		// Modified in interrupt routine
volatile uint32_t Tenms ;						// Modified in interrupt routine
volatile uint8_t tick10ms = 0 ;
uint16_t S_anaFilt[8] ;				// Analog inputs after filtering
uint16_t Volume ;

uint8_t sysFlags = 0 ;


uint32_t Lcd_analog_display ;
uint32_t Per10ms_action ;
uint32_t Permenu_action ;

int16_t g_ppmIns[8];
uint8_t ppmInState = 0; //0=unsync 1..8= wait for value i-1


EEGeneral  g_eeGeneral;
ModelData  g_model;

 const uint8_t chout_ar[] = { //First number is 0..23 -> template setup,  Second is relevant channel out
                                1,2,3,4 , 1,2,4,3 , 1,3,2,4 , 1,3,4,2 , 1,4,2,3 , 1,4,3,2,
                                2,1,3,4 , 2,1,4,3 , 2,3,1,4 , 2,3,4,1 , 2,4,1,3 , 2,4,3,1,
                                3,1,2,4 , 3,1,4,2 , 3,2,1,4 , 3,2,4,1 , 3,4,1,2 , 3,4,2,1,
                                4,1,2,3 , 4,1,3,2 , 4,2,1,3 , 4,2,3,1 , 4,3,1,2 , 4,3,2,1    };


const char modi12x3[]=
  "RUD ELE THR AIL "
  "RUD THR ELE AIL "
  "AIL ELE THR RUD "
  "AIL THR ELE RUD ";

MenuFuncP g_menuStack[5];

uint8_t  g_menuStackPtr = 0;


// Temporary to allow compile
uint8_t g_vbat100mV = 98 ;
//int16_t calibratedStick[7] ;
//int16_t g_chans512[NUM_CHNOUT] ;
uint8_t heartbeat ;

uint8_t Timer2_running = 0 ;
uint8_t Timer2_pre = 0 ;
uint16_t Timer2 = 0 ;



#define DO_SQUARE(xx,yy,ww)         \
{uint8_t x,y,w ; x = xx; y = yy; w = ww ; \
    lcd_vline(x-w/2,y-w/2,w);  \
    lcd_hline(x-w/2,y+w/2,w);  \
    lcd_vline(x+w/2,y-w/2,w);  \
    lcd_hline(x-w/2,y-w/2,w);}

/*
//#define DO_CROSS(xx,yy,ww)          \
//    lcd_vline(xx,yy-ww/2,ww);  \
//    lcd_hline(xx-ww/2,yy,ww);  \

//#define V_BAR(xx,yy,ll)       \
//    lcd_vline(xx-1,yy-ll,ll); \
//    lcd_vline(xx  ,yy-ll,ll); \
//    lcd_vline(xx+1,yy-ll,ll); \
	*/


/*=========================================================================*/
/*  DEFINE: All code exported                                              */
/*=========================================================================*/

/***************************************************************************/
/*  main                                                                   */
/***************************************************************************/

// extern uint32_t __RBIT(uint32_t value);


// WARNING check MATRIX->CCFG_SYSIO doesn't BLOCK I/O lines


// Still to test
// SPI to external flash eeprom

// PPM-in						PC23 TIOA3
// Sound out				PB14 DAC1
// SSC out					PA17 TD (sync PPM)
// Two-wire i/f			PA3, PA4
// EL backlight			PA0, PA1

// Serial ports
// PA5, PA6
// PB2, PB3


// Tested
// RF-power-in			PC17
// PPM-jack-in			PC19

// The stock Beeper is on PA16.


int main (void)
{
	register uint32_t i ;
	register Pio *pioptr ;
	// Debug variable
	uint32_t both_on ;

	WDT->WDT_MR = 0x3FFFAFFF ;			// Disable watchdog

	MATRIX->CCFG_SYSIO |= 0x000000F0L ;		// Disable syspins, enable B4,5,6,7

  PMC->PMC_PCER0 = 0x3900 ;				// Enable clocks to PIOB and PIOA and PIOC and UART0
	pioptr = PIOA ;
	pioptr->PIO_PER = 0x00200000L ;		// Enable bit A21 (EXT3)
	pioptr->PIO_OER = 0x00200000L ;		// Set bit A21 as output
	pioptr->PIO_SODR = 0x00200000L ;	// Set bit A21 ON

	pioptr->PIO_PUER = 0x80000000 ;		// Enable pullup on bit A31 (EXIT)
	pioptr->PIO_PER = 0x80000000 ;		// Enable bit A31

	pioptr = PIOC ;
	pioptr->PIO_PER = 0x82000000L ;		// Enable bit C31 (EXT1), C25 (USB-detect)
	pioptr->PIO_OER = 0x80000000L ;		// Set bits C18 and C31 as output
	pioptr->PIO_SODR = 0x80000000L ;	// Set bit C31

	// Configure RF_power (PC17) and PPM-jack-in (PC19), neither need pullups
	pioptr->PIO_PER = 0x000A0000L ;		// Enable bit C19, C17
	pioptr->PIO_ODR = 0x000A0000L ;		// Set bits C19 and C17 as input

	config_free_pins() ;

	// Next section configures the key inputs on the LCD data
	pioptr->PIO_PER = 0x0000003DL ;		// Enable bits 2,3,4,5, 0
	pioptr->PIO_OER = 0x00000001L ;		// Set bit 0 output
	pioptr->PIO_ODR = 0x0000003CL ;		// Set bits 2, 3, 4, 5 input
	pioptr->PIO_PUER = 0x0000003CL ;		// Set bits 2, 3, 4, 5 with pullups

	pioptr = PIOB ;
	pioptr->PIO_PUER = 0x40 ;					// Enable pullup on bit B6 (MENU)
	pioptr->PIO_PER = 0x40 ;					// Enable bit B6

	setup_switches() ;

  // Enable PCK2 on PB3, This is for testing of Timer 2 working
	// It will be used as serial data to the Bluetooth module
	pioptr->PIO_ABCDSR[0] |=  0x00000008 ;	// Peripheral B
  pioptr->PIO_ABCDSR[1] &= ~0x00000008 ;	// Peripheral B
  pioptr->PIO_PDR = 0x00000008 ;					// Assign to peripheral
	PMC->PMC_SCER |= 0x0400 ;								// PCK2 enabled
	PMC->PMC_PCK[2] = 2 ;										// PCK2 is PLLA

	UART_Configure( 9600, Master_frequency ) ;
	start_timer2() ;
	start_timer0() ;
	init_adc() ;
	init_pwm() ;

	__enable_irq() ;

  g_menuStack[0] =  menuProc0 ;

	start_sound() ;
	
	init_spi() ;
		
	lcd_init() ;		
//	lcd_putsn_P( 5*FW, 0, "ERSKY9X", 7 ) ;
//	lcd_putsn_P( 13*FW, 0, VERSION, sizeof( VERSION )-1 ) ;
	
//	refreshDisplay() ;

	set_volume( Volume = 2 ) ;

	i = Timer2_count ;

	txmit( 'E' ) ;
	crlf() ;

	init_eeprom() ;	
	 
	eeReadAll() ;
//	generalDefault() ;
//	modelDefault( 0 ) ;

//  pushMenu(menuProcModelSelect);
  pushMenu(menuProc0) ;
  popMenu(true);  // this is so the first instance of [MENU LONG] doesn't freak out!

	lcd_clear() ;
	lcd_putsn_P( 5*FW, 0, "ERSKY9X", 7 ) ;
	lcd_putsn_P( 13*FW, 0, VERSION, sizeof( VERSION )-1 ) ;
	
	refreshDisplay() ;
	start_ppm_capture() ;

	both_on = 0 ;
  while (1)
  {
//	  PMC->PMC_PCER0 = 0x1800 ;				// Enable clocks to PIOB and PIOA
//		PIOA->PIO_PER = 0x04200000L ;		// Enable bit A21 (EXT3), A26 (EXT2)
//		PIOA->PIO_OER = 0x04200000L ;		// Set bits A21, A26 as output

//		PIOB->PIO_PUER = 0x40 ;					// Enable pullup on bit B6
//		PIOB->PIO_PER = 0x40 ;					// Enable bit B6
//		PIOA->PIO_PUER = 0x80000000 ;		// Enable pullup on bit A31 (EXIT)
//		PIOA->PIO_PER = 0x80000000 ;		// Enable bit A31

		// EXT3 controlled by MENU key for testing/debug
		if ( PIOB->PIO_PDSR & 0x40 )
		{
			PIOA->PIO_SODR = 0x00200000L ;	// Set bit A21 ON
			both_on = 0 ;
		}
		else
		{
			PIOA->PIO_CODR = 0x00200000L ;	// Clear bit A21 OFF
			if ( ( PIOA->PIO_PDSR & 0x80000000L ) == 0 )	// EXIT and MENU ON together
			{
				if ( ! both_on )
				{
					both_on = 1 ;
					if ( Per10ms_action )
					{
						Per10ms_action = 0 ;			
					}
					else
					{
						Per10ms_action = 1 ;
					}
					if ( Permenu_action )
					{
						Permenu_action = 0 ;			
					}
					else
					{
						Permenu_action = 1 ;
					}
				}
			}
			else
			{
				both_on = 0 ;
			}
		}


		// EXT2 (A26) driven by Timer 2 TIOA
		if ( i != Timer2_count )
		{
			i = Timer2_count ;
			if ( Lcd_analog_display )
			{
				register uint32_t j ;

//				read_8_adc() ;
				getADC_osmp() ;
				lcd_clear() ;

    		for( j=0; j<8; j++)
  	  	{
      	  uint8_t y=j*FH;
      	  lcd_putc( 4*FW, y, 'A' ) ;
      	  lcd_putc( 5*FW, y, '1'+j ) ;
      	  //        lcd_putsn_P( 4*FW, y,PSTR("A1A2A3A4A5A6A7A8")+2*i,2);
      	  lcd_outhex4( 7*FW, y,Analog_values[j]);
      	  lcd_outhex4( 13*FW, y,S_anaFilt[j]);
	    	}
				refreshDisplay() ;
			}
		}
		
		if ( Per10ms_action )
		{
			if ( Tenms )
			{
				Tenms = 0 ;
				 per10ms() ;				
			}
			if ( Permenu_action == 0 )
			{
				lcd_clear() ;
				screen0() ;
				refreshDisplay() ;
			}
		}

		pioptr = PIOC ;
		// Toggle bits BACKLIGHT and EXT1, Backlight now on PWM
//		if ( i & 1 )
//		{
//			pioptr->PIO_CODR = 0x00040000L ;	// Set bit C18 OFF
//		}
//		else
//		{
//			pioptr->PIO_SODR = 0x00040000L ;	// Set bit C18 ON
//		}

		if ( Timer2_count & 4 )
		{
			pioptr->PIO_SODR = 0x80000000L ;	// Set bit C31 ON
		}
		else
		{
			pioptr->PIO_CODR = 0x80000000L ;	// Set bit C31 OFF
		}



		handle_serial() ;

		if ( pioptr->PIO_PDSR & 0x02000000 )
		{
			// Detected USB
			break ;
		}
  
		// output a sinewave, untimed
//		DACC->DACC_CDR = Sine_values[Sine_index++] ;		// Sinewave output
//		if ( Sine_index >= 100 )
//		{
//			Sine_index = 0 ;			
//		}

		mainSequence() ;

	}

	lcd_clear() ;
	lcd_putcAtt( 48, 24, 'U', DBLSIZE ) ;
	lcd_putcAtt( 60, 24, 'S', DBLSIZE ) ;
	lcd_putcAtt( 72, 24, 'B', DBLSIZE ) ;
	refreshDisplay() ;

	// This might be replaced by a software reset
	// Any interrupts that have been enabled must be disabled here
	// BEFORE calling sam_boot()
	end_ppm_capture() ;
	end_spi() ;
	end_sound() ;
	TC0->TC_CHANNEL[2].TC_IDR = TC_IDR0_CPCS ;
	NVIC_DisableIRQ(TC2_IRQn) ;
//	PWM->PWM_IDR1 = PWM_IDR1_CHID0 ;
	PWM->PWM_IDR1 = PWM_IDR1_CHID3 ;
	NVIC_DisableIRQ(PWM_IRQn) ;
	sam_boot() ;

  /*
   * Prevent compiler warnings
   */
//  (void)c;

  /*
   * This return here make no sense.
   * But to prevent the compiler warning:
   * "return type of 'main' is not 'int'
   * we use an int as return :-)
   */
  return(0);
}



void mainSequence()
{
//      uint16_t t0 = getTmr16KHz();
	getADC_osmp() ;	//      getADC[g_eeGeneral.filterInput]();
//      ADMUX=0x1E|ADC_VREF_TYPE;   // Select bandgap
	perMain();      // Give bandgap plenty of time to settle
//      getADC_bandgap() ;
//      //while(get_tmr10ms()==old10ms) sleep_mode();
//      if(heartbeat == 0x3)
//      {
//          wdt_reset();
//          heartbeat = 0;
//      }
//      t0 = getTmr16KHz() - t0;
//      if ( t0 > g_timeMain ) g_timeMain = t0 ;


//#ifdef FRSKY
//			if ( FrskyAlarmCheckFlag )
//			{
//				FrskyAlarmCheckFlag = 0 ;
//				// Check for alarms here
//				// Including Altitude limit
//			}
//#endif
}

void perMain()
{
  static uint16_t lastTMR;
  tick10ms = (get_tmr10ms() != lastTMR);
  lastTMR = get_tmr10ms();

    perOut(g_chans512, 0);
  if(!tick10ms) return ; //make sure the rest happen only every 10ms.

  //  if ( Timer2_running )
  if ( Timer2_running & 1)  // ignore throttle started flag
  {
    if ( (Timer2_pre += 1 ) >= 100 )
    {
      Timer2_pre -= 100 ;
      Timer2 += 1 ;
    }
  }

  eeCheck();

  lcd_clear();
  uint8_t evt=getEvent();
  evt = checkTrim(evt);

//    if(g_LightOffCounter) g_LightOffCounter--;
//    if(evt) g_LightOffCounter = g_eeGeneral.lightAutoOff*500; // on keypress turn the light on 5*100

//    if(getSwitch(g_eeGeneral.lightSw,0) || g_LightOffCounter)
//        BACKLIGHT_ON;
//    else
//        BACKLIGHT_OFF;


//    static int16_t p1valprev;
//    p1valdiff = (p1val-calibratedStick[6])/32;
//    if(p1valdiff) {
//        p1valdiff = (p1valprev-calibratedStick[6])/2;
//        p1val = calibratedStick[6];
//    }
//    p1valprev = calibratedStick[6];
//   if ( g_eeGeneral.disablePotScroll )
//   {
//      p1valdiff = 0 ;			
//   	}

	if ( Permenu_action )
	{
    g_menuStack[g_menuStackPtr](evt);
//		menuProcStatistic2(evt) ;		
    refreshDisplay();
	}


//    if(checkSlaveMode()) {
//        PORTG &= ~(1<<OUT_G_SIM_CTL); // 0=ppm out
//    }else{
//        PORTG |=  (1<<OUT_G_SIM_CTL); // 1=ppm-in
//    }

		


  switch( get_tmr10ms() & 0x1f )
	{ //alle 10ms*32

    case 2:
//        //check v-bat
////        Calculation By Mike Blandford
////        Resistor divide on battery voltage is 5K1 and 2K7 giving a fraction of 2.7/7.8
////        If battery voltage = 10V then A2D voltage = 3.462V
////        11 bit A2D count is 1417 (3.462/5*2048).
////        1417*18/256 = 99 (actually 99.6) to represent 9.9 volts.
////        Erring on the side of low is probably best.

        int32_t ab = anaIn(7);
        ab = ( ab + ab*(g_eeGeneral.vBatCalib)/128 ) * 4191 ;
//        ab = (uint16_t) ab / (g_eeGeneral.disableBG ? 240 : BandGap ) ;  // ab might be more than 32767
        ab /= 55296  ;
        g_vbat100mV = (ab + g_vbat100mV + 1) >> 1 ;  // Filter it a bit => more stable display

//        static uint8_t s_batCheck;
//        s_batCheck+=32;
//        if((s_batCheck==0) && (g_vbat100mV<g_eeGeneral.vBatWarn) && (g_vbat100mV>49)){

//            audioDefevent(AUDIO_TX_BATTERY_LOW);
//            if (g_eeGeneral.flashBeep) g_LightOffCounter = FLASH_DURATION;
//        }
    break ;

//    case 3:
//    {
//    	/*
//        static prog_uint8_t APM beepTab[]= {
//            // 0   1   2   3    4
//            0,  0,  0,  0,   0, //quiet
//            0,  1,  8, 30, 100, //silent
//            1,  1,  8, 30, 100, //normal
//            1,  1, 15, 50, 150, //for motor
//            10, 10, 30, 50, 150, //for motor
//        };
//        memcpy_P(g_beepVal,beepTab+5*g_eeGeneral.beeperVal,5);
//        //g_beepVal = BEEP_VAL;
//        */
//        /* all this gone and replaced in new sound system */
//    }
//        break;
  }

}

//void generalDefault()
//{
//	register int i ;
//  register int16_t sum=0 ;

//  memset(&g_eeGeneral,0,sizeof(g_eeGeneral));
//  g_eeGeneral.myVers   =  MDVERS;
//  g_eeGeneral.currModel=  0 ;
//  g_eeGeneral.contrast = 25 ;
//  g_eeGeneral.vBatWarn = 65 ;
//  g_eeGeneral.stickMode=  1 ;
//  for (int i = 0; i < 7; ++i) {
//    g_eeGeneral.calibMid[i]     = 0x200;
//    g_eeGeneral.calibSpanNeg[i] = 0x180;
//    g_eeGeneral.calibSpanPos[i] = 0x180;
//  }
//  strcpy(g_eeGeneral.ownerName,"ME        ");
//  for( i=0; i<12;i++) sum+=g_eeGeneral.calibMid[i];
//  g_eeGeneral.chkSum = sum;
//}

//void modelDefault(uint8_t id)
//{
//  memset(&g_model, 0, sizeof(ModelData));
//  strcpy(g_model.name,"MODEL     ");
//  g_model.name[5]='0'+(id+1)/10;
//  g_model.name[6]='0'+(id+1)%10;
//  g_model.mdVers = MDVERS;

////  applyTemplate(0); //default 4 channel template
//}



// Test, starts TIMER2 at 100Hz, drives TIOA2 (A26, EXT2) out
void start_timer2()
{
	register Pio *pioptr ;
  register Tc *ptc ;
	register uint32_t timer ;

	// Enable peripheral clock TC0 = bit 23 thru TC5 = bit 28
  PMC->PMC_PCER0 |= 0x02000000L ;		// Enable peripheral clock to TC2

	pioptr = PIOA ;
	timer = Master_frequency / 12800 ;		// MCK/128 and 100 Hz

  ptc = TC0 ;		// Tc block 0 (TC0-2)
	ptc->TC_BCR = 0 ;			// No sync
	ptc->TC_BMR = 0 ;
	ptc->TC_CHANNEL[2].TC_CMR = 0x00008000 ;	// Waveform mode
	ptc->TC_CHANNEL[2].TC_RC = timer ;			// 10 Hz
	ptc->TC_CHANNEL[2].TC_RA = timer >> 1 ;
	ptc->TC_CHANNEL[2].TC_CMR = 0x0009C003 ;	// 0000 0000 0000 1001 1100 0000 0000 0011
																						// MCK/128, set @ RA, Clear @ RC waveform

  pioptr->PIO_ABCDSR[0] |= 0x04000000 ;		// Peripheral B = TIOA2
  pioptr->PIO_ABCDSR[1] &= ~0x04000000 ;	// Peripheral B
	pioptr->PIO_PDR = 0x04000000L ;		// Disable bit A26 (EXT2) Assign to peripheral
	ptc->TC_CHANNEL[2].TC_CCR = 5 ;		// Enable clock and trigger it (may only need trigger)
	
	NVIC_EnableIRQ(TC2_IRQn) ;
	TC0->TC_CHANNEL[2].TC_IER = TC_IER0_CPCS ;

}


// Test, starts TIMER0 at full speed (MCK/2) for delay timing
void start_timer0()
{
  register Tc *ptc ;

	// Enable peripheral clock TC0 = bit 23 thru TC5 = bit 28
  PMC->PMC_PCER0 |= 0x00800000L ;		// Enable peripheral clock to TC0

  ptc = TC0 ;		// Tc block 0 (TC0-2)
	ptc->TC_BCR = 0 ;			// No sync
	ptc->TC_BMR = 2 ;
	ptc->TC_CHANNEL[0].TC_CMR = 0x00008000 ;	// Waveform mode
	ptc->TC_CHANNEL[0].TC_RC = 0xFFF0 ;
	ptc->TC_CHANNEL[0].TC_RA = 0 ;
	ptc->TC_CHANNEL[0].TC_CMR = 0x00008040 ;	// 0000 0000 0000 0000 1000 0000 0100 0000, stop at regC
	ptc->TC_CHANNEL[0].TC_CCR = 5 ;		// Enable clock and trigger it (may only need trigger)
}

extern "C" void TC2_IRQHandler()
{
  register uint32_t dummy;
	static uint32_t pre_scale ;		// Used to get 10 Hz counter

  /* Clear status bit to acknowledge interrupt */
  dummy = TC0->TC_CHANNEL[2].TC_SR;
	(void) dummy ;		// Discard value - prevents compiler warning
	Tenms |= 1 ;			// 10 mS has passed

	if ( ++pre_scale >= 10 )
	{
		Timer2_count += 1 ;
		pre_scale = 0 ;
	}
}




// ADC channels are assigned to:
// AD1  stick_RH
// AD2  stick_LH
// AD3  PIT_TRIM
// AD4  battery
// AD5  HOV_PIT
// AD9  stick_LV
// AD13 HOV_THR
// AD14 stick_RV
// Peripheral ID 29 (0x20000000)
// Note ADC sequencing won't work as it only operates on channels 0-7
//      and we need 9, 13 and 14 as well
// ALSO: Errata says only one channel converted for each trigger

// Needed implementation (11 bit result)
// SINGLE - 1 read then >> 1
// OVERSAMPLE - 4 reads - average = sum then >> 3
// FILTERED - 1 read but filter processing

// Filtering algorithm
// o/p = half previous o/p + temp1
// temp1 = average temp1 and temp0
// temp0 = average new reading and temp0

uint16_t anaIn(uint8_t chan)
{
  static uint8_t crossAna[]={1,5,7,0,4,6,2,3};
  volatile uint16_t *p = &S_anaFilt[crossAna[chan]] ;
  return  *p;
}


void getADC_single()
{
	register uint32_t x ;

	read_8_adc() ;

	for( x = 0 ; x < 8 ; x += 1 )
	{
		S_anaFilt[x] = Analog_values[x] >> 1 ;
	}
}


void getADC_osmp()
{
	register uint32_t x ;
	register uint32_t y ;
	uint16_t temp[8] ;

	for( x = 0 ; x < 8 ; x += 1 )
	{
		temp[x] = 0 ;
	}
	for( y = 0 ; y < 4 ; y += 1 )
	{
		read_8_adc() ;
		for( x = 0 ; x < 8 ; x += 1 )
		{
			temp[x] += Analog_values[x] ;
		}
	}
	for( x = 0 ; x < 8 ; x += 1 )
	{
		S_anaFilt[x] = temp[x] >> 3 ;
	}
}


void getADC_filt()
{
	register uint32_t x ;
	static uint16_t t_ana[2][8] ;

	read_8_adc() ;
	for( x = 0 ; x < 8 ; x += 1 )
	{
		S_anaFilt[x] = S_anaFilt[x]/2 + (t_ana[1][x] >> 1 ) ;
		t_ana[1][x] = ( t_ana[1][x] + t_ana[0][x] ) >> 1 ;
		t_ana[0][x] = ( t_ana[0][x] + Analog_values[x] ) >> 1 ;
	}	 
}


// PWM used for PPM generation, and LED Backlight
// Output pin PB5 not used, PA17 used as PWMH3 peripheral C
// PWM peripheral ID = 31 (0x80000000)
// Ensure PB5 is three state/input

// configure PWM3 as PPM drive, PWM0 will become LED backlight PWM on PWMH0
// This is PC18 peripheral B

void init_pwm()
{
	register Pio *pioptr ;
	register Pwm *pwmptr ;
	register uint32_t timer ;

  PMC->PMC_PCER0 |= 0x80000000L ;		// Enable peripheral clock to PWM
  
	MATRIX->CCFG_SYSIO |= 0x00000020L ;				// Disable TDO let PB5 work!
	
	/* Configure PIO */
	pioptr = PIOB ;
	pioptr->PIO_PER = 0x00000020L ;		// Enable bit B5
	pioptr->PIO_ODR = 0x00000020L ;		// set as input

	pioptr = PIOA ;
  pioptr->PIO_ABCDSR[0] &= ~0x00020000 ;		// Peripheral C
  pioptr->PIO_ABCDSR[1] |= 0x00020000 ;			// Peripheral C
	pioptr->PIO_PDR = 0x00020000L ;						// Disable bit A17 Assign to peripheral

	pioptr = PIOC ;
  pioptr->PIO_ABCDSR[0] |= 0x00040000 ;			// Peripheral B
  pioptr->PIO_ABCDSR[1] &= ~0x00040000 ;		// Peripheral B
	pioptr->PIO_PDR = 0x00040000L ;						// Disable bit C18 Assign to peripheral

	// Configure clock - depends on MCK frequency
	timer = Master_frequency / 2000000 ;
	timer |= ( Master_frequency / ( 32* 10000 ) ) << 16 ;
	timer &= 0x00FF00FF ;

	pwmptr = PWM ;
	pwmptr->PWM_CLK = 0x05000000 | timer ;	// MCK for DIVA, DIVA = 18 gives 0.5uS clock period @35MHz
																		// MCK/32 / timer = 10000Hz for CLKB
	
	// PWM0 for LED backlight
	pwmptr->PWM_CH_NUM[0].PWM_CMR = 0x0000000C ;	// CLKB
	pwmptr->PWM_CH_NUM[0].PWM_CPDR = 100 ;			// Period
	pwmptr->PWM_CH_NUM[0].PWM_CPDRUPD = 100 ;		// Period
	pwmptr->PWM_CH_NUM[0].PWM_CDTY = 40 ;				// Duty
	pwmptr->PWM_CH_NUM[0].PWM_CDTYUPD = 40 ;		// Duty
	pwmptr->PWM_ENA = PWM_ENA_CHID0 ;						// Enable channel 0

	// PWM3 for PPM output	 
	pwmptr->PWM_CH_NUM[3].PWM_CMR = 0x0000000B ;	// CLKA
	pwmptr->PWM_CH_NUM[3].PWM_CPDR = 3000 ;			// Period in half uS
	pwmptr->PWM_CH_NUM[3].PWM_CPDRUPD = 3000 ;	// Period in half uS
	pwmptr->PWM_CH_NUM[3].PWM_CDTY = 400 ;			// Duty in half uS
	pwmptr->PWM_CH_NUM[3].PWM_CDTYUPD = 400 ;		// Duty in half uS
	pwmptr->PWM_ENA = PWM_ENA_CHID3 ;						// Enable channel 3

	NVIC_EnableIRQ(PWM_IRQn) ;
//	pwmptr->PWM_IER1 = PWM_IER1_CHID0 ;
	pwmptr->PWM_IER1 = PWM_IER1_CHID3 ;


}

extern "C" void PWM_IRQHandler (void)
{
	if ( PWM->PWM_ISR1 & PWM_ISR1_CHID3 )
	{
		PWM->PWM_CH_NUM[3].PWM_CPDRUPD = Pulses[Pulses_index++] ;	// Period in half uS
		if ( Pulses[Pulses_index] == 0 )
		{
			Pulses_index = 0 ;
		}
	}
}


//void poll_pwm()
//{
//	if ( PWM->PWM_ISR1 & PWM_ISR1_CHID3 )
//	{
//		PWM->PWM_CH_NUM[3].PWM_CPDRUPD = Pulses[Pulses_index++] ;	// Period in half uS
//		if ( Pulses[Pulses_index] == 0 )
//		{
//			Pulses_index = 0 ;
//		}
//	}
//}




/*-------------------------------------------------------------------*/
/* Convert hex string to internal long value */

uint32_t hextoi( uint8_t *string )
{
	register uint8_t *p ;
  register uint32_t value ;
  register uint8_t c ;

  p = string ;
  value = 0 ;
  while( ( c = toupper( *p ) ) != 0 )
  {
    if ( c < '0' || c > 'F' || ( c < 'A' && c > '9') )
    {
      break ;
    }
    else
    {
      value <<= 4 ;
      c -= (c > '9') ? '7' : '0' ;
      value += c ;
    }
    p++ ;
  }
  if ( p == string )
  {
    value = 0xFFFFFFFF ;
  }
  return value ;
}

/*-------------------------------------------------------------------*/
/* Get a string from the serial port into *string */

//uint32_t gets( register uint8_t *string, register uint32_t maxcount )
//{
//  register uint32_t count ;
//  register uint16_t c ;

//  count = 0 ;
//  c = '\0' ;
//  while ( ( c != '\r' ) && ( count < maxcount-2 ) )
//	{
//		while ( ( c = rxuart() ) == 0xFFFF )
//		{
//			/* Null body, wait for char */
//		}
//    switch (c)
//    {
//      case '\r' :
//			break ;

//			case '\010' :
//      case 0x7F :
//				if (count > 0)
//				{
//				  txmit('\010') ;
//				  txmit(' ') ;
//				  txmit('\010') ;
//				  count -= 1 ;
//				  string -= 1 ;
//				}
//			break ;

//			case '\t' :
//      case ' '  :
//				c = ' ' ;
//				if (count > 0)
//				{
//				  txmit(c) ;
//				  *string++ = c ;
//				  count += 1 ;
//				}
//			break ;

//      default :
//				if (c < ' ')
//				{
//					if (c != 10 )
//					{
//					  txmit('\007') ;
//					}
//				}
//				else
//				{
//					txmit( c ) ;
//				  *string++ = c ;
//				  count++ ;
//				}
//			break ;
//    }
//  }
//  *string = '\0';
//  return count ;
//}


// Switch input pins
// AIL-DR  PA2
// TRIM_LH_UP PA7
// ELE_DR   PA8
// RUN_DR   PA15
// TRIM_LV_DOWN  PA27
// SW_TCUT     PA28
// TRIM_RH_DOWN    PA29
// TRIM_RV_UP    PA30
// TRIM_LH_DOWN    PB4
// SW-TRAIN    PC8
// TRIM_RH_UP   PC9
// TRIM_RV_DOWN   PC10
// SW_IDL2     PC11
// SW_IDL1     PC14
// SW_GEAR     PC16
// TRIM_LV_UP   PC28

// KEY_MENU    PB6
// KEY_EXIT    PA31
// Shared with LCD data
// KEY_DOWN  LCD5  PC3
// KEY_UP    LCD6  PC2
// KEY_RIGHT LCD4  PC4
// KEY_LEFT  LCD3  PC5

// PORTA 0111 1000 0000 0000 1000 0001 1000 0100 = 0x78008184
// PORTB 0000 0000 0001 0000										 = 0x0010
// PORTC 0001 0000 0000 0001 0100 1001 0000 0000 = 0x10014900


// Assumes PMC has already enabled clocks to ports
void setup_switches()
{
	register Pio *pioptr ;

	pioptr = PIOA ;
	pioptr->PIO_PER = 0x78008184 ;		// Enable bits
	pioptr->PIO_ODR = 0x78008184 ;		// Set bits input
	pioptr->PIO_PUER = 0x78008184 ;		// Set bits with pullups

	pioptr = PIOB ;
	pioptr->PIO_PER = 0x00000010 ;		// Enable bits
	pioptr->PIO_ODR = 0x00000010 ;		// Set bits input
	pioptr->PIO_PUER = 0x00000010 ;		// Set bits with pullups

	pioptr = PIOC ;
	pioptr->PIO_PER = 0x10014900 ;		// Enable bits
	pioptr->PIO_ODR = 0x10014900 ;		// Set bits input
	pioptr->PIO_PUER = 0x10014900 ;		// Set bits with pullups

}




//uint32_t get_switches()
//{
//	uint32_t x ;

//	x = 0 ; 
//	if ( read_switch( SW_ElevDR ) )
//	{
//		x |= 1 ;
//	}
//	if ( read_switch( SW_AileDR ) )
//	{
//		x |= 2 ;
//	}


//// Need to add more here


//	return x ;
//}

// Free pins
// PA16, PA23, PA24, PA25, PB7, PB13
// PC20, PC21(labelled 17), PC22, PC24
void config_free_pins()
{
	register Pio *pioptr ;
	
	pioptr = PIOA ;
	pioptr->PIO_PER = 0x03810000L ;		// Enable bits A25,24,23,16
	pioptr->PIO_ODR = 0x03810000L ;		// Set as input
	pioptr->PIO_PUER = 0x03810000L ;	// Enable pullups

	pioptr = PIOB ;
	pioptr->PIO_PER = 0x00002080L ;		// Enable bits B13, 7
	pioptr->PIO_ODR = 0x00002080L ;		// Set as input
	pioptr->PIO_PUER = 0x00002080L ;	// Enable pullups

	pioptr = PIOC ;
	pioptr->PIO_PER = 0x01700000L ;		// Enable bits C24,22,21,20
	pioptr->PIO_ODR = 0x01700000L ;		// Set as input
	pioptr->PIO_PUER = 0x01700000L ;	// Enable pullups
}


int8_t *TrimPtr[4] = 
{
  &g_model.trim[0],
  &g_model.trim[1],
  &g_model.trim[2],
  &g_model.trim[3]
} ;

uint8_t checkTrim(uint8_t event)
{
  int8_t  k = (event & EVT_KEY_MASK) - TRM_BASE;
  int8_t  s = g_model.trimInc;
  if (s>1) s = 1 << (s-1);  // 1=>1  2=>2  3=>4  4=>8

  if((k>=0) && (k<8))// && (event & _MSK_KEY_REPT))
  {
    //LH_DWN LH_UP LV_DWN LV_UP RV_DWN RV_UP RH_DWN RH_UP
    uint8_t idx = k/2;
    int8_t tm = *TrimPtr[idx] ;
    int8_t  v = (s==0) ? (abs(tm)/4)+1 : s;
    bool thrChan = ((2-(g_eeGeneral.stickMode&1)) == idx);
    bool thro = (thrChan && (g_model.thrTrim));
    if(thro) v = 4; // if throttle trim and trim trottle then step=4
    if(thrChan && g_eeGeneral.throttleReversed) v = -v;  // throttle reversed = trim reversed
    int16_t x = (k&1) ? tm + v : tm - v;   // positive = k&1

    if(((x==0)  ||  ((x>=0) != (tm>=0))) && (!thro) && (tm!=0)){
      *TrimPtr[idx]=0;
      killEvents(event);
//      audioDefevent(AUDIO_TRIM_MIDDLE);

    } else if(x>-125 && x<125){
      *TrimPtr[idx] = (int8_t)x;
//      STORE_MODELVARS_TRIM;
      //if(event & _MSK_KEY_REPT) warble = true;
//			if(x <= 125 && x >= -125){
//				audio.event(AUDIO_TRIM_MOVE,(abs(x)/4)+60);
//			}	
    }
    else
    {
      *TrimPtr[idx] = (x>0) ? 125 : -125;
//      STORE_MODELVARS_TRIM;
//			if(x <= 125 && x >= -125){
//				audio.event(AUDIO_TRIM_MOVE,(-abs(x)/4)+60);
//			}	
    }

    return 0;
  }
  return event;
}


void screen0()
{
  register  uint8_t x=FW*2;
  register uint8_t att = (g_vbat100mV < g_eeGeneral.vBatWarn ? BLINK : 0) | DBLSIZE;
  register uint32_t i ;

	for( i=0 ; i<sizeof(g_model.name);i++)
	{
    lcd_putcAtt(x+i*2*FW-i-2, 0*FH, g_model.name[i],DBLSIZE);
//    lcd_putcAtt( x+i*2*FW-i-2, 0*FH, "MODEL1   "[i], DBLSIZE ) ;
	}

  putsVBat(x+4*FW, 2*FH, att|NO_UNIT ) ;
  lcd_putc( x+4*FW, 3*FH, 'V' ) ;

//  if(s_timerState != TMR_OFF)
//	{
//      uint8_t att = DBLSIZE | (s_timerState==TMR_BEEPING ? BLINK : 0);
//      putsTime(x+14*FW-2, FH*2, s_timerVal, att,att);
//      putsTmrMode(x+7*FW-FW/2,FH*3,0);
//  }

  lcd_putsnAtt(x+4*FW,     2*FH,PSTR("ExpExFFneMedCrs")+3*g_model.trimInc,3, 0);
  lcd_putsnAtt(x+8*FW-FW/2,2*FH,PSTR("   TTm")+3*g_model.thrTrim,3, 0);
//  lcd_putsnAtt(x+4*FW,     2*FH,PSTR("ExpExFFneMedCrs")+3*1,3, 0);
//  lcd_putsnAtt(x+8*FW-FW/2,2*FH,PSTR("   TTm")+3*1,3, 0);

  //trim sliders
  for( i=0 ; i<4 ; i++ )
  {
#define TL 27
    //                        LH LV RV RH
    static uint8_t x[4]    = {128*1/4+2, 4, 128-4, 128*3/4-2};
    static uint8_t vert[4] = {0,1,1,0};
    register uint8_t xm, ym ;
    xm=x[i] ;
    register int8_t val = max((int8_t)-(TL+1),min((int8_t)(TL+1),(int8_t)(*TrimPtr[i]/4)));
//		register int8_t val = 0 ;
    if(vert[i])
		{
      ym=31;
      lcd_vline(xm,   ym-TL, TL*2);

//        if(((g_eeGeneral.stickMode&1) != (i&1)) || !(g_model.thrTrim)){
            lcd_vline(xm-1, ym-1,  3);
            lcd_vline(xm+1, ym-1,  3);
//        }
        ym -= val;
    }else{
      ym=59;
      lcd_hline(xm-TL,ym,    TL*2);
      lcd_hline(xm-1, ym-1,  3);
      lcd_hline(xm-1, ym+1,  3);
      xm += val;
    }
    DO_SQUARE(xm,ym,7)
  }
  register uint32_t a = 0 ; // (view == e_inputs1) ? 0 : 9+(view-3)*6;
  register uint32_t b = 6 ; // (view == e_inputs1) ? 6 : 12+(view-3)*6;
  for( i=a; i<(a+3); i++) lcd_putsnAtt(2*FW-2 ,(i-a)*FH+4*FH,get_switches_string()+3*i,3,getSwitch(i+1, 0, 0) ? INVERS : 0);
  for( i=b; i<(b+3); i++) lcd_putsnAtt(17*FW-1,(i-b)*FH+4*FH,get_switches_string()+3*i,3,getSwitch(i+1, 0, 0) ? INVERS : 0);

}



void putsTime(uint8_t x,uint8_t y,int16_t tme,uint8_t att,uint8_t att2)
{
  if ( tme<0 )
  {
    lcd_putcAtt( x - ((att&DBLSIZE) ? FWNUM*6-2 : FWNUM*3),    y, '-',att);
    tme = -tme;
  }

  lcd_putcAtt(x, y, ':',att&att2);
  lcd_outdezNAtt(x+ ((att&DBLSIZE) ? 2 : 0), y, tme/60, LEADING0|att,2);
  x += (att&DBLSIZE) ? FWNUM*6-2 : FW*3-1;
  lcd_outdezNAtt(x, y, tme%60, LEADING0|att2,2);
}

void putsVolts(uint8_t x,uint8_t y, uint8_t volts, uint8_t att)
{
  lcd_outdezAtt(x, y, volts, att|PREC1);
  if(!(att&NO_UNIT)) lcd_putcAtt(Lcd_lastPos, y, 'v', att);
}


void putsVBat(uint8_t x,uint8_t y,uint8_t att)
{
  att |= g_vbat100mV < g_eeGeneral.vBatWarn ? BLINK : 0;
  putsVolts(x, y, g_vbat100mV, att);
}

void putsChnRaw(uint8_t x,uint8_t y,uint8_t idx,uint8_t att)
{
  if(idx==0)
    lcd_putsnAtt(x,y,PSTR("----"),4,att);
  else if(idx<=4)
    lcd_putsnAtt(x,y,modi12x3+g_eeGeneral.stickMode*16+4*(idx-1),4,att);
  else if(idx<=NUM_XCHNRAW)
    lcd_putsnAtt(x,y,PSTR("P1  P2  P3  MAX FULLCYC1CYC2CYC3PPM1PPM2PPM3PPM4PPM5PPM6PPM7PPM8CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16"TELEMETRY_CHANNELS)+4*(idx-5),4,att);
}

void putsChn(uint8_t x,uint8_t y,uint8_t idx1,uint8_t att)
{
  // !! todo NUM_CHN !!
  lcd_putsnAtt(x,y,PSTR("--- CH1 CH2 CH3 CH4 CH5 CH6 CH7 CH8 CH9 CH10CH11CH12CH13CH14CH15CH16"
                        "CH17CH18CH19CH20CH21CH22CH23CH24CH25CH26CH27CH28CH29CH30")+4*idx1,4,att);
}

void putsDrSwitches(uint8_t x,uint8_t y,int8_t idx1,uint8_t att)//, bool nc)
{
  switch(idx1){
    case  0:            lcd_putsAtt(x+FW,y,PSTR("---"),att);return;
    case  MAX_DRSWITCH: lcd_putsAtt(x+FW,y,PSTR("ON "),att);return;
    case -MAX_DRSWITCH: lcd_putsAtt(x+FW,y,PSTR("OFF"),att);return;
  }
  lcd_putcAtt(x,y, idx1<0 ? '!' : ' ',att);
  lcd_putsnAtt(x+FW,y,get_switches_string()+3*(abs(idx1)-1),3,att);
}


const char *get_switches_string()
{
  return PSTR(SWITCHES_STR)	;
}	


bool Last_switch[NUM_CSW] ;

bool getSwitch(int8_t swtch, bool nc, uint8_t level)
{
//  bool ret_value ;
//  uint8_t cs_index ;
  
	if(level>5) return FALSE ; //prevent recursive loop going too deep

  switch(swtch){
    case  0:            return  nc;
    case  MAX_DRSWITCH: return  TRUE ;
    case -MAX_DRSWITCH: return  FALSE ;
  }

  uint8_t dir = swtch>0;
  if(abs(swtch)<(MAX_DRSWITCH-NUM_CSW)) {
    if(!dir) return ! keyState((enum EnumKeys)(SW_BASE-swtch-1));
    return            keyState((enum EnumKeys)(SW_BASE+swtch-1));
  }

	return FALSE ;

//  //custom switch, Issue 78
//  //use putsChnRaw
//  //input -> 1..4 -> sticks,  5..8 pots
//  //MAX,FULL - disregard
//  //ppm
//  cs_index = abs(swtch)-(MAX_DRSWITCH-NUM_CSW);
//  CSwData &cs = g_model.customSw[cs_index];
//  if(!cs.func) return false;

//  if ( level>4 )
//  {
//    ret_value = Last_switch[cs_index] ;
//    return swtch>0 ? ret_value : !ret_value ;
//  }

//  int8_t a = cs.v1;
//  int8_t b = cs.v2;
//  int16_t x = 0;
//  int16_t y = 0;

//  // init values only if needed
//  uint8_t s = CS_STATE(cs.func);

//  if(s == CS_VOFS)
//  {
//      x = getValue(cs.v1-1);
//#ifdef FRSKY
//      if (cs.v1 > CHOUT_BASE+NUM_CHNOUT)
//        y = 125+cs.v2;
//      else
//#endif
//      y = calc100toRESX(cs.v2);
//  }
//  else if(s == CS_VCOMP)
//  {
//      x = getValue(cs.v1-1);
//      y = getValue(cs.v2-1);
//  }

//  switch (cs.func) {
//  case (CS_VPOS):
//      ret_value = (x>y);
//      break;
//  case (CS_VNEG):
//      ret_value = (x<y) ;
//      break;
//  case (CS_APOS):
//  {
//      ret_value = (abs(x)>y) ;
//  }
////      return swtch>0 ? (abs(x)>y) : !(abs(x)>y);
//      break;
//  case (CS_ANEG):
//  {
//      ret_value = (abs(x)<y) ;
//  }
////      return swtch>0 ? (abs(x)<y) : !(abs(x)<y);
//      break;

////  case (CS_AND):
////      return (getSwitch(a,0,level+1) && getSwitch(b,0,level+1));
////      break;
////  case (CS_OR):
////      return (getSwitch(a,0,level+1) || getSwitch(b,0,level+1));
////      break;
////  case (CS_XOR):
////      return (getSwitch(a,0,level+1) ^ getSwitch(b,0,level+1));
////      break;
//  case (CS_AND):
//  case (CS_OR):
//  case (CS_XOR):
//  {
//    bool res1 = getSwitch(a,0,level+1) ;
//    bool res2 = getSwitch(b,0,level+1) ;
//    if ( cs.func == CS_AND )
//    {
//      ret_value = res1 && res2 ;
//    }
//    else if ( cs.func == CS_OR )
//    {
//      ret_value = res1 || res2 ;
//    }
//    else  // CS_XOR
//    {
//      ret_value = res1 ^ res2 ;
//    }
//  }
//  break;

//  case (CS_EQUAL):
//      ret_value = (x==y);
//      break;
//  case (CS_NEQUAL):
//      ret_value = (x!=y);
//      break;
//  case (CS_GREATER):
//      ret_value = (x>y);
//      break;
//  case (CS_LESS):
//      ret_value = (x<y);
//      break;
//  case (CS_EGREATER):
//      ret_value = (x>=y);
//      break;
//  case (CS_ELESS):
//      ret_value = (x<=y);
//      break;
//  default:
//      ret_value = false;
//      break;
//  }
//	Last_switch[cs_index] = ret_value ;
//	return swtch>0 ? ret_value : !ret_value ;

}





void perOut(int16_t *chanOut, uint8_t att)
{
//    int16_t  trimA[4];
//    uint8_t  anaCenter = 0;
//    uint16_t d = 0;

//    if(tick10ms) {
//        if(s_noHi) s_noHi--;
//        if( (g_eeGeneral.inactivityTimer + 10) && (g_vbat100mV>49)) {
//            if (++inacPrescale > 15 )
//            {
//                inacCounter++;
//                inacPrescale = 0 ;
//            }
//            uint16_t tsum = 0;
//            for(uint8_t i=0;i<4;i++) tsum += anas[i];
//            if(abs(int16_t(tsum-inacSum))>INACTIVITY_THRESHOLD){
//                inacSum = tsum;
//                inacCounter=0;
//            }
//            if(inacCounter>((uint16_t)(g_eeGeneral.inactivityTimer+10)*(100*60/16)))
//                if((inacCounter&0x3)==1) {
//                    audioDefevent(AUDIO_INACTIVITY);
//                }
//        }
//    }
//    {
//        uint8_t ele_stick, ail_stick ;
//        ele_stick = ELE_STICK ;
//        ail_stick = AIL_STICK ;
//        //===========Swash Ring================
//        if(g_model.swashRingValue)
//        {
//            uint32_t v = (int32_t(calibratedStick[ele_stick])*calibratedStick[ele_stick] +
//                          int32_t(calibratedStick[ail_stick])*calibratedStick[ail_stick]);
//            uint32_t q = int32_t(RESX)*g_model.swashRingValue/100;
//            q *= q;
//            if(v>q)
//                d = isqrt32(v);
//        }
//        //===========Swash Ring================
	register uint32_t i ;
        
				for( i=0;i<7;i++)
				{        // calc Sticks

            //Normalization  [0..2048] ->   [-1024..1024]

            int16_t v = anaIn(i);
//            v -= g_eeGeneral.calibMid[i];
						v -= 1024 ;
//            v  =  v * (int32_t)RESX /  (max((int16_t)100,(v>0 ?
//                                                              g_eeGeneral.calibSpanPos[i] :
//                                                              g_eeGeneral.calibSpanNeg[i])));
            if(v <= -RESX) v = -RESX ;
            if(v >=  RESX) v =  RESX ;
//	  				if ( g_eeGeneral.throttleReversed )
//						{
//							if ( i == THR_STICK )
//							{
//								v = -v ;
//							}
//						}
            calibratedStick[i] = v; //for show in expo
				}
}

void resetTimer2()
{
  Timer2_pre = 0 ;
  Timer2 = 0 ;
  Timer2_running = 0 ;   // Stop and clear throttle started flag
}

inline uint8_t keyDown()
{
    return ~read_keys() & 0x7E ;
}

void clearKeyEvents()
{
    while(keyDown())
		{
			  // loop until all keys are up
			if ( PIOC->PIO_PDSR & 0x02000000 )
			{
				// Detected USB
				break ;
			}
		}	
    putEvent(0);
}


void alertMessages( const char * s, const char * t )
{
  lcd_clear();
  lcd_putsAtt(64-5*FW,0*FH,PSTR("ALERT"),DBLSIZE);
  lcd_puts_P(0,4*FH,s);
  lcd_puts_P(0,5*FH,t);
  lcd_puts_P(0,6*FH,  PSTR("Press any key to skip") ) ;
  refreshDisplay();
//  lcdSetRefVolt(g_eeGeneral.contrast);

  clearKeyEvents();
}


void alert(const char * s, bool defaults)
{
    lcd_clear();
    lcd_putsAtt(64-5*FW,0*FH,PSTR("ALERT"),DBLSIZE);
    lcd_puts_P(0,4*FH,s);
    lcd_puts_P(64-6*FW,7*FH,PSTR("press any Key"));
    refreshDisplay();
    lcdSetRefVolt(defaults ? 0x22 : g_eeGeneral.contrast);

//    audioDefevent(AUDIO_ERROR);
    clearKeyEvents();
    while(1)
    {
        if(keyDown())
        {
            return;  //wait for key release
        }
        if(heartbeat == 0x3)
        {
            wdt_reset();
            heartbeat = 0;
        }
				if ( PIOC->PIO_PDSR & 0x02000000 )
				{
					// Detected USB
					break ;
				}

//        if(getSwitch(g_eeGeneral.lightSw,0) || g_eeGeneral.lightAutoOff || defaults)
//            BACKLIGHT_ON;
//        else
//            BACKLIGHT_OFF;
    }
}

void message(const char * s)
{
  lcd_clear();
  lcd_putsAtt(64-5*FW,0*FH,PSTR("MESSAGE"),DBLSIZE);
  lcd_puts_P(0,4*FW,s);
  refreshDisplay();
//  lcdSetRefVolt(g_eeGeneral.contrast);
}



uint8_t char2idx(char c)
{
	int8_t ret ;
    for(ret=0;;ret++)
    {
        char cc= s_charTab[ret] ;
        if(cc==c) return ret;
        if(cc==0) return 0;
    }
}
char idx2char(uint8_t idx)
{
    if(idx < NUMCHARS) return s_charTab[idx] ;
    return ' ';
}


MenuFuncP lastPopMenu()
{
  return  g_menuStack[g_menuStackPtr+1];
}

void popMenu(bool uppermost)
{
  if(g_menuStackPtr>0 || uppermost){
    g_menuStackPtr = uppermost ? 0 : g_menuStackPtr-1;
    audioDefevent(AUDIO_MENUS);
    (*g_menuStack[g_menuStackPtr])(EVT_ENTRY_UP);
  }else{
    alert(PSTR("menuStack underflow"));
  }
}

void chainMenu(MenuFuncP newMenu)
{
  g_menuStack[g_menuStackPtr] = newMenu;
  (*newMenu)(EVT_ENTRY);
  audioDefevent(AUDIO_MENUS);
}
void pushMenu(MenuFuncP newMenu)
{

  g_menuStackPtr++;
  if(g_menuStackPtr >= DIM(g_menuStack))
  {
    g_menuStackPtr--;
    alert(PSTR("menuStack overflow"));
    return;
  }
  audioDefevent(AUDIO_MENUS);
  g_menuStack[g_menuStackPtr] = newMenu;
  (*newMenu)(EVT_ENTRY);
}


//global helper vars
bool    checkIncDec_Ret;
int16_t p1val;
int16_t p1valdiff;

int16_t checkIncDec16(uint8_t event, int16_t val, int16_t i_min, int16_t i_max, uint8_t i_flags)
{
  int16_t newval = val;
  uint8_t kpl=KEY_RIGHT, kmi=KEY_LEFT, kother = -1;

  if(event & _MSK_KEY_DBL){
    uint8_t hlp=kpl;
    kpl=kmi;
    kmi=hlp;
    event=EVT_KEY_FIRST(EVT_KEY_MASK & event);
  }
  if(event==EVT_KEY_FIRST(kpl) || event== EVT_KEY_REPT(kpl) || (s_editMode && (event==EVT_KEY_FIRST(KEY_UP) || event== EVT_KEY_REPT(KEY_UP))) ) {
    newval++;

		audioDefevent(AUDIO_KEYPAD_UP);

    kother=kmi;
  }else if(event==EVT_KEY_FIRST(kmi) || event== EVT_KEY_REPT(kmi) || (s_editMode && (event==EVT_KEY_FIRST(KEY_DOWN) || event== EVT_KEY_REPT(KEY_DOWN))) ) {
    newval--;

		audioDefevent(AUDIO_KEYPAD_DOWN);

    kother=kpl;
  }
  if((kother != (uint8_t)-1) && keyState((EnumKeys)kother)){
    newval=-val;
    killEvents(kmi);
    killEvents(kpl);
  }
  if(i_min==0 && i_max==1 && event==EVT_KEY_FIRST(KEY_MENU))
  {
      s_editMode = false;
      newval=!val;
      killEvents(event);
  }

  //change values based on P1
  newval -= p1valdiff;

  if(newval>i_max)
  {
    newval = i_max;
    killEvents(event);
    audioDefevent(AUDIO_KEYPAD_UP);
  }
  else if(newval < i_min)
  {
    newval = i_min;
    killEvents(event);
    audioDefevent(AUDIO_KEYPAD_DOWN);

  }
  if(newval != val) {
    if(newval==0) {
      pauseEvents(event);
  
		if (newval>val){
			audioDefevent(AUDIO_KEYPAD_UP);
		} else {
			audioDefevent(AUDIO_KEYPAD_DOWN);
		}		

    }
    eeDirty(i_flags & (EE_GENERAL|EE_MODEL));
    checkIncDec_Ret = true;
  }
  else {
    checkIncDec_Ret = false;
  }
  return newval;
}

int8_t checkIncDec(uint8_t event, int8_t i_val, int8_t i_min, int8_t i_max, uint8_t i_flags)
{
  return checkIncDec16(event,i_val,i_min,i_max,i_flags);
}

int8_t checkIncDec_hm(uint8_t event, int8_t i_val, int8_t i_min, int8_t i_max)
{
  return checkIncDec(event,i_val,i_min,i_max,EE_MODEL);
}

int8_t checkIncDec_hg(uint8_t event, int8_t i_val, int8_t i_min, int8_t i_max)
{
  return checkIncDec(event,i_val,i_min,i_max,EE_GENERAL);
}


/*

 Test code

inline int32_t calc100toRESX(register int8_t x)
{
  return (((uint32_t)x*168) - (uint32_t)x)>>6 ;
}

inline int16_t calc1000toRESX( register int32_t x)  // improve calc time by Pat MacKenzie
{
    register int32_t y = x>>5;
    x+=y;
    y=y>>2;
    x-=y;
    return x+(y>>2);
    //  return x + x/32 - x/128 + x/512;
}



int32_t test_calc( register int8_t x )
{
	return calc100toRESX( x ) ;	
}

int32_t test_calc1( register int16_t x )
{
	return calc1000toRESX(x) ;
}

int32_t test_calc2( register int16_t x )
{
	return (int32_t) x *256 / 25 ;
}

 */



/*** EOF ***/

