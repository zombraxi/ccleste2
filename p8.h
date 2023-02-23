#include <stdint.h>

// Application-defined

extern void _P8_init();
extern void _P8_update();
extern void _P8_draw();

// P8 vm...

enum {
    P8_CALLBACK_NONE,

    P8_CALLBACK_LOADMAPDATA,
    P8_CALLBACK_LOADATLASDATA,
    P8_CALLBACK_LOADFLAGSDATA,

    P8_CALLBACK_CLS,
    P8_CALLBACK_FLIP,
    P8_CALLBACK_COLOR,
};

extern void P8_Callback(int iCallback, int iArgCount, ...);

inline void P8_LOADMAP(const uint8_t* dataPtr) { 
    P8_Callback(P8_CALLBACK_LOADMAPDATA, 1, dataPtr); 
}
inline void P8_LOADATLASDATA(const uint8_t* dataPtr) {
    P8_Callback(P8_CALLBACK_LOADATLASDATA, 1, dataPtr);
}
inline void P8_LOADFLAGSDATA(const uint8_t* dataPtr) {
    P8_Callback(P8_CALLBACK_LOADFLAGSDATA, 1, dataPtr);
}

inline void P8_CLS() {
    P8_Callback(P8_CALLBACK_CLS, 0);
}
inline void P8_CLS_EX(int c) {
    P8_Callback(P8_CALLBACK_CLS, 1, c);
}

inline void P8_FLIP() {
    P8_Callback(P8_CALLBACK_FLIP, 0);
}

inline void P8_COLOR() {
    P8_Callback(P8_CALLBACK_COLOR, 0);
}
inline void P8_COLOR_EX(int c) {
    P8_Callback(P8_CALLBACK_COLOR, 1, c);
}