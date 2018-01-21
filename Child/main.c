
#include "common.h"
#include "nrf24l01.h"

static void SystemInit(void);
static u16_t ReadTemperature(void);
static void Beep(void);

//read battery

#define TIMER_A0_RELOAD   (62500)  /* 8us x reload = period) */
#define TIMER_COUNT_MAX   (2)      /* x period = 1sec */
#define MAX_TEMPERATURE   (763)     /* ~37C */
#define PIEZO             (P2OUT_bit.P2OUT_6)

static u16_t timerCount = TIMER_COUNT_MAX;

void main(void)
{
  SystemInit();
  NRF24L01Init();
  Beep();
  
  while (1) { /* __low_power_mode_3(); */ };
}

static u16_t ReadTemperature(void)
{
  ADC10CTL1 = INCH_10 + ADC10DIV_4;
  ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON + ADC10SR;
  __delay_cycles(10000);
  ADC10CTL0 |= ENC + ADC10SC;
  while (ADC10CTL1 & ADC10BUSY);
  ADC10CTL0 &= ~ENC;
  return ADC10MEM;

  /*
    TEMPC = (VTEMP - 0.986) / 0.00355
    VTEMP = (ADC / 1024) * 1.5
    VTEMP = 0.00355 * (TEMPC) + 0.986
    ADC = (VTEMP / 1.5) * 1024
  */
}

#pragma vector = TIMERA0_VECTOR
__interrupt void TimerA0IntrHandler(void)
{ 
  TACCR0 += TIMER_A0_RELOAD;
  
  if (--timerCount == 0)
  {
    u16_t temperature;
    timerCount = TIMER_COUNT_MAX;
    temperature = ReadTemperature();
    if (temperature > MAX_TEMPERATURE)
      Beep();
    NRF24L01SendPacket((u8_t *)&temperature, 2);
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
