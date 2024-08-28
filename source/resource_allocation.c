#include "resource_allocation.h"

int current_obj_tile_idx;
int current_obj_pal_idx;
int current_bg_tile_idx;
int current_bg_pal_idx;
int current_bg_sbb;
void clear_resources(BOOL bitmap_mode){
    current_obj_tile_idx=bitmap_mode?512:0;
    current_obj_pal_idx=0;
    current_bg_pal_idx=TONC_BG_PAL_IDX+1;
    current_bg_tile_idx=(TONC_SBB+1)*64; //Tonc takes 1 cbb + 192 on the second cbb
    current_bg_sbb=31; // goes from 31 to 0. hopefully not overlapping with other resources
}
void set_sb(int sbb_idx,int num,int cb_idx_shift,int pal_idx){
    // apply the palette to the screen block and shift the charblock index
    for(int i=0;i<32*32*num;i++)
        se_mem[sbb_idx][i]=(se_mem[sbb_idx][i]+cb_idx_shift)|pal_idx<<12;
}
void load_obj_tile_LZ77(const u32* src, int n_titles, int* tile_idx){
    LZ77UnCompVram(src, &tile_mem[4][current_obj_tile_idx]);
    *tile_idx=current_obj_tile_idx;
    current_obj_tile_idx+=n_titles;
}
void load_obj_tile(const u32* src,int n_titles,int* tile_idx){
    memcpy32(&tile_mem[4][current_obj_tile_idx],src,n_titles<<3);
    *tile_idx=current_obj_tile_idx;
    current_obj_tile_idx+=n_titles;
}
void load_obj_tile_16x16(const u32* src,int metatile_idx){
    memcpy32(&tile_mem[4][current_obj_tile_idx],&src[metatile_idx<<5],32);
    current_obj_tile_idx+=4;
}
void shuffle_obj_tile_32x16(int tile_idx){
    u32* dst=(u32*)&tile_mem[4][tile_idx];
    u32 tmp[16];
    memcpy32(tmp,&dst[16],16);
    memcpy32(&dst[16],&dst[32],16);
    memcpy32(&dst[32],tmp,16);
}
void load_obj_pal_LZ77(const u16* src, int* pal_idx){
    LZ77UnCompVram(src, pal_obj_bank[current_obj_pal_idx]);
    *pal_idx=current_obj_pal_idx;
    current_obj_pal_idx++;
}
void load_obj_pal(const u16* src,int* pal_idx){
    memcpy32(pal_obj_bank[current_obj_pal_idx],src,8);
    *pal_idx=current_obj_pal_idx;
    current_obj_pal_idx++;
}
void load_bg_pal_LZ77(const u16* src, int* pal_idx){
    LZ77UnCompVram(src, pal_bg_bank[current_bg_pal_idx]);
    *pal_idx=current_bg_pal_idx;
    current_bg_pal_idx++;
}
void load_bg_tile_LZ77(const u32* src,int n_tiles,int* tile_idx){
    *tile_idx=current_bg_tile_idx;
    LZ77UnCompVram(src, &tile_mem[0][current_bg_tile_idx]);
    current_bg_tile_idx+=n_tiles;
}
void load_bg_map_LZ77(const u16* src,int* sbb){
    LZ77UnCompVram(src, &se_mem[current_bg_sbb]);
    *sbb=current_bg_sbb;
    current_bg_sbb-=1;
}