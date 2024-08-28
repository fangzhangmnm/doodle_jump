#ifndef RESOURCES_H
#define RESOURCES_H
#include "resource_allocation.h"


// ========== Loading Resources ==========

extern int gfx_doodle_tile_idx,gfx_doodle_pal_idx;
extern int gfx_platform_tile_idx, gfx_platform_pal_idx;
extern int gfx_bg_tile_idx,gfx_bg_pal_idx;
extern int gfx_bg_drop_tile_idx,gfx_bg_drop_pal_idx,gfx_bg_drop_sbb_idx;
extern int gfx_monsters_tile_idx,gfx_monsters_pal_idx;

void load_resources();


#endif