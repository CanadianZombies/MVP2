#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"

struct int_apply_type int_app[36];
struct wis_apply_type wis_app[36];
struct con_apply_type con_app[36];
//added the dex to this line because it was causing errors in
//emlen.h... it is the only spply_type that was nowhere in the code.
//struct dex_apply_type dex_app[36];

int lne = 1;
char line[40960];

int
grab_num_2 (char *txt)
{
  int n = 0;
  char *t;
  for (t = txt; *t != '\0'; t++)
    {
      if (*t >= '0' && *t <= '9')
	{
	  n *= 10;
	  n += (*t) - '0';
	  continue;
	}
    }
  return n;
}

bool prmpt = FALSE;

char *
interpret_line (char *inlne, CHAR_DATA * ch)
{
  char tmpcd[100];
  int gpos;
  int pos;
  char *t;
  line[0] = '\0';
  lne = 1;
  gpos = 0;
  if (inlne == NULL)
    return "(type prompt all)";
  for (t = inlne; *t != '\0'; t++)
    {
      tmpcd[0] = '\0';
      pos = 0;
      if (*t == '\n')
	lne++;
      if (prmpt && (*t == '$' || *t == '%') &&
	  *(t + 1) != 'B' && *(t + 1) != 'R' && (*(t + 1) < '0'
						 || *(t + 1) > '9'))
	{
	  *t = '.';
	}
      if (*t == '*')
	{
	  for (t++; *t != '*'; t++)
	    {
	      if (pos >= 10)
		{
		  fprintf (stderr, "Bad interpret * code.. '%s'.\n", inlne);
		  line[0] = '\0';
		  return line;
		}
	      tmpcd[pos] = *t;
	      pos++;
	    }
	  tmpcd[pos] = '\0';
	  line[gpos] = '\0';
	  strcat (line, parse_txt (tmpcd, ch));
	  gpos = strlen (line);
	  continue;
	}
      line[gpos] = *t;
      gpos++;
    }

  line[gpos] = '\0';

  return line;
}



char *
parse_txt (char *code, CHAR_DATA * ch)
{
  static char bf[5550];
  int sl = 0;
  int pad = 0;
  int i;
  bf[0] = '\0';

  pad = grab_num_2 (code);

  if (!str_infix ("mnn", code))
    {
      sprintf (bf, "%d", mana_now (ch));
    }
  else if (!str_infix ("str", code))
    {
      sprintf (bf, "%d", get_curr_str (ch));
    }
  else if (!str_infix ("pstr", code))
    {
      sprintf (bf, "%d", get_perm_stat (ch, STR_I));
    }
  else if (!str_infix ("int", code))
    {
      sprintf (bf, "%d", get_curr_int (ch));
    }
  else if (!str_infix ("pint", code))
    {
      sprintf (bf, "%d", get_perm_stat (ch, INT_I));
    }
  else if (!str_infix ("wis", code))
    {
      sprintf (bf, "%d", get_curr_wis (ch));
    }
  else if (!str_infix ("pwis", code))
    {
      sprintf (bf, "%d", get_perm_stat (ch, WIS_I));
    }
  else if (!str_infix ("dex", code))
    {
      sprintf (bf, "%d", get_curr_dex (ch));
    }
  else if (!str_infix ("pdex", code))
    {
      sprintf (bf, "%d", get_perm_stat (ch, DEX_I));
    }
  else if (!str_infix ("con", code))
    {
      sprintf (bf, "%d", get_curr_con (ch));
    }
  else if (!str_infix ("pcon", code))
    {
      sprintf (bf, "%d", get_perm_stat (ch, CON_I));
    }
  else if (!str_infix ("rem", code))
    {
      sprintf (bf, "%d", ch->pcdata->remort_times);
    }
  else if (!str_infix ("rank", code))
    {
      if (LEVEL (ch) < 10)
	sprintf (bf, "inexperienced");
      else if (LEVEL (ch) < 25)
	sprintf (bf, "familiar");
      else if (LEVEL (ch) < 45)
	sprintf (bf, "well respected");
      else if (LEVEL (ch) < 70)
	sprintf (bf, "a hero");
      else if (LEVEL (ch) < 100)
	sprintf (bf, "a legend");
      else
	sprintf (bf, "a God");
    }
  else if (!str_infix ("aff", code))
    {
      if (ch->affected != NULL)
	{
	  SPELL_DATA *spell;
	  AFFECT_DATA *paf;
	  char buf[500];
	  strcat (bf, "\n\r\x1B[1;32mYou are affected by:\x1B[0;37m");
	  lne++;
	  for (paf = ch->affected; paf != NULL; paf = paf->next)
	    {
	      if ((spell = skill_lookup (NULL, paf->type)) == NULL)
		continue;
	      lne++;
	      sprintf (buf, "\n\r%s", capitalize (spell->spell_funky_name));
	      strcat (bf, buf);
	      if (IS_MOB (ch) || (LEVEL (ch) > 100))
		sprintf (buf, " (modifies %s by %d for %d mud hours)", affect_loc_name (paf->location), paf->modifier, paf->duration);
	      else
		{
		  if (paf->duration >= 72)
		    sprintf (buf, ", which won't wear off for many days!");
		  else if (paf->duration >= 48)
		    sprintf (buf, ", which will wear off in a couple of days.");
		  else if (paf->duration >= 24)
		    sprintf (buf, ", which will wear off in about a day.");
		  else if (paf->duration >= 12)
		    sprintf (buf, ", which will wear off in about half a day.");
		  else if (paf->duration >= 6)
		    sprintf (buf, ", which will wear off in about a quarter of a day.");
		  else if (paf->duration >= 3)
		    sprintf (buf, ", which will wear off in a few hours.");
		  else if (paf->duration > 1)
		    sprintf (buf, ", which will wear off in one or two hours.");
		  else if (paf->duration == 1)
		    sprintf (buf, ", which will wear off very soon.");
		  else
		    sprintf (buf, ", which will wear off at any moment.");
		}
	      strcat (bf, buf);
	    }
	}
    }
  else if (!str_infix ("warm", code) && IS_PLAYER (ch))
    {
      if (cur_temp (ch) < 50)
	{
	  int ii;
	  ii = get_warmth (ch);
	  if (ii > 10 && ii < 20)
	    strcat (bf, "\n\rYour clothing doesn't protect you too well in cold weather.");
	  if (ii > 19 && ii < 31)
	    strcat (bf, "\n\rYour clothing protects you moderately well in cold weather.");
	  if (ii > 30 && ii < 42)
	    strcat (bf, "\n\rYour clothing protects you quite well in cold weather.");
	  if (ii > 41 && ii < 62)
	    strcat (bf, "\n\rYour clothing would keep you warm in some of the most frigid conditions.");
	  if (ii > 61)
	    strcat (bf, "\n\rYour clothing would keep you very warm even in the middle of a blizzard!");
	  lne++;
	}
    }
  else if (!str_infix ("cond", code))
    {
      char tbf[100];
      tbf[0] = '\0';
      if (ch->pcdata->condition[COND_DRUNK] > 10)
	strcat (tbf, "You are drunk.\n\r");
      if (ch->pcdata->condition[COND_FULL] < 10 &&
	  ch->pcdata->condition[COND_FULL] > 5)
	strcat (tbf, "You are starting to get \x1B[1;33mhungry\x1B[37;0m.\n\r");
      if (ch->pcdata->condition[COND_THIRST] < 10 &&
	  ch->pcdata->condition[COND_THIRST] > 5)
	strcat (tbf, "You are beginning to get \x1B[1;34mthirsty\x1B[37;0m.\n\r");
      if (ch->pcdata->condition[COND_FULL] < 5 &&
	  ch->pcdata->condition[COND_FULL] > 0)
	strcat (tbf, "You are \x1B[1;33mhungry.\n\r");
      if (ch->pcdata->condition[COND_THIRST] < 5 &&
	  ch->pcdata->condition[COND_THIRST] > 0)
	strcat (tbf, "You are \x1B[1;34mthirsty.\n\r");
      if (ch->pcdata->condition[COND_THIRST] < 1)
strcat (tbf, "You are totally \x1B[1;34md\x1B[1;30me\x1B[1;34mh\x1B[1;30my\x1B[1;34md\x1B[1;30mr\x1B[1;34ma\x1B[1;30mt\x1B[1;34me\x1B[1;30md\x1B[37;0m!\n\r");
      if (ch->pcdata->condition[COND_FULL] < 1)
	strcat (tbf, "You are \x1B[1;33mS\x1B[1;30mT\x1B[1;33mA\x1B[1;30mR\x1B[1;33mV\x1B[1;30mI\x1B[1;33mN\x1B[1;30mG\x1B[37;0m!\n\r");
      if (tbf[0] != '\0')
	{
	  lne++;
	  strcat (bf, "\n\r");
	  strcat (bf, tbf);
	}
    }
  else if (!str_infix ("pos", code))
    {
      char tbf[100];
      tbf[0] = '\0';
      switch (ch->position)
	{
	case POSITION_DEAD:
	  strcpy (tbf, "You are DEAD!!");
	  break;
	case POSITION_MORTAL:
	  strcpy (tbf, "You are mortally wounded.");
	  break;
	case POSITION_INCAP:
	  strcpy (tbf, "You are incapacitated.");
	  break;
	case POSITION_STUNNED:
	  strcpy (tbf, "You are stunned.");
	  break;
	case POSITION_SLEEPING:
	  strcpy (tbf, "You are sleeping.");
	  break;
	case POSITION_RESTING:
	  strcpy (tbf, "You are resting.");
	  break;
	case POSITION_MEDITATING:
	  strcpy (tbf, "You are meditating.");
	  break;
	}
      if (tbf[0] != '\0')
	{
	  strcat (bf, "\n\r");
	  lne++;
	  strcat (bf, tbf);
	}
    }
  else if (!str_infix ("mmm", code))
    {
      sprintf (bf, "%d", max_mana (ch));
    }
  else if (!str_infix ("dth", code))
    {
      sprintf (bf, "%d", (IS_MOB (ch) ? 0 : ch->pcdata->deaths));
    }
  else if (!str_infix ("pra", code))
    {
      sprintf (bf, "%d", (IS_MOB (ch) ? 0 : ch->pcdata->practice));
    }
  else if (!str_infix ("lea", code))
    {
      sprintf (bf, "%d", (IS_MOB (ch) ? 0 : ch->pcdata->learn));
    }
  else if (!str_infix ("sect", code))
    {
      int ii;
      if (IS_PLAYER (ch))
	{
	  ii = clan_number_2 (ch);
	  if (ii > 0)
	    {
	      sprintf (bf, "\n\rMember of \x1B[36;1m%s\x1B[37;0m sect.",
		       get_clan_index_2 (ii)->name);
	      lne++;
	    }
	}
    }
  else if (!str_infix ("clan", code))
    {
      int ii;
      if (IS_PLAYER (ch))
	{
	  ii = clan_number (ch);
	  if (ii > 0)
	    {
	      sprintf (bf, "\x1B[0;37mMember of \x1B[1;31m%s\x1B[0;37m clan.", get_clan_index (ii)->name);
	      lne++;
	    }
	}
    }
  else if (!str_infix ("guil", code) /*&& ch->pcdata->guilds != 0*/ && IS_PLAYER (ch))
    {
      bool prev;
      prev = FALSE;
      strcat (bf, "Guilds: \x1B[37;1m");
      lne++;
      if (IS_SET (ch->pcdata->guilds, GUILD_MYSTICS))
        {
          strcat (bf, "Mystic's Guild");
          prev = TRUE;
        }
      if (IS_SET (ch->pcdata->guilds, GUILD_SHAMAN))
        {
          if (prev)
            strcat (bf, ", ");
          strcat (bf, "Shaman's Guild");
          prev = TRUE;
        }
      if (IS_SET (ch->pcdata->guilds, GUILD_TATICS))
        {
          if (prev)
            strcat (bf, ", ");
          strcat (bf, "Tatics's Guild");
          prev = TRUE;
        }
      if (IS_SET (ch->pcdata->guilds, GUILD_TINKER))
	{
          if (prev)
            strcat (bf, ", ");
	  strcat (bf, "Tinker's Guild");
	  prev = TRUE;
	}
      if (IS_SET (ch->pcdata->guilds, GUILD_WARRIOR))
	{
	  if (prev)
	    strcat (bf, ", ");
	  strcat (bf, "Warrior's Guild");
	  prev = TRUE;
	}
      if (IS_SET (ch->pcdata->guilds, GUILD_HEALER))
	{
	  if (prev)
	    strcat (bf, ", ");
	  strcat (bf, "Healer's Guild");
	  prev = TRUE;
	}
      if (IS_SET (ch->pcdata->guilds, GUILD_WIZARD))
	{
	  if (prev)
	    strcat (bf, ", ");
	  strcat (bf, "Wizard's Guild");
	  prev = TRUE;
	}
      if (IS_SET (ch->pcdata->guilds, GUILD_THIEFG))
	{
	  if (prev)
	    strcat (bf, ", ");
	  strcat (bf, "Thief's Guild");
	  prev = TRUE;
	}
      if (IS_SET (ch->pcdata->guilds, GUILD_RANGER))
	{
	  if (prev)
	    strcat (bf, ", ");
	  strcat (bf, "Ranger's Guild");
	  prev = TRUE;
	}
      if (IS_SET (ch->pcdata->guilds, GUILD_ASSASSIN))
	{
	  if (prev)
	    strcat (bf, ", ");
	  strcat (bf, "Assassin's Guild");
	  prev = TRUE;
	}
      if (IS_SET (ch->pcdata->guilds, GUILD_MARAUDER))
        {
          if (prev)
            strcat (bf, ", ");
          strcat (bf, "Marauder's Guild");
          prev = TRUE;
        }
      if (IS_SET (ch->pcdata->guilds, GUILD_NECROMANCER))
        {
          if (prev)
            strcat (bf, ", ");
          strcat (bf, "Necromancer's Guild");
          prev = TRUE;
        }
      if (IS_SET (ch->pcdata->guilds, GUILD_BATTLEMAGE))
        {
          if (prev)
            strcat (bf, ", ");
          strcat (bf, "Battlemage's Guild");
          prev = TRUE;
        }
      if (IS_SET (ch->pcdata->guilds, GUILD_ELEMENTAL))
        {
          if (prev)
            strcat (bf, ", ");
          strcat (bf, "Elemental's Guild");
          prev = TRUE;
        }
      if (IS_SET (ch->pcdata->guilds, GUILD_PALADIN))
        {
          if (prev)
            strcat (bf, ", ");
          strcat (bf, "Paladin's Guild");
          prev = TRUE;
        }
      strcat (bf, "\x1B[37;0m.");
    }
  else if (!str_infix ("wim", code))
    {
      sprintf (bf, "%d", (!ch->ced ? 0 : ch->ced->wimpy));
    }
  else if (!str_infix ("lvl", code))
    {
      sprintf (bf, "%d", LEVEL (ch));
    }
//KILITH
  /*else if (!str_infix ("fi", code) && IS_SET (ch->pcdata->act2, PLR_PROMPT_UP) 
&& (IS_SET (ch->pcdata->act3, ACT3_MAPPING) && !IS_SET (ch->pcdata->act3, ACT3_DISABLED)))
    return bf;*/
  else if (!str_infix ("fi", code))
    {
      if (!FIGHTING (ch))
	bf[0] = '\0';
      else
	sprintf (bf, "\x1B[0;36m[Oppnt: %s\x1B[0;36m]\x1B[37;0m ", capitalize (STRING_HITS (FIGHTING (ch))));
    }
  /*else if (!str_infix ("tk", code) && IS_SET (ch->pcdata->act2, PLR_PROMPT_UP)
	   && (IS_SET (ch->pcdata->act3, ACT3_MAPPING) && !IS_SET (ch->pcdata->act3, ACT3_DISABLED)))
    return bf;*/
  else if (!str_infix ("tk", code))
    {
      if (!FIGHTING (ch) || !FIGHTING (FIGHTING (ch))
	  || FIGHTING (FIGHTING (ch)) == ch)
	bf[0] = '\0';
      else
	sprintf (bf, "\x1B[0;36m[\x1B[1;37m%s: %s\x1B[0;36m]\x1B[37;0m ", NAME (FIGHTING (FIGHTING (ch))),
		 capitalize (STRING_HITS (FIGHTING (FIGHTING (ch)))));
    }
  else if (!str_infix ("hn", code))
    {
      sprintf (bf, "%d", ch->hit);
    }
  else if (!str_infix ("cv", code))
    {
      if (LEVEL (ch) > 100)
	sprintf (bf, "%d", ch->in_room->vnum);
      else
	bf[0] = '\0';
    }
  else if (!str_infix ("hm", code))
    {
      sprintf (bf, "%d", ch->max_hit);
    }
  else if (!str_infix ("al", code))
    {
      sprintf (bf, "%d", ALIGN (ch));
    }
  else if (!str_infix ("hw", code))
    {
      sprintf (bf, "%s", capitalize (STRING_HITS (ch)));
    }
  else if (!str_infix ("mn", code))
    {
      sprintf (bf, "%d", ch->move);
    }
  else if (!str_infix ("mm", code))
    {
      sprintf (bf, "%d", ch->max_move);
    }
  else if (!str_infix ("mw", code))
    {
      sprintf (bf, "%s", capitalize (STRING_MOVES (ch)));
    }
  else if (!str_infix ("pt", code))
    {
      int hours = ((ch->pcdata->played) +  (((int) (current_time - ch->pcdata->logon)))) / 3600;
      if (hours < 24)
	sprintf (bf, "(%d Hour%s)", hours, (hours != 1 ? "s" : ""));
      else if (hours < 720)
	sprintf (bf, "(%d Day%s, %d Hour%s)",
		 hours / 24, ((hours / 24) > 1 ? "s" : ""), hours % 24,
		 ((hours % 24) != 1 ? "s" : ""));
      else
	sprintf (bf, "(%d Month%s, %d Day%s, %d Hour%s)", 
                 hours / 720, ((hours / 720) != 1 ? "s" : ""),
		 ((hours % 720) / 24),
		 (((hours % 720) / 24) != 1 ? "s" : ""),
		 ((hours % 720) % 24),
		 (((hours % 720) % 24) != 1 ? "s" : ""));
    }
  else if (!str_infix ("ap", code))
    {
      if (!ch->ced)
	sprintf (bf, "0");
      else
	sprintf (bf, "%d",
		 GET_DAMROLL (ch) +
		 (IS_SET (ch->ced->fight_ops, F_POWER) ? 1 : 0) +
		 (IS_SET (ch->ced->fight_ops, F_AGGRESSIVE) ? 2 : 0));
    }
  else if (!str_infix ("ob", code))
    {
      int bonushit, offensive;
      bonushit = 0;
      if (!ch->ced)
	sprintf (bf, "0");
      else
	{
	  SINGLE_OBJECT *objt;
	  if ((objt = get_eq_char (ch, WEAR_HOLD_1)) != NULL)
	    {
	      if (objt->pIndexData->item_type == ITEM_WEAPON)
		{
		  bonushit = addoffense (ch, objt);
		}
	      else if (((objt = get_eq_char (ch, WEAR_HOLD_2)) != NULL)
		       && (bonushit == 0))
		{
		  if (objt->pIndexData->item_type == ITEM_WEAPON)
		    {
		      bonushit = addoffense (ch, objt);
		    }
		}
	    }
	  offensive =
	    bonushit + GET_HITROLL (ch) + (LEVEL (ch) / 20) +
	    get_curr_dex (ch) +
	    (IS_SET (ch->ced->fight_ops, F_POWER) ? 0 : 2);
	  sprintf (bf, "%d", offensive);
	}
    }
  else if (!str_infix ("eb", code))
    {
      int evasion;
      if (!ch->ced)
	sprintf (bf, "0");
      else
	{
	  evasion =
	    get_curr_dex (ch) + (LEVEL (ch) / 5) +
	    (is_member (ch, GUILD_THIEFG) ? 5 : 0) +
	    (ch->pcdata->learned[gsn_parry] ==
	     -100 ? 0 : ch->pcdata->learned[gsn_parry] / 7) +
	    (ch->pcdata->learned[gsn_dodge] ==
	     -100 ? 0 : ch->pcdata->learned[gsn_dodge] / 6) +
	    (ch->pcdata->learned[gsn_shield_block] ==
	     -100 ? 0 : ch->pcdata->learned[gsn_shield_block] / 9) +
	    (IS_SET (ch->ced->fight_ops, F_AGGRESSIVE) ? 0 : 2);
	  sprintf (bf, "%d", evasion);
	}
    }
  else if (!str_infix ("wm", code))
    {
      sprintf (bf, "%s", capitalize (string_mana (mana_now (ch))));
    }
  else if (!str_infix ("kw", code))
    {
      sprintf (bf, "%d", ch->pcdata->carry_weight);
    }
  else if (!str_infix ("cr", code))
    {
      sprintf (bf, "%d", ch->pcdata->carry_number);
    }
  else if (!str_infix ("ww", code))
    {
      if (ch->pcdata->carry_weight < (get_curr_str (ch)))
	sprintf (bf, "You are unburdened.");
      else if (ch->pcdata->carry_weight < (get_curr_str (ch) * 3))
	sprintf (bf, "No problem carrying this weight.");
      else if (ch->pcdata->carry_weight < (get_curr_str (ch) * 4))
	sprintf (bf, "A tad heavy, but you will manage.");
      else if (ch->pcdata->carry_weight < (get_curr_str (ch) * 5))
	sprintf (bf, "Heavy, but not unbearable.");
      else if (ch->pcdata->carry_weight < (get_curr_str (ch) * 6))
	sprintf (bf, "VERY HEAVY! Each step hurts.");
      else
	sprintf (bf, "You are trying to forget the pain!");
    }
  else if (!str_infix ("cn", code))
    {
      sprintf (bf, "%d", (ch->gold * 100) + ch->copper);
    }
  else if (!str_infix ("cb", code))
    {
      sprintf (bf, "%ld", ch->pcdata->bank);
    }
  else if (!str_infix ("ht", code))
    {
      sprintf (bf, "%d' %d\"", ch->height / 12, ch->height % 12);
    }
  else if (!str_infix ("tc", code))
    {
      sprintf (bf, "%d", ch->pcdata->totalkills);
    }
  else if (!str_infix ("kp", code))
    {
      sprintf (bf, "%d", ch->pcdata->killpoints);
    }
  else if (!str_infix ("wp", code))
    {
      sprintf (bf, "%d", ch->pcdata->warpoints);
    }
  else if (!str_infix ("sav", code))
    {
      sprintf (bf, "%d", ch->pcdata->saving_throw);
    }
 else if (!str_infix ("kd", code))
    {
      sprintf (bf, "%d",  ch->pcdata->plus_kick);
    }
 else if (!str_infix ("mgc", code))
    {
      sprintf (bf, "%d",  ch->pcdata->plus_magic);
    }
 else if (!str_infix ("heal", code))
    {
      sprintf (bf, "%d",  ch->pcdata->plus_heal);
    }
 else if (!str_infix ("bsd", code))
    {
      sprintf (bf, "%d",  ch->pcdata->plus_bs);
    }
  else if (!str_infix ("tp", code))
    {
      sprintf (bf, "%d", ch->pcdata->tps);
    }
  else if (!str_infix ("xp", code))
    {
      sprintf (bf, "%ld", ch->exp);
    }
  else if (!str_infix ("xl", code))
    {
      sprintf (bf, "%ld", ((FIND_EXP (LEVEL (ch), ch->race)) - ch->exp));
    }
  else if (!str_infix ("tl", code))
    {
      sprintf (bf, "%d", UMAX (0, ((FIND_TPS (LEVEL (ch), 0)) - ch->pcdata->tps)));
    }
  else if (!str_infix ("n", code))
    {
      sprintf (bf, "%s", NAME (ch));
    }
  else if (!str_infix ("r", code))
    {
      sprintf (bf, "%s", race_info[ch->pcdata->race].name);
    }
  else if (!str_infix ("t", code))
    {
      sprintf (bf, "%s", ch->pcdata->title);
    }
  else if (!str_infix ("a", code))
    {
      sprintf (bf, "%d", get_age (ch));
    }
  else if (!str_infix ("g", code))
    {
      int i2, j;
      char *t;
      i2 = 0;
      for (t = line + (strlen (line)); t != line && *t != '\r' && *t != '\n';
	   t--)
	{
	  i2++;
	}
      for (j = pad - i2; j > 0; j--)
	{
	  strcat (bf, " ");
	  if (strlen (bf) > 80)
	    break;
	}
      return bf;
    }
  sl = strlen (bf);
  if (pad > sl)
    {
      for (i = pad - sl; i > 0; i--)
	{
	  strcat (bf, " ");
	  if (strlen (bf) > 80)
	    break;
	}
    }

  if (pad > 1 && pad < sl)
    {
      bf[pad] = '\0';
    }

  return bf;
}
