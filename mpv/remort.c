#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"
#define CTRL_R ""
char update_char[400];

void
do_afk (CHAR_DATA * ch, char *argy)
{
  DEFINE_COMMAND ("afk", do_afk, POSITION_DEAD, 0, LOG_NORMAL, "This command logs you as away from the keyboard.")
    send_to_char ("You have been flagged as AFK. Typing any input will turn this off.\n\r", ch);
    send_to_char ("If you are away for too long, your character *will* autosave and autoquit.\n\r", ch);
  ch->timer = 70;
  return;
}

void
upd_xp (CHAR_DATA * ch)
{
  if (IS_PLAYER (ch) && ch->desc != NULL && IS_SET (ch->pcdata->act2, PLR_BLANKPROMPT))
    {
      sprintf (update_char, "\x1B[%d;54fXp2L: \x1B[37;1m%ld\x1B[K\x1B[0m\x1B[%d;%df%s",
	       ch->pcdata->pagelen - 1, ((FIND_EXP (ch->pcdata->level, ch->pcdata->race)) - ch->exp),
	       ch->pcdata->pagelen - 3, 3 + strlen (ch->desc->inbuf), CTRL_R);
      write_to_descriptor2 (ch->desc, update_char, 0);
    }
  return;
}

void
set_it_up (CHAR_DATA * ch)
{
  if (IS_PLAYER (ch) && ch->desc != NULL && ch->pcdata->prompt == 2)
    {
      sprintf (update_char, "\x1B[2J\x1B[1;%dr\x1B[%d;1f\x1B[37;1;44m\x1B[2K",
	       ch->pcdata->pagelen - 2, ch->pcdata->pagelen);
      send_to_char (update_char, ch);
      nupd_hps (ch);
      nupd_mps (ch);
      nupd_mana (ch);
      nupd_tps (ch);
    }
  if (IS_PLAYER (ch) && ch->desc != NULL && ch->pcdata->prompt == 1)
    {
      sprintf (update_char, "\x1B[2J\x1B[2;%dr\x1B[1;1f\x1B[37;1;44m\x1B[2K",
	       ch->pcdata->pagelen);
      send_to_char (update_char, ch);
      nupd_hps (ch);
      nupd_mps (ch);
      nupd_mana (ch);
      nupd_tps (ch);
    }
  return;
}

#ifdef OLD_NOMOVE
void
scrn_upd (CHAR_DATA * ch)
{
  if (IS_PLAYER (ch) && ch->desc != NULL
      && IS_SET (ch->pcdata->act2, PLR_BLANKPROMPT))
    {
      sprintf (update_char, "\x1B[2J\x1B[1;%dr", ch->pcdata->pagelen - 3);
      write_to_descriptor2 (ch->desc, update_char, 0);
      sprintf (update_char,
	       "\x1B[%d;1f-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-",
	       ch->pcdata->pagelen - 2);
      write_to_descriptor2 (ch->desc, update_char, 0);
      upd_hps (ch);
      upd_mps (ch);
      upd_mana (ch);
      upd_xp (ch);
      upd_hunger (ch);
      upd_thirst (ch);
    }
  return;
}

void
upd_hunger (CHAR_DATA * ch)
{
  if (IS_PLAYER (ch) && ch->desc != NULL
      && IS_SET (ch->pcdata->act2, PLR_BLANKPROMPT))
    {
      if (ch->pcdata->condition[COND_FULL] < 7)
	{
	  sprintf (update_char,
		   "\x1B[37;1m\x1B[%d;60f[Hungry]\x1B[%d;%df\x1B[0m%s",
		   ch->pcdata->pagelen - 2, ch->pcdata->pagelen - 3,
		   3 + strlen (ch->desc->inbuf), CTRL_R);
	  write_to_descriptor2 (ch->desc, update_char, 0);
	}
      else
	{
	  sprintf (update_char, "\x1B[%d;60f=-=-=-=-=-\x1B[%d;%df%s",
		   ch->pcdata->pagelen - 2, ch->pcdata->pagelen - 3,
		   3 + strlen (ch->desc->inbuf), CTRL_R);
	  write_to_descriptor2 (ch->desc, update_char, 0);
	}
    }
  return;
}

void
upd_hungert (CHAR_DATA * ch)
{
  if (IS_PLAYER (ch) && ch->desc != NULL
      && IS_SET (ch->pcdata->act2, PLR_BLANKPROMPT))
    {
      if (ch->pcdata->condition[COND_FULL] < 7)
	{
	  sprintf (update_char,
		   "\x1B[37;1m\x1B[%d;60f[Hungry]\x1B[%d;%df\x1B[0m%s",
		   ch->pcdata->pagelen - 2, ch->pcdata->pagelen - 3,
		   3 + strlen (ch->desc->inbuf), CTRL_R);
	  send_to_char (update_char, ch);
	}
      else
	{
	  sprintf (update_char, "\x1B[%d;60f=-=-=-=-=-\x1B[%d;%df%s",
		   ch->pcdata->pagelen - 2, ch->pcdata->pagelen - 3,
		   3 + strlen (ch->desc->inbuf), CTRL_R);
	  send_to_char (update_char, ch);
	}
    }
  return;
}

void
upd_thirst (CHAR_DATA * ch)
{
  if (IS_PLAYER (ch) && ch->desc != NULL
      && IS_SET (ch->pcdata->act2, PLR_BLANKPROMPT))
    {
      if (ch->pcdata->condition[COND_THIRST] < 7)
	{
	  sprintf (update_char,
		   "\x1B[37;1m\x1B[%d;70f[Thirsty]\x1B[%d;%df\x1B[0m%s",
		   ch->pcdata->pagelen - 2, ch->pcdata->pagelen - 3,
		   3 + strlen (ch->desc->inbuf), CTRL_R);
	  write_to_descriptor2 (ch->desc, update_char, 0);
	}
      else
	{
	  sprintf (update_char, "\x1B[%d;70f=-=-=-=-=-\x1B[%d;%df%s",
		   ch->pcdata->pagelen - 2, ch->pcdata->pagelen - 3,
		   3 + strlen (ch->desc->inbuf), CTRL_R);
	  write_to_descriptor2 (ch->desc, update_char, 0);
	}
    }
  return;
}
#endif

void
nupd_tps (CHAR_DATA * ch)
{
  if (IS_PLAYER (ch) && ch->desc != NULL && ch->pcdata->prompt == 2)
    {
      sprintf (update_char,
	       "\x1B[%d;68f\x1B[37;1;44m Tps: %d   \x1B[%d;1f\x1B[37;0m",
	       ch->pcdata->pagelen, ch->pcdata->tps, ch->pcdata->pagelen - 2);
      send_to_char (update_char, ch);
    }
  if (IS_PLAYER (ch) && ch->desc != NULL && ch->pcdata->prompt == 1)
    {
      sprintf (update_char,
	       "\x1B[1;68f\x1B[37;1;44m Tps: %d   \x1B[%d;1f\x1B[37;0m",
	       ch->pcdata->tps, ch->pcdata->pagelen);
      send_to_char (update_char, ch);
    }
  return;
}

void
nupd_hps (CHAR_DATA * ch)
{
  if (IS_PLAYER (ch) && ch->desc != NULL && ch->pcdata->prompt == 2)
    {
      sprintf (update_char,
	       "\x1B[%d;1f\x1B[37;1;44m Hp: %s          \x1B[%d;1f\x1B[37;0m",
	       ch->pcdata->pagelen, STRING_HITS (ch),
	       ch->pcdata->pagelen - 2);
      send_to_char (update_char, ch);
    }
  if (IS_PLAYER (ch) && ch->desc != NULL && ch->pcdata->prompt == 1)
    {
      sprintf (update_char,
	       "\x1B[1;1f\x1B[37;1;44m Hp: %s          \x1B[%d;1f\x1B[37;0m",
	       STRING_HITS (ch), ch->pcdata->pagelen);
      send_to_char (update_char, ch);
    }
  return;
}

void
nupd_mps (CHAR_DATA * ch)
{
  if (IS_PLAYER (ch) && ch->desc != NULL && ch->pcdata->prompt == 2)
    {
      sprintf (update_char,
	       "\x1B[%d;20f\x1B[37;1;44mMp: %s          \x1B[%d;1f\x1B[37;0m",
	       ch->pcdata->pagelen, STRING_MOVES (ch),
	       ch->pcdata->pagelen - 2);
      send_to_char (update_char, ch);
    }
  if (IS_PLAYER (ch) && ch->desc != NULL && ch->pcdata->prompt == 1)
    {
      sprintf (update_char,
	       "\x1B[1;20f\x1B[37;1;44mMp: %s          \x1B[%d;1f\x1B[37;0m",
	       STRING_MOVES (ch), ch->pcdata->pagelen);
      send_to_char (update_char, ch);
    }
  return;
}

void
nupd_mana (CHAR_DATA * ch)
{
  if (IS_PLAYER (ch) && ch->desc != NULL && ch->pcdata->prompt == 2)
    {
      sprintf (update_char,
	       "\x1B[%d;44f\x1B[37;1;44mMana: %s            \x1B[%d;1f\x1B[37;0m",
	       ch->pcdata->pagelen, string_mana_p (mana_now (ch)),
	       ch->pcdata->pagelen - 2);
      send_to_char (update_char, ch);
    }
  if (IS_PLAYER (ch) && ch->desc != NULL && ch->pcdata->prompt == 1)
    {
      sprintf (update_char,
	       "\x1B[1;44f\x1B[37;1;44mMana: %s            \x1B[%d;1f\x1B[37;0m",
	       string_mana_p (mana_now (ch)), ch->pcdata->pagelen);
      send_to_char (update_char, ch);
    }
  return;
}



void
upd_hps (CHAR_DATA * ch)
{
  nupd_hps (ch);
  return;
  if (IS_PLAYER (ch) && ch->desc != NULL
      && IS_SET (ch->pcdata->act2, PLR_BLANKPROMPT))
    {
      sprintf (update_char,
	       "\x1B[%d;1fHp: \x1B[34;1m%d\x1B[32m/\x1B[34m%d \x1B[%d;%df\x1B[37;0m%s",
	       ch->pcdata->pagelen, ch->hit, ch->max_hit,
	       ch->pcdata->pagelen - 3, 3 + strlen (ch->desc->inbuf), CTRL_R);
      write_to_descriptor2 (ch->desc, update_char, 0);
    }
  return;
}

void
upd_mps (CHAR_DATA * ch)
{
  nupd_mps (ch);
  return;
  if (IS_PLAYER (ch) && ch->desc != NULL
      && IS_SET (ch->pcdata->act2, PLR_BLANKPROMPT))
    {
      sprintf (update_char,
	       "\x1B[%d;26fMp: \x1B[34;1m%d\x1B[32m/\x1B[34m%d \x1B[%d;%df\x1B[37;0m%s",
	       ch->pcdata->pagelen, ch->move, ch->max_move,
	       ch->pcdata->pagelen - 3, 3 + strlen (ch->desc->inbuf), CTRL_R);
      write_to_descriptor2 (ch->desc, update_char, 0);
    }
  return;
}

void
upd_mana (CHAR_DATA * ch)
{
  nupd_mana (ch);
  return;
  if (IS_PLAYER (ch) && ch->desc != NULL
      && IS_SET (ch->pcdata->act2, PLR_BLANKPROMPT))
    {
      if (max_mana (ch) > (ch->pcdata->n_max_mana + ch->pcdata->ma_mort))
	sprintf (update_char,
		 "\x1B[%d;54fMana: \x1B[34;1m%d\x1B[32m/\x1B[34m%d \x1B[%d;%df\x1B[37;0m%s",
		 ch->pcdata->pagelen, mana_now (ch), max_mana (ch),
		 ch->pcdata->pagelen - 3, 3 + strlen (ch->desc->inbuf),
		 CTRL_R);
      else
	sprintf (update_char,
		 "\x1B[%d;54fMana: \x1B[34;1m%d\x1B[32m/\x1B[34m%d \x1B[%d;%df\x1B[37;0m%s",
		 ch->pcdata->pagelen, ch->pcdata->n_mana,
		 (ch->pcdata->n_max_mana + ch->pcdata->ma_mort), ch->pcdata->pagelen - 3,
		 3 + strlen (ch->desc->inbuf), CTRL_R);
      write_to_descriptor2 (ch->desc, update_char, 0);
    }
  return;
}

void
blank_prompt (CHAR_DATA * ch)
{
  if (IS_MOB (ch))
    return;
  if (IS_SET (ch->pcdata->act2, PLR_ANSI)
      && IS_SET (ch->pcdata->act2, PLR_BLANKPROMPT))
    {
      sprintf (update_char, "\x1B[%d;%dr\x1B[J", ch->pcdata->pagelen - 3,
	       ch->pcdata->pagelen);
      write_to_descriptor2 (ch->desc, update_char, 0);
      sprintf (update_char, "\x1B[1;%dr\x1B[%d;1f", ch->pcdata->pagelen - 4,
	       ch->pcdata->pagelen - 4);
      write_to_descriptor2 (ch->desc, update_char, 0);
    }
  return;
}


void
do_limited (CHAR_DATA * ch, char *argy)
{
  int i;
  OBJ_PROTOTYPE *oid;
  int iHash;
  char arg1[128];
  DEFINE_COMMAND ("limited", do_limited, POSITION_DEAD, 110, LOG_NORMAL, "This command can be used to modify and/or view limited objects in the game.")
    hugebuf_o[0] = '\0';
  if (argy == "" || argy[0] == '\0')
    {
      send_to_char ("Limited items owned:\n\r", ch);
      for (i = 0; i < MAX_LIMITED; i++)
	{
	  if (limited.limited_array[i] > 0)
	    {
	      if ((oid = get_obj_index (i)) == NULL)
		{
		 limited.limited_array[i] = 0;
		}
	      else
		{
		 sprintf (hugebuf_o + strlen (hugebuf_o), "%5d [Owned: %2d] - %20s\x1B[37;0m\n\r", oid->vnum, limited.limited_array[i], oid->short_descr);
		}
	    }
	}
      sprintf (hugebuf_o + strlen (hugebuf_o), "\n\rLimited items:\n\r");
      for (iHash = 0; iHash < HASH_MAX; iHash++)
	{
	  for (oid = obj_index_hash[iHash]; oid != NULL; oid = oid->next)
	    {
	      if (oid->how_many > 0)
		{
		  sprintf (hugebuf_o + strlen (hugebuf_o),
			   "%5d [Max %2d] - %20s\x1B[37;0m\n\r", oid->vnum,
			   oid->how_many, oid->short_descr);
		  if (strlen (hugebuf_o) > 29500)
		    {
		      hugebuf_o[29500] = '\0';
		      goto tooln;
		    }
		}
	    }
	}
    tooln:
      sprintf (hugebuf_o + strlen (hugebuf_o), "\n\r");
      page_to_char (hugebuf_o, ch);
      return;
    }
  argy = one_argy (argy, arg1);
  if (is_number (arg1) && is_number (argy) && (i = atoi (arg1)) < MAX_LIMITED)
    {
      int amt;
      if (i < 0 || (amt = atoi (argy)) < 0)
	return;
      limited.limited_array[i] = amt;
      save_limited (i);
      send_to_char ("Set.\n\r", ch);
    }
  return;
}

void
write_playerbase_to_file (void)
{
  FILE *fff;
  char write_one_line[500];
  PLAYERBASE_DATA *playerbase;
  int tps, np, rp, average_level, level_tot;
  tps = 0;
  np = 0;
  rp = 0;
  average_level = 0;
  level_tot = 0;
  if ((fff = fopen ("player.txt", "w+")) == NULL)
    {
      fprintf (stderr, "The write failed! Bad news!! VERY bad news!!!!\n\r");
      return;
    }
  for (playerbase = playerbase_zero; playerbase != NULL;
       playerbase = playerbase->next)
    {
      if (playerbase->player_level != -1)
	{
	  tps++;
	  if (playerbase->player_level < 6)
	    np++;
	  if (playerbase->player_level > 190)
	    rp++;
	  level_tot +=
	    ((playerbase->player_level > 199) ? (playerbase->player_level - 200) : playerbase->player_level);
	  sprintf (write_one_line, "%-17s R: %s Lvl: %3d Last_on: %4ld hrs NoD: %s\n",
		   playerbase->player_name, (playerbase->player_level > 190 ? "y" : "n"),
		   ((playerbase->player_level > 199) ? (playerbase->player_level - 200) : playerbase->player_level),
		   ((current_time - playerbase->player_last_login) / 3600), (playerbase->player_nodelete ? "y" : "n"));
	  fprintf (fff, write_one_line);
	}
    }
  if (tps == 0)
    average_level = 0;
  else
    average_level = level_tot / tps;
  fprintf (fff, "Total Playerbase size: %d. Max Playerbase size: Unlimited.\n\r", tps);
  fprintf (fff, "Newbies: %d. Remorted players: %d. Average Lvl: %d.\n\r", np, rp, average_level);
  fclose (fff);
  return;
}

void
do_players (CHAR_DATA * ch, char *argy)
{
  int tps, np, rp, average_level, level_tot;
  PLAYERBASE_DATA *playerbase;
  DEFINE_COMMAND ("players", do_players, POSITION_DEAD, 0, LOG_NORMAL, "This command lists the playerbase size, and gives some general info.")
    tps = 0;
  average_level = 0;
  level_tot = 0;
  np = 0;
  rp = 0;
  if (!str_cmp (argy, "file") && ch->pcdata->level > 109)
    write_playerbase_to_file ();
  for (playerbase = playerbase_zero; playerbase != NULL;
       playerbase = playerbase->next)
    {
      if (playerbase->player_level != -1)
	{
	  if (playerbase->player_level > 99 && playerbase->player_level < 120)
	    continue;
	  tps++;
	  if (playerbase->player_level < 6)
	    np++;
	  if (playerbase->player_level > 190)
	    rp++;
	  level_tot +=
	    ((playerbase->player_level > 199) ? (playerbase->player_level - 200) : playerbase->player_level);
	}
    }
  if (tps == 0)
    average_level = 0;
  else
    average_level = level_tot / tps;
  sprintf (update_char, "Total Playerbase size: \x1B[1m%d\x1B[0m.  Max Playerbase size: \x1B[1mUnlimited\x1B[0m.\n\r", tps);
  send_to_char (update_char, ch);
  sprintf (update_char, "Newbies: \x1B[1m%d\x1B[0m.  Average Lvl: \x1B[1m%d\x1B[0m.\n\r", np, average_level);
  send_to_char (update_char, ch);
  return;
}

void
do_pdelete (CHAR_DATA * ch, char *argy)
{
  PLAYERBASE_DATA *playerbase;
  CHAR_DATA *ccc;
  DEFINE_COMMAND ("pdelete", do_pdelete, POSITION_DEAD, 110, LOG_ALWAYS, "This command is used to delete characters.")
    if (IS_MOB (ch))
    return;
  if (!IS_REAL_GOD (ch))
    return;
  if (!argy || argy[0] == '\0')
    {
      send_to_char ("Syntax is PDELETE <playername>.\n\r", ch);
      return;
    }
  ccc = get_char_world (ch, argy);
  if (ccc && IS_MOB (ccc))
    {
      send_to_char ("You can't pdelete mobs.\n\r", ch);
      return;
    }
  // Eraser 6 Feb 2003 below
  // if we're online and level 111+, don't delete
  if (ccc && LEVEL (ccc) >= 111)
    return;
  // if we're offline lookup level in database
  {
    bool found = FALSE;
    for (playerbase = playerbase_zero; playerbase && !found;
	 playerbase = playerbase->next)
      {
	if (!strcmp (argy, playerbase->player_name))
	  {
	    if ((playerbase->player_level > 200 ? playerbase->player_level - 200 : playerbase->player_level) >= 111)
	      return;
	    else
	      {
		found = TRUE;
		break;
	      }
	  }
      }
  }
  //if (!strcmp (NAME (ch), "Kilith") || !strcmp (NAME (ch), "Eraser"))
    //return;
  // end of addition
  if (ccc && IS_PLAYER (ccc))
    {
      do_delete (ccc, "character forever");
    }
  for (playerbase = playerbase_zero; playerbase != NULL;
       playerbase = playerbase->next)
    {
      if (!str_cmp (argy, playerbase->player_name) && !playerbase->player_nodelete)
	{
	  sprintf (update_char, "%s%s.cor", PLAYER_DIR, playerbase->player_name);
	  unlink (update_char);
	  sprintf (update_char, "%s%s.note", PLAYER_DIR, playerbase->player_name);
	  unlink (update_char);
	  sprintf (update_char, "%s%s.poll", PLAYER_DIR, playerbase->player_name);
	  unlink (update_char);
	  /*sprintf (update_char, "%s%s.bak", PLAYER_DIR, playerbase->player_name);
	  unlink (update_char);*/
	  sprintf (update_char, "%s%s", PLAYER_DIR, playerbase->player_name);
	  unlink (update_char);
	  playerbase->player_level = -1;
	  send_to_char ("Character deleted!\n\r", ch);
	  save_playerbase ();
	  return;
	}
      else
	if (!str_cmp (argy, playerbase->player_name)
	    && playerbase->player_nodelete)
	{
	  send_to_char ("Character has no-delete flag!\n\r", ch);
	  return;
	}
    }
  send_to_char ("Player not found.\n\r", ch);
  return;
}

void
do_nodelete (CHAR_DATA * ch, char *argy)
{
  PLAYERBASE_DATA *playerbase;
  DEFINE_COMMAND ("z_nodelete", do_nodelete, POSITION_DEAD, 110, LOG_ALWAYS, "This toggles a no delete flag on/off for a given player.")
    if (argy == "" || argy[0] == '\0')
    {
      char line[3][17];
      char outbuf[200];
      line[0][0] = '\0';
      line[1][0] = '\0';
      line[2][0] = '\0';
      send_to_char ("Syntax is NODELETE <charname>.\n\r", ch);
      send_to_char ("\n\rCurrent no_delete flags on:\n\r", ch);
      for (playerbase = playerbase_zero; playerbase != NULL;
	   playerbase = playerbase->next)
	{
	  if (playerbase->player_level == -1)
	    continue;
	  if (playerbase->player_nodelete)
	    {
	      if (line[0][0] == '\0')
		strcpy (line[0], playerbase->player_name);
	      else if (line[1][0] == '\0')
		strcpy (line[1], playerbase->player_name);
	      else if (line[2][0] == '\0')
		strcpy (line[2], playerbase->player_name);
	      else
		{
		  sprintf (outbuf, "%17s %17s %17s\n\r", line[0], line[1],
			   line[2]);
		  send_to_char (outbuf, ch);
		  strcpy (line[0], playerbase->player_name);
		  line[1][0] = '\0';
		  line[2][0] = '\0';
		}
	    }
	}
      sprintf (outbuf, "%17s %17s %17s\n\r",
	       (line[0][0] == '\0' ? " " : line[0]),
	       (line[1][0] == '\0' ? " " : line[1]),
	       (line[2][0] == '\0' ? " " : line[2]));
      send_to_char (outbuf, ch);
      return;
    }
  for (playerbase = playerbase_zero; playerbase != NULL;
       playerbase = playerbase->next)
    {
      if (!str_cmp (argy, playerbase->player_name))
	{
	  if (playerbase->player_nodelete)
	    {
	      send_to_char ("PLAYER NOW CAN BE DELETED AFTER INACTIVITY!\n\r", ch);
	      playerbase->player_nodelete = FALSE;
	      save_playerbase ();
	      return;
	    }
	  else
	    {
	      send_to_char ("Player is now safe from inactivity deletion.\n\r", ch);
	      playerbase->player_nodelete = TRUE;
	      save_playerbase ();
	      return;
	    }
	}
    }
  send_to_char ("I could not find that player in the database.\n\r", ch);
  return;
}

void
show_remort (CHAR_DATA * ch)
{
  char buf[STD_LENGTH];

  if (IS_MOB (ch))
    return;

  send_to_char ("You should read help Transform first, before you go any further.\n\r", ch);
  send_to_char ("\n\rThe correct syntax for transforming is:\n\r", ch);
  if (ch->pcdata->remort_times < 5)
    {
      send_to_char ("Transform <\x1B[37;1mstat\x1B[37;0m> <\x1B[37;1mstat\x1B[37;0m> <\x1B[37;1mstat\x1B[37;0m> <\x1B[37;1mstat\x1B[37;0m> <\x1B[37;1mstat\x1B[37;0m> <\x1B[34;1mdam\x1B[37;0m/\x1B[34;1mprac\x1B[37;0m/\x1B[34;1mhp\x1B[37;0m/\x1B[34;1mmove\x1B[37;0m/\x1B[34;1mmana\x1B[37;0m>\n\r", ch);
    }
  else
    {				// Kenor '03`
      send_to_char ("Transform <\x1B[34;1mdam\x1B[37;0m/\x1B[34;1mprac\x1B[34;1mhp\x1B[37;0m/\x1B[34;1mmove\x1B[37;0m/\x1B[34;1mmana\x1B[37;0m>\n\r", ch);
    }
  send_to_char ("<stat> is STR, DEX, CON, WIS, INT and last argy is DAM, PRAC, HP, MOVE or MANA.\n\r\n\r", ch);
  sprintf (buf, "DAM  = +%2d to attack power bonus.\n\r", REMORT_DAM_AMOUNT);
  send_to_char (buf, ch);
  sprintf (buf, "PRAC = +%2d practices to begin with.\n\r", REMORT_PRAC_AMOUNT);
  send_to_char (buf, ch);
  sprintf (buf, "HP   = +%2d hitpoints to begin with.\n\r", REMORT_HP_AMOUNT);
  send_to_char (buf, ch);
  sprintf (buf, "MOVE = +%2d movement points to begin with.\n\r", REMORT_MOVE_AMOUNT);
  send_to_char (buf, ch);
  sprintf (buf, "MANA = +%2d natural mana.\n\r", REMORT_MANA_AMOUNT);
  send_to_char (buf, ch);
  return;
}
