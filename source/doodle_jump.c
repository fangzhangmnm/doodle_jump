#include <tonc.h>

#include "resources.h"
#include "utils.h"
#include "audio.h"


#define VIEWPORT_WIDTH 72
#define VIEWPORT_HEIGHT 160
struct{
    int dy;
    int sbb;
}background;
struct{
    int score;
    int next_plat_y;
    int fake_plat_y_total;
    int screen_scroll;
    BOOL game_over;
}game;
typedef enum{
    DOODLE_ANIM_LOCOMOTION,
    DOODLE_ANIM_SHOOT,
} DoodleAnimState;
struct{
    FIXED x,y,vx,vy,last_y;
    FIXED acc,dec,g,max_vx,velo;
    FIXED bullet_vy;
    FIXED extend;
    int turn;
    int max_plat_dist;
    int shoot_cooldown;
    int shoot_timer;
    int anim_timer;
    DoodleAnimState anim_state;
} doodle;
typedef enum{
    PLATFORM_INACTIVE=0,
    PLATFORM_NORMAL=1,
    PLATFORM_MOVING=2,
    PLATFORM_FAKE=3,
} PlatformType;
typedef struct{
    PlatformType type;
    FIXED x,y,vx;
    FIXED extend,extend_y;
} Platform;
#define MAX_PLATFORMS 50
Platform platforms[MAX_PLATFORMS]; // use a fifo queue for object pooling
int platform_tail=0;
typedef enum{
    MONSTER_INACTIVE=0,
    MONSTER_TALL,
    MONSTER_WIDE,
    MONSTER_WALK,
    MONSTER_FLY,
} MonsterType;
typedef struct{
    MonsterType type;
    FIXED x,y;
    FIXED extend,extend_y;
} Monster;
#define MAX_MONSTERS 16
Monster monsters[MAX_MONSTERS];
int monster_tail=0;
typedef enum{
    BULLET_INACTIVE=0,
    BULLET_NORMAL=1,
} BulletType;
typedef struct{
    BulletType type;
    FIXED x,y,vx,vy;
} Bullet;
#define MAX_BULLETS 16
Bullet bullets[MAX_BULLETS];
int bullet_tail=0;

// ---------- Background ----------
void configure_background(){
    REG_DISPCNT|=DCNT_BG2;
    background.sbb=current_bg_sbb;current_bg_sbb--;
    for(int i=0;i<32*32;i++)se_mem[background.sbb][i]=GET_CB(gfx_bg_tile_idx)|gfx_bg_pal_idx<<12;
    REG_BG2CNT=BG_CBB(GET_CBB(gfx_bg_tile_idx))|BG_SBB(background.sbb)|BG_4BPP|BG_REG_32x32|BG_PRIO(2);

    REG_DISPCNT|=DCNT_BG3;
    REG_BG3CNT=BG_CBB(GET_CBB(gfx_bg_drop_tile_idx))|BG_SBB(gfx_bg_drop_sbb_idx)|BG_4BPP|BG_REG_32x32|BG_PRIO(3);
    REG_BG3HOFS=0;REG_BG3VOFS=0;
}
void draw_background(){
    REG_BG2HOFS=0;REG_BG2VOFS=-(background.dy>>3);
}
// ---------- Doodle ----------
// const int doodle_extend=5;
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
    doodle.max_vx=float2fx(2.0f/2); // max jump height should less than half screen height to prevent platform despawning
    doodle.velo=float2fx(5.0f/2);
    doodle.max_plat_dist=60; // max jump height=velo^2/(2*g)=62.5
    doodle.bullet_vy=float2fx(-3.0f);
    doodle.extend=float2fx(5);
    doodle.shoot_cooldown=10;
    doodle.shoot_timer=0;
    doodle.anim_timer=0;
    doodle.anim_state=DOODLE_ANIM_LOCOMOTION;
}
void debug_clear_surface(){
    // Slow. just for debug
    // schr4c_rect(&tte_get_context()->dst,0,0,VIEWPORT_WIDTH,VIEWPORT_HEIGHT,0);
    tte_erase_rect(0,0,VIEWPORT_WIDTH,VIEWPORT_HEIGHT);
}
void debug_draw_cross(int x,int y){
    // Slow. just for debug
    schr4c_hline(&tte_get_context()->dst,0,y,VIEWPORT_WIDTH-1,1);
    schr4c_vline(&tte_get_context()->dst,x,0,VIEWPORT_HEIGHT-1,1);
}
void draw_doodle(){
    OBJ_ATTR *spr= &obj_buffer[obj_buffer_current++];
    // obj_set_attr(spr,ATTR0_SQUARE,ATTR1_SIZE_16,ATTR2_BUILD(gfx_doodle_tile_idx,gfx_doodle_pal_idx,3));
    int x=fx2int(doodle.x)-8;
    int y=fx2int(doodle.y)-16;
    int tile_idx=gfx_doodle_tile_idx;
    switch(doodle.anim_state){
        case DOODLE_ANIM_LOCOMOTION: tile_idx+=0; break;
        case DOODLE_ANIM_SHOOT: tile_idx+=4; break;
    }
    obj_set_attr(spr,ATTR0_SQUARE,ATTR1_SIZE_16,ATTR2_ID(tile_idx)|ATTR2_PALBANK(gfx_doodle_pal_idx)|ATTR2_PRIO(1));
    if(!doodle.turn){spr->attr1|=ATTR1_HFLIP;}
    obj_set_pos(spr,x,y);
}
Bullet* add_bullet(FIXED x,FIXED y,FIXED vx,FIXED vy);
void update_doodle(){
    // Screen Scroll
    doodle.y+=int2fx(game.screen_scroll);
    // Locomotion
    doodle.vx=fxadd(doodle.vx,key_tri_horz()*doodle.acc);
    if(!key_tri_horz()){
        if(doodle.vx>0)
            doodle.vx=max(fxsub(doodle.vx,doodle.dec),0);
        if(doodle.vx<0)
            doodle.vx=min(fxadd(doodle.vx,doodle.dec),0);
    }
    doodle.vx=clamp(doodle.vx,-doodle.max_vx,doodle.max_vx);
    doodle.vy=fxadd(doodle.vy,doodle.g);
    doodle.x=fxadd(doodle.x,doodle.vx);
    if(doodle.x<int2fx(0))doodle.x=int2fx(VIEWPORT_WIDTH-1);
    if(doodle.x>int2fx(VIEWPORT_WIDTH-1))doodle.x=int2fx(0);
    doodle.last_y=doodle.y;
    doodle.y=fxadd(doodle.y,doodle.vy);
    if(doodle.vx<0)doodle.turn=1;
    if(doodle.vx>0)doodle.turn=0;
    // Shooting
    if(doodle.shoot_timer>0)doodle.shoot_timer--;
    else{
        if(key_hit(KEY_A)){
            doodle.shoot_timer=doodle.shoot_cooldown;
            // play_song(TODO);
            add_bullet(doodle.x,doodle.y-int2fx(8),0,doodle.bullet_vy);
            doodle.anim_state=DOODLE_ANIM_SHOOT;
            doodle.anim_timer=20;
        }
    }
    // Animation
    if(doodle.anim_timer>0)doodle.anim_timer--;
    else{
        doodle.anim_state=DOODLE_ANIM_LOCOMOTION;
    }


    // if(doodle.y>int2fx(VIEWPORT_HEIGHT-1)){ // bounce off the floor, for testing
    //     doodle.y=int2fx(VIEWPORT_HEIGHT-1);
    //     doodle.vy=-doodle.velo;
    // }
    if(doodle.y>int2fx(VIEWPORT_HEIGHT+16)){
        play_song(&song_failure);
        game.game_over=true;
    }
}
// ---------- Bullets ----------
Bullet* add_bullet(FIXED x,FIXED y,FIXED vx,FIXED vy){
    Bullet* p=&bullets[bullet_tail];
    p->x=x;
    p->y=y;
    p->vx=vx;
    p->vy=vy;
    p->type=BULLET_NORMAL;
    bullet_tail++;
    if(bullet_tail>=MAX_BULLETS)bullet_tail=0;
    return p;
}
void draw_bullet(Bullet* p){
    if(p->type==BULLET_INACTIVE)return;
    OBJ_ATTR *spr= &obj_buffer[obj_buffer_current++];
    obj_set_attr(spr,ATTR0_SQUARE,ATTR1_SIZE_8,ATTR2_ID(gfx_doodle_tile_idx+8)|ATTR2_PALBANK(gfx_doodle_pal_idx)|ATTR2_PRIO(1));
    int x=fx2int(p->x)-2;
    int y=fx2int(p->y)-2;
    obj_set_pos(spr,x,y);
}
void update_bullet(Bullet* p){
    if(p->type==BULLET_INACTIVE)return;
    p->x=fxadd(p->x,p->vx);
    p->y=fxadd(p->y,p->vy);
    if(p->y<int2fx(0)-2)p->type=BULLET_INACTIVE;
}
void clear_bullets(){
    for(int i=0;i<MAX_BULLETS;i++)
        bullets[i].type=BULLET_INACTIVE;
}
// ---------- Monsters ----------
Monster* add_monster(FIXED x,FIXED y,MonsterType type){
    Monster* p=&monsters[monster_tail];
    p->x=x;
    p->y=y;
    p->type=type;
    switch(type){
        case MONSTER_TALL:  p->extend=int2fx(6); p->extend_y=int2fx(19); break;
        case MONSTER_WIDE:  p->extend=int2fx(14);p->extend_y=int2fx(13); break;
        case MONSTER_WALK:  p->extend=int2fx(6); p->extend_y=int2fx(14); break;
        case MONSTER_FLY:   p->extend=int2fx(7); p->extend_y=int2fx(15); break;
        default: return NULL;
    }
    monster_tail++;
    if(monster_tail>=MAX_MONSTERS)monster_tail=0;
    return p;
}
void draw_monster(Monster* p){
    if(p->type==MONSTER_INACTIVE)return;
    int tile_idx=gfx_monsters_tile_idx;
    int attr0,attr1;
    int x=fx2int(p->x),y=fx2int(p->y);
    switch(p->type){
        case MONSTER_TALL:  tile_idx+=0; attr0=ATTR0_TALL; attr1=ATTR1_SIZE_16x32; x-=8; y-=32; break;
        case MONSTER_WIDE:  tile_idx+=8; attr0=ATTR0_WIDE; attr1=ATTR1_SIZE_32x16; x-=16; y-=16; break;
        case MONSTER_WALK:  tile_idx+=16;attr0=ATTR0_SQUARE; attr1=ATTR1_SIZE_16; x-=8; y-=16; break;
        case MONSTER_FLY:   tile_idx+=20;attr0=ATTR0_SQUARE; attr1=ATTR1_SIZE_16; x-=8; y-=16; break;
        default: return;
    }
    OBJ_ATTR *spr= &obj_buffer[obj_buffer_current++];
    obj_set_attr(spr,attr0,attr1,ATTR2_ID(tile_idx)|ATTR2_PALBANK(gfx_monsters_pal_idx)|ATTR2_PRIO(1));
    obj_set_pos(spr,x,y);

    // debug_draw_cross(fx2int(p->x),fx2int(p->y));
}



void update_monster(Monster* m){
    if(m->type==MONSTER_INACTIVE)return;
    m->y+=int2fx(game.screen_scroll);
    if(m->y>int2fx(VIEWPORT_HEIGHT+64)){
        m->type=MONSTER_INACTIVE;return;
    }
    // check collision against bullets
    for(int i=0;i<MAX_BULLETS;i++){
        Bullet* b=&bullets[i];
        if(b->type==BULLET_INACTIVE)continue;
        BOOL collide=point_box(b->x,b->y,m->x-m->extend,m->x+m->extend,m->y-m->extend_y,m->y);
        if(collide){
            m->type=MONSTER_INACTIVE;
            b->type=BULLET_INACTIVE;
            // play_song(&TODO);
        }
    }

    // check collision against player
    BOOL collide=point_box(doodle.x,doodle.y,m->x-m->extend,m->x+m->extend,m->y-m->extend_y,m->y);
    if(collide){
        if(doodle.vy>0){
            randomize_song_jump();
            play_song(&song_jump);
            doodle.vy=-doodle.velo;
            m->type=MONSTER_INACTIVE;
        }else{
            play_song(&song_failure);
            game.game_over=true;
        }
    }
}
void clear_monsters(){
    for(int i=0;i<MAX_MONSTERS;i++)
        monsters[i].type=MONSTER_INACTIVE;
}
// ---------- Platforms ----------
# define default_platform_extend 7

Platform* add_platform(FIXED x,FIXED y,PlatformType type){
    Platform* p=&platforms[platform_tail];
    p->x=x;
    p->y=y;
    p->type=type;
    p->vx=int2fx(0);
    p->extend=int2fx(default_platform_extend);
    p->extend_y=int2fx(4);
    platform_tail++;
    if(platform_tail>=MAX_PLATFORMS)platform_tail=0;
    return p;
}
void draw_platform(Platform* p){
    if(p->type==PLATFORM_INACTIVE)return;
    int tile_idx=gfx_platform_tile_idx;
    switch(p->type){
        case PLATFORM_NORMAL:   tile_idx+=0;    break;
        case PLATFORM_MOVING:   tile_idx+=2;    break;
        case PLATFORM_FAKE:     tile_idx+=4;    break;
        default: return;
    }
    OBJ_ATTR *spr= &obj_buffer[obj_buffer_current++];
    obj_set_attr(spr,ATTR0_WIDE,ATTR1_SIZE_16x8,ATTR2_ID(tile_idx)|ATTR2_PALBANK(gfx_platform_pal_idx)|ATTR2_PRIO(1));
    int x=fx2int(p->x)-8;
    int y=fx2int(p->y);
    obj_set_pos(spr,x,y);
}
void update_platform(Platform* p){
    if(p->type==PLATFORM_INACTIVE)return;
    p->y+=int2fx(game.screen_scroll);
    // destroy platform if out of screen
    if(p->y>int2fx(VIEWPORT_HEIGHT)){
        p->type=PLATFORM_INACTIVE;return;
    }
    // check collision against player
    BOOL collide=(
        doodle.vy>0
        && p->x-(p->extend+doodle.extend)<=doodle.x
        && doodle.x<=p->x+(p->extend+doodle.extend)
        && doodle.last_y<=p->y 
        && doodle.y>=p->y);
    if(collide){
        if(p->type==PLATFORM_FAKE){
            play_song(&song_fake);
            p->type=PLATFORM_INACTIVE;
        }else{
            randomize_song_jump();
            play_song(&song_jump);
            doodle.vy=-doodle.velo;
            doodle.y=p->y;
        }
    }
    // move platform
    p->x=fxadd(p->x,p->vx);
    if(p->x<p->extend){
        p->x=p->extend;
        p->vx=-p->vx;
    }
    if(p->x>int2fx(VIEWPORT_WIDTH)-p->extend){
        p->x=int2fx(VIEWPORT_WIDTH)-p->extend;
        p->vx=-p->vx;
    }
}
void clear_platforms(){
    for(int i=0;i<MAX_PLATFORMS;i++)
        platforms[i].type=PLATFORM_INACTIVE;
}
// ---------- Game Manager ----------
void init_game(){
    game.score=0;
    game.next_plat_y=VIEWPORT_HEIGHT;
    game.fake_plat_y_total=0;
    game.game_over=FALSE;
}
#define min_doodle_screen_height ((VIEWPORT_HEIGHT)/2)
void update_game(){
    game.screen_scroll=max(0,min_doodle_screen_height-fx2int(doodle.y));
    background.dy+=game.screen_scroll;
    
    game.score+=game.screen_scroll;
    game.next_plat_y+=game.screen_scroll;
}
void update_level_spawn(){
    if(key_is_down(KEY_L) && key_hit(KEY_R))game.score+=1500;

    int plat_y_interval;FIXED plat_vy;int freq_moving;int freq_fake;int freq_monster;
    if(game.score<1){plat_y_interval=10;plat_vy=0;freq_moving=0;freq_fake=0;freq_monster=0;}
    else if(game.score<1500) {plat_y_interval=20;plat_vy=0;freq_moving=0;freq_fake=0;freq_monster=0;}
    else if(game.score<3000) {plat_y_interval=20;plat_vy=float2fx(0.3f/2);freq_moving=20;freq_fake=10;freq_monster=5;}
    else if(game.score<4500) {plat_y_interval=15;plat_vy=float2fx(0.3f/2);freq_moving=30;freq_fake=20;freq_monster=5;}
    else if(game.score<6000) {plat_y_interval=15;plat_vy=float2fx(0.6f/2);freq_moving=50;freq_fake=20;freq_monster=10;}
    else if(game.score<9000) {plat_y_interval=20;plat_vy=float2fx(0.9f/2);freq_moving=60;freq_fake=20;freq_monster=10;}
    else if(game.score<12000){plat_y_interval=20;plat_vy=float2fx(1.2f/2);freq_moving=60;freq_fake=30;freq_monster=10;}
    else {plat_y_interval=20;plat_vy=float2fx(1.5f/2);freq_moving=40;freq_fake=40;freq_monster=15;}
    // if(game.score>0){ //This part is for debugging level settings
    //     freq_moving=0;freq_fake=100;plat_y_interval=15;freq_monster=100;
    // }

    if(game.next_plat_y>=plat_y_interval){
        game.next_plat_y-=plat_y_interval;
        int x=rand()%(VIEWPORT_WIDTH-2*default_platform_extend)+default_platform_extend;
        PlatformType t;
        int r=rand()%100;
        if(r<freq_moving)t=PLATFORM_MOVING;
        else if(r<freq_moving+freq_fake)t=PLATFORM_FAKE;
        else t=PLATFORM_NORMAL;
        if(t==PLATFORM_FAKE){//limit consecutive fake platforms
            game.fake_plat_y_total+=plat_y_interval;
            if(game.fake_plat_y_total>=doodle.max_plat_dist){
                t=PLATFORM_NORMAL;
                game.fake_plat_y_total=0;
            }
        }

        Platform* platform=add_platform(int2fx(x),int2fx(game.next_plat_y),t);
        if(t==PLATFORM_MOVING){
            platform->vx=(rand()&0x1)==0?plat_vy:-plat_vy;
        }

        if(rand()%100<freq_monster){
            int r=rand()&0x3;
            if(r<1)add_monster(int2fx(x),int2fx(game.next_plat_y),MONSTER_TALL);
            else if(r<2)add_monster(int2fx(x),int2fx(game.next_plat_y),MONSTER_WIDE);
            else if(r<3)add_monster(int2fx(x),int2fx(game.next_plat_y),MONSTER_WALK);
            else add_monster(int2fx(x),int2fx(game.next_plat_y),MONSTER_FLY);
        }
    }



}
// ---------- UI ----------
void configure_ui(){
    tte_init_chr4c(
        0,
        BG_CBB(TONC_CBB)|BG_SBB(TONC_SBB)|BG_4BPP|BG_REG_32x32|BG_PRIO(0),
        TONC_BG_PAL_IDX<<12,
        bytes2word(13,15,0,0),
        CLR_BLACK,
        &verdana9_b4Font,
        (fnDrawg)chr4c_drawg_b4cts_fast);
    tte_set_color(TTE_INK,CLR_WHITE);
    tte_set_color(TTE_SHADOW,CLR_BLACK);

    REG_WIN0CNT=WIN_ALL|WIN_BLD;
    REG_BLDCNT= (BLD_ALL&~BIT(0)) | BLD_BLACK;
    REG_BLDY=8;
    REG_DISPCNT|=DCNT_WIN0;
    REG_DISPCNT|=DCNT_BG0;
}
void draw_ui(){
    char str[32];
    REG_WIN0H=0<<8|VIEWPORT_WIDTH;
    REG_WIN0V=0<<8|10;
    siprintf(str,"Score:%8d",min(game.score,99999999));
    tte_set_pos(8,-2);
    tte_erase_rect(0,0,VIEWPORT_WIDTH,10);
    tte_write(str);
}
// ========== Menus ==========
int menu_game_over(){
    REG_WIN0H=0<<8|VIEWPORT_WIDTH;
    REG_WIN0V=0<<8|VIEWPORT_HEIGHT;
    tte_set_pos(10,72);
    tte_write("Game Over");
    tte_set_pos(8,80);
    tte_write("Press Start");
    while(1){
        key_poll();
        VBlankIntrWait();
        update_sound();
        if(key_hit(KEY_START)){
            tte_erase_rect(0,0,VIEWPORT_WIDTH,VIEWPORT_HEIGHT);
            return 0;
        }
    }
}

int main() 
{
    irq_init(NULL);irq_add(II_VBLANK, NULL);
    txt_init_std();
    oam_init(obj_buffer,128);
 
    load_resources();
    
    REG_DISPCNT= DCNT_MODE0 | DCNT_OBJ | DCNT_OBJ_1D;
    REG_WIN1H=0<<8|VIEWPORT_WIDTH;
    REG_WIN1V=0<<8|VIEWPORT_HEIGHT;
    REG_WIN1CNT=WIN_ALL;
    REG_WINOUTCNT=WIN_BG0|WIN_BG3;
    REG_DISPCNT|=DCNT_WIN1;
    configure_background();
    configure_ui();
    configure_sound();
    while(1){
        // ======== Initialize GameObjects ========
        init_game();
        init_doodle();
        clear_platforms();
        clear_monsters();
        clear_bullets();

        while(!game.game_over){
            //======== Updating Game Logic at VDraw(197120 cycles) ========
            profile_start();
            key_poll();
            update_game();
            update_doodle();
            for(int i=0;i<MAX_MONSTERS;i++)
                update_monster(&monsters[i]);
            for(int i=0;i<MAX_PLATFORMS;i++)
                update_platform(&platforms[i]);
            for(int i=0;i<MAX_BULLETS;i++)
                update_bullet(&bullets[i]);

            update_level_spawn();
            
            //======== Drawing at obj_buffer, it is faster than doing it at VBlank, because  ========
            clear_objects();
            draw_doodle();
            for(int i=0;i<MAX_MONSTERS;i++)
                draw_monster(&monsters[i]);
            for(int i=0;i<MAX_PLATFORMS;i++)
                draw_platform(&platforms[i]);
            for(int i=0;i<MAX_BULLETS;i++)
                draw_bullet(&bullets[i]);
            int time1=profile_stop();
            //======== Updating VRAM at VBlank (83776 cycles) ========
            VBlankIntrWait();
            profile_start();
            update_sound();
            draw_ui();
            oam_copy(oam_mem,obj_buffer,128);
            draw_background();
            int time2=profile_stop();
            //========Debug Display (Not included into profiling)========
            char str[32];
            siprintf(str,"%d %d",time1,time2);
            tte_set_pos(VIEWPORT_WIDTH,148);
            tte_erase_rect(VIEWPORT_WIDTH,148,240,160);
            tte_write(str);
        }
        menu_game_over();
    }
	return 0;
}