#include "p8.h"

#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <memory.h>
#include <stdbool.h>

#define SDL_MAIN_HANDLED
#include <SDL.h>

typedef struct _P8_Colour
{
    uint8_t r, g, b;
} P8_Colour;

static const P8_Colour P8ColourPalette[16] = {
    { 0, 0, 0 }, { 29, 43, 83 },
    { 126, 37, 83 }, { 0, 135, 81 },
    { 171, 82, 54 }, { 95, 87, 79 },
    { 194, 195, 199 }, { 255, 241, 232 },
    { 255, 0, 77 }, { 255, 163, 0 },
    { 255, 236, 39 }, { 0, 228, 54 },
    { 41, 173, 255 }, { 131, 118, 156 },
    { 255, 119, 168 }, { 255, 204, 170 }
};

// draw state
static uint8_t AtlasData[8192];
static uint8_t MapData[8192];
static uint8_t FlagData[128];
static uint16_t FillPattern;
static bool FillPatternTransparency;
static uint16_t Transparency;
static int PaletteIndexTbl[16];
static int ColourPicked;

static void ResetDrawState()
{
    int i;
    for (i = 0; i < 8192; i++)
    {
        AtlasData[i] = 0;
        MapData[i] = 0;
    }
    for (i = 0; i < 128; i++)
        FlagData[i] = 0;
    FillPattern = 0x0;
    FillPatternTransparency = false;
    Transparency = 0b1000000000000000;
    for (i = 0; i < 16; i++)
        PaletteIndexTbl[i] = i;
    ColourPicked = 0;
}

int main(int argc, char** argv)
{

    return 0;
}

void P8_Callback(int iCallback, int iArgCount, ...)
{
    uint8_t* dataPtr = (uint8_t*)0;

    va_list Args;
    va_start(Args, iArgCount);

    switch (iCallback)
    {
    case P8_CALLBACK_NONE:
        break;

    case P8_CALLBACK_LOADATLASDATA:
        if (iArgCount > 0)
        {
            dataPtr = va_arg(Args, uint8_t*);
            if (dataPtr != NULL)
                memcpy(AtlasData, dataPtr, 8192);
        }
        break;

    case P8_CALLBACK_LOADMAPDATA:
        if (iArgCount > 0)
        {
            dataPtr = va_arg(Args, uint8_t*);
            if (dataPtr != NULL)
                memcpy(MapData, dataPtr, 8192);
        }
        break;

    case P8_CALLBACK_LOADFLAGSDATA:
        if (iArgCount > 0)
        {
            dataPtr = va_arg(Args, uint8_t*);
            if (dataPtr != NULL)
                memcpy(FlagData, dataPtr, 128);
        }
        break;

    
    case P8_CALLBACK_CLS:
        if (iArgCount == 0)
        { // black clear screen

        }
        else if (iArgCount == 1)
        { // coloured clear screen
            
        }

        break;
    }

    return;
}