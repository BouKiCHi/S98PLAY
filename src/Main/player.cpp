//
// S98PLAY : S98 File Player for mbed
//
// example to write :
// ./lpc21isp -bin file.bin /dev/cu.usbserial-??? 115200 12000
//

#include <stdio.h>

#include "mbed.h"
#include "player.h"

#define PRG_NAME "S98PLAY"
#define PRG_VER_BASE "V1.11"

#ifndef USE_SPI
#define PRG_VER PRG_VER_BASE
#else
#define PRG_VER PRG_VER_BASE "S"
#endif

// コンストラクタ
player::player() : playerBase(".S98") {
  memset(&songContext, 0, sizeof(songContext));
}

int player::song_update() {
  while(1) {
    /* コマンドの読み出し */
    int cmd = ReadS98(&songContext);
    if (cmd == EOF) return EOF;

    if (cmd < 0x80) {
      command_write(cmd);
    } else {
      if (command_control(cmd)) return 0;
    }
    if (eofFlag) return EOF;
  }
}

bool player::command_write(int cmd) {
  int a = ReadS98(&songContext); // Address
  int v = ReadS98(&songContext); // Data

  int id = cmd / 2;
  
  if (cmd & 1) a += 0x100;

  switch(id) {
    case 0:
      write_opm1(a, v);
    break;
    case 1:
      write_opm2(a, v);
    break; 
    case 2:
      write_psg(a, v);
  }
  return false;
}

bool player::command_control(int cmd) {
  int tmp;
  switch (cmd) {
      case S98_CMD_SYNC:
        add_count_us(GetTickS98(&songContext));
        return true;
      case S98_CMD_NSYNC:
        tmp = GetTickS98(&songContext);
        tmp *= ReadS98VV(&songContext);
        add_count_us(tmp);
        return true;      
      case S98_CMD_END:
        if (songContext.dump_loop && songContext.loop_count < maxLoopCount) 
          LoopS98(&songContext); 
        else 
          eofFlag = true;
      break;
  }
  return false;
}

int player::song_open(const char *filename) {
  if (OpenS98(&songContext, filename) < 0) return -1;
  maxLoopCount = 3;
  return 0;
}

void player::song_close() {
  CloseS98(&songContext);
}

const char *player::get_program_name() {
  return PRG_NAME;
}

const char *player::get_program_version() {
  return PRG_VER;
}

