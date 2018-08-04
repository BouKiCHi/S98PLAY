/*********
 S98micro
 This file is no warranty, but free to use.
 ********/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "s98u.h"

typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned long  dword;

// 変数読み出し(DWORD)
static dword ReadDWORD(byte *p) {
  return ((dword)p[0]) | ((dword)p[1])<<8 | ((dword)p[2])<<16 | ((dword)p[3])<<24;
 }

// ファイルを開く
int OpenS98(S98CTX *ctx,const char *file) {
  int i;
  byte hdr[0x80];
    
  memset(ctx, 0, sizeof(S98CTX));

  ctx->file = fopen(file, "rb");
  
  if (!ctx->file) {
    printf("File open error:%s\n", file);
    return -1;
  }

  fread(hdr, 0x20, 1, ctx->file);
  
  // フォーマット確認
  if (memcmp(hdr, "S98", 3) != 0) {
    printf("Invalid S98 header!!\n");
    fclose(ctx->file);
    return -1;
  }
  
  // バージョン確認
  // ctx->version = 1;
  // if (hdr[3] == '3')
  //   ctx->version = 3;

  // ループカウント
  ctx->loop_count = 0;
  
  // タイミング 
  ctx->time_nume = (int)ReadDWORD(hdr + 0x04);
  ctx->time_denom = (int)ReadDWORD(hdr + 0x08);
  
  // デフォルト値の設定
  if (ctx->time_nume == 0) ctx->time_nume = 10;
  if (ctx->time_denom == 0) ctx->time_denom = 1000;
  
  // ウェイトをus単位に変換する
  ctx->time_us = (int)(((double)ctx->time_nume * S98_US) / ctx->time_denom);
  
  // データ位置の取得
  ctx->dump_ptr = (int)ReadDWORD(hdr + 0x14);
  ctx->dump_loop = (int)ReadDWORD(hdr + 0x18);

  // デバイスカウント  
  ctx->dev_count = (int)ReadDWORD(hdr + 0x1c);
  
  // デバイス情報の取得
  for(i = 0; i < ctx->dev_count && i < MAX_S98DEV; i++) {
    fread(hdr, 0x10, 1, ctx->file);
    ctx->dev_type[i] = (int)ReadDWORD(hdr + 0x00);
    ctx->dev_freq[i] = (int)ReadDWORD(hdr + 0x04);
  }

  // データ先頭までシーク
  fseek(ctx->file, ctx->dump_ptr, SEEK_SET); 
  return 0;
}

// ファイルを閉じる
void CloseS98(S98CTX *ctx)
{
  if (!ctx) return;
  
  if (ctx->file) {
      fclose(ctx->file);
      ctx->file = NULL;
  }  
}

// データの読み出し
int ReadS98(S98CTX *ctx) {
  return fgetc(ctx->file);
}

// ループ開始
void LoopS98(S98CTX *ctx) {
  ctx->loop_count++;
  fseek(ctx->file, ctx->dump_loop, SEEK_SET); 
}


// ファイルポインタの位置を取得
long TellS98(S98CTX *ctx) {
  return ftell(ctx->file);
}

// ティックの読み出し
int GetTickS98(S98CTX *ctx) {
    return ctx->time_us;
}


// デバイス数の読み出し
int GetDeviceCountS98(S98CTX *ctx) {
    return ctx->dev_count;
}

// デバイスタイプの読み出し
int GetDeviceTypeS98(S98CTX *ctx, int idx) {
    return ctx->dev_type[idx];
}

// デバイス周波数の読み出し
int GetDeviceFreqS98(S98CTX *ctx, int idx) {
    return ctx->dev_freq[idx];
}


// 可変長データの読み出し
int ReadS98VV(S98CTX *ctx) {
  int out = 0;
  int v = 0;
  int s = 0;

  do {
    v = fgetc(ctx->file);
    out |= (v & 0x7f) << s;
    s += 7;
  }while(v & 0x80);
  
  return out + 2;
}


