/*
 * lcd.cpp : AQM0802A mini library
 *
 *  Based on LCD module "AQM0802A-RN-GBW" sample program
 *  Copyright (c) 2013 Yoshihiro TSUBOI
 *
 *  Original Arduino version was developed by
 *  Copyright (c) 2013 Masahiro WAKAYAMA at SWITCH SCIENCE
 *
 *  Released under the MIT License: http://mbed.org/license/mit
 *
 */

#include "lcd.h"

LCD::LCD() {
  i2c = new I2C(DP_SDA, DP_SCL); // sda, scl
}

#define LCD_ADDR 0x7C

void LCD::cmd(char x) {
  char data[2];
  data[0] = 0x00; // CO = 0,RS = 0
  data[1] = x;
  i2c->write(LCD_ADDR, data, 2);
}

void LCD::contdata(char x) {
  char data[2];
  data[0] = 0xC0; //0b11000000 CO = 1, RS = 1
  data[1] = x;
  i2c->write(LCD_ADDR, data, 2);
}

void LCD::lastdata(char x){
  char data[2];
  data[0] = 0x40; //0b11000000 CO = 0, RS = 1
  data[1] = x;
  i2c->write(LCD_ADDR, data, 2);
}


void LCD::setContrast(unsigned char c) {
  cmd(0x39);
  cmd(0x70 | (c & 0x0f)); // contrast Low
  cmd(0x5C | ((c >> 4) & 0x03)); // contast High/icon/power
  cmd(0x38);
}

void LCD::init() {
  int cont = 0;
  wait(0.04);
  // LCD initialize
  cmd(0x38); // function set
  cmd(0x39); // function set
  cmd(0x04); // EntryModeSet
  cmd(0x14); // interval osc
  cmd(0x70 | (cont & 0xF)); // contrast Low
  cmd(0x5C | ((cont >> 4) & 0x3)); // contast High/icon/power
  cmd(0x6C); // follower control
  wait(0.2);
  cmd(0x38); // function set
  cmd(0x0C); // Display On
  cmd(0x01); // Clear Display
  wait(0.2); // need additional wait to Clear Display

  setContrast(36);
}


// 文字と空白で1行を埋める
void LCD::printStrFill(const char *s){
  int idx = 0;
  for(idx = 0; idx < 8; idx++) {
    if(idx < 7) contdata(*s); else lastdata(*s);
    if (*s) s++;
  }
}

void LCD::printStr(const char *s) {
  int idx = 0;
  while(*s && idx < 8) {
    if(*(s + 1)) contdata(*s); else lastdata(*s);
    s++;
    idx++;
  }
}

void LCD::setCursor(unsigned char x,unsigned char y) {
  cmd(0x80 | (y * 0x40 + x));
}

void LCD::printStrY(int y,const char *s) {
  setCursor(0, y);
  printStrFill(s);
}

void LCD::printStrYscr(int y,const char *s) {
  int cnt = strlen(s) - 7;
  if (cnt <= 0)
    cnt = 1;

  for(int i = 0; i < cnt; i++) {
    setCursor(0, y);
    printStr(s + i);
    wait(0.5);
  }
}


void LCD::printStr2(const char *s, const char *s2) {
  setCursor(0,0);
  printStrFill(s);
  setCursor(0,1);
  printStrFill(s2);
}

void LCD::cls(void) {
    setCursor(0, 0);
    printStrFill("");
    setCursor(0, 1);
    printStrFill("");
}



