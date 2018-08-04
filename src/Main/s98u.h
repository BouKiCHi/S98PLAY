/*********
 s98u.h by BouKiCHi 2016
 from s98x.h
 This file is no warranty, but free to use. 
 ********/

#ifndef __S98X_H__
#define __S98X_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// #define S98_READONLY

#define MAX_S98DEV 4

typedef struct {
    FILE *file;
    
    int time_nume; // 分子(デフォルトは10)
    int time_denom; // 分母(デフォルトは1000)
    int time_us; // us単位での1SYNC
    
    int def_denom;
    
    double s98_step; // S98の単位時間
    double sys_time; // 実際の時間
    double sys_step; // 実際の単位時間    
        
    int dump_ptr; // データ先頭
    int dump_loop; // ループ位置
    
    int loop_count; // ループカウント

    int dev_count;
    int dev_type[MAX_S98DEV];
    int dev_freq[MAX_S98DEV];
} S98CTX;


// 
#define S98_US 1000000

enum S98_DEVID
{
    S98_NONE = 0,
    S98_SSG = 1, // YM2149
    S98_OPN,    // YM2203
    S98_OPN2,   // YM2612
    S98_OPNA,   // YM2608
    S98_OPM,    // YM2151
    S98_OPLL,   // YM2413
    S98_OPL,    // YM3526
    S98_OPL2,   // YM3812
    S98_OPL3,   // YMF262
    S98_PSG,    // AY-3-8910
    S98_DCSG,   // DCSG
};

int OpenS98(S98CTX *ctx, const char *file);
void CloseS98(S98CTX *ctx);
int ReadS98(S98CTX *ctx);
long TellS98(S98CTX *ctx);
int GetTickS98(S98CTX *ctx);
int GetDeviceCountS98(S98CTX *ctx);
int GetDeviceTypeS98(S98CTX *ctx, int idx);
int GetDeviceFreqS98(S98CTX *ctx, int idx);
int ReadS98VV(S98CTX *ctx);
void LoopS98(S98CTX *ctx);


#define S98_CMD_SYNC 0xff
#define S98_CMD_NSYNC 0xfe
#define S98_CMD_END 0xfd

#ifdef __cplusplus
}
#endif


#endif

