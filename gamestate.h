#ifndef _GAMESTATE_H_
#define _GAMESTATE_H_

#include <stdbool.h>

typedef void(*LEVEL_PALETTE_ROUTINE)(void);

typedef struct _CC2_LEVEL
{
    int offset, width, height, camera_mode, music,
        fogmode, clouds, columns,
        camera_barriers_x, camera_barrier_y, bg;
    bool is_camera_barriers_x, right_edge;
    char* title;
    LEVEL_PALETTE_ROUTINE pal;
} CC2_LEVEL;

extern CC2_LEVEL level;
extern CC2_LEVEL levels[8];
extern int level_checkpoint;
extern bool level_checkpoint_active;

extern int c_offset;
extern bool c_flag;
extern int camera_target_x;
extern int camera_target_y;

typedef void(*CAMERA_MODE_ROUTINE)(int, int);
extern CAMERA_MODE_ROUTINE camera_modes[8];

extern void camera_x_barrier(int tile_x, int px, int py);
extern void snap_camera();
extern int tile_y(int py);

extern void goto_level(int index);
extern void next_level();
extern void restart_level();
extern int tile_at(int x, int y);

#endif