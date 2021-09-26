#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include "mpv.h"

#define ATTACH_TO_OBJ(func, vnum, trigger, keywords)

/* Basic stuctures */

#define OBJ_FUNCTION(text)   void (text)(CHAR_DATA *ch, SINGLE_OBJECT *obj, char *argy)
#define END_FUNCTION         obj->running_char_function=NULL; obj->script_delay=0; obj->tag_in_proc=0; if (obj->argy) {free_string(obj->argy); obj->argy=NULL;} obj->caller=NULL; return

/* Aliases for characters and mobs */

#define DELAY_SCRIPT(val)             obj->script_delay+=(val); obj->tag_in_proc=obj_chunk+1; obj->running_obj_function=oj; obj->caller=ch; if (obj->argy!=NULL) free_string(obj->argy); obj->argy=str_dup(argy); return
#define MARK_CHUNK(val)               obj_chunk=val; if ( obj->tag_in_proc <= (val) )

#define CHECK_INTEGRITY(ch,func)   if ( (ch)->data_type==-1 ) {END_FUNCTION;} oj=(func)
#define DONT_ALLOW_INTERRUPTIONS  if ( (obj->caller!=NULL && ch!=obj->caller) || obj->script_delay!=0 ) return
#define ALLOW_INTERRUPTIONS  if (obj->caller!=NULL && obj->caller!=ch) obj->tag_in_proc=0;
