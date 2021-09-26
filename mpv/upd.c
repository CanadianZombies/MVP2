#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "emlen.h"
//fatecode

// Some stuff from other code - not much.. mostly weather crap

int prior_mana, prior_hp, prior_mv, tttt;
long average_time[MT];
long times_through[MT];
long last_time[MT];
long this_time[MT];
long total_time[MT];
long temp_time[MT];
char *itoa (int);
int hit_gain (CHAR_DATA * ch);
int move_gain (CHAR_DATA * ch);
void mobile_update (void);
void weather_update (void);
void char_update (void);
void char_only_update (void);
void transport_update (void);
void obj_update (void);
void aggr_update (void);
void do_disaster (void);
static int pulse_field = 3;
static int pulse_mobile = 1;
static int pulse_violence = 1;
static int pulse_point = 2;
static int pulse_aggro = 1;
static int player_upd = 2;
static int transport_upd = 1;
static bool flaggy;
static int hours_up = 0;

void
fix_char (CHAR_DATA * ch)
{
  int i;
  AFFECT_DATA *afd;
  SINGLE_OBJECT *bah;
  if (IS_MOB (ch))
    return;
  ch->hitroll = 0;
  ch->damroll = 0;
  ch->armor = ch->pcdata->nat_armor;
  if (ch->hit > ch->max_hit)
    ch->hit = ch->max_hit;

  if (ch->pcdata != NULL)
    {
      i = ch->pcdata->guilds;
      ch->pcdata->guilds = 0;
      if (IS_SET (i, GUILD_TINKER))
	ch->pcdata->guilds |= GUILD_TINKER;
      if (IS_SET (i, GUILD_WARRIOR))
	ch->pcdata->guilds |= GUILD_WARRIOR;
      if (IS_SET (i, GUILD_THIEFG))
	ch->pcdata->guilds |= GUILD_THIEFG;
      if (IS_SET (i, GUILD_HEALER))
	ch->pcdata->guilds |= GUILD_HEALER;
      if (IS_SET (i, GUILD_WIZARD))
	ch->pcdata->guilds |= GUILD_WIZARD;
      if (IS_SET (i, GUILD_RANGER))
	ch->pcdata->guilds |= GUILD_RANGER;
      if (IS_SET (i, GUILD_ASSASSIN))
	ch->pcdata->guilds |= GUILD_ASSASSIN;
      if (IS_SET (i, GUILD_MARAUDER))
        ch->pcdata->guilds |= GUILD_MARAUDER;
      if (IS_SET (i, GUILD_ELEMENTAL))
        ch->pcdata->guilds |= GUILD_ELEMENTAL;
      if (IS_SET (i, GUILD_PALADIN))
        ch->pcdata->guilds |= GUILD_PALADIN;
      if (IS_SET (i, GUILD_BATTLEMAGE))
        ch->pcdata->guilds |= GUILD_BATTLEMAGE;
      if (IS_SET (i, GUILD_NECROMANCER))
        ch->pcdata->guilds |= GUILD_NECROMANCER;
      if (IS_SET (i, GUILD_MYSTICS))
        ch->pcdata->guilds |= GUILD_MYSTICS;
      if (IS_SET (i, GUILD_TATICS))
        ch->pcdata->guilds |= GUILD_TATICS;
      if (IS_SET (i, GUILD_SHAMAN))
        ch->pcdata->guilds |= GUILD_SHAMAN;
    }

  LOOP (afd, ch->affected, afd->next)
  {
    if (afd->location == APPLY_DAMROLL)
      ch->damroll += afd->modifier;
    if (afd->location == APPLY_HITROLL)
      ch->hitroll += afd->modifier;
    if (afd->location == APPLY_AC)
      ch->armor += afd->modifier;
  }

  LOOP (bah, ch->carrying, bah->next_content)
  {
    if (bah->wear_loc == -1 || bah->wear_loc == WEAR_BELT_1 ||
	bah->wear_loc == WEAR_BELT_2 || bah->wear_loc == WEAR_BELT_3)
      continue;
    ch->armor -= apply_ac (bah, bah->wear_loc);
    LOOP (afd, bah->pIndexData->affected, afd->next)
    {
      if (afd->location == APPLY_DAMROLL)
	ch->damroll += afd->modifier;
      if (afd->location == APPLY_HITROLL)
	ch->hitroll += afd->modifier;
      if (afd->location == APPLY_AC)
	ch->armor += afd->modifier;
    }
  }
  {				// fixing chars that remorted the oldfashioned way (+2 dam every remort)
    short tot = 0;
    short ix;
    for (ix = 0; ix < MAX_REMORT_OPTIONS; ix++)
      tot += ch->pcdata->remort[ix];
    if (tot < ch->pcdata->remort_times)
      ch->pcdata->remort[REMORT_DAM] += ch->pcdata->remort_times - tot;
  }
  ch->damroll += (ch->pcdata->remort[REMORT_DAM] * REMORT_DAM_AMOUNT);
  return;
}


void
advance_level (CHAR_DATA * ch)
{
  int add_prac;
  int sn;
  int add_learn;
  int ct;
  ct = 0;
  if (IS_MOB (ch))
    return;

  add_level_bonuses (ch, 1);
  add_prac = (pow.practice_percent * wis_app[get_curr_wis (ch) - 1].practice) / 100;

  for (sn = 0; sn < SKILL_COUNT; sn++)
    {
      if (ch->pcdata->learned[sn] > 0)
	ct++;
    }
  add_learn = 0;
  add_learn = LEVEL (ch) < 4 ? 2 : 1;

  ch->pcdata->practice += add_prac;
  ch->pcdata->learn += add_learn;
  natural_mana (ch);
  upd_mana (ch);
  upd_hps (ch);
  return;
}

void
award_tps (CHAR_DATA * ch, int gain)
{
  ROOM_DATA *rid = NULL;
  char general_use[500];
  int gg = gain;
  int ii = 0;
  while (gg > 0)
    {
      while (((rid = get_room_index (number_range (1000, 90000))) == NULL) ||
	     (rid->sector_type == SECT_WATER_NOSWIM) ||
	     (rid->sector_type == SECT_JAVA) || (!rid->area->open));
      rid->tps++;
      gg--;
    }
  if (IS_MOB (ch) || gain == 0)
    return;
  ch->pcdata->tps += gain;
 prior_mana=0;
  prior_hp=0;
  prior_mv=0;
  tttt=0;
  while (LEVEL (ch) < 90
	 && ch->exp >= (unsigned) FIND_EXP (LEVEL (ch), ch->pcdata->race)
	 && ch->pcdata->tps >= FIND_TPS (LEVEL (ch), ch->pcdata->race))
    {
      send_to_char ("You have become more powerful!\n\r", ch);
      ch->pcdata->level++;
      sprintf (general_use, "\x1B[30;1m-->\x1B[37;0mYour overall power and learning capacities have increased!\n\r");
      send_to_char (general_use, ch);
//Fate Mana code Kilith 13'
      prior_mana = ch->pcdata->n_max_mana + ch->pcdata->ma_mort;
      prior_hp   = ch->max_hit;
      prior_mv   = ch->max_move;
      advance_level (ch);
      tttt=(ch->pcdata->n_max_mana + ch->pcdata->ma_mort)  - prior_mana;
      prior_mana=tttt;
      tttt=ch->max_hit - prior_hp;
      prior_hp=tttt;
      tttt=ch->max_move - prior_mv;
      prior_mv=tttt;

      /* MARMAR */
      log_string (log_buf);
      sprintf (log_buf, "\x1B[1;31m%s \x1b[0;37mhas just gained a level! \x1B[1;34m < < \x1B[1;31m%d \x1B[1;34m > > \x1b[0;37m",
	       NAME (ch), ch->pcdata->level);
      ii = clan_number (ch);
      if (ii > 0)
	clan_notify (log_buf, ii);
    }
  upd_xp (ch);
  return;
}

void
gain_exp (CHAR_DATA * ch, int gain)
{
  char general_use[500];
  int ii = 0;
  int ohp, omv, omm;
  if (IS_MOB (ch) || LEVEL (ch) >= 95 || gain == 0)
    return;
  if (IN_BATTLE (ch) || CHALLENGE (ch) == 10)
    return;
  if (gain < 1)
    if ((unsigned) -gain >= ch->exp)
      return;
  ch->exp += gain;
  if (ch->exp >= (unsigned) FIND_EXP ((LEVEL (ch) + 2), ch->pcdata->race)
      && ch->pcdata->tps < FIND_TPS (LEVEL (ch), ch->pcdata->race))
    {
      ch->exp = FIND_EXP ((LEVEL (ch) + 2), ch->pcdata->race) - 1;
      upd_xp (ch);
      return;
    }
//Fate mana code Kilith 13'
  prior_hp=0;
  prior_mana=0;
  prior_mv=0;
  tttt=0;
  while (LEVEL (ch) < pow.max_level
	 && ch->exp >= (unsigned) FIND_EXP (LEVEL (ch), ch->pcdata->race)
	 && ch->pcdata->tps >= FIND_TPS (LEVEL (ch), ch->pcdata->race))
    {
      send_to_char ("You have become more powerful!\n\r", ch);
      sprintf (general_use,"\x1B[30;1m-->\x1B[34;1mYour overall power and learning capacities have increased!\n\r");
      send_to_char (general_use, ch);
//Fate Mana code Kilith 13'
      prior_mana = ch->pcdata->n_max_mana + ch->pcdata->ma_mort;
      prior_hp   = ch->max_hit;   
      prior_mv   = ch->max_move;

      //ohp = ch->max_hit;
      //omv = ch->max_move;
      //omm = ch->pcdata->n_max_mana;
      ch->pcdata->level++;
      advance_level (ch);
//Fate Mana Code Kilith 13'
      tttt=(ch->pcdata->n_max_mana + ch->pcdata->ma_mort ) - prior_mana;
      prior_mana=tttt;
      tttt=ch->max_hit - prior_hp;
      prior_hp=tttt;
      tttt=ch->max_move - prior_mv;
      prior_mv=tttt;

      sprintf (general_use, "You have gained \x1B[31;1m%d \x1B[34;1mhitpoints, \x1B[31;1m%d \x1B[34;1mmovement points and \x1B[31;1m%d \x1B[34;1mmore mana! You are lvl \x1B[31m%d\x1B[34;1m!!!\x1B[37;0m\n\r",
         ch->pcdata->level, prior_hp, prior_mv, prior_mana); //Fate mana code Kilith 13'

	       //ch->max_hit - ohp, ch->max_move - omv, ch->pcdata->n_max_mana - omm,  ch->pcdata->level);
      send_to_char (general_use, ch);
      /*Kilith added in mana to level line 03'*/
      // raise wimpy for newbies
      if (ch->pcdata->level <= 10 && ch->pcdata->remort_times < 1)
	{
	  check_ced (ch);
	  ch->ced->wimpy += (ch->max_hit - ohp) / 3;
	  sprintf (general_use, "\x1b[0;37mYour wimpy has been auto-raised to \x1B[1;31m%d\x1b[0;37m.\n\r", ch->ced->wimpy);
	  send_to_char (general_use, ch);
	}

      log_string (log_buf);
      sprintf (log_buf, "\x1B[1;31m%s \x1b[0;37mhas just gained a level! \x1B[1;34m< < \x1B[1;31m%d \x1B[1;34m> >\x1b[0;37m", NAME (ch), ch->pcdata->level);
      ii = clan_number (ch);
      if (ii > 0)
	clan_notify (log_buf, ii);
    }
  upd_xp (ch);
  if (IS_JAVA (ch))
    do_vscore (ch, "");
  return;
}

int
max_mana (CHAR_DATA * ch)
{
  int maxmana;
  SINGLE_OBJECT *obj;
  int reval;
  if (IS_MOB (ch))
    return 0;
  maxmana = 1000;  /*100 + ((get_curr_int (ch) - 14) * 4) + ((get_curr_wis (ch) - 14) * 4);*/
  reval = ch->pcdata->n_max_mana;	/* + (( (LEVEL(ch)/3) * (get_curr_int(ch) + get_curr_wis(ch))) / 10); */
  if ((obj = get_item_held (ch, ITEM_GEM)) != NULL)
    {
      if (((I_GEM *) obj->more)->max_mana < maxmana)
	maxmana = ((I_GEM *) obj->more)->max_mana;
      reval += maxmana;
    }
  return reval;
}

void
mana_gain (CHAR_DATA * ch, short hehe)
{
  int maxmana;
  SINGLE_OBJECT *obj;
  char general_use[500];
/*MANA REGEN HERE */
  if (IS_MOB (ch))
    return;
  maxmana = 1000; /*100 + ((get_curr_int (ch) - 14) * 4) + ((get_curr_wis (ch) - 14) * 4) + ch->pcdata->n_max_mana;*/
  if (ch->pcdata->n_mana < ch->pcdata->n_max_mana && hehe != 2)
    {
      int tt = pow.base_mana_regen_bonus;
      if (ch->position == POSITION_MEDITATING)
	tt += pow.med_mana_regen_bonus;
      ch->pcdata->n_mana = UMIN (ch->pcdata->n_max_mana, ch->pcdata->n_mana + tt);
      upd_mana (ch);
      if (IS_JAVA (ch))
	java_mana (ch);
    }
  if (hehe == 2 && ch->pcdata->n_mana > 0)
    {
      --ch->pcdata->n_mana;
      upd_mana (ch);
      if (IS_JAVA (ch))
	java_mana (ch);
    }
  if ((obj = get_item_held (ch, ITEM_GEM)) != NULL)
    {
      if (((I_GEM *) obj->more)->max_mana < maxmana)
	maxmana = ((I_GEM *) obj->more)->max_mana;
      if (ch->pcdata->condition[COND_THIRST] > 1)
	{
	  if (hehe == 2)
	    {
	      ((I_GEM *) obj->more)->mana_now -= 1;
	      if (((I_GEM *) obj->more)->mana_now < 0)
		((I_GEM *) obj->more)->mana_now = 0;
	      upd_mana (ch);
	      if (IS_JAVA (ch))
		java_mana (ch);
	      return;
	    }
	  if (((I_GEM *) obj->more)->mana_now == maxmana)
	    return;
	  if ((((I_GEM *) obj->more)->mana_now < maxmana) &&
	      (((I_GEM *) obj->more)->mana_now + 1 >= maxmana))
	    {
	      sprintf (general_use, "%s has been charged to its maximum capacity!\n\r", capitalize (STR (obj, short_descr)));
	      send_to_char (general_use, ch);
	    }
	  if (hehe)
	    ((I_GEM *) obj->more)->mana_now += 1;
	  else if (ch->position == POSITION_MEDITATING)
	    ((I_GEM *) obj->more)->mana_now += (7 + (get_curr_int (ch) / 5));
	  else if (ch->position == POSITION_SLEEPING)
	    ((I_GEM *) obj->more)->mana_now += (4 + (get_curr_int (ch) / 8));
	  else if (ch->position == POSITION_RESTING)
	    ((I_GEM *) obj->more)->mana_now += 3;
	  else
	    ((I_GEM *) obj->more)->mana_now += 2;
	  if (get_curr_wis (ch) > 19)
	    ((I_GEM *) obj->more)->mana_now++;
	  if (get_curr_wis (ch) > 23)
	    ((I_GEM *) obj->more)->mana_now++;
	}
      if (((I_GEM *) obj->more)->mana_now > maxmana)
	((I_GEM *) obj->more)->mana_now = maxmana;
      else
	{
	  if (MANANOW (obj) > 19 && MANANOW (obj) < 22)
	    send_to_char ("Your gem begins to glow softly.\n\r", ch);
	  if (MANANOW (obj) > 32 && MANANOW (obj) < 35)
	    send_to_char
	      ("Your gem begins to warm your hand with its power.\n\r", ch);
	  if (MANANOW (obj) > 61 && MANANOW (obj) < 64)
	    send_to_char ("Your gem hums with powerful magical energy!\n\r", ch);
	}
      upd_mana (ch);
      if (IS_JAVA (ch))
	java_mana (ch);
    }
/*END MANA REGEN */
  return;
}

/*
   * Regeneration stuff.
 */
int
hit_gain (CHAR_DATA * ch)
{
  int gain;
  if (FIGHTING (ch))
    return 0;
  gain = pow.base_hp_regen;
  switch (ch->position)
    {
    case POSITION_SLEEPING:
      gain +=
	pow.hp_bonus_sleeping + (number_range (1, get_curr_con (ch) / 4));
      break;
    case POSITION_RESTING:
      gain +=
	pow.hp_bonus_resting + (number_range (1, get_curr_con (ch) / 8));
      break;
    }
#ifdef NEW_WORLD
  if (IS_MOB (ch) && ch->hit < (ch->max_hit / 2) && !FIGHTING (ch))
    {
      gain += LEVEL (ch) / 5;
    }
#else
  if (IS_MOB (ch))
    {
      gain += LEVEL (ch) / 5;
    }
#endif
  if (IS_PLAYER (ch))
    {
      if (ch->pcdata->condition[COND_FULL] <= 5)
	gain /= 2;
      if (ch->pcdata->condition[COND_FULL] <= 0)
	gain = 1;
      if (ch->pcdata->condition[COND_THIRST] <= 5)
	gain /= 2;
      if (ch->pcdata->condition[COND_THIRST] <= 0)
	gain = 1;
    }
  if (IS_AFFECTED (ch, AFF_POISON) && (IS_MOB (ch)))
    {
      gain /= 3;
    }
  if (IS_AFFECTED (ch, AFF_POISON) && (IS_PLAYER (ch)))
    {
      gain /= 4;
    }
  //if (IS_AFFECTED (ch, AFF_PLAGUE))
  //  gain /= 8;
  if (IS_PLAYER (ch) && race_info[ch->pcdata->race].regen_hit_percent != 100)
    {
      gain = (gain * race_info[ch->pcdata->race].regen_hit_percent) / 100;
    }
  return UMIN (gain, ch->max_hit - ch->hit);
}

int
move_gain (CHAR_DATA * ch)
{
  int gain = 0;
  if (FIGHTING (ch))
    return 1;
  gain = pow.base_mp_regen;
  switch (ch->position)
    {
    case POSITION_SLEEPING:
      gain += pow.mp_bonus_sleeping + get_curr_dex (ch) / 3;
      break;
    case POSITION_RESTING:
      gain += pow.mp_bonus_resting + get_curr_dex (ch) / 5;
      break;
    }
  if (IS_PLAYER (ch))
    {
      if (ch->pcdata->condition[COND_FULL] <= 0)
	{
	  gain = 1;
	}
      if (ch->pcdata->condition[COND_THIRST] <= 0)
	gain = 1;
    }
  if (IS_AFFECTED (ch, AFF_POISON))
    gain /= 2;
  //if (IS_AFFECTED (ch, AFF_PLAGUE))
   // gain /= 4;
  if (IS_PLAYER (ch) && race_info[ch->pcdata->race].regen_move_percent != 100)
    {
      gain = (gain * race_info[ch->pcdata->race].regen_move_percent) / 100;
    }
  return UMIN (gain, ch->max_move - ch->move);
}

void
gain_condition (CHAR_DATA * ch, int iCond, int value)
{
  int condition;
  if (value == 0 || IS_MOB (ch))
    return;
  ch->pcdata->condition[iCond] += value;
  if (ch->pcdata->condition[COND_DRUNK] < 0)
    ch->pcdata->condition[COND_DRUNK] = 0;
  if (ch->pcdata->condition[iCond] > 48)
    ch->pcdata->condition[iCond] = 48;
  condition = ch->pcdata->condition[iCond];
  if (ch->pcdata->condition[iCond] < -2)
    {
      switch (iCond)
	{
	case COND_FULL:
	  send_to_char ("You \x1B[1;30mhurt \x1B[37;0mfrom \x1B[0;33mhunger\x1B[37;0m!\n\r", ch);
	  break;
	case COND_THIRST:
	  send_to_char ("You \x1B[1;30mhurt \x1B[37;0mfrom \x1B[0;34mthirst\x1B[37;0m!\n\r", ch);
	  break;
	default:
	  return;
	}
      if (ch->desc != NULL)
	{
	  if (ch->hit + (condition * 10) < 0)
	    {
	      SUBHIT (ch, 1);
	      upd_hps (ch);
	      update_pos (ch);
	    }
	  else
	    {
	      ADDHIT (ch, (condition * 5));
	    }
	  upd_hps (ch);
	  if (ch->hit == -1)
	    {
	      send_to_char ("Soooo verrrryyyy hunnnnggrrryyyy....\n\r", ch);
	      ch->pcdata->condition[COND_THIRST] = 48;
	      ch->pcdata->condition[COND_FULL] = 48;
	      strcpy (reasonfd, "Starvation");
	      raw_kill (ch, FALSE);
	      send_to_char
		("Your vision blurs as the world slips away around you...\n\r",
		 ch);
	      MAXHIT (ch);
	      upd_hps (ch);
	      update_pos (ch);
	      return;
	    }
	  return;
	}
    }

  if (ch->pcdata->condition[iCond] == 9)
    {
      switch (iCond)
	{
	case COND_FULL:
	  send_to_char
	    ("You are beginning to get hungry... better get some food.\n\r",
	     ch);
	  break;
	case COND_THIRST:
	  send_to_char ("You are beginning to get thirsty.\n\r", ch);
	  break;
	}
    }
  if (ch->pcdata->condition[iCond] <= 1)
    {
      switch (iCond)
	{
	default:
	  break;
	case COND_FULL:
	  send_to_char
	    ("You begin to feel yourself weaken from intense hunger!\n\r",
	     ch);
	  break;
	case COND_THIRST:
	  send_to_char
	    ("You feel so dehydrated that your vision begins to blur!\n\r",
	     ch);
	  break;
	}
    }
  if (ch->pcdata->condition[iCond] == 3)
    {
      switch (iCond)
	{
	default:
	  break;
	case COND_FULL:
	  send_to_char ("Your stomach rumbles with hunger.\n\r", ch);
	  break;
	case COND_THIRST:
	  send_to_char ("You begin to feel dehydrated.\n\r", ch);
	  break;
	case COND_DRUNK:
	  if (condition != 0)
	    send_to_char ("You are sober.\n\r", ch);
	  break;
	case COND_STARVING:
	  send_to_char ("You are STARVING! Get some food!\n\r", ch);
	  break;
	}
    }
  return;
}


void
mobile_update (void)
{
  char buffy[STD_LENGTH];
  CHAR_DATA *ch;
  CHAR_DATA * victim;
  CHAR_DATA *ch_next;
  EXIT_DATA *pexit;
  int door;
  buffy[0] = '\0';
/* Examine all mobs. */
  LOOP (ch, char_list, ch_next)
  {
    ch_next = ch->next;
    if (ch->in_room == NULL)
      {
	CHAR_DATA *fx;
	for (fx = char_list; fx != NULL; fx = fx->next)
	  {
	    if (FIGHTING (fx) && FIGHTING (fx) == ch)
	      fx->fgt->fighting = NULL;
	  }
	fprintf (stderr, "Extracted mob/char that was not in any room...\n");
	extract_char (ch, TRUE);
	continue;
      }
    if (FIGHTING (ch) && FIGHTING (ch)->data_type == 50)
      {
	fprintf (stderr, "That was it2! upd.c\n");
	ch->fgt->fighting = NULL;
      }
    if (IS_PLAYER (ch))
      {
	if (ch->position != POSITION_FIGHTING
	    && ch->position != POSITION_CASTING
	    && ch->position != POSITION_SEARCHING
	    && ch->position != POSITION_BASHED
	    && ch->position != POSITION_BACKSTAB
	    && ch->position != POSITION_CIRCLE
	    && ch->position != POSITION_GROUNDFIGHTING)
	  ch->wait = 0;
	if (FIGHTING (ch) == NULL && ch->position == POSITION_BASHED)
	  NEW_POSITION (ch, POSITION_STANDING);
	if (ch->wait > 0)
	  ch->wait -= 13;
	if (ch->wait < 0)
	  ch->wait = 0;
	continue;
      }
    if (ch->fgt && ch->fgt->running_char_function != NULL)
      {
	if (ch->fgt->script_delay == 0)
	  {
	    (*ch->fgt->running_char_function) (ch->fgt->caller, ch, ch->fgt->argy);
	  }
	else
	  ch->fgt->script_delay--;
      }

    if (ch->ced)
      {
	if (ch->ced->scr_wait > 0 && ch->ced->wait_type == WAIT_TYPE_PULSE)
	  {
	    ch->ced->scr_wait--;
	    if (ch->ced->scr_wait == 0)
	      {
		do_scriptline (ch, NULL, NULL);
	      }
	  }
      }
    if (ch->in_room->area->nplayer < 1)
      continue;
    if (IS_AFFECTED (ch, AFF_CHARM))
      continue;
    if (IS_MOB (ch) && ch->pIndexData->spec_fun
	&& (ch->position != POSITION_GROUNDFIGHTING)
	&& (ch->position != POSITION_BASHED))
      {

	if ((*ch->pIndexData->spec_fun) (ch))
	  continue;
      }
/*     if ( ch->npcdata->hire != NULL )
   {
   if ( ch->position == POSITION_SLEEPING )
   if ( dice(1,10) == 5 )
   {
   do_stand(ch,"");
   }
   }
 */
    if (IS_SET (ch->act, ACT_REST) && FIGHTING (ch) == NULL)
      {
	if (ch->next_in_room
	    && (ch->position == POSITION_RESTING
		|| ch->position == POSITION_SLEEPING))
	  NEW_POSITION (ch, POSITION_STANDING);
	else if (ch->position == POSITION_STANDING)
	  {
	    /* If a mob's exhausted, let it rest. */
	    if (ch->move < ch->max_move / 4)
	      do_sleep (ch, "");
	    else if (ch->move < ch->max_move / 2)
	      do_rest (ch, "");
	    if (ch->hit < ch->max_hit / 7)
	      do_sleep (ch, "");
	    else if (ch->hit < ch->max_hit / 4)
	      do_rest (ch, "");
	  }
      }
    if (ch->position != POSITION_STANDING && ch->position != POSITION_STOPCAST)
      continue;
    if (ch->in_room->more && IS_SET (ch->act, ACT_SLEEPAGGRO) && (FIGHTING (ch) == NULL))
      {
	CHAR_DATA *tch;
	CHAR_DATA *tch_next;
	for (tch = ch->in_room->more->people; tch != NULL; tch = tch_next)
	  {
	    if (!tch->next_in_room)
	      tch_next = NULL;
	    else
	      tch_next = tch->next_in_room;
	    if (IS_PLAYER (tch) && tch->position == POSITION_SLEEPING)
	      {
		multi_hit (ch, tch, TYPE_UNDEFINED);
		break;
	      }
	  }
      }

    if (FIGHTING (ch) == NULL && IS_MOB (ch))
      {
	if (ch->in_room->more && ch->in_room->more->pcs > 0)
	  {
	    if (IS_SET (ch->pIndexData->act3, ACT3_KILL_EVIL))
	      do_kill (ch, "evil");
	    if (IS_SET (ch->pIndexData->act3, ACT3_KILL_GOOD))
	      do_kill (ch, "good");
	    if (IS_SET (ch->act, ACT_ASSISTALL))
	      {
		CHAR_DATA *tcr;
		for (tcr = ch->in_room->more->people; tcr != NULL;
		     tcr = tcr->next_in_room)
		  {
		    if (IS_MOB (tcr) && FIGHTING (tcr) != NULL
			&& IS_PLAYER (tcr->fgt->fighting))
		      {
			set_fighting (ch, tcr->fgt->fighting);
			break;
		      }
		  }
	      }
	    if (IS_SET (ch->act, ACT_RESCUEONE))
	      do_rescue (ch, "prtct");
	    if (IS_SET (ch->act, ACT_RESCUETWO))
	      do_rescue (ch, "codeone");
	    if (IS_SET (ch->act, ACT_ASSISTONE))
	      do_assist (ch, "aone");
	    if (IS_SET (ch->act, ACT_ASSISTTWO))
	      do_assist (ch, "atwo");
	  }
           
	if (IS_SET (ch->pIndexData->act4, ACT4_KILLALLONE)
	    && FIGHTING (ch) == NULL)
	  {
	    CHAR_DATA *to_attack;
	    for (to_attack = ch->in_room->more->people; to_attack != NULL;
		 to_attack = to_attack->next_in_room)
	      {
		if (to_attack == ch
		    || (IS_PLAYER (to_attack)
			&& to_attack->pcdata->level > 99))
		  continue;
		if (IS_MOB (to_attack)
		    && IS_SET (to_attack->pIndexData->act4, ACT4_KILLALLONE))
		  continue;
		break;
	      }
	    if (to_attack != NULL && can_see (ch, to_attack))
	      {
		if (ch->position == POSITION_RESTING)
		  {
		    do_stand (ch, "");
		  }
		if (ch->position != POSITION_STANDING && ch->position != POSITION_STOPCAST)
		  goto pasthere;
		set_fighting (ch, to_attack);
		break;
	      }
	  }
	if (IS_SET (ch->pIndexData->act4, ACT4_KILLALLTWO)
	    && FIGHTING (ch) == NULL)
	  {
	    CHAR_DATA *to_attack;
	    for (to_attack = ch->in_room->more->people; to_attack != NULL;
		 to_attack = to_attack->next_in_room)
	      {
		if (to_attack == ch
		    || (IS_PLAYER (to_attack)
			&& to_attack->pcdata->level > 99))
		  continue;
		if (IS_MOB (to_attack)
		    && IS_SET (to_attack->pIndexData->act4, ACT4_KILLALLTWO))
		  continue;
		break;
	      }
	    if (to_attack != NULL && can_see (ch, to_attack))
	      {
		if (ch->position == POSITION_RESTING)
		  {
		    do_stand (ch, "");
		  }
		if (ch->position != POSITION_STANDING && ch->position != POSITION_STOPCAST)
		  goto pasthere;
		set_fighting (ch, to_attack);
		break;
	      }
	  }
	if (IS_SET (ch->pIndexData->act4, ACT4_KILLMEVIL)
	    && FIGHTING (ch) == NULL)
	  {
	    CHAR_DATA *to_attack;
	    for (to_attack = ch->in_room->more->people; to_attack != NULL;
		 to_attack = to_attack->next_in_room)
	      {
		if (to_attack == ch
		    || (IS_PLAYER (to_attack)
			&& to_attack->pcdata->level > 99))
		  continue;
		if (!IS_EVIL (to_attack))
		  continue;
		break;
	      }
	    if (to_attack != NULL && can_see (ch, to_attack))
	      {
		if (ch->position == POSITION_RESTING)
		  {
		    do_stand (ch, "");
		  }
		if (ch->position != POSITION_STANDING && ch->position != POSITION_STOPCAST)
		  goto pasthere;
		set_fighting (ch, to_attack);
		break;
	      }
	  }
	if (IS_SET (ch->pIndexData->act4, ACT4_KILLMGOOD)
	    && FIGHTING (ch) == NULL)
	  {
	    CHAR_DATA *to_attack;
	    for (to_attack = ch->in_room->more->people; to_attack != NULL;
		 to_attack = to_attack->next_in_room)
	      {
		if (to_attack == ch
		    || (IS_PLAYER (to_attack)
			&& to_attack->pcdata->level > 99))
		  continue;
		if (IS_EVIL (to_attack))
		  continue;
		break;
	      }
	    if (to_attack != NULL && can_see (ch, to_attack))
	      {
		if (ch->position == POSITION_RESTING)
		  {
		    do_stand (ch, "");
		  }
		if (ch->position != POSITION_STANDING && ch->position != POSITION_STOPCAST)
		  goto pasthere;
		set_fighting (ch, to_attack);
		break;
	      }
	  }
      pasthere:
	if (IS_SET (ch->act, ACT_KILLGLADIATOR))
	  do_kill (ch, "gladiator");
	if (IS_SET (ch->act, ACT_RANGER))
	  do_kill (ch, "beast");
	if (IS_SET (ch->pIndexData->affected_by, AFF_HIDE)
	    && !IS_SET (ch->affected_by, AFF_HIDE)
	    && ch->next_in_room == NULL)
	  SET_BIT (ch->affected_by, AFF_HIDE);
      }
    /* Wander */
    if (!IS_SET (ch->act, ACT_SENTINEL)
/*        && (ch->npcdata->hire == NULL) */
	&& (MOUNTED_BY (ch) == NULL)
	&& !IS_SET (ch->pIndexData->act3, ACT3_FOLLOW)
	&& (door = number_bits (5)) <= 5
	&& (pexit = ch->in_room->exit[door]) != NULL
	&& (ROOM_DATA *) pexit->to_room != NULL
	&& (!pexit->d_info || !IS_SET (pexit->d_info->exit_info, EX_CLOSED))
	&& !IS_SET (((ROOM_DATA *) pexit->to_room)->room_flags, ROOM_NO_MOB)
	&& number_range (1, 3) == 2
	&& (!IS_SET (ch->act, ACT_STAY_SECTOR)
	    || ((ROOM_DATA *) pexit->to_room)->sector_type ==
	    ch->in_room->sector_type) && (!IS_SET (ch->act, ACT_STAY_AREA)
					  || ((ROOM_DATA *) pexit->to_room)->
					  area == ch->in_room->area))
      {
	move_char (ch, door);
	if (ch->position < POSITION_STANDING)
	  continue;
      }
    /* Flee */
    if (ch->hit < ch->max_hit / 10
	&& (!IS_SET (ch->act, ACT_SENTINEL))
	&& (door = number_bits (3)) <= 5
	&& (pexit = ch->in_room->exit[door]) != NULL
	&& (ROOM_DATA *) pexit->to_room != NULL
	&& (!pexit->d_info || !IS_SET (pexit->d_info->exit_info, EX_CLOSED))
	&& !IS_SET (((ROOM_DATA *) pexit->to_room)->room_flags, ROOM_NO_MOB))
      {
	CHAR_DATA *rch;
	bool found;
	found = FALSE;
	if (((ROOM_DATA *) pexit->to_room)->more)
	  {
	    for (rch = ((ROOM_DATA *) pexit->to_room)->more->people;
		 rch != NULL; rch = rch->next_in_room)
	      {
		if (IS_PLAYER (rch))
		  {
		    found = TRUE;
		    break;
		  }
	      }
	    if (!found)
	      move_char (ch, door);
	  }
      }
  }
  return;
}

void
weather_update (void)
{
  char buf[STD_LENGTH];
  char general_use[500];
  char buff[STD_LENGTH];
  int diff;
  CHAR_DATA *wch;
  CHAR_DATA *ch_next;
  CHAR_DATA *pMob;
  SCRIPT_INFO *scr;
  SCRIPT_INFO *scr_next;
  SINGLE_TRIGGER *tr;
  SCRIPT_INFO *s;
  MOB_PROTOTYPE *pMobIndex;
  ROOM_DATA *rooom;
  int lightning = 0;
  int extracted = 0;
  int newday = 0;
  buf[0] = '\0';


  /* Check running scripts for delay resumes */
  for (scr = info_list; scr != NULL; scr = scr_next)
  {
    scr_next = scr->next;
    if (scr->delay_ticks <= 0)
      continue;
    if (scr->tick_type != 3)
      continue;
    scr->delay_ticks--;
    if (scr->delay_ticks == 0)
    {
      execute_code (scr);
    }
  }

  if (ticks_to_reboot != 0)
  {
    if (ticks_to_reboot == 1)
    {         
      CHAR_DATA *bah;
      FILE *fpqr;
      extern bool aturion_down;
      //if (IS_MOB(char_list))
      do_asave (char_list, "changed");
      do_asave (char_list, "triggers");
      do_asave (char_list, "code");
      do_asave (char_list, "clans");
      fprintf (stderr, "Auto-saved changed areas.\n");
      for (bah = char_list; bah != NULL; bah = bah->next)
      {
        if (IS_PLAYER (bah))
        {
	  //save_corpse (bah);
          //do_save (bah, "yy2zz11");
          save_char_obj (bah);
          //save_char_tro (bah);
          if (bah->desc && bah->in_room)
          {
            send_to_char ("\e[34;1m----[\e[32mRebooting...\e[34m]----\e[0;37m\n\r", bah);
          }
        }
      }
      fprintf (stderr, "Auto-saved all players.\n");
      fprintf (stderr, "Auto-reboot invoked...\n");
      fpqr = fopen ("reason.fi", "w+");
      fprintf (fpqr, "Mud-Auto-Reboot \n\rEnd~\n\r");
      fclose (fpqr);
      aturion_down = TRUE;
      return;
    }
    --ticks_to_reboot;
    sprintf (general_use, "\x1B[34;1m----[\x1B[32mMud Autosave and Reboot in \x1B[37m%d\x1B[32m game tick%s/hour%s!\x1B[34m]----\x1B[37;0m",
        ticks_to_reboot, (ticks_to_reboot == 1 ? "" : "s"),
        (ticks_to_reboot == 1 ? "" : "s"));
    {
      CHAR_DATA *sch;
      sch = create_mobile (get_mob_index (1));
      char_to_room (sch, get_room_index (2));
      do_echo (sch, general_use);
      extract_char (sch, TRUE);
    }
  }
  if (ticks_to_battlefield != 0)
  {
    if (ticks_to_battlefield == 1)
    {
      allow_anything = TRUE;
      {
        CHAR_DATA *sch;
        sch = create_mobile (get_mob_index (1));
        char_to_room (sch, get_room_index (2));
        do_transfer (sch, "all Battleground");
        extract_char (sch, TRUE);
      }
      allow_anything = FALSE;
    }
    --ticks_to_battlefield;
    if (ticks_to_battlefield == 6 || ticks_to_battlefield == 5 || ticks_to_battlefield == 4
       || ticks_to_battlefield == 3|| ticks_to_battlefield == 2 || ticks_to_battlefield == 1)
    {
      sprintf (general_use,
"\x1B[1;34m-->\x1B[1;31m%d \x1B[1;34mgame hour%s to battleground! \x1B[1;30m[\x1B[1;37mLevels \x1B[1;31m%d\x1B[1;37m-\x1B[1;31m%d\x1B[1;30m]\x1B[37;0m",
          ticks_to_battlefield, (ticks_to_battlefield == 1 ? "" : "s"), min_level, max_level);
      {
        CHAR_DATA *sch;
        sch = create_mobile (get_mob_index (1));
        char_to_room (sch, get_room_index (2));
        do_echo (sch, general_use);
        extract_char (sch, TRUE);
      }
      if (prize_vnum_2 == 0)
sprintf (general_use, "\x1B[1;34m-->\x1B[1;31mPrize is \x1B[37;0m%s \x1B[1;34mplus \x1B[1;37m5 \x1B[1;31mWarpoints \x1B[1;34mand \x1B[1;37m100,000 \x1B[1;31mExperience.\x1B[37;0m",
            get_obj_index (prize_vnum_1)->short_descr);
      else
sprintf (general_use, "\x1B[1;34m-->\x1B[1;37mPrizes \x1B[1;34m- \x1B[37;0m%s \x1B[1;34mand \x1B[37;0m%s \x1B[1;34mplus \x1B[1;37m5 \x1B[1;31mWarpoints \x1B[1;34mand \x1B[1;37m100,000 \x1B[1;31mExperience..\x1B[37;0m",
            get_obj_index (prize_vnum_1)->short_descr,
            get_obj_index (prize_vnum_2)->short_descr);
      {
        CHAR_DATA *sch;
        sch = create_mobile (get_mob_index (1));
        char_to_room (sch, get_room_index (2));
        do_echo (sch, general_use);
        extract_char (sch, TRUE);
      }
      if (ticks_to_battlefield > 5 && ticks_to_battlefield < 1)
      {
        CHAR_DATA *sch;
        sch = create_mobile (get_mob_index (1));
        char_to_room (sch, get_room_index (2));
        do_echo (sch, "\x1B[1;37mTo sign up for the battleground, type \x1B[1;34mbattle\x1B[1;37m.\x1B[37;0m");
        extract_char (sch, TRUE);
      }
       if (ticks_to_battlefield == 0)
      {
        CHAR_DATA *sch;
        sch = create_mobile (get_mob_index (1));
        char_to_room (sch, get_room_index (2));
        do_echo (sch, "\x1B[1;31mThe Battleground has started!!\x1B[1;37m.\x1B[37;0m");
        extract_char (sch, TRUE);
      }
    }
  }
  /*Check for any winners in the battleground */
  if (ticks_to_battlefield == 0)
  {
    DESCRIPTOR_DATA *dd;
    SINGLE_OBJECT *prize;
    CHAR_DATA *found_winner;
    ROOM_DATA *rid;
    int cnt;
    char gen_use[500];
    int people_in_bground;
    people_in_bground = 0;
    found_winner = NULL;
    for (dd = descriptor_list; dd != NULL; dd = dd->next)
    {
      if (!dd->character || dd->connected != CON_PLAYING)
        continue;
      if (dd->character->in_room
          && dd->character->in_room->vnum >= BATTLEGROUND_START_VNUM
          && dd->character->in_room->vnum <= BATTLEGROUND_END_VNUM)
      {
        people_in_bground++;
        found_winner = dd->character;
      }
    }
    if (people_in_bground == 1 && found_winner)
    {
      /*Below checks for mobs that may still be in bground */
      for (cnt = BATTLEGROUND_START_VNUM; cnt < BATTLEGROUND_END_VNUM;
          cnt++)
      {
        if ((rid = get_room_index (cnt)) == NULL)
          continue;
        if ((rid->more && rid->more->people != NULL
              && rid->more->people != found_winner) || (rid->more
                && rid->more->
                people
                && rid->more->
                people->
                next_in_room))
        {
          found_winner = NULL;
          break;
        }
      }
      if (found_winner != NULL && IS_PLAYER (found_winner))
      {
        char_from_room (found_winner);
        if (IS_SET (found_winner->pcdata->raceflag, RC_EVILRACE))
          char_to_room (found_winner, get_room_index (99));
        else
          char_to_room (found_winner, get_room_index (100));
        sprintf (gen_use, "\x1B[31;1m%s \x1B[37;1mis the battleground winner! \x1B[34;1mCongratulations!\x1B[37;0m", NAME (found_winner));
        {
          CHAR_DATA *sch;
          sch = create_mobile (get_mob_index (1));
          char_to_room (sch, get_room_index (2));
          do_echo (sch, gen_use);
          do_log (sch, gen_use);
          sprintf (buf, "%s addxp %d", NAME(found_winner), 100000);
          do_pset(sch, buf);
          sprintf (buff, "%s addwps %d", NAME(found_winner), 5);
          do_pset(sch, buff);
          do_restore (sch, NAME(found_winner));
          extract_char (sch, TRUE);
        }
        if (prize_vnum_1 != 0)
        {
          prize = create_object (get_obj_index (prize_vnum_1), 1);
          if (prize != NULL)
          {
            obj_to (prize, found_winner);
            prize->wear_loc = -1;
          }
        }
        if (prize_vnum_2 != 0)
        {
          SINGLE_OBJECT *prize_two;
          prize_two = create_object (get_obj_index (prize_vnum_2), 1);
          if (prize_two != NULL)
          {
            obj_to (prize_two, found_winner);
            prize_two->wear_loc = -1;
          }
        }
        prize_vnum_1 = 0;
        prize_vnum_2 = 0;
        send_to_char ("\x1B[31;1mCongratulations! \x1B[34;1mYou've won the battleground!\x1B[37;0m\n\r", found_winner);
      }
    }
  }
  room_update ();
  switch (++time_info.hour)
  {
 /*   case 4: 4:00am
      weather_info.sunlight = MOON_SET;
 strcat (buf, "\x1B[0;37mThe \x1B[1;36mmoon \x1B[0;37msets, and the sky slowly lights up in anticipation of \x1B[33;1mdaylight.\x1B[37;0m\n\r");
      break;*/
    case 6: /*6:00am*/
      weather_info.sunlight = SUN_LIGHT;
 strcat (buf, "\x1B[0;37mThe sky begins to grow lighter as \x1B[1;33mdaylight \x1B[0;37mapproaches.\x1B[37;0m\n\r");
      if (time_info.month <= 4 || time_info.month >= 17)
        weather_info.temperature = number_fuzzy (20);
      else
        weather_info.temperature = number_fuzzy (50);
      break;
    case 7: /*7:00am*/
      weather_info.sunlight = SUN_RISE;
      strcat (buf, "\x1B[0;37mThe \x1B[1;33msun \x1B[0;37mslowly rises in the east.\x1B[37;0m\n\r");
      weather_info.temperature += number_fuzzy (10);
      break;
    case 12: /*12:00pm*/
      strcat( buf, "\x1B[0;37mIt's high \x1B[1;33mnoon\x1B[0;37m.\x1B[37;0m\n\r" );
      weather_info.temperature += number_fuzzy (15);
      break;
    case 19: /*7:00pm*/
      weather_info.sunlight = SUN_SET;
  strcat (buf, "\x1B[0;31mThe sky begins to \x1B[0;34mdarken \x1B[0;31min anticipation of the \x1B[1;34mnight \x1B[0;31mto come.\x1B[37;0m\n\r");
      weather_info.temperature -= number_fuzzy (15);
      break;
    case 21: /*9:00pm*/
      weather_info.sunlight = SUN_DARK;
      strcat (buf, "\x1B[1;30mThe \x1B[1;34mnight \x1B[1;30mhas begun.\x1B[37;0m\n\r");
      weather_info.temperature -= number_fuzzy (10);
      break;
    case 24: /*0:00am midnight*/
      weather_info.sunlight = MOON_RISE;
strcat( buf, "\x1B[1;30mThe \x1B[1;36mmoon \x1B[1;30mrises, casting a \x1B[1;37msilver glow \x1B[1;30mover the \x1B[1;34mnight\x1B[1;30m.\x1B[0;37m\n\r" );
      weather_info.temperature -= number_fuzzy (10);
      time_info.hour = 0;
      time_info.day++;
      day_counter++;
      save_day_counter ();
      newday = 1;
      break;
  }
  if (time_info.day >= 35)
  {
    time_info.day = 0;
    time_info.month++;
  }
  if (time_info.month >= 17)
  {
strcat (buf, "\x1B[31;1mIt \x1B[33;1mis \x1B[34;1mthe \x1B[35;1mdawn \x1B[36;1mof \x1B[37;0ma \x1B[30;1mNew \x1B[32;1mYear!!!\x1B[37;0m\n\r");
    time_info.month = 0;
    time_info.year++;
  }
  /*
  * Weather change.
   */
  weather_info.winddir += number_range (0, 2) - 1;
  if (time_info.month >= 3 && time_info.month <= 19)
    diff = weather_info.mmhg > 985 ? -2 : 2;
  else
    diff = weather_info.mmhg > 1015 ? -2 : 2;
  weather_info.change += diff * dice (1, 4) + dice (2, 6) - dice (2, 6);
  weather_info.change = UMAX (weather_info.change, -12);
  weather_info.change = UMIN (weather_info.change, 12);
  weather_info.mmhg += weather_info.change;
  weather_info.mmhg = UMAX (weather_info.mmhg, 960);
  weather_info.mmhg = UMIN (weather_info.mmhg, 1040);
  switch (weather_info.sky)
  {
    default:
      bug ("Weather_update: bad sky %d.", weather_info.sky);
      weather_info.sky = SKY_CLOUDLESS;
      break;
    case SKY_CLOUDLESS:
      if (weather_info.mmhg < 990
          || (weather_info.mmhg < 1010 && number_bits (2) == 0))
      {
        if (time_info.month <= 4 || time_info.month >= 17)
        {
          strcat (buf, "\x1B[0;37mA few flakes of \x1B[1;37msnow \x1B[0;37mare falling... there is definately a chill in the air!\n\r");
          weather_info.temperature -= 10;
        }
        else
        {
          if (number_range (1, 2) == 2)
    strcat (buf, "\x1B[0;37mLarge, thick clouds begin to form, blocking the sky from view.\n\r");
          else
    strcat (buf, "\x1B[0;37mLow clouds begin to form, obstructing the sky from your view.\n\r");
        }
        weather_info.sky = SKY_CLOUDY;
        weather_info.windspeed += 10;
      }
      break;
    case SKY_CLOUDY:
      if (weather_info.mmhg < 970
          || (weather_info.mmhg < 990 && number_bits (2) == 0))
      {
        if (time_info.month <= 4 || time_info.month >= 17)
        {
     strcat (buf, "\x1B[0;37mIt begins to \x1B[1;37msnow\x1B[0;37m, and the air grows bitter cold.\x1B[37;0m\n\r");
          weather_info.temperature -= 6;
        }
        else if (weather_info.temperature < 75
            && weather_info.temperature > 45 && number_bits (3) <= 1)
        {
          strcat (buf, "\x1B[30;1mA thick layer of fog rolls in. \x1B[0;37mIt sure is hard to see anyting.\x1B[37;0m\n\r");
          weather_info.sky = SKY_FOGGY;
          weather_info.temperature -= 2;
          break;
        }
        else
        {
          int hash;
          ROOM_DATA *ridd;
          TRACK_DATA *tr1;
          TRACK_DATA *tr_n;
          bool stp = FALSE;
  strcat (buf, "\x1B[1;30mDark clouds \x1B[0;37min the sky begin to \x1B[1;34mpour rain \x1B[0;37mdown upon the lands.\x1B[37;0m\n\r");
          /* CLEAR ALL TRACKS!! WASHED AWAY */
          for (hash = 0; hash < HASH_MAX; hash++)
          {
            for (ridd = room_hash[hash]; ridd != NULL;
                ridd = ridd->next)
            {
              for (tr1 = ridd->tracks; !stp && tr1; tr1 = tr_n)
              {
                tr_n = tr1->next_track_in_room;
                if (tr_n == NULL)
                  stp = TRUE;
                free_m (tr1);
              }
              stp = FALSE;
              ridd->tracks = NULL;
            }
          }
        }
        weather_info.sky = SKY_RAINING;
        weather_info.windspeed += 10;
      }
      if (weather_info.mmhg > 1030 && number_bits (2) == 0)
      {
        if (time_info.month <= 3 || time_info.month >= 17)
        {
          strcat (buf,
              "\x1B[0;37mThe \x1B[1;37msnow \x1B[0;37mlets up, and the temperature rises slightly.\x1B[37;0m\n\r");
          weather_info.temperature += 10;
        }
        else
  strcat (buf, "\x1B[0;37mThe \x1B[1;37mclouds \x1B[0;37mdisappear, and the beautiful, clear \x1B[1;34msky \x1B[0;37mis visible above.\x1B[37;0m\n\r");
        weather_info.sky = SKY_CLOUDLESS;
        weather_info.windspeed -= 10;
      }
      break;
    case SKY_RAINING:
      if (weather_info.mmhg < 970 && number_bits (2) == 0)
      {
        if (time_info.month <= 4 || time_info.month >= 17)
        {
          strcat (buf, "\x1B[0;37mYou are caught in a \x1B[1;37mblizzard\x1B[0;37m... It's *damn* cold!\x1B[37;0m\n\r");
          weather_info.temperature -= 30;
        }
        else
strcat (buf, "\x1B[33;1mLightning flashes \x1B[0;37min the sky; it looks as if a \x1B[1;30mstorm \x1B[0;37mis brewing.\x1B[37;0m\n\r");
        weather_info.sky = SKY_LIGHTNING;
        weather_info.windspeed += 10;
      }
      if (weather_info.mmhg > 1030
          || (weather_info.mmhg > 1010 && number_bits (2) == 0))
      {
        if (time_info.month <= 4 || time_info.month >= 17)
        {
          strcat (buf, "\x1B[0;37mThe \x1B[1;37msnow \x1B[0;37mlets up, and the temperature rises.\x1B[37;0m\n\r");
          weather_info.temperature += 30;
        }
        else
          strcat (buf, "\x1B[0;37mThe \x1B[1;34mrain \x1B[0;37mhas stopped.\x1B[37;0m\n\r");
        weather_info.sky = SKY_CLOUDY;
        weather_info.windspeed -= 10;
      }
      break;
    case SKY_FOGGY:
      if (weather_info.mmhg > 970
          || (weather_info.mmhg > 990 && number_bits (2) == 0))
      {
        strcat (buf, "\x1B[30;1mThe fog slowly dissolves... \x1B[0;37mYou can see for miles again!\x1B[37;0m\n\r");
        weather_info.sky = SKY_CLOUDY;
        weather_info.temperature += 5;
      }
      break;
    case SKY_LIGHTNING:
      if (weather_info.mmhg > 1010
          || (weather_info.mmhg > 990 && number_bits (2) == 0))
      {
        if (time_info.month <= 4 || time_info.month >= 17)
        {
          strcat (buf, "\x1B[0;37mThe blizzard subsides.\x1B[37;0m\n\r");
          weather_info.temperature += 10;
        }
        else
 strcat (buf, "\x1B[0;37mThe \x1B[33;1mlightning \x1B[0;37mhas stopped.\x1B[37;0m\n\r");
        weather_info.sky = SKY_RAINING;
        weather_info.windspeed -= 10;
        break;
      }
      break;
  }

  /* Examine all mobs. */
  for (wch = char_list; wch != NULL; wch = ch_next)
  {
    if (!wch->next)
      ch_next = NULL;
    else
      ch_next = wch->next;
    if (!wch->in_room)
      continue;
    /*if (IS_PLAYER (wch) && IS_AFFECTED_EXTRA (wch, AFF_INSANITY))
    {
      short ow;
      ow = number_range (1, 8);
      switch (ow)
      {
        case 1:
          do_sleep (wch, "");
          break;
        case 2:
          send_to_char ("Wham!  You slap yourself HARD, just for the hell of it!\n\r", wch);
          if (wch->hit < 10)
            wch->hit = 2;
          else
            SUBHIT (wch, 9);
          break;
        case 3:
          check_social (wch, "insane", "");
          break;
        case 4:
          send_to_char ("You feel very hungry for human flesh!!\n\r", wch);
          break;
        case 5:
          {
            CHAR_DATA *c;
            for (c = wch->in_room->more->people; c != NULL;
                c = c->next_in_room)
            {
              if (c == wch)
                continue;
              goto gg;
            }
gg:
            if (c != NULL)
              check_social (wch, "lick", NAME (c));
            else
              check_social (wch, "scream", "");
          }
          break;
        case 6:
          if (number_range (1, 2) == 1)
            do_say (wch, "You will all perish in flames!!");
          else
            do_say (wch, "My pants are on fire! HELP!");
          break;
        default:
          break;
      }
    }*/
    if (IS_PLAYER (wch)
        && (wch->pcdata->no_quit > 0 || wch->pcdata->no_quit_pk > 0))
    {
      if (wch->pcdata->no_quit_pk > 0
          && (FIGHTING (wch) == NULL
            || wch->position != POSITION_FIGHTING))
        wch->pcdata->no_quit_pk--;
      if (wch->pcdata->no_quit > 0
          && (FIGHTING (wch) == NULL
            || wch->position != POSITION_FIGHTING))
        wch->pcdata->no_quit--;
    }
    if (IS_PLAYER (wch) && wch->pcdata->jail_served != 0)
    {
      if (wch->in_room->vnum != wch->in_room->area->jail)
      {
        send_to_char ("How did you manage to get out of JAIL!?\n\r", wch);
        wch->pcdata->bounty += 25;
        wch->pcdata->jail_served = 0;
        wch->pcdata->warned = 0;
      }
      else
      {
        wch->pcdata->jail_served++;
        wch->pcdata->bounty -= 3;
        if (wch->pcdata->bounty <= 0)
        {
          /*gets outta prison */
          if ((rooom = get_room_index (wch->in_room->area->outjail)) == NULL)
          {
            rooom = get_room_index (1);
          }
          char_from_room (wch);
          char_to_room (wch, rooom);
          wch->pcdata->jail_served = 0;
          wch->pcdata->bounty = 0;
          wch->pcdata->warned = 0;
          send_to_char ("Your sentence is up! You are free!\n\r", wch);
          do_look (wch, "auto");
        }
        else
        {
          sprintf (general_use,
              "Hours in prison: %d. Hours left in prison: %d.\n\r",
              wch->pcdata->jail_served - 1,
              wch->pcdata->bounty / 3);
          send_to_char (general_use, wch);
        }
      }
    }
    if (IS_PLAYER (wch))
    {
      if (weather_info.sky == SKY_LIGHTNING && dice (1, 6000) == 6
          && !lightning && wch->in_room
          && wch->in_room->sector_type != SECT_INSIDE
          && wch->in_room->sector_type != SECT_UNDERWATER)
      {
        send_to_char ("You feel your hair stand on your neck...", wch);
        send_to_char ("ZAP!!\n\r", wch);
        act ("Watch out! $n just got struck by lightning!", wch, NULL, NULL, TO_ROOM);
        if (wch->hit > 31)
          damage (wch, wch, dice (5, 20), TYPE_UNDEFINED);
        if (wch->data_type == 50)
          continue;
        lightning = 1;
      }
      if (buf[0] != '\0')
      {
        if (wch->desc != NULL && wch->desc->connected == CON_PLAYING
            && IS_OUTSIDE (wch)
            && !IS_SET (wch->in_room->room_flags, ROOM_INDOORS)
            && wch->in_room->sector_type != SECT_INSIDE
            && wch->in_room->sector_type != SECT_UNDERWATER
            && IS_AWAKE (wch))
        {
          if (wch->in_room->sector_type == SECT_AIR
              && find_str (buf, "fog"))
            continue;
          if (wch->in_room->sector_type == SECT_SNOW
              && (!find_str (buf, "snow")
                || !find_str (buf, "blizzard")
                || !find_str (buf, "cloud")))
            continue;
          if (wch->in_room->sector_type == SECT_DESERT
              && (find_str (buf, "snow")
                || find_str (buf, "blizzard")))
            continue;
          if (wch->in_room->sector_type == SECT_TROPICAL
              && (find_str (buf, "snow")
                || find_str (buf, "blizzard")))
            continue;
          if (wch->in_room
              && !IS_SET (wch->in_room->room_flags, ROOM_INDOORS)
              && wch->in_room->sector_type != SECT_INSIDE)
            send_to_char (buf, wch);
        }
      }
      continue;
    }
    if (IS_PLAYER (wch))
      continue;
check_a1:
    /* Check for triggers on mobbies! */
    for (tr = trigger_list[TEVERY_HOUR]; tr != NULL; tr = tr->next)
    {
      if (wch->pIndexData->vnum == tr->attached_to_mob)
      {
        if (tr->players_only)
          continue;
        if (tr->running_info && !tr->interrupted)
          continue;	/* Already running, interrupted, but script says not to allow interruptions. */
        if (tr->running_info && tr->interrupted != 2)
        {
          end_script (tr->running_info);
          goto check_a1;
        }
        /* ----------------- */
        /* Start the script! */
        /* ----------------- */
        tr->running_info = mem_alloc (sizeof (*tr->running_info));
        s = tr->running_info;
        bzero (s, sizeof (*s));
        s->current = wch;
        s->mob = wch;
        strcpy (s->code_seg, tr->code_label);
        s->current_line = 0;
        s->called_by = tr;
        s->next = info_list;
        info_list = s;
        execute_code (s);
        /* ----------------- */
      }
    }
    /* End trigger check! */



    /*
       Handles shops creating things - by Owen :)
     */
    if (wch->pIndexData->pShop != NULL
        && wch->pIndexData->pShop->creates_vnum != 0)
    {
      SHOP_DATA *pShop;
      SINGLE_OBJECT *obj;
      SINGLE_OBJECT *ooo;
      OBJ_PROTOTYPE *obj_mask;
      int vnum_same;
      vnum_same = 0;
      pShop = wch->pIndexData->pShop;
      if (pShop->creates_hours == 1
          || (number_range (1, pShop->creates_hours) == 2))
      {
        for (ooo = wch->carrying; ooo != NULL; ooo = ooo->next_content)
          if (ooo->pIndexData->vnum == pShop->creates_vnum)
            vnum_same++;
        if ((obj_mask = get_obj_index (pShop->creates_vnum)) == NULL)
        {
         
          goto skip_over;
        }
        if (obj_mask->item_type == ITEM_FOOD && vnum_same > 24)
          goto skip_over;
        if (obj_mask->item_type != ITEM_FOOD && vnum_same > 4)
          goto skip_over;
        /*Damnit, Jim... I create object now, not a doctor!! */
        {
          obj = create_object (obj_mask, 5);
          obj_to (obj, wch);
          SET_BIT (obj->extra_flags, ITEM_INVENTORY);
          if (pShop->creates_message != NULL)
          {
            sprintf (general_use, "%s", pShop->creates_message);
            act (general_use, wch, NULL, wch, TO_ROOM);
          }
        }
      }
    }
skip_over:
    if (wch->in_room == NULL || wch->in_room->area == NULL || wch->in_room->area->nplayer < 1)
      continue;

    if (IS_MOB (wch) && wch->in_room->more->pcs > 0)
    {

    }
    if (wch->pIndexData->mobtype == MOB_NIGHT && wch->position != POSITION_SLEEPING && time_info.hour > 5 && time_info.hour < 20)
    {
      if (wch->position == POSITION_STANDING)
      {
        act ("$n enters into a deep sleep.", wch, NULL, NULL, TO_ROOM);
        wch->position = POSITION_SLEEPING;
      }
      else
      {
        act ("$n sizzles and smokes as $s flesh burns by the sun!", wch,
            NULL, NULL, TO_ROOM);
        if (wch->hit - 5 < 1)
          wch->hit = 1;
        else
          damage (wch, wch, 5, TYPE_UNDEFINED);
        if (wch->data_type == 50)
          continue;
      }
    }
    if (wch->position == POSITION_SLEEPING
        && IS_SET (wch->pIndexData->act3, ACT3_SLEEP)
        && time_info.hour > 7 && time_info.hour < 21 && dice (1, 3) == 2)
    {
      act ("$n wakes up from $s sleep.", wch, NULL, NULL, TO_ROOM);
      wch->position = POSITION_STANDING;
      continue;
    }
    if (IS_SET (wch->pIndexData->act3, ACT3_SLEEP)
        && (wch->position == POSITION_STANDING
          || wch->position == POSITION_RESTING) && FIGHTING (wch) == NULL
        && (time_info.hour >= 21 || time_info.hour <= 7)
        && dice (1, 3) == 2)
    {
      act ("$n lays down for the night.", wch, NULL, NULL, TO_ROOM);
      wch->position = POSITION_SLEEPING;
      continue;
    }
    if (IS_MOB (wch) && wch->timer > 0)
    {
      if (--wch->timer == 0)
      {
        act ("$n disappears.", wch, NULL, NULL, TO_ROOM);
        act ("Your time has run out.", wch, NULL, NULL, TO_CHAR);
        raw_kill (wch, FALSE);
        continue;
      }
    }
    /* Werebeasts - They transform at 10 pm and 5 am. */
    if (IS_MOB (wch) && wch->ced && wch->ced->original
        && time_info.hour == 5
        && FIGHTING (wch) == NULL
        && wch->in_room->vnum != 1 && wch->pIndexData->mobtype == MOB_LYCAN)
    {
      act ("$n has mysteriously transformed!", wch, NULL, NULL, TO_ROOM);
      pMob = wch->ced->original;
      if (pMob == NULL)
      {
        fprintf (stderr, "Bad transform mob set up.\n");
        continue;
      }
      if (room_is_dark (wch->in_room))
        SET_BIT (pMob->affected_by, AFF_INFRARED);
      pMob->position = wch->position;
      char_from_room (pMob);
      char_to_room (pMob, wch->in_room);
      if (pMob->ced)
        pMob->ced->original = NULL;
      extracted = 1;
      extract_char (wch, TRUE);
    }
    else if (IS_MOB (wch) && wch->pIndexData->alt_vnum != 0
        && time_info.hour == 22
        && wch->in_room->vnum != 1
        && wch->pIndexData->mobtype != MOB_LYCAN
        && FIGHTING (wch) == NULL)
    {
      if ((pMobIndex = get_mob_index (wch->pIndexData->alt_vnum)) == NULL)
      {
        continue;
      }
      act ("$n has mysteriously transformed!", wch, NULL, NULL, TO_ROOM);
      pMob = create_mobile (pMobIndex);
      check_ced (pMob);
      if (room_is_dark (wch->in_room))
        SET_BIT (pMob->affected_by, AFF_INFRARED);
      pMob->position = wch->position;
      char_from_room (pMob);
      char_to_room (pMob, wch->in_room);
      char_from_room (wch);
      char_to_room (wch, get_room_index (1));
      pMob->ced->original = wch;
    }
    /* Night monsters - They come out at 8pm, and hide at 5am. */
    if (!extracted && wch->pIndexData->mobtype == MOB_NIGHT
        && time_info.hour >= 20 && time_info.hour < 5
        && (wch->position == POSITION_SLEEPING))
    {
      act ("$n has awoken from $s slumber.", wch, NULL, NULL, TO_ROOM);
      wch->position = POSITION_STANDING;
    }
  }
  return;
}

/*
   * Update all chars, including mobs.
   * This function is performance sensitive.
 */
void
char_update (void)
{
  CHAR_DATA *ch;
  CHAR_DATA *ch_next;
  time_t save_time;
  CHAR_DATA *plague;
  AFFECT_DATA af;
  SPELL_DATA *spell;
  bzero (&af, sizeof (af));
  if (chars_sent > 1048576)
    {
      meg_sent++;
      chars_sent -= 1048576;
    }
  save_time = current_time;

  LOOP (ch, char_list, ch_next)
  {
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;
    ch_next = ch->next;
    if (ch->desc && ch->desc->connected != CON_PLAYING)
      continue;
    if (ch->in_room == NULL)
      continue;
    if (CHALLENGE (ch) == 10
	&& (ch->in_room->vnum > 507 || ch->in_room->vnum < 504))
      ch->fgt->challenge = 0;
    if (IS_PLAYER (ch))
      {
	SINGLE_OBJECT *obj;
	if (ch->pcdata->challenge_time > 0)
	  ch->pcdata->challenge_time--;
	if (LEVEL (ch) < 100)
	  {
	    ch->timer++;
	    if (ch->timer > 90)
	      {
		if (ch->desc)
		  {
		    close_socket (ch->desc);
		    continue;
		  }
	      }
	    if (ch->timer > 5 && ch->timer < 16)
	      ch->timer = 90;
	    if (number_range (1, 3) == 2)
	      {
		gain_condition (ch, COND_DRUNK, -1);
		if (ch->pcdata->remort_times < 2 && !NO_HUNGER (ch))
		  gain_condition (ch, COND_FULL, -1);
		if (ch->pcdata->remort_times < 3 && !NO_THIRST (ch))
		  gain_condition (ch, COND_THIRST, -1);
	      }
	    if (IS_PLAYER (ch) && ch->in_room->sector_type == SECT_DESERT
		&& time_info.hour >= 11 && time_info.hour <= 17 &&
		!NO_THIRST (ch))
	      {
		send_to_char (" \x1B[1;31mWhew it's hot!\x1B[0;37m", ch);
		gain_condition (ch, COND_THIRST, -2);
	      }
	  }
      }
    LOOP (paf, ch->affected, paf_next)
    {
      paf_next = paf->next;
      if (paf->duration > 0)
	paf->duration--;
      else if (paf->duration < 0);
      else
	{
	  if (paf_next == NULL
	      || paf_next->type != paf->type || paf_next->duration > 0)
	    {
	      spell = skill_lookup (NULL, paf->type);
	      if (paf->type > 0 && spell && spell->wear_off_msg != NULL &&
		  str_cmp (spell->wear_off_msg, "None"))
		{
		  send_to_char (spell->wear_off_msg, ch);
		  send_to_char ("\n\r", ch);
		}
	    }
	  affect_remove (ch, paf);
	}
    }
    /*-------------------------------------------*/
    /* * Careful with the damages here,          */
    /* * MUST NOT refer to ch after damage taken,*/
    /* * as it may be lethal damage (on NPC).    */
    /*-------------------------------------------*/
    if (IS_AFFECTED (ch, AFF_POISON))
      {
	act ("\x1B[0;31m$n shivers and suffers. $n looks \x1B[1;32mpoisoned\x1B[0;31m!\x1B[0;37m", ch, NULL, NULL, TO_ROOM);
send_to_char ("\x1B[0;31mYou feel painfully ill from the affects of the \x1B[1;32mpoison \x1B[0;31min your blood\x1B[0;37m.\n\r", ch);
	if (ch->hit - 2 < 1)
	  {
	    ch->hit = 1;
	    upd_hps (ch);
	  }
	else
	  SUBHIT (ch, 2);
      }
    /*else if (IS_AFFECTED (ch, AFF_PLAGUE) && ch->in_room->more->pcs > 0)
      {
	act ("\x1B[0;33m$n winces in pain as disease gradually rots his body\x1B[0;37m.\n\r", ch, NULL, NULL, TO_ROOM);
	send_to_char ("\x1B[0;33mYou scream in pain as disease slowly rots your body\x1B[0;37m.\n\r", ch);
	LOOP (plague, ch->in_room->more->people, plague->next) if (plague != ch	
                      && !IS_AFFECTED (plague, AFF_PLAGUE)
		      && LEVEL (plague) >= 10
		      && number_range (1, 100) < pow.plague_chance)
	  {
	    bzero (&af, sizeof (af));
	    af.type = gsn_plague;
	    af.duration = 6;
	    af.location = APPLY_STR;
	    af.modifier = -5;
	    af.bitvector = AFF_PLAGUE;
	    renew_affect (plague, &af);
	    send_to_char ("\x1B[0;33mYou feel as if your body is being eaten away from the inside\x1B[0;37m.\n\r", plague);
	    act ("\x1B[0;33m$n's face turns a deathly pale as the plague spreads to $s body\x1B[0;37m.", plague, NULL, NULL, TO_ROOM);
	  }
	if (ch->hit - 8 < 1)
	  {
	    ch->hit = 1;
	    upd_hps (ch);
	  }
	else
	  SUBHIT (ch, 8);
      }*/
  }
  return;
}

void
dummycall ()
{
  return;
}

void
char_only_update (void)
{
  CHAR_DATA *ch;
  CHAR_DATA *ch_next;
  SINGLE_TRIGGER *tr;
  SCRIPT_INFO *s;
  SCRIPT_INFO *scr;
  SCRIPT_INFO *scr_next;

  /* Check running scripts for delay resumes */
  for (scr = info_list; scr != NULL; scr = scr_next)
  {
    scr_next = scr->next;
    if (scr->delay_ticks <= 0)
      continue;
    if (scr->tick_type != 2)
      continue;
    scr->delay_ticks--;
    if (scr->delay_ticks == 0)
    {
      execute_code (scr);
    }
  }



  for (ch = char_list; ch != NULL; ch = ch_next)
  {
    ch_next = ch->next;
    if (ch->desc && ch->desc->connected != CON_PLAYING)
      continue;
    if (ch->position <= POSITION_STUNNED && ch->position >= 0
        && ch->hit > 0)
      update_pos (ch);
    if (!ch->in_room)
      continue;
    if (ch->in_room->sector_type == SECT_WATER_NOSWIM && (!IN_BOAT (ch)) &&
        !IS_SET (ch->act, PLR_HOLYWALK))
    {
      dummycall ();
      if (ch->move > 20)
      {
        send_to_char ("You can barely stay afloat in this deep, rough water!\n\r", ch);
        SUBMOVE (ch, 17);
        continue;
      }
      else
      {
        send_to_char ("You can't manage to stay afloat! You are drowning!\n\r", ch);
        SUBHIT (ch, number_range (5, 15));
        update_pos (ch);
        if (ch->hit < 0)
        {
          strcpy (reasonfd, "Drowning");
          raw_kill (ch, FALSE);
          return;
        }
        upd_hps (ch);
        continue;
      }
    }
    if (IS_PLAYER (ch))
    {
      ch->affected_by |= ch->pcdata->nat_abilities;
      FIXIT (ch) if (ch->pcdata->hours_expired)
      {
        if (ch->pcdata->monthly_pass > 0)
        {
          ch->pcdata->hours_expired = FALSE;
        }
        else
        {
          java_timeup (ch);
          do_quit (ch, NULL);
          continue;
        }
      }
      if (ch->pcdata->monthly_pass > 0
          && ch->pcdata->monthly_expired < monthly_key)
      {
        send_to_char ("Your previous month's monthly pass expired.\n\r", ch);
        send_to_char ("Type credit to see credit information.\n\r", ch);
        ch->pcdata->monthly_expired = monthly_key;
        ch->pcdata->monthly_pass--;
        do_save (ch, "yy2zz11");
      }
      mana_gain (ch, FALSE);
      if (ch->in_room->sector_type == SECT_AIR && FIGHTING (ch) == NULL
          && !IS_SET (ch->act, PLR_HOLYWALK)
          && !IS_AFFECTED (ch, AFF_FLYING)
          && !find_vehicle (ch, SECT_AIR))
      {
        send_to_char ("Oh oh! You are falling to the ground!\n\r", ch);
        act ("$n starts to fall towards the earth!", ch, NULL, NULL,
            TO_ROOM);
        NEW_POSITION (ch, POSITION_FALLING);
        continue;
      }
      if (ch->in_room->sector_type == SECT_UNDERWATER &&
          !IS_AFFECTED (ch, AFF_BREATH_WATER))
      {
        if (!find_vehicle (ch, SECT_UNDERWATER))
        {
          send_to_char ("You are drowning!!\n\r", ch);
          SUBHIT (ch, number_range (6, 17));
          update_pos (ch);
          if (ch->hit < 0)
          {
            strcpy (reasonfd, "Drowning");
            raw_kill (ch, FALSE);
            return;
          }
          upd_hps (ch);
          continue;
        }
      }
    }
    /*if (IS_PLAYER(ch) && LEVEL(ch)<100 && ch->in_room && number_range(1,6)==4) 
      {
      CHAR_DATA *pir;
      if (ch->pcdata->bounty>0 && IS_EVIL(ch)) ch->pcdata->bounty=0;
      for (pir=ch->in_room->more->people;pir!=NULL;pir=pir->next_in_room)
      {
      if (IS_MOB(pir) || pir->level>99 || pir->fighting!=NULL ||
      (pir->position!=POSITION_STANDING && pir->position!=POSITION_RESTING &&
      pir->position!=POSITION_MEDITATING && !IS_AFFECTED(pir,AFF_HIDE))) continue;
      if (IS_EVIL(ch) && !IS_EVIL(pir)) 
      {
      do_stand(ch,""); 
      do_kill(ch,NAME(pir)); 
      break;
      }
      if (!IS_EVIL(ch) && IS_EVIL(pir)) 
      {
      do_stand(ch,""); 
      do_kill(ch,NAME(pir)); 
      break;
      }
      }
      }
     */
    if (IS_MOB (ch))
    {
check_a2:
      /* Check for triggers on mobbies! */
      for (tr = trigger_list[TEVERY_REGENTICK]; tr != NULL; tr = tr->next)
      {
        if (ch->pIndexData->vnum == tr->attached_to_mob)
        {
          if (tr->players_only)
            continue;
          if (tr->running_info && !tr->interrupted)
            continue;	/* Already running, interrupted, but script says not to allow interruptions. */
          if (tr->running_info && tr->interrupted != 2)
          {
            end_script (tr->running_info);
            goto check_a2;
          }
          /* ----------------- */
          /* Start the script! */
          /* ----------------- */
          tr->running_info = mem_alloc (sizeof (*tr->running_info));
          s = tr->running_info;
          bzero (s, sizeof (*s));
          s->current = ch;
          s->mob = ch;
          strcpy (s->code_seg, tr->code_label);
          s->current_line = 0;
          s->called_by = tr;
          s->next = info_list;
          info_list = s;
          execute_code (s);
          /* ----------------- */
        }
      }
      /* End trigger check! */


      /* Scavenge */
      if (IS_SET (ch->act, ACT_SCAVENGER)
          && ch->in_room->more->contents != NULL
          && number_range (0, 4) == 1)
      {
        SINGLE_OBJECT *obj;
        SINGLE_OBJECT *obj_best;
        int max;
        max = 1;
        obj_best = NULL;
        for (obj = ch->in_room->more->contents; obj != NULL;
            obj = obj->next_content)
        {
          if (obj->in_room
              && IS_SET (obj->pIndexData->wear_flags, ITEM_TAKE)
              && CAN_WEAR (obj, ITEM_TAKE) && obj->cost > max)
          {
            obj_best = obj;
            max = obj->cost;
          }
        }
        if (obj_best != NULL && obj_best->in_room == ch->in_room
            && IS_SET (obj_best->pIndexData->wear_flags, ITEM_TAKE))
        {
          act ("$n picks up $p.", ch, obj_best, ch, TO_ROOM);
          obj_from (obj_best);
          obj_to (obj_best, ch);
          if (obj_best->in_room == NULL)
          {
            wear_obj (ch, obj_best, FALSE, 0);
          }
        }
      }
      if (IS_SET (ch->pIndexData->act3, ACT3_REGENERATE)
          && number_range (1, 7) == 2)
      {
        if (ch->hit < ch->max_hit)
        {
          int healnum;
          act ("$n regenerates.", ch, NULL, NULL, TO_ROOM);
          healnum = .20 * (float) (ch->max_hit - ch->hit);
          ADDHIT (ch, number_range (healnum, (int) (healnum * 1.5)));
          update_pos (ch);
          if (ch->hit > ch->max_hit)
            MAXHIT (ch);
        }
      }
    }
    fix_char (ch);
    if (ch->hit < ch->max_hit)
    {
      ADDHIT (ch, hit_gain (ch));
      upd_hps (ch);
    }
    if (IS_PLAYER (ch) && number_range (1, 11) == 4)
    {
      check_rating (ch);
      do_save (ch, "yy2zz11");
      //save_char_tro (ch);
    }
    if (ch->move < ch->max_move)
    {
      ADDMOVE (ch, move_gain (ch));
      upd_mps (ch);
    }
    if (ch->hit == -1)
    {
      NEW_POSITION (ch, POSITION_STANDING);
      ch->hit = 1;
    }
    if (ch->position == POSITION_STUNNED)
      update_pos (ch);
    if (number_range (1, 3) > 1 && check_body_temp (ch))
      continue;
    if (IS_PLAYER (ch) && ch->timer > 93 && ch->pcdata->no_quit < 1)
    {
      do_quit (ch, "specqa");
      continue;
    }
  }
  return;
}

void
transport_update (void)
{
  CHAR_DATA *ch;
  EXIT_DATA *pexit;
  CHAR_DATA *ch_next;
  ROOM_DATA *to_room;
  ROOM_DATA *roomav;
  ROOM_DATA *roomstink;
  bool avalancheoff;
  CHAR_DATA *victim;
  SCRIPT_INFO *scr;
  SCRIPT_INFO *scr_next;
  SINGLE_TRIGGER *tr;
  SCRIPT_INFO *s;
  int door;
  char general_use[500];
  bool FLAGGYSET;
  FLAGGYSET = FALSE;
  avalancheoff = FALSE;
  roomstink = NULL;
  roomav = NULL;
  ch_next = NULL;
  check_background_processes ();

/* Check running scripts for delay resumes */
  for (scr = info_list; scr != NULL; scr = scr_next)
    {
      scr_next = scr->next;
      if (scr->delay_ticks <= 0)
	continue;
      if (scr->tick_type != 1)
	continue;
      scr->delay_ticks--;
      if (scr->delay_ticks == 0)
	{
	  execute_code (scr);
	}
    }

  for (ch = char_list; ch != NULL; ch = ch_next)
    {
      ch_next = ch->next;
      if (IS_PLAYER(ch))
        {
          if (IS_AFFECTED (ch, AFF_WOUND))
            {
             if (number_range(1,10)==5)
                send_to_char("\x1B[0;31mYour wounds continue to bleed, leaving you in extreme pain!\x1B[0;37m\n\r", ch);
              if (number_range(1,20)==1)
                act ("\x1B[0;31m$n screams in pain as their wounds continue to blead.\x1B[0;37m\n\r", ch, NULL, NULL, TO_ROOM);
	      SUBHIT(ch, 10);
              if (ch->hit < -10) raw_kill (ch, TRUE);
            }
          if (IS_AFFECTED (ch, AFF_REGENERATE))
            {
              if (ch->hit < ch->max_hit-5)
                {
                  ADDHIT(ch, number_range(3,10));
                  if (number_range(1,10)==1)
                    send_to_char ("Your wounds close and your bones fuse back together.\n\r", ch);
                }
              if (ch->move < ch->max_move)
                {
                  ADDMOVE(ch, 1);
                  if (IS_PLAYER(ch) && !strcmp(race_info[ch->pcdata->race].name, "behemoth"))
                     ADDMOVE (ch, number_range(10, 30));
                }
            }
      if (IS_MOB (ch))
	{
	check_a3:
/* Check for triggers on mobbies! */
	  for (tr = trigger_list[TEVERY_QUICKTICK]; tr != NULL; tr = tr->next)
	    {
	      if (ch->pIndexData->vnum == tr->attached_to_mob)
		{
		  if (tr->players_only)
		    continue;
		  if (tr->running_info && !tr->interrupted)
		    continue;	/* Already running, interrupted, but script says not to allow interruptions. */
		  if (tr->running_info && tr->interrupted != 2)
		    {
		      end_script (tr->running_info);
		      goto check_a3;
		    }
		  /* ----------------- */
		  /* Start the script! */
		  /* ----------------- */
		  tr->running_info = mem_alloc (sizeof (*tr->running_info));
		  s = tr->running_info;
		  bzero (s, sizeof (*s));
		  s->current = ch;
		  s->mob = ch;
		  strcpy (s->code_seg, tr->code_label);
		  s->current_line = 0;
		  s->called_by = tr;
		  s->next = info_list;
		  info_list = s;
		  execute_code (s);
		  /* ----------------- */
		}
	    }
/* End trigger check! */



	  if (!IS_SET (ch->act, ACT_SENTINEL)
	      && IS_SET (ch->act, ACT_SCRAMBLE)
	      && (door = number_range (0, 7)) <= 5
	      && (pexit = ch->in_room->exit[door]) != NULL
	      && (ROOM_DATA *) pexit->to_room != NULL
	      && (!pexit->d_info || !IS_SET (pexit->d_info->exit_info, EX_CLOSED))
	      && !IS_SET (((ROOM_DATA *) pexit->to_room)->room_flags, ROOM_NO_MOB) && (!IS_SET (ch->act, ACT_STAY_AREA) 
              || ((ROOM_DATA *) pexit->to_room)-> area == ch->in_room->area))
	    {
	      move_char (ch, door);
	    }

	  if (ch->ced)
	    {
	      if (ch->ced->scr_wait > 0
		  && ch->ced->wait_type == WAIT_TYPE_SMALL_TICK)
		{
		  ch->ced->scr_wait--;
		  if (ch->ced->scr_wait == 0)
		    {
		      do_scriptline (ch, NULL, NULL);
		    }
		}
	      check_short_tick_script (ch, ch);
	    }

	  if (HUNTING (ch) != NULL && ch->position != POSITION_FIGHTING && ch->position != POSITION_BASHED && ch->position >= POSITION_RESTING)
	    {
	      char buf[200];
	      if ((victim = get_char_room (ch, HUNTING (ch))) == NULL)
		continue;
	      if (FIGHTING (ch) != NULL || ch->position == POSITION_FIGHTING || ch->position == POSITION_GROUNDFIGHTING)
		continue;
	      if (!IS_SET (ch->pIndexData->act3, ACT3_MUTE) && (FIGHTING (victim) != NULL && victim->fgt->fighting->position != POSITION_GROUNDFIGHTING))
		{
		  if (ch->pIndexData->will_help && ch->pIndexData->yeller_number != 0)
		    {
		      sprintf (buf, "You shall pay for your cruelty!!");
		      if (can_yell (ch))
			do_say (ch, buf);
		    }
		  else
		    {
		      sprintf (buf, "There you are, %s, but not for long!", NAME (victim));
		      if (can_yell (ch))
			do_yell (ch, buf);
		    }
		}
	      set_fighting (ch, victim);
	    }
	  ch->wait -= 13;
	  if (ch->wait < 0)
	    ch->wait = 0;
	}

      if (IS_MOB (ch) && FIGHTING (ch) == NULL && number_range (1, 3) == 2 && HUNTING (ch) != NULL 
                      && (is_number (ch->fgt->hunting) || !IS_SET (ch->act, ACT_SENTINEL)))
	hunt_victim (ch);
      if (ch->in_room->area->nplayer < 1)
	continue;
      if (IS_PLAYER (ch))
	{
	  if (FIGHTING (ch) == NULL
	      && ch->position == POSITION_GROUNDFIGHTING)
	    NEW_POSITION (ch, POSITION_STANDING);
	  if (FIGHTING (ch) == NULL && ch->position == POSITION_FIGHTING)
	    NEW_POSITION (ch, POSITION_STANDING);
	  if (IS_IMMORTAL (ch) && !IS_SET (ch->pcdata->act2, PLR_HOLYLAG))
	    ch->wait = 0;
	  ch->wait -= 8;
	  if (ch->wait < 0)
	    ch->wait = 0;
	  if (ch->position == POSITION_FALLING
	      && (IS_AFFECTED (ch, AFF_FLYING) || IS_SET (ch->act, PLR_HOLYWALK)))
	    {
	      send_to_char ("You stop falling.\n\r", ch);
	      act ("$n stops falling.", ch, NULL, NULL, TO_ROOM);
	      NEW_POSITION (ch, POSITION_STANDING);
	      continue;
	    }
	  else if (ch->position == POSITION_FALLING)
	    {
	      if (ch->in_room->exit[DIR_DOWN] &&
		  (ROOM_DATA *) ch->in_room->exit[DIR_DOWN]->to_room != NULL
		  && ch->in_room->sector_type == SECT_AIR)
		{
		  send_to_char ("Still falling...\n\r", ch);
		  act ("$n falls right by you.", ch, NULL, NULL, TO_ROOM);
		  to_room = ch->in_room->exit[DIR_DOWN]->to_room;
		  char_from_room (ch);
		  char_to_room (ch, to_room);
		  do_look (ch, "auto");
		  act ("$n falls from above...", ch, NULL, NULL, TO_ROOM);
		  continue;
		}
	      else
		{
		  send_to_char ("OUCH! You land with a loud THUD!\n\r", ch);
		  act ("$n slams into the ground!", ch, NULL, NULL, TO_ROOM);
		  NEW_POSITION (ch, POSITION_STANDING);
		  if (ch->hit > 25)
		    {
		      SUBHIT (ch, number_range (14, 20));
		      upd_hps (ch);
		    }
		  if (ch->position >= POSITION_STUNNED)
		    NEW_POSITION (ch, POSITION_RESTING);
		  continue;
		}
	    }
	}
      if (ch->in_room->more && ch->in_room->more->pcs > 0)
	{
	  if (ch->in_room->room_flags >= ROOM_ACIDSPRAY)
	    /*{
	      if (IS_PLAYER (ch) && IS_AFFECTED (ch, AFF_NOXIOUS)
		  && !FLAGGYSET
		  && !IS_SET (ch->in_room->room_flags, ROOM_GASTRAPONE))
		{
		  ch->in_room->room_flags ^= ROOM_GASTRAPONE;
		  roomstink = ch->in_room;
		  FLAGGYSET = TRUE;
		}*/

	      if (IS_PLAYER (ch) && IS_SET (ch->in_room->room_flags, ROOM_MANADRAIN))
		{
		  mana_gain (ch, -25);
		}
	      if (IS_PLAYER (ch) && IS_SET (ch->in_room->room_flags, ROOM_EXTRAMANA))
		{
		  mana_gain (ch, TRUE);
		}
	      if (IS_SET (ch->in_room->room_flags, ROOM_EXTRAHEAL))
		{
		  if (ch->hit < ch->max_hit && ch->position >= POSITION_STANDING)
		    {
		      ADDHIT (ch, 5);
		      upd_hps (ch);
		    }
		}
	      if (IS_PLAYER (ch) && IS_SET (ch->act, PLR_HOLYWALK))
		continue;
	      if (number_range (1, 2) == 2)
		{
		  if (IS_SET (ch->in_room->room_flags, ROOM_FIRETRAP))
		    {
		      int chn;
		      if (IS_MOB (ch))
			chn = (LEVEL (ch) / (double) 1.5);
		      else
			chn = get_curr_dex (ch) + get_curr_int (ch);
		      if (number_range (1, 55) > chn)
			{
			  act ("Searing flames rise up around you, burning your flesh!", ch, NULL, NULL, TO_CHAR);
			  act ("$N is engulfed in flames!", ch, NULL, ch, TO_ROOM);
			  SUBHIT (ch, number_range (1, 30));
			  update_pos (ch);
			  if (ch->position == POSITION_DEAD)
			    {
			      strcpy (reasonfd, "Fire");
			      raw_kill (ch, FALSE);
			      continue;
			    }
			  upd_hps (ch);
			}
		      else
			act ("Searing flames rise up around you, but you quickly move out of the way!!", ch, NULL, NULL, TO_CHAR);
		    }
		  if (IS_SET (ch->in_room->room_flags, ROOM_ROCKTRAP))
		    {
		      if (number_range (0, 2) == 1)
			{
			  int chn;
			  int dam;
			  if (IS_MOB (ch))
			    chn = (LEVEL (ch) / (double) 1.5);
			  else
			    chn = get_curr_dex (ch) + get_curr_int (ch);
			  if (number_range (1, 55) > chn)
			    {
			      act ("Large rocks fall from above, hitting you on the head!", ch, NULL, NULL, TO_CHAR);
			      act ("$N is smashed on the head by large rocks!", ch, NULL, ch, TO_ROOM);
			      dam = number_range (1, 50);
			      if (dam < 1)
				dam = 1;
			      SUBHIT (ch, dam);
			      update_pos (ch);
			      if (ch->position == POSITION_DEAD)
				{
				  strcpy (reasonfd, "Falling rocks");
				  raw_kill (ch, FALSE);
				  continue;
				}
			      upd_hps (ch);
			    }
			  else
			    act ("Your quick thinking and fast action allows you to dodge a bunch of\n\rlarge rocks that fall from the ceiling!",ch, NULL, NULL, TO_CHAR);
			}
		    }
		  if (IS_SET (ch->in_room->room_flags, ROOM_ARROWTRAP) ||
		      IS_SET (ch->in_room->room_flags, ROOM_POISONTRAP))
		    {
		      int chn;
		      int dam;
		      if (IS_MOB (ch))
			chn = (LEVEL (ch));
		      else
			chn =
			  get_curr_dex (ch) + get_curr_wis (ch) +
			  get_curr_int (ch);
		      if (number_range (1, 80) > chn)
			{
			  act ("Small arrows seem to shoot out at you from nowhere. OUCH!", ch, NULL, NULL, TO_CHAR);
			  act ("$N is pierced by a small barage of arrows!", ch, NULL, ch, TO_ROOM);
			  dam = number_range (1, 25);
			  if (ch->armor < -60)
			    {
			      act("Your superb armor shrugs off most of the puny arrows!", ch, NULL, NULL, TO_CHAR);
			      dam -= 15;
			    }
			  else if (ch->armor < 10)
			    {
			      act("Your armor lessens the penetration of the arrows!", ch, NULL, NULL, TO_CHAR);
			      dam -= 6;
			    }
			  if (dam < 1)
			    dam = 1;
			  SUBHIT (ch, dam);
			  update_pos (ch);
			  upd_hps (ch);
			  if (ch->position == POSITION_DEAD)
			    {
			      raw_kill (ch, FALSE);
			      continue;
			    }
			  if (IS_PLAYER (ch) && (!IS_SET (ch->in_room->room_flags, ROOM_POISONTRAP)))
			    {
			      AFFECT_DATA af;
			      af.type = gsn_poison;
			      af.duration = number_range (2, 10);
			      af.modifier = -2;
			      if (IS_AFFECTED (ch, AFF_POISON))
				{
				  af.duration = 1;
				  af.modifier = 0;
				}
			      af.location = APPLY_STR;
			      af.bitvector = AFF_POISON;
			      renew_affect (ch, &af);
			      act ("You feel very sick! Those arrows were poison-tipped!", ch, NULL, NULL, TO_CHAR);
			    }
			}
		      else
			act ("Arrows suddenly fire at you from out of nowhere, but your quick thinking\n\rand fast reflexes allow you to dodge them!", ch, NULL, NULL, TO_CHAR);
		    }
		  if (IS_SET (ch->in_room->room_flags, ROOM_SPIKETRAP)
		      && number_range (1, 3) == 2)
		    {
		      int chn;
		      int dam;
		      if (IS_MOB (ch))
			chn = (LEVEL (ch));
		      else
			chn =
			  get_curr_dex (ch) + get_curr_wis (ch) +
			  get_curr_int (ch);
		      if (number_range (1, 80) > chn)
			{
			  act ("You stumble and fall on some nasty spikes!", ch, NULL, NULL, TO_CHAR);
			  act ("$N stumbles and lands hard on some spikes!", ch, NULL, ch, TO_ROOM);
			  dam = number_range (1, 35);
			  if (ch->armor < -80)
			    {
			      act ("Your superb armor prevents penetration!", ch, NULL, NULL, TO_CHAR);
			      dam -= 25;
			    }
			  else if (ch->armor < 5)
			    {
			      act ("Your armor lessens the penetration of the spikes!", ch, NULL, NULL, TO_CHAR);
			      dam -= 8;
			    }
			  if (dam < 1)
			    dam = 1;
			  SUBHIT (ch, dam);
			  update_pos (ch);
			  upd_hps (ch);
			  if (ch->position == POSITION_DEAD)
			    {
			      raw_kill (ch, FALSE);
			      continue;
			    }
			}
		    }
		  if (IS_SET (ch->in_room->room_flags, ROOM_GASTRAPONE))/* || IS_SET (ch->in_room->room_flags, ROOM_GASTRAPTWO))*/
		    {
		      int dam;
		      if (number_range (1, 2) == 1)
			{
			  act ("Horrible noxious gasses which hang in the air here make your lungs burn!", ch, NULL, NULL, TO_CHAR);
			  act ("$N coughs uncontrollably and $s eyes water!", ch, NULL, ch, TO_ROOM);
			  if (IS_SET (ch->in_room->room_flags, ROOM_GASTRAPONE))
			    {
                              /*ch->in_room->room_flags^=ROOM_GASTRAPONE; */
			      dam = number_range (1, 4);
			    }
			  else
			    dam = number_range (1, 35);
			  SUBHIT (ch, dam);
			  update_pos (ch);
			  upd_hps (ch);
			  if (ch->position == POSITION_DEAD)
			    {
			      raw_kill (ch, FALSE);
			      continue;
			    }
			  /*if (IS_SET (ch->in_room->room_flags, ROOM_GASTRAPTWO))
			    {
			      AFFECT_DATA af;
			      af.type = gsn_poison;
			      af.duration = number_range (2, 10);
			      af.modifier = -2;
			      if (IS_AFFECTED (ch, AFF_POISON))
				{
				  af.duration = 1;
				  af.modifier = 0;
				}
			      af.location = APPLY_STR;
			      af.bitvector = AFF_POISON;
			      renew_affect (ch, &af);
			      act ("You feel very sick! The gas must be poisonous!", ch, NULL, NULL, TO_CHAR);
			    }*/
			}
		    }
		}
	      if (IS_SET (ch->in_room->room_flags, ROOM_AVALANCHE))
		{
		  int dam;
		  avalancheoff = TRUE;
		  roomav = ch->in_room;
		  act ("You are smashed by huge chunks of ice and snow!\n\rAVALANCHE!!!!", ch, NULL, NULL, TO_CHAR);
		  dam = number_range (10, 55);
		  if (number_range (1, 3) == 2)
		    {
		      act ("Snow completely covers you! You can't breath!!!", ch, NULL, NULL, TO_CHAR);
		      dam += number_range (20, 40);
		    }
		  SUBHIT (ch, dam);
		  update_pos (ch);
		  upd_hps (ch);
		  if (ch->position == POSITION_DEAD)
		    {
		      raw_kill (ch, FALSE);
		      continue;
		    }
		}
	      if (IS_SET (ch->in_room->room_flags, ROOM_GYSERTRAP)
		  && number_range (1, 10) == 2)
		{
		  int dam;
		  act ("You are sprayed by scalding hot water!!!", ch, NULL, NULL, TO_CHAR);
		  dam = number_range (10, 40);
		  SUBHIT (ch, dam);
		  update_pos (ch);
		  upd_hps (ch);
		  if (ch->position == POSITION_DEAD)
		    {
		      raw_kill (ch, FALSE);
		      continue;
		    }
		}
	      if (FLAGGYSET
		  && IS_SET (roomstink->room_flags, ROOM_GASTRAPONE))
		roomstink->room_flags ^= ROOM_GASTRAPONE;
	      if (avalancheoff)
		roomav->room_flags ^= ROOM_AVALANCHE;
	    }
	  /*End traps */
	  if (IS_MOB (ch) && (FIGHTING (ch) == NULL && IS_SET (ch->act, ACT_SENTINEL)))
	    continue;
	  if (IS_SET (ch->in_room->room_flags, ROOM_MOVING)
	      && ch->in_room->exit[ch->in_room->more->move_dir] != NULL
	      && ch->in_room->more
	      && (ROOM_DATA *) ch->in_room->exit[ch->in_room->more-> move_dir]->to_room != NULL
	      && !IS_AFFECTED (ch, AFF_FLYING) && FIGHTING (ch) == NULL
	      && (LEVEL (ch) < 99 || !IS_SET (ch->act, PLR_HOLYWALK))
	      && ch->position != POSITION_FALLING && ch->in_room->more)
	    {
	      if (ch->in_room->more->move_message && ch->in_room->more->move_message[0] != '\0')
		{
		  sprintf (general_use, "You are %s", ch->in_room->more->move_message);
		  send_to_char (general_use, ch);
		  sprintf (general_use, "$n is %s", ch->in_room->more->move_message);
		  act (general_use, ch, NULL, NULL, TO_ROOM);
		}
	      else
		{
		  send_to_char ("You feel yourself being swept away...\n\r", ch);
		  act ("$n is swept right by you.", ch, NULL, NULL, TO_ROOM);
		}
	      to_room =	(ROOM_DATA *) ch->in_room->exit[ch->in_room->more->move_dir]->to_room;
	      char_from_room (ch);
	      char_to_room (ch, to_room);
	      do_look (ch, "auto");
	      act ("$n is thrown into the room.", ch, NULL, NULL, TO_ROOM);
	    }
	}
    }
  return;
}

void
obj_update (void)
{
  SINGLE_OBJECT *obj;
  SINGLE_OBJECT *obj_next;
  SINGLE_OBJECT *objtmp;
  SINGLE_OBJECT *objnxt;
  if (arena_timelimit > -1)
    arena_timelimit--;
  if (arena_timelimit == 0)
    {
      arena_timelimit = -1;
      give_back_money ();
      {
	CHAR_DATA *sch;
	sch = create_mobile (get_mob_index (1));
	char_to_room (sch, get_room_index (2));
	do_cancel (sch, "");
	extract_char (sch, TRUE);
      }
    }

  if (number_range (1, 3) > 1)
    return;
  for (obj = object_list; obj != NULL; obj = obj_next)
    {
      CHAR_DATA *owner;
      CHAR_DATA *rch;
      char *message;
      if (!obj->next)
	obj_next = NULL;
      else
	obj_next = obj->next;
      if (obj->in_room == NULL && obj->carried_by == NULL
	  && obj->in_obj == NULL)
	continue;


/* Move this to object update */
      if (obj->carried_by && obj->carried_by->in_room &&
	  obj->pIndexData->item_type == ITEM_LIGHT
	  && IS_LIT (obj) && ((I_LIGHT *) obj->more)->max_light > 0)
	{
	  if (--((I_LIGHT *) obj->more)->light_now == 0
	      && obj->carried_by->in_room != NULL)
	    {
	      --obj->carried_by->in_room->light;
	      act ("$p goes out.", obj->carried_by, obj, NULL, TO_ROOM);
	      act ("$p goes out.", obj->carried_by, obj, NULL, TO_CHAR);
	      REMOVE_BIT (((I_LIGHT *) obj->more)->light_lit, LIGHT_LIT);
	      if (!IS_SET
		  (((I_LIGHT *) obj->more)->light_lit, LIGHT_FILLABLE))
		{
		  free_it (obj);
		  continue;
		}
	    }
	  if (((I_LIGHT *) obj->more)->light_now > 0)
	    {
	      switch (PERCENTAGE
		      (((I_LIGHT *) obj->more)->light_now,
		       ((I_LIGHT *) obj->more)->max_light) / 10)
		{
		case 0:
		  act ("$p belonging to $n, sputters.", obj->carried_by, obj,
		       NULL, TO_ROOM);
		  act ("$p flickers and sputters.", obj->carried_by, obj,
		       NULL, TO_CHAR);
		  break;
		case 1:
		  act ("$p belonging to $n, flickers.", obj->carried_by, obj,
		       NULL, TO_ROOM);
		  act ("$p flickers.", obj->carried_by, obj, NULL, TO_CHAR);
		  break;
		case 2:
		  act ("$p belonging to $n, flickers slightly.",
		       obj->carried_by, obj, NULL, TO_ROOM);
		  act ("$p flickers slightly.", obj->carried_by, obj, NULL,
		       TO_CHAR);
		  break;
		}
	    }
	}

      if (obj->timer <= 0 || --obj->timer > 0)
	continue;

      if ((owner = obj->carried_by) != NULL && owner->in_room
	  && obj->wear_loc != -1)
	unequip_char (owner, obj);
      if (owner && !owner->in_room)
	continue;

      switch (obj->pIndexData->item_type)
	{
	default:
	  message = "$p vanishes.";
	  break;
	case ITEM_FOUNTAIN:
	  message = "$p dries up.";
	  break;
	case ITEM_CORPSE_NPC:
	  message = "$p decays into dust.\x1B[37;0m";
	  break;
	case ITEM_CORPSE_PC:
	  {
	    char name[SML_LENGTH];
	    char *pd;
	    char buffy[200];
	    pd = OOSTR (obj, short_descr);
	    pd = one_argy (pd, name);
	    pd = one_argy (pd, name);
	    pd = one_argy (pd, name);
	    pd = one_argy (pd, name);
	    /*name now equals the name of the person */
	    sprintf (buffy, "rm %s%s.cor &", PLAYER_DIR_2, capitalize (name));
	    system (buffy);

	    if (obj->in_room == NULL)
	      continue;
	    for (objtmp = obj->contains; objtmp != NULL; objtmp = objnxt)
	      {
		objnxt = objtmp->next_content;
		obj_from (objtmp);
		obj_to (objtmp, obj->in_room);
	      }
	    message = "$p decays into dust.";
	    break;
	  }
	case ITEM_FOOD:
	  message = "$p rots.";
	  break;
	case ITEM_BLOOD:
	  message = "$p dries up.";
	  break;
	}
      if (obj->pIndexData->vnum != 40)
	{
	  if (obj->carried_by != NULL)
	    {
	      act (message, obj->carried_by, obj, NULL, TO_CHAR);
	    }
	  else if (obj->in_room != NULL && obj->in_room->more
		   && (rch = obj->in_room->more->people) != NULL)
	    {
	      act (message, rch, obj, NULL, TO_ROOM);
	      act (message, rch, obj, NULL, TO_CHAR);
	    }
	}
      /*get rid of power coin!? */
      if (obj->carried_by != NULL && obj->wear_loc != -1)
	unequip_char (obj->carried_by, obj);
      free_it (obj);
    }
  return;
}

bool
can_yell (CHAR_DATA * ch)
{
  if (IS_PLAYER (ch))
    return TRUE;
  if (ch->pIndexData->mobtype == MOB_PLANT ||
      ch->pIndexData->mobtype == MOB_GENERAL_ANIMAL ||
      ch->pIndexData->mobtype == MOB_DUMMY ||
      ch->pIndexData->mobtype == MOB_INSECT ||
      ch->pIndexData->mobtype == MOB_UNDEAD ||
      ch->pIndexData->mobtype == MOB_BIRD ||
      ch->pIndexData->mobtype == MOB_FISH ||
      ch->pIndexData->mobtype == MOB_CANINE ||
      ch->pIndexData->mobtype == MOB_FELINE ||
      ch->pIndexData->mobtype == MOB_REPTILE ||
      ch->pIndexData->mobtype == MOB_GHOST ||
      ch->pIndexData->mobtype == MOB_RODENT)
    return FALSE;
  return TRUE;
}

void
racehate_message (CHAR_DATA * ch)
{
  int nummy;
  if (IS_PLAYER (ch))
    return;
  if (ch->pIndexData->mobtype == MOB_PLANT ||
      ch->pIndexData->mobtype == MOB_GENERAL_ANIMAL ||
      ch->pIndexData->mobtype == MOB_DUMMY ||
      ch->pIndexData->mobtype == MOB_INSECT ||
      ch->pIndexData->mobtype == MOB_UNDEAD ||
      ch->pIndexData->mobtype == MOB_BIRD ||
      ch->pIndexData->mobtype == MOB_FISH ||
      ch->pIndexData->mobtype == MOB_CANINE ||
      ch->pIndexData->mobtype == MOB_FELINE ||
      ch->pIndexData->mobtype == MOB_REPTILE ||
      ch->pIndexData->mobtype == MOB_GHOST ||
      ch->pIndexData->mobtype == MOB_RODENT)
    return;
  nummy = number_range (1, 3);
  if (nummy == 1)
    do_say (ch, "I don't like your kind! I think I'll just kill you...");
  if (nummy == 2)
    do_say (ch, "All of your kind must die!");
  if (nummy == 3)
    do_say (ch, "You are not of my alignment and must be killed!!!!");
  return;
}

void
aggr_update (void)
{
  CHAR_DATA *wch;
  CHAR_DATA *wch_next;
  CHAR_DATA *ch;
  CHAR_DATA *ch_next;
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  CHAR_DATA *victim;
  SCRIPT_INFO *scr;
  SCRIPT_INFO *scr_next;

/* Check running scripts for delay resumes */
  for (scr = info_list; scr != NULL; scr = scr_next)
    {
      scr_next = scr->next;
      if (scr->tick_type != 4)
	continue;
      if (scr->delay_ticks <= 0)
	continue;
      scr->delay_ticks--;
      if (scr->delay_ticks == 0)
	{
	  execute_code (scr);
	}
    }


  for (wch = aggro_check; wch != NULL; wch = wch_next)
    {
      wch_next = wch->gen_next;
      if (wch->data_type == 50 || wch->data_type == 0)
	{
	  remove_from_aggro_list (wch);
	  continue;
	}
      if (wch->in_room->area->nplayer < 1)
	continue;
      --wch->wait;

      if (!wch->in_room->more || !wch->in_room->more->people
	  || wch->in_room->more->pcs < 1)
	continue;

      for (ch = wch->in_room->more->people; ch && ch != NULL; ch = ch_next)
	{
	  int count;
	  ch_next = ch->next_in_room;
	  if (IS_MOB (ch) && ch->pIndexData->race_hate != 0
	      && FIGHTING (ch) == NULL)
	    {
	      for (vch = wch->in_room->more->people; vch != NULL;
		   vch = vch_next)
		{
		  if (!vch->next_in_room)
		    vch_next = NULL;
		  else
		    vch_next = vch->next_in_room;
		  if (IS_PLAYER (vch) && can_see (ch, vch)
		      && (IS_SET (ch->pIndexData->race_hate, vch->pcdata->raceflag))
		      && LEVEL (vch) < 101 && FIGHTING (ch) == NULL)
		    {
		      racehate_message (ch);
		      set_fighting (ch, vch);
		      continue;
		    }
		}
	    }
	  /*dun: */
	  if (ch->in_room == NULL)
	    continue;
	  if (IS_PLAYER (ch)
	      || (!IS_SET (ch->act, ACT_AGGRESSIVE) && !IS_SET (ch->act, ACT_ANGRY))
	      || FIGHTING (ch) != NULL
	      || IS_AFFECTED (ch, AFF_CHARM)
	      || !IS_AWAKE (ch)
	      || (IS_SET (ch->act, ACT_WIMPY) && IS_AWAKE (wch))
	      || !can_see (ch, wch))
	    continue;


	  if (IS_SET (ch->act, ACT_ANGRY) && (number_range (0, 6) != 1))
	    continue;
	  count = 0;
	  victim = NULL;
	  for (vch = wch->in_room->more->people; vch != NULL; vch = vch_next)
	    {
	      vch_next = vch->next_in_room;
	      if (IS_PLAYER (vch)
		  && LEVEL (vch) < LEVEL_IMMORTAL
		  && (!IS_SET (ch->act, ACT_WIMPY) || !IS_AWAKE (vch))
		  && can_see (ch, vch)
		  /*&& (!IS_AFFECTED( vch, AFF_SNEAK ) */ )
		{
		  if (number_range (0, count) == 0)
		    victim = vch;
		  count++;
		}
	    }
	  if (victim == NULL)
	    {
	      continue;
	    }

	  /* Safety spell reduces chance of getting attacked by aggro */
	  if (IS_AFFECTED (victim, AFF_SAFETY) && LEVEL (ch) < LEVEL (victim) && number_range (1, 4) < 4)
	    continue;

	  if (IS_SET (ch->pIndexData->act3, ACT3_KILL_GOOD))
	    {
	      if (!IS_EVIL (victim))
		act ("$6Those pure of heart must die!$R", ch, NULL, victim, TO_ROOM);
	    }
	  else if (IS_SET (ch->pIndexData->act3, ACT3_KILL_EVIL))
	    {
	      if (IS_EVIL (victim))
		act ("$BAll evil must die!$R", ch, NULL, victim, TO_ROOM);
	    }
	  else
	    {
	      switch (number_bits (2))
		{
		default:
		  act ("$4$n attacks $N!$R", ch, NULL, victim, TO_NOTVICT);
		  act ("$4$n attacks you!$R", ch, NULL, victim, TO_VICT);
		  break;
		case 1:
		  if (can_yell (ch))
		    {
		      act ("$4$n screams and attacks $N!$R", ch, NULL, victim, TO_NOTVICT);
		      act ("$4$n screams and attacks you!$R", ch, NULL, victim, TO_VICT);
		    }
		  break;
		case 2:
		  if (can_yell (ch))
		    {
		      act ("$4$n leaps towards $N!$R", ch, NULL, victim, TO_NOTVICT);
		      act ("$4$n leaps towards you!$R", ch, NULL, victim, TO_VICT);
		    }
		  else
		    {
		      act ("$4$n attacks $N!$R", ch, NULL, victim, TO_NOTVICT);
		      act ("$4$n attacks you!$R", ch, NULL, victim, TO_VICT);
		    }
		  break;
		case 3:
		  act ("$4$N's eyes widen in horror as $n attacks!$R", ch, NULL, victim, TO_NOTVICT);
		  act ("$4$n attacks you!$R", ch, NULL, victim, TO_VICT);
		  break;
		}
	    }
	  multi_hit (ch, victim, TYPE_UNDEFINED);
	}
    }
  return;
}

void
dum (void)
{
  return;
}

/*
   * Reset one room.
 */
void
reset_room (ROOM_DATA * pRoom)
{
  RESET_DATA *pReset;
  CHAR_DATA *pMob;
  SINGLE_OBJECT *pObj;
  CHAR_DATA *LastMob = NULL;
  SINGLE_OBJECT *LastObj = NULL;
  SINGLE_OBJECT *LastObjPrev = NULL;
  int ie = 0;
  bool last;
  int iExit;
  int olevel = 2;
  if (pRoom == NULL)
    return;
  if (get_room_index (pRoom->vnum) == NULL)
    {
      bug ("Reset_room: invalid room %d", pRoom->vnum);
      return;
    }
  pMob = NULL;
  last = TRUE;
  for (iExit = 0; iExit < MAX_DIR; iExit++)
    {
      if (pRoom->exit[iExit] != NULL && pRoom->exit[iExit]->d_info)
	{
	  if (IS_SET (pRoom->exit[iExit]->d_info->rs_flags, EX_ISDOOR)
	      && pRoom->exit[iExit]->d_info->maxstr <= 0)
	    {
	      pRoom->exit[iExit]->d_info->str = 36;
	    }
	  if (pRoom->exit[iExit]->d_info &&
	      pRoom->exit[iExit]->d_info->str < 0
	      && !IS_SET (pRoom->exit[iExit]->d_info->exit_info, EX_RESET))
	    continue;
	  if (pRoom->exit[iExit]->d_info)
	    pRoom->exit[iExit]->d_info->exit_info =
	      pRoom->exit[iExit]->d_info->rs_flags;
	  if (pRoom->exit[iExit]->d_info
	      && IS_SET (pRoom->exit[iExit]->d_info->rs_flags, EX_ISDOOR)
	      && pRoom->exit[iExit]->d_info->maxstr > 0)
	    {
	      pRoom->exit[iExit]->d_info->str =
		pRoom->exit[iExit]->d_info->maxstr;
	    }
	  if ((ROOM_DATA *) pRoom->exit[iExit]->to_room != NULL &&
	      ((ROOM_DATA *) pRoom->exit[iExit]->to_room)->
	      exit[rev_dir[iExit]] != NULL
	      && ((ROOM_DATA *) pRoom->exit[iExit]->to_room)->
	      exit[rev_dir[iExit]]->d_info != NULL)
	    ((ROOM_DATA *) pRoom->exit[iExit]->to_room)->
	      exit[rev_dir[iExit]]->d_info->exit_info =
	      ((ROOM_DATA *) pRoom->exit[iExit]->to_room)->
	      exit[rev_dir[iExit]]->d_info->rs_flags;
	}
    }
  if (!pRoom->more)
    return;
  LastMob = NULL;
  LastObj = NULL;
  for (pReset = pRoom->more->reset_first; pReset != NULL;
       pReset = pReset->next)
    {
      MOB_PROTOTYPE *pMobIndex;
      OBJ_PROTOTYPE *pObjIndex;
      pObj = NULL;
      if (number_percent () > pReset->percent)
	{
	  if (pReset->command == 'M')
	    LastMob = NULL;
	  continue;
	}
      switch (pReset->command)
	{
	default:
	  bug ("Reset_room: bad command %c.", pReset->command);
	  break;
	case 'M':
	  if ((pMobIndex = get_mob_index (pReset->rs_vnum)) == NULL)
	    {
	      bug ("Reset_room: 'M': bad vnum %d.", pReset->rs_vnum);
	      bug ("Reset in room : %d.", pRoom->vnum);
	      continue;
	    }
	  if (pMobIndex->count >= pMobIndex->max_in_world /*pReset->loc */ )
	    {
	      LastMob = NULL;
	      break;
	    }
	  pMob = create_mobile (pMobIndex);
	  if (room_is_dark (pRoom))
	    SET_BIT (pMob->affected_by, AFF_INFRARED);
	  char_to_room (pMob, pRoom);

/* Mob was born, check for triggers on mobbie! */
	checkagain:
	  {
	    SINGLE_TRIGGER *tr;
	    SCRIPT_INFO *s;
	    for (tr = trigger_list[TCREATED]; tr != NULL; tr = tr->next)
	      {
		if (pMob->pIndexData->vnum == tr->attached_to_mob)
		  {
		    if (tr->running_info && !tr->interrupted)
		      continue;	/* Already running, interrupted, but script says not to allow interruptions. */
		    if (tr->running_info && tr->interrupted != 2)
		      {
			end_script (tr->running_info);
			goto checkagain;
		      }
		    /* ----------------- */
		    /* Start the script! */
		    /* ----------------- */
		    tr->running_info = mem_alloc (sizeof (*tr->running_info));
		    s = tr->running_info;
		    bzero (s, sizeof (*s));
		    s->current = pMob;
		    s->mob = pMob;
		    strcpy (s->code_seg, tr->code_label);
		    s->current_line = 0;
		    s->called_by = tr;
		    s->next = info_list;
		    info_list = s;
		    execute_code (s);
		    /* ----------------- */
		  }
	      }
	  }
/* End trigger check! */


	  if (IS_AFFECTED (pMob, AFF_SLEEP))
	    {
	      do_sleep (pMob, "");
	    }
	  if (IS_SET (pMob->pIndexData->act3, ACT3_RIDE))
	    {
	      if (LastMob != NULL && RIDING (LastMob) == NULL)
		{
		  do_mount (pMob, RNAME (LastMob));
		}
	    }
	  if (IS_SET (pMob->pIndexData->act3, ACT3_FOLLOW))
	    {
	      if (LastMob != NULL)
		{
		  add_follower (pMob, LastMob);
		}
	    }
	  LastObj = NULL;
	  LastObjPrev = NULL;
	  LastMob = pMob;
	  olevel = LEVEL (pMob);
#ifdef NEW_WORLD
	  handle_points (pMobIndex, pMob);
#endif
	  if (pMobIndex->vnum < 10000 && g_mobpop[pMobIndex->vnum])
	    {
	      int iet;
	      bool fndit;
	      dum ();
	    tti:
	      fndit = FALSE;
	      for (; ie < MAX_RANDOMPOP; ie++)
		{
		  if (g_obj_pop[ie] == 0)
		    continue;
		  for (iet = 0; iet < 10; iet++)
		    {
		      if (g_moblist[ie][iet] == pMobIndex->vnum)
			{
			  fndit = TRUE;
			  goto ffjt;
			}
		    }
		}
	    ffjt:
	      if (fndit)
		{
		  if (number_range (1, 254) < g_percent[ie])
		    {
		      if ((pObjIndex = get_obj_index (g_obj_pop[ie])) == NULL)
			{
			  continue;
			}
		      if (pObjIndex->vnum < MAX_LIMITED
			  && pObjIndex->how_many > 0
			  && pObjIndex->how_many <=
			  limited.limited_array[pObjIndex->vnum])
			continue;
		      pObj = create_object (pObjIndex, 7);
		      obj_to (pObj, pMob);
		      wear_obj (pMob, pObj, FALSE, 0);
		      ie++;
		      goto tti;
		    }
		}
	    }

	  break;
	case 'O':
	  pObj = NULL;
	  if ((pObjIndex = get_obj_index (pReset->rs_vnum)) == NULL)
	    {
	      bug ("Reset_room: 'O': bad vnum %d.", pReset->rs_vnum);
	      bug ("Reset in room : %d.", pRoom->vnum);
	      continue;
	    }
	  if (pObjIndex->vnum < MAX_LIMITED && pObjIndex->how_many > 0 &&
	      pObjIndex->how_many <= limited.limited_array[pObjIndex->vnum])
	    continue;
	  if (pReset->loc == RESET_LOC_INROOM)
	    {
	      SINGLE_OBJECT *obj2;
	      int count_in_room;
	      count_in_room = 0;
	      if (pRoom->more)
		for (obj2 = pRoom->more->contents; obj2 != NULL;
		     obj2 = obj2->next_content)
		  {
		    if (obj2->pIndexData->vnum ==
			/*pObj->pIndexData->vnum */ pObjIndex->vnum)
		      count_in_room++;
		  }
	      if (pObjIndex->max_in_room <= count_in_room)
		continue;
	      pObj = create_object (pObjIndex, number_fuzzy (olevel));
	      obj_to (pObj, pRoom);
	      if ( /*LastObj == NULL || */ pObj->pIndexData->item_type ==
		  ITEM_CONTAINER)
		{
		  LastObjPrev = LastObj;
		  LastObj = pObj;
		}
	      continue;
	    }
	  if (pReset->loc == RESET_LOC_INOBJ && LastObj != NULL)
	    {
	      pObj = create_object (pObjIndex, number_fuzzy (olevel));
	      obj_to (pObj, LastObj);
	      continue;
	    }
	  if (pReset->loc == RESET_LOC_ONOBJ && LastObj != NULL)
	    {
	      pObj = create_object (pObjIndex, number_fuzzy (olevel));
	      obj_to (pObj, LastObj);
	      continue;
	    }
	  if (LastMob != NULL)
	    {
	      pObj = create_object (pObjIndex, number_fuzzy (olevel));
	      obj_to (pObj, LastMob);
	      if ( /*LastObj == NULL || */ pObj->pIndexData->item_type ==
		  ITEM_CONTAINER)
		{
		  LastObjPrev = LastObj;
		  LastObj = pObj;
		}
	      /* For Shopkeepers */
	      if (LastMob->pIndexData->pShop != NULL
		  && pObj->wear_loc == WEAR_NONE)
		{
		  /* pObj->wear_loc = URANGE( WEAR_NONE, pReset->loc, MAX_WEAR ); */
		  pObj->wear_loc = -1;
		  SET_BIT (pObj->extra_flags, ITEM_INVENTORY);
		  if (pReset->loc > 1)
		    {
		      int counter;
		      for (counter = 1; counter < pReset->loc; counter++)
			{
			  pObj = NULL;
			  pObj = create_object (pObjIndex, 1);
			  obj_to (pObj, LastMob);
			  pObj->wear_loc = WEAR_NONE;
			  SET_BIT (pObj->extra_flags, ITEM_INVENTORY);
			}
		    }
		  continue;
		}
	      if (pReset->loc != 99)
		wear_obj (LastMob, pObj, FALSE, 0);
	    }
	  break;
	}
      /*END OF CASE */
    }
/*END OF LOOP */
  return;
}

void
do_purgearea (CHAR_DATA * ch, char *argy)
{
  ROOM_DATA *pRoom;
  int ihash;
  DEFINE_COMMAND ("areapurge", do_purgearea, POSITION_DEAD, 110, LOG_NORMAL,
		  "This command performs an invisible purge command in all rooms in the current area.")
    if (IS_MOB (ch))
    return;
  for (ihash = 0; ihash < HASH_MAX; ihash++)
    {
      for (pRoom = room_hash[ihash]; pRoom != NULL; pRoom = pRoom->next)
	{
	  if (ch->in_room->area != pRoom->area)
	    continue;
	  raw_purge (pRoom);
	}
    }
  send_to_char ("Area purged (all objects and mobs).\n\r", ch);
  return;
}

bool temp = FALSE;
/*void
do_setmiw (CHAR_DATA * ch, char *argy)
{
  int i;
  int j;
  MOB_PROTOTYPE *m;
  int hash;
  char tm[500];
  char arg[500];
  DEFINE_COMMAND ("z_setmiw", do_setmiw, POSITION_DEAD, 110, LOG_ALWAYS,
		  "Kilith's use...") argy = one_argy (argy, arg);
  if (!is_number (arg) || !is_number (argy))
    return;
  i = atoi (arg);
  j = atoi (argy);
  sprintf (arg, "%d to %d.\n\r", i, j);
  send_to_char (arg, ch);
  temp = TRUE;
  for (hash = 0; hash < HASH_MAX; hash++)
    {
      for (m = mob_index_hash[hash]; m != NULL; m = m->next)
	{
	  if (m->vnum < i || m->vnum > j)
	    continue;
	  strcpy (tm, itoa (m->vnum));
	  do_mreset (ch, tm);
	  m->max_in_world = ti;
	  SET_BIT (m->area->area_flags, AREA_CHANGED);
	}
    }
  temp = FALSE;
  send_to_char ("Done.\n\r", ch);
  return;
}
*/

void
do_mreset (CHAR_DATA * ch, char *argy)
{
  RESET_DATA *pReset;
  ROOM_DATA *pRoom;
  int ihash;
  int reset_num;
  char general_use[500];
  DEFINE_COMMAND ("mreset", do_mreset, POSITION_DEAD, 110, LOG_NORMAL,
		  "This command can be used to find resets of the specified mob prototype.")
    if (IS_MOB (ch))
    return;
  if (argy == "" || argy[0] == '\0')
    {
      send_to_char ("mreset <vnum> (vnum is mob number to find reset of)\n\r",
		    ch);
      return;
    }
  if (is_number (argy))
    {
      ti = 0;
      reset_num = atoi (argy);
      if (reset_num < 1)
	return;
      for (ihash = 0; ihash < HASH_MAX; ihash++)
	{
	  for (pRoom = room_hash[ihash]; pRoom != NULL; pRoom = pRoom->next)
	    {
	      if (!pRoom->more || pRoom->more->reset_first == NULL)
		continue;
	      for (pReset = pRoom->more->reset_first; pReset != NULL;
		   pReset = pReset->next)
		{
		  if (pReset->rs_vnum == reset_num && pReset->command == 'M')
		    {
		      ti++;
		      if (!temp)
			{
			  sprintf (general_use,
				   "Mob %d reset in room %d at %d chance.\n\r",
				   reset_num, pRoom->vnum, pReset->percent);
			  send_to_char (general_use, ch);
			}
		    }
		}
	    }
	}
    }
  else
    {
      MOB_PROTOTYPE *mmid;
      int iHsh;
      bool found_item;
      bool area_only = TRUE;
      if (str_prefix ("maintenance", argy))
	return;
      if (!str_infix ("all", argy))
	area_only = FALSE;
      for (iHsh = 0; iHsh < HASH_MAX; iHsh++)
	{
	  for (mmid = mob_index_hash[iHsh]; mmid != NULL; mmid = mmid->next)
	    {
	      if (mmid->area->open == 0)
		continue;
	      if (area_only && mmid->area != ch->in_room->area)
		continue;
	      found_item = FALSE;
	      for (ihash = 0; ihash < HASH_MAX; ihash++)
		{
		  for (pRoom = room_hash[ihash]; pRoom != NULL;
		       pRoom = pRoom->next)
		    {
		      if (!pRoom->more || pRoom->more->reset_first == NULL)
			continue;
		      for (pReset = pRoom->more->reset_first; pReset != NULL;
			   pReset = pReset->next)
			{
			  if (pReset->rs_vnum == mmid->vnum
			      && pReset->command == 'M')
			    {
			      found_item = TRUE;
			    }
			}
		      if (found_item)
			goto firstloopbreak;
		    }
		}
	      sprintf (general_use, "Mob %d is unused!\n\r", mmid->vnum);
	      write_to_descriptor2 (ch->desc, general_use, 0);
	      mmid->guard = -32000;
	    firstloopbreak:
	      if (mmid)
		{
		};
	    }
	}
    }
  return;
}

void
do_setmiw (CHAR_DATA * ch, char *argy)
{
  int i;
  int j;
  MOB_PROTOTYPE *m;
  int hash;
  char tm[500];
  char arg[500];
  DEFINE_COMMAND ("z_setmiw", do_setmiw, POSITION_DEAD, 110, LOG_ALWAYS,
                  "Kilith's use...") argy = one_argy (argy, arg);
  if (!is_number (arg) || !is_number (argy))
    return;
  i = atoi (arg);
  j = atoi (argy);
  sprintf (arg, "%d to %d.\n\r", i, j);
  send_to_char (arg, ch);
  temp = TRUE;
  for (hash = 0; hash < HASH_MAX; hash++)
    {
      for (m = mob_index_hash[hash]; m != NULL; m = m->next)
        {
          if (m->vnum < i || m->vnum > j)
            continue;
          strcpy (tm, itoa (m->vnum));
          do_mreset (ch, tm);
          m->max_in_world = ti;
          SET_BIT (m->area->area_flags, AREA_CHANGED);
        }
    }
  temp = FALSE;
  send_to_char ("Done.\n\r", ch);
  return;
}


void
do_oreset (CHAR_DATA * ch, char *argy)
{
  RESET_DATA *pReset;
  ROOM_DATA *pRoom;
  char general_use[500];
  int reset_num;
  int ihash;
  DEFINE_COMMAND ("oreset", do_oreset, POSITION_DEAD, 110, LOG_NORMAL, "This command can be used to find resets of the specified object prototype.")
    if (IS_MOB (ch))
    return;
  if (argy == "" || argy[0] == '\0')
    {
      send_to_char ("oreset <vnum> (vnum is object number to find reset of)\n\r", ch);
      return;
    }
  if (is_number (argy))
    {
      reset_num = atoi (argy);
      if (reset_num < 1)
	return;
      for (ihash = 0; ihash < HASH_MAX; ihash++)
	{
	  for (pRoom = room_hash[ihash]; pRoom != NULL; pRoom = pRoom->next)
	    {
	      if (!pRoom->more || pRoom->more->reset_first == NULL)
		continue;
	      for (pReset = pRoom->more->reset_first; pReset != NULL;
		   pReset = pReset->next)
		{
		  if (pReset->rs_vnum == reset_num && pReset->command == 'O')
		    {
		      sprintf (general_use, "Object %d resets in room vnum %d at %d chance.\n\r", reset_num, pRoom->vnum, pReset->percent);
		      send_to_char (general_use, ch);
		    }
		}
	    }
	}
    }
  else
    {
      int iHsh;
      OBJ_PROTOTYPE *objj;
      bool found_item;
      bool area_only = TRUE;
      if (str_prefix ("maintenance", argy))
	return;
      if (!str_infix ("all", argy))
	area_only = FALSE;
      for (iHsh = 0; iHsh < HASH_MAX; iHsh++)
	{
	  for (objj = obj_index_hash[iHsh]; objj != NULL; objj = objj->next)
	    {
	      if (objj->area->open == 0)
		continue;
	      if (area_only && objj->area != ch->in_room->area)
		continue;
	      found_item = FALSE;
	      for (ihash = 0; ihash < HASH_MAX; ihash++)
		{
		  for (pRoom = room_hash[ihash]; pRoom != NULL;
		       pRoom = pRoom->next)
		    {
		      if (!pRoom->more || pRoom->more->reset_first == NULL)
			continue;
		      for (pReset = pRoom->more->reset_first; pReset != NULL;
			   pReset = pReset->next)
			{
			  if (pReset->rs_vnum == objj->vnum
			      && pReset->command == 'O')
			    {
			      found_item = TRUE;
			    }
			}
		      if (found_item)
			goto firstloopbreak;
		    }
		}
	      sprintf (general_use, "Object %d is unused!\n\r", objj->vnum);
	      write_to_descriptor2 (ch->desc, general_use, 0);
	      objj->item_type = 99999;
	    firstloopbreak:
	      if (objj)
		{
		};
	    }
	}
    }
  return;
}

void
room_update (void)
{
  ROOM_DATA *pRoom;
  CHAR_DATA *pch;
  AREA_DATA *ar;
  int iHash;
  for (ar = area_first; ar != NULL; ar = ar->next)
    {
      ar->repop_counter--;
      if (ar->repop_counter < 1)
	{
	  for (iHash = 0; iHash < HASH_MAX; iHash++)
	    {
	      for (pRoom = room_hash[iHash]; pRoom != NULL;
		   pRoom = pRoom->next)
		{
		  if (pRoom->area != ar)
		    continue;
		  if (ar->repop[0] != '\0' && pRoom->more)
		    {
		      for (pch = pRoom->more->people; pch != NULL;
			   pch = pch->next_in_room)
			{
			  send_to_char (ar->repop, pch);
			}
		    }
		  reset_room (pRoom);
		}
	    }
	  ar->repop_counter = ar->repop_rate;
	}
    }
  return;
}

void
fade_tracks(void)
{
  // Fade tracks, Eraser 9 Feb 2003
  int hash;
  ROOM_DATA *rhash;
  TRACK_DATA *tr_1;
  TRACK_DATA *tr_2;
  TRACK_DATA *prev;
  //put back in
  for (hash = 0; hash < HASH_MAX; hash++)
  {
    for (rhash = room_hash[hash]; rhash != NULL; rhash = rhash->next)
    {
      for (tr_1 = rhash->tracks; tr_1; tr_1 = tr_1->next_track_in_room)
      {
        tr_1->hours--;
      }
    }
  }
  //put back in
  for (hash = 0; hash < HASH_MAX; hash++)
  {
    for (rhash = room_hash[hash]; rhash != NULL; rhash = rhash->next)
    {
      prev = NULL;
      //tr_2 = rhash->tracks ? rhash->tracks->next_track_in_room : NULL;
      tr_2 = rhash->tracks;
      for (tr_1 = rhash->tracks; tr_1 && tr_2; tr_1 = tr_2)
      {
        tr_2 = tr_1->next_track_in_room;
        if (--tr_1->hours < 1)
        {
          if (prev)
            prev->next_track_in_room = tr_2;
          else
            rhash->tracks = tr_2;
          free_m (tr_1);
        }
        else
          prev = tr_1;
      }
      //rhash->tracks = NULL;
    }
  }
}

void
update_handler (void)
{
  if (--transport_upd <= 0)
  {
    transport_upd = PULSE_AREA / 34;
    transport_update ();
  }
  if (--player_upd <= 0)
  {
    int i;
    player_upd = PULSE_AREA / 13;
    if (saving != NULL)
    {
      for (i = 0; i < 7; i++)
      {
        save_area (saving);
        REMOVE_BIT (saving->area_flags, AREA_CHANGED);
        saving = saving->next;
        if (saving == NULL)
        {
          do_global ("Database save finished.\n\r", 0, 0);
          break;
        }
      }
    }
    if (pow.auto_auction)
      auction_update ();
    char_only_update ();
  }
  if (--pulse_mobile <= 0)
  {
    pulse_mobile = PULSE_MOBILE;
    mobile_update ();
  }
  if (--pulse_field <= 0)
  {
    combat_field_update ();
    pulse_field = PULSE_FIELD;
  }
  if (--pulse_violence <= 0)
  {
    if (pow.old_attack)
      pulse_violence = (PULSE_VIOLENCE * 3) / 2;
    else
      pulse_violence = PULSE_VIOLENCE;
    violence_update (FALSE /*flaggy */ );
    if (flaggy)
      flaggy = FALSE;
    else
      flaggy = TRUE;
  }
  if (--pulse_point <= 0)
  {
    CHAR_DATA *mob;
    for (mob = char_list; mob != NULL; mob = mob->next)
    {
      if (mob->in_room == NULL)
        continue;
      if (IS_PLAYER (mob))
        continue;
      if (HUNTING (mob) != NULL && (is_number (mob->fgt->hunting) || !IS_SET (mob->act, ACT_SENTINEL)))
        hunt_victim (mob);
    }
    pulse_point = number_range (PULSE_TICK, 3 * PULSE_TICK / 2);
    fade_tracks(); // moved to here, Ferdi
    char_update();
    obj_update();
    if (yes_log_all)
    {
      fprintf (ffg, "%s\n", ((char *) ctime (&current_time)));
    }
    if (biddata.waitticks == 0)
    {
      biddata.waitticks = -1;
      arena ();
    }
    if (biddata.waitticks > 0)
      biddata.waitticks--;
    weather_update ();
    if (ticks_to_reboot == 0)
    {
      hours_up++;
      /* KILITH - THIS DETERMINES REBOOT TIME */
      /* 450 was 5-8 hour reboots */
      if (hours_up >= 1200 && number_range(1, 50)<=(1+(hours_up-250)/90))
      {
         ticks_to_reboot = 11;
      }
    }
    if (dice (1, 12000) == 1356)
      do_disaster ();
  }
  if (--pulse_aggro < 0)
  {
    aggr_update ();
    pulse_aggro = 5;
  }
  return;
}

void
do_disaster ()
{
  CHAR_DATA *mob;
  AREA_DATA *area;
  area = NULL;
  for (mob = char_list; mob != NULL; mob = mob->next)
    {
      if (number_percent () > 90)
	{
	  if (mob->in_room && mob->in_room->area)
	    area = mob->in_room->area;
	  break;
	}
    }
  if (area == NULL)
    return;
  for (mob = char_list; mob != NULL; mob = mob->next)
    {
      if (mob->in_room == NULL || mob->in_room->area == NULL)
	continue;
      if (mob->in_room->area == area)
	{
	  if (IS_AFFECTED (mob, AFF_FLYING))
	    {
	      send_to_char ("You see the earth shake and rumble below!\n\r", mob);
	      continue;
	    }
	  send_to_char ("The earth shakes and rumbles violently!\n\r", mob);
	  if (dice (3, 10) > get_curr_dex (mob))
	    {
	      send_to_char ("The quake knocks you to the ground.\n\r", mob);
	      act ("$n is knocked down by the quake.", mob, NULL, NULL,
		   TO_ROOM);
	      if (IS_SET (mob->act, PLR_HOLYWALK))
		continue;
	      NEW_POSITION (mob, POSITION_RESTING);
	      if (mob->hit > 11)
		{
		  SUBHIT (mob, 10);
		  upd_hps (mob);
		}
	    }
	}
    }
  return;
}
