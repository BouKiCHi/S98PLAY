#include "dataOut.h"

#ifndef USE_SPI
#include "dataOutIo.h"
#else
#include "dataOutSpi.h"
#endif


// (pinname, mode)
DigitalIn playButton(DP_PLAY, PullUp);
DigitalIn nextButton(DP_NEXT, PullUp);
DigitalIn prevButton(DP_PREV, PullUp);

// LED
DigitalOut led1(DP_LED1, 0);
DigitalOut led2(DP_LED2, 0);

// SPI: MOSI, MISO, SCLK, CS
SDFileSystem sd(DP_MOSI, DP_MISO, DP_SCLK, DP_SCS, "sd");


// ボタンが離されるのを待つ
void dataOut::wait_key_up(void) {
  while(!nextButton || !playButton || !prevButton);
}

// ボタンの入力を待つ
void dataOut::wait_key_down(void) {
  while(nextButton && playButton && prevButton);
}

// キー入力
int dataOut::get_key(void) {
  int key = 0;
  if (!nextButton) key |= SW_NEXT;
  if (!playButton) key |= SW_PLAY;
  if (!prevButton) key |= SW_PREV;
  return key;
}

// キー開放待ち
void dataOut::wait_release_key(void) {
  wait_key_up();
  wait(0.1);
}

// キー待ち
int dataOut::wait_any_key(void) {
  wait_key_down();
  int key = get_key();

  wait(0.1);
  wait_release_key();
  return key;
}

// LEDリセット
void dataOut::reset_led(void) {
  led1 = 0;
  led2 = 0;
}

void dataOut::set_led1(int val) {
  led1 = val;
}

void dataOut::set_led2(int val) {
  led2 = val;
}

void dataOut::change_led1(void) {
  led1 = !led1;
}

void dataOut::change_led2(void) {
  led2 = !led2;
}



/* NBV2互換出力 */
void dataOut::register_out_base(int addr, int data, int select) {
  /* アドレスを出力 */
  /* A0をローにして待つ */
  int ctrl = ACTLOW;

  /* 裏レジスタ */
  if (addr >= 0x100) ctrl |= CTL;
  addr &= 0xff;

  SHIFTOUT(ctrl, 0x00);
  SHIFTOUT(ctrl & ~(select | WR), addr);
  SHIFTOUT(ctrl, addr);

  /* チップ処理待ち */

  /* データを出力 */
  /* A0をハイにして待つ */
  ctrl |= A0;
  SHIFTOUT(ctrl, 0x00);
  SHIFTOUT(ctrl & ~(select | WR), data);
  SHIFTOUT(ctrl, data);
}

/* デバイス同時に出力 */
void dataOut::dev_out(int flag,int addr,int value) {
  if (flag & 1) psg_out(addr,value);
  if (flag & 2) fm1_out(addr,value);
  if (flag & 4) fm2_out(addr,value);
}

/* ミュート */
void dataOut::board_mute(void) {
  int i;

  /* PSG初期化 */
  dev_out(DEV_ALL,0x00,0);
  dev_out(DEV_ALL,0x01,0);

  dev_out(DEV_ALL,0x06, 0x00);
  dev_out(DEV_ALL,0x07, 0x3f); // ALL OFF
  dev_out(DEV_ALL,0x08, 0x00); // CH.A 0
  dev_out(DEV_ALL,0x09, 0x00); // CH.B 0
  dev_out(DEV_ALL,0x0a, 0x00); // CH.C 0

  /* MUTE(disable) */
  for(i = 0x20; i < 0x28; i++) {
    dev_out(DEV_FM,i,0x00);
  }

  // KEYOFF
  for(i = 0x00; i < 0x08; i++) {
    dev_out(DEV_FM,0x08, i & 0x07);
  }

  // FORCE RELEASE
  for(i= 0x00; i < 0x20; i++) {
    dev_out(DEV_FM,0xE0 + i, 0x0f);
  }

  // OPLL ミュート
  for(i= 0x00; i <= 0x08; i++) {
    dev_out(DEV_FM, 0x20 + i, 0x00);
    dev_out(DEV_FM, 0x30 + i, 0x0f);
  }
  
  // OPL3 ミュート
  for(i= 0x00; i <= 0x08; i++) {
    fm1_out(0xb0 + i, 0x00);
    fm1_out(0x1b0 + i, 0x00);
  }
  // OPL3/OPNA共通
  for(i= 0x00; i <= 0x1e; i++) {
    dev_out(DEV_FM, 0x40 + i, 0x00);
    dev_out(DEV_FM, 0x140 + i, 0x00);
  }
  // OPNAキーOFF
  for(i= 0x00; i <= 0x3; i++) {
    dev_out(DEV_FM,0x28,i);
    dev_out(DEV_FM,0x128,i);
  }
}

/* ボード初期化 */
void dataOut::board_init(void) {
  wait_ms(20);
  /* ICLのみをLOW(アクティブ)にする */
  SHIFTOUT(ACTLOW & ~(ICL), 0);
  wait_ms(150);

  /* 元に戻す */
  SHIFTOUT(ACTLOW, 0);
  wait_ms(10);
}

/* ボード起動 */
void dataOut::board_boot(void) {
    // シフトレジスタの初期化
    SHIFTOUT(ACTLOW & ~(ICL), 0x00);
    // 立ち上がり待ち
    wait_ms(20);
    SHIFTOUT(ACTLOW, 0x00);
}

dataOut::dataOut() {
    serial_out_mode = false;
    initialize_io();
    board_boot();

}



