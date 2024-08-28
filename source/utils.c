#include "utils.h"
int _rand_seed=42;

OBJ_ATTR obj_buffer[MAX_OBJS]; // GBA supports 128 sprites
int obj_buffer_current=0;
OBJ_AFFINE *obj_aff_buffer= (OBJ_AFFINE*)obj_buffer; // the same address is also used by affine sprites
void clear_objects(){
    for (; obj_buffer_current < MAX_OBJS; obj_buffer_current++)
        obj_buffer[obj_buffer_current].attr0 = ATTR0_HIDE;
    obj_buffer_current = 0;
}

BOOL point_box(FIXED x,FIXED y,FIXED left, FIXED right, FIXED top, FIXED bottom){
    return left<=x && x<=right && top<=y && y<=bottom;
}