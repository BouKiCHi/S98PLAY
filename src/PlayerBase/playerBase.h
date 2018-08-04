#ifndef __PLAYER_BASE_H__
#define __PLAYER_BASE_H__

#include "mbed.h"
#include "lcd.h"

#include "dataOut.h"

#define MODE_FILE 0
#define MODE_GETDIR 1
#define MODE_GETLIST 2

#define EXT_LIST ".lst"

// メニュー選択
#define MENU_FILE 0
#define MENU_DIR 1
#define MENU_LIST 2
#define MENU_INFO 3
#define MENU_COM 4
#define MENU_TEST 5
#define MENU_SMODE 6
#define MENU_MAX 6

// パス
#define MAX_PATH 128
#define MAX_FILENAME 64
#define MAX_DIR 64

class playerBase : public dataOut {
public:
  Timer waitTimer;

  int diffUs;
  int countUs;
  int totalUs;

  bool updateDisplay;

  const char *songExtension;

  char workingDirectory[MAX_DIR];
  char playlist[MAX_FILENAME];
  char songFile[MAX_FILENAME];

  bool stopFlag;
  bool prevFlag;
  bool eofFlag;

  int songIndex;
  int maxEntries;
  int totalSec;

  bool waitSkipFlag;
  int nextButtonCount;

  LCD lcd;

  int ledCount1;
  int ledCount2;

  playerBase(const char *ext);

  void run();
  int put_title();
  int play_file(const char *filename);
  
  virtual const char *get_program_name();
  virtual const char *get_program_version();

  virtual int song_open(const char *filename);
  virtual void song_close();
  virtual int song_play();
  virtual int song_update();


  void reset_timer(void);
  void reset_control_data(void);
  void add_count_us(int us);
  void calc_next(int us);
  int wait_next();
  bool check_control_button(int us);

  void error_sdcard(void);
  void loop_for_com(void);
  bool chk_isdir(const char *cwd, const char *dir);

  int get_file_entry(int index, int mode);

  void disp_time();
  void show_info(int files);
  void bit_test();
  void display_file_selector(int mode);


  int file_select(int mode);
  int get_playlist(int index);
  void make_path(char *path, const char *dir, const char *file);
  void play_mode(void);
  void menu_disp(int sel);
  int menu_select();

  void menu_mode();

};

#endif

