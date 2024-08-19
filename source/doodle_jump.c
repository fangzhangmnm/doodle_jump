#include <tonc.h>
#include <stdlib.h>
#include <string.h>
// ========== Obj Buffer API ==========

OBJ_ATTR obj_buffer[128];
int obj_buffer_current=0;
OBJ_AFFINE *obj_aff_buffer= (OBJ_AFFINE*)obj_buffer;
void clear_obj_buffer(){
    for (; obj_buffer_current < 128; obj_buffer_current++)
        obj_buffer[obj_buffer_current].attr0 = ATTR0_HIDE;
    obj_buffer_current = 0;
}


// ========== Resource Allocation ==========

int current_obj_tile_idx=0;
int current_pal_idx=0;
void reset_resource_idx(){
    current_obj_tile_idx=512;
    current_pal_idx=0;
}


#include "gfx_doodle.h"
int doodle_tile_idx,doodle_pal_idx;
void load_doodle_gfx(){
    memcpy(&tile_mem[4][current_obj_tile_idx],gfx_doodleTiles,gfx_doodleTilesLen);
    doodle_tile_idx=current_obj_tile_idx;
    current_obj_tile_idx+=gfx_doodleTilesLen/32;
    memcpy(&pal_obj_bank[current_pal_idx],gfx_doodlePal,gfx_doodlePalLen);
    doodle_pal_idx=current_pal_idx;
    current_pal_idx++;
}
#include "gfx_platforms.h"
int platform_tile_idx, platform_pal_idx;
void load_platform_gfx(){
    memcpy(&tile_mem[4][current_obj_tile_idx],gfx_platformsTiles,gfx_platformsTilesLen);
    platform_tile_idx=current_obj_tile_idx;
    current_obj_tile_idx+=gfx_platformsTilesLen/32;
    memcpy(&pal_obj_bank[current_pal_idx],gfx_platformsPal,gfx_platformsPalLen);
    platform_pal_idx=current_pal_idx;
    current_pal_idx++;
}

// ========== GameObjects ==========

#define VIEWPORT_WIDTH 72
#define VIEWPORT_HEIGHT 160
#define CLR_BG RGB15(30,29,29)

void draw_background(){
    m3_rect(0,0,VIEWPORT_WIDTH,VIEWPORT_HEIGHT,CLR_BG);
}



struct{
    FIXED x,y,vx,vy,last_y;
    FIXED acc,dec,g,max_vx,velo;
    int turn;
    int max_plat_dist;
} doodle;
const int doodle_extend=5;
void init_doodle(){
    doodle.x=int2fx(VIEWPORT_WIDTH/2);
    doodle.y=int2fx(VIEWPORT_HEIGHT/2);
    doodle.turn=0;
    doodle.last_y=doodle.y;
    doodle.vx=float2fx(0);
    doodle.vy=float2fx(0);
    doodle.acc=float2fx(0.5f/4);
    doodle.dec=float2fx(0.1f/4);
    doodle.g=float2fx(0.2f/4);
    doodle.max_vx=float2fx(2.0f/2);
    doodle.velo=float2fx(5.0f/2);
    doodle.max_plat_dist=60; // max jump height=velo^2/(2*g)=62.5
}
void draw_doodle(){
    OBJ_ATTR *spr= &obj_buffer[obj_buffer_current++];
    obj_set_attr(spr,ATTR0_SQUARE,ATTR1_SIZE_16,ATTR2_BUILD(doodle_tile_idx,doodle_pal_idx,0));
    int x=fx2int(doodle.x)-7;
    int y=fx2int(doodle.y)-15;
    if(!doodle.turn){spr->attr1|=ATTR1_HFLIP;x--;}
    obj_set_pos(spr,x,y);

    // m3_hline(0,fx2int(doodle.y),VIEWPORT_WIDTH,CLR_RED);
    // m3_vline(fx2int(doodle.x),0,VIEWPORT_HEIGHT,CLR_RED);
}
void update_doodle(){
    doodle.vx=fxadd(doodle.vx,key_tri_horz()*doodle.acc);
    if(!key_tri_horz()){
        if(doodle.vx>0)
            doodle.vx=max(fxsub(doodle.vx,doodle.dec),0);
        if(doodle.vx<0)
            doodle.vx=min(fxadd(doodle.vx,doodle.dec),0);
    }
    doodle.vx=clamp(doodle.vx,-doodle.max_vx,doodle.max_vx);
    if(doodle.vx<0)doodle.turn=1;
    if(doodle.vx>0)doodle.turn=0;
    doodle.x=fxadd(doodle.x,doodle.vx);
    if(doodle.x<int2fx(0))doodle.x=int2fx(VIEWPORT_WIDTH-1);
    if(doodle.x>int2fx(VIEWPORT_WIDTH-1))doodle.x=int2fx(0);
    doodle.vy=fxadd(doodle.vy,doodle.g);
    doodle.last_y=doodle.y;
    doodle.y=fxadd(doodle.y,doodle.vy);
    if(doodle.y>int2fx(VIEWPORT_HEIGHT-1)){
        doodle.y=int2fx(VIEWPORT_HEIGHT-1);
        doodle.vy=-doodle.velo;
    }
}

typedef enum{
    PLATFORM_INACTIVE=0,
    PLATFORM_NORMAL=1,
    PLATFORM_MOVING=2,
    PLATFORM_FAKE=3,
} PlatformType;
typedef struct{
    FIXED x,y,vx;
    PlatformType type;
} Platform;
#define platform_extend 7
#define platform_extend_y 3
#define MAX_PLATFORMS 50
Platform platforms[MAX_PLATFORMS]; // use a fifo queue for object pooling
int platform_tail=0;

Platform* add_platform(FIXED x,FIXED y,PlatformType type){
    Platform* p=&platforms[platform_tail];
    p->x=x;
    p->y=y;
    p->type=type;
    p->vx=int2fx(0);
    platform_tail=(platform_tail+1)%MAX_PLATFORMS;
    return p;
}
void draw_platform(Platform* p){
    OBJ_ATTR *spr= &obj_buffer[obj_buffer_current++];
    int tile_idx=platform_tile_idx;
    switch(p->type){
        case PLATFORM_NORMAL:   tile_idx+=0;    break;
        case PLATFORM_MOVING:   tile_idx+=2;    break;
        case PLATFORM_FAKE:     tile_idx+=4;    break;
    }
    obj_set_attr(spr,ATTR0_WIDE,ATTR1_SIZE_16x8,ATTR2_BUILD(tile_idx,platform_pal_idx,0));
    int x=fx2int(p->x)-platform_extend;
    int y=fx2int(p->y);
    obj_set_pos(spr,x,y);
}
void update_platform(Platform* p){
    // check collision against player
    if(doodle.vy>0){
        if(p->x-int2fx(platform_extend+doodle_extend)<=doodle.x
        && doodle.x<=p->x+int2fx(platform_extend+doodle_extend)
        && doodle.last_y<=p->y && doodle.y>=p->y){
            if(p->type==PLATFORM_FAKE){
                p->type=PLATFORM_INACTIVE;
            }else{
                doodle.vy=-doodle.velo;
                doodle.y=p->y;
            }
        }
    }
    // move platform
    p->x=fxadd(p->x,p->vx);
    if(p->x<int2fx(platform_extend)){
        p->x=int2fx(platform_extend);
        p->vx=-p->vx;
    }
    if(p->x>int2fx(VIEWPORT_WIDTH-platform_extend)){
        p->x=int2fx(VIEWPORT_WIDTH-platform_extend);
        p->vx=-p->vx;
    }
    // destroy platform if out of screen
    if(p->y>int2fx(VIEWPORT_HEIGHT)){
        p->type=PLATFORM_INACTIVE;
    }
}
struct{
    int score;
    int next_plat_y;
    int fake_plat_y_total;
}game;
void init_game(){
    game.score=0;
    game.next_plat_y=VIEWPORT_HEIGHT;
    game.fake_plat_y_total=0;
}
#define min_doodle_screen_height (VIEWPORT_HEIGHT/2)
void update_camera(){
    int screen_scroll=min_doodle_screen_height-fx2int(doodle.y);
    if(screen_scroll<=0)return;

    doodle.y+=int2fx(screen_scroll);
    for(int i=0;i<MAX_PLATFORMS;i++)
        if(platforms[i].type!=PLATFORM_INACTIVE)
            platforms[i].y+=int2fx(screen_scroll);
    
    game.score+=screen_scroll;
    game.next_plat_y+=screen_scroll;
}
void update_platform_spawn(){

    int plat_y_interval;FIXED plat_vy;int freq_moving;int freq_fake;
    if(game.score<1){plat_y_interval=10;plat_vy=0;freq_moving=0;freq_fake=0;}
    else if(game.score<1500) {plat_y_interval=20;plat_vy=0;freq_moving=0;freq_fake=0;}
    else if(game.score<3000) {plat_y_interval=20;plat_vy=float2fx(0.3f/2);freq_moving=20;freq_fake=10;}
    else if(game.score<4500) {plat_y_interval=15;plat_vy=float2fx(0.3f/2);freq_moving=30;freq_fake=20;}
    else if(game.score<6000) {plat_y_interval=15;plat_vy=float2fx(0.6f/2);freq_moving=50;freq_fake=20;}
    else if(game.score<9000) {plat_y_interval=20;plat_vy=float2fx(0.9f/2);freq_moving=60;freq_fake=20;}
    else if(game.score<12000){plat_y_interval=20;plat_vy=float2fx(1.2f/2);freq_moving=60;freq_fake=30;}
    else {plat_y_interval=20;plat_vy=float2fx(1.5f/2);freq_moving=40;freq_fake=40;}


    if(game.next_plat_y>=plat_y_interval){
        game.next_plat_y-=plat_y_interval;
        int x=rand()%(VIEWPORT_WIDTH-2*platform_extend)+platform_extend;
        PlatformType t;
        int r=rand()%100;
        if(r<freq_moving)t=PLATFORM_MOVING;
        else if(r<freq_moving+freq_fake)t=PLATFORM_FAKE;
        else t=PLATFORM_NORMAL;
        if(t==PLATFORM_FAKE){//limit consecutive fake platforms
            game.fake_plat_y_total+=platform_extend_y;
            if(game.fake_plat_y_total>doodle.max_plat_dist){
                t=PLATFORM_NORMAL;
                game.fake_plat_y_total=0;
            }
        }

        add_platform(int2fx(x),int2fx(game.next_plat_y),t);
        if(t==PLATFORM_MOVING){
            platforms[platform_tail-1].vx=rand()%2==0?plat_vy:-plat_vy;
        }
    }

}
void draw_ui(){
    char str[32];
    siprintf(str,"%8d",min(game.score,99999999));
    bm_clrs(8,1,str,CLR_BG);
    bm_puts(8,1,str,CLR_BLACK);
}

int frame=0;

int main() 
{
    // ======== Load Resources ========
    reset_resource_idx();
    load_doodle_gfx();
    load_platform_gfx();

    // ======== Initialize Devices and Libraries ========
    irq_init(NULL);irq_add(II_VBLANK, NULL);
    txt_init_std();
    oam_init(obj_buffer,128);
	REG_DISPCNT= DCNT_MODE3 | DCNT_BG2 | DCNT_OBJ | DCNT_OBJ_1D;
    m3_fill(CLR_BLACK);
    draw_background();
    // ======== Initialize Game State ========
    init_game();
    init_doodle();

	while(1){
        //======== Updating Game Logic at VDraw(197120 cycles) ========
        profile_start();
        key_poll();
        update_doodle();
        for(int i=0;i<MAX_PLATFORMS;i++)
            if(platforms[i].type!=PLATFORM_INACTIVE)
                update_platform(&platforms[i]);
        update_camera();
        update_platform_spawn();
        
        //======== Drawing at obj_buffer, it is faster than doing it at VBlank, because  ========
        clear_obj_buffer();
        draw_doodle();
        for(int i=0;i<MAX_PLATFORMS;i++)
            if(platforms[i].type!=PLATFORM_INACTIVE)
                draw_platform(&platforms[i]);
        int time1=profile_stop();
        //======== Updating VRAM at VBlank (83776 cycles) ========
        VBlankIntrWait();
        profile_start();
        draw_ui();
        oam_copy(oam_mem,obj_buffer,128);
        int time2=profile_stop();
        //========Debug Display========
        char str[32];
        siprintf(str,"%d %d",time1,time2);
        bm_clrs(80,150,str,CLR_BLACK);
        bm_puts(80,150,str,CLR_WHITE);
        frame++;
    }

	return 0;
}
