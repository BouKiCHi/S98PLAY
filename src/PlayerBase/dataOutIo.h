// 非SPIモード (8x2 or 16)

// FASTIOでも定義が必要(11U35)
DigitalOut io01(DPIO_1, 0);
DigitalOut io02(DPIO_2, 0); 
DigitalOut io03(DPIO_3, 0); 
DigitalOut io04(DPIO_4, 0);

#ifndef USE_FASTIO
// ポート出力
#define DATA_HI io01 = 1
#define DATA_LO io01 = 0
#define CTRL_HI io02 = 1
#define CTRL_LO io02 = 0
#define SCK_HI io03 = 1
#define SCK_LO io03 = 0
#define RCK_HI io04 = 1
#define RCK_LO io04 = 0

// 出力設定
#define PORT_DIR

#endif

// I/O初期化
void dataOut::initialize_io() {
  PORT_DIR;
}

// 8bit 出力
void dataOut::byte_out(unsigned char data) {
  int i;

  for(i = 0; i < 8; i++) {
    if (data & 0x80) DATA_HI; else DATA_LO;

    data <<= 1;
    SCK_HI;
    SCK_LO;
  }
}

// 8x2ビット出力 or 16bit出力選択可能
void dataOut::shift_out(unsigned int ctrl, unsigned int data) {
  int i;
  // シフトレジスタ直列モード
  if (serial_out_mode) {
    // データ順
    // <- CCCCCCCC76543210

    byte_out(ctrl);
    byte_out(data);

    // ラッチ
    RCK_HI;
    RCK_LO;
  } else {
  // シフトレジスタ並列モード
    for(i = 0; i < 8; i++) {
      /* 2ビット分のデータをそれぞれ出力 */
      if (ctrl & 0x80) CTRL_HI; else CTRL_LO;
      if (data & 0x80) DATA_HI; else DATA_LO;

      ctrl <<= 1;
      data <<= 1;
      // シフトクロック
      SCK_HI;
      SCK_LO;
    }

    // ラッチ
    RCK_HI;
    RCK_LO;
  }
}


