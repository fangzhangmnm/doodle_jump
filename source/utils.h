#ifndef UTILS_H
#define UTILS_H
#include <tonc.h>
//========== RNG API ==========

extern int _rand_seed;
INLINE int rand(){// returns a [0,32767] random number
    _rand_seed=1664525*_rand_seed+1013904223;
    return (_rand_seed>>16)&0x7FFF;
}

// ========== Obj Buffer API ==========
// Here we use shadow object buffer, and copy it to VRAM at VBlank
// The reason we use shadow object buffer is because writing to shadow object buffer is far more faster than writing to VRAM
#define MAX_OBJS 128

extern OBJ_ATTR obj_buffer[MAX_OBJS]; // GBA supports 128 sprites
extern int obj_buffer_current;
extern OBJ_AFFINE *obj_aff_buffer; // the same address is also used by affine sprites

void clear_objects();

// ========== Math and Collision ==========
BOOL point_box(FIXED x,FIXED y,FIXED left, FIXED right, FIXED top, FIXED bottom);

#endif