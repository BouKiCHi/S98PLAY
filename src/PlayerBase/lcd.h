#ifndef __LCD_H__
#define __LCD_H__

#include "mbed.h"
#include "pindef.h"

class LCD {
public:
    LCD();

    void printStr(const char *s);
    void setContrast(unsigned char c);
    void setCursor(unsigned char x,unsigned char y);
    void cls(void);
    void init(void);
    void printStr2(const char *s, const char *s2);
    void printStrY(int y,const char *s);
    void printStrYscr(int y,const char *s);

protected:
    void cmd(char x);
    void contdata(char x);
    void lastdata(char x);
    void printStrFill(const char *s);
    I2C *i2c;
};

#endif



