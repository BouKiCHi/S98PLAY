#include "playerBase.h"

#define WAIT_US 10000 // 10ms
#define SEC_US 1000000 // 1sec
#define NEXT_MAX_US 300000 // 300ms

// コンストラクタ
playerBase::playerBase(const char *ext) : dataOut() {
  lcd.init();

#ifdef USE_SPI
  spi = sd.getSPI();
#endif
  maxEntries = 0;
  songIndex = 0;
  totalSec = 0;

  songExtension = ext;
  workingDirectory[0] = 0;
  playlist[0] = 0;
  songFile[0] = 0;
  strcpy(workingDirectory,"/sd");
}

// 実行
void playerBase::run() {
  board_mute();

  int key = put_title();
  // NEXTボタンでメニュー表示
  if (key & SW_NEXT) menu_mode();

  // 再生モード
  while(1) play_mode();
}


// タイトル表示
int playerBase::put_title() {
  reset_control_data();

  int key = get_key();
  if (key) return key;

#ifndef DEBUG
  lcd.printStr2(get_program_name(), get_program_version());
#else
  lcd.printStr2(__TIME__, get_program_version());
#endif
  reset_timer();
  int loopCount = 0;

  // 1500msまでループする
  while(!stopFlag) {
    /* ウエイトの処理 */
    wait_next();
    key = get_key();
    if (key) return key;

    if (loopCount >= 150) return 0;
    add_count_us(10000); // 10ms
    loopCount++;
  }

  return 0;
}

const char *playerBase::get_program_name() {
  return "";
}

const char *playerBase::get_program_version() {
  return "";
}

void playerBase::reset_timer(void) {  
  waitTimer.reset();
  waitTimer.start();

  diffUs = 0;
  countUs = 0;
  totalUs = 0;
  totalSec = 0;

  updateDisplay = true;
}

void playerBase::reset_control_data(void) {
  prevFlag = false;
  stopFlag = false;
  waitSkipFlag = false;
  nextButtonCount = 0;
}

void playerBase::add_count_us(int us) {
  countUs += us;
}

void playerBase::calc_next(int us) {
  // 差分を計算
  diffUs += us;
  if (countUs <= diffUs) {
    diffUs -= countUs;
    totalUs += countUs;
    countUs = 0;
  }

  if (totalUs < SEC_US) return;

  updateDisplay = true;
  while(totalUs >= SEC_US) {
      totalSec++;
      totalUs -= SEC_US;
  }
}

// 返り値:経過実時間(us)
int playerBase::wait_next(void) {
  // スキップであればカウント分を進める
  int us = countUs;

  if (countUs < WAIT_US) {
    us = waitTimer.read_us();
    waitTimer.reset();
    diffUs += us;
    return us;
  }

  if (!waitSkipFlag) {
    while((us = waitTimer.read_us()) + diffUs < countUs);
    waitTimer.reset();
  }

  calc_next(us);
  return us;
}

bool playerBase::check_control_button(int us) {
    int key = get_key();

    // NEXTが押されている
    if (key & SW_NEXT) {
      if (nextButtonCount < NEXT_MAX_US)
        nextButtonCount += us;
      else {
        // 早送り
        nextButtonCount = NEXT_MAX_US;
        waitSkipFlag = true;
      }
    } else {
      // NEXTが離されている
      waitSkipFlag = false;

      // 一瞬だけボタンが押された場合、次の曲へ
      if (nextButtonCount > 0 && nextButtonCount < NEXT_MAX_US) {
        nextButtonCount = 0;
        return true;
      }

      // 早送りの終了
      if (nextButtonCount == NEXT_MAX_US) {
          nextButtonCount = 0;
      }
    }
    // PREVが押された
    if (key & SW_PREV) {
      prevFlag = true;
      return true;
    }

    // PLAYが押された
    if (key & SW_PLAY) {
      stopFlag = true;
      return true;
    }
    return false;
}



// エラー表示
void playerBase::error_sdcard(void) {
  lcd.printStr2("SDCARD", "ERROR!");
  while(1);
}

// COM mode
void playerBase::loop_for_com(void) {
  int sw, val;
  int adr = 0x00;
  int baud = 9600;
  char buf[16];
  Serial pc(DP_TX, DP_RX);

  lcd.cls();
  lcd.printStrY(0, "COM MODE");

  sprintf(buf, "%-8d", baud);
  lcd.printStrY(1, buf);

  pc.printf("COM\n");

  // タイマースタート
  Timer t;
  t.start();
  val = 0;

  while(1) {
    sw = pc.getc();

    // 0は同期リセット
    if (sw == 0x00) continue;

    if (sw >= 1 && sw <= 3) {
      adr = pc.getc();
      val = pc.getc();
    }

    switch(sw) {
      case 1:
        fm1_out(adr, val);
      break;
      case 2:
        fm2_out(adr, val);
      break;
      case 3:
        psg_out(adr, val);
      break;
      case 0x0f:
        // 通信速度設定
        val = pc.getc();
        baud = (9600 * val);
        sprintf(buf, "%-8d", baud);
        lcd.printStrY(1, buf);

        pc.baud(baud);
      break;
      case 0x10:
        //
        val = pc.getc();
        val = (val * 10000);
        while(t.read_us() < val);
        pc.putc('.');
        t.reset();
      break;
      case 0x11:
        t.reset();
      break;
    }
    // pc.printf("sw = %02x, adr = %02x, val = %02x\n", sw, adr, val);
  }
}


// chk_isdir
// ディレクトリか否か
// return : bool
// false if not directory

bool playerBase::chk_isdir(const char *cwd, const char *dir) {
  char tmp[256];
  sprintf(tmp,"%s/%s", cwd, dir);

  DIR *dp = opendir(tmp);
  if (!dp) return false;

  closedir(dp);
  return true;
}


// get_file_entry
// index : index of list , -1 = return number of entries
// is_mode : 0 = normal mode
// mode = MODE_GETDIR | MODE_GETLIST | MODE_FILE
// return : int
// -1 = not found or error

int playerBase::get_file_entry(int index, int mode) {
  // return -1 if entry is zero.
  int count = 0;
  DIR *dp = opendir(workingDirectory);

  songFile[0] = 0;

  if (!dp) return -1;

  struct dirent *dent = NULL;

  while(1) {
    // エントリの読み出し
    dent = readdir(dp);

    // 失敗
    if (!dent) break;

    // リソースか隠しファイル
    if (dent->d_name[0] == '.') continue;

    // 拡張子の取得
    char *ext = strrchr(dent->d_name, '.');

    switch(mode) {
      // ディレクトリモード
      case MODE_GETDIR:
        // ディレクトリでなければ継続
        if (!chk_isdir(workingDirectory, dent->d_name)) continue;
      break;

      // プレイリストモード
      case MODE_GETLIST:
        // リストでなければ無視
        if (!ext || strcasecmp(ext, EXT_LIST) != 0) continue;
      break;

      // ファイルモード
      case MODE_FILE:
        // NLGファイルでなければ継続
        if (!ext || strcasecmp(ext, songExtension) != 0) continue;
      break;
    }

    // カウントアップ
    count++;

    // カウントモードかカウントがindex未満で継続
    if (index < 0 || count <= index) continue;

    // ファイル名をコピーして終了
    strcpy(songFile, dent->d_name);
    break;
  }
  closedir(dp);
  return count;
}

// 表示
void playerBase::disp_time() {
  char buf[16];
  sprintf(buf, "%02d:%02d", totalSec / 60, totalSec % 60);

  lcd.setCursor(3,1);
  lcd.printStr(buf);

  updateDisplay = false;
}

// 情報の表示
void playerBase::show_info(int files) {
  char buf[16];

  Timer t;
  int result_us = 0;
  t.reset();
  t.start();

  // 実際の書き込み時間を計測する
  fm1_out(0x20, 0x00);
  fm1_out(0x20, 0x00);
  fm1_out(0x20, 0x00);
  fm1_out(0x20, 0x00);
  fm1_out(0x20, 0x00);

  // 経過時間を得る
  result_us = t.read_us();
  t.stop();

  // 平均値
  result_us /= 5;

  // 結果表示
  printf("result_us=%dus\n", result_us);
  sprintf(buf, "R:%dus", result_us);
  lcd.printStrY(1, buf);
  wait(3);

  // コンパイル時の日付
  sprintf(buf, "%8s", __DATE__);
  lcd.printStrYscr(1, buf);
  wait(3);

  if (files < 0) lcd.printStrY(1, "NO FILES");
  else {
    sprintf(buf, "%3dfiles", files);
    lcd.printStrY(1, buf);
  }
  wait(3);
  lcd.cls();
}

// ビットテスト

void playerBase::bit_test() {
  char buf[9];
  lcd.printStrY(0, "BIT_TEST");

  wait_key_up();

  int pin = 0;
  int mode = 0;
  char sig;

  while(1) {
    // 表示
    if (mode & 1) sig = 'C'; // コントロール
    else sig = 'D'; // データ

    if (mode & 2) strcpy(buf,"00");
    else sprintf(buf, "%c%d", sig, pin);

    lcd.printStrY(1, buf);

    // 出力
    if (mode & 2) { SHIFTOUT(0xff, 0x00); } 
    else {
      int bit = (1 << pin);
      if (mode & 1) SHIFTOUT(0xff & ~(bit), 0); // コントロール
      else SHIFTOUT(0xff, bit); // データ
    }

    // キー待ち
    int key = wait_any_key();

    // 再生
    if (key & SW_PLAY) {
      mode ^= 2;
      continue;
    }
    // 次
    if (key & SW_NEXT) {
      pin++;
      if (pin > 7) {
        pin = 0;
        mode = (mode + 1) & 1;
      }
    }
    // 前
    if (key & SW_PREV) {
      pin--;
      if (pin < 0) {
        pin = 7;
        mode = (mode - 1) & 1;
      }
    }
  }
}

// メニュー文字列
const char *menu_modes[] = {
  "FILE", // 0
  "DIR ",  // 1
  "LIST", // 2
  "INFO", // 3
  "COM ",  // 4
  "TEST", // 5
  "Sx16", // 6
};

//
// ファイル選択表示
void playerBase::display_file_selector(int mode) {
  char buf[16];
  char buf_mode[16];

  const char *mode_name;
  switch(mode) {
    case MODE_GETLIST:
      mode_name = menu_modes[MENU_LIST];
    break;
    case MODE_GETDIR:
      mode_name = menu_modes[MENU_DIR];
    break;
    default:
      mode_name = menu_modes[MENU_FILE];
  }

  sprintf(buf_mode, "%s %03d", mode_name, songIndex + 1);
  strncpy(buf, songFile, 8);
  buf[8] = 0;

  // 表示
  lcd.printStr2(buf, buf_mode);
}



//
// ファイル/ディレクトリ選択
// mode = MODE_GETLIST | MODE_GETDIR | MODE_FILE
//
int playerBase::file_select(int mode) {
  // ファイル数を得る
  int files = get_file_entry(-1, mode);
  maxEntries = files - 1;
  get_file_entry(songIndex, mode);
  display_file_selector(mode);

  wait_release_key();

  while(1) {
    get_file_entry(songIndex, mode);
    display_file_selector(mode);
    int key = wait_any_key();

    // 次のエントリ
    if (key & SW_NEXT) {
      if (songIndex < maxEntries) songIndex++;
      else songIndex = 0;
    }

    // 前のエントリ
    if (key & SW_PREV) {
      if (songIndex > 0) songIndex--;
      else songIndex = maxEntries;
    }
    // 再生ボタンを押した
    if (key & SW_PLAY) break;
  }

  return songIndex;
}

// get_playlist()
// index = リスト位置, -1でエントリ数を返す
int playerBase::get_playlist(int index) {
  int count = 0;
  FILE *fp = fopen(playlist, "r");

  songFile[0] = 0;

  if (!fp) return -1;

  while(1) {
    char *p = fgets(songFile, MAX_FILENAME, fp);

    // EOFなので終了
    if (!p) break;

    int len = strlen(songFile);

    while(len > 0 && (unsigned char)songFile[len - 1] < 0x20) {
      songFile[len - 1] = 0x00;
      len--;
    }

    // 空行は飛ばす
    if (!len) continue;
    count++;

    // カウントモードかカウントがindex未満で継続
    if (index < 0 || count <= index) continue;
    else break;
  }

  fclose(fp);

  // ライブラリバグ対策
#if defined(__MICROLIB) && defined(__ARMCC_VERSION)
  free(fp);
#endif

  return count;
}

// パスの作成
void playerBase::make_path(char *path, const char *dir, const char *file) {
  sprintf(path, "%s/%s", dir, file);
}


// 再生モード
void playerBase::play_mode(void) {
  int files = -1;

  // プレイリストモードか否か
  if (playlist[0]) files = get_playlist(-1);
  else files = get_file_entry(-1, MODE_FILE);

  // エラー表示
  if (files < 0) error_sdcard();

  maxEntries = files - 1;

  bool repeat_flag = false;

  // ファイルが無い
  if (files < 1) {
      menu_mode();
      return;
  }

  // 再生モードループ
  while(1) {
    char path[MAX_PATH];
    char buf[16];

    // プレイリストかどうか？
    if (playlist[0]) get_playlist(songIndex);
    else get_file_entry(songIndex, MODE_FILE);

    // フルパスを作成
    make_path(path, workingDirectory, songFile);

    // 曲番号
    sprintf(buf, "%2d ", songIndex + 1);

    // ファイル名表示
    lcd.cls();
    lcd.printStrY(0, songFile);
    lcd.printStrY(1, buf);

    // 再生開始
    if (play_file(path) < 0) {
        menu_mode();
        return;
    }
    board_mute();

    // キー開放待ち
    wait_release_key();


    // 再生ボタンが押された
    if (stopFlag) {
      // ストップ表示
      lcd.printStrY(1, "  STOP  ");

      stopFlag = false;
      prevFlag = false;
      totalSec = 0;

      // ボード初期化
      board_init();

      // ファイル選択
      file_select(MODE_FILE);
      continue;
    }

    // 前の曲を再生
    if (prevFlag) {
      prevFlag = false;

      // 同じ曲を再生
      if (totalSec >= 2) continue;

      if (songIndex > 0) songIndex--;
      else songIndex = maxEntries;

      continue;
    }

    // 繰り返しではない
    if (!repeat_flag) {
      if (songIndex < maxEntries) songIndex++;
      else songIndex = 0;
    }
    repeat_flag = false;
  }
}

// ファイルを再生する
int playerBase::play_file(const char *filename) {
  if (song_open(filename) < 0) return -1;
  song_play();
  song_close();
  return 0;
}

int playerBase::song_open(const char *filename) {
  return 0;
}

void playerBase::song_close() {
}

int playerBase::song_play() {
  reset_control_data();
  reset_timer();
  reset_led();
  disp_time();
  ledCount1 = ledCount2 = 0;
  eofFlag = false;
  while(!stopFlag) {
    int us = wait_next();
    if (check_control_button(us)) return 0;
    if (updateDisplay) disp_time();
    if (song_update() == EOF) return EOF;
  }
  return 0;
}

int playerBase::song_update() {
  return 0;
}


// メニュー選択表示
void playerBase::menu_disp(int sel) {
  char buf[16];

  // 表示
  lcd.printStrY(0, "MENU SEL");
  sprintf(buf, "%02d %-4s", sel, menu_modes[sel]);
  lcd.printStrY(1, buf);
}

// モード選択
int playerBase::menu_select() {
  int count = 0;
  menu_disp(0);
  wait_key_up();

  while(1) {
    int key = wait_any_key();

    if (key & SW_NEXT) {
      if (count < MENU_MAX) count++;
      else count = 0;
    }

    if (key & SW_PREV) {
      if (count > 0) count--;
      else count = MENU_MAX;
    }

    if (key & SW_PLAY) break;

    menu_disp(count);
  }

  return count;
}

// メニューモード
void playerBase::menu_mode(void) {
  int files = 0;
  char path[MAX_PATH];

  menu_start:

  // ボードの初期化
  board_init();

  int sw = menu_select();
  switch(sw) {
    // ファイル選択
    case MENU_FILE:
      file_select(MODE_FILE);
    break;
    // ディレクトリ選択
    case MENU_DIR:
      file_select(MODE_GETDIR);
      // パスを結合し、インデックスを初期化
      make_path(path, workingDirectory, songFile);
      strcpy(workingDirectory, path);
      songIndex = 0;
    break;
    // プレイリスト選択
    case MENU_LIST:
        file_select(MODE_GETLIST);
    break;
    // 情報モード
    case MENU_INFO:
      files = get_file_entry(-1, MODE_FILE);
      show_info(files);
      goto menu_start;
    // 通信モード
    case MENU_COM:
      loop_for_com();
    break;
    // テストモード
    case MENU_TEST:
      bit_test();
    break;
    // ストレートモード
    case MENU_SMODE:
      serial_out_mode = true;
      lcd.printStr2("","Sx16 ON");
      wait(0.5);
      goto menu_start;
  }
}


