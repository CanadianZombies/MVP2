#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include "emlen.h"

#define ATTACH_TO_ROOM(func, vnum, trigger, keywords)

/* Basic stuctures */

#define ROOM_FUNCTION(text)  void (text)(CHAR_DATA *ch, ROOM_DATA *room, char *argy)
#define END_FUNCTION         room->more->running_room_function=NULL; room->more->script_delay=0; room->more->tag_in_proc=0; if (room->more->argy) {free_string(room->more->argy); room->more->argy=NULL;} room->more->caller=NULL; return

#define DELAY_SCRIPT(val)    room->more->script_delay+=(val); room->more->tag_in_proc=room_chunk+1; room->more->running_room_function=rf; room->more->caller=ch; if (room->more->argy==NULL) room->more->argy=str_dup(argy); return
#define MARK_CHUNK(val)      room_chunk=val; if ( room->more->tag_in_proc <= (val) )

#define CHECK_INTEGRITY(ch,func)       if ( (ch)->data_type==-1 ) {END_FUNCTION;} rf=(func)
#define DONT_ALLOW_INTERRUPTIONS  if ((room->more->caller!=NULL && ch!=room->more->caller) || room->more->script_delay!=0) return
#define ALLOW_INTERRUPTIONS       if (room->more->caller!=NULL && room->more->caller!=ch) room->tag_in_proc=0;
