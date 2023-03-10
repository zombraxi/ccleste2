#include "gamestate.h"

#include "p8.h"
#include "celeste2.h"
#include "object.h"

CC2_LEVEL level = { 0 };
CC2_LEVEL levels[8];
int level_checkpoint = 0;
bool level_checkpoint_active = false;

int c_offset = 0;
bool c_flag = false;
int camera_target_x = 0;
int camera_target_y = 0;

static void cm1(int px, int py)
{
    if (px < 42)
        camera_target_x = 0;
    else camera_target_x = P8_MIN_INT(40, P8_MIN_INT(level.width * 8 - 128, px - 48));
}
static void cm2(int px, int py)
{
    if (px < 120)
        camera_target_x = 0;
    else if (px > 136) camera_target_x = 128;
    else camera_target_x = px - 64;
    camera_target_y = P8_MAX_INT(P8_MIN_INT(level.height * 8 - 128, py - 64),0);
}
static void cm3(int px, int py)
{
    camera_target_x = P8_MAX_INT(P8_MIN_INT(level.width * 8 - 128, px - 56),0);
    if (level.is_camera_barriers_x == true)
        camera_x_barrier(level.camera_barriers_x, px, py);

    if (py < level.camera_barrier_y * 8 + 3)
        camera_target_y = 8;
    else
        camera_target_y = level.camera_barrier_y * 8;
}
static void cm4(int px, int py)
{
    if (px % 128 > 8 && px % 128 < 120)
        px = (int)(px / 128) * 128 + 64;
    if (px % 128 > 4 && py % 128 < 124)
        py = (int)(py / 128) * 128 + 64;

    camera_target_x = P8_MAX_INT(P8_MIN_INT(level.width * 8 - 128, px - 64),0);
    camera_target_y = P8_MAX_INT(P8_MIN_INT(level.height * 8 - 128, py - 64), 0);
}
static void cm5(int px, int py)
{
    camera_target_x = P8_MAX_INT(P8_MIN_INT(level.width * 8 - 128, px - 32),0);
}
static void cm6(int px, int py)
{
    if (px > 848)
        c_offset = 48;
    else if (px < 704)
    {
        c_flag = false;
        c_offset = 42;
    }
    else if (px > 808)
    {
        c_flag = true;
        c_offset = 96;
    }

    camera_target_x = P8_MAX_INT(P8_MIN_INT(level.width * 8 - 128, px - c_offset),0);

    if (level.is_camera_barriers_x == true)
        camera_x_barrier(level.camera_barriers_x, px, py);

    if (c_flag == true)
        camera_target_x = P8_MAX_INT(camera_target_x, 672);
}
static void cm7(int px, int py)
{
    if (px > 420)
    {
        if (px < 436)
            c_offset = 32 + px - 420;
        else if (px > 808)
            c_offset = 48 - P8_MIN_INT(16, px - 808);
        else c_offset = 48;
    }
    else c_offset = 32;
    camera_target_x = P8_MAX_INT(0,P8_MIN_INT(level.width * 8 - 128, px - c_offset));
}
static void cm8(int px, int py)
{
    camera_target_y = P8_MAX_INT(0, P8_MIN_INT(level.height * 8 - 128, py - 32));
}
CAMERA_MODE_ROUTINE camera_modes[8] = {
    &cm1,&cm2,&cm3,&cm4,&cm5,&cm6,&cm7,&cm8
};

void camera_x_barrier(int tile_x, int px, int py)
{
    int bx = tile_x * 8;
    if (px < bx - 8)
        camera_target_x = P8_MIN_INT(camera_target_x, bx - 128);
    else if (px > bx + 8)
    {
        camera_target_x = P8_MAX_INT(camera_target_x, bx);
    }
}

void snap_camera()
{
    camera_x = camera_target_x;
    camera_y = camera_target_y;
    P8_CAMERA2(camera_x, camera_y);
}

int tile_y(int py)
{
    return P8_MAX_INT(0, P8_MIN_INT((int)(py / 8), level.height - 1));
}

#include "gamedata.h"
void goto_level(int index)
{
    level = levels[index - 1]; // lua arrays index start at 1... c is 0
    level_index = index;
    level_checkpoint = 0;
    level_checkpoint_active = false;

    if (level.title != NULL && standalone != true)
        level_intro = 60;

    if (level_index == 2)
        psfx(17, 8, 16, 0);

    // load into ram the map
    switch (index)
    {
    case 1: P8_LOADMAPDATA(CC2_MAP1_DATA); break;
    case 2: P8_LOADMAPDATA(CC2_MAP2_DATA); break;
    case 3: P8_LOADMAPDATA(CC2_MAP3_DATA); break;
    case 4: P8_LOADMAPDATA(CC2_MAP4_DATA); break;
    case 5: P8_LOADMAPDATA(CC2_MAP5_DATA); break;
    case 6: P8_LOADMAPDATA(CC2_MAP6_DATA); break;
    case 7: P8_LOADMAPDATA(CC2_MAP7_DATA); break;
    case 8: P8_LOADMAPDATA(CC2_MAP8_DATA); break;
    default: break;
    }

    // start music
    if (current_music != level.music && level.music != 0)
    {
        current_music = level.music;
        P8_MUSIC(level.music);
    }

    // load level contents
    restart_level();
}

void next_level()
{
    level_index += 1;
    goto_level(level_index);
}

void restart_level()
{

}

int tile_at(int x, int y)
{
    if (x < 0 || y < 0 || x >= level.width || y >= level.height) return 0;
    return P8_MGET(x, y);
}