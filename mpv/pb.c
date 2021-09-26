#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"

bool
pfile_exists (const char *pname)
{
  char buff[500];
  FILE *fp;
  sprintf (buff, "%s%s", PLAYER_DIR_2, pname);
  if ((fp = fopen (buff, "r")) == NULL)
    return false;
  fclose (fp);
  return true;
}

void
load_playerbase (void)
{
  FILE *fp;
  PLAYERBASE_DATA *play;
  char buff[500];
  playerbase_zero = NULL;

  if ((fp = fopen ("pbase.dat", "r")) == NULL)
    {
      return;
    }
  while (1)
    {
      strcpy (buff, fread_word (fp));
      if (!str_cmp (buff, "END"))
	break;
      if (pfile_exists (buff))
	{
	  play = new_player ();
	  strcpy (play->player_name, buff);
	  strcpy (buff, fread_word (fp));
	  strcpy (play->email_address, buff);
	  play->player_last_login = fread_number (fp);
	  play->player_good = fread_number (fp);
	  play->player_level = fread_number (fp);
	  play->player_nodelete = fread_number (fp);
	}
    }
  fclose (fp);
  return;
}


void
reload_pbase (CHAR_DATA * ch, char *argy)
{
  PLAYERBASE_DATA *play;
  PLAYERBASE_DATA *pn;
  DEFINE_COMMAND ("z_reload_pbase", reload_pbase, POSITION_DEAD, 110,
		  LOG_ALWAYS, "This command reloads the pbase datafile.")
  for (play = playerbase_zero; play != NULL; play = pn)
    {
      pn = play->next;
      free (play);
    }
  load_playerbase ();
  return;
}


void
p_refresh (CHAR_DATA * ch, char *argy)
{
  char buf[200];
  DEFINE_COMMAND ("z_refresh", p_refresh, POSITION_DEAD, 110, LOG_ALWAYS,
		  "Resets all players' inactivity counters.")
  sprintf (buf, "util -refresh");
  system (buf);
  reload_pbase (ch, argy);
  return;
}



void
clean_up (CHAR_DATA * ch, char *argy)
{
  char buf[200];
  DEFINE_COMMAND ("z_clean_pfile", clean_up, POSITION_DEAD, 110, LOG_ALWAYS,
		  "Deletes pfiles of all people not in the pfile database!!  Dangerous.")
    sprintf (buf, "util -cleanup &");
  system (buf);
  return;
}



void
clean_oldchars (CHAR_DATA * ch, char *argy)
{
  char buf[200];
  DEFINE_COMMAND ("z_clean_oldchars", clean_oldchars, POSITION_DEAD, 110,
		  LOG_ALWAYS,
		  "Cleans up people who haven't logged in for quite a while.  Lags the mud.")
    sprintf (buf, "util -deleteold");
  system (buf);
  reload_pbase (ch, argy);
  return;
}

void
update_pbase (CHAR_DATA * ch)
{
  bool ffund;
  PLAYERBASE_DATA *player;
  ffund = FALSE;
  for (player = playerbase_zero; player != NULL; player = player->next)
    {
      if (player->player_level != -1
	  && !str_cmp (player->player_name, NAME (ch)))
	{
	  if (ch->pcdata->level !=
	      ((player->player_level >
		200) ? (player->player_level - 200) : player->player_level))
	    {
	      player->player_last_login = (int) ch->pcdata->logon;
	      if (player->email_address[0] == '\0')
		strcpy (player->email_address, ch->pcdata->email);
	      player->player_good = !(IS_EVIL (ch));
	      player->player_level =
		((ch->pcdata->remort_times >
		  0) ? (ch->pcdata->level + 200) : ch->pcdata->level);
	      save_playerbase ();
	      return;
	    }
	  ffund = TRUE;
	  break;
	}
    }
  if (ffund == FALSE)
    {
      bool isfree;
      isfree = FALSE;
      player = new_player ();
      strcpy (player->player_name, NAME (ch));
      player->player_last_login = (int) ch->pcdata->logon;
      player->player_good = !(IS_EVIL (ch));
      strcpy (player->email_address, ch->pcdata->email);
      player->player_level = ch->pcdata->level;
      player->player_nodelete = FALSE;
      isfree = TRUE;
      save_playerbase ();
    }
  return;
}

void
fcheck_pbase (char *n)
{
  bool ffund;
  PLAYERBASE_DATA *player;
  ffund = FALSE;
  for (player = playerbase_zero; player != NULL; player = player->next)
    {
      if (!str_cmp (player->player_name, n))
	return;
    }
  player = new_player ();
  strcpy (player->player_name, n);
  player->player_last_login = (int) current_time;
  player->player_good = 1;
  strcpy (player->email_address, "not_recorded_yet");
  player->player_level = 5;
  player->player_nodelete = FALSE;
  save_playerbase ();
  return;
}
