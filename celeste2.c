#include "celeste2.h"

int level_index = 0;
int level_intro = 0;

int freeze_time = 0;
int frames = 0;
int seconds = 0;
int minutes = 0;
int shake = 0;
int sfx_timer = 0;
int berry_count = 0;
int death_count = 0;
CC2_COLLECTED_RECORD collected[CC2_MAX_BERRIES];
int camera_x = 0;
int camera_y = 0;
int show_score = 0;
bool titlescreen_flash_nil = false;
int titlescreen_flash = 0;
int current_music = 0;
int infade = 0;

static void game_start()
{
    memset(snow, 0, sizeof(CC2_SNOW) * CC2_MAX_SNOW_CLOUDS);
    memset(clouds, 0, sizeof(CC2_CLOUD) * CC2_MAX_SNOW_CLOUDS);

    freeze_time = 0;
    frames = 0;
    seconds = 0;
    minutes = 0;
    shake = 0;
    sfx_timer = 0;
    berry_count = 0;
    death_count = 0;
    memset(collected, 0, sizeof(CC2_COLLECTED_RECORD) * CC2_MAX_BERRIES);
    camera_x = 0;
    camera_y = 0;
    show_score = 0;
    titlescreen_flash_nil = true;
    titlescreen_flash = 0;

    int i;
    CC2_SNOW* s = NULL;
    CC2_CLOUD* c = NULL;
    for (i = 0; i < CC2_MAX_SNOW_CLOUDS; i++)
    {
        s = ((CC2_SNOW*)snow + i);
        c = ((CC2_CLOUD*)clouds + i);
        s->x = (float)P8_RND(132);
        s->y = (float)P8_RND(132);
        c->x = (float)P8_RND(132);
        c->y = (float)P8_RND(132);
        c->s = 16 + P8_RND(32);
    }

    // goto titlescreen or level
    if (level_index == 0)
    {
        current_music = 38;
        P8_MUSIC(current_music);
    }
    else goto_level(level_index);
}

void _P8_init() { game_start(); }

void _P8_update()
{

}

void _P8_draw()
{
    
}