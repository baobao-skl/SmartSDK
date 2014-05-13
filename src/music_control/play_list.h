#ifndef __PLAY_LIST_H__
#define __PLAY_LIST_H__

#include "music.h"

void output_all_music_info(music_list_item_t *item,char *music_list);
music_list_item_t *create_music_song(int *len);
music_list_item_t *look_for_music(const char *musicname);
void music_destory(music_list_item_t *L,int len);


#endif
