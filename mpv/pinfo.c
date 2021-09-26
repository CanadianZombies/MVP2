#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"

bool
locate_person (CHAR_DATA * ch, int level, char *argy)
{
  char buf[STD_LENGTH];
  char arg[SML_LENGTH];
  bool found;
  CHAR_DATA *victim;
  one_argy (argy, arg);
  if ((LEVEL (ch) > 100) && (LEVEL (ch) < 111))
    return FALSE;
  found = FALSE;
  for (victim = char_list; victim != NULL; victim = victim->next)
    {
      if (victim->in_room != NULL
	  && !IS_AFFECTED (victim, AFF_HIDE)
	  && !IS_SET (victim->in_room->room_flags, ROOM_UNTRACKABLE)
	  && can_see (ch, victim)
	  && is_name (arg, NAME (victim)) && level + 18 >= LEVEL (victim))
	{
	  found = TRUE;
	  sprintf (buf, "\x1B[0;31m%s is sitting in \x1B[1;30m%s.\x1B[37;0m\n\r", PERS (victim, ch), show_room_name (victim, victim->in_room->name));
	  send_to_char (buf, ch);
	  if (number_range (1, 125) < LEVEL (victim))
	    /*{
	      if (number_range (1, 3) == 1)
		{
		  sprintf (buf, "You feel as if %s might have pin-pointed your location!\n\r", NAME (ch));
		  send_to_char (buf, victim);
		}
	      else*/
		{
		  sprintf (buf, "\x1B[0;31mThrough a \x1B[1;30mvoid \x1B[0;31min the fabric of time you feel someone watching you!\x1B[37;0m\n\r");
                    send_to_char (buf, victim);
		}
//	    }
	  break;
	}
    }

  return found;
}

bool
scry (CHAR_DATA * ch, int level, char *argy)
{
  char arg[SML_LENGTH];
  char buf[STD_LENGTH];
  bool found;
  CHAR_DATA *victim;
  one_argy (argy, arg);
  if ((LEVEL (ch) > 100) && (LEVEL (ch) < 111))
    return FALSE;
  found = FALSE;
  for (victim = char_list; victim != NULL; victim = victim->next)
    {
      if (victim->in_room != NULL
	  && !IS_AFFECTED (victim, AFF_HIDE)
	  && !IS_SET (victim->in_room->room_flags, ROOM_UNTRACKABLE)
	  && can_see (ch, victim)
	  && is_name (arg, NAME (victim)) && level + 17 >= LEVEL (victim))
	{
	  found = TRUE;
	  sprintf (buf, "%s look", NAME (victim));
	  do_at (ch, buf);
	  if (number_range (1, 125) < LEVEL (victim))
	    {
	      if (number_range (1, 3) == 1)
		{
		  sprintf (buf, "You feel as if %s is watching you!!\n\r",
			   NAME (ch));
		  send_to_char (buf, victim);
		}
	      else
		{
		  sprintf (buf,
			   "You feel as if someone is looking into the room you are in...\n\r");
		  send_to_char (buf, victim);
		}
	    }
	  break;
	}
    }

  return found;
}

void
do_finger (CHAR_DATA * ch, char *argy)
{
  char buf[STD_LENGTH];
  char arg[SML_LENGTH];
  CHAR_DATA *victim;
  PLAYERBASE_DATA *playerbase;
  DEFINE_COMMAND ("finger", do_finger, POSITION_DEAD, 0, LOG_NORMAL, "This command can be used to lookup information on the specified player.")
    one_argy (argy, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Finger whom?\n\r", ch);
      return;
    }
  if ((!str_cmp ("Sabelis", argy) || !str_cmp ("Kilith", argy)) && (LEVEL (ch) < 111))
    {
      send_to_char ("Player not found or not visible.\n\r", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      bool ffund;
      ffund = FALSE;
      for (playerbase = playerbase_zero; playerbase != NULL;
	   playerbase = playerbase->next)
	{

	  if (!str_cmp (argy, playerbase->player_name)
	      && (LEVEL (ch) > 100 || (playerbase->player_good && !IS_EVIL (ch)) || (!playerbase->player_good && IS_EVIL (ch))))
	    {
	      char bufr[500];
	      ffund = TRUE;
sprintf (bufr, "\x1B[37;0m[\x1B[1m%s\x1B[0m] Logged on last about \x1B[1m%ld\x1B[0m hours ago.\n\r", playerbase->player_name, (((current_time) - playerbase->player_last_login) / 3600));
	      send_to_char (bufr, ch);
	      if (LEVEL (ch) > 107)
		{
sprintf (bufr, "GOD INFO ONLY: Fingered character is level %d and is remort %d\n\r", playerbase->player_level, playerbase->player_remort_times);
         send_to_char (bufr, ch);
//sprintf(bufr, "Player was created %d days, %d hours ago.\n\r", (current_time - playerbase->player_created)/(3600*24), ((current_time - 
//playerbase->player_created)/(3600)) % 24);
//                  send_to_char(bufr, ch);
		}
	      break;
	    }
	}
      if (!ffund)
	{
	  send_to_char ("Player not found or not visible.\n\r", ch);
	}
      return;
    }

  if (LEVEL (ch) < 100 && ((IS_EVIL (ch) && !IS_EVIL (victim)) || (IS_EVIL (victim) && !IS_EVIL (ch)) || !can_see (ch, victim) || IS_MOB (victim)))
    {
      send_to_char ("Player not found or not visible.\n\r", ch);
      return;
    }

  sprintf (buf, "\x1B[34;1m%s\x1B[37;0m the %s %s is currently playing.\n\r",
	   NAME (victim),
	   (SEX (victim) == SEX_MALE ? "male" : SEX (victim) ==
	    SEX_FEMALE ? "female" : "sexless"),
	   race_info[victim->pcdata->race].name);
  send_to_char (buf, ch);
  if (DSC (victim) && DSC (victim)[0] != '\0')
    {
      sprintf (hugebuf_o, "Description:\n\r%s", DSC (victim));
      send_to_char (hugebuf_o, ch);
    }
  if (LEVEL (ch) > 107 && victim->pcdata->email[0] != '\0')
    {
      sprintf (buf, "God Info: Email: %s.\n\r", victim->pcdata->email);
      send_to_char (buf, ch);
    }

  return;
}

void
do_credits (CHAR_DATA * ch, char *argy)
{
  DEFINE_COMMAND ("credits", do_credits, POSITION_DEAD, 0, LOG_NORMAL, "Required as per Diku Copyright") 
  do_help (ch, "credits");
  return;
}
