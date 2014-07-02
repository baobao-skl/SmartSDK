#include "play_list.h"
#include "music.h"
#include <malloc.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

extern music_list_item_t *music_head;


void output_all_music_info(music_list_item_t *item,char *music_list)
{
	char buffer[1024] = {0};
	music_list_item_t *curr = item;
	strcat(music_list,"{\"ver\":\"");
	strcat(music_list,"ARM11.000.000.000.001");
	strcat(music_list,"\",\"music\":[");

	while(curr!=NULL){
		//DEBUG_MSG("item name:%s author:%s length:%s\n",curr->item.name,curr->item.author,curr->item.length);
		sprintf(buffer,"{\"name\":\"%s\",\"author\":\"%s\",\"len\":\"%s\"},",
			curr->item.name,
			curr->item.author,
			curr->item.length);
			strcat(music_list,buffer);
		curr = curr->next;
		if(curr==NULL || curr==item)break;
	}
	music_list[strlen(music_list)-1] = ']';
	strcat(music_list,"}\0");
}

music_list_item_t *create_music_song(int *len)
{
	music_list_item_t *head = NULL;
	music_list_item_t *p1 = NULL;	
	music_list_item_t *p2 = NULL;

	DIR *dir = NULL;
	struct dirent *file_t = NULL;
	*len = 0;
	dir = opendir(MUSIC_FOLDER);
	while((file_t = readdir(dir)) != NULL){
		if(strcmp(file_t->d_name,".")==0 || strcmp(file_t->d_name,"..")==0)
			continue;
		if(strstr(file_t->d_name,".mp3")){
			(*len) ++;
			if(*len == 1){
				p1 = (music_list_item_t *)malloc(sizeof(music_list_item_t));
				if(p1 == NULL){
					*len = 0;
					return NULL;
				}
				strcpy(p1->item.author,"liudehua");
				strcpy(p1->item.name,file_t->d_name);
				strcpy(p1->item.length,"00:01:00");
				head = p1;
			}else{
				p2 = p1;
				p1 = (music_list_item_t *)malloc(sizeof(music_list_item_t));
				if(p1 == NULL){
					DEBUG_MSG("malloc memory error!\n");
					*len = 0;
					return NULL;
				}
				strcpy(p1->item.author,"liudehua");
				strcpy(p1->item.length,"00:01:00");
				strcpy(p1->item.name,file_t->d_name);
				p2->next = p1;
				p1->prev = p2;
			}
		}
	}
	closedir(dir);
	p1->next = head;
	head->prev = p1;
	p1 = NULL;
	p2 = NULL;
	return head;
}

music_list_item_t *look_for_music(const char *musicname)
{
	music_list_item_t *next1 = NULL;
	if(strcmp(music_head->item.name,musicname)==0){
		return music_head;
	}
	next1 = music_head->next;
	while(next1 !=NULL && next1!=music_head){
		if(strcmp(next1->item.name,musicname)==0){
			DEBUG_MSG("find it: name:%s author:%s length:%s\n",next1->item.name,next1->item.author,next1->item.length);
			break;
		}
		next1 = next1->next;
	}
	return next1;
}


void music_destory(music_list_item_t *L,int len)
{
	music_list_item_t *tempL = NULL ,*temp2 = L;
	int count  = 0;
	while(count++ < len && temp2!=NULL){
		tempL = temp2->next;
		if(tempL != NULL)free(temp2);
		temp2 = tempL;
	}
	tempL = NULL;
	temp2 = NULL;
}
