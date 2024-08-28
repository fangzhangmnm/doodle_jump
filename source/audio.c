#include "audio.h"
#include "rng.h"

Song song_jump={
    .len=3,
    .notes=(s8[]){-64+5, 16+0, 16+7},
    .waits=(u8[]){0,     3,     3},
    .channels=(u8[]){4, 1,     1},
};
const Song song_fake={
    .len=1,
    .notes=(s8[]){-48+10},
    .waits=(u8[]){0},
    .channels=(u8[]){4},
};
void randomize_song_jump(){
    const int scales[]={9,16+0,16+2,16+4,16+7,16+9,32+0,32+2,32+4,32+7};
    int idx=rand()%(10-2);
    song_jump.notes[1]=scales[idx];
    song_jump.notes[2]=scales[idx+2];
}
const Song song_failure={
    .len=5,
    .notes=(s8[]){ -48+0, 16+7, 16+3, 16+0,0+9},
    .waits=(u8[]){ 0, 3,     3,    3,   3},
    .channels=(u8[]){4, 1,     1,    1,   1},
};
const Song* current_song;
int current_song_idx=0;
int current_song_wait=0;
void play_song(const Song* song){
    current_song=song;
    current_song_idx=0;
    current_song_wait=0;
}
void update_song(){
    while(true){
        if(current_song_idx>=current_song->len)return;
        if(current_song_wait>0){current_song_wait--;return;}
        int note= current_song->notes[current_song_idx];
        int channel= current_song->channels[current_song_idx];
        switch (channel){
            case 1: REG_SND1FREQ = SFREQ_RESET | SND_RATE(note&0xf, note>>4); break;
            case 4: REG_SND4FREQ = SFREQ_RESET | SND_RATE(note&0xf, note>>4); break;
            default: break;
        }
        current_song_wait= current_song->waits[current_song_idx];
        current_song_idx++;
    }
}

void configure_sound(){
    REG_SNDSTAT= SSTAT_ENABLE;
    REG_SNDDMGCNT= SDMG_BUILD_LR(SDMG_SQR1|SDMG_SQR2|SDMG_NOISE, 7);
    REG_SNDDSCNT= SDS_DMG100;
    REG_SND1SWEEP= SSW_OFF;
	REG_SND1CNT= SSQR_ENV_BUILD(12, 0, 3) | SSQR_DUTY1_2;
	REG_SND1FREQ= 0;
    REG_SND4CNT= SSQR_ENV_BUILD(15, 0, 1);
    REG_SND4FREQ= 0;

}
void update_sound(){
    update_song();
}