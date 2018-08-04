#ifndef __DATAOUT_H__
#define __DATAOUT_H__

#include "mbed.h"

#include "config.h"
#include "pindef.h"

#include "SDFileSystem.h"

// キーコード
#define SW_PLAY 1
#define SW_NEXT 2
#define SW_PREV 4


/* 制御信号定義 */
#define CS_FM3 (1 << 0)
#define CS_PSG (1 << 1)
#define CS_FM1 (1 << 2)
#define CS_FM2 (1 << 3)
#define A0     (1 << 4)
#define WR     (1 << 5)
#define ICL    (1 << 6)
#define CTL    (1 << 7)

/* アクティブローの制御信号 */
#define ACTLOW (CS_PSG | CS_FM1 | CS_FM2 | CS_FM3 | WR | ICL)
/* 出力 */
#define psg_out(addr, data) register_out_base(addr, data, CS_PSG)
#define fm1_out(addr, data) register_out_base(addr, data, CS_FM1)
#define fm2_out(addr, data) register_out_base(addr, data, CS_FM2)

/* 音源出力 */
#define write_psg psg_out
#define write_opm1 fm1_out
#define write_opm2 fm2_out

#define DEV_ALL 7
#define DEV_FM 6

#define SHIFTOUT shift_out

class dataOut {
public:
  SPI *spi;
  bool serial_out_mode;

  void wait_key_up(void);
  void wait_key_down(void);
  int get_key(void);
  void wait_release_key(void);
  int wait_any_key(void);

  void reset_led(void);
  void set_led1(int val);
  void set_led2(int val);
  void change_led1(void);
  void change_led2(void);


  void initialize_io();
  void byte_out(unsigned char data);
  void shift_out(unsigned int ctrl, unsigned int data);

  void register_out_base(int addr, int data, int select);
  void dev_out(int flag,int addr,int value);
  void board_mute(void);
  void board_init(void);
  void board_boot(void);

  dataOut();
};

#endif



