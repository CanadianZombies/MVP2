/*Offline Player editor */
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"
char *itoa (int);

/* True is character is fine with payment, false if not. */
bool
check_hours (CHAR_DATA * ch)
{
  if (ch->pcdata->monthly_pass > 0)
    {
      if (ch->pcdata->hours_purchased < 0)
	ch->pcdata->hours_purchased = 0;
      return TRUE;
    }

  ch->pcdata->played += (int) (current_time - ch->pcdata->logon);
#ifdef BILLING
  if (ch->pcdata->monthly_pass <= 0)
    ch->pcdata->hours_purchased -= (int) (current_time - ch->pcdata->logon);
#endif
  ch->pcdata->logon = current_time;

  if (ch->pcdata->hours_purchased < ((-10) * 3600))
    return FALSE;
  return TRUE;
}

#ifdef NEW_WORLD
void
display_hours (CHAR_DATA * ch, char *argy)
{
  char buf[500];
  DEFINE_COMMAND ("credit", display_hours, POSITION_DEAD, 0, LOG_NORMAL,
		  "Checks your current credit situation.") if (ch->pcdata->
							       monthly_pass >
							       0)
    {
      sprintf (buf, "You have %d monthly pass%s.\n\r",
	       ch->pcdata->monthly_pass,
	       ((ch->pcdata->monthly_pass > 1) ? "es" : ""));
      send_to_char (buf, ch);
    }
  ch->pcdata->played += (int) (current_time - ch->pcdata->logon);
#ifdef BILLING
  if (ch->pcdata->monthly_pass <= 0)
    ch->pcdata->hours_purchased -= (int) (current_time - ch->pcdata->logon);
#endif
  ch->pcdata->logon = current_time;
  sprintf (buf, "You have %d hours and %d minutes left on hourly credit.\n\r",
	   ch->pcdata->hours_purchased / 3600,
	   (ch->pcdata->hours_purchased / 60) -
	   ((ch->pcdata->hours_purchased / 3600) * 60));
  send_to_char (buf, ch);
  if (ch->pcdata->hours_purchased < 0)
    {
      send_to_char ("(You are allowed a 10 hour grace period).\n\r", ch);
    }
  return;
}
#endif

char *
natural_abilities (int flag)
{
  static char tttb[256];
  bool prev = FALSE;
  tttb[0] = '\0';
  if (IS_SET (flag, AFF_BLIND))
    {
      strcat (tttb, "Blind");
      prev = TRUE;
    }
  if (IS_SET (flag, AFF_INVISIBLE))
    {
      if (prev)
        strcat (tttb, ", ");
      strcat (tttb, "Invisible");
      prev = TRUE;
    }
  if (IS_SET (flag, AFF_DETECT_EVIL))
    {
      if (prev)
        strcat (tttb, ", ");
      strcat (tttb, "Detect Evil");
      prev = TRUE;
    }
  if (IS_SET (flag, AFF_DETECT_INVIS))
    {
      if (prev)
        strcat (tttb, ", ");
      strcat (tttb, "Detect Invis");
      prev = TRUE;
    }
  if (IS_SET (flag, AFF_DETECT_HIDDEN))
    {
      if (prev)
        strcat (tttb, ", ");
      strcat (tttb, "Detect Hidden");
      prev = TRUE;
    }
  if (IS_SET (flag, AFF_SANCTUARY))
    {
      if (prev)
        strcat (tttb, ", ");
      strcat (tttb, "Sanctuary");
      prev = TRUE;
    }
  if (IS_SET (flag, AFF_REGENERATE))
    {
      if (prev)
        strcat (tttb, ", ");
      strcat (tttb, "Regenerate");
      prev = TRUE;
    }
  if (IS_SET (flag, AFF_INFRARED))
    {
      if (prev)
        strcat (tttb, ", ");
      strcat (tttb, "Infravision");
      prev = TRUE;
    }
  if (IS_SET (flag, AFF_CURSE))
    {
      if (prev)
	strcat (tttb, ", ");
      strcat (tttb, "Curse");
      prev = TRUE;
    }
  if (IS_SET (flag, AFF_POISON))
    {
      if (prev)
	strcat (tttb, ", ");
      strcat (tttb, "Poison");
      prev = TRUE;
    }
  if (IS_SET (flag, AFF_PROTECT))
    {
      if (prev)
        strcat (tttb, ", ");
      strcat (tttb, "Protection");
      prev = TRUE;
    }
  if (IS_SET (flag, AFF_SNEAK))
    {
      if (prev)
        strcat (tttb, ", ");
      strcat (tttb, "Sneak");
      prev = TRUE;
    }
  if (IS_SET (flag, AFF_HIDE))
    {
      if (prev)
        strcat (tttb, ", ");
      strcat (tttb, "Hide");
      prev = TRUE;
    }
  if (IS_SET (flag, AFF_SLEEP))
    {
      if (prev)
        strcat (tttb, ", ");
      strcat (tttb, "Sleep");
      prev = TRUE;
    }
 /* if (IS_SET (flag, AFF_CHARM))
    {
      if (prev)
        strcat (tttb, ", ");
      strcat (tttb, "Charm");
      prev = TRUE;
    }*/
  if (IS_SET (flag, AFF_FLYING))
    {
      if (prev)
	strcat (tttb, ", ");
      strcat (tttb, "Fly");
      prev = TRUE;
    }
  if (IS_SET (flag, AFF_PASS_DOOR))
    {
      if (prev)
        strcat (tttb, ", ");
      strcat (tttb, "Pass Door");
      prev = TRUE;
    }
  if (IS_SET (flag, AFF_SLOW))
    {
      if (prev)
        strcat (tttb, ", ");
      strcat (tttb, "Slow");
      prev = TRUE;
    }
 /* if (IS_SET (flag, AFF_PLAGUE))
    {
      if (prev)
        strcat (tttb, ", ");
      strcat (tttb, "Plague");
      prev = TRUE;
    }*/
  if (IS_SET (flag, AFF_BREATH_WATER))
    {
      if (prev)
	strcat (tttb, ", ");
      strcat (tttb, "Underwater Breath");
      prev = TRUE;
    }
  if (IS_SET (flag, AFF_PROT_EVIL))
    {
      if (prev)
        strcat (tttb, ", ");
      strcat (tttb, "Protection from Evil");
      prev = TRUE;
    }
  if (IS_SET (flag, AFF_PROT_GOOD))
    {
      if (prev)
        strcat (tttb, ", ");
      strcat (tttb, "Protection from Good");
      prev = TRUE;
    }
  if (IS_SET (flag, AFF_AWARENESS))
    {
      if (prev)
        strcat (tttb, ", ");
      strcat (tttb, "Awareness");
      prev = TRUE;
    }
  if (IS_SET (flag, AFF_WOUND))
    {
      if (prev)
        strcat (tttb, ", ");
      strcat (tttb, "Wound");
      prev = TRUE;
    }
  if (IS_SET (flag, AFF_DETECT_GOOD))
    {
      if (prev)
        strcat (tttb, ", ");
      strcat (tttb, "Detect Good");
      prev = TRUE;
    }
  if (IS_SET (flag, AFF_NOCHARM))
    {
      if (prev)
        strcat (tttb, ", ");
      strcat (tttb, "No Charm");
      prev = TRUE;
    }
  if (IS_SET (flag, AFF_SAFETY))
    {
      if (prev)
        strcat (tttb, ", ");
      strcat (tttb, "Safety");
      prev = TRUE;
    }
  /*if (IS_SET (flag, AFF_PROT_FIRE))
    {
      if (prev)
        strcat (tttb, ", ");
      strcat (tttb, "Protection from Fire");
      prev = TRUE;
    }
  if (IS_SET (flag, AFF_PROT_GAS))
    {
      if (prev)
        strcat (tttb, ", ");
      strcat (tttb, "Protection from Gas");
      prev = TRUE;
    }*/
  if (IS_SET (flag, AFF_MINDSHIELD))
    {
      if (prev)
        strcat (tttb, ", ");
      strcat (tttb, "Mindshield");
      prev = TRUE;
    }
  if (IS_SET (flag, AFF_SPEED))
    {
      if (prev)
        strcat (tttb, ", ");
      strcat (tttb, "Haste");
      prev = TRUE;
    }
  /*if (IS_SET (flag, AFF_INSANITY))
    {
      if (prev)
        strcat (tttb, ", ");
      strcat (tttb, "Insanity");
      prev = TRUE;
    }*/
  if (tttb[0] == '\0')
    strcat (tttb, "None");
  return tttb;
}

bool
check_toggle_implant (CHAR_DATA * ch, char *arg)
{
  bool ad = FALSE;
  bool sb = FALSE;
  if (arg[0] == '+')
    ad = TRUE;
  else if (arg[0] == '-')
    ad = FALSE;
#ifdef NEW_WORLD
  if (!str_cmp (arg, "ArmStrength"))
    {
      if (ad)
	ch->pcdata->implants_1 |= IMPLANT_ARMS;
      else if (sb && IS_SET (ch->pcdata->implants_1, IMPLANT_ARMS))
	{
	  ch->pcdata->implants_1 ^= IMPLANT_ARMS;
	}
      else if (!sb)
	ch->pcdata->implants_1 ^= IMPLANT_ARMS;
      return TRUE;
    }
  if (!str_cmp (arg, "LegStrength"))
    {
      if (ad)
	ch->pcdata->implants_1 |= IMPLANT_LEGS;
      else if (sb && IS_SET (ch->pcdata->implants_1, IMPLANT_LEGS))
	{
	  ch->pcdata->implants_1 ^= IMPLANT_LEGS;
	}
      else if (!sb)
	ch->pcdata->implants_1 ^= IMPLANT_LEGS;
      return TRUE;
    }
  if (!str_cmp (arg, "HandStrength"))
    {
      if (ad)
	ch->pcdata->implants_1 |= IMPLANT_HANDS;
      else if (sb && IS_SET (ch->pcdata->implants_1, IMPLANT_HANDS))
	{
	  ch->pcdata->implants_1 ^= IMPLANT_HANDS;
	}
      else if (!sb)
	ch->pcdata->implants_1 ^= IMPLANT_HANDS;
      return TRUE;
    }
  if (!str_cmp (arg, "BodyStrength"))
    {
      if (ad)
	ch->pcdata->implants_1 |= IMPLANT_BODY;
      else if (sb && IS_SET (ch->pcdata->implants_1, IMPLANT_BODY))
	{
	  ch->pcdata->implants_1 ^= IMPLANT_BODY;
	}
      else if (!sb)
	ch->pcdata->implants_1 ^= IMPLANT_BODY;
      return TRUE;
    }
  if (!str_cmp (arg, "ArmDexterity"))
    {
      if (ad)
	ch->pcdata->implants_2 |= IMPLANT_ARMS;
      else if (sb && IS_SET (ch->pcdata->implants_2, IMPLANT_ARMS))
	{
	  ch->pcdata->implants_2 ^= IMPLANT_ARMS;
	}
      else if (!sb)
	ch->pcdata->implants_2 ^= IMPLANT_ARMS;
      return TRUE;
    }
  if (!str_cmp (arg, "LegRunning"))
    {
      if (ad)
	ch->pcdata->implants_2 |= IMPLANT_LEGS;
      else if (sb && IS_SET (ch->pcdata->implants_2, IMPLANT_LEGS))
	{
	  ch->pcdata->implants_2 ^= IMPLANT_LEGS;
	}
      else if (!sb)
	ch->pcdata->implants_2 ^= IMPLANT_LEGS;
      return TRUE;
    }
  if (!str_cmp (arg, "Brain"))
    {
      if (ad)
	ch->pcdata->implants_2 |= IMPLANT_HEAD;
      else if (sb && IS_SET (ch->pcdata->implants_2, IMPLANT_HEAD))
	{
	  ch->pcdata->implants_2 ^= IMPLANT_HEAD;
	}
      else if (!sb)
	ch->pcdata->implants_2 ^= IMPLANT_HEAD;
      return TRUE;
    }
  if (!str_cmp (arg, "Feet"))
    {
      if (ad)
	ch->pcdata->implants_2 |= IMPLANT_FEET;
      else if (sb && IS_SET (ch->pcdata->implants_2, IMPLANT_FEET))
	{
	  ch->pcdata->implants_2 ^= IMPLANT_FEET;
	}
      else if (!sb)
	ch->pcdata->implants_2 ^= IMPLANT_FEET;
      return TRUE;
    }
  if (!str_cmp (arg, "HandArmor"))
    {
      if (ad)
	ch->pcdata->implants_2 |= IMPLANT_HANDS;
      else if (sb && IS_SET (ch->pcdata->implants_2, IMPLANT_HANDS))
	{
	  ch->pcdata->implants_2 ^= IMPLANT_HANDS;
	}
      else if (!sb)
	ch->pcdata->implants_2 ^= IMPLANT_HANDS;
      return TRUE;
    }
  if (!str_cmp (arg, "BodyArmor"))
    {
      if (ad)
	ch->pcdata->implants_2 |= IMPLANT_BODY;
      else if (sb && IS_SET (ch->pcdata->implants_2, IMPLANT_BODY))
	{
	  ch->pcdata->implants_2 ^= IMPLANT_BODY;
	}
      else if (!sb)
	ch->pcdata->implants_2 ^= IMPLANT_BODY;
      return TRUE;
    }
#endif
  return FALSE;
}

void
show_implants (CHAR_DATA * ch, CHAR_DATA * looker)
{
  char buf[500];
  int i;
  bool prev = FALSE;
  if (IS_MOB (ch))
    return;
  if (ch->pcdata->implants_1 == 0 && ch->pcdata->implants_2 == 0)
    return;
  sprintf (buf, "Implants: ");
  if (HAS_ARMS_STRENGTH_IMPLANT (ch))
    {
      if (prev)
	strcat (buf, ", ");
      strcat (buf, "ArmStrength");
      prev = TRUE;
    }
  if (HAS_LEGS_STRENGTH_IMPLANT (ch))
    {
      if (prev)
	strcat (buf, ", ");
      strcat (buf, "LegStrength");
      prev = TRUE;
    }
  if (HAS_HANDS_STRENGTH_IMPLANT (ch))
    {
      if (prev)
	strcat (buf, ", ");
      strcat (buf, "HandStrength");
      prev = TRUE;
    }
  if (HAS_BODY_STRENGTH_IMPLANT (ch))
    {
      if (prev)
	strcat (buf, ", ");
      strcat (buf, "BodyStrength");
      prev = TRUE;
    }
  if (HAS_ARMS_DEXTERITY_IMPLANT (ch))
    {
      if (prev)
	strcat (buf, ", ");
      strcat (buf, "ArmDexterity");
      prev = TRUE;
    }
  if (HAS_LEGS_RUNNING_IMPLANT (ch))
    {
      if (prev)
	strcat (buf, ", ");
      strcat (buf, "LegRunning");
      prev = TRUE;
    }
  if (HAS_HEAD_BRAIN_IMPLANT (ch))
    {
      if (prev)
	strcat (buf, ", ");
      strcat (buf, "Brain");
      prev = TRUE;
    }
  if (HAS_FEET_IMPLANT (ch))
    {
      if (prev)
	strcat (buf, ", ");
      strcat (buf, "Feet");
      prev = TRUE;
    }
  if (HAS_HANDS_ARMOR_IMPLANT (ch))
    {
      if (prev)
	strcat (buf, ", ");
      strcat (buf, "HandArmor");
      prev = TRUE;
    }
  if (HAS_BODY_ARMOR_IMPLANT (ch))
    {
      if (prev)
	strcat (buf, ", ");
      strcat (buf, "BodyArmor");
      prev = TRUE;
    }
  strcat (buf, ".");
  i = looker->position;
  looker->position = POSITION_STANDING;
  act (buf, looker, NULL, looker, TO_CHAR);
  looker->position = i;
  return;
}

void
show_player (CHAR_DATA * god, CHAR_DATA * victim)
{
  char buf[1024];
  SINGLE_OBJECT *obj;
  int tt;
  bool prev;
  sprintf (buf,
	   "[\x1B[35;1m%d\x1B[37;0m] Name: \x1B[37;1m%s\x1B[37;0m  (\x1B[37;1m%s #%d\x1B[37;0m)  Gender: %s  Email: %s\n\r",
	   LEVEL (victim), NAME (victim),
	   race_info[victim->pcdata->race].name, victim->pcdata->race,
	   (victim->pcdata->sex == SEX_MALE ? "M" : victim->pcdata->sex ==
	    SEX_FEMALE ? "F" : "N"), victim->pcdata->email);
  send_to_char (buf, god);

  sprintf (buf, "Align: %d  Remorts: %d (",
	   victim->pcdata->alignment, victim->pcdata->remort_times);
  send_to_char (buf, god);
  for (tt = 0; tt < MAX_REMORT_OPTIONS; tt++)
    {
      sprintf (buf, "%s%d", (tt > 0 ? "/" : ""), victim->pcdata->remort[tt]);
      send_to_char (buf, god);
    }
  sprintf (buf, ") NatAC: %d  Deaths: %d  Warpts: %d\n\r",
	   victim->armor, victim->pcdata->deaths, victim->pcdata->warpoints);
  send_to_char (buf, god);
  sprintf (buf,
	   "Clan: %s  Clan2: %s  Room #%d  BankMoney: %ld  Pracs: %d  Learns: %d\n\r",
	   (clan_number (victim) <
	    0) ? "None" : (itoa (clan_number (victim))),
	   (clan_number_2 (victim) <
	    0) ? "None" : (itoa (clan_number_2 (victim))),
	   (victim->in_room ? victim->in_room->vnum : 0),
	   victim->pcdata->bank, victim->pcdata->practice,
	   victim->pcdata->learn);
  send_to_char (buf, god);
  sprintf (buf,
	   "Total kills: %d  Kill points: %d  HP: \x1B[37;1m%d/%d\x1B[37;0m  MV: \x1B[37;1m%d/%d\x1B[37;0m\n\r",
	   victim->pcdata->totalkills, victim->pcdata->killpoints,
	   victim->hit, victim->max_hit, victim->move, victim->max_move);
  send_to_char (buf, god);
  sprintf (buf,
	   "Hitroll: %d  Damroll: %d  Thirst: %d/50  Full: %d/50  Drunk: %d/0\n\r",
	   GET_HITROLL (victim), GET_DAMROLL (victim),
	   victim->pcdata->condition[COND_THIRST],
	   victim->pcdata->condition[COND_FULL],
	   victim->pcdata->condition[COND_DRUNK]);
  send_to_char (buf, god);
  sprintf (buf,
	   "Saving_throw: %d  Items Car: %d  Weight Car: %d  Age: %d  Dntn: %d  Tps: %d\n\r",
	   victim->pcdata->saving_throw, victim->pcdata->carry_number,
	   victim->pcdata->carry_weight, get_age (victim),
	   victim->pcdata->donated, victim->pcdata->tps);
  send_to_char (buf, god);
  if (!str_cmp (NAME (god), "Eraser") || !str_cmp (NAME (god), "Kilith"))
    {
      sprintf (buf, "Hours left: %d hours, %d mins.  Monthly Passes: %d.\n\r",
	       victim->pcdata->hours_purchased / 3600,
	       (victim->pcdata->hours_purchased / 60) -
	       (((victim->pcdata->hours_purchased / 3600) * 60)),
	       victim->pcdata->monthly_pass);
      send_to_char (buf, god);
    }
  print_profs (god, victim->pcdata->profession_flag);
  sprintf (buf,
	   "\x1B[34;1m----- Stats and Extras ------------ (%s) ----------\x1B[37;0m\n\r",
	   victim->desc ? "Player currently online" : "Player not logged in");
  send_to_char (buf, god);
  sprintf (buf,
	   "Str: %2d/%2d  Con: %2d/%2d  Dex: %2d/%2d  Int: %2d/%2d  Wis: %2d/%2d\n\r",
	   get_curr_str (victim), victim->pcdata->perm_stat[STR_I],
	   get_curr_con (victim), victim->pcdata->perm_stat[CON_I],
	   get_curr_dex (victim), victim->pcdata->perm_stat[DEX_I],
	   get_curr_int (victim), victim->pcdata->perm_stat[INT_I],
	   get_curr_wis (victim), victim->pcdata->perm_stat[WIS_I]);
  send_to_char (buf, god);
  sprintf (buf, "Use Smd, Cmd, Dmd, Imd, and Wmd to set stat modifiers.   Exp: %ld\n\r", victim->exp);
  send_to_char (buf, god);
  if (victim->pcdata->cool_skills != 0)
    {
      bool prv;
      prv = FALSE;
      send_to_char ("Cool Skills: ", god);
      if (NO_HUNGER (victim))
	{
	  if (prv)
	    send_to_char (", ", god);
	  send_to_char ("No_hunger", god);
	  prv = TRUE;
	}
      if (NO_THIRST (victim))
	{
	  if (prv)
	    send_to_char (", ", god);
	  send_to_char ("No_thirst", god);
	  prv = TRUE;
	}
      if (VIEW_MOBS (victim))
	{
	  if (prv)
	    send_to_char (", ", god);
	  send_to_char ("view_mobs", god);
	  prv = TRUE;
	}
      if (NO_PKILL (victim))
	{
	  if (prv)
	    send_to_char (", ", god);
	  send_to_char ("no_pkill", god);
	  prv = TRUE;
	}
      if (CHEAP_RECALL (victim))
	{
	  if (prv)
	    send_to_char (", ", god);
	  send_to_char ("Cheap_recall", god);
	  prv = TRUE;
	}

      if (MARTIAL_ARTIST (victim))
	{
	  if (prv)
	    send_to_char (", ", god);
	  send_to_char ("Martial_artist", god);
	  prv = TRUE;
	}
      if (ARCHMAGE (victim))
	{
	  if (prv)
	    send_to_char (", ", god);
	  send_to_char ("Archmage", god);
	  prv = TRUE;
	}
      send_to_char (".\n\r", god);
    }
  if (victim->pcdata->nat_abilities != 0)
    {
      send_to_char ("Natural Abilities: ", god);
      send_to_char (natural_abilities (victim->pcdata->nat_abilities), god);
      send_to_char (".\n\r", god);
    }
  show_guilds (god, victim->pcdata->guilds);
  show_implants (victim, god);
  send_to_char ("Type EXTRA to see more data.  Other bits: ", god);
  if (IS_SET (victim->pcdata->act2, PLR_SILENCE))
    {
      send_to_char ("Silenced ", god);
    }
  if (IS_SET (victim->act, PLR_FREEZE))
    {
      send_to_char ("Frozen ", god);
    }
  if (IS_SET (victim->act, ACT_UNVAL))
    {
      send_to_char ("UNVALIDATED ", god);
    }
  send_to_char ("\n\r", god);
  return;
}

void
show_extras (CHAR_DATA * god, CHAR_DATA * victim)
{
  int i = 0;
  int tt;
  SINGLE_OBJECT *prev;
  SINGLE_OBJECT *obj;
  SPELL_DATA *s;
  hugebuf_o[0] = '\0';
  strcpy (hugebuf_o, "\x1B[37;1mSkills...\x1B[37;0m");
  for (tt = 0; tt < SKILL_COUNT; tt++)
    {
      if (victim->pcdata->learned[tt] > 0)
	{
	  if ((s = skill_lookup (NULL, tt)) != NULL)
	    {
	      if (i % 2 == 0)
		strcat (hugebuf_o, "\n\r");
	      i++;
	      sprintf (hugebuf_o + strlen (hugebuf_o),
		       " [%24s #%3d %3d%%]   ", s->spell_name, tt,
		       victim->pcdata->learned[tt]);
	    }
	}
    }
  strcat (hugebuf_o, "\n\r");
  strcat (hugebuf_o, "\x1B[37;1m\n\rInventory...\x1B[37;0m");
  prev = FALSE;
  i = 0;
  for (obj = victim->carrying; obj != NULL; obj = obj->next_content)
    {
      if (i % 11 == 0)
	strcat (hugebuf_o, "\n\r");
      i++;
      if (obj->wear_loc == -1)
	sprintf (hugebuf_o + strlen (hugebuf_o), "%-6d ",
		 obj->pIndexData->vnum);
      else
	sprintf (hugebuf_o + strlen (hugebuf_o), "*%-5d ",
		 obj->pIndexData->vnum);
    }
  strcat (hugebuf_o, "\n\r");
  i = 0;
  strcat (hugebuf_o, "\x1B[37;1m\n\rStorage...\x1B[37;0m");
  prev = FALSE;
  for (tt = 0; tt < MAXST; tt++)
    {
      if (victim->pcdata->storage[tt] == NULL)
	continue;
      if (i % 11 == 0)
	strcat (hugebuf_o, "\n\r");
      i++;
      sprintf (hugebuf_o + strlen (hugebuf_o), "%6d ",
	       victim->pcdata->storage[tt]->pIndexData->vnum);
    }
  strcat (hugebuf_o, "\n\r");
  page_to_char (hugebuf_o, god);
  return;
}

void
pedit (CHAR_DATA * ch, char *argy)
{
  CHAR_DATA *victim;
  char buf[1024];
  char arg1[512];
  char arg2[512];
  char arg3[512];
  char arg4[512];
  int bit = 0;
  static char store_arg[SML_LENGTH];
  int value;
  strcpy (store_arg, argy);
  victim = (CHAR_DATA *) ch->desc->pEdit;
  if (!victim)
  {
    ch->desc->connected = CON_PLAYING;
    ch->desc->pEdit = NULL;
    return;
  }
  arg1[0] = '\0';
  arg2[0] = '\0';
  arg3[0] = '\0';
  arg4[0] = '\0';
  value = -1;
  argy = one_argy (argy, arg1);
  if (arg1[0] == '\0' || !str_cmp (arg1, "show"))
  {
    show_player (ch, victim);
    return;
  }
#ifdef NEW_WORLD
  S_HEALER (arg1, bit)
    S_WIZARD (arg1, bit)
    S_KNIGHT (arg1, bit)
    S_BARBARIAN (arg1, bit)
    S_RANGER (arg1, bit)
    S_PALADIN (arg1, bit)
    S_DRUID (arg1, bit)
    S_ARCHMAGE (arg1, bit)
    S_ASSASSIN (arg1, bit)
    S_MONK (arg1, bit)
    S_BARD (arg1, bit)
    S_HUNTER (arg1, bit)
    S_ENGINEER (arg1, bit) S_PHILOSOPHER (arg1, bit) if (bit != 0)
    {
      victim->pcdata->profession_flag ^= bit;
      send_to_char ("Profession toggled.\n\r", ch);
      return;
    }
#endif
  if (!str_prefix ("extra", arg1))
  {
    show_extras (ch, victim);
    return;
  }
  if (argy && argy != "")
    argy = one_argy (argy, arg2);
  if (argy && argy != "")
    argy = one_argy (argy, arg3);
  if (argy && argy != "")
    argy = one_argy (argy, arg4);
  if (arg2[0] == '-')
    value = atoi (arg2);
  else
    value = is_number (arg2) ? atoi (arg2) : -1;
  if (check_toggle_implant (victim, arg1))
  {
    send_to_char ("Implant flag toggled.\n\r", ch);
    return;
  }
  if (!str_prefix ("valid", arg1))
  {
    if (IS_SET (victim->act, ACT_UNVAL))
      REMOVE_BIT (victim->act, ACT_UNVAL);
    else
      SET_BIT (victim->act, ACT_UNVAL);
    send_to_char ("Validated flag toggled.\n\r", ch);
    victim->pcdata->level = 1;
    return;
  }
  if (!str_cmp (arg1, "height") && value >= 0)
  {
    if (value < 1 || value > 32000)
    {
      send_to_char ("Valid range for height is 1 - 32000.\n\r", ch);
      return;
    }
    victim->height = value;
    send_to_char ("Victim height set.\n\r", ch);
    return;
  }
  if (!str_cmp (arg1, "name") && arg2[0] != '\0')
  {
    FILE *fblep;
    DESCRIPTOR_DATA *dblep;
    if (!str_cmp (arg2, "Kilith"))
      return;
    if (!str_cmp (arg2, "Eraser"))
      return;
    if (!check_parse_name (arg2, TRUE))
    {
      send_to_char ("Sorry, you cannot use that name.\n\r", ch);
      return;
    }
    sprintf (buf, "%s%s", PLAYER_DIR, capitalize (arg2));
    if ((fblep = fopen (buf, "r")))
    {
      send_to_char ("But somebody is already using that name!\n\r", ch);
      fclose (fblep);
      return;
    }
    for (dblep = descriptor_list; dblep; dblep = dblep->next)
    {
      if (!str_cmp (NAME (dblep->character), arg2))
      {
        send_to_char ("Somebody is trying to roll a char with that name!\n\r", ch);
        return;
      }
    }
    sprintf (buf, "%s%s", PLAYER_DIR, NAME (victim));
    unlink (buf);
    sprintf (buf, "%s%s.bak", PLAYER_DIR, NAME (victim));
    unlink (buf);
    sprintf (buf, "%s%s.cor", PLAYER_DIR, NAME (victim));
    unlink (buf);
    sprintf (buf, "%s%s.note", PLAYER_DIR, NAME (victim));
    unlink (buf);
    sprintf (buf, "%s%s.poll", PLAYER_DIR, NAME (victim));
    unlink (buf);
    free_string (victim->pcdata->name);
    victim->pcdata->name = str_dup (capitalize (arg2));
    save_char_obj (victim);
    send_to_char ("Name changed.\n\r", ch);
    return;
  }
#ifdef NEW_WORLD
  if (!str_cmp (arg1, "addtps") && value >= 0)
  {
    victim->pcdata->tps += value;
    send_to_char ("Tps added.\n\r", ch);
    return;
  }
  if (!str_cmp (arg1, "donated") && value >= 0)
  {
    victim->pcdata->donated = value;
    send_to_char ("Donation value set.\n\r", ch);
    return;
  }
#endif
  if (!str_cmp (arg1, "race") && value >= 0 && value < RACE_COUNT)
  {
    victim->pcdata->race = value;
    victim->pcdata->nat_armor = race_info[victim->pcdata->race].natarmor;
    victim->pcdata->nat_abilities =
      race_info[victim->pcdata->race].affect_bits;
    send_to_char ("Race changed.\n\r", ch);
    return;
  }
  if (!str_cmp (arg1, "sex") && value >= 0 && value < 3)
  {
    victim->pcdata->sex = value;
    send_to_char ("Victim's sex set.\n\r", ch);
    return;
  }
  if (!str_cmp ("remort", arg1) && value >= 0)
  {
    victim->pcdata->remort_times = value;
    send_to_char ("Victim remort times set.\n\r", ch);
    return;
  }
  if (!str_prefix ("remort_dam", arg1) && value >= 0)
  {
    victim->pcdata->remort[REMORT_DAM] = value;
    send_to_char ("Victim remort times on dam set.\n\r", ch);
    return;
  }
  if (!str_prefix ("remort_prac", arg1) && value >= 0)
  {
    victim->pcdata->remort[REMORT_PRAC] = value;
    send_to_char ("Victim remort times on prac set.\n\r", ch);
    return;
  }
  if (!str_prefix ("remort_mana", arg1) && value >= 0)
  {
    victim->pcdata->remort[REMORT_MANA] = value;
    natural_mana (victim);
    send_to_char ("Victim remort times on mana set.\n\r", ch);
    return;
  }
  if (!str_prefix ("remort_hp", arg1) && value >= 0)
  {
    victim->pcdata->remort[REMORT_HP] = value;
    send_to_char ("Victim remort times on hp set.\n\r", ch);
    return;
  }
  if (!str_prefix ("remort_move", arg1) && value >= 0)
  {
    victim->pcdata->remort[REMORT_MOVE] = value;
    send_to_char ("Victim remort times on move set.\n\r", ch);
    return;
  }

  if (!str_cmp ("email", arg1) && arg2[0] != '\0')
  {
    strcpy (victim->pcdata->email, arg2);
    send_to_char ("Victim email set.\n\r", ch);
    return;
  }
  if (!str_prefix ("nat", arg1) && value != -1)
  {
    victim->pcdata->nat_armor = value;
    send_to_char ("Victim natural armor set.\n\r", ch);
    return;
  }
  if (!str_prefix ("death", arg1) && value >= 0)
  {
    victim->pcdata->deaths = value;
    send_to_char ("Victim deaths set.\n\r", ch);
    return;
  }
  if (!str_prefix ("war", arg1) && value >= 0)
  {
    victim->pcdata->warpoints = value;
    send_to_char ("Victim warpoints set.\n\r", ch);
    return;
  }
  if (!str_cmp ("room", arg1) && value > 1)
  {
    CHAR_DATA *ii;
    ROOM_DATA *rd;
    for (ii = char_list; ii != NULL; ii = ii->next)
    {
      if (ii == victim)
      {
        send_to_char
          ("You can't change a player's room while they are online!\n\r",
           ch);
        return;
      }
    }
    if ((rd = get_room_index (value)) == NULL)
    {
      send_to_char ("That room doesn't exist.\n\r", ch);
      return;
    }
    victim->in_room = rd;
    send_to_char ("Victim in_room changed.\n\r", ch);
    return;
  }
  if (!str_cmp ("bank", arg1) && value >= 0)
  {
    victim->pcdata->bank = value;
    send_to_char ("Victim bank amount set.\n\r", ch);
    return;
  }
  if (!str_prefix ("prac", arg1) && value >= 0)
  {
    victim->pcdata->practice = value;
    send_to_char ("Victim practices set.\n\r", ch);
    return;
  }
  if (!str_prefix ("learn", arg1) && value >= 0)
  {
    victim->pcdata->learn = value;
    send_to_char ("Victim learns set.\n\r", ch);
    return;
  }
  if ((!str_cmp ("tkills", arg1) || !str_cmp ("kills", arg1)) && value >= 0)
  {
    victim->pcdata->totalkills = value;
    send_to_char ("Total kills set.\n\r", ch);
    return;
  }
  if ((!str_cmp ("kpts", arg1) || !str_prefix ("killpo", arg1)) && value >= 0)
  {
    victim->pcdata->killpoints = value;
    send_to_char ("Killpoints set.\n\r", ch);
    return;
  }
  if (!str_cmp ("ac", arg1))
  {
    if (str_cmp ("yes", arg2))
    {
      send_to_char
        ("Make sure the chracter is NAKED before you do this, and that\n\r",
         ch);
      send_to_char
        ("NO AFFECTS are present on him/her (use noaffect). Then type ac yes\n\r",
         ch);
      return;
    }
    victim->armor = 100;
    send_to_char ("Victim's armor set to 100.\n\r", ch);
    return;
  }
  if (!str_cmp ("hpnow", arg1) && value >= 0)
  {
    victim->hit = value;
    send_to_char ("Hit points now set.\n\r", ch);
    return;
  }
  if (!str_cmp ("hp", arg1) && value >= 0)
  {
    victim->max_hit = value;
    send_to_char ("Hit points max set.\n\r", ch);
    return;
  }
  if (!str_cmp ("mvnow", arg1) && value >= 0)
  {
    victim->move = value;
    send_to_char ("Move points now set.\n\r", ch);
    return;
  }
  if (!str_cmp ("mv", arg1) && value >= 0)
  {
    victim->max_move = value;
    send_to_char ("Move points max set.\n\r", ch);
    return;
  }
  if (!str_cmp (arg1, "wis") && value >= 0)
  {
    if (value < 3 || value > MAX_STAT)
    {
      sprintf(buf, "Wisdom range is 3 to %d.\n\r", MAX_STAT);
      send_to_char(buf, ch);
      return;
    }
    send_to_char ("Wisdom set.\n\r", ch);
    victim->pcdata->perm_stat[WIS_I] = value;
    return;
  }
  if (!str_cmp (arg1, "dex") && value >= 0)
  {
    if (value < 3 || value > MAX_STAT)
    {
      sprintf(buf, "Dexterity range is 3 to %d.\n\r", MAX_STAT);
      send_to_char(buf, ch);
      return;
    }
    send_to_char ("Dexterity set.\n\r", ch);
    victim->pcdata->perm_stat[DEX_I] = value;
    return;
  }
  if (!str_cmp (arg1, "con") && value >= 0)
  {
    if (value < 3 || value > MAX_STAT)
    {
      sprintf(buf, "Constitution range is 3 to %d.\n\r", MAX_STAT);
      send_to_char(buf, ch);
      return;
    }
    send_to_char ("Constitution set.\n\r", ch);
    victim->pcdata->perm_stat[CON_I] = value;
    return;
  }
  if (!str_cmp (arg1, "int") && value >= 0)
  {
    if (value < 3 || value > MAX_STAT)
    {
      sprintf(buf, "Intelligence range is 3 to %d.\n\r", MAX_STAT);
      send_to_char(buf, ch);
      return;
    }
    send_to_char ("Intelligence set.\n\r", ch);
    victim->pcdata->perm_stat[INT_I] = value;
    return;
  }
  if (!str_cmp (arg1, "str") && value >= 0)
  {
    if (value < 3 || value > MAX_STAT)
    {
      sprintf(buf, "Strength range is 3 to %d.\n\r", MAX_STAT);
      send_to_char(buf, ch);
      return;
    }
    send_to_char ("Strength set.\n\r", ch);
    victim->pcdata->perm_stat[STR_I] = value;
    return;
  }
  if (!str_cmp (arg1, "wmd"))
  {
    send_to_char ("Wisdom mod set.\n\r", ch);
    victim->pcdata->mod_stat[WIS_I] = value;
    return;
  }
  if (!str_cmp (arg1, "dmd"))
  {
    send_to_char ("Dexterity mod set.\n\r", ch);
    victim->pcdata->mod_stat[DEX_I] = value;
    return;
  }
  if (!str_cmp (arg1, "imd"))
  {
    send_to_char ("Intelligence mod set.\n\r", ch);
    victim->pcdata->mod_stat[INT_I] = value;
    return;
  }
  if (!str_cmp (arg1, "cmd"))
  {
    send_to_char ("Constitution mod set.\n\r", ch);
    victim->pcdata->mod_stat[CON_I] = value;
    return;
  }
  if (!str_cmp (arg1, "smd"))
  {
    send_to_char ("Strength mod set.\n\r", ch);
    victim->pcdata->mod_stat[STR_I] = value;
    return;
  }
  if (!str_cmp (arg1, "Sec") && value > 0 && value < 10)
  {
    send_to_char ("Security set.\n\r", ch);
    victim->pcdata->security = value;
    return;
  }
  if (!str_cmp (arg1, "hitrl"))
  {
    send_to_char ("Hitroll set.\n\r", ch);
    victim->hitroll = value;
    return;
  }
  if (!str_cmp (arg1, "damrl"))
  {
    send_to_char ("Damroll set.\n\r", ch);
    victim->damroll = value;
    return;
  }
  if (!str_cmp (arg1, "thirst"))
  {
    send_to_char ("Thirst set.\n\r", ch);
    victim->pcdata->condition[COND_THIRST] = value;
    return;
  }
  if (!str_cmp (arg1, "full"))
  {
    send_to_char ("Full set.\n\r", ch);
    victim->pcdata->condition[COND_FULL] = value;
    return;
  }
  if (!str_cmp (arg1, "drunk"))
  {
    send_to_char ("Drunk set.\n\r", ch);
    victim->pcdata->condition[COND_DRUNK] = value;
    return;
  }
  if (!str_prefix ("sav", arg1))
  {
    send_to_char ("Saving throw set.\n\r", ch);
    victim->pcdata->saving_throw = value;
    return;
  }
  if (!str_cmp (arg1, "cool") && arg2[0] != '\0')
  {
    if (!str_cmp (arg2, "no_hunger") || !str_cmp (arg2, "hunger"))
    {
      if (NO_HUNGER (victim))
      {
        send_to_char ("No_hunger Removed.\n\r", ch);
        victim->pcdata->cool_skills ^= COOL_SKILL_NO_HUNGER;
      }
      else
      {
        send_to_char ("No_hunger Added.\n\r", ch);
        victim->pcdata->cool_skills ^= COOL_SKILL_NO_HUNGER;
      }
      return;
    }
    if (!str_cmp (arg2, "no_thirst") || !str_cmp (arg2, "thirst"))
    {
      if (NO_THIRST (victim))
      {
        send_to_char ("No_thirst Removed.\n\r", ch);
        victim->pcdata->cool_skills ^= COOL_SKILL_NO_THIRST;
      }
      else
      {
        send_to_char ("No_thirst Added.\n\r", ch);
        victim->pcdata->cool_skills ^= COOL_SKILL_NO_THIRST;
      }
      return;
    }
    if (!str_cmp (arg2, "cheap_recall") || !str_cmp (arg2, "recall"))
    {
      if (CHEAP_RECALL (victim))
      {
        send_to_char ("Cheap_recall Removed.\n\r", ch);
        victim->pcdata->cool_skills ^= COOL_SKILL_CHEAP_RECALL;
      }
      else
      {
        send_to_char ("Cheap_recall added.\n\r", ch);
        victim->pcdata->cool_skills ^= COOL_SKILL_CHEAP_RECALL;
      }
      return;
    }
    if (!str_cmp (arg2, "view_mobs") || !str_cmp (arg2, "view_mobs"))
    {
      if (VIEW_MOBS (victim))
      {
        send_to_char ("View_mobs Removed.\n\r", ch);
        victim->pcdata->cool_skills ^= COOL_SKILL_VIEW_MOBS;
      }
      else
      {
        send_to_char ("View_mobs added.\n\r", ch);
        victim->pcdata->cool_skills ^= COOL_SKILL_VIEW_MOBS;
      }
      return;
    }
    if (!str_cmp (arg2, "no_pkill"))
    {
      if (NO_PKILL (victim))
      {
        send_to_char ("No_pkill Removed.\n\r", ch);
        victim->pcdata->cool_skills ^= COOL_SKILL_NO_PKILL;
      }
      else
      {
        send_to_char ("No_pkill added.\n\r", ch);
        victim->pcdata->cool_skills ^= COOL_SKILL_NO_PKILL;
      }
      return;
    }

    if (!str_cmp (arg2, "archmage") || !str_cmp (arg2, "archmage"))
    {
      if (ARCHMAGE (victim))
      {
        send_to_char ("Archmage Removed.\n\r", ch);
        victim->pcdata->cool_skills ^= COOL_SKILL_ARCHMAGE;
      }
      else
      {
        send_to_char ("Archmage Added.\n\r", ch);
        victim->pcdata->cool_skills ^= COOL_SKILL_ARCHMAGE;
      }
      return;
    }
    if (!str_cmp (arg2, "martial_artist") || !str_cmp (arg2, "martial"))
    {
      if (MARTIAL_ARTIST (victim))
      {
        send_to_char ("Martial Artist Removed.\n\r", ch);
        victim->pcdata->cool_skills ^= COOL_SKILL_MARTIAL_ARTIST;
      }
      else
      {
        send_to_char ("Martial Artist Added.\n\r", ch);
        victim->pcdata->cool_skills ^= COOL_SKILL_MARTIAL_ARTIST;
      }
      return;
    }
  }
  if (!str_cmp (arg1, "guild") && arg2[0] != '\0')
  {
    if (!str_cmp (arg2, "tinker"))
    {
      if (IS_SET (victim->pcdata->guilds, ACT3_TINKER))
      {
        send_to_char ("Tinker's Guild Removed.\n\r", ch);
        victim->pcdata->guilds ^= ACT3_TINKER;
      }
      else
      {
        send_to_char ("Tinker's Guild Added.\n\r", ch);
        victim->pcdata->guilds ^= ACT3_TINKER;
      }
      return;
    }
    if (!str_cmp (arg2, "warrior"))
    {
      if (IS_SET (victim->pcdata->guilds, ACT3_WARRIOR))
      {
        send_to_char ("Warrior's Guild Removed.\n\r", ch);
        victim->pcdata->guilds ^= ACT3_WARRIOR;
      }
      else
      {
        send_to_char ("Warrior's Guild Added.\n\r", ch);
        victim->pcdata->guilds ^= ACT3_WARRIOR;
      }
      return;
    }
    if (!str_cmp (arg2, "healer"))
    {
      if (IS_SET (victim->pcdata->guilds, ACT3_HEALER))
      {
        send_to_char ("Healer's Guild Removed.\n\r", ch);
        victim->pcdata->guilds ^= ACT3_HEALER;
      }
      else
      {
        send_to_char ("Healer's Guild Added.\n\r", ch);
        victim->pcdata->guilds ^= ACT3_HEALER;
      }
      return;
    }
    if (!str_cmp (arg2, "wizard"))
    {
      if (IS_SET (victim->pcdata->guilds, ACT3_WIZARD))
      {
        send_to_char ("Wizard's Guild Removed.\n\r", ch);
        victim->pcdata->guilds ^= ACT3_WIZARD;
      }
      else
      {
        send_to_char ("Wizard's Guild Added.\n\r", ch);
        victim->pcdata->guilds ^= ACT3_WIZARD;
      }
      return;
    }
    if (!str_cmp (arg2, "thief") || !str_cmp (arg2, "thiefg"))
    {
      if (IS_SET (victim->pcdata->guilds, ACT3_THIEFG))
      {
        send_to_char ("Thief's Guild Removed.\n\r", ch);
        victim->pcdata->guilds ^= ACT3_THIEFG;
      }
      else
      {
        send_to_char ("Thief's Guild Added.\n\r", ch);
        victim->pcdata->guilds ^= ACT3_THIEFG;
      }
      return;
    }
    if (!str_cmp (arg2, "ranger"))
    {
      if (IS_SET (victim->pcdata->guilds, ACT3_RANGER))
      {
        send_to_char ("Ranger's Guild Removed.\n\r", ch);
        victim->pcdata->guilds ^= ACT3_RANGER;
      }
      else
      {
        send_to_char ("Ranger's Guild Added.\n\r", ch);
        victim->pcdata->guilds ^= ACT3_RANGER;
      }
      return;
    }
    if (!str_cmp (arg2, "assassin"))
    {
      if (IS_SET (victim->pcdata->guilds, ACT3_ASSASSIN))
      {
        send_to_char ("Assassin's Guild Removed.\n\r", ch);
        victim->pcdata->guilds ^= ACT3_ASSASSIN;
      }
      else
      {
        send_to_char ("Assassin's Guild Added.\n\r", ch);
        victim->pcdata->guilds ^= ACT3_ASSASSIN;
      }
      return;
    }
    if (!str_cmp (arg2, "paladin"))
    {  
      if (IS_SET (victim->pcdata->guilds, ACT3_PALADIN))
      {
        send_to_char ("Paladin's Guild Removed.\n\r", ch);
        victim->pcdata->guilds ^= ACT3_PALADIN;
      }
      else
      {
        send_to_char ("Paladin's Guild Added.\n\r", ch);
        victim->pcdata->guilds ^= ACT3_PALADIN;
      }
      return;
    }  
    if (!str_cmp (arg2, "marauder")) 
    {  
      if (IS_SET (victim->pcdata->guilds, ACT3_MARAUDER)) 
      {
        send_to_char ("Marauder's Guild Removed.\n\r", ch); 
        victim->pcdata->guilds ^= ACT3_MARAUDER; 
      }
      else
      {
        send_to_char ("Marauder's Guild Added.\n\r", ch); 
        victim->pcdata->guilds ^= ACT3_MARAUDER; 
      }
      return;
    }  
    if (!str_cmp (arg2, "battlemage")) 
    {  
      if (IS_SET (victim->pcdata->guilds, ACT3_BATTLEMAGE)) 
      {
        send_to_char ("Battlemage's Guild Removed.\n\r", ch); 
        victim->pcdata->guilds ^= ACT3_BATTLEMAGE; 
      }
      else
      {
        send_to_char ("Battlemage's Guild Added.\n\r", ch); 
        victim->pcdata->guilds ^= ACT3_BATTLEMAGE; 
      }
      return;
    }  
    if (!str_cmp (arg2, "elemental")) 
    {  
      if (IS_SET (victim->pcdata->guilds, ACT3_ELEMENTAL)) 
      {
        send_to_char ("Elemental's Guild Removed.\n\r", ch); 
        victim->pcdata->guilds ^= ACT3_ELEMENTAL; 
      }
      else
      {
        send_to_char ("Elemental's Guild Added.\n\r", ch); 
        victim->pcdata->guilds ^= ACT3_ELEMENTAL; 
      }
      return;
    }
    if (!str_cmp (arg2, "necromancer"))
    {
      if (IS_SET (victim->pcdata->guilds, ACT3_NECROMANCER))
      {
        send_to_char ("Necromancer's Guild Removed.\n\r", ch);
        victim->pcdata->guilds ^= ACT3_NECROMANCER;
      }
      else
      {
        send_to_char ("Necromancer's Guild Added.\n\r", ch);
        victim->pcdata->guilds ^= ACT3_NECROMANCER;
      }
      return;
    }
    if (!str_cmp (arg2, "mystics"))
    {
      if (IS_SET (victim->pcdata->guilds, ACT3_MYSTICS))
      {
        send_to_char ("Mystic's Guild Removed.\n\r", ch);
        victim->pcdata->guilds ^= ACT3_MYSTICS;
      }
      else
      {
        send_to_char ("Mystic's Guild Added.\n\r", ch);
        victim->pcdata->guilds ^= ACT3_MYSTICS;
      }
      return;
    }
    if (!str_cmp (arg2, "tatics"))
    {
      if (IS_SET (victim->pcdata->guilds, ACT3_TATICS))
      {
        send_to_char ("Tatic's Guild Removed.\n\r", ch);
        victim->pcdata->guilds ^= ACT3_TATICS;
      }
      else
      {
        send_to_char ("Tatic's Guild Added.\n\r", ch);
        victim->pcdata->guilds ^= ACT3_TATICS;
      }
      return;
    }
    if (!str_cmp (arg2, "shaman"))
    {
      if (IS_SET (victim->pcdata->guilds, ACT3_SHAMAN))
      {
        send_to_char ("Shaman's Guild Removed.\n\r", ch);
        victim->pcdata->guilds ^= ACT3_SHAMAN;
      }
      else
      {
        send_to_char ("Shaman's Guild Added.\n\r", ch);
        victim->pcdata->guilds ^= ACT3_SHAMAN;
      }
      return;
    }
    send_to_char ("Unknown guild.\n\r", ch);
    return;
  }
  if (!str_cmp (arg1, "level"))
  {
    if (value < 1 || value > 99)
      return;
    victim->exp = (FIND_EXP ((value - 1), 1));
    send_to_char ("Level set.\n\r", ch);
    return;
  }
  if (!str_prefix (arg1, "align"))
  {
    if (value < -5000 || value > 5000)
      return;
    victim->pcdata->alignment = value;
    send_to_char ("Alignment set.\n\r", ch);
    return;
  }
  if (!str_cmp (arg1, "fly"))
  {
    if (IS_SET (victim->pcdata->nat_abilities, AFF_FLYING))
    {
      act ("$N no longer has natural flying abilities.", ch, NULL, victim,
          TO_CHAR);
      victim->pcdata->nat_abilities -= AFF_FLYING;
    }
    else
    {
      act ("$N now has natural flying abilities.", ch, NULL, victim,
          TO_CHAR);
      victim->pcdata->nat_abilities += AFF_FLYING;
    }
    return;
  }
  if (!str_prefix ("infra", arg1))
  {
    if (IS_SET (victim->pcdata->nat_abilities, AFF_INFRARED))
    {
      act ("$N can no longer see in the dark naturally.", ch, NULL,
          victim, TO_CHAR);
      victim->pcdata->nat_abilities -= AFF_INFRARED;
    }
    else
    {
      act ("$N can now see in the dark naturally.", ch, NULL, victim,
          TO_CHAR);
      victim->pcdata->nat_abilities += AFF_INFRARED;
    }
    return;
  }
  if (!str_prefix ("d_hid", arg1))
  {
    if (IS_SET (victim->pcdata->nat_abilities, AFF_DETECT_HIDDEN))
    {
      act ("$N can no longer sense hidden objs/lifeforms naturally.", ch,
          NULL, victim, TO_CHAR);
      victim->pcdata->nat_abilities -= AFF_DETECT_HIDDEN;
    }
    else
    {
      act ("$N can now sense hidden lifeforms/objs naturally.", ch, NULL,
          victim, TO_CHAR);
      victim->pcdata->nat_abilities += AFF_DETECT_HIDDEN;
    }
    return;
  }
  if (!str_prefix ("d_invis", arg1))
  {
    if (IS_SET (victim->pcdata->nat_abilities, AFF_DETECT_INVIS))
    {
      act ("$N no longer has natural detect invis.", ch, NULL, victim,
          TO_CHAR);
      victim->pcdata->nat_abilities -= AFF_DETECT_INVIS;
    }
    else
    {
      act ("$N now has natural detect invis.", ch, NULL, victim, TO_CHAR);
      victim->pcdata->nat_abilities += AFF_DETECT_INVIS;
    }
    return;
  }
  if (!str_prefix ("breath", arg1))
  {
    if (IS_SET (victim->pcdata->nat_abilities, AFF_BREATH_WATER))
    {
      act ("$N can no longer breath water naturally.", ch, NULL, victim,
          TO_CHAR);
      victim->pcdata->nat_abilities -= AFF_BREATH_WATER;
    }
    else
    {
      act ("$N now can breath water naturally.", ch, NULL, victim,
          TO_CHAR);
      victim->pcdata->nat_abilities += AFF_BREATH_WATER;
    }
    return;
  }
  if (!str_prefix ("skill", arg1) && value > 0 && value < SKILL_COUNT
      && is_number (arg3))
  {
    int set_to;
    set_to = atoi (arg3);
    if (set_to < 0 || set_to > 100)
    {
      send_to_char
        ("Valid ranges are 0 (unlearned) to 100 (perfect).\n\r", ch);
      return;
    }
    if (set_to == 0)
      set_to = -100;
    victim->pcdata->learned[value] = set_to;
    send_to_char ("Skill/Spell set.\n\r", ch);
    return;
  }
  if (!str_prefix ("obj", arg1) && value != -1)
  {
    SINGLE_OBJECT *otr;
    SINGLE_OBJECT *ofnd;
    OBJ_PROTOTYPE *crobj;
    ofnd = NULL;
    if (value < 0)		/*Take away an object */
    {
      value = 0 - value;
      for (otr = victim->carrying; otr != NULL; otr = otr->next_content)
        if (otr->pIndexData->vnum == value)
        {
          ofnd = otr;
          break;
        }
      if (ofnd == NULL)
      {
        send_to_char ("Obj not found.\n\r", ch);
        return;
      }
      if (ofnd->wear_loc != -1)
      {
        unequip_char (victim, ofnd);
      }
      obj_from (ofnd);
      free_it (ofnd);
      send_to_char ("Object removed.\n\r", ch);
      return;
    }
    if ((crobj = get_obj_index (value)) == NULL)
    {
      send_to_char ("Invalid object.\n\r", ch);
      return;
    }
    ofnd = create_object (crobj, 1);
    obj_to (ofnd, victim);
    ofnd->wear_loc = -1;
    send_to_char ("Object added.\n\r", ch);
    return;
  }
  if (!str_prefix ("stor", arg1) && value != -1)
  {
    int otr;
    SINGLE_OBJECT *ofnd;
    ofnd = NULL;
    if (value < 0)		/*Take away an object */
    {
      value = 0 - value;
      for (otr = 0; otr < MAXST; otr++)
      {
        if (victim->pcdata->storage[otr] == NULL)
          continue;
        if (victim->pcdata->storage[otr]->pIndexData->vnum == value)
        {
          ofnd = victim->pcdata->storage[otr];
          break;
        }
      }
      if (ofnd == NULL)
      {
        send_to_char ("Obj not found.\n\r", ch);
        return;
      }
      free_it (victim->pcdata->storage[otr]);
      victim->pcdata->storage[otr] = NULL;
      send_to_char ("Object removed.\n\r", ch);
      return;
    }
    send_to_char
      ("You can only remove personal storage items, not give.\n\r", ch);
    return;
  }
  if (!str_prefix ("silence", arg1))
  {
    victim->pcdata->act2 ^= PLR_SILENCE;
    send_to_char ("Toggled.\n\r", ch);
    return;
  }
  if (!str_cmp ("frozen", arg1))
  {
    victim->act ^= PLR_FREEZE;
    send_to_char ("Toggled.\n\r", ch);
    return;
  }
  if (!str_cmp ("done", arg1))
  {
    CHAR_DATA *iii;
    ch->desc->connected = CON_PLAYING;
    ch->desc->pEdit = NULL;
    save_char_obj (victim);
    for (iii = char_list; iii != NULL; iii = iii->next)
    {
      if (iii == victim)
        return;
    }
    extract_char (victim, TRUE);
    return;
  }
  if (!str_suffix ("edit", arg1))
  {
    send_to_char ("Please type done before you go to another editor.\n\r",
        ch);
    return;
  }
  interpret (ch, store_arg);
  return;
}

void
do_pedit (CHAR_DATA * ch, char *argy)
{
  CHAR_DATA *victim;
  CHAR_DATA *cy;
  char edbuf[500];
  DEFINE_COMMAND ("pedit", do_pedit, POSITION_DEAD, 110, LOG_ALWAYS,
      "Allows you to edit a player online or offline.")
  if (!IS_REAL_GOD (ch))
  {
    send_to_char ("Huh?\n\r", ch);
    return;
  }
  if (!argy || argy[0] == '\0')
  {
    send_to_char ("Pedit who?", ch);
    return;
  }
  pedit_found = NULL;
  victim = NULL;
  for (cy = char_list; cy != NULL; cy = cy->next)
  {
    if (IS_MOB (cy))
      continue;
    if (!str_cmp (NAME (cy), argy))
    {
      victim = cy;
      break;
    }
  }
  sprintf (edbuf, "%s", argy);
  if (victim == NULL)
  {
    if (!load_char_obj (NULL, edbuf, TRUE))
    {
      send_to_char ("That character was not found.\n\r", ch);
      return;
    }
  }
  if (victim == NULL)
    victim = pedit_found;
  ch->desc->pEdit = (void *) victim;
  ch->desc->connected = CON_PEDITOR;
  return;
}
