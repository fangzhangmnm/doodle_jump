#ifndef AUDIO_H
#define AUDIO_H
#include <tonc.h>

// ========== Audio ==========
// C C# D D# E F F# G G# A A# B
// 0 1  2 3  4 5 6  7 8  9 10 11
typedef struct{
    int len;
    s8* notes;
    u8* waits;
    u8* channels;
} Song;

extern Song song_jump;
extern const Song song_fake,song_failure;
void randomize_song_jump();
void play_song(const Song* song);
void update_song();
void configure_sound();
void update_sound();



#endif // AUDIO_H