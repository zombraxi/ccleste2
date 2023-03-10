#include "celeste2.h"
#include "object.h"
#include "input.h"

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

CC2_SNOW snow[CC2_MAX_SNOW_CLOUDS];
CC2_CLOUD clouds[CC2_MAX_SNOW_CLOUDS];

#include "gamedata.h"

static void game_start()
{
    P8_LOADATLASDATA(CC2_ATLAS_DATA);
    P8_LOADFLAGSDATA(CC2_FLAGS_DATA);
    P8_LOADMAPDATA(CC2_MAP0_DATA);

    level_index = 0;
    level_intro = 0;

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

    // other globals to reset that arent necessarily apart of the original
    // "gamestart" routine 

    infade = 0;
    ResetCC2Objects();

    int i;
    CC2_SNOW* s = NULL;
    CC2_CLOUD* c = NULL;
    for (i = 0; i < CC2_MAX_SNOW_CLOUDS; i++)
    {
        s = ((CC2_SNOW*)snow + i);
        c = ((CC2_CLOUD*)clouds + i);
        s->x = P8_RND(132);
        s->y = P8_RND(132);
        c->x = P8_RND(132);
        c->y = P8_RND(132);
        c->s = 16 + (int)P8_RND(32);
    }

    // goto titlescreen or level
    if (level_index == 0)
    {
        current_music = 38;
        P8_MUSIC(current_music);
    }
    else goto_level(level_index); // standalone
}

void _P8_init() { game_start(); }

void draw_time(int x, int y);
void draw_clouds(int scale, float ox, float oy, float sx, float sy, int color, int count);
void draw_snow();
void print_center(const char* text, int x, int y, int c);

// approach is absent here and placed as an inline
// in celeste2.h

void draw_sine(float x0, float x1, float y, int col, float ampltiude, 
                float time_freq, float x_freq, float fade_x_dist);

void _P8_update()
{
    CC2_OBJECT* o = NULL;
    int i = 0;

    // titlescreen
    if (level_index == 0)
    {
        if (titlescreen_flash_nil != true)
        {
            titlescreen_flash -= 1;
            if (titlescreen_flash < -30)
                goto_level(1); // this is the redirection that causes
                                // the level_index to no longer be 0,
                                // and makes this titlescreen update routine
                                // never be ran again
        }
        else if (P8_BTN(4) == CR_TRUE || P8_BTN(5) == CR_TRUE)
        {
            titlescreen_flash = 50;
            titlescreen_flash_nil = false; // causes the flash to 
                                            // begin
                                            // because nil is no good!
            P8_SFX2(22, 3);
        }
    }

    // level intro card
    else if (level_intro > 0) // whenever we first get into a level...
                                // a special sound effect is played
    {
        level_intro -= 1;
        if (level_intro == 0) psfx(17, 24, 9, 0);
    }

    // normal level
    else
    {
        // update timers
        sfx_timer = P8_MAX_INT(sfx_timer - 1, 0);
        shake = P8_MAX_INT(shake - 1, 0);
        infade = P8_MIN_INT(infade + 1, 60);
        if (level_index != 8) frames += 1;
        if (frames == 30) {
            seconds += 1;
            frames = 0;
        }
        if (seconds == 60) {
            minutes += 1;
            seconds = 0;
        }

        update_input(); // process P8 input

        // freeze
        if (freeze_time > 0)
            freeze_time -= 1;
        else { // update game objects
            for ( i = 0; i < CC2_MAX_OBJECTS_COUNT; i++)
            {
                o = ((CC2_OBJECT*)objects + i);
                if (o->freeze > 0)
                    o->freeze -= 1;
                else if (o->bActive == true) {
                    o->update(o);
                }

                if (o->bDestroyed == true)
                    DeleteCC2Object(o);
            }
        }  
    }
}

void _P8_draw()
{
    
}

void draw_time(int x, int y)
{
    int m = minutes % 60;
    int h = (minutes / 60);

    // P8_RECTFILL2(x, y, x + 32, y + 6, 0);
    // print...
}

void draw_clouds(int scale, float ox, float oy, float sx, float sy, int color, int count)
{
    int i;
    int s;
    float x;
    float y;

    CC2_CLOUD c;
    for (i = 0; i < count; i++)
    {
        c = clouds[i];
        s = c.s * scale;
        x = ox + (camera_x + (int)(c.x - camera_x * 0.9f) % (128 + s) - s / 2) * sx;
        y = oy + (camera_y + (int)(c.y - camera_y * 0.9f) % (128 + s / 2)) * sy;
        P8_CLIP2( (int)(x - s / 2 - camera_x), y - s / 2 - camera_y, s, s / 2);
        P8_CIRCFILL2(x, y, s / 3, color);
        if (i % 2 == 0)
            P8_CIRCFILL2(x - s / 3, y, s / 5, color);
        if (i % 2 == 0)
            P8_CIRCFILL2(x + s / 3, y, s / 6, color);
        
        c.x += (4 - i % 4) * 0.25f;
    }
    P8_CLIP2(0,0,128,128);
}

void draw_snow()
{
    int i;
    CC2_SNOW s;
    for (i = 0; i < CC2_MAX_SNOW_CLOUDS; i++)
    {
        s = snow[i];
        P8_CIRCFILL2(camera_x + (int)(s.x - camera_x * 0.5f) % 132 - 2,
            camera_y + (int)(s.y - camera_y * 0.5f) % 132,
            i % 2,
            7);
        s.x += (4 - i % 4);
        s.y += P8_SIN(P8_TIME() * 0.25f + i * 0.1f);
    }
}

void print_center(const char* text, int x, int y, int c)
{

}

void psfx(int id, int off, int len, int lock)
{
    if (sfx_timer <= 0 || lock)
    {
        P8_SFX4(id, 3, off , len);
        if (lock) sfx_timer = lock;
    }
}