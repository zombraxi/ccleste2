#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <memory.h>
#include <stdbool.h>

#include "p8.h"

// some pre-defined P8 stuff

typedef struct _P8_VecI2
{
    int x, y;
} P8_VecI2;

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

// draw state
static uint8_t AtlasData[8192];
static uint8_t MapData[8192];
static uint8_t FlagData[128];
static uint16_t FillPattern;
static bool FillPatternTransparency;
static uint16_t Transparency;
static int PaletteIndexTbl[16];
static int ColourPicked;
static P8_VecI2 Camera;

static void ResetClipRectSDL()
{
    SDL_Rect clip_rect;
    clip_rect.x = 0;
    clip_rect.y = 0;
    clip_rect.w = 128;
    clip_rect.h = 128;
    SDL_RenderSetClipRect(Renderer, &clip_rect);
}

static void SetClipRectSDL(int x, int y, int w, int h)
{
    SDL_Rect clip_rect;
    clip_rect.x = x;
    clip_rect.y = y;
    clip_rect.w = w;
    clip_rect.h = h;
    SDL_RenderSetClipRect(Renderer, &clip_rect);
}

static void InternalFlipSDL()
{
    P8_Colour colour = P8ColourPalette[PaletteIndexTbl[ColourPicked]];
    if (SDL_RenderFlush(Renderer) == 0) // flush any extra commands
    { // if success
        SDL_RenderPresent(Renderer); // present to the window
        if (SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255) == 0) // set draw color to black
        { // if succeeds...
            SDL_RenderClear(Renderer); // clear screen with black
            SDL_SetRenderDrawColor(Renderer, colour.r, colour.g, colour.b, 255);
            // restore SDL draw colour
        }
    }
}

// Clear screen and reset clipping rectangle
static void InternalClearScreenSDL(int c)
{
    // original color
    P8_Colour colour = P8ColourPalette[PaletteIndexTbl[ColourPicked]];
    // color to clear with
    P8_Colour clear_colour = P8ColourPalette[PaletteIndexTbl[c]];

    // set the draw color to clear color,
    // then clear, and restore to original draw color
    if (SDL_SetRenderDrawColor(Renderer, clear_colour.r, clear_colour.g, clear_colour.b, 255) == 0)
    {
        SDL_RenderClear(Renderer);
        SDL_SetRenderDrawColor(Renderer, colour.r, colour.g, colour.b, 255);
    }

    ResetClipRectSDL();
}

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
    ResetClipRectSDL();    
}

// p8 "system flag"
enum {
    P8_SYSTEMFLAG_NONE,
    P8_SYSTEMFLAG_RESET,
    P8_SYSTEMFLAG_SHUTDOWN
};
static int SystemFlag = P8_SYSTEMFLAG_NONE;
static void SetResetFlag() { SystemFlag = P8_SYSTEMFLAG_RESET; }
static void SetShutdownFlag() { SystemFlag = P8_SYSTEMFLAG_SHUTDOWN; }
static void ClearSystemFlag() { SystemFlag = P8_SYSTEMFLAG_NONE; }
static bool ContinueMainLoopBasedOnFlag() { 
    return (SystemFlag != P8_SYSTEMFLAG_RESET && SystemFlag != P8_SYSTEMFLAG_SHUTDOWN) ?
                true : false;
}
static bool IsShutdownFlagSet() { return (SystemFlag == P8_SYSTEMFLAG_SHUTDOWN) ? 
                                            true : false; }

// p8 input state
static uint8_t InputState = 0;
static void ResetInputState() { InputState = 0; }

static void InternalProcessInputSDL()
{
    SDL_Event ev;
    while (SDL_PollEvent(&ev) == 1)
    {
        switch (ev.type)
        {
        case SDL_KEYDOWN:
            break;

        case SDL_KEYUP:
            break;
        
        case SDL_QUIT:
            SetShutdownFlag();
            break;
        }
    }
}

static bool InternalRunP8()
{
    bool ContinueRunning = true;

    ClearSystemFlag();
    ResetDrawState();
    ResetInputState();

    _P8_init(); // call application-defined init function
    do {
        InternalProcessInputSDL();
        _P8_update();
        _P8_draw();
        InternalFlipSDL();
    } while (ContinueMainLoopBasedOnFlag() == true);

    if (IsShutdownFlagSet() == true) // if we need to shutdown, we need to cut off
        ContinueRunning = false;     // for good instead of just resetting

    return ContinueRunning;
}

int main(int argc, char** argv)
{
    bool ContinueRunningP8 = true;
    if (InternalInitSDL() == true)
    {
        // if we initialized successfully, allow ourselves to continue
        do {
            ContinueRunningP8 = InternalRunP8();
        } while (ContinueRunningP8 == true);

        // only shutdown if initialization succeeded
        InternalShutdownSDL();
        SDL_Log("Graceful exit");
    }
    return 0;
}

void P8_Callback(int iCallback, int iArgCount, ...)
{
    // stores context for each callback
    union
    {
        struct {
            uint8_t* data;
        } LoadAtlas;

        struct {
            uint8_t* data;
        } LoadMap;

        struct {
            uint8_t* data;
        } LoadFlags;

        struct {
            char** fileNames;
            int fileCount;
        } LoadSfx;

        struct {
            char** fileNames;
            int fileCount;
        } LoadMusic;

        struct {
            int colour;
        } Cls;

        struct {
            int colour;
        } Color;

        struct {
            int tile_x, tile_y, sx, sy, tile_w, tile_h, layers;
        } Map;

        struct {
            int n, x, y, w, h;
            bool flip_x, flip_y;
        } Spr;

        struct {
            int sx, sy, sw, sh, dx, dy, dw, dh;
            bool flip_x, flip_y;
        } Sspr;

        struct {
            int x0, y0, x1, y1, col;
        } Rect;

        struct {
            int x, y, r, col;
        } Circ;

        struct {
            uint16_t pattern;
            bool useTransparency;
        } Fillp;

        struct {
            uint16_t bits;
            int c;
            bool t;
        } Palt;

        struct {
            int c0, c1, p;
        } Pal;

    } Context;

    P8_Colour colour;

    va_list Args;
    va_start(Args, iArgCount);

    switch (iCallback)
    {
    case P8_CALLBACK_NONE:
        break;

    case P8_CALLBACK_LOADATLASDATA:
        if (iArgCount > 0)
        {
            Context.LoadAtlas.data = va_arg(Args, uint8_t*);
            if (Context.LoadAtlas.data != NULL)
                memcpy(AtlasData, Context.LoadAtlas.data, 8192);
        }
        break;

    case P8_CALLBACK_LOADMAPDATA:
        if (iArgCount > 0)
        {
            Context.LoadMap.data = va_arg(Args, uint8_t*);
            if (Context.LoadMap.data != NULL)
                memcpy(MapData, Context.LoadMap.data, 8192);
        }
        break;

    case P8_CALLBACK_LOADFLAGSDATA:
        if (iArgCount > 0)
        {
            Context.LoadFlags.data = va_arg(Args, uint8_t*);
            if (Context.LoadFlags.data != NULL)
                memcpy(FlagData, Context.LoadFlags.data, 128);
        }
        break;

    case P8_CALLBACK_LOADMUSICDATA:
        if (iArgCount >= 2)
        {
            Context.LoadMusic.fileNames = va_arg(Args, char**);
            Context.LoadMusic.fileCount = va_arg(Args, int);
        }
        break;

    case P8_CALLBACK_LOADSFXDATA:
        if (iArgCount >= 2)
        {
            Context.LoadSfx.fileNames = va_arg(Args, char**);
            Context.LoadSfx.fileCount = va_arg(Args, int);
        }
        break;
    
    case P8_CALLBACK_CLS:
        if (iArgCount == 0)
        { // black clear screen
            InternalClearScreenSDL(0);
        }
        else if (iArgCount == 1)
        { // coloured clear screen
            Context.Cls.colour = va_arg(Args, int);
            InternalClearScreenSDL(Context.Cls.colour);
        }

        break;

    case P8_CALLBACK_COLOR:
        if (iArgCount == 0) // if no arguments, sets color to 6
        {
            ColourPicked = 6;
            colour = P8ColourPalette[PaletteIndexTbl[ColourPicked]];
            SDL_SetRenderDrawColor(Renderer, colour.r, colour.g, colour.b, 255);
        }
        else if (iArgCount == 1)
        {
            Context.Color.colour = va_arg(Args, int);
            ColourPicked = Context.Color.colour;
            colour = P8ColourPalette[PaletteIndexTbl[ColourPicked]];
            SDL_SetRenderDrawColor(Renderer, colour.r, colour.g, colour.b, 255);
        }
        break;

    case P8_CALLBACK_FLIP:
        InternalFlipSDL();
        break;

    // sprite, map shit
    case P8_CALLBACK_MAP: 
        break;

    case P8_CALLBACK_SPR: 
        break;

    case P8_CALLBACK_SSPR: 
        break;


    // primitives
    case P8_CALLBACK_RECT: 
        break;

    case P8_CALLBACK_RECTFILL: 
        break;

    case P8_CALLBACK_CIRC: 
        break;

    case P8_CALLBACK_CIRCFILL: 
        break;

    case P8_CALLBACK_OVAL: 
        break;

    case P8_CALLBACK_LINE: 
        break;

    // modify some special draw state
    case P8_CALLBACK_FILLP: 
        break;

    case P8_CALLBACK_PAL: 
        break;

    case P8_CALLBACK_PALT: 
        break;

    }

    return;
}