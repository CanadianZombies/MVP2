#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"

bool wwrap = TRUE;

char *
string_mana_p (int zmana)
{
  static char ret[500];
  ret[0] = '\0';
  if (zmana < 2)
    sprintf (ret, "Empty");
  else if (zmana < 11)
    sprintf (ret, "Icy");
  else if (zmana < 26)
    sprintf (ret, "Cold");
  else if (zmana < 84)
    sprintf (ret, "Warm");
  else if (zmana < 121)
    sprintf (ret, "Hot");
  else
    sprintf (ret, "Burning");
  return ret;
}

char *
string_mana (int zmana)
{
  static char ret[500];
  ret[0] = '\0';
  if (zmana < 2)
    sprintf (ret, "Empty");
  else if (zmana < 11)
    sprintf (ret, "Nearly Empty");
  else if (zmana < 26)
    sprintf (ret, "Some Energy");
  else if (zmana < 40)
    sprintf (ret, "Moderate Energy");
  else if (zmana < 67)
    sprintf (ret, "Powerful");
  else if (zmana < 92)
    sprintf (ret, "Very Powerful");
  else if (zmana < 114)
    sprintf (ret, "Extremely Powerful");
  else if (zmana < 144)
    sprintf (ret, "Sparking with Energy");
  else if (zmana < 195)
    sprintf (ret, "Extreme Magical Presence");
  else
    sprintf (ret, "Incredible Magical Presence");
  return ret;
}

void
do_weight (CHAR_DATA * ch, char *argy)
{
  char buf[STD_LENGTH];
  DEFINE_COMMAND ("weight", do_weight, POSITION_DEAD, 0, LOG_NORMAL, "This command will show you a summary of how much weight you are carrying, and how hard it is to carry this weight.")
    sprintf (buf, "\x1B[37;0mYou are carrying %d items (Weight: %d kg) - ", ch->pcdata->carry_number, ch->pcdata->carry_weight);
  send_to_char (buf, ch);
  if (ch->pcdata->carry_weight < (get_curr_str (ch)))
    sprintf (buf, "You are unburdened.\n\r");
  else if (ch->pcdata->carry_weight < (get_curr_str (ch) * 2))
    sprintf (buf, "No problem carrying this weight.\n\r");
  else if (ch->pcdata->carry_weight < (get_curr_str (ch) * 3))
    sprintf (buf, "A tad heavy, but you will manage.\n\r");
  else if (ch->pcdata->carry_weight < (get_curr_str (ch) * 4))
    sprintf (buf, "Heavy, but not unbearable.\n\r");
  else if (ch->pcdata->carry_weight < (get_curr_str (ch) * 5))
    sprintf (buf, "VERY HEAVY! Each step hurts.\n\r");
  else
    sprintf (buf, "You are trying to forget the pain!\n\r");
  send_to_char (buf, ch);
  return;
}

/*void
do_effect (CHAR_DATA * ch, char *argy)
{
  DEFINE_COMMAND ("effect", do_affect, POSITION_DEAD, 0, LOG_NORMAL, "This command shows what current affects your character has.")
    return;
}*/
void
do_affect (CHAR_DATA * ch, char *argy)
{
  CHAR_DATA *tvict;
  AFFECT_DATA *paf;
  char buf[STD_LENGTH];
  SPELL_DATA *spell;
  bool enlarge_yn;
  DEFINE_COMMAND ("affect", do_affect, POSITION_DEAD, 0, LOG_NORMAL, "This command shows what current affects your character has.")
    enlarge_yn = FALSE;
  if (!argy || argy[0] == '\0' || LEVEL (ch) < 110)
    tvict = ch;
  else if ((tvict = get_char_world (ch, argy)) == NULL)
    tvict = ch;
  if (tvict->affected != NULL ||
      ch->pcdata->condition[COND_FULL] < 10 ||
      ch->pcdata->condition[COND_THIRST] < 10 ||
      ch->pcdata->condition[COND_DRUNK] > 0 || IS_AFFECTED (ch, AFF_HIDE))
    {
      send_to_char ("\x1B[37;1m---> \x1B[34mCurrent Affects \x1B[37m<---\x1B[37;0m\n\r\n\r", ch);
      for (paf = tvict->affected; paf != NULL; paf = paf->next)
	{
	  if (!(spell = skill_lookup (NULL, paf->type)) && paf->type)
	    continue;
	  sprintf (buf, "\x1B[1;31m%s\x1B[37;0m", capitalize (spell->spell_name));
	  send_to_char (buf, ch);
	  if (IS_MOB (ch) || (LEVEL (ch) > 100))
	    {
	      if (paf->location == APPLY_NONE && !paf->bitvector
		  && !paf->bitvector2)
		{
		  sprintf (buf, " (no affects)\n\r");
		}
	      else
		if ((paf->location == APPLY_NONE) &&
		    (paf->bitvector || paf->bitvector2))
		{
		  if (!paf->bitvector2)
		    {
      sprintf (buf, " \x1B[1;30m(\x1B[0;36msets bits \x1B[1;31m%s \x1B[0;36mfor \x1B[1;31m%d \x1B[0;36mmud hours\x1B[1;30m)\x1B[37;0m\n\r",
			       affect_bit_name (paf->bitvector), paf->duration);
		    }
		  else if (!paf->bitvector)
		    {
      sprintf (buf, "  \x1B[1;30m(\x1B[0;36msets bits \x1B[1;31m%s \x1B[0;36mfor \x1B[1;31m%d \x1B[0;36mmud hours\x1B[1;30m)\x1B[37;0m\n\r",
			       affect_bit_name_two (paf->bitvector2),
			       paf->duration);
		    }
		  else
		    {
		      sprintf (buf,
       " \x1B[1;30m(\x1B[0;36msets bits \x1B[1;31m%s  \x1B[0;36mand \x1B[1;31m%s \x1B[0;36mfor \x1B[1;31m%d \x1B[0;36mmud hours\x1B[1;30m)\x1B[37;0m\n\r",
			       affect_bit_name (paf->bitvector),
			       affect_bit_name_two (paf->bitvector2),
			       paf->duration);
		    }
		}
	      else
		{
		  if (!paf->bitvector && !paf->bitvector2)
		    {
		      sprintf (buf,
 " \x1B[1;30m(\x1B[0;36mmodifies \x1B[1;31m%s \x1B[0;36mby \x1B[1;31m%d \x1B[0;36mfor \x1B[1;31m%d \x1B[0;36mmud hours\x1B[1;30m)\x1B[37;0m\n\r",
			       affect_loc_name (paf->location), paf->modifier,
			       paf->duration);
		    }
		  else if (!paf->bitvector2)
		    {
		      sprintf (buf,
 " \x1B[1;30m(\x1B[0;36mmodifies \x1B[1;31m%s \x1B[0;36mby \x1B[1;31m%d \x1B[0;36mand sets bits \x1B[1;31m%s \x1B[0;36mfor \x1B[1;31m%d \x1B[0;36mmud hours\x1B[1;30m)\x1B[37;0m\n\r",
			       affect_loc_name (paf->location), paf->modifier,
			       affect_bit_name (paf->bitvector),
			       paf->duration);
		    }
		  else if (!paf->bitvector)
		    {
		      sprintf (buf,
 " \x1B[1;30m(\x1B[0;36mmodifies \x1B[1;31m%s \x1B[0;36mby \x1B[1;31m%d \x1B[0;36mand sets bits \x1B[1;31m%s \x1B[0;36mfor \x1B[1;31m%d \x1B[0;36mmud hours\x1B[1;30m)\x1B[37;0m\n\r",
			       affect_loc_name (paf->location), paf->modifier,
			       affect_bit_name_two (paf->bitvector2),
			       paf->duration);
		    }
		  else
		    {
		      sprintf (buf,
 " \x1B[1;30m(\x1B[0;36mmodifies \x1B[1;31m%s \x1B[0;36mby \x1B[1;31m%d \x1B[0;36mand sets bits \x1B[1;31m%s \x1B[0;36mand \x1B[1;31m%s \x1B[0;36mfor \x1B[1;31m%d \x1B[0;36mmud hours)\n\r",
			       affect_loc_name (paf->location), paf->modifier,
			       affect_bit_name (paf->bitvector),
			       affect_bit_name_two (paf->bitvector2),
			       paf->duration);
		    }
		}
	    }
	  else
	    {
	      if (paf->duration >= 72)
		sprintf (buf, "\x1B[0;37m, \x1B[1;34mwhich won't wear off for many days!\x1B[37;0m\n\r");
	      else if (paf->duration >= 48)
		sprintf (buf, "\x1B[0;37m, \x1B[1;37mwhich will wear off in a day or two.\x1B[37;0m\n\r");
	      else if (paf->duration >= 24)
		sprintf (buf, "\x1B[0;37m, \x1B[0;37mwhich will wear off in about a day.\x1B[37;0m\n\r");
	      else if (paf->duration >= 12)
		sprintf (buf, "\x1B[0;37m, \x1B[1;36mwhich will wear off in about half a day.\x1B[37;0m\n\r");
	      else if (paf->duration >= 6)
		sprintf (buf, "\x1B[0;37m, \x1B[0;36mwhich will wear off in about a quarter of a day.\x1B[37;0m\n\r");
	      else if (paf->duration >= 3)
		sprintf (buf, "\x1B[0;37m, \x1B[1;31mwhich will wear off in a few hours.\x1B[37;0m\n\r");
	      else if (paf->duration > 1)
		sprintf (buf, "\x1B[0;37m, \x1B[0;31mwhich will wear off in one or two hours.\x1B[37;0m\n\r");
	      else if (paf->duration == 1)
		sprintf (buf, "\x1B[0;37m, \x1B[1;30mwhich will wear off very soon.\x1B[37;0m\n\r");
	      else
		sprintf (buf, "\x1B[0;37m, \x1B[1;30mwhich will wear off at any moment.\x1B[37;0m\n\r");
	    }
	  send_to_char (buf, ch);
	}
      if (IS_AFFECTED (ch, AFF_HIDE))
	{
	  send_to_char ("\x1B[1;30mYou are hiding.\x1B[37;0m\n\r", ch);
	}
      if (ch->pcdata->condition[COND_FULL] < 10)
	{
	  switch (ch->pcdata->condition[COND_FULL])
	    {
	    case 9:
	      send_to_char ("\x1B[0;36mYou are a bit \x1B[0;33mhungry.\x1B[37;0m\n\r", ch);
	      break;
	    case 8:
	      send_to_char ("\x1B[0;36mYou are somewhat \x1B[0;33mhungry.\x1B[37;0m\n\r", ch);
	      break;
	    case 7:
	      send_to_char ("\x1B[0;36mYou are rather \x1B[0;33mhungry.\x1B[37;0m\n\r", ch);
	      break;
	    case 6:
	      send_to_char ("\x1B[0;36mYou are quite \x1B[0;33mhungry.\x1B[37;0m\n\r", ch);
	      break;
	    case 5:
	      send_to_char ("\x1B[0;36mYou are \x1B[0;33mhungry.\x1B[37;0m\n\r", ch);
	      break;
	    case 4:
	      send_to_char ("\x1B[0;36mYou are \x1B[0;31mvery \x1B[0;33mhungry.\x1B[37;0m\n\r", ch);
	      break;
	    case 3:
	      send_to_char ("\x1B[0;36mYou are \x1B[0;31mextremely \x1B[0;33mhungry.\x1B[37;0m\n\r", ch);
	      break;
	    case 2:
	      send_to_char ("\x1B[0;36mYou are \x1B[0;31mweak \x1B[0;36mfrom \x1B[0;33mhunger.\x1B[37;0m\n\r", ch);
	      break;
	    case 1:
	      send_to_char ("\x1B[0;36mYou are \x1B[0;31mextremely weak \x1B[0;36mfrom \x1B[0;33mhunger.\x1B[37;0m\n\r", ch);
	      break;
	    default:
	      send_to_char ("\x1B[0;33mYou are starving to death.\x1B[37;0m\n\r", ch);
	      break;
	    }
	}
      if (ch->pcdata->condition[COND_THIRST] < 10)
	{
	  switch (ch->pcdata->condition[COND_THIRST])
	    {
	    case 9:
	      send_to_char ("\x1B[0;36mYou are a bit \x1B[1;34mthirsty.\x1B[37;0m\n\r", ch);
	      break;
	    case 8:
	      send_to_char ("\x1B[0;36mYou are somewhat \x1B[1;34mthirsty.\x1B[37;0m\n\r", ch);
	      break;
	    case 7:
	      send_to_char ("\x1B[0;36mYou are rather \x1B[1;34mthirsty.\x1B[37;0m\n\r", ch);
	      break;
	    case 6:
	      send_to_char ("\x1B[0;36mYou are quite \x1B[1;34mthirsty.\x1B[37;0m\n\r", ch);
	      break;
	    case 5:
	      send_to_char ("\x1B[0;36mYou are \x1B[1;34mthirsty.\x1B[37;0m\n\r", ch);
	      break;
	    case 4:
	      send_to_char ("\x1B[0;36mYou are very \x1B[1;34mthirsty.\x1B[37;0m\n\r", ch);
	      break;
	    case 3:
	      send_to_char ("\x1B[0;36mYou are \x1B[0;31mextremely \x1B[1;34mthirsty.\x1B[37;0m\n\r", ch);
	      break;
	    case 2:
	      send_to_char ("\x1B[0;36mYou are \x1B[0;31mweak \x1B[0;36mfrom \x1B[1;34mthirst.\x1B[37;0m\n\r", ch);
	      break;
	    case 1:
	      send_to_char ("\x1B[0;36mYou are \x1B[0;31mextremely weak \x1B[0;36mfrom \x1B[1;34mthirst.\x1B[37;0m\n\r", ch);
	      break;
	    default:
	      send_to_char ("\x1B[0;31mYou are dying of thirst.\x1B[37;0m\n\r", ch);
	      break;
	    }
	}
      if (ch->pcdata->condition[COND_DRUNK] > 0)
	{
	  if (ch->pcdata->condition[COND_DRUNK] < 5)
	    send_to_char ("You feel tipsy.\n\r", ch);
	  else if (ch->pcdata->condition[COND_DRUNK] < 10)
	    send_to_char ("You are slightly intoxicated.\n\r", ch);
	  else if (ch->pcdata->condition[COND_DRUNK] < 15)
	    send_to_char ("You are drunk.\n\r", ch);
	  else if (ch->pcdata->condition[COND_DRUNK] < 20)
	    send_to_char ("You are piss DruNk.\n\r", ch);
	  else
	    send_to_char ("You have too little blood in your alcohol!\n\r", ch);
	}
      send_to_char ("\n\r", ch);
      return;
    }
  else
    {
      send_to_char ("\x1B[37;1m---> \x1B[34mCurrent Affects \x1B[37m<---\x1B[37;0m\n\r\n\r", ch);
      send_to_char ("\x1B[1;32mNo affects present.\x1B[37;0m\n\r", ch);
      return;
    }
  return;
}

void
do_effect (CHAR_DATA * ch, char *argy)
{
  DEFINE_COMMAND ("effect", do_affect, POSITION_DEAD, 0, LOG_NORMAL, "This command shows what current affects your character has.")
    return;
}


int
rating (CHAR_DATA * ch)
{
  int h;
  int i;
  int topteny;
  int wdam;
  int bonushit;
  SINGLE_OBJECT *w_one;
  SINGLE_OBJECT *objt;
  topteny = 0;
  h = 0;
  wdam = 0;
  bonushit = 0;
  if (IS_MOB (ch))
    return 0;
  check_ced (ch);
  if ((w_one = get_item_held (ch, ITEM_WEAPON)) == NULL)
    wdam = 1;
  else
    wdam = (FIRSTDICE (w_one) * SECONDDICE (w_one)) / 2;
  if ((objt = get_eq_char (ch, WEAR_HOLD_1)) != NULL)
  {
    if (objt->pIndexData->item_type == ITEM_WEAPON)
    {
      bonushit = addoffense (ch, objt);
    }
    else if (((objt = get_eq_char (ch, WEAR_HOLD_2)) != NULL)
        && (bonushit == 0))
      if (objt->pIndexData->item_type == ITEM_WEAPON)
      {
        bonushit = addoffense (ch, objt);
      }
  }
  topteny = 0;
  for (i = 0; i < MAX_TOPTEN; i++)
  {
    if (!str_cmp (NAME (ch), topten.topten_name[i]))
    {
      topteny = 30;
      break;
    }
  }
  h = ((100 - GET_AC (ch)) / 3) +
      get_curr_str (ch) +
      get_curr_con (ch) +
      get_curr_dex (ch) +
      get_curr_int (ch) +
      get_curr_wis (ch) +
      (wdam * 6) + (ch->max_hit / 40) + topteny +
      (max_mana (ch) / 4) +
      ((LEVEL (ch) * 3) / 2) +
      (ch->pcdata->learned[gsn_parry] ==
       -100 ? 0 : ch->pcdata->learned[gsn_parry] / 18) +
      (ch->pcdata->learned[gsn_dodge] ==
       -100 ? 0 : ch->pcdata->learned[gsn_dodge] / 18) +
      ((bonushit + GET_HITROLL (ch) + (LEVEL (ch) / 20) +
           (IS_SET(ch->ced->fight_ops, F_POWER) ? 0 : 2)) / 4) +
      ((GET_DAMROLL (ch) * 2) / 3)+
      (IS_SET (ch->ced->fight_ops, F_POWER) ? 1 : 0) +
      (IS_SET (ch->ced->fight_ops, F_AGGRESSIVE) ? 2 : 0) -
      ch->pcdata->deaths;
  return h;
}

int
get_perm_stat (CHAR_DATA * ch, int st)
{
  // Should be impossible, but you never know
  if (IS_MOB (ch))
    return number_range (3, MAX_STAT);

  if (IS_IMMORTAL (ch) /*|| pow.guilds_break_max_stats */ )
  {
    switch (st)
    {
      case STR_I:
        return (UMIN (ch->pcdata->perm_stat[STR_I], MAX_STAT));
      case CON_I:
        return (UMIN (ch->pcdata->perm_stat[CON_I], MAX_STAT));
      case DEX_I:
        return (UMIN (ch->pcdata->perm_stat[DEX_I], MAX_STAT));
      case INT_I:
        return (UMIN (ch->pcdata->perm_stat[INT_I], MAX_STAT));
      case WIS_I:
        return (UMIN (ch->pcdata->perm_stat[WIS_I], MAX_STAT));
    }
    return 15;
  }
  switch (st)
  {
    case STR_I:
      return URANGE (1, ch->pcdata->perm_stat[STR_I],
          race_info[ch->pcdata->race].limits[STR_I]);
    case CON_I:
      return URANGE (1, ch->pcdata->perm_stat[CON_I],
          race_info[ch->pcdata->race].limits[CON_I]);
    case DEX_I:
      return URANGE (1, ch->pcdata->perm_stat[DEX_I],
          race_info[ch->pcdata->race].limits[DEX_I]);
    case INT_I:
      return URANGE (1, ch->pcdata->perm_stat[INT_I],
          race_info[ch->pcdata->race].limits[INT_I]);
    case WIS_I:
      return URANGE (1, ch->pcdata->perm_stat[WIS_I],
          race_info[ch->pcdata->race].limits[WIS_I]);
  }
  return 15;
}

/*
   The attribute command
 */
void
do_attribute (CHAR_DATA * ch, char *argy)
{
  int position;
  char buf[STD_LENGTH];
  DEFINE_COMMAND ("attribute", do_attribute, POSITION_DEAD, 0, LOG_NORMAL, "This command shows you various info about your character.")
    if (ch->in_room && ch->in_room->vnum >= 400 && ch->in_room->vnum < 500)
    {
      send_to_char
        ("Your stats have not been determined yet; you must finish character creation.\n\r", ch);
      return;
    }
  if (pow.professions && LEVEL (ch) < 20)
  {
    send_to_char
      ("You may see a full description of your stats at level 20.\n\r", ch);
    return;
  }
  if (LEVEL (ch) < pow.level_to_see_numeric_stats)
  {
    if (IS_MOB (ch))
      return;
    if (ch->in_room && ch->in_room->vnum > 440 && ch->in_room->vnum < 600)
    {
      send_to_char
        ("You can not 'attribute' during character creation.\n\r", ch);
      return;
    }
    position = ch->position;
    ch->position = POSITION_STANDING;
    act ("$B$2+$1---------------------------------------$2+", ch, NULL,
        NULL, TO_CHAR);
    sprintf (buf, "\x1B[37;0m Strength      : $2$B%-10s\x1B[37;0m",
        str_name (get_curr_str (ch)));
    act (buf, ch, NULL, NULL, TO_CHAR);
    sprintf (buf, "\x1B[37;0m Constitution  : $4$B%-10s\x1B[37;0m",
        con_name (get_curr_con (ch)));
    act (buf, ch, NULL, NULL, TO_CHAR);
    sprintf (buf, "\x1B[37;0m Dexterity     : $5$B%-10s\x1B[37;0m",
        dex_name (get_curr_dex (ch)));
    act (buf, ch, NULL, NULL, TO_CHAR);
    sprintf (buf, "\x1B[37;0m Intelligence  : $7$B%-10s\x1B[37;0m",
        int_name (get_curr_int (ch)));
    act (buf, ch, NULL, NULL, TO_CHAR);
    sprintf (buf, "\x1B[37;0m Wisdom        : $1$B%-10s\x1B[37;0m",
        wis_name (get_curr_wis (ch)));
    act (buf, ch, NULL, NULL, TO_CHAR);
    act ("$B$2+$1---------------------------------------$2+", ch, NULL,
        NULL, TO_CHAR);
    send_to_char ("\x1B[37;0m", ch);
    ch->position = position;
  }
  else
  {
    position = ch->position;
    ch->position = POSITION_STANDING;
    act ("$B$2+$1---------------------------------------$2+", ch, NULL,
        NULL, TO_CHAR);
    sprintf (buf,
        "\x1B[37;0m Strength      : \x1B[%d;1m%2d\x1B[37;0m/%2d\x1B[37;0m",
        (get_curr_str (ch) >
         get_perm_stat (ch,
           STR_I)) ? 32 : ((get_curr_str (ch) ==
             get_perm_stat (ch,
               STR_I)) ? 37 :
           31), get_curr_str (ch),
        get_perm_stat (ch, STR_I));
    act (buf, ch, NULL, NULL, TO_CHAR);
    sprintf (buf,
        "\x1B[37;0m Constitution  : \x1B[%d;1m%2d\x1B[37;0m/%2d\x1B[37;0m",
        (get_curr_con (ch) >
         get_perm_stat (ch,
           CON_I)) ? 32 : ((get_curr_con (ch) ==
             get_perm_stat (ch,
               CON_I)) ? 37 :
           31), get_curr_con (ch),
        get_perm_stat (ch, CON_I));
    act (buf, ch, NULL, NULL, TO_CHAR);
    sprintf (buf,
        "\x1B[37;0m Dexterity     : \x1B[%d;1m%2d\x1B[37;0m/%2d\x1B[37;0m",
        (get_curr_dex (ch) >
         get_perm_stat (ch,
           DEX_I)) ? 32 : ((get_curr_dex (ch) ==
             get_perm_stat (ch,
               DEX_I)) ? 37 :
           31), get_curr_dex (ch),
        get_perm_stat (ch, DEX_I));
    act (buf, ch, NULL, NULL, TO_CHAR);
    sprintf (buf,
        "\x1B[37;0m Intelligence  : \x1B[%d;1m%2d\x1B[37;0m/%2d\x1B[37;0m",
        (get_curr_int (ch) >
         get_perm_stat (ch,
           INT_I)) ? 32 : ((get_curr_int (ch) ==
             get_perm_stat (ch,
               INT_I)) ? 37 :
           31), get_curr_int (ch),
        get_perm_stat (ch, INT_I));
    act (buf, ch, NULL, NULL, TO_CHAR);
    sprintf (buf,
        "\x1B[37;0m Wisdom        : \x1B[%d;1m%2d\x1B[37;0m/%2d\x1B[37;0m",
        (get_curr_wis (ch) >
         get_perm_stat (ch,
           WIS_I)) ? 32 : ((get_curr_wis (ch) ==
             get_perm_stat (ch,
               WIS_I)) ? 37 :
           31), get_curr_wis (ch),
        get_perm_stat (ch, WIS_I));
    act (buf, ch, NULL, NULL, TO_CHAR);
    act ("$B$2+$1---------------------------------------$2+", ch, NULL,
        NULL, TO_CHAR);
    send_to_char ("\x1B[37;0m", ch);
    ch->position = position;
  }
  return;
}

/*
   109 [(Good/Evil/Neutral) Racename] NAME TITLE
   +--------------------------------------------------------------------------+
   Hitpts: 14000/14000 Movement: 10000/10000 Mana: 981/981 (24/24 natural)
   TExper: 49348394839 Exp2Levl: 49348394839 Played: 5933 hours
   TKills: 943743 Killpnts: 943843 Warpts: 99999
   InBank: 49384933 MoneyNow: 91824884 PlrAge: 94 years
   CWimpy: 25 TRemorts: 0 PagLen: 24
   Learns: 94 Practces: 500 Wanted/Not Wanted
   Hungry Thirsty Drunk
   +-------------------------------------------------------------------------+
   Bonuses: Offensive: 100 Attack Power: 100 Evasion: 100
   Str: 25 Int: 25 Wis: 25 Dex: 25 Con: 25 Alignmt: -9431
   Status: Arena/Battleground/Normal Position: Sleeping/(flying/standing)
   +-------------------------------------------------------------------------+
   You are carrying 9484kg (You are trying to forget the pain!)
   Armor Rating: You armor is superbly durable and tough!
   Member of The Mithril Knights of Xylar clan.
   Guilds: Ranger's Guild, Tinker's Guild, Assassin's Guild, Thief's Guild
   You are listening to Joe.
 */
void
do_info (CHAR_DATA * ch, char *argy)
{
  char buf[STD_LENGTH];
  int ii;
  char tmpbuf[500];
  CHAR_DATA *tvict;
  int bonushit;
  DEFINE_COMMAND ("information", do_info, POSITION_DEAD, 0, LOG_NORMAL,
		  "This command shows you an info page about your character.")
    if (LEVEL (ch) < 110 && FIGHTING (ch) != NULL
	&& !pow.score_while_fighting)
    {
      send_to_char ("You cannot info while fighting!\n\r", ch);
      return;
    }
  ansi_color (NTEXT, ch);
  bonushit = 0;
  if (argy == "" || argy[0] == '\0' || LEVEL (ch) < 110)
    tvict = ch;
  else
    {
      if ((tvict = get_char_world (ch, argy)) == NULL)
	tvict = ch;
    }
/*if (IS_MOB(tvict)) 
   {
   send_to_char("You may not INFO NPC's.\n\r",ch);
   return;
   }
 */
  check_ced (tvict);
  check_ced (ch);
  sprintf (buf,
	   "\x1B[32;1m%d \x1B[37m[%s %s\x1B[37m] \x1B[0m%s %s\x1B[37;0m\n\r",
	   LEVEL (tvict),
	   (IS_NEUTRAL (tvict) ? "\x1B[34mGood"
	    : (IS_GOOD (tvict) ? "\x1B[36mGood" : "\x1B[30mEvil")),
	   race_info[tvict->pcdata->race].name, NAME (tvict),
	   tvict->pcdata->title);
  send_to_char (buf, ch);
  send_to_char
    ("+--------------------------------------------------------------------------+\n\r",
     ch);
  sprintf (buf,
	   "\x1B[34;1m Hitpts: \x1B[37m%5d\x1B[34m/\x1B[36m%-5d  \x1B[34mMovement: \x1B[37m%5d\x1B[34m/\x1B[36m%-5d\x1B[34m   Mana: \x1B[37m%s\x1B[0m\n\r",
	   tvict->hit, tvict->max_hit, tvict->move, tvict->max_move,
	   string_mana (mana_now (tvict)));
  send_to_char (buf, ch);
  sprintf (buf,
	   "\x1B[34;1m TKills: \x1B[31m%-12d \x1B[34mKillPnts: \x1B[31m%-12d \x1B[34mWarPts: \x1B[31m%d\n\r",
	   tvict->pcdata->totalkills, tvict->pcdata->killpoints,
	   tvict->pcdata->warpoints);
  send_to_char (buf, ch);
  sprintf (buf,
	   "\x1B[34;1m InBank: \x1B[37m%-12ld \x1B[34mMoneyNow: \x1B[37m%-12d \x1B[34mPlrAge: \x1B[37m%d\n\r",
	   tvict->pcdata->bank, tally_coins (tvict), get_age (tvict));
  send_to_char (buf, ch);
  sprintf (buf,
	   "\x1B[34;1m CWimpy: \x1B[37m%-12d \x1B[34mTRemorts: \x1B[37m%-12d \x1B[34mPagLen: \x1B[37m%d\x1B[0m\n\r",
	   tvict->ced->wimpy, tvict->pcdata->remort_times,
	   tvict->pcdata->pagelen);
  send_to_char (buf, ch);
  sprintf (tmpbuf, "\x1B[34;1mHeight: \x1B[37;1m%d' %d\"\x1B[0m",
	   tvict->height / 12, tvict->height % 12);
  sprintf (buf,
	   "\x1B[34;1m Learns: \x1B[37m%-12d \x1B[34mPractces: \x1B[37m%-12d \x1B[37;0m%s\n\r",
	   tvict->pcdata->learn, tvict->pcdata->practice, tmpbuf);
  send_to_char (buf, ch);
  sprintf (buf, " %s %s %s %s\n\r",
	   (tvict->pcdata->condition[COND_FULL] <
	    7 ? "\x1B[37;1m\x1B[5mHungry\x1B[0m" : " "),
	   (tvict->pcdata->condition[COND_THIRST] <
	    7 ? "\x1B[37;1m\x1B[5mThirsty\x1B[0m" : " "),
	   (tvict->pcdata->condition[COND_DRUNK] >
	    10 ? "\x1B[37;1m\x1B[5mDrunk\x1B[0m" : " "),
	   (tvict->pcdata->bounty >
	    0 ? "\x1B[37;1mWanted Criminal\x1B[0m" : " "));
  send_to_char (buf, ch);
  send_to_char
    ("+--------------------------------------------------------------------------+\n\r",
     ch);

  if (LEVEL (ch) > 100 && ch != tvict)
    {
      sprintf (buf,
	       "\x1B[34;1m Str: \x1B[37m%-2d \x1B[34mInt: \x1B[37m%-2d \x1B[34mWis: \x1B[37m%-2d \x1B[34mDex: \x1B[37m%-2d \x1B[34mCon: \x1B[37m%-2d\x1B[37;0m Alignmt: \x1B[1m%d\n\r",
	       get_curr_str (tvict), get_curr_int (tvict),
	       get_curr_wis (tvict), get_curr_dex (tvict),
	       get_curr_con (tvict), ALIGN (tvict));
      send_to_char (buf, ch);
    }
  sprintf (buf,
	   "\x1B[36;1m You are carrying \x1B[37;1m%d\x1B[36;1mkg \x1B[37;0m(",
	   tvict->pcdata->carry_weight);
  send_to_char (buf, ch);
  if (tvict->pcdata->carry_weight < (get_curr_str (tvict)))
    sprintf (buf, "You are unburdened.)\n\r");
  else if (tvict->pcdata->carry_weight < (get_curr_str (tvict) * 2))
    sprintf (buf, "No problem carrying this weight.)\n\r");
  else if (tvict->pcdata->carry_weight < (get_curr_str (tvict) * 3))
    sprintf (buf, "A tad heavy, but you will manage.)\n\r");
  else if (tvict->pcdata->carry_weight < (get_curr_str (tvict) * 4))
    sprintf (buf, "Heavy, but not unbearable.)\n\r");
  else if (tvict->pcdata->carry_weight < (get_curr_str (tvict) * 5))
    sprintf (buf, "VERY HEAVY! Each step hurts!)\n\r");
  else
    sprintf (buf, "You are trying to forget the pain!)\n\r");
  send_to_char (buf, ch);
  ii = clan_number (tvict);
  if (ii > 0)
    {
      sprintf (buf, " \x1B[0;37mMember of \x1B[1;31m%s\x1B[0;37m clan.\n\r",
	       get_clan_index (ii)->name);
      send_to_char (buf, ch);
    }
  if (tvict->pcdata->guilds != 0)
    {
      bool prev;
      prev = FALSE;
      send_to_char ("\x1B[37;0m Guild(s): \x1B[37;1m", ch);
      if (IS_SET (tvict->pcdata->guilds, GUILD_SHAMAN))
        {
          if (prev)
            send_to_char (", ", ch);
          send_to_char ("Shaman's Guild", ch);
          prev = TRUE;
        }
      if (IS_SET (tvict->pcdata->guilds, GUILD_TATICS))
        {
          if (prev)
            send_to_char (", ", ch);
          send_to_char ("Tatics's Guild", ch);
          prev = TRUE;
        }
      if (IS_SET (tvict->pcdata->guilds, GUILD_MYSTICS))
        {
          if (prev)
            send_to_char (", ", ch);
          send_to_char ("Mystic's Guild", ch);
          prev = TRUE;
        }
      if (IS_SET (tvict->pcdata->guilds, GUILD_TINKER))
	{
          if (prev)
            send_to_char (", ", ch);
	  send_to_char ("Tinker's Guild", ch);
	  prev = TRUE;
	}
      if (IS_SET (tvict->pcdata->guilds, GUILD_WARRIOR))
	{
	  if (prev)
	    send_to_char (", ", ch);
	  send_to_char ("Warrior's Guild", ch);
	  prev = TRUE;
	}
      if (IS_SET (tvict->pcdata->guilds, GUILD_HEALER))
	{
	  if (prev)
	    send_to_char (", ", ch);
	  send_to_char ("Healer's Guild", ch);
	  prev = TRUE;
	}
      if (IS_SET (tvict->pcdata->guilds, GUILD_WIZARD))
	{
	  if (prev)
	    send_to_char (", ", ch);
	  send_to_char ("Wizard's Guild", ch);
	  prev = TRUE;
	}
      if (IS_SET (tvict->pcdata->guilds, GUILD_THIEFG))
	{
	  if (prev)
	    send_to_char (", ", ch);
	  send_to_char ("Thief's Guild", ch);
	  prev = TRUE;
	}
      if (IS_SET (tvict->pcdata->guilds, GUILD_RANGER))
	{
	  if (prev)
	    send_to_char (", ", ch);
	  send_to_char ("Ranger's Guild", ch);
	  prev = TRUE;
	}
      if (IS_SET (tvict->pcdata->guilds, GUILD_ASSASSIN))
	{
	  if (prev)
	    send_to_char (", ", ch);
	  send_to_char ("Assassin's Guild", ch);
	  prev = TRUE;
	}
      if (IS_SET (tvict->pcdata->guilds, GUILD_BATTLEMAGE))
        {
          if (prev)
            send_to_char (", ", ch);
          send_to_char ("Battlemage's Guild", ch);
          prev = TRUE;
        }
      if (IS_SET (tvict->pcdata->guilds, GUILD_MARAUDER))
        {
          if (prev)
            send_to_char (", ", ch);
          send_to_char ("Marauder's Guild", ch);
          prev = TRUE;
        }
      if (IS_SET (tvict->pcdata->guilds, GUILD_NECROMANCER))
        {
          if (prev)
            send_to_char (", ", ch);
          send_to_char ("Necromancer's Guild", ch);
          prev = TRUE;
        }
      if (IS_SET (tvict->pcdata->guilds, GUILD_ELEMENTAL))
        {
          if (prev)
            send_to_char (", ", ch);
          send_to_char ("Elemental's Guild", ch);
          prev = TRUE;
        }
      if (IS_SET (tvict->pcdata->guilds, GUILD_PALADIN))
        {
          if (prev)
            send_to_char (", ", ch);
          send_to_char ("Paladin's Guild", ch);
          prev = TRUE;
        }
      send_to_char ("\x1B[37;0m.\n\r", ch);
    }

  return;
}

/*void
do_quickscore2 (CHAR_DATA * ch, char *argy)
{
  DEFINE_COMMAND ("statistics", do_quickscore, POSITION_DEAD, 0, LOG_NORMAL,
		  "This command shows a brief summary of your character.")
    return;
}

void
do_quickscore (CHAR_DATA * ch, char *argy)
{
  int pos;
  DEFINE_COMMAND ("sco", do_quickscore, POSITION_DEAD, 0, LOG_NORMAL,
		  "This command shows a brief summary of your character.")
    if (FIGHTING (ch) && !pow.score_while_fighting)
    {
      send_to_char ("You cannot use this command during combat.\n\r", ch);
      return;
    }

  if (IS_JAVA (ch))
    {
      java_package (ch);
      return;
    }

  pos = ch->position;
  ch->position = POSITION_STANDING;
  wwrap = FALSE;
  if (sh_score != NULL)
    act (interpret_line (sh_score, ch), ch, NULL, ch, TO_CHAR);
  else
    act (interpret_line (score, ch), ch, NULL, ch, TO_CHAR);
  wwrap = TRUE;
  ch->position = pos;

  return;
}*/


void
do_vscore (CHAR_DATA * ch, char *argy)
{
  int pos;
  DEFINE_COMMAND ("vscore", do_vscore, POSITION_DEAD, 0, LOG_NORMAL,
		  "This command shows you complete information about your character.")
    if (IS_JAVA (ch))
    {
      java_package (ch);
      return;
    }
  if (FIGHTING (ch) && !pow.score_while_fighting)
    {
      send_to_char ("You cannot use this command during combat.\n\r", ch);
      return;
    }

  pos = ch->position;
  ch->position = POSITION_STANDING;
  wwrap = FALSE;
  act (interpret_line (score, ch), ch, NULL, ch, TO_CHAR);
  wwrap = TRUE;
  ch->position = pos;

  return;
}



void
do_score (CHAR_DATA * ch, char *argy)
{
  int pos;
  DEFINE_COMMAND ("score", do_score, POSITION_DEAD, 0, LOG_NORMAL, "This command shows you complete information about your character.")
    if (FIGHTING (ch) && !pow.score_while_fighting)
    {
      send_to_char ("You cannot use this command during combat.\n\r", ch);
      return;
    }

  pos = ch->position;
  ch->position = POSITION_STANDING;
  wwrap = FALSE;
  act (interpret_line (score, ch), ch, NULL, ch, TO_CHAR);
  wwrap = TRUE;
  ch->position = pos;

  return;
}

void
read_score (CHAR_DATA * ch, char *argy)
{
  FILE *ff;
  DEFINE_COMMAND ("z_reload_score", read_score, POSITION_DEAD, 110, LOG_ALWAYS, "This command rereads score.dat from disk.") 
  if (score)
    free (score);
  if (sh_score)
    free (sh_score);
  score = NULL;
  sh_score = NULL;

  if ((ff = fopen ("score.dat", "r")) == NULL)
    {
      fprintf (stderr, "Error on score.dat read.\n");
      exit (20);
    }
  else
    {
      score = str_dup (fread_string2 (ff, NULL));
      fclose (ff);
    }

  if ((ff = fopen ("sh_score.dat", "r")) == NULL)
    {
      fprintf (stderr,
	       "Error on sh_score.dat read... generalizing to score.dat.\n");
      sh_score = NULL;
    }
  else
    {
      sh_score = str_dup (fread_string2 (ff, NULL));
      fclose (ff);
    }

  return;
}
