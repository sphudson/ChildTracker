
#ifndef _NRF24L01_H_
#define _NRF24L01_H_

#include "common.h"

#define NRF24L01_MAX_PAYLOAD_SIZE    (32)
#define NRF24L01_MAX_REGS            (0x1E)

/* Interrupt flags */
#define NRF24L01_INT_IDLE        (0x00)  /* Idle, no interrupt pending */
#define NRF24L01_INT_MAX_RT      (0x10)  /* Max #of TX retrans interrupt */
#define NRF24L01_INT_TX_DS       (0x20)  /* TX data sent interrupt */
#define NRF24L01_INT_RX_DR       (0x40)  /* RX data received */
#define NRF24L01_INT_ALL         (NRF24L01_INT_RX_DR | \
                                  NRF24L01_INT_TX_DS | \
                                  NRF24L01_INT_MAX_RT) /* All flags */

/* SPI commands */
#define NRF24L01_READ_REG            (0x00)  /* Read from register */
#define NRF24L01_WRITE_REG           (0x20)  /* Write to register */
#define NRF24L01_RD_RX_PLOAD         (0x61)  /* RX payload register address */
#define NRF24L01_WR_TX_PLOAD         (0xA0)  /* TX payload register address */
#define NRF24L01_FLUSH_TX            (0xE1)  /* Flush TX register */
#define NRF24L01_FLUSH_RX            (0xE2)  /* Flush RX register */
#define NRF24L01_REUSE_TX_PL         (0xE3)  /* Reuse TX payload register */
#define NRF24L01_ACTIVATE            (0x50)  /* Activate feature command */
#define NRF24L01_R_RX_PL_WID         (0x60)  /* Read RX payload width */
#define NRF24L01_W_ACK_PAYLOAD       (0xA8)  /* Write payload with ACK packet */
#define NRF24L01_W_TX_PAYLOAD_NOACK  (0xB0)  /* Disable ACK with this packet */
#define NRF24L01_NOP                 (0xFF)  /* No operation */

/* register addresses */
typedef enum
{
  NRF24L01_REG_CONFIG          = 0x00,
  NRF24L01_REG_EN_AA,
  NRF24L01_REG_EN_RXADDR,
  NRF24L01_REG_SETUP_AW,
  NRF24L01_REG_SETUP_RETR,
  NRF24L01_REG_RF_CH,
  NRF24L01_REG_RF_SETUP,
  NRF24L01_REG_STATUS,
  NRF24L01_REG_OBSERVE_TX,
  NRF24L01_REG_CD,
  NRF24L01_REG_RX_ADDR_PO,
  NRF24L01_REG_RX_ADDR_P1,
  NRF24L01_REG_RX_ADDR_P2,
  NRF24L01_REG_RX_ADDR_P3,
  NRF24L01_REG_RX_ADDR_P4,
  NRF24L01_REG_RX_ADDR_P5,
  NRF24L01_REG_TX_ADDR,
  NRF24L01_REG_RX_PW_P0,
  NRF24L01_REG_RX_PW_P1,
  NRF24L01_REG_RX_PW_P2,
  NRF24L01_REG_RX_PW_P3,
  NRF24L01_REG_RX_PW_P4,
  NRF24L01_REG_RX_PW_P5,
  NRF24L01_REG_FIFO_STATUS,
  NRF24L01_REG_ACK_PLD,
  NRF24L01_REG_TX_PLD,
  NRF24L01_REG_RX_PLD,
  NRF24L01_REG_RESERVED,
  NRF24L01_REG_DYNPD,
  NRF24L01_REG_FEATURE
} NRF24L01RegAddr_t;

/* registers */
typedef union
{
  struct
  {
    u8_t PRIM_RX     : 1;  /* RX/TX control:  0=PTX, 1=PRX         */
    u8_t PWR_UP      : 1;  /* 0=Power down, 1=Power up */
    u8_t CRCO        : 1;  /* CRC encoding: 0=8-bit, 1=16-bit */
    u8_t EN_CRC      : 1;  /* Enable CRC */
    u8_t MASK_MAX_RT : 1;  /* MAX_RT IRQ: 0=active, 1=off */
    u8_t MASK_TX_DS  : 1;  /* TX_DS IRQ: 0=active, 1=off */
    u8_t MASK_RX_DR  : 1;  /* RX_DR IRQ: 0=active, 1=off */
    u8_t reserved    : 1;
  } bits;
  u8_t byte;
} NRF24L01RegConfig_t;

typedef union
{
  struct
  {
    u8_t ENAA_P0     : 1;  /* Enable auto-ack data pipe 0         */
    u8_t ENAA_P1     : 1;  /* Enable auto-ack data pipe 1         */
    u8_t ENAA_P2     : 1;  /* Enable auto-ack data pipe 2         */
    u8_t ENAA_P3     : 1;  /* Enable auto-ack data pipe 3         */
    u8_t ENAA_P4     : 1;  /* Enable auto-ack data pipe 4         */
    u8_t ENAA_P5     : 1;  /* Enable auto-ack data pipe 5         */
    u8_t reserved    : 2;
  } bits;
  u8_t byte;
} NRF24L01RegEnAA_t;

typedef union
{
  struct
  {
    u8_t ERX_P0      : 1;  /* Enable data pipe 0         */
    u8_t ERX_P1      : 1;  /* Enable data pipe 1         */
    u8_t ERX_P2      : 1;  /* Enable data pipe 2         */
    u8_t ERX_P3      : 1;  /* Enable data pipe 3         */
    u8_t ERX_P4      : 1;  /* Enable data pipe 4         */
    u8_t ERX_P5      : 1;  /* Enable data pipe 5         */
    u8_t reserved    : 2;
  } bits;
  u8_t byte;
} NRF24L01RegEnRxAddr_t;

typedef union
{
  struct
  {
    u8_t AW          : 2;  /* RX/TX addr bytes: 0=illegal, 1=3, 2=4, 3=5 */
    u8_t reserved    : 6;
  } bits;
  u8_t byte;
} NRF24L01RegSetupAw_t;

typedef union
{
  struct
  {
    u8_t ARC    : 4;  /* Auto retransmit count: 0=disabled, 1=1, ..., 15=15 */
    u8_t ARD    : 4;  /* Auto retransmit delay: 0=250uS, 1=500uS, ..., 15=4ms */
  } bits;
  u8_t byte;
} NRF24L01RegSetupRetR_t;

typedef union
{
  struct
  {
    u8_t RF_CH       : 7;  /* Frequency channel */
    u8_t reserved    : 1;
  } bits;
  u8_t byte;
} NRF24L01RegRfCh_t;

typedef union
{
  struct
  {
    u8_t LNA_HCURR       : 1;  /* Setup LNA gain */
    u8_t RF_PWR          : 2;  /* Output power (dBm): 0=-18, 1=-12, 2=-6, 3=0 */
    u8_t RF_DR           : 1;  /* Air data rate: 0=1Mbps, 1=2Mbps */
    u8_t PLL_LOCK        : 1;  /* Force PLL lock signal:  set to zero */
    u8_t reserved        : 3;
  } bits;
  u8_t byte;
} NRF24L01RegRfSetup_t;

typedef union
{
  struct
  {
    u8_t TX_FULL      : 1;  /* TX FIFO full flag */
    u8_t RX_P_NO      : 3;  /* Data pipe # for RX_FIFO reading */
    u8_t MAX_RT       : 1;  /* Max # TX retransmits int flag */
    u8_t TX_DS        : 1;  /* Data sent int flag */
    u8_t RX_DR        : 1;  /* Data received int flag */
    u8_t reserved     : 1;
  } bits;
  u8_t byte;
} NRF24L01RegStatus_t;

typedef union
{
  struct
  {
    u8_t ARC_CNT      : 4;  /* Count of lost packets */
    u8_t PLOS_CNT     : 4;  /* Count of retransmitted packets */
  } bits;
  u8_t byte;
} NRF24L01RegObserveTx_t;

typedef union
{
  struct
  {
    u8_t CD           : 1;  /* Carrier detect */
    u8_t reserved     : 7;  
  } bits;
  u8_t byte;
} NRF24L01RegCd_t;

typedef union
{
  struct
  {
    u8_t RX_PW_PO     : 1;  /* Num bytes in data pipe 0 */
    u8_t reserved     : 7;  
  } bits;
  u8_t byte;
} NRF24L01RegRxPwP0_t;

typedef union
{
  struct
  {
    u8_t RX_PW_P1     : 1;  /* Num bytes in data pipe 1 */
    u8_t reserved     : 7;  
  } bits;
  u8_t byte;
} NRF24L01RegRxPwP1_t;

typedef union
{
  struct
  {
    u8_t RX_PW_P2     : 1;  /* Num bytes in data pipe 2 */
    u8_t reserved     : 7;  
  } bits;
  u8_t byte;
} NRF24L01RegRxPwP2_t;

typedef union
{
  struct
  {
    u8_t RX_PW_P3     : 1;  /* Num bytes in data pipe 3 */
    u8_t reserved     : 7;  
  } bits;
  u8_t byte;
} NRF24L01RegRxPwP3_t;

typedef union
{
  struct
  {
    u8_t RX_PW_P4     : 1;  /* Num bytes in data pipe 4 */
    u8_t reserved     : 7;  
  } bits;
  u8_t byte;
} NRF24L01RegRxPwP4_t;

typedef union
{
  struct
  {
    u8_t RX_PW_P5     : 1;  /* Num bytes in data pipe 5 */
    u8_t reserved     : 7;  
  } bits;
  u8_t byte;
} NRF24L01RegRxPwP5_t;

#define NRF24L01_FIFO_STATUS_RX_EMPTY     (0x01)
#define NRF24L01_FIFO_STATUS_RX_FULL      (0x02)
#define NRF24L01_FIFO_STATUS_TX_EMPTY     (0x10)
#define NRF24L01_FIFO_STATUS_TX_FULL      (0x20)
#define NRF24L01_FIFO_STATUS_TX_REUSE     (0x40)

typedef union
{
  struct
  {
    u8_t RX_EMPTY     : 1;  /* RX FIFO empty */
    u8_t RX_FULL      : 1;  /* RX FIFO full */
    u8_t reserved1    : 2;
    u8_t TX_EMPTY     : 1;  /* TX FIFO empty */
    u8_t TX_FULL      : 1;  /* TX FIFO full */
    u8_t TX_REUSE     : 1;  /* Reuse last transmitted packet */
    u8_t reserved2    : 1;
  } bits;
  u8_t byte;
} NRF24L01RegFifoStatus_t;

typedef union
{
  struct
  {
    u8_t DPL_P0       : 1;  /* Enable dynamic payload data pipe 0 */
    u8_t DPL_P1       : 1;  /* Enable dynamic payload data pipe 1 */
    u8_t DPL_P2       : 1;  /* Enable dynamic payload data pipe 2 */
    u8_t DPL_P3       : 1;  /* Enable dynamic payload data pipe 3 */
    u8_t DPL_P4       : 1;  /* Enable dynamic payload data pipe 4 */
    u8_t DPL_P5       : 1;  /* Enable dynamic payload data pipe 5 */
    u8_t reserved1    : 2;
  } bits;
  u8_t byte;
} NRF24L01RegDynPd_t;

typedef union
{
  struct
  {
    u8_t EN_DYN_ACK     : 1;  /* Enable W_TX_PAYLOAD_NOACK command */
    u8_t EN_ACK_PAY     : 1;  /* Enable payload with ACK */
    u8_t EN_DPL         : 1;  /* Enable dynamic payload length */
    u8_t reserved1      : 5;
  } bits;
  u8_t byte;
} NRF24L01RegFeature_t;

u8_t NRF24L01Init(void);
u8_t NRF24L01SendPacket(u8_t *pPacket, u8_t numBytes);
u8_t NRF24L01StartTransmitMode(void);
bool NRF24L01IsPacketTransmitted(void);
u8_t NRF24L01WriteFifo(u8_t *pPacket, u8_t numBytes);
u8_t NRF24L01InitiateTransmit(void);
u8_t NRF24L01EndTransmitMode(void);
u8_t NRF24L01ReceivePacket(u8_t *pPacket, u8_t maxBytes);
u8_t NRF24L01StartReceiveMode(void);
u8_t NRF24L01EndReceiveMode(void);
bool NRF24L01IsPacketReceived(void);
u8_t NRF24L01ReadFifo(u8_t *pPacket, u8_t maxBytes);

#endif


