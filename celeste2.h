#ifndef _CELESTE2_H_
#define _CELESTE2_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <memory.h>

#include "p8.h"

typedef struct _CC2_SNOW
{
    float x, y;
} CC2_SNOW;

typedef struct _CC2_CLOUD
{
    float x, y;
    int s;
} CC2_CLOUD;

typedef struct _CC2_COLLECTED_RECORD
{
    bool bCollected;
    int iId;
} CC2_COLLECTED_RECORD;

#define CC2_MAX_SNOW_CLOUDS 25
#define CC2_MAX_BERRIES 1

extern int level_index;
extern int level_intro;

extern CC2_SNOW snow[CC2_MAX_SNOW_CLOUDS];
extern CC2_CLOUD clouds[CC2_MAX_SNOW_CLOUDS];

extern int frames;
extern int seconds;
extern int minutes;

extern int shake;

extern int sfx_timer;

extern int berry_count;
extern int death_count;

// collected...
extern CC2_COLLECTED_RECORD collected[CC2_MAX_BERRIES];

extern int camera_x;
extern int camera_y;

extern int show_score;

extern bool titlescreen_flash_nil;
extern int titlescreen_flash;

extern int current_music;

extern int infade;

#endif