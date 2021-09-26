#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include "emlen.h"
#ifndef WINDOWS
#include <sys/time.h>
#endif
#define ATTACH_TO_MOB(func, vnum, trigger, keywords)

/* Basic stuctures */

#define MOB_FUNCTION(text)   void (text)(CHAR_DATA *ch, CHAR_DATA *mob, char *argy)
#define END_FUNCTION         mob->fgt->running_char_function=NULL; mob->fgt->script_delay=0; mob->fgt->tag_in_proc=0; if (mob->fgt->argy) {free_string(mob->fgt->argy); mob->fgt->argy=NULL;} mob->fgt->caller=NULL; return

/* Aliases for characters and mobs */

#define DELAY_SCRIPT(val)             mob->fgt->script_delay+=(val); mob->fgt->tag_in_proc=mob_chunk+1; mob->fgt->running_char_function=cf; mob->fgt->caller=ch; if (!mob->fgt->argy) mob->fgt->argy=str_dup(argy); return
#define MARK_CHUNK(val)               mob_chunk=val; if ( mob->fgt->tag_in_proc <= (val) )

#define CHECK_INTEGRITY(ch,func)   check_fgt((ch)); check_fgt(mob); if ( (ch)->data_type==-1 ) {END_FUNCTION;} cf=(func)
#define DONT_ALLOW_INTERRUPTIONS  if ( (mob->fgt->caller!=NULL && ch!=mob->fgt->caller) || mob->fgt->script_delay!=0 ) return
#define ALLOW_INTERRUPTIONS  if (mob->fgt->caller!=NULL && mob->fgt->caller!=ch) mob->fgt->tag_in_proc=0;
