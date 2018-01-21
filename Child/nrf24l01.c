
/*! \file nrf24l01.c
    \brief nRF24L01 RF driver module
*/

#include "common.h"
#include "nrf24l01.h"

static u8_t NRF24L01WriteByte(u8_t byte);
static u8_t NRF24L01ReadByte(void);
static u8_t NRF24L01ReadRegister(NRF24L01RegAddr_t addr);
static void NRF24L01WriteRegister(NRF24L01RegAddr_t addr, u8_t byte);
static u8_t NRF24L01WriteCommand(u8_t cmd);

#define NRF24L01_IRQ          (P1IN_bit.P1IN_1)
#define NRF24L01_CE           (P1OUT_bit.P1OUT_3)
#define NRF24L01_CSN          (P1OUT_bit.P1OUT_2)

/* shadow registers */
union
{
  u8_t regArray[NRF24L01_MAX_REGS];
  struct
  {
    NRF24L01RegConfig_t      regConfig;
    NRF24L01RegEnAA_t        regEnAA;
    NRF24L01RegEnRxAddr_t    regEnRxAddr;
    NRF24L01RegSetupAw_t     regSetupAw;
    NRF24L01RegSetupRetR_t   regSetupRetR;
    NRF24L01RegRfCh_t        regRfCh;
    NRF24L01RegRfSetup_t     regRfSetup;
    NRF24L01RegStatus_t      regStatus;
    NRF24L01RegObserveTx_t   regObserveTx;
    NRF24L01RegCd_t          regCd;
    NRF24L01RegRxPwP0_t      regPwP0;
    NRF24L01RegRxPwP1_t      regPwP1;
    NRF24L01RegRxPwP2_t      regPwP2;
    NRF24L01RegRxPwP3_t      regPwP3;
    NRF24L01RegRxPwP4_t      regPwP4;
    NRF24L01RegRxPwP5_t      regPwP5;
    NRF24L01RegFifoStatus_t  regFifoStatus;
    u8_t                     regAckPld;
    u8_t                     regTxPld;
    u8_t                     regRxPld;
    u8_t                     reserved;
    NRF24L01RegDynPd_t       regDynPd;
    NRF24L01RegFeature_t     regFeature;
  };
} NRF24L01Regs;

/*!
 \brief 
 */
u8_t NRF24L01Init(void)
{ 
  u8_t retVal = RET_SUCCESS;
  u8_t i;
  
  NRF24L01_CSN = 1;
  NRF24L01_CE = 0;

  /* read in shadow registers */
  for (i = 0; i < NRF24L01_MAX_REGS; i++)
    NRF24L01Regs.regArray[i] = NRF24L01ReadRegister((NRF24L01RegAddr_t)i);

  /* CONFIG register setup */
  NRF24L01Regs.regConfig.byte = 0;
  NRF24L01Regs.regConfig.bits.MASK_MAX_RT = 1;
  NRF24L01Regs.regConfig.bits.EN_CRC = 1;
  NRF24L01Regs.regConfig.bits.CRCO = 1;
  NRF24L01Regs.regConfig.bits.PRIM_RX = 1;
  NRF24L01Regs.regConfig.bits.PWR_UP = 1;
  NRF24L01WriteRegister(NRF24L01_REG_CONFIG, NRF24L01Regs.regConfig.byte);
  if (NRF24L01ReadRegister(NRF24L01_REG_CONFIG) != NRF24L01Regs.regConfig.byte)
    retVal |= RET_FAIL;
  
  /* wait for power-up, to to Standby-I mode */
  __delay_cycles(100);
  
  /* RETR register setup */
  NRF24L01Regs.regSetupRetR.byte = 0;  /* 500us + 86us (10 retries) */
  NRF24L01WriteRegister(NRF24L01_REG_SETUP_RETR, NRF24L01Regs.regSetupRetR.byte);
  if (NRF24L01ReadRegister(NRF24L01_REG_SETUP_RETR) != NRF24L01Regs.regSetupRetR.byte)
    retVal |= RET_FAIL;

  /* AW register setup */
  NRF24L01Regs.regSetupAw.byte = 0;
  NRF24L01Regs.regSetupAw.bits.AW = 1;
  NRF24L01WriteRegister(NRF24L01_REG_SETUP_AW, NRF24L01Regs.regSetupAw.byte);
  if (NRF24L01ReadRegister(NRF24L01_REG_SETUP_AW) != NRF24L01Regs.regSetupAw.byte)
    retVal |= RET_FAIL;

  /* RF_SETUP register setup */
  NRF24L01Regs.regRfSetup.byte = 0;
  NRF24L01Regs.regRfSetup.bits.RF_DR = 1;  /* 2Mbps */
  NRF24L01Regs.regRfSetup.bits.RF_PWR = 3; /* 0dBm */
  NRF24L01Regs.regRfSetup.bits.LNA_HCURR = 1;
  NRF24L01WriteRegister(NRF24L01_REG_RF_SETUP, NRF24L01Regs.regRfSetup.byte);
  if (NRF24L01ReadRegister(NRF24L01_REG_RF_SETUP) != NRF24L01Regs.regRfSetup.byte)
    retVal |= RET_FAIL;

  /* RX_PW_P0 register setup */
  NRF24L01WriteRegister(NRF24L01_REG_RX_PW_P0, 2);
  if (NRF24L01ReadRegister(NRF24L01_REG_RX_PW_P0) != 2)
    retVal |= RET_FAIL;

  /* RF_CH register setup */
  NRF24L01Regs.regRfCh.byte = 0;
  NRF24L01Regs.regRfCh.bits.RF_CH = 40;
  NRF24L01WriteRegister(NRF24L01_REG_RF_CH, NRF24L01Regs.regRfCh.byte);
  if (NRF24L01ReadRegister(NRF24L01_REG_RF_CH) != NRF24L01Regs.regRfCh.byte)
    retVal |= RET_FAIL;

  /* TX_ADDR register setup */
  NRF24L01_CSN = 0;
  NRF24L01WriteByte(NRF24L01_REG_TX_ADDR);
  NRF24L01WriteByte(0xE7);
  NRF24L01WriteByte(0xE7);
  NRF24L01WriteByte(0xE7);
  NRF24L01WriteByte(0xE7);
  NRF24L01WriteByte(0xE7);
  NRF24L01_CSN = 1;

  /* EN_AA register setup */
  NRF24L01Regs.regEnAA.byte = 0;
  NRF24L01WriteRegister(NRF24L01_REG_EN_AA, NRF24L01Regs.regEnAA.byte);
  if (NRF24L01ReadRegister(NRF24L01_REG_EN_AA) != NRF24L01Regs.regEnAA.byte)
    retVal |= RET_FAIL;
  
  
//  debug
//  memset(NRF24L01Regs.regArray, 0, sizeof(NRF24L01Regs.regArray));
//  for (i = 0; i < NRF24L01_MAX_REGS; i++)
//    NRF24L01Regs.regArray[i] = NRF24L01ReadRegister(i);
     
  return retVal;
}


/*!
 \brief 
 */
static u8_t NRF24L01WriteByte(u8_t byte)
{ 
  USISRL = byte;
  USICNT = 8;
  while (!(USICTL1 & USIIFG)) { };
  return USISRL;  
}

/*!
 \brief 
 */
static u8_t NRF24L01ReadByte(void)
{
  USICNT = 8;
  while (!(USICTL1 & USIIFG)); 
  return USISRL;
}

/*!
 \brief 
 */
static u8_t NRF24L01ReadRegister(NRF24L01RegAddr_t addr)
{
  u8_t retByte;
  
  NRF24L01_CSN = 0;
  NRF24L01WriteByte(NRF24L01_READ_REG | addr);
  retByte = NRF24L01ReadByte();
  NRF24L01_CSN = 1;
  
  return retByte;
}

/*!
 \brief 
 */
static void NRF24L01WriteRegister(NRF24L01RegAddr_t addr, u8_t byte)
{
  NRF24L01_CSN = 0;
  NRF24L01WriteByte(NRF24L01_WRITE_REG | addr);
  NRF24L01WriteByte(byte);
  NRF24L01_CSN = 1;
}

/*!
 \brief 
 */
static u8_t NRF24L01WriteCommand(u8_t cmd)
{
  u8_t retByte;
  
  NRF24L01_CSN = 0;
  retByte = NRF24L01WriteByte(cmd);  
  NRF24L01_CSN = 1;
  
  return retByte;
}

/*!
 \brief 
 */
u8_t NRF24L01StartTransmitMode(void)
{ 
  /* clear all interrupt flags */
	NRF24L01WriteRegister(NRF24L01_REG_STATUS, NRF24L01_INT_ALL);

  /* Go into TX mode */
  NRF24L01Regs.regConfig.bits.PRIM_RX = 0;
  NRF24L01WriteRegister(NRF24L01_REG_CONFIG, NRF24L01Regs.regConfig.byte);
  
  return RET_SUCCESS;
}

/*!
 \brief 
 */
bool NRF24L01IsPacketTransmitted(void)
{ 
  bool rcvd;
  
  rcvd = NRF24L01WriteCommand(NRF24L01_NOP) & NRF24L01_INT_TX_DS;
  
  return rcvd ? TRUE: FALSE;
}

/*!
 \brief 
 */
u8_t NRF24L01WriteFifo(u8_t *pPacket, u8_t numBytes)
{
  /* flush transmit FIFO */
  NRF24L01WriteCommand(NRF24L01_FLUSH_TX);
  
  /* write payload to FIFO */
  NRF24L01_CSN = 0;
  NRF24L01WriteByte(NRF24L01_WR_TX_PLOAD);
  while (numBytes--)
    NRF24L01WriteByte(*pPacket++);
  NRF24L01_CSN = 1;
    
  return RET_SUCCESS;
}

/*!
 \brief 
 */
u8_t NRF24L01InitiateTransmit(void)
{ 
  /* initiate TX */
  NRF24L01_CE = 1;
  __delay_cycles(10);
  NRF24L01_CE = 0;
  
  return RET_SUCCESS;
}

/*!
 \brief 
 */
u8_t NRF24L01EndTransmitMode(void)
{ 
  /* clear data sent (DS) interrupt */
  NRF24L01WriteRegister(NRF24L01_REG_STATUS, NRF24L01_INT_TX_DS);
  
  return RET_SUCCESS;
}

/*!
 \brief 
 */
u8_t NRF24L01SendPacket(u8_t *pPacket, u8_t numBytes)
{
  u8_t retVal = RET_SUCCESS;
  
  /* clear all interrupt flags */
	NRF24L01WriteRegister(NRF24L01_REG_STATUS, NRF24L01_INT_ALL);

  /* Go into TX mode */
  NRF24L01Regs.regConfig.bits.PRIM_RX = 0;
  NRF24L01WriteRegister(NRF24L01_REG_CONFIG, NRF24L01Regs.regConfig.byte);

  /* flush transmit FIFO */
  NRF24L01WriteCommand(NRF24L01_FLUSH_TX);
  
  /* write payload to FIFO */
  NRF24L01_CSN = 0;
  NRF24L01WriteByte(NRF24L01_WR_TX_PLOAD);
  while (numBytes--)
    NRF24L01WriteByte(*pPacket++);
  NRF24L01_CSN = 1;
  
  /* initiate TX */
  NRF24L01_CE = 1;
  __delay_cycles(10);
  NRF24L01_CE = 0;
  
  /* wait until TX done */
  while ((NRF24L01WriteCommand(NRF24L01_NOP) & NRF24L01_INT_TX_DS) == 0) { };
  
  /* clear data sent (DS) interrupt */
  NRF24L01WriteRegister(NRF24L01_REG_STATUS, NRF24L01_INT_TX_DS);
  
  return retVal;
}

/*!
 \brief 
 */
u8_t NRF24L01StartReceiveMode(void)
{ 
  /* Go into RX mode */
  NRF24L01Regs.regConfig.bits.PRIM_RX = 1;
  NRF24L01WriteRegister(NRF24L01_REG_CONFIG, NRF24L01Regs.regConfig.byte);
  NRF24L01_CE = 1;
  
  return RET_SUCCESS;
}

/*!
 \brief 
 */
u8_t NRF24L01EndReceiveMode(void)
{ 
  /* get out of RX mode */
  NRF24L01_CE = 0;
  
    /* flush receive FIFO */
  NRF24L01WriteCommand(NRF24L01_FLUSH_RX);

  /* clear data received (DR) interrupt */
  NRF24L01WriteRegister(NRF24L01_REG_STATUS, NRF24L01_INT_RX_DR);
  
  return RET_SUCCESS;
}

/*!
 \brief 
 */
bool NRF24L01IsPacketReceived(void)
{ 
  bool rcvd;
  
  rcvd = NRF24L01WriteCommand(NRF24L01_NOP) & NRF24L01_INT_RX_DR;
  
  return rcvd ? TRUE: FALSE;
}

/*!
 \brief 
 */
u8_t NRF24L01ReadFifo(u8_t *pPacket, u8_t maxBytes)
{
  u8_t numBytes = 0;
  
  /* read payload from FIFO */
  NRF24L01_CSN = 0;
  NRF24L01WriteByte(NRF24L01_RD_RX_PLOAD);
  while (maxBytes--)
  {
    *pPacket++ = NRF24L01ReadByte();
    numBytes++;
  }
  NRF24L01_CSN = 1;
    
  return numBytes;
}


/*!
 \brief 
 */
u8_t NRF24L01ReceivePacket(u8_t *pPacket, u8_t maxBytes)
{
  u8_t numBytes = 0;
    
  /* Go into RX mode */
  NRF24L01Regs.regConfig.bits.PRIM_RX = 1;
  NRF24L01WriteRegister(NRF24L01_REG_CONFIG, NRF24L01Regs.regConfig.byte);
  NRF24L01_CE = 1;

  /* wait until packet received */
  while ((NRF24L01WriteCommand(NRF24L01_NOP) & NRF24L01_INT_RX_DR) == 0) { };

  /* read payload from FIFO */
  NRF24L01_CSN = 0;
  NRF24L01WriteByte(NRF24L01_RD_RX_PLOAD);
  while (maxBytes--)
  {
    *pPacket++ = NRF24L01ReadByte();
    numBytes++;
  }
  NRF24L01_CSN = 1;

  /* flush receive FIFO */
  NRF24L01WriteCommand(NRF24L01_FLUSH_RX);

  /* clear data received (DR) interrupt */
  NRF24L01WriteRegister(NRF24L01_REG_STATUS, NRF24L01_INT_RX_DR);
  
  /* get out of RX mode */
  NRF24L01_CE = 0;
  
  return numBytes;
}

