#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"

bool
not_within_levels (CHAR_DATA * ch, CHAR_DATA * victim)
{
  return FALSE;
/*  int range_1 = 0;
  int range_2 = 0;
  if (IS_MOB (ch) || IS_MOB (victim))
    return FALSE;
  if (LEVEL (ch) <= 10 || LEVEL (victim) <= 10)
    return TRUE;
  if (LEVEL (ch) < 20)
    range_1 = 9;
  if (LEVEL (victim) < 20)
    range_2 = 9;
  if (LEVEL (ch) >= 20)
    range_1 = 5 + ((LEVEL (ch) - 6) / 2);
  if (LEVEL (victim) >= 20)
    range_2 = 5 + ((LEVEL (victim) - 6) / 2);
  if (LEVEL (ch) + range_1 < LEVEL (victim) || LEVEL (ch) - range_1 > LEVEL (victim))
    return TRUE;
  if (LEVEL (victim) + range_2 < LEVEL (ch) || LEVEL (victim) - range_2 > LEVEL (ch))
    return TRUE;
  return FALSE;
  */
}

char *
rNAME (CHAR_DATA * to, CHAR_DATA * ch)
{
  char tmprace[150];
  static char ret[500];
  int tr;
  bool a_an = FALSE;
  ret[0] = '\0';
  tmprace[0] = '\0';
  if (IS_PLAYER (to))
    tr = to->pcdata->race;
  else
    tr = 0;
  if (((IS_EVIL (to) && !IS_EVIL (ch)) || (!IS_EVIL (to) && IS_EVIL (ch))) &&
      IS_PLAYER (ch) && IS_PLAYER (to) && (can_see_nextdoor (ch, to) || can_see (ch, to))
      && (not_is_same_align (ch, to)))
    {
#ifdef NEW_WORLD
/* Change the not within level dealie here */
      if (!IN_BATTLE (to) && !IN_BATTLE (ch)
	  && (NO_PKILL (to) || NO_PKILL (ch) || not_within_levels (to, ch)))
	{
	  strcpy (ret, "~~~");
	  return ret;
	}
#endif
      if (UPPER (race_info[tr].name[0]) == 'A' ||
	  UPPER (race_info[tr].name[0]) == 'E' ||
	  UPPER (race_info[tr].name[0]) == 'I' ||
	  UPPER (race_info[tr].name[0]) == 'O' ||
	  UPPER (race_info[tr].name[0]) == 'U')
	a_an = TRUE;
      if (a_an)
	strcpy (tmprace, "+* An ");
      else
	strcpy (tmprace, "+* A ");
      strcat (tmprace, race_info[tr].name);
      strcat (tmprace, " *+");
      strcpy (ret, tmprace);
    }
  else
    strcpy (ret, PERS (to, ch));
  return ret;
}

void
replace (CHAR_DATA * ch, char *argy)
{
  DEFINE_COMMAND ("replace", replace, POSITION_DEAD, 111, LOG_ALWAYS,
		  "This command allows you to replace 1 learn for 10 practices or 10 practices for 1 learn.")
    if (!argy || argy[0] == '\0'
	|| (str_cmp (argy, "learns") && str_cmp (argy, "practices")))
    {
      send_to_char
	("Format is 'replace <learns>' to convert 1 learn to 10 practices.\n\r",
	 ch);
      send_to_char
	("Format is 'replace <practices>' to convert 10 practices to 1 learn.\n\r",
	 ch);
      return;
    }
  if (!str_cmp (argy, "learns"))
    {
      if (ch->pcdata->learn < 1)
	{
	  send_to_char ("You don't have any learns to convert.\n\r", ch);
	  return;
	}
      ch->pcdata->learn--;
      ch->pcdata->practice += 10;
      send_to_char ("Done.\n\r", ch);
      return;
    }
  if (!str_cmp (argy, "practices"))
    {
      if (ch->pcdata->practice < 10)
	{
	  send_to_char ("You don't have 10 practices to convert.\n\r", ch);
	  return;
	}
      ch->pcdata->learn++;
      ch->pcdata->practice -= 10;
      send_to_char ("Done.\n\r", ch);
      return;
    }
  return;
}
