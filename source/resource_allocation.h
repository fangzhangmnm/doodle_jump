#ifndef VRAM_H
#define VRAM_H
#include <tonc.h>

// ========== Resource Allocation ==========

// TODO fix overlapping of charblock and screen block bugs

// background CBB (char block base) goes from 0-3, and SBB (screen block base) goes from 0-31, they share the same memory address
// 32 screen blocks = 4 char blocks = 64KB VRAM
// 1 screen block = 2KB = 32x32 screen tiles (2 Byte per tile)
// 1 char block = 16KB = 512 char tiles (32 Byte per tile) = 32x16 grid
// 1 char tile = 32 Bytes = 8x8 pixels (4 bit per pixel) =  8 words
// 1 word = int32 = 4 Bytes
// 1 char block = 8 screen blocks, 1 screen block = 64 char tiles
// 1 nonrepeating screen = 240x160 pixels or 30x20 tiles = 600 tiles = 18.75KB
// TTE takes 1 screen of char blocks and 1 screen block. so total 11 screen blocks = 22KB VRAM. We can start at CB 704, which is the CB 192 at the CBB 1
// 1 palette = 32 Bytes = 16 colors (2 Bytes per color) = 8 words

// ========== Constants and Macros ==========
#define TONC_CBB 0
#define TONC_SBB 10
#define TONC_BG_PAL_IDX 0

#define GET_CB(tile_idx) ((tile_idx) & 0x1FF)
#define GET_CBB(tile_idx) ((tile_idx) >> 9)

// ========== Global Variables ==========
extern int current_obj_tile_idx;
extern int current_obj_pal_idx;
extern int current_bg_tile_idx;
extern int current_bg_pal_idx;
extern int current_bg_sbb;

void clear_resources(BOOL bitmap_mode);
void set_sb(int sbb_idx, int num, int cb_idx_shift, int pal_idx);
void load_obj_tile_LZ77(const u32* src, int n_tiles, int* tile_idx);
void load_obj_tile(const u32* src, int n_tiles, int* tile_idx);
void load_obj_tile_16x16(const u32* src, int metatile_idx);
void shuffle_obj_tile_32x16(int tile_idx);
void load_obj_pal_LZ77(const u16* src, int* pal_idx);
void load_obj_pal(const u16* src, int* pal_idx);
void load_bg_pal_LZ77(const u16* src, int* pal_idx);
void load_bg_tile_LZ77(const u32* src, int n_tiles, int* tile_idx);
void load_bg_map_LZ77(const u16* src, int* sbb);
#endif // RESOURCE_ALLOCATION_H