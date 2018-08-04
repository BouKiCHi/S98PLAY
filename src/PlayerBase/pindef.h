#ifndef __PINDEF_H__
#define __PINDEF_H__

#ifdef TARGET_LPC1114

#define DPIO_1 dp13 // P1_4
#define DPIO_2 dp26 // P0_3
#define DPIO_3 dp17 // P1_8
#define DPIO_4 dp4 // P0_11

#ifdef USE_FASTIO
#define FASTIO_01 (1 << 4)  // DATA  P1_4 
#define FASTIO_02 (1 << 3)  // CTRL  P0_3
#define FASTIO_03 (1 << 8)  // SCLK  P1_8
#define FASTIO_04 (1 << 11) // LATCH P0_11

#define DATA_HI LPC_GPIO1->DATA |= FASTIO_01
#define DATA_LO LPC_GPIO1->DATA &= ~FASTIO_01
#define CTRL_HI LPC_GPIO0->DATA |= FASTIO_02
#define CTRL_LO LPC_GPIO0->DATA &= ~FASTIO_02
#define SCK_HI LPC_GPIO1->DATA |= FASTIO_03
#define SCK_LO LPC_GPIO1->DATA &= ~FASTIO_03
#define RCK_HI LPC_GPIO0->DATA |= FASTIO_04
#define RCK_LO LPC_GPIO0->DATA &= ~FASTIO_04

// 出力設定
#define PORT_DIR  LPC_GPIO0->DIR |= (FASTIO_02 | FASTIO_04); \
 LPC_GPIO1->DIR |= (FASTIO_01 | FASTIO_03)

#endif

// BUTTON
#define DP_PLAY dp24
#define DP_NEXT dp10
#define DP_PREV dp9

// SPI
#define DP_MOSI dp2
#define DP_MISO dp1
#define DP_SCLK dp6
#define DP_SCS  dp25

// I2C
#define DP_SDA dp5
#define DP_SCL dp27

// SERIAL
#define DP_TX USBTX
#define DP_RX USBRX

// LED
#define DP_LED1 LED1
#define DP_LED2 LED2

#else // TARGET_LPC1114

//  LPC11U35
#ifdef USE_FASTIO
#define FASTIO_01 (1 << 14)  // DATA  P0_14 
#define FASTIO_02 (1 << 13)  // CTRL  P0_13
#define FASTIO_03 (1 << 12)  // SCLK  P0_12
#define FASTIO_04 (1 << 11) // LATCH P0_11

#define DATA_HI LPC_GPIO->SET[0] = FASTIO_01
#define DATA_LO LPC_GPIO->CLR[0] = FASTIO_01
#define CTRL_HI LPC_GPIO->SET[0] = FASTIO_02
#define CTRL_LO LPC_GPIO->CLR[0] = FASTIO_02
#define SCK_HI LPC_GPIO->SET[0] = FASTIO_03
#define SCK_LO LPC_GPIO->CLR[0] = FASTIO_03
#define RCK_HI LPC_GPIO->SET[0] = FASTIO_04
#define RCK_LO LPC_GPIO->CLR[0] = FASTIO_04

// 出力設定
#define PORT_DIR  LPC_GPIO->DIR[0] |= (FASTIO_02 | FASTIO_04); \
LPC_GPIO->DIR[0] |= (FASTIO_01 | FASTIO_03)

#endif // USE_FASTIO

#define DPIO_1 P0_14 
#define DPIO_2 P0_13 
#define DPIO_3 P0_12 
#define DPIO_4 P0_11 


// LED
#define DP_LED1 P0_7 // QuickStart Board
//#define DP_LED1 P0_20
#define DP_LED2 P0_21

// BUTTON
#define DP_PLAY P0_1
#define DP_NEXT P0_16
#define DP_PREV P0_15

// SPI
#define DP_MOSI P0_9
#define DP_MISO P0_8
#define DP_SCLK P0_10
#define DP_SCS  P0_2

// I2C
#define DP_SDA P0_5
#define DP_SCL P0_4

// SERIAL
#define DP_TX P0_19
#define DP_RX P0_18

#endif // TARGET_LPC1114

#endif



