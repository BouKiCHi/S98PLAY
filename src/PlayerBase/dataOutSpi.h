// SPIモード
// 16bitモード


// I/O初期化
void dataOut::initialize_io() {
  PORT_DIR;
}

void dataOut::byte_out(unsigned char data) {}

// 16bit 出力
void dataOut::shift_out(unsigned int ctrl, unsigned int data) {
  spi->write(ctrl);
  spi->write(data);

  // byte_out(ctrl);
  // byte_out(data);

  // LATCH
  RCK_HI;
  RCK_LO;
}


