#ifndef _P8_H_
#define _P8_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

// Application-defined

extern void _P8_init();
extern void _P8_update();
extern void _P8_draw();

// P8 vm...
// Math-based stuff is not considered a part of the "VM",
// thus, excluded from CallResults... (like sin, cos, etc)

#define P8_KEY_INVALID 6
#define P8_KEY_LEFT 0
#define P8_KEY_RIGHT 1
#define P8_KEY_UP 2
#define P8_KEY_DOWN 3
#define P8_KEY_CZ 4
#define P8_KEY_X 5

// Callbacks...
enum {
    P8_CALLBACK_NONE,

    P8_CALLBACK_LOADMAPDATA,
    P8_CALLBACK_LOADATLASDATA,
    P8_CALLBACK_LOADFLAGSDATA,
    P8_CALLBACK_LOADSFXDATA,
    P8_CALLBACK_LOADMUSICDATA,

    P8_CALLBACK_CLS,
    P8_CALLBACK_FLIP,
    P8_CALLBACK_COLOR,

    P8_CALLBACK_MAP,
    P8_CALLBACK_SPR,
    P8_CALLBACK_SSPR,

    P8_CALLBACK_RECT,
    P8_CALLBACK_RECTFILL,
    P8_CALLBACK_CIRC,
    P8_CALLBACK_CIRCFILL,
    P8_CALLBACK_OVAL,
    P8_CALLBACK_LINE,

    P8_CALLBACK_FILLP,
    P8_CALLBACK_PAL,
    P8_CALLBACK_PALT,
    P8_CALLBACK_CAMERA,
    P8_CALLBACK_CLIP,

    P8_CALLBACK_SFX,
    P8_CALLBACK_MUSIC,

};

// Callback in this context meaning that it does not return a value,
// simply runs a VM operation
extern void P8_Callback(int iCallback, int iArgCount, ...);

inline void P8_LOADMAPDATA(const uint8_t* dataPtr) { P8_Callback(P8_CALLBACK_LOADMAPDATA, 1, dataPtr);  }
inline void P8_LOADATLASDATA(const uint8_t* dataPtr) { P8_Callback(P8_CALLBACK_LOADATLASDATA, 1, dataPtr); }
inline void P8_LOADFLAGSDATA(const uint8_t* dataPtr) { P8_Callback(P8_CALLBACK_LOADFLAGSDATA, 1, dataPtr); }
inline void P8_LOADSFXDATA(const char* fileNames[], int count) { P8_Callback(P8_CALLBACK_LOADSFXDATA, 2, fileNames, count); }
inline void P8_LOADMUSICDATA(const char* fileNames[], int count) { P8_Callback(P8_CALLBACK_LOADMUSICDATA, 2, fileNames, count); }

inline void P8_CLS() { P8_Callback(P8_CALLBACK_CLS, 0); }
inline void P8_CLS2(int c) { P8_Callback(P8_CALLBACK_CLS, 1, c); }

inline void P8_FLIP() { P8_Callback(P8_CALLBACK_FLIP, 0); }

inline void P8_COLOR() { P8_Callback(P8_CALLBACK_COLOR, 0); }
inline void P8_COLOR2(int c) { P8_Callback(P8_CALLBACK_COLOR, 1, c); }

inline void P8_MAP(int tile_x, int tile_y) { P8_Callback(P8_CALLBACK_MAP, 2, tile_x, tile_y); }
inline void P8_MAP2(int tile_x, int tile_y, int sx, int sy) { P8_Callback(P8_CALLBACK_MAP, 4, tile_x, tile_y, sx, sy); }
inline void P8_MAP3(int tile_x, int tile_y, int sx, int sy, int tile_w, int tile_h) { P8_Callback(P8_CALLBACK_MAP, 6, tile_x, tile_y, sx, sy, tile_w, tile_h); }
inline void P8_MAP4(int tile_x, int tile_y, int sx, int sy, int tile_w, int tile_h, int layers) { P8_Callback(P8_CALLBACK_MAP, 7, tile_x, tile_y, sx, sy, tile_w, tile_h, layers); }

inline void P8_SPR(int n, int x, int y) { P8_Callback(P8_CALLBACK_SPR, 3, n, x, y); }
inline void P8_SPR2(int n, int x, int y, int w, int h) { P8_Callback(P8_CALLBACK_SPR, 5, n, x, y, w, h); }
inline void P8_SPR3(int n, int x, int y, int w, int h, bool flip_x, bool flip_y) { P8_Callback(P8_CALLBACK_SPR, 7, n, x, y, w, h, flip_x, flip_y); }

inline void P8_SSPR(int sx, int sy, int sw, int sh, int dx, int dy) { P8_Callback(P8_CALLBACK_SSPR, 6, sx, sy, sw, sh, dx, dy); }
inline void P8_SSPR2(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh) { P8_Callback(P8_CALLBACK_SSPR, 8, sx, sy, sw, sh, dx, dy, dw, dh); }
inline void P8_SSPR3(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, bool flip_x) { P8_Callback(P8_CALLBACK_SSPR, 9, sx, sy, sw, sh, dx, dy, dw, dh, flip_x); }
inline void P8_SSPR4(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, bool flip_x, bool flip_y) { P8_Callback(P8_CALLBACK_SSPR, 10, sx, sy, sw, sh, dx, dy, dw, dh, flip_x, flip_y); }

inline void P8_RECT(int x0, int y0, int x1, int y1) { P8_Callback(P8_CALLBACK_RECT, 4, x0, y0, x1, y1); }
inline void P8_RECT2(int x0, int y0, int x1, int y1, int col) { P8_Callback(P8_CALLBACK_RECT, 5, x0, y0, x1, y1, col); }

inline void P8_RECTFILL(int x0, int y0, int x1, int y1) { P8_Callback(P8_CALLBACK_RECTFILL, 4, x0, y0, x1, y1); }
inline void P8_RECTFILL2(int x0, int y0, int x1, int y1, int col) { P8_Callback(P8_CALLBACK_RECTFILL, 5, x0, y0, x1, y1, col); }

inline void P8_CIRC(int x, int y, int r) { P8_Callback(P8_CALLBACK_CIRC, 3, x, y, r); }
inline void P8_CIRC2(int x, int y, int r, int col) { P8_Callback(P8_CALLBACK_CIRC, 4, x, y, r, col); }

inline void P8_CIRCFILL(int x, int y, int r) { P8_Callback(P8_CALLBACK_CIRCFILL, 3, x, y, r); }
inline void P8_CIRCFILL2(int x, int y, int r, int col) { P8_Callback(P8_CALLBACK_CIRCFILL, 4, x, y, r, col); }

// audio
inline void P8_SFX(int n) { P8_Callback(P8_CALLBACK_SFX, 1, n); }
inline void P8_SFX2(int n, int channel) { P8_Callback(P8_CALLBACK_SFX, 2, n, channel); }
inline void P8_SFX3(int n, int channel, int offset) { P8_Callback(P8_CALLBACK_SFX, 3, n, channel, offset); }
inline void P8_SFX4(int n, int channel, int offset, int length) { P8_Callback(P8_CALLBACK_SFX, 4, n, channel, offset, length); }

inline void P8_MUSIC(int n) { P8_Callback(P8_CALLBACK_MUSIC, 1, n); }
inline void P8_MUSIC2(int n, int fade_len) { P8_Callback(P8_CALLBACK_MUSIC, 2, n, fade_len); }
inline void P8_MUSIC3(int n, int fade_len, int channel_mask) { P8_Callback(P8_CALLBACK_MUSIC, 3, n, fade_len, channel_mask); }

inline void P8_FILLP() { P8_Callback(P8_CALLBACK_FILLP, 0); }
inline void P8_FILLP2(uint16_t pat) { P8_Callback(P8_CALLBACK_FILLP, 1, (int)pat);  }
inline void P8_FILLP3(uint16_t pat, bool trans) { P8_Callback(P8_CALLBACK_FILLP, 2, (int)pat, (trans == true) ? 1 : 0); }

inline void P8_PAL() { P8_Callback(P8_CALLBACK_PAL, 0); }
inline void P8_PAL2(int c0, int c1) { P8_Callback(P8_CALLBACK_PAL, 2, c0, c1); }
inline void P8_PAL3(int c0, int c1, int p) { P8_Callback(P8_CALLBACK_PAL, 3, c0, c1, p); }

inline void P8_PALT() { P8_Callback(P8_CALLBACK_PALT, 0); }
inline void P8_PALT1(uint16_t pat) { P8_Callback(P8_CALLBACK_PALT, 1, (int)pat); }
inline void P8_PALT2(int c, bool t) { P8_Callback(P8_CALLBACK_PALT, 2, c, (t == true) ? 1 : 0); }

inline void P8_CAMERA() { P8_Callback(P8_CALLBACK_CAMERA, 0); }
inline void P8_CAMERA2(int x, int y) { P8_Callback(P8_CALLBACK_CAMERA, 2, x, y); }

inline void P8_CLIP() { P8_Callback(P8_CALLBACK_CLIP, 0); }
inline void P8_CLIP2(int x, int y, int w, int h) { P8_Callback(P8_CALLBACK_CLIP, 4, x, y, w, h); }
inline void P8_CLIP3(int x, int y, int w, int h, bool clip_previous) { P8_Callback(P8_CALLBACK_CLIP, 5, x, y, w, h, (clip_previous == true) ? 1 : 0); }

// Call Results...
enum {
    P8_CALLRESULT_MGET,
    P8_CALLRESULT_FGET,
    P8_CALLRESULT_BTN,
    P8_CALLRESULT_PEEK,
    P8_CALLRESULT_PEEK2,
    P8_CALLRESULT_PEEK4
};

#define CR_TRUE (int)1
#define CR_FALSE (int)0

// Call result is a VM operation which returns a value ( an int in this case )
extern int P8_CallResult(int iCallResult, int iArgCount, ...);

inline int P8_BTN(int button) { return P8_CallResult(P8_CALLRESULT_BTN, 1, button); }
inline int P8_MGET(int x, int y) { return P8_CallResult(P8_CALLRESULT_MGET, 2, x, y); }
inline int P8_FGET(int n, int f) { return P8_CallResult(P8_CALLRESULT_FGET, 2, n, f); }
inline int P8_PEEK(int addr) { return P8_CallResult(P8_CALLRESULT_PEEK, 1, addr) & 0xff; }
inline int P8_PEEK2(int addr) { return P8_CallResult(P8_CALLRESULT_PEEK2, 1, addr) & 0xffff; }
inline int P8_PEEK4(int addr) { return P8_CallResult(P8_CALLRESULT_PEEK4, 1, addr) & 0xffffffff; }

// Math, other operations that dont fit
// the bill for being a Callback or CallResult

extern float P8_RND(float x);
extern float P8_SIN(float x);

// time returns a float... so it cant be a callresult
extern float P8_TIME();

inline int P8_MAX_INT(int a, int b) { return (a > b) ? a : b; }
inline int P8_MIN_INT(int a, int b) { return (a < b) ? a : b; }
inline float P8_MAX_FLOAT(float a, float b) { return (a > b) ? a : b; }
inline float P8_MIN_FLOAT(float a, float b) { return (a < b) ? a : b; }

#endif
