#include "resources.h"

#include "gfx_doodle.h"
#include "gfx_platforms.h"
#include "gfx_bg_tile.h"
#include "gfx_bg_drop.h"
#include "gfx_monsters.h"
int gfx_doodle_tile_idx,gfx_doodle_pal_idx;
int gfx_platform_tile_idx, gfx_platform_pal_idx;
int gfx_bg_tile_idx,gfx_bg_pal_idx;
int gfx_bg_drop_tile_idx,gfx_bg_drop_pal_idx,gfx_bg_drop_sbb_idx;
int gfx_monsters_tile_idx,gfx_monsters_pal_idx;
void load_resources(){
    clear_resources(FALSE);
    load_obj_tile_LZ77(gfx_doodleTiles,12,&gfx_doodle_tile_idx);
    load_obj_pal_LZ77(gfx_doodlePal,&gfx_doodle_pal_idx);
    load_obj_tile_LZ77(gfx_platformsTiles,6,&gfx_platform_tile_idx);
    load_obj_pal_LZ77(gfx_platformsPal,&gfx_platform_pal_idx);
    load_bg_tile_LZ77(gfx_bg_tileTiles,1,&gfx_bg_tile_idx);
    load_bg_pal_LZ77(gfx_bg_tilePal,&gfx_bg_pal_idx);
    load_bg_tile_LZ77(gfx_bg_dropTiles,251,&gfx_bg_drop_tile_idx);
    load_bg_pal_LZ77(gfx_bg_dropPal,&gfx_bg_drop_pal_idx);
    load_bg_map_LZ77(gfx_bg_dropMap,&gfx_bg_drop_sbb_idx);
    set_sb(gfx_bg_drop_sbb_idx,1,GET_CB(gfx_bg_drop_tile_idx),gfx_bg_drop_pal_idx);
    gfx_monsters_tile_idx=current_obj_tile_idx;
    const u8 order[]={0,3,1,2,4,5};
    for(int i=0;i<6;i++)
        load_obj_tile_16x16(gfx_monstersTiles,order[i]);
    shuffle_obj_tile_32x16(gfx_monsters_tile_idx+8);
    load_obj_pal(gfx_monstersPal,&gfx_monsters_pal_idx);
}
