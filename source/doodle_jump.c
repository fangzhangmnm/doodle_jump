#include <tonc.h>
#include <stdlib.h>
#include "gfx_doodle.h"

#define VIEWPORT_WIDTH 72
#define VIEWPORT_HEIGHT 160


void draw_background(){
    m3_rect(0,0,VIEWPORT_WIDTH,VIEWPORT_HEIGHT,RGB15(30,29,29));
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
    doodle.velo=float2fx(5.5f/2);
    // max height=velo^2/(2*g)\approx 75
    doodle.max_plat_dist=75;
}
void draw_doodle(){
    int x=fx2int(doodle.x);
    int y=fx2int(doodle.y);
    int turn=doodle.turn;
    for(int j=0;j<=11;j++){
        int n=y+j-11;
        if(0<=n && n<VIEWPORT_HEIGHT)
        for(int i=0;i<=11;i++){
            int m;
            if(turn==1)m=x-7+i;else m=x-i+7;
            COLOR t=((u16*)gfx_doodleBitmap)[j*12+i];
            if(0<=m && m<VIEWPORT_WIDTH && t!=CLR_RED)
                m3_plot(m,n,t);
        }
    }		
}
void update_doodle(){
    if(key_is_down(KEY_LEFT))
        doodle.vx=fxsub(doodle.vx,doodle.acc);
    if(key_is_down(KEY_RIGHT))
        doodle.vx=fxadd(doodle.vx,doodle.acc);
    if(!key_is_down(KEY_LEFT) && !key_is_down(KEY_RIGHT)){
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
    PLATFORM_FAKE=2,
    PLATFORM_MOVING=3,
} PlatformType;
typedef struct{
    FIXED x,y,vx;
    PlatformType type;
} Platform;
const int platform_extend=6;
const int platform_extend_y=2;
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
    int x=fx2int(p->x);
    int y=fx2int(p->y);
    switch(p->type){
        case PLATFORM_NORMAL:
            m3_frame(x-platform_extend,y,x+platform_extend+1,y+platform_extend_y+1,CLR_BLACK);
            m3_rect(x-platform_extend+1,y+1,x+platform_extend,y+platform_extend_y,RGB15(5,28,8));
            break;
        case PLATFORM_FAKE:
            m3_frame(x-platform_extend,y,x+platform_extend+1,y+platform_extend_y+1,CLR_BLACK);
            m3_rect(x-platform_extend+1,y+1,x+platform_extend,y+platform_extend_y,RGB15(15,3,8));
            m3_vline(x,y,y+platform_extend_y,CLR_BLACK);
            break;
        case PLATFORM_MOVING:
            m3_frame(x-platform_extend,y,x+platform_extend+1,y+platform_extend_y+1,CLR_BLACK);
            m3_rect(x-platform_extend+1,y+1,x+platform_extend,y+platform_extend_y,RGB15(9,25,29));
            break;
        default:
            break;
    }
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


const int min_doodle_screen_height=VIEWPORT_HEIGHT/2;
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


	// NextPlat=20;PlatV=0;FreqFake=100;FreqMove=100;
	// if(Score>1500){NextPlat=20;PlatV=0.3;FreqFake=70;FreqMove=80;}
	// if(Score>3000){NextPlat=15;PlatV=0.3;FreqFake=50;FreqMove=70;}
	// if(Score>4500){NextPlat=15;PlatV=0.6;FreqFake=30;FreqMove=50;}
	// if(Score>6000){NextPlat=20;PlatV=0.9;FreqFake=30;FreqMove=50;}
	// if(Score>9000){NextPlat=20;PlatV=1.2;FreqFake=10;FreqMove=40;}
	// if(Score>12000){NextPlat=20;PlatV=1.5;FreqFake=0;FreqMove=40;}

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


int frame=0;

int main() 
{
	REG_DISPCNT= DCNT_MODE3 | DCNT_BG2;
    irq_init(NULL);
    irq_add(II_VBLANK, NULL);

    init_game();
    init_doodle();
    m3_fill(RGB15(0,0,0));

	while(1){
        VBlankIntrWait();
        key_poll();
        update_doodle();
        for(int i=0;i<MAX_PLATFORMS;i++)
            if(platforms[i].type!=PLATFORM_INACTIVE)
                update_platform(&platforms[i]);
        update_camera();
        update_platform_spawn();
        draw_background();
        for(int i=0;i<MAX_PLATFORMS;i++)
            if(platforms[i].type!=PLATFORM_INACTIVE)
                draw_platform(&platforms[i]);
        draw_doodle();
        frame++;
    }

	return 0;
}
