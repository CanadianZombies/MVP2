#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"

void
do_r (CHAR_DATA * ch, char *argy)
{
 DEFINE_COMMAND ("r", do_r, POSITION_DEAD, 1, LOG_NORMAL, "To prevent the use of clear or reset screen by accidently hitting r.")
 send_to_char("huh?",ch);
      return;
}
 
void
do_anon (CHAR_DATA * ch, char *argy)
{
  DEFINE_COMMAND ("anonymous", do_anon, POSITION_DEAD, 0, LOG_NORMAL, "This command toggles whether or not your level shows up on the who list.")
    if (ch->pcdata->show_lev)
    {
      send_to_char
	("Your level will not be shown in the who list from now on.\n\r", ch);
      ch->pcdata->show_lev = FALSE;
      return;
    }
  send_to_char
    ("Your level will now be shown in the who list for others to see.\n\r", ch);
  ch->pcdata->show_lev = TRUE;
  return;
}

void
do_levels (CHAR_DATA * ch, char *argy)
{
  int i, a, b;
  char buffr[256];
  char arg1[256];
  DEFINE_COMMAND ("levels", do_levels, POSITION_DEAD, 110, LOG_NORMAL, "This command shows exp and tps needed for the level range specified.")
    buffr[0] = '\0';
  argy = one_argy (argy, arg1);
  if (!is_number (argy) || !is_number (arg1))
    {
      send_to_char ("Syntax is \"level <min> <max>\"\n\r", ch);
      return;
    }
  a = atoi (arg1);
  b = atoi (argy);
  if (a >= b || a < 1 || a > 90 || b < 2 || b > 90)
    {
      send_to_char ("Invalid range.\n\r", ch);
      return;
    }
  for (i = a; i < b; i++)
    {
      sprintf (buffr, "Level %d Exp: %ld Tps: %d\n\r", i + 1, FIND_EXP (i, ch->race), FIND_TPS (i, ch->race));
      send_to_char (buffr, ch);
    }
  return;
}

void
do_reload_help (CHAR_DATA * ch, char *argy)
{
  FILE *fp;
  char word[500];
  HELP_DATA *hnx;
  HELP_DATA *hc;
  DEFINE_COMMAND ("rld_help", do_reload_help, POSITION_DEAD, 110, LOG_ALWAYS, "This command reloads the helpfiles from disk.") 
     if ((fp = fopen ("help.are", "r")) == NULL)
    {
      send_to_char ("Unable to open the helpfile...", ch);
      return;
    }
  for (hc = help_first; hc != NULL; hc = hnx)
    {
      hnx = hc->next;
      free_string (hc->keyword);
      free_string (hc->text);
    }
  help_last = NULL;
  help_first = NULL;
  top_help = 0;
  word[0] = '\0';
  while (str_cmp ("#HELPS", word))
    strcpy (word, fread_word (fp));
  load_helps (fp);
  send_to_char ("Help files were successfully reloaded.\n\r", ch);
  fclose (fp);
  return;
}

void
do_lowlevel_help (CHAR_DATA * ch, char *argy)
{
  char argall[SML_LENGTH];
  char argone[SML_LENGTH];
  char j[50];
  HELP_DATA *pHelp;
  // No define command here, mortal access not needed.

  if (argy[0] == '\0')		// Shouldn't use this for this, but for compatability
    argy = "summary";

  /* Not including wasteful JAVA stuff. */

  argall[0] = '\0';
  while (argy[0] != '\0')
    {
      argy = one_argy (argy, argone);
      if (argall[0] != '\0')
	strcat (argall, " ");

      strcat (argall, argone);
    }
  for (pHelp = help_first; pHelp && pHelp != NULL; pHelp = pHelp->next)
    {
      if (pHelp->level > LEVEL (ch))
	continue;
      if (is_name (argall, pHelp->keyword))
	{
	  if (pHelp->level >= 0)	// Removed imotd stuff: unused
	    send_to_char ("\n\r", ch);
	  if (pHelp->text[0] == '.')
	    write_to_descriptor2 (ch->desc, decompress (pHelp->text + 1), 0);
	  else
	    write_to_descriptor2 (ch->desc, decompress (pHelp->text), 0);
	  return;
	}
    }
  // Low level function does not return 'no help' message.
  return;
}

void
do_help (CHAR_DATA * ch, char *argy)
{
  char argall[SML_LENGTH];
  char argone[SML_LENGTH];
  char j[50];
  HELP_DATA *pHelp;
  DEFINE_COMMAND ("help", do_help, POSITION_DEAD, 0, LOG_NORMAL, "This is the basic help command.  Syntax is 'help' or 'help <topic>'.")
    if (argy[0] == '\0')
    argy = "summary";

  if (IS_JAVA (ch))
    {
      if (!str_cmp (argy, "motd"))
	{
	  sprintf (j, "%c", 31);
	  write_to_buffer (ch->desc, j, 0);
	}
      else
	{
	  sprintf (j, "%c", 28);
	  write_to_buffer (ch->desc, j, 0);

	}
    }


  //Tricky argy handling so 'help a b' doesn't match a.
 
  argall[0] = '\0';
  while (argy[0] != '\0')
    {
      argy = one_argy (argy, argone);
      if (argall[0] != '\0')
	strcat (argall, " ");
      strcat (argall, argone);
    }
  for (pHelp = help_first; pHelp && pHelp != NULL; pHelp = pHelp->next)
    {
      if (pHelp->level > LEVEL (ch))
	continue;
      if (is_name (argall, pHelp->keyword))
	{
	  if (pHelp->level >= 0 && str_cmp (argall, "imotd"))
	    {
	      send_to_char ("\n\r", ch);
	    }
	  /*
	     * Strip leading '.' to allow initial blanks.
	   */

/*#ifdef NEW_WORLD*/
	  if (pHelp->text[0] == '.')
	    {
	      if (IS_JAVA (ch))
		send_to_char (decompress (pHelp->text + 1), ch);
	      else
		page_to_char (decompress (pHelp->text + 1), ch);
	    }
	  else
	    {
	      if (IS_JAVA (ch))
		send_to_char (decompress (pHelp->text), ch);
	      else
		page_to_char (decompress (pHelp->text), ch);
	    }
/*#else
	  if (pHelp->text[0] == '.')
	    page_to_char (pHelp->text + 1, ch);
	  else
	    page_to_char (pHelp->text, ch);

#endif*/
	  if (IS_JAVA (ch))
	    {
	      write_to_buffer (ch->desc, j, 0);
	    }

	  return;
	}
    }
  send_to_char ("No help on that word.\n\r", ch);
  return;
}

void
do_raceinfo (CHAR_DATA * ch, char *argy)
{
 DEFINE_COMMAND ("race info", do_raceinfo, POSITION_DEAD, 1, LOG_NORMAL, "this command brings up out todo list in a command insted of a help file.")
    do_help (ch, "races");
} // Kilith 2006

void
do_remortcost (CHAR_DATA * ch, char *argy)
{
 DEFINE_COMMAND ("remort cost", do_remortcost, POSITION_DEAD, 1, LOG_NORMAL, "this command brings up out todo list in a command insted of a help file.")
    do_help (ch, "remort cost");
} // Kilith 2006

void
do_change (CHAR_DATA * ch, char *argy)
{
 DEFINE_COMMAND ("change", do_change, POSITION_DEAD, 1, LOG_NORMAL, "this command brings up out todo list in a command insted of a help file.")
    do_help (ch, "changes");
} // Kilith 2003
void
do_todo (CHAR_DATA * ch, char *argy)
{
 DEFINE_COMMAND ("todo", do_todo, POSITION_DEAD, 110, LOG_NORMAL, "this command brings up out todo list in a command insted of a help file.")
    do_help (ch, "todo");
} // Kilith 2003
void
do_wizlist (CHAR_DATA * ch, char *argy)
{
  DEFINE_COMMAND ("wizlist", do_wizlist, POSITION_DEAD, 0, LOG_NORMAL, "This command shows the wizlist help file.")
    do_help (ch, "wizlist");
}  // Kilith 2002

void
do_mudmap (CHAR_DATA * ch, char *argy)
{
  DEFINE_COMMAND ("mudmap", do_mudmap, POSITION_DEAD, 0, LOG_NORMAL, "This command shows the wizlist help file.")
    do_help (ch, "mudmap");
}  // Kilith 2005


char *
pos_name (int pos)
{
  static char r[STD_LENGTH];
  memset (r, 0, STD_LENGTH);
  switch (pos)
    {
    case POSITION_SLEEPING:
      strcpy (r, "Sleeping");
      break;
    case POSITION_RESTING:
      strcpy (r, "Resting");
      break;
    case POSITION_MEDITATING:
      strcpy (r, "Meditating");
      break;
    case POSITION_BASHED:
    case POSITION_FIGHTING:
    case POSITION_CIRCLE:
    case POSITION_BACKSTAB:
    case POSITION_GROUNDFIGHTING:
      strcpy (r, "Fighting");
      break;
    default:
      strcpy (r, "---");
      break;
    }
  return r;
}


void
do_who (CHAR_DATA * ch, char *argy)
{
  CHAR_DATA *wch;
  int pass;
  DESCRIPTOR_DATA *d;
  int vis;
  int ii;
  int nMatch;
  bool tempbool;
  int gMatch;
  int align;
  int levl;
  bool namey;
  char doe[STD_LENGTH];
  bool clanny;
  bool tally_all = FALSE;
  DEFINE_COMMAND ("who", do_who, POSITION_DEAD, 0, LOG_NORMAL, "This command shows who is playing and visible to you at the moment.")
    ii = clan_number (ch);
  align = 0;
  nMatch = 0;
  clanny = FALSE;
  gMatch = 0;
  vis = 0;
  hugebuf_o[0] = '\0';
  sprintf 
(hugebuf_o,"\n\r\x1b[1;30m[\x1b[1;31m+\x1b[1;30m]\x1b[1;34m-=-=-=-=-=-=-=-=-=-\x1b[1;30m[\x1b[1;31m+\x1b[1;30m] \x1b[1;37mVoid of Reality Players \x1b[1;30m[\x1b[1;31m+\x1b[1;30m]\x1b[1;34m-=-=-=-=-=-=-=-=-=-\x1b[1;30m[\x1b[1;31m+\x1b[1;30m]\x1B[37;0m\n\r");
  namey = FALSE;
  levl = 0;
  if (argy[0] == '\0' || argy == "")
    {
      if (LEVEL (ch) > 100 && LEVEL (ch) < 101)
	align = 3;
      else
	namey = FALSE;
      tally_all = TRUE;
    }
  else if (!str_cmp (argy, "old") && LEVEL (ch) >= 110)
    {
      namey = FALSE;
      tally_all = TRUE;
    }
  else
    {
      if (!str_cmp (argy, "clan"))
	{
	  clanny = TRUE;
	  if (ii < 1)
	    {
	      send_to_char ("You aren't in a clan!\n\r", ch);
	      return;
	    }
	}
      else if (!str_cmp (argy, "battle") || !str_cmp (argy, "battleground"))
	align = 4;
      else if (!str_cmp (argy, "evil"))
	align = 1;
      else if (!str_cmp (argy, "good"))
	align = 2;
      else if (!str_cmp (argy, "god") || !str_cmp (argy, "immortal"))
	align = 3;
      else if (LEVEL (ch) > 100 && is_number (argy))
	{
	  levl = atoi (argy);
	}
      else
	{
	  argy[0] = UPPER (argy[0]);
	  namey = TRUE;
	}
    }
  tempbool = FALSE;
  for (pass = 0; pass != 3; pass = pass + 1)
    {
      if (!tempbool && pass != 0 && (gMatch > 0 || LEVEL (ch) >= 1))
	{
	  tempbool = TRUE;
sprintf (hugebuf_o + strlen (hugebuf_o), "\x1B[0;36m  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\x1B[37;0m\n\r");
	}

      for (d = descriptor_list; d != NULL; d = d->next)
	{
	  wch = NULL;
	  if (d->character == NULL)
	    continue;
	  if (IS_MOB (d->character))
	    continue;
	  if (d->character->desc == NULL)
	    continue;
	  if ((d->connected != CON_PLAYING
	       && d->connected != CON_AEDITOR
	       && d->connected != CON_REDITOR
	       && d->connected != CON_CEDITOR
	       && d->connected != CON_OEDITOR
	       && d->connected != CON_HEDITOR
	       && d->connected != CON_MEDITOR) || !can_see (ch, d->character))
	    continue;
	  if (d->connected != CON_PLAYING && !IS_IMMORTAL (ch))
	    continue;
	  wch = (d->original != NULL) ? d->original : d->character;
	  if (clanny && clan_number (wch) != ii)
	    continue;
	  if (LEVEL (ch) > 100 && LEVEL (wch) < levl)
	    continue;
	  if (namey && str_prefix (capitalize (argy), NAME (wch)))
	    continue;
	  if (LEVEL (wch) > 100 && pass != 0)
	    continue;
	  if (LEVEL (wch) <= 100 && pass == 0)
	    continue;
	  if (LEVEL (wch) <= 100 && !IS_EVIL (wch) && pass != 2)
	    continue;
	  if (LEVEL (wch) <= 100 && pass == 2 && IS_EVIL (wch))
	    continue;
	  if (align == 1 && !IS_EVIL (wch))
	    continue;
	  if (align == 2 && IS_EVIL (wch))
	    continue;
	  if (align == 3 && LEVEL (wch) < 100)
	    continue;
	  if (LEVEL (ch) < 100 && !IS_EVIL (ch) && IS_EVIL (wch) && LEVEL (wch) < 110)
	    continue;
	  if (align == 4 && !IN_BATTLE (wch))
	    continue;
	  if (IS_EVIL (ch) && LEVEL (ch) < 100 && !IS_EVIL (wch) && LEVEL (wch) < 110)
	    continue;
	  if (LEVEL (wch) < 105 && LEVEL (wch) > 99 && LEVEL (ch) < 100)
	    continue;

	  if (tally_all)
	    nMatch++;
	  doe[0] = '\0';


	  if (LEVEL (wch) > 99)
	    {
	      /* }
	         nMatch++;
	         doe[0] = '\0';
	         if (LEVEL (wch) > IMM_LEVEL) {
	         if (wch->pcdata->wizinvis == 0)
	         gMatch++;
	         if (d->connected != CON_PLAYING) {
	         if (d->connected == CON_SEDITOR)
	         sprintf (doe, "\x1B[1;36m Spell Editing ");
	         else if (d->connected == CON_HEDITOR)
	         sprintf (doe, "\x1B[1;36mHelp Editing");
	         else */
	      if (wch->pcdata->wizinvis == 0)
		gMatch++;
	      if (d->connected != CON_PLAYING)
		sprintf (doe, "\x1B[1;36m     Building    ");
	      else
		{
		    if (!wch->pcdata->rank || !str_cmp(strip_ansi_codes(wch->pcdata->rank), "(null)"))
			{
		      switch (LEVEL (wch))
			{
			default:
			sprintf (doe, "\x1B[1;36m    Immortal     ");
			  break;
			case 105:
			  if (wch->pcdata->sex == SEX_MALE)
			    {
			sprintf (doe, "\x1B[1;31m   *Demi God*    ");
			      break;
			    }
			sprintf (doe, "\x1B[1;31m *Demi Goddess*  ");
			  break;
			case 106:
			  if (wch->pcdata->sex == SEX_MALE)
			    {
			sprintf (doe, "\x1B[0;36m  *Lesser God*   ");
			    break;
			    }
			sprintf (doe, "\x1B[0;36m *Lsr. Goddess*  ");
			  break;
			case 109:
			  if (wch->pcdata->sex == SEX_MALE)
			    {
			sprintf (doe, "\x1B[1;30m     * God *     ");
			      break;
			    }
			sprintf (doe, "\x1B[1;30m   * Goddess *   ");
			  break;
			case 110:
			  if (wch->pcdata->sex == SEX_MALE)
			    {
			sprintf (doe, "\x1B[1;34m-* Greater God *-");
			      break;
			    }
			sprintf (doe, "\x1B[1;34m-* Gr. Goddess *-");
			  break;
			case TOP_LEVEL:
			sprintf (doe, "\x1B[1;35m  -*  Admin  *-  ");
			  break;
			}
		    }
		  else
		    sprintf (doe, wch->pcdata->rank);
		}
	      if (wch->pcdata->quiet == 1)
		sprintf (doe, "\x1B[1;35m  - Quiet Mode - ");
	      if (wch->pcdata->quiet == 2)
		sprintf (doe, "\x1B[1;34m  - Busy Mode -  ");
	    }
	  {
	    if (!tally_all)
	      nMatch++;
	    if (IS_PLAYER (ch) && LEVEL (ch) < 100 && !IS_MOB (wch)
		&& (!not_is_same_align (ch, wch)))
	      vis++;
	    if (ii > 0)
	      {
		if (clan_number (wch) == ii)
		  {
		    sprintf (hugebuf_o + strlen (hugebuf_o), "<C>");
		  }
		else
		  sprintf (hugebuf_o + strlen (hugebuf_o), "   ");
	      }
	    if (LEVEL (ch) >= 110 && LEVEL (wch) <= 100
		&& IS_SET (ch->act, PLR_HOLYLIGHT))
	      {
		/*
		   sprintf (hugebuf_o + strlen (hugebuf_o),
		   "\x1B[1;30m[\x1B[32m%s%s %s%3d%s%s\x1B[1;30m]\x1B[37;0m %s%s%s%s%s%s\x1B[37;0m\n\r",
		   (wch->pcdata->bounty ? "B" : " "), (IS_EVIL (wch) ? "\x1B[31;1mA\x1B[32m" : " "), (IN_BATTLE (wch) ? "+" : " "), LEVEL (wch),
		   (IS_SET (wch->pcdata->act2, PLR_SILENCE) ? "s" : " "),
		   (wch->pcdata->bgf ? "-" : " "),
		   wch->timer > 49 ? "\x1B[37;1m<AFK> \x1B[37;0m" : "", NAME (wch),
		   ((wch->pcdata->title[0] != ',' && wch->pcdata->title[0] != '.') ? " " : ""),
		   wch->pcdata->title,
		   (IS_AFFECTED (wch, AFF_INVISIBLE)) ? " (Invis)" : "",
		   (wch->pcdata->wizinvis != 0 ? " (Invis)" : ""));
		 */

		if (LEVEL (ch) >= 110 && str_cmp (argy, "old"))
		  {
        //char smallbuf[SML_LENGTH];
        //sprintf(smallbuf, pos_name(wch->position));
        //if (FIGHTING(ch) && IS_EVIL(ch) != IS_EVIL(FIGHTING(ch)))
        //  sprintf(smallbuf, "PK")

//Immortal Who list below!

sprintf (hugebuf_o + strlen (hugebuf_o),
//     evil or good?     lvl         remorts        Guild      Name                 -,+,A,N,I,S,F,P                 ROOM NUMBER               POSITION    
"\x1B[1;30m[%s \x1B[1;37m%2d \x1B[0;36mR%d\x1B[1;30m] %s \x1B[1;37m%-15s \x1B[1;30m[%s%s%s%s%s%s%s%s\x1B[1;30m] [\x1B[0;37m%5d\x1B[30;1m] [\x1B[0;37m%s\x1B[30;1m]\x1B[37;0m\n\r",
 (IS_EVIL (wch) ? "\x1B[31;1mD\x1B[32m" : "\x1B[1;34mW\x1B[32m"),
 LEVEL (wch),
 wch->pcdata->remort_times,
 get_main_guild(wch),
 NAME (wch),
 (wch->pcdata->bgf ? "\x1B[1;37m-\x1B[37;0m" : " "), 
 (IN_BATTLE (wch) ? "\x1B[1;34m+\x1B[37;0m" : " "), 
 wch->timer > 49 ? "\x1B[1;37mA\x1B[37;0m" : " ", 
 wch->ced && wch->ced->pnote ? "\x1B[1;37mN\x1B[37;0m" : " ", 
 (IS_AFFECTED (wch, AFF_INVISIBLE)) ? "\x1B[0;36mI\x1B[37;0m" : " ", 
 (IS_SET (wch->pcdata->act2, PLR_SILENCE) ? "\x1B[1;31mS\x1B[37;0m" : " "), 
 (IS_SET (wch->act, PLR_FREEZE) ? "\x1B[1;34mF\x1B[37;0m" : " "),
 (wch->pcdata->no_quit_pk > 3 ? "\x1B[1;31mP\x1B[37;0m" : " "), 
 (wch->in_room ? wch->in_room->vnum : 0), 
 pos_name(wch->position));
		  }
		else
		  {
		    sprintf (hugebuf_o + strlen (hugebuf_o),
			     "\x1B[1;30m[\x1B[32m%c%s %c%3d%c%c\x1B[1;30m]\x1B[37;0m %s%s%s%s%s%s%s\x1B[37;0m\n\r",
			     (wch->pcdata->no_quit_pk > 4 ? 'P' : ' '),
			     (IS_EVIL (wch) ? "\x1B[31;1mE\x1B[32m" : " "),
			     (IN_BATTLE (wch) ? '+' : ' '),
			     LEVEL (wch),
			     (IS_SET (wch->pcdata->act2, PLR_SILENCE) ? 's'
			      : (IS_SET (wch->act, PLR_FREEZE) ? 'f' : ' ')),
			     (wch->pcdata->bgf ? '-' : ' '), wch->timer > 49 ? "\x1B[37;1m<AFK> \x1B[0;37m" 
                             : (wch->ced != NULL ? (wch->ced->pnote !=NULL ? "\x1B[37;1m<NOTE> \x1B[0;37m" : "") : ""),
			     NAME (wch),
			     ((wch->pcdata->title[0] != ',' && wch->pcdata->title[0] != '.') ? " " : ""),
			     wch->pcdata->title,
			     (IS_AFFECTED (wch, AFF_INVISIBLE)) ? " \x1B[0;36m(\x1B[1;30mInvis\x1B[0;36m)\x1B[0;37m" : "",
			     (wch->pcdata->wizinvis != 0 ? " \x1B[0;36m(\x1B[1;30mWizInvis\x1B[0;36m)\x1B[0;37m" : ""));
		  }

	      }

//Player Who List Below
	    else
	      {
		if (doe[0] == '\0')
		  {
		    int iii;
		    iii = (number_range (1, 2) == 1 ? 34 : 32);
		    if (IS_EVIL (wch))
		      /*sprintf (doe, "\x1B[31;1m %-7s\x1B[0;37m", evil);*/
		      sprintf (doe, "\x1B[1;37mLvl \x1B[1;34m%2d\x1B[0;37m", LEVEL (wch));
		    if (!wch->pcdata->show_lev)
		      sprintf (doe, "\x1B[1;34m-=A-n-o-n=-\x1B[0;37m");
		    else
                //Are they in a BG?                          lvl           remorts
    sprintf (doe, "\x1B[1;30mLvl:\x1B[1;37m %2d \x1B[0;36mR:%d",  
    LEVEL (wch), 
    wch->pcdata->remort_times);
		  }
		sprintf (hugebuf_o + strlen (hugebuf_o),
//           doe,           Guild      Name  afk          T,I,W,B,S,F
 "\x1B[30;1m[%s\x1B[30;1m]%s \x1B[1;37m%s %s\x1B[37;0m%s%s%s%s%s%s\x1B[37;0m\n\r",
 doe,
 get_main_guild(wch),
 NAME (wch),
 wch->timer > 49 ? "\x1B[37;1m<AFK> \x1B[37;0m" : "",
 wch->pcdata->title,
 (IS_AFFECTED (wch, AFF_INVISIBLE)) ? " \x1B[0;36m(\x1B[1;30mInvis\x1B[0;36m)\x1B[0;37m" : "",
 (wch->pcdata->wizinvis != 0 ? " \x1B[0;36m(\x1B[1;30mWizInvis\x1B[0;36m)\x1B[0;37m" : ""),
 (IN_BATTLE (wch) ? "\x1B[0;36m(\x1B[1;37mBG'ing\x1B[0;36m)" : ""),
 (IS_SET (wch->pcdata->act2, PLR_SILENCE) ? " \x1B[1;31m(\x1B[0;31mSilenced\x1B[1;31m)" : ""), 
 (IS_SET (wch->act, PLR_FREEZE) ? " \x1B[0;34m(\x1B[1;34mFrozen\x1B[0;34m)" : ""));
	      }
	  }
	}
      /*end Pass */
    }
sprintf (hugebuf_o + strlen (hugebuf_o), 
"\x1B[1;30m[\x1B[1;31m+\x1B[1;30m]\x1B[1;34m-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\x1B[1;30m[\x1B[1;31m+\x1B[1;30m]\x1B[37;0m\n\r");
  sprintf (hugebuf_o + strlen (hugebuf_o),
	   "There %s %d total player%s visible to you logged into VoR.\n\r",
	   nMatch > 1 ? "are" : "is", nMatch, nMatch > 1 ? "s" : "");
/*sprintf(hugebuf_o+strlen(hugebuf_o),"Max players online at once since reboot: %d.\n\r",
   max_connected); */
  page_to_char (hugebuf_o, ch);
  return;
}

//Taking from LOC for guilds
char* get_main_guild (CHAR_DATA * ch)
{
  char* buf;

     if (is_member(ch, GUILD_ELEMENTAL) && is_member(ch, GUILD_MYSTICS))   
buf = "\x1b[0;36m[\x1b[1;30m-\x1b[1;31mE\x1b[1;30mL\x1b[1;34mE\x1b[1;30m-\x1b[0;36m]";
else if (is_member(ch, GUILD_HEALER) && is_member(ch, GUILD_SHAMAN))       
buf = "\x1b[0;36m[\x1b[1;30m-\x1b[1;33mH\x1b[0;32mE\x1b[1;33mA\x1b[1;30m-\x1b[0;36m]";
else if (is_member(ch, GUILD_WIZARD)&& is_member(ch, GUILD_MYSTICS))       
buf = "\x1b[0;36m[\x1b[1;30m-\x1b[1;34mW\x1b[0;34mI\x1b[1;34mZ\x1b[1;30m-\x1b[0;36m]";
else if (is_member(ch, GUILD_THIEFG) && is_member(ch, GUILD_ASSASSIN))     
buf = "\x1b[0;36m[\x1b[1;30m-\x1b[1;30mT\x1B[0;37mH\x1b[1;30mI\x1b[1;30m-\x1b[0;36m]";
else if (is_member(ch, GUILD_MARAUDER) && is_member(ch, GUILD_TATICS))     
buf = "\x1b[0;36m[\x1b[1;30m-\x1b[0;37mM\x1b[1;30mA\x1b[0;37mR\x1b[1;30m-\x1b[0;36m]";
else if (is_member(ch, GUILD_WARRIOR) && is_member(ch, GUILD_TATICS))      
buf = "\x1b[0;36m[\x1b[1;30m-\x1b[0;37mW\x1b[0;31mA\x1b[0;37mR\x1b[1;30m-\x1b[0;36m]";
else if (is_member(ch, GUILD_NECROMANCER) && is_member(ch, GUILD_SHAMAN))  
buf = "\x1b[0;36m[\x1b[1;30m-\x1b[1;31mN\x1b[1;30mE\x1b[1;31mC\x1b[1;30m-\x1b[0;36m]";
else if (is_member(ch, GUILD_PALADIN) && is_member(ch, GUILD_TATICS) || is_member(ch, GUILD_PALADIN) && is_member(ch, GUILD_SHAMAN))      
buf = "\x1b[0;36m[\x1b[1;30m-\x1b[0;37mP\x1b[0;33mA\x1b[0;37mL\x1b[1;30m-\x1b[0;36m]";
else if (is_member(ch, GUILD_BATTLEMAGE) && is_member(ch, GUILD_TATICS) || is_member(ch, GUILD_BATTLEMAGE) && is_member(ch, GUILD_MYSTICS))   
buf = "\x1b[0;36m[\x1b[1;30m-\x1b[0;37mB\x1b[1;34mL\x1B[0;37mM\x1b[1;30m-\x1b[0;36m]";
else                                                                       
buf = "\x1b[1;30m[\x1B[0;37mComnr\x1b[1;30m]";

  return buf;
}

void
do_vwho (CHAR_DATA * ch, char *argy)
{
  CHAR_DATA *wch;
  int pass;
  DESCRIPTOR_DATA *d;
  int vis;
  int ii;
  int nMatch;
  bool tempbool;
  int gMatch;
  int align;
  int levl;
  bool namey;
  char doe[STD_LENGTH];
  bool clanny;
  bool tally_all = FALSE;
  DEFINE_COMMAND ("vwho", do_vwho, POSITION_DEAD, 0, LOG_NORMAL, "This command shows who is playing and visible to you at the moment.")
    if (IS_JAVA (ch))
    invoke_who_window (ch);

  ii = clan_number (ch);
  align = 0;
  nMatch = 0;
  clanny = FALSE;
  gMatch = 0;
  vis = 0;
  hugebuf_o[0] = '\0';
  sprintf (hugebuf_o,
	   "\x1B[0;36m-----------------------------------------------------------------------------\x1B[37;0m\n\r");
  namey = FALSE;
  levl = 0;
  if (argy[0] == '\0' || argy == "")
    {
      if (LEVEL (ch) > 100 && LEVEL (ch) < 105)
	align = 3;
      else
	namey = FALSE;
      tally_all = TRUE;
    }
  else
    {
      if (!str_cmp (argy, "clan"))
	{
	  clanny = TRUE;
	  if (ii < 1)
	    {
	      send_to_char ("You aren't in a clan!\n\r", ch);
	      return;
	    }
	}
      else if (!str_cmp (argy, "battle") || !str_cmp (argy, "battleground"))
	align = 4;
      else if (!str_cmp (argy, "evil"))
	align = 1;
      else if (!str_cmp (argy, "good"))
	align = 2;
      else if (!str_cmp (argy, "god") || !str_cmp (argy, "immortal"))
	align = 3;
      else if (LEVEL (ch) > 100 && is_number (argy))
	{
	  levl = atoi (argy);
	}
      else
	{
	  argy[0] = UPPER (argy[0]);
	  namey = TRUE;
	}
    }
  tempbool = FALSE;
  for (pass = 0; pass != 3; pass = pass + 1)
    {
      if (!tempbool && pass != 0 && (gMatch > 0 || LEVEL (ch) > 100))
	{
	  tempbool = TRUE;
	  sprintf (hugebuf_o + strlen (hugebuf_o),
		   "\x1B[0;36m- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\x1B[37;0m\n\r");
	}

      for (d = descriptor_list; d != NULL; d = d->next)
	{
	  wch = NULL;
	  if (d->character == NULL)
	    continue;
	  if (IS_MOB (d->character))
	    continue;
	  if (d->character->desc == NULL)
	    continue;
	  if ((d->connected != CON_PLAYING
	       && d->connected != CON_AEDITOR
	       && d->connected != CON_REDITOR
	       && d->connected != CON_CEDITOR
	       && d->connected != CON_OEDITOR
	       && d->connected != CON_MEDITOR) || !can_see (ch, d->character))
	    continue;
	  if (d->connected != CON_PLAYING && !IS_IMMORTAL (ch))
	    continue;
	  wch = (d->original != NULL) ? d->original : d->character;
	  if (clanny && clan_number (wch) != ii)
	    continue;
	  if (LEVEL (ch) > 100 && LEVEL (wch) < levl)
	    continue;
	  if (namey && str_prefix (capitalize (argy), NAME (wch)))
	    continue;
	  if (LEVEL (wch) > 100 && pass != 0)
	    continue;
	  if (LEVEL (wch) <= 100 && pass == 0)
	    continue;
	  if (LEVEL (wch) <= 100 && !IS_EVIL (wch) && pass != 2)
	    continue;
	  if (LEVEL (wch) <= 100 && pass == 2 && IS_EVIL (wch))
	    continue;
	  if (align == 1 && !IS_EVIL (wch))
	    continue;
	  if (align == 2 && IS_EVIL (wch))
	    continue;
	  if (align == 3 && LEVEL (wch) < 100)
	    continue;
	  if (LEVEL (ch) < 100 && !IS_EVIL (ch) && IS_EVIL (wch))
	    continue;
	  if (align == 4 && !IN_BATTLE (wch))
	    continue;
	  if (IS_EVIL (ch) && LEVEL (ch) < 100 && !IS_EVIL (wch))
	    continue;
	  if (LEVEL (wch) < 110 && LEVEL (wch) > 99 && LEVEL (ch) < 100)
	    continue;
	  if (tally_all)
	    nMatch++;
	  doe[0] = '\0';



	  if (IS_JAVA (ch))
	    {
	      if (!tally_all)
		nMatch++;
	      if (IS_PLAYER (ch) && LEVEL (ch) < 100 && !IS_MOB (wch)
		  && (!not_is_same_align (ch, wch)))
		vis++;
	      add_who_entry (ch, wch);
	      continue;
	    }



	  if (LEVEL (wch) > 99)
	    {
	      if (wch->pcdata->wizinvis == 0)
		gMatch++;
	      if (d->connected != CON_PLAYING)
		sprintf (doe, "\x1B[1;39m  Building    ");
	      else
		{
		  switch (LEVEL (wch))
		    {
		    default:
		      sprintf (doe, "\x1B[1;33m  Builder  ");
		      break;
		    case TOP_LEVEL:
		      sprintf (doe, "\x1B[1;35m-* Admin *-");
		      break;
		    }
		}
	      if (wch->pcdata->quiet == 1)
		sprintf (doe, "\x1B[1;35m- Quiet Mode -");
	      if (wch->pcdata->quiet == 2)
		sprintf (doe, "\x1B[1;34m- Busy Mode - ");
	    }
	  {
	    if (!tally_all)
	      nMatch++;
	    if (IS_PLAYER (ch) && LEVEL (ch) < 100 && !IS_MOB (wch)
		&& (!not_is_same_align (ch, wch)))
	      vis++;
	    if (ii > 0)
	      {
		if (clan_number (wch) == ii)
		  {
		    sprintf (hugebuf_o + strlen (hugebuf_o), "<C>");
		  }
		else
		  sprintf (hugebuf_o + strlen (hugebuf_o), "   ");
	      }

	    if (LEVEL (ch) >= 110 && LEVEL (wch) <= 100
		&& IS_SET (ch->act, PLR_HOLYLIGHT))
	      {
		/*
		   sprintf (hugebuf_o + strlen (hugebuf_o),
		   "\x1B[1;30m[\x1B[32m%s%s %s%3d%s%s\x1B[1;30m]\x1B[37;0m %s%s%s%s%s%s\x1B[37;0m\n\r",
		   (wch->pcdata->bounty ? "B" : " "),
		   (IS_EVIL (wch) ? "\x1B[31;1mA\x1B[32m" : " "),
		   (IN_BATTLE (wch) ? "+" : " "), LEVEL (wch),
		   (IS_SET (wch->pcdata->act2, PLR_SILENCE) ? "s" : " "),
		   (wch->pcdata->bgf ? "-" : " "),
		   wch->timer > 49 ? "\x1B[37;1m<AFK> \x1B[37;0m" : "", NAME (wch),
		   ((wch->
		   pcdata->title[0] != ','
		   && wch->pcdata->title[0] != '.') ? " " : ""), wch->pcdata->title,
		   (IS_AFFECTED (wch, AFF_INVISIBLE)) ? " (Invis)" : "",
		   (wch->pcdata->wizinvis != 0 ? " (Invis)" : ""));
		 */

		if (LEVEL (ch) >= 110 && str_cmp (argy, "old"))
		  {
        //char smallbuf[SML_LENGTH];
        //sprintf(smallbuf, pos_name(wch->position));
        //if (FIGHTING(ch) && IS_EVIL(ch) != IS_EVIL(FIGHTING(ch)))
        //  sprintf(smallbuf, "PK");
		    sprintf (hugebuf_o + strlen (hugebuf_o),
			     "\x1B[1;30m[\x1B[32m%c%s %c%3d%c%c\x1B[1;30m]\x1B[37;0m %-15s \x1B[30;1m[\x1B[37;0m%c%c%c\x1B[30;1m] [ \x1B[37;0m%5d \x1B[30;1m] [ \x1B[37;0m%s \x1B[30;1m]\x1B[37;0m\n\r",
			     (wch->pcdata->bounty ? 'B' : ' '),
			     (IS_EVIL (wch) ? "\x1B[31;1mE\x1B[32m" : " "),
			     (IN_BATTLE (wch) ? '+' : ' '),
			     LEVEL (wch),
			     (IS_SET (wch->pcdata->act2, PLR_SILENCE) ? 's'
			      : (IS_SET (wch->act, PLR_FREEZE) ? 'f' : ' ')),
			     (wch->pcdata->bgf ? '-' : ' '), NAME (wch),
			     wch->timer > 49 ? 'A' : ' ', wch->ced
			     && wch->ced->pnote ? 'N' : ' ',
			     (IS_AFFECTED (wch, AFF_INVISIBLE)) ? 'I' : ' ',
			     (wch->in_room ? wch->in_room->vnum : 0),
			     //smallbuf);
           pos_name(wch->position));
		  }
		else
		  {
		    sprintf (hugebuf_o + strlen (hugebuf_o),
			     "\x1B[1;30m[\x1B[32m%c%s %c%3d%c%c\x1B[1;30m]\x1B[37;0m %s%s%s%s%s%s\x1B[37;0m\n\r",
			     (wch->pcdata->bounty ? 'B' : ' '),
			     (IS_EVIL (wch) ? "\x1B[31;1mE\x1B[32m" : " "),
			     (IN_BATTLE (wch) ? '+' : ' '),
			     LEVEL (wch),
			     (IS_SET (wch->pcdata->act2, PLR_SILENCE) ? 's'
			      : (IS_SET (wch->act, PLR_FREEZE) ? 'f' : ' ')),
			     (wch->pcdata->bgf ? '-' : ' '),
			     wch->timer > 49 ? "\x1B[37;1m<AFK> \x1B[37;0m" : 
                             (wch->ced != NULL ? (wch-> ced-> pnote != NULL ? "\x1B[37;1m<NOTE> \x1B[37;0m" : "") : ""),
			     NAME (wch),
			     ((wch->pcdata->title[0] != ',' && wch->pcdata->title[0] != '.') ? " " : ""),
			     wch->pcdata->title,
			     (IS_AFFECTED (wch, AFF_INVISIBLE)) ? " (Invis)" : "",
			     (wch->pcdata->wizinvis != 0 ? " (Invis)" : ""));
		  }

	      }
	    else
	      {
		if (doe[0] == '\0')
		  {
		    int iii;
		    iii = (number_range (1, 2) == 1 ? 34 : 32);
		    if (IS_EVIL (wch))
/* 		      sprintf (doe, "\x1B[31;1m %-7s", evil); */
		      sprintf (doe, "\x1B[%d;1mLevel %2d", 32, LEVEL (wch));
		    if (!wch->pcdata->show_lev)
		      sprintf (doe, "\x1B[%d;1m- - - - ", 34);
		    else
		      sprintf (doe, "\x1B[%d;1mLevel %2d", 32, LEVEL (wch));
		  }
		sprintf (hugebuf_o + strlen (hugebuf_o),
			 "\x1B[30;1m[ %s \x1B[30;1m]\x1B[37;0m %s%s%s%s%s\x1B[37;0m\n\r",
			 doe,
			 wch->timer > 49 ? "\x1B[37;1m<AFK> \x1B[37;0m" : "",
			 NAME (wch),
			 (wch->pcdata->title[0] != ',' ? " " : ""),
			 wch->pcdata->title,
			 (wch->pcdata->wizinvis != 0 ? " (Invis)" : ""));

	      }
	  }
	}
      /*end Pass */
    }
  sprintf (hugebuf_o + strlen (hugebuf_o),
	   "\x1B[0;36m-----------------------------------------------------------------------------\x1B[37;0m\n\r");
  sprintf (hugebuf_o + strlen (hugebuf_o),
	   "There %s %d player%s visible to you and %d total player%s.\n\r",
	   vis > 1 ? "are" : "is", vis, vis > 1 ? "s" : "", nMatch,
	   nMatch > 1 ? "s" : "");
/*sprintf(hugebuf_o+strlen(hugebuf_o),"Max players online at once since reboot: %d.\n\r",
   max_connected); */
  if (IS_JAVA (ch))
    show_who_window (ch);
  else
    page_to_char (hugebuf_o, ch);
  return;
}


char *const day_name[] = {
  "Monday", "Tuesday", "Wednesday", "Thursday",
  "Friday", "Saturday", "Sunday"
}

 ;
char *const month_name[] = {
  "Winter", "Winter", "Midwinter", "Midwinter",
  "Early Spring", "Spring", "Spring",
  "Late Spring", "Early Summer", "Summer",
  "Midsummer", "High Summer", "Late Summer", "Early Autumn",
  "Autumn", "Mid-Autumn", "Late Autumn", "Aturion Twilight",
  "Aturion Twilight", "The Frigid Moon", "Early Winter"
}

 ;
void
do_time (CHAR_DATA * ch, char *argy)
{
  char buf[STD_LENGTH];
  char *suf;
  int day;
  DEFINE_COMMAND ("time", do_time, POSITION_DEAD, 0, LOG_NORMAL, "This command shows you the current time.") 
  day = time_info.day + 1;
  if (day > 4 && day < 20)
    suf = "th";
  else if (day % 10 == 1)
    suf = "st";
  else if (day % 10 == 2)
    suf = "nd";
  else if (day % 10 == 3)
    suf = "rd";
  else
    suf = "th";
  sprintf (buf,
	   "It is %d o'clock %s on the day of %s.\n\r(The %d%s of %s. Year: %d)\n\r",
	   (time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
	   time_info.hour >= 12 ? "pm" : "am",
	   day_name[day % 7],
	   day, suf, month_name[time_info.month], time_info.year);
  send_to_char (buf, ch);
  return;
}

int
cur_temp (CHAR_DATA * ch)
{
  int temp_adj;
  int rweth;
  rweth = weather_info.temperature;
  temp_adj = 0;
  if (!ch->in_room || !ch->in_room->area)
    return 60;
  switch (ch->in_room->sector_type)
    {
    case SECT_DESERT:
      temp_adj = number_fuzzy (110 - weather_info.temperature);
      break;
    case SECT_TROPICAL:
      temp_adj = number_fuzzy (110 - weather_info.temperature);
      break;
    case SECT_SNOW:
      temp_adj = number_fuzzy (30 - weather_info.temperature);
      break;
    case SECT_INSIDE:
      temp_adj = number_fuzzy (80 - weather_info.temperature);
      break;
    case SECT_MOUNTAIN:
      temp_adj = number_fuzzy (50 - weather_info.temperature);
      break;
    case SECT_AIR:
      temp_adj = number_fuzzy (60 - weather_info.temperature);
      break;
    }
  rweth += temp_adj;
  return rweth;
}

// Some Weather crap borrowed from other codebases */

void
do_weather (CHAR_DATA * ch, char *argy)
{
  char buf[STD_LENGTH];
  char descr[STD_LENGTH];
  short wind;
  int temp_adj, sky, wt;
  static char *const sky_look[] = {
    "cloudless",
    "cloudy",
    "rainy",
    "lit by flashes of lightning",
    "thick with fog",
    "slightly sunny",
    "slightly overcast",
    "scattered with a few flakes",
    "filled with flakes",
    "a blizzard of white",
    "thick with fog",
    "not visible underwater.",
    "stormy"
  }
   ;
  static char *const wind_tmp[4] = {
    "cold", "cool", "warm", "hot"
  }
   ;
  DEFINE_COMMAND ("weather", do_weather, POSITION_RESTING, 0, LOG_NORMAL,
		  "This command allows you to see the weather (outdoors).")
    wind = weather_info.windspeed;
  sky = weather_info.sky;
  temp_adj = 0;
  do_time (ch, "");
  switch (ch->in_room->sector_type)
    {
    case SECT_UNDERWATER:
      temp_adj = number_fuzzy (70 - weather_info.temperature);
      sky = 11;
      wt = -1;
      break;
    case SECT_DESERT:
      temp_adj = number_fuzzy (110 - weather_info.temperature);
      if (sky == 2)
	sky = 12;
      break;
    case SECT_TROPICAL:
      temp_adj = number_fuzzy (110 - weather_info.temperature);
      break;
    case SECT_SNOW:
      temp_adj = number_fuzzy (30 - weather_info.temperature);
      break;
    case SECT_INSIDE:
      temp_adj = number_fuzzy (80 - weather_info.temperature);
      break;
    case SECT_MOUNTAIN:
      temp_adj = number_fuzzy (50 - weather_info.temperature);
      break;
    case SECT_AIR:
      temp_adj = number_fuzzy (60 - weather_info.temperature);
      break;
    }
  if (temp_adj + weather_info.temperature > 90)
    wt = 3;
  else if (temp_adj + weather_info.temperature > 75)
    wt = 2;
  else if (temp_adj + weather_info.temperature > 50)
    wt = 1;
  else
    wt = 0;
  if (!IS_OUTSIDE (ch))
    {
      send_to_char ("You can only see the weather from the outdoors.\n\r",
		    ch);
      return;
    }
  descr[0] = '\0';
  sprintf (buf, "The sky is %s ",
	   ((time_info.month <= 4 || time_info.month >= 17)
	    && (temp_adj + weather_info.temperature <= 32)) ?
	   sky_look[sky + 5] : sky_look[sky]);
  strcat (descr, buf);
  if (wt != -1)
    {
      sprintf (buf, "and a %s %sward %s blows.\n\r",
	       wind_tmp[wt],
	       dir_name[abs (weather_info.winddir) % 3],
	       wind < 10 ? "breeze" :
	       wind < 20 ? "wind" : wind < 30 ? "gust" : "torrent");
      strcat (descr, buf);
    }
  sprintf (buf, "It is %d degrees right now.\n\r",
	   (temp_adj + weather_info.temperature));
  strcat (descr, buf);
/*if ( IS_IMMORTAL( ch ) )
   {
   sprintf( buf, "(temp: %d, windspd: %d, wind dir: %d, sky: %d, mmhg: %d, change: %d)\n\r",
   (temp_adj+weather_info.temperature),
   weather_info.windspeed,
   weather_info.winddir,
   weather_info.sky,
   weather_info.mmhg,
   weather_info.change );
   strcat( descr, buf );
   }
 */
  send_to_char (descr, ch);
  return;
}
