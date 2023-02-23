#include "p8.h"

#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <memory.h>
#include <stdbool.h>

#define SDL_MAIN_HANDLED
#include <SDL.h>

// SDL manage
static SDL_Renderer* Renderer = NULL;
static SDL_Window* Window = NULL;

static bool InternalInitSDL()
{
    bool didInit = true;
    SDL_DisplayMode displayMode;
    const int windowRealWH = 128;
    int winHeightWidth = 128;

    // init SDL
    SDL_SetMainReady(); // ensure SDL_Init wont fail
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) != 0)
    { // if it fails
        didInit = false;
        goto EndProc;
    }

    // get our desktop display
    if (SDL_GetDesktopDisplayMode(0, &displayMode) != 0)
    { // fails to get display mode
        didInit = false;
        goto EndProc;
    }

    // get the window creation size 1:1 ratio
    winHeightWidth = (displayMode.h > 128) ? 
        ( ((int)(displayMode.h / windowRealWH) - 1) * windowRealWH) : windowRealWH;

    // create our window and renderer
    if (SDL_CreateWindowAndRenderer(winHeightWidth, winHeightWidth,
            SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN,
            &Window, &Renderer) != 0)
    { // failed to create the window and the renderer
        didInit = false;
        goto EndProc;
    }

    // set the renderer's logical size to make scaling ez pz
    if (SDL_RenderSetLogicalSize(Renderer, windowRealWH,windowRealWH) != 0)
    {    // failed to set the logical size
        didInit = false;
        goto EndProc;
    }  

    // hide, set the title, then show and raise the attention to the user...
    SDL_HideWindow(Window);
    SDL_SetWindowTitle(Window, "Celeste Classic 2");
    SDL_ShowWindow(Window);
    SDL_RaiseWindow(Window);

EndProc:
    if (didInit == false)
    {    // if we failed to initialize... log the reason
        SDL_Log("%s", SDL_GetError());
    }
    return didInit;
}

static void InternalShutdownSDL()
{
    if (Renderer != NULL)
        SDL_DestroyRenderer(Renderer);
    if (Window != NULL)
        SDL_DestroyWindow(Window);
    SDL_Quit();
}

// some pre-defined P8 shit

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
    if (InternalInitSDL() == true)
    {
        // if we initialized successfully, allow ourselves to continue

        // only shutdown if initialization succeeded
        InternalShutdownSDL();
    }
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