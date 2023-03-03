#ifndef _OBJECT_H_
#define _OBJECT_H_

#include <stdbool.h>

#include "celeste2.h"

typedef void(*SELF_ROUTINE)(void*);
typedef bool(*ON_COLLIDE_ROUTINE)(void*, float, float);
typedef bool(*MOVE_ROUTINE)(void*, float, ON_COLLIDE_ROUTINE);
typedef bool(*OVERLAPS_ROUTINE)(void*, void*, float, float);
typedef bool(*CONTAINS_ROUTINE)(void*, float, float);
typedef bool(*CHECK_SOLID_ROUTINE)(void*, float, float);
typedef bool(*CORNER_CORRECT_FUNC)(void*, int, int);
typedef bool(*CORNER_CORRECT_ROUTINE)(void*, int, int, int, int, CORNER_CORRECT_FUNC);

typedef struct _CC2_OBJECT
{
    // routine that every object should have
    SELF_ROUTINE init;
    SELF_ROUTINE update;
    SELF_ROUTINE draw;

    MOVE_ROUTINE move_x, move_y;
    ON_COLLIDE_ROUTINE on_collide_x, on_collide_y;
    OVERLAPS_ROUTINE overlaps;
    CONTAINS_ROUTINE contains;
    CHECK_SOLID_ROUTINE check_solid;
    CORNER_CORRECT_ROUTINE corner_correct;

    // member vars
    bool bActive;   // whether the object is currently active
    bool bDestroyed; // indicates whether it has just been destroyed

    float x, y;
    float speed_x, speed_y;
    float remainder_x, remainder_y;

    float hit_x, hit_y, hit_w, hit_h;

    int grapple_mode,
        hazard, facing,
        freeze;

} CC2_OBJECT;

#define CC2_MAX_OBJECTS_COUNT 305

extern CC2_OBJECT objects[CC2_MAX_OBJECTS_COUNT];
extern int types[128];

inline int id(int tx, int ty) { return (level_index * 100 + tx + ty * 128); }

extern void ResetCC2Objects();
extern CC2_OBJECT* CreateCC2Object(int type, int x, int y);
extern void DeleteCC2Object(CC2_OBJECT* obj);

extern void MarkCC2ObjectForDestruction(CC2_OBJECT* obj);

#endif