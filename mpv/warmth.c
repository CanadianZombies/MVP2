#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"
int
get_warmth (CHAR_DATA * ch)
{
  SINGLE_OBJECT *obj;
  int cnt;
  cnt = 0;
  if (IS_MOB (ch))
    return -1;
  for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
      if (obj->wear_loc != -1 && obj->pIndexData->item_type == ITEM_ARMOR)
	cnt += ((I_ARMOR *) obj->more)->warmth;
    }
  return cnt;
}

bool
too_warm (int warmth, CHAR_DATA * ch)
{
  int temp = cur_temp (ch);
  if (temp < 80)
    return FALSE;
  if (temp > 80 && temp < 91 && warmth > 48)
    return TRUE;
  if (temp > 90 && temp < 101 && warmth > 37)
    return TRUE;
  if (temp > 100 && temp < 111 && warmth > 24)
    return TRUE;
  if (temp > 110 && temp < 121 && warmth > 13)
    return TRUE;
  if (temp > 120)
    return TRUE;
  return FALSE;
}

bool
too_cold (int warmth, CHAR_DATA * ch)
{
  int temp = cur_temp (ch);
  if (temp > 44)
    return FALSE;
  if (temp > 37 && temp < 45 && warmth < 14)
    return TRUE;
  if (temp > 20 && temp < 38 && warmth < 21)
    return TRUE;
  if (temp > 1 && temp < 21 && warmth < 34)
    return TRUE;
  if (temp > -13 && temp < 2 && warmth < 44)
    return TRUE;
  if (temp > -33 && temp < -14 && warmth < 54)
    return TRUE;
  if (temp <= -33)
    return TRUE;
  return FALSE;
}

bool
check_body_temp (CHAR_DATA * ch)
{
  int warmth;
  bool shirt;
  bool shoes;
  bool pants;
  SINGLE_OBJECT *obj;
  int temp = cur_temp (ch);
  if (!pow.weather)
    return FALSE;
  if (number_range (1, 3) != 2)
    return FALSE;
  shirt = FALSE;
  shoes = FALSE;
  pants = FALSE;
  if (ch->in_room && ch->in_room->vnum < 1000)
    return FALSE;
  if (IS_MOB (ch) || !ch->in_room || ch->in_room->sector_type == SECT_INSIDE
      || IS_SET (ch->in_room->room_flags, ROOM_INDOORS)
      || IS_SET (ch->act, PLR_HOLYWALK))
    return FALSE;
  warmth = get_warmth (ch);
  for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
      if (obj->wear_loc == -1)
	continue;
      if (obj->wear_loc == WEAR_BODY)
	shirt = TRUE;
      if (obj->wear_loc == WEAR_FEET)
	shoes = TRUE;
      if (obj->wear_loc == WEAR_LEGS)
	pants = TRUE;
    }
  if (too_cold (warmth, ch))
    {
      if (ch->position == POSITION_SLEEPING)
	{
	  send_to_char
	    ("You awaken, wracked by shivering spasms from the cold...\n\r",
	     ch);
	  NEW_POSITION (ch, POSITION_STANDING);
	  SUBHIT (ch, 4);
	}
      if (temp > 15)
	{
	  send_to_char
	    ("It's quite chilly out, considering your attire...\n\r", ch);
	  SUBHIT (ch, 1);
	  if (!shirt)
	    {
	      send_to_char
		("Your bare nipples are all prickly in the cold!\n\r", ch);
	      SUBHIT (ch, 1);
	    }
	  if (!shoes)
	    {
	      send_to_char ("Your feet are numb and cold...\n\r", ch);
	      SUBHIT (ch, 1);
	    }
	}
      else
	{
	  SUBHIT (ch, 2);
	  send_to_char ("You are FREEZING!! brrrrrr...\n\r", ch);
	  if (!pants)
	    {
	      if (SEX (ch) == SEX_MALE)
		send_to_char
		  ("Your manhood shrinks into boyhood as the cold grabs at your groin.\n\r",
		   ch);
	      else
		send_to_char
		  ("The freezing air hits your bare legs and buttocks, causing shivering spasms.\n\r",
		   ch);
	      SUBHIT (ch, 2);
	    }
	  if (!shirt)
	    {
	      send_to_char
		("Your nipples are all prickly in the freezing cold!\n\r",
		 ch);
	      SUBHIT (ch, 2);
	    }
	  if (!shoes)
	    {
	      send_to_char
		("Your feet are completely numb and sore from the intense cold!\n\r",
		 ch);
	      SUBHIT (ch, 2);
	    }
	}
    }
  if (too_warm (warmth, ch))
    {
      send_to_char
	("You are HOT... Sweat covers your body, and your vision becomes fuzzy...\n\r",
	 ch);
      SUBHIT (ch, 1);
      if (ch->move > 8)
	SUBMOVE (ch, number_range (3, 6));
    }
  if (ch->hit < 0)
    {
      send_to_char
	("Your vision blurs, and the world slips away in front of you...\n\r",
	 ch);
      raw_kill (ch, FALSE);
      return TRUE;
    }
  return FALSE;
}
