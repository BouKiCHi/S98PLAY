#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "playerBase.h"
#include "s98u.h"

class player : public playerBase {
public:
    S98CTX songContext;
    int maxLoopCount;

    player();

    int song_update(void);
    int song_open(const char *filename);
    void song_close();
    const char *get_program_name();
    const char *get_program_version();

    bool command_write(int cmd);
    bool command_control(int cmd);
};

#endif

