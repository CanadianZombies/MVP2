#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"

void
hardcode_give_scripts (CHAR_DATA * ch, CHAR_DATA * mob, char *argy)
{
#ifdef NEW_WORLD
  if (mob->pIndexData->vnum == 1114)
    {
      check_fgt (ch);
      check_fgt (mob);
      mob_1114_2 (ch, mob, argy);
    }
/*if (mob->pIndexData->vnum==1514) {
   check_fgt(ch);
   check_fgt(mob);
   mob_1514_2(ch,mob,argy);
   } */
#endif
  return;
}

void
hardcode_move_scripts (CHAR_DATA * ch, CHAR_DATA * mob)
{
#ifdef NEW_WORLD
  if (mob->pIndexData->vnum == 1114)
    {
      check_fgt (ch);
      check_fgt (mob);
      mob_1114_1 (ch, mob, "none");
    }
/*if (mob->pIndexData->vnum==1514) {
   check_fgt(ch);
   check_fgt(mob);
   mob_1514_1(ch,mob,"none");
   } */
  if (mob->pIndexData->vnum == 101)
    {
      check_fgt (ch);
      check_fgt (mob);
      mob_101_1 (ch, mob, "none");
    }
  if (mob->pIndexData->vnum == 1029)
    {
      check_fgt (ch);
      check_fgt (mob);
      mob_1029_1 (ch, mob, "none");
    }
#endif
  return;
}

void
hardcode_room_enter (CHAR_DATA * ch, ROOM_DATA * room)
{
  return;
}

void
hardcode_interpret_scripts (CHAR_DATA * ch, CHAR_DATA * mob, char *argy)
{
#ifdef NEW_WORLD
/*if (mob->pIndexData->vnum==2013) {
   check_fgt(ch);
   check_fgt(mob);
   mob_2013_2(ch,mob,argy);
   } */
#endif
  return;
}

void
hardcode_saytell_scripts (CHAR_DATA * ch, CHAR_DATA * mob, char *argy)
{
#ifdef NEW_WORLD
/*if (mob->pIndexData->vnum==2013) {
   check_fgt(ch);
   check_fgt(mob);
   mob_2013_1(ch,mob,argy);
   } */
  if (mob->pIndexData->vnum == 101)
    {
      mob_101_2 (ch, mob, argy);
      check_fgt (ch);
      check_fgt (mob);
    }
  if (mob->pIndexData->vnum == 1029)
    {
      check_fgt (ch);
      check_fgt (mob);
      mob_1029_2 (ch, mob, argy);
    }
#endif
  return;
}
