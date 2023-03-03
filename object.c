#include "object.h"

#include <stdlib.h>
#include <memory.h>

CC2_OBJECT objects[CC2_MAX_OBJECTS_COUNT] = { };

int types[128] = { 

};

void ResetCC2Objects()
{
    // nullify the entire objects
    int i;
    CC2_OBJECT* o;
    for (i = 0 ; i < CC2_MAX_OBJECTS_COUNT; i++)
    {
        o = (CC2_OBJECT*)objects + i;
        DeleteCC2Object(o);
    }
}

CC2_OBJECT* CreateCC2Object(int type, int x, int y)
{
    return NULL;
}

void DeleteCC2Object(CC2_OBJECT* obj)
{
    // nullify the entire object
    memset(obj, 0, sizeof(CC2_OBJECT));
    obj->bActive = false;
    obj->bDestroyed = false;
}

void MarkCC2ObjectForDestruction(CC2_OBJECT* obj)
{
    obj->bDestroyed = true;
}