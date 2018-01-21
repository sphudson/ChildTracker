
#include "common.h"
#include "nrf24l01.h"

static void SystemInit(void);
static void Beep(void);

#define TIMER_A0_RELOAD   (62500)  /* 8us x reload = period = 500ms) */
#define TIMER_COUNT_MAX   (10)     /* x period = 5sec */
#define PIEZO             (P2OUT_bit.P2OUT_6)
#define MAX_TEMPERATURE   (763)     /* ~37C */

static u16_t timerCount = TIMER_COUNT_MAX;

void main(void)
{
  SystemInit();
  NRF24L01Init();
  Beep();
  __delay_cycles(65536);
  Beep();
  
  NRF24L01StartReceiveMode();
 
  while (1) { /* __low_power_mode_3(); */ };
}

#pragma vector = TIMERA0_VECTOR
__interrupt void TimerA0IntrHandler(void)
{ 
  TACCR0 += TIMER_A0_RELOAD;
  
  if (NRF24L01IsPacketReceived())
  {
    u16_t temperature;
    
    timerCount = TIMER_COUNT_MAX;
    NRF24L01ReadFifo((u8_t *)&temperature, 2);
    NRF24L01EndReceiveMode();
    NRF24L01StartReceiveMode();
    if (temperature > MAX_TEMPERATURE)
      Beep();
  }
  
  if (--timerCount == 0)
  {
    timerCount = TIMER_COUNT_MAX;
    Beep();
    __delay_cycles(65536);
    Beep();
  }
}

static void SystemInit(void)
{
  /* stop watchdog */
  WDTCTL = WDTPW + WDTHOLD;
    
  /* delay for ~65ms (clock is ~1MHz at power-up) for Vcc to stabilize */
  __delay_cycles(65536);

  /* setup port 1 */
  P1SEL = BIT5 + BIT6 + BIT7;
  P1OUT = 0;
  P1DIR = BIT2 + BIT3 + BIT5 + BIT6;
  P1IE = 0;

  /* setup port 2 */
  P2SEL = 0;
  P2OUT = 0;
  P2DIR = BIT6;
  
  /* setup timer A0 */
  TACTL = TASSEL_2 + MC_2 + ID_3;
  TACCR0 = TAR + TIMER_A0_RELOAD;
  TACCR1 = 0;
  TACCTL0_bit.CCIFG = 0;
  TACCTL0_bit.CCIE = 1;
  TACCTL1_bit.CCIE = 0;
  
    /* setup USI */
  USICTL0 |= USIPE5 + USIPE6 + USIPE7 + USIMST + USIOE;
  USICTL1 |= USICKPH;
  USICKCTL = USISSEL_2 + USIDIV_1;
  USICTL1 &= ~USIIFG;
  USICTL0 &= ~USISWRST;
  
  __enable_interrupt();
}

static void Beep(void)
{
  u16_t counter = 1000;
  while (counter--)
  {
    __delay_cycles(126);
    PIEZO ^= 1;
  }
}
