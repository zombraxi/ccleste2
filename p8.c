#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <memory.h>
#include <stdbool.h>

#include "p8.h"

// some pre-defined P8 stuff

static uint8_t byte_single_on[ 8 ] = {
		0x80,0x40,0x20,0x10,0x8,0x4,0x2,0x1
	};
static uint8_t byte_single_off[ 8 ] = {
	0x7f,0xbf,0xdf,0xef,0xf7,0xfb,0xfd,0xfe
};
static uint16_t short_single_on[ 16 ] = {
	0x8000,0x4000,0x2000,0x1000,0x800,0x400,0x200,0x100,0x80,0x40,0x20,0x10,0x8,0x4,0x2,0x1
};
static uint16_t short_single_off[ 16 ] = {
		0x7fff,0xbfff,0xdfff,0xefff,0xf7ff,0xfbff,0xfdff,0xfeff,0xff7f,0xffbf,0xffdf,0xffef,0xfff7,0xfffb,0xfffd,0xfffe
};

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
#include <SDL_mixer.h>

// SDL manage
static SDL_Renderer* Renderer = NULL;
static SDL_Window* Window = NULL;
static bool InFullscreen = false;

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

    int MixFlags = MIX_INIT_MP3 | MIX_INIT_OGG;
    if (Mix_Init(MixFlags) != MixFlags)
    { // failed because no flags were returned meaning
        // none initialized
        didInit = false;
        goto EndProc;
    }
    if (Mix_OpenAudio(22050, AUDIO_S16SYS, 1, 1024) != 0)
    {    // failed
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

    // DESTROY ALL MIXER SHIT

    Mix_CloseAudio();
    Mix_Quit();

    // DESTROY ALL SDL BASE SHIT
    if (Renderer != NULL)
        SDL_DestroyRenderer(Renderer);
    if (Window != NULL)
        SDL_DestroyWindow(Window);
    SDL_Quit();
}

static void InternalFullscreenSDL()
{
    int Status = -1;
    if (Window != NULL)
    {
        if (InFullscreen == true)
        {
            do { // unset the fullscreen and go back to windowed
                Status = SDL_SetWindowFullscreen(Window, 0);
            } while (Status != 0);
            InFullscreen = false;
        }
        else {
            do {
                Status = SDL_SetWindowFullscreen(Window, SDL_WINDOW_FULLSCREEN);
            } while (Status != 0);
            InFullscreen = true;
        }
    }
}

// draw state

#define P8_MAP_WIDTH 128
#define P8_MAP_HEIGHT 64
#define P8_ATLAS_WIDTH 128
#define P8_ATLAS_HEIGHT 64
#define P8_MAP_LENGTH 8192
#define P8_ATLAS_LENGTH 8192
#define P8_MAX_SPRITES 128
#define P8_PALETTE_SIZE 16

#define TRANSPARENCY_REG_DEFAULT 0b1000000000000000
#define FILLPATTERN_REG_DEFAULT 0b0000000000000000
#define FILLPATTERN_4x4_LENGTH 16

static uint8_t AtlasData[P8_ATLAS_LENGTH];
static uint8_t MapData[P8_MAP_LENGTH];
static uint8_t FlagData[P8_MAX_SPRITES];
static uint16_t FillPattern = FILLPATTERN_REG_DEFAULT; // before becoming a 4x4 square
static uint8_t FillPattern4x4[FILLPATTERN_4x4_LENGTH]; // represents a 4x4 square
                                                        // if 1... draw that color...
                                                        // of 0... dont draw it
                                                        // atleast, depending on the
                                                        // fillPatternTransparency
static bool FillPatternTransparency = false;
static uint16_t Transparency = TRANSPARENCY_REG_DEFAULT; // PaletteTransparency
static int PaletteIndexTbl[P8_PALETTE_SIZE];
static int ColourPicked = 0;
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

static void ResetVirtPalette()
{
    int i = 0;
    for (; i < P8_PALETTE_SIZE; i++)
    {
        PaletteIndexTbl[i] = i;
    }
}

static void SwapVirtPalette(int c0, int c1)
{
    PaletteIndexTbl[c0] = PaletteIndexTbl[c1];
}

static void ResetFillPattern()
{
    FillPattern = 0;
    FillPatternTransparency = false;

    int i;
    for (i = 0; i < FILLPATTERN_4x4_LENGTH; i++)
    {
        FillPattern4x4[i] = 0;
    }
}

static void SetFillPattern(uint16_t bits, int trans)
{
    FillPattern = bits;
    FillPatternTransparency = (trans != 0) ? true : false;

    int i;
    for (i = 0; i < FILLPATTERN_4x4_LENGTH; i++)
    {
        FillPattern4x4[i] = (bits & short_single_on[i]) ? 1 : 0;
    }
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
    ResetFillPattern();
    Transparency = TRANSPARENCY_REG_DEFAULT;
    for (i = 0; i < 16; i++)
        PaletteIndexTbl[i] = i;
    ColourPicked = 0;
    Camera.x = 0;
    Camera.y = 0;
    ResetClipRectSDL();    
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

static int IsFlagBitSet(int flag, int bit)
{
    int ret = 0;
    if (FlagData[flag] & byte_single_on[bit])
        ret = 1;
    return ret;
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
static void SetInputState(int bit)
{
    InputState |= byte_single_on[bit];
}
static void UnsetInputState(int bit)
{
    InputState &= byte_single_off[bit];
}
static int IsSetInputState(int bit)
{
    int ret = 0;
    if (InputState & byte_single_on[bit])
        ret = 1;
    return ret;
}
static void ResetInputState() { InputState = 0; }

static void InternalProcessInputSDL()
{
    SDL_Event ev;
    int set_me;
    int unset_me;
    while (SDL_PollEvent(&ev) == 1)
    {
        switch (ev.type)
        {
        case SDL_KEYDOWN:
			if (ev.key.state == SDL_PRESSED && ev.key.repeat == 0) // pressed & not repeat
			{
				set_me = P8_KEY_INVALID;
				unset_me = P8_KEY_INVALID;
				
				switch (ev.key.keysym.sym)
				{
				case SDLK_LEFT:
					set_me = P8_KEY_LEFT;
					unset_me = P8_KEY_RIGHT;
					break;
				case SDLK_RIGHT:
					set_me = P8_KEY_RIGHT;
					unset_me = P8_KEY_LEFT;
					break;
				case SDLK_UP:
					set_me = P8_KEY_UP;
					unset_me = P8_KEY_DOWN;
					break;
				case SDLK_DOWN:
					set_me = P8_KEY_DOWN;
					unset_me = P8_KEY_UP;
					break;

				case SDLK_c:
				case SDLK_z:
					set_me = P8_KEY_CZ;
					break;

				case SDLK_x:
					set_me = P8_KEY_X;
					break;

                case SDLK_r:
                    SetResetFlag();
                    break;

                case SDLK_e:
                    SetShutdownFlag();
                    break;

                case SDLK_f:
                    InternalFullscreenSDL();
                    break;
				}

				// now lock and modify if not invalid key

				if (set_me != P8_KEY_INVALID)
					SetInputState(set_me);
				if (unset_me != P8_KEY_INVALID)
					UnsetInputState(unset_me);

			}
			break;

		case SDL_KEYUP:
			if (ev.key.state == SDL_RELEASED && ev.key.repeat == 0) // released & not repeat
			{
				unset_me = P8_KEY_INVALID;
				
				switch (ev.key.keysym.sym)
				{
				case SDLK_LEFT:
					unset_me = P8_KEY_LEFT;
					break;
				case SDLK_RIGHT:
					unset_me = P8_KEY_RIGHT;
					break;
				case SDLK_UP:
					unset_me = P8_KEY_UP;
					break;
				case SDLK_DOWN:
					unset_me = P8_KEY_DOWN;
					break;

				case SDLK_c:
				case SDLK_z:
					unset_me = P8_KEY_CZ;
					break;

				case SDLK_x:
					unset_me = P8_KEY_X;
					break;
				}

				// now lock and modify if not invalid key

				if (unset_me != P8_KEY_INVALID)
					UnsetInputState(unset_me);

			}
			break;

        
        case SDL_QUIT:
            SetShutdownFlag();
            break;

        default:
            break;
        }
    }
}

static void Tick()
{

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
        Tick();
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
            SDL_Log("Running P8...");
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
            int useTransparency;
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
        if (iArgCount == 0)
        {
            ResetFillPattern();
        }
        else if (iArgCount == 1)
        {
            Context.Fillp.pattern = (uint16_t)va_arg(Args, int);
            Context.Fillp.useTransparency = 0;
            
            if (Context.Fillp.pattern != 0)
            {
                SetFillPattern(Context.Fillp.pattern, Context.Fillp.useTransparency);
            }
            else ResetFillPattern();
        }
        else if (iArgCount == 2)
        {
            Context.Fillp.pattern = (uint16_t)va_arg(Args, int);
            Context.Fillp.useTransparency = va_arg(Args, int);

            if (Context.Fillp.pattern != 0)
            {
                SetFillPattern(Context.Fillp.pattern, Context.Fillp.useTransparency);
            }
            else ResetFillPattern();
        }
        break;

    case P8_CALLBACK_PAL: 
        if (iArgCount == 0)
        {
            ResetVirtPalette();
        }
        else if (iArgCount == 2)
        {
            Context.Pal.c0 = va_arg(Args, int);
            Context.Pal.c1 = va_arg(Args, int);

            SwapVirtPalette(Context.Pal.c0, Context.Pal.c1);
        }
        else if (iArgCount == 3)
        {
            Context.Pal.c0 = va_arg(Args, int);
            Context.Pal.c1 = va_arg(Args, int);
            Context.Pal.p = va_arg(Args, int);
        }
        break;

    case P8_CALLBACK_PALT: 
        break;

    default:
        break;
    }

    va_end(Args);

    return;
}

int P8_CallResult(int iCallResult, int iArgCount, ...)
{
    int Return = 0;

    va_list Args;
    va_start(Args, iArgCount);

    union {
        struct {
            int button;
        } Btn;
        struct {
            int n, f;
        } Fget;
        struct {
            int x, y;
        } Mget;
    } Context;

    switch (iCallResult)
    {
    case P8_CALLRESULT_BTN:
        if (iArgCount == 1)
        {
            Context.Btn.button = va_arg(Args, int);
            if (Context.Btn.button >= 0 && Context.Btn.button <= 5)
            {
                if (IsSetInputState(Context.Btn.button) == 1)
                    Return = 1;
                else Return = 0;
            }
            else Return = 0;
        }
        else Return = 0;

        break;

    case P8_CALLRESULT_FGET:
        if (iArgCount == 2)
        {
            Context.Fget.n = va_arg(Args, int);
            Context.Fget.f = va_arg(Args, int);
            // check in bounds
            if (Context.Fget.n >= 0 && Context.Fget.n < 128 &&
                    Context.Fget.f >= 0 && Context.Fget.f <= 7)
            {
                if (IsFlagBitSet(Context.Fget.n, Context.Fget.f) == 1)
                {
                    Return = 1;
                }
                else Return = 0;
            }
            else Return = 0;
        }
        else Return = 0;

        break;

    case P8_CALLRESULT_MGET:
        if (iArgCount == 2)
        {
            // context arguments
            Context.Mget.x = va_arg(Args, int);
            Context.Mget.y = va_arg(Args, int);
            // in bounds
            if (Context.Mget.x >= 0 && Context.Mget.x < P8_MAP_WIDTH &&
                    Context.Mget.y >= 0 && Context.Mget.y < P8_MAP_HEIGHT)
            {
                // sprite that is there
                Return = (int)MapData[(Context.Mget.y * P8_MAP_WIDTH) + Context.Mget.x];
            }
            else Return = 0;
        }
        else Return = 0;

        break;

    default:
        break;
    }

    va_end(Args);

    return Return;
}

static uint32_t p8_m_high = 0xDEADBEEFU; // lol, dead beef are u kidding ?
static uint32_t p8_m_low = 0x1234567U;

// random number in range of "x"
static uint32_t p8_rnd(uint32_t x)
{
	uint32_t ret = 0;
	if (x != 0)
	{
		p8_m_high = (p8_m_high << 0x10 | p8_m_high >> 0x10) + p8_m_low;
		p8_m_low = p8_m_low + p8_m_high;
		ret = (uint32_t)p8_m_high % (uint32_t)x;
	}
	return ret;
}

// seed our random number generator
static void p8_srand(uint32_t x) 
{
	if (x == 0) {
		p8_m_high = (uint32_t)0x60009755U;
		x = 0xdeadbeefU; // DEAD BEEF BABY!!!
	}
	else p8_m_high = x ^ 0xbead29baU; // not quite dead beef!
	int32_t unk = 0x20L;
	do {
		p8_m_high = (p8_m_high << 0x10 | p8_m_high >> 0x10) + x;
		x += p8_m_high;
		unk -= 1;
	} while (unk != 0);
	p8_m_low = x;
}

float P8_RND(float x)
{
    uint32_t n = p8_rnd(x * (1 << 16));
    return (float)n / (1 << 16);
}

#include <math.h>
