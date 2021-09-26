#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"
AFFECT_DATA *affect_free = NULL;
RESET_DATA *reset_free = NULL;
SCRIPT_DATA *script_free = NULL;
VARIABLE_DATA *variable_free = NULL;
int total_fgt = 0;

void affect_modify (CHAR_DATA * ch, AFFECT_DATA * paf, bool fAdd,
		    short flaggy);
extern char *ans_uppercase (const char *txt);

FGT *
new_fgt (void)
{
  FGT *f;
  total_fgt++;
  f = mem_alloc (sizeof (*f));
  f->ears = 2;
  f->pos_x = 0;
  f->pos_y = 0;
  f->combat_delay_count = -1;
  f->next_command = NULL;
  return f;
}

void
clear_fgt (CHAR_DATA * ch)
{
  if (ch->fgt)
     {
      if (ch->fgt->hunting)
	free_string (ch->fgt->hunting);
      if (ch->fgt->next_command)
	{
	  free (ch->fgt->next_command);
	  ch->fgt->next_command = NULL;
	}
    }
  if (ch->fgt)
    {
      free (ch->fgt);
      total_fgt--;
    }
  ch->fgt = NULL;
  return;
}

void
check_fgt (CHAR_DATA * ch)
{
  if (!ch->fgt)
    ch->fgt = new_fgt ();
  return;
}

void
do_coins (CHAR_DATA * ch, char *argy)
{
  int coinage;
  DEFINE_COMMAND ("coins", do_coins, POSITION_DEAD, 100, LOG_ALWAYS,
		  "Creates the specified amount of coins.")
    if (!is_number (argy) || argy[0] == '\0')
    {
      return;
    }
  coinage = atoi (argy);
  if (coinage > 0 && coinage < 1000000)
    create_amount (coinage, ch, NULL, NULL);
  return;
}

void
no_spam (CHAR_DATA * ch, char *argy)
{
  DEFINE_COMMAND ("nospam", no_spam, POSITION_DEAD, 0, LOG_NORMAL,
		  "Toggles between levels of spam.") if (IS_PLAYER (ch))
    {
      ch->pcdata->no_spam = 1;
      send_to_char ("NoSpam mode ENABLED.\n\r", ch);
      send_to_char
	("NOTE: You can toggle through the modes using the SPAM command.\n\r",
	 ch);
    }
  return;
}

void
add_to_aggro_list (CHAR_DATA * ch)
{
  CHAR_DATA *c;
  if (IS_SET (ch->special, IS_IN_AGGRO_LIST))
    return;
  for (c = aggro_check; c != NULL; c = c->gen_next)
    {
      if (c == ch)
	return;
    }
  ch->gen_next = aggro_check;
  aggro_check = ch;
  SET_BIT (ch->special, IS_IN_AGGRO_LIST);
  return;
}



void
spam (CHAR_DATA * ch, char *argy)
{
  DEFINE_COMMAND ("spam", spam, POSITION_DEAD, 0, LOG_NORMAL,
		  "Turns on nospam mode.") if (IS_MOB (ch))
    return;
  if (ch->pcdata->no_spam == 0)
    {
      send_to_char ("Moderate SPAM mode enabled.\n\r", ch);
      ch->pcdata->no_spam = 2;
      return;
    }
  else if (ch->pcdata->no_spam == 1)
    {
      send_to_char ("FULL SPAM mode enabled.\n\r", ch);
      ch->pcdata->no_spam = 0;
      return;
    }
  else
    send_to_char ("NO SPAM mode enabled.\n\r", ch);
  ch->pcdata->no_spam = 1;
  return;
}

int
get_race_look (char *argy)
{
  int i;
  for (i = 0; i < RACE_COUNT; i++)
    {
      if (!str_prefix (argy, race_info[i].name))
	return (1 << i);
    }
  if (!str_cmp (argy, "alien"))
    return RC_EVILRACE;
  if (!str_cmp (argy, "evil"))
    return RC_EVILRACE;
  if (!str_cmp (argy, "good"))
    return RC_GOODRACE;
  return 0;
}

/*
   * Retrieve a character's age.
 */
int
get_age (CHAR_DATA * ch)
{
  return 18 + (((ch->pcdata->played / 2) + (((int) (current_time - ch->pcdata->logon)) / 2)) / 7200) / 4;
}

/*
   * Retrieve character's current strength.
 */
int
get_curr_str (CHAR_DATA * ch)
{
  int max;
  int workspace;
  max = MAX_STAT;
  if (IS_MOB (ch))
    return number_range (20, max);
  workspace = ch->pcdata->perm_stat[STR_I] + ch->pcdata->mod_stat[STR_I];
  if (pow.penalize_evil_in_sun && ch->in_room && IS_EVIL (ch))
    {
      if (IN_SUN (ch))
	workspace -= 2;
    }
  return URANGE (3, workspace, max);
}

/*
   * Retrieve character's current intelligence.
 */
int
get_curr_int (CHAR_DATA * ch)
{
  int max;
  int i;
  max = MAX_STAT;
  if (IS_MOB (ch))
    return number_range (20, max);
  i =
    URANGE (3, ch->pcdata->perm_stat[INT_I] + ch->pcdata->mod_stat[INT_I], max);
  return i;
}

/*
   * Retrieve character's current wisdom.
 */
int
get_curr_wis (CHAR_DATA * ch)
{
  int max;
  max = MAX_STAT;
  if (IS_MOB (ch))
    return number_range (20, max);
  else
    return URANGE (3, ch->pcdata->perm_stat[WIS_I] + ch->pcdata->mod_stat[WIS_I], max);
}

/*
   * Retrieve character's current dexterity.
 */
int
get_curr_dex (CHAR_DATA * ch)
{
  int max;
  int workspace;
  max = MAX_STAT;
  if (IS_MOB (ch))
    return number_range (20, max);
  workspace = ch->pcdata->perm_stat[DEX_I] + ch->pcdata->mod_stat[DEX_I];
  if (pow.penalize_evil_in_sun && ch->in_room && IS_EVIL (ch))
    {
      if (IN_SUN (ch))
	workspace -= 3;
    }
  return URANGE (3, workspace, max);
}

int
get_curr_con (CHAR_DATA * ch)
{
  int max;
  max = MAX_STAT;
  if (IS_MOB (ch))
    return number_range (20, max);
  else
    return URANGE (3, ch->pcdata->perm_stat[CON_I] + ch->pcdata->mod_stat[CON_I], max);
}

bool
is_name (const char *str, char *namelist)
{
  char name[SML_LENGTH];
  if (!str_cmp (str, namelist))
    return TRUE;
  for (;;)
    {
      if (namelist == NULL)
	return FALSE;
      if (!str_cmp (str, namelist))
	return TRUE;
      namelist = one_argy (namelist, name);
      if (name[0] == '\0')
	return FALSE;
      if (!str_cmp (str, name))
	return TRUE;
    }
  return FALSE;
}


bool
is_approx_name (const char *str, char *namelist)
{
  char name[SML_LENGTH];
  if (!str_cmp (str, namelist))
    return TRUE;
  for (;;)
    {
      if (namelist == NULL)
	return FALSE;
      namelist = one_argy (namelist, name);
      if (name[0] == '\0')
	return FALSE;
      if (!str_cmp (str, name))
	return TRUE;
      if (strlen (str) > 1 && !str_prefix (str, name))
	return TRUE;
    }
  return FALSE;
}



void
affect_modify (CHAR_DATA * ch, AFFECT_DATA * paf, bool fAdd, short flaggy)
{
  int mod;
  mod = paf->modifier;
  if (fAdd)
    {
      SET_BIT (ch->affected_by, paf->bitvector);
      SET_BIT (ch->more_affected_by, paf->bitvector2);
    }
  else
    {
      REMOVE_BIT (ch->affected_by, paf->bitvector);
      REMOVE_BIT (ch->more_affected_by, paf->bitvector2);
      mod = 0 - mod;
    }
  if (IS_MOB (ch))
    {
      switch (paf->location)
	{
	default:
	  bug ("Affect_modify: unknown location %d.", paf->location);
	  return;
	case APPLY_NONE:
	  break;
	  if (flaggy == 0)
	    {
	case APPLY_STR:
	      break;
	case APPLY_DEX:
	      break;
	case APPLY_INT:
	      break;
	case APPLY_WIS:
	      break;
	case APPLY_CON:
	      break;
	    }
	  if (flaggy == 0)
	    {
	case APPLY_HIT:
	      ch->max_hit += mod;
	      break;
	case APPLY_MOVE:
	      ch->max_move += mod;
	      break;
        case APPLY_MANA:
              ch->pcdata->n_max_mana +=mod;
              break;
	    }
	case APPLY_AC:
	  ch->armor += mod;
	  break;
	case APPLY_HITROLL:
	  ch->hitroll += mod;
	  break;
	case APPLY_DAMROLL:
	  ch->damroll += mod;
	  break;
	case APPLY_SAVING_THROW:
	  if (IS_PLAYER (ch))
	    ch->pcdata->saving_throw += mod;
	  break;
	case APPLY_RESIST_SUMMON:
	  if (IS_PLAYER (ch))
	    ch->pcdata->resist_summon += mod;
	  break;
	case APPLY_KICK_DAMAGE:
	  if (IS_PLAYER (ch))
	    ch->pcdata->plus_kick += mod;
	  break;
        case APPLY_BACKSTAB_DAMAGE:
          if (IS_PLAYER (ch))
            ch->pcdata->plus_bs += mod;
          break;
        case APPLY_MAGIC_DAMAGE:
          if (IS_PLAYER (ch))
            ch->pcdata->plus_magic += mod;
          break;
        case APPLY_HEAL_BONUS:
          if (IS_PLAYER (ch))
            ch->pcdata->plus_heal += mod;
          break;
	case APPLY_SNEAK:
	  if (IS_PLAYER (ch))
	    ch->pcdata->plus_sneak += mod;
	  break;
	case APPLY_HIDE:
	  if (IS_PLAYER (ch))
	    ch->pcdata->plus_hide += mod;
	  break;
	}
      return;
    }
  switch (paf->location)
    {
    default:
      bug ("Affect_modify: unknown location %d.", paf->location);
      return;
    case APPLY_NONE:
      break;
    case APPLY_STR:
      if (flaggy != 1)
	ch->pcdata->mod_stat[STR_I] += mod;
      break;
    case APPLY_DEX:
      if (flaggy != 1)
	ch->pcdata->mod_stat[DEX_I] += mod;
      break;
    case APPLY_INT:
      if (flaggy != 1)
	ch->pcdata->mod_stat[INT_I] += mod;
      break;
    case APPLY_WIS:
      if (flaggy != 1)
	ch->pcdata->mod_stat[WIS_I] += mod;
      break;
    case APPLY_CON:
      if (flaggy != 1)
	ch->pcdata->mod_stat[CON_I] += mod;
      break;
    case APPLY_HIT:
      if (flaggy != 1)
	ch->max_hit += mod;
      break;
    case APPLY_MOVE:
      if (flaggy != 1)
	ch->max_move += mod;
      break;
     case APPLY_MANA:
      if (flaggy != 1)
        ch->pcdata->n_max_mana += mod;
      break;
    case APPLY_AC:
      ch->armor += mod;
      break;
    case APPLY_HITROLL:
      ch->hitroll += mod;
      break;
    case APPLY_DAMROLL:
      ch->damroll += mod;
      break;
    case APPLY_SAVING_THROW:
      if (IS_PLAYER (ch))
	ch->pcdata->saving_throw += mod;
      break;
    case APPLY_RESIST_SUMMON:
      if (IS_PLAYER (ch))
	ch->pcdata->resist_summon += mod;
      break;
    case APPLY_KICK_DAMAGE:
      if (IS_PLAYER (ch))
	ch->pcdata->plus_kick += mod;
      break;
    case APPLY_BACKSTAB_DAMAGE:
      if (IS_PLAYER (ch))
        ch->pcdata->plus_bs += mod;
      break;
    case APPLY_MAGIC_DAMAGE:
      if (IS_PLAYER (ch))
        ch->pcdata->plus_magic += mod;
      break;
    case APPLY_HEAL_BONUS:
      if (IS_PLAYER (ch))
        ch->pcdata->plus_heal += mod;
      break;
    case APPLY_SNEAK:
      if (IS_PLAYER (ch))
	ch->pcdata->plus_sneak += mod;
      break;
    case APPLY_HIDE:
      if (IS_PLAYER (ch))
	ch->pcdata->plus_hide += mod;
      break;
    }

  if (IS_PLAYER (ch))
    {
      ch->affected_by |= ch->pcdata->nat_abilities;
      ch->affected_by |= race_info[ch->pcdata->race].affect_bits;
    FIXIT (ch)}
  return;
}


void
affect_to_char (CHAR_DATA * ch, AFFECT_DATA * paf)
{
  AFFECT_DATA *paf_new;
  if (IS_PLAYER (ch) && (IN_BATTLE (ch) || CHALLENGE (ch) == 10)
      && paf->type != gsn_sneak)
    return;
  if (IS_PLAYER (ch)
      && (IS_SET (paf->bitvector, AFF_POISON) || paf->type == gsn_poison)
      && race_info[ch->pcdata->race].poison_immunity)
    {
      send_to_char ("You are immune to poison; you resist the effect!\n\r", ch);
      return;
    }
  /*if (IS_PLAYER (ch) && (IS_SET (paf->bitvector, AFF_PLAGUE) || paf->type == gsn_plague)
      && race_info[ch->pcdata->race].plague_immunity)
    {
      send_to_char ("You are immune to plague; you resist the effect!\n\r", ch);
      return;
    }*/
  if (affect_free == NULL)
    {
      paf_new = mem_alloc (sizeof (*paf_new));
    }
  else
    {
      paf_new = affect_free;
      affect_free = affect_free->next;
    }
  *paf_new = *paf;
  paf_new->next = ch->affected;
  ch->affected = paf_new;
  affect_modify (ch, paf_new, TRUE, 0);
  return;
}


void
affect_remove (CHAR_DATA * ch, AFFECT_DATA * paf)
{
  if (ch->affected == NULL)
    {
      bug ("Affect_remove: no affect.", 0);
      return;
    }
  affect_modify (ch, paf, FALSE, 0);
  if (paf == ch->affected)
    {
      ch->affected = paf->next;
    }
  else
    {
      AFFECT_DATA *prev;
      for (prev = ch->affected; prev != NULL;
	   prev = (!prev->next ? NULL : prev->next))
	{
	  if (prev->next == paf)
	    {
	      prev->next = paf->next;
	      break;
	    }
	}
      if (prev == NULL)
	{
	  bug ("Affect_remove: can't find paf.", 0);
	  return;
	}
    }
  paf->next = affect_free;
  affect_free = paf->next;
  return;
}

void
strip_all_affects (CHAR_DATA * ch)
{
  AFFECT_DATA *paf;
  AFFECT_DATA *paf_next;
  for (paf = ch->affected; paf != NULL; paf = paf_next)
    {
      paf_next = paf->next;
      affect_remove (ch, paf);
    }
  ch->affected_by = 0;
  ch->more_affected_by = 0;
  if (IS_PLAYER (ch))
    {
      ch->affected_by |= ch->pcdata->nat_abilities;
      ch->affected_by |= race_info[ch->pcdata->race].affect_bits;
    FIXIT (ch)}
  return;
}

void
affect_strip_bits (CHAR_DATA * ch, int bits)
{
  AFFECT_DATA *paf;
  AFFECT_DATA *paf_next;
  for (paf = ch->affected; paf != NULL; paf = paf_next)
    {
      if (!paf->next)
	paf_next = NULL;
      else
	paf_next = paf->next;
      if (IS_SET (paf->bitvector, bits))
	affect_remove (ch, paf);
    }
  if (IS_PLAYER (ch))
    {
      ch->affected_by |= ch->pcdata->nat_abilities;
      ch->affected_by |= race_info[ch->pcdata->race].affect_bits;
    FIXIT (ch)}
  return;
}

void
affect_strip (CHAR_DATA * ch, int sn)
{
  AFFECT_DATA *paf;
  AFFECT_DATA *paf_next;
  if (sn < 0)
    return;
  for (paf = ch->affected; paf != NULL; paf = paf_next)
    {
      if (!paf->next)
	paf_next = NULL;
      else
	paf_next = paf->next;
      if (paf->type == sn)
	affect_remove (ch, paf);
    }
  if (IS_PLAYER (ch))
    {
      ch->affected_by |= ch->pcdata->nat_abilities;
      ch->affected_by |= race_info[ch->pcdata->race].affect_bits;
    FIXIT (ch)}
  return;
}


bool
is_affected (CHAR_DATA * ch, int sn)
{
  AFFECT_DATA *paf;
  for (paf = ch->affected; paf != NULL; paf = paf->next)
    {
      if (paf->type == sn)
	return TRUE;
    }
  return FALSE;
}

bool
is_affectedt (CHAR_DATA * ch, int bits)
{
  AFFECT_DATA *paf;
  for (paf = ch->affected; paf != NULL; paf = paf->next)
    {
      if (IS_SET (paf->bitvector, bits))
	return TRUE;
    }
  return FALSE;
}


void
affect_join (CHAR_DATA * ch, AFFECT_DATA * paf)
{
  AFFECT_DATA *paf_old;
  bool found;
  if (paf->type < 0)
    return;
  if (IS_PLAYER (ch) && (IN_BATTLE (ch) || CHALLENGE (ch) == 10))
    return;
  found = FALSE;
  for (paf_old = ch->affected; paf_old != NULL; paf_old = paf_old->next)
    {
      if (paf_old->type == paf->type)
	{
	  paf->duration += paf_old->duration;
	  paf->modifier += paf_old->modifier;
	  affect_remove (ch, paf_old);
	  break;
	}
    }
  affect_to_char (ch, paf);
  return;
}

void
remove_from_fighting_list (CHAR_DATA * ch)
{
  CHAR_DATA *c;
  if (IS_SET (ch->special, IS_IN_FIGHTING_LIST))
    REMOVE_BIT (ch->special, IS_IN_FIGHTING_LIST);

  if (ch == f_first)
    {
      f_first = ch->next_fighting;
      ch->next_fighting = NULL;
      return;
    }
  for (c = f_first; c != NULL; c = c->next_fighting)
    {
      if (c->next_fighting == ch)
	{
	  c->next_fighting = ch->next_fighting;
	  ch->next_fighting = NULL;
	  return;
	}
    }

  return;
}


void
add_to_fighting_list (CHAR_DATA * ch)
{
  CHAR_DATA *c;
  CHAR_DATA *cn;
  if (IS_SET (ch->special, IS_IN_FIGHTING_LIST))
    return;
  for (c = f_first; c != NULL; c = cn)
    {
      cn = c->next_fighting;
      if (c == ch)
	{
	  fprintf (stderr,
		   "Found %s already in fighting list without bit set!\n",
		   NAME (c));
	  return;
	}
    }
  ch->next_fighting = f_first;
  f_first = ch;
  SET_BIT (ch->special, IS_IN_FIGHTING_LIST);
  return;
}

void
remove_from_aggro_list (CHAR_DATA * ch)
{
  CHAR_DATA *c;
  if (IS_SET (ch->special, IS_IN_AGGRO_LIST))
    {
      REMOVE_BIT (ch->special, IS_IN_AGGRO_LIST);
    }
  if (ch == aggro_check)
    {
      aggro_check = ch->gen_next;
      ch->gen_next = NULL;
    }
  else
    for (c = aggro_check; c != NULL; c = c->gen_next)
      {
	if (ch == c->gen_next)
	  {
	    c->gen_next = ch->gen_next;
	    ch->gen_next = NULL;
	    return;
	  }
      }
  return;
}

#define XPOS(ch)  ((ch)->pcdata->transport_quest%80)
#define YPOS(ch)  ((ch)->pcdata->transport_quest/80)


void
char_from_room (CHAR_DATA * ch)
{
  CHAR_DATA *c;
  DESCRIPTOR_DATA *d;
  int x, y;
  int can_see_to_min_x;
  int can_see_to_min_y;
  int can_see_to_max_x;
  int can_see_to_max_y;
  SINGLE_OBJECT *obj;
  CHAR_DATA *pir;
  int kkk = 0;
  int iWear;
  if (ch->in_room == NULL)
    {
      return;
    }

  check_room_more (ch->in_room);

  for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
      if (obj->wear_loc != WEAR_NONE
	  && obj->pIndexData->item_type == ITEM_LIGHT
	  && IS_LIT (obj) && ch->in_room->light > 0)
	--ch->in_room->light;
    }


  if (ch->in_room->more->pcs > 0)
    {
      CHAR_DATA *t;
      for (t = ch->in_room->more->people; t; t = t->next_in_room)
	{
	  if (t == ch)
	    continue;
	  if (IS_JAVA (t))
	    {
	      java_remove_mob_stuff (t, ch);
	    }
	}
    }

  if (IS_PLAYER (ch))
    {
      char bb[150];
      if (!(ch->in_room && ch->in_room->x == 0 && ch->in_room->y == 0 && ch->in_room->z == 0 && ch->in_room->vnum != 1508))
	{
	  if (IS_SET (ch->pcdata->act3, ACT3_MAPPING) && !IS_SET (ch->pcdata->act3, ACT3_DISABLED))
	    {
	      if ((ch->pcdata->resist_summon == -10 || ch->pcdata->resist_summon == -15))
		{
		  noibm = ch->pcdata->noibm;
		  kkk = ch->pcdata->kk;
		  mb = ch->pcdata->mb;
		  dbs = ch->pcdata->dbs;
		  if (IS_JAVA (ch))
		    {
		      if (ch->in_room->more->linked_to)
			display_room_mark (ch, ch->in_room->more->linked_to,
					   ch->pcdata->transport_quest % 80,
					   ch->pcdata->transport_quest / 80,
					   0, FALSE, FALSE);
		      else
			display_room_mark (ch, ch->in_room,
					   ch->pcdata->transport_quest % 80,
					   ch->pcdata->transport_quest / 80,
					   0, FALSE, FALSE);
		    }
		  else
		    {
		      if (ch->in_room->more->linked_to)
			display_room_mark (ch, ch->in_room->more->linked_to,
					   ch->pcdata->transport_quest % 80,
					   ch->pcdata->transport_quest / 80,
					   0, TRUE, FALSE);
		      else
			display_room_mark (ch, ch->in_room,
					   ch->pcdata->transport_quest % 80,
					   ch->pcdata->transport_quest / 80,
					   0, TRUE, FALSE);
		    }
		  if (!IS_JAVA (ch))
		    {
		      gotoxy (ch, 1, ch->pcdata->pagelen);
		      send_to_char ("\x1B[37;0m", ch);
		    }
		}
	    }
	  for (d = descriptor_list; d != NULL; d = d->next)
	    {
	      x = 0;
	      y = 0;
	      if ((c = d->character) == NULL)
		continue;
	      if (!c->in_room)
		continue;
	      if (ch == c)
		continue;
	      if (!can_see (c, ch) || not_is_same_align (c, ch))
		continue;
	      if (!IS_SET (c->pcdata->act3, ACT3_MAPPING) || IS_SET (c->pcdata->act3, ACT3_DISABLED))
		continue;
	      if (IS_JAVA (c))
		{
		  can_see_to_min_x = (XPOS (c) - 4) / 2;
		  can_see_to_max_x = (24 - XPOS (c)) / 2;
		  can_see_to_min_y = (YPOS (c) - 1);
		  can_see_to_max_y = (8 - YPOS (c));
		}
	      else
		{
		  can_see_to_min_x = (XPOS (c) - 4) / 2;
		  can_see_to_max_x = (20 - XPOS (c)) / 2;
		  can_see_to_min_y = (YPOS (c) - 1);
		  can_see_to_max_y = (7 - YPOS (c));
		}
	      if (c->in_room->x - can_see_to_min_x > ch->in_room->x)
		continue;
	      if (c->in_room->x + can_see_to_max_x < ch->in_room->x)
		continue;
	      if (c->in_room->y + can_see_to_min_y < ch->in_room->y)
		continue;
	      if (c->in_room->y - can_see_to_max_y > ch->in_room->y)
		continue;
	      if (c->in_room->z != ch->in_room->z || c->in_room == ch->in_room)
		continue;
	      x = (ch->in_room->x - c->in_room->x);
	      y = (c->in_room->y - ch->in_room->y);
	      x *= 2;
	      x += XPOS (c);
	      y += YPOS (c);
	      if (IS_JAVA (c))
		{
		  display_room_mark (c, ch->in_room, x, y, 1, TRUE, IS_MOB (ch));
		}
	      else
		{
		  sprintf (bb, "%c%c%s%s%c%c", '\x1B', '7', display_room_mark_direct (c, ch->in_room, x, y, 1, TRUE, IS_MOB (ch)),
			   "\x1B[37;0m", '\x1B', '8');
		  write_to_descriptor (d->descriptor, bb, 0);
		}
	      c->timer = -5;
	    }
	}
    }

  if (ch == ch->in_room->more->people)
    {
      ch->in_room->more->people = ch->next_in_room;
      if (IS_PLAYER (ch))
	{
	  ch->in_room->area->nplayer--;
	  if (ch->in_room->more->pcs != 31999)
	    ch->in_room->more->pcs--;
	}
    }
  else
    {
      CHAR_DATA *prev;
      for (prev = ch->in_room->more->people; prev; prev = prev->next_in_room)
	{
	  if (prev->next_in_room == ch)
	    {
	      prev->next_in_room = ch->next_in_room;
	      if (IS_PLAYER (ch))
		{

		  ch->in_room->area->nplayer--;
		  if (ch->in_room->more->pcs != 31999)
		    ch->in_room->more->pcs--;


		  /*End IS_PLAYER below */
		}
	      break;
	    }
	}
      if (prev == NULL)
	{
	  /*     bug( "Char_from_room: ch not found.", 0 ); */
	  ch->in_room = NULL;
	  ch->next_in_room = NULL;
	  return;
	}
    }
  if (IS_MOB (ch) && ch->in_room->more->pcs == 0)
    remove_from_aggro_list (ch);
  if (IS_PLAYER (ch))
    {
      for (pir = ch->in_room->more->people; pir != NULL;
	   pir = pir->next_in_room)
	{
	  if (ch->in_room->more->pcs == 0)
	    {
	      remove_from_aggro_list (pir);
	    }
	  if (IS_MOB (pir) && IS_AFFECTED (pir, AFF_CHARM)
	      && MASTER (pir) == ch)
	    {
	      while (ch->pcdata->pet_temp[kkk] != 0)
		kkk++;
	      ch->pcdata->pet_temp[kkk] = pir->pIndexData->vnum;
	      ch->pcdata->pet_hps[kkk] = pir->hit;
	      ch->pcdata->pet_move[kkk] = pir->move;
	    }
	}
    }
  if (FIGHTING (ch))
    {
      CHAR_DATA *vch;
      for (vch = char_list; vch != NULL; vch = vch->next)
	{
	  if (FIGHTING (vch) == ch)
	    {
	      vch->fgt->fighting = NULL;
	      vch->position = POSITION_STANDING;
	    }
	}
      ch->fgt->fighting = NULL;
      ch->position = POSITION_STANDING;
    }
  check_clear_more (ch->in_room);
  ch->in_room = NULL;
  ch->next_in_room = NULL;
  return;
}

void
check_stats (CHAR_DATA * ch)
{
  short i;
  for (i = 0; i < MAX_I - 1; i++)
  {
    if (ch->pcdata->perm_stat[i] > race_info[ch->pcdata->race].limits[i])
      ch->pcdata->perm_stat[i] = race_info[ch->pcdata->race].limits[i];
    else if (ch->pcdata->perm_stat[i] > MAX_ONESTAT)
      ch->pcdata->perm_stat[i] = MAX_ONESTAT;
    else if (ch->pcdata->perm_stat[i] < 1)
      ch->pcdata->perm_stat[i] = 1;
  }
  return;
}

#define INDOORS_MAP(ch) (((ch)->in_room->more && (ch)->in_room->more->linked_to) ? IS_SET((ch)->in_room->more->linked_to->room_flags,ROOM_INDOORS) :IS_SET((ch)->in_room->room_flags,ROOM_INDOORS) )
void
char_to_room (CHAR_DATA * ch, ROOM_DATA * oneroom)
{
  SINGLE_OBJECT *obj;
  int can_see_to_min_x;
  int can_see_to_min_y;
  int can_see_to_max_x;
  int can_see_to_max_y;
  CHAR_DATA *c;
  int x, y;
  DESCRIPTOR_DATA *d;
  bool neww;
  int iWear;
  neww = FALSE;
  check_room_more (oneroom);
  if (IS_SET (oneroom->room_flags, ROOM_INDOORS) && IS_AFFECTED (ch, AFF_FLYING))
    {
      send_to_char ("You cannot fly while indoors.\n\r", ch);
      do_land (ch, "");
    }
  if (ch->in_room)
    {
      char_from_room (ch);
    }
  if (oneroom && oneroom->area && oneroom->area->open != 0 && IS_PLAYER (ch) && ch->pcdata->level > 100 && ch->pcdata->level < 109)
    {
      send_to_char ("You have no reasons to be in an open area...\n\r", ch);
      send_to_char ("If you are a god, you may check out characters using the snoop\n\r", ch);
      send_to_char ("command. Please stay in your zone or another closed area.\n\r", ch);
      if (LEVEL (ch) < 100)
	oneroom = get_room_index (3);
      else
	oneroom = get_room_index (2);
      check_room_more (oneroom);
    }
  else if (oneroom->area->open == 0 && IS_PLAYER (ch) && ch->pcdata->level < 101)
    {
      send_to_char ("You may not go to unopened areas. Sorry!\n\r", ch);
      oneroom = get_room_index (3);
      check_room_more (oneroom);
    }

  if (IS_PLAYER (ch) && !IS_SET (ch->act, PLR_HOLYWALK))
    {
      if (oneroom->vnum == 100 || oneroom->vnum == 99)
	{
	  DESCRIPTION_DATA *ded;
	  char *cp;
	  char kw[500];
	  char argo[200];
	  ch->affected_by |= ch->pcdata->nat_abilities;
	  ch->affected_by |= race_info[ch->pcdata->race].affect_bits;
	  FIXIT (ch) natural_mana (ch);
	  upd_mana (ch);
	  for (ded = oneroom->more->extra_descr; ded != NULL; ded = ded->next)
	    {
	      if (!ded->keyword || ded->keyword[0] == '\0')
		continue;
	      strcpy (kw, ded->keyword);
	      cp = one_argy (kw, argo);
	      if (is_number (argo) && is_number (cp))
		{
		  int racenum;
		  int roomvn;
		  ROOM_DATA *nr;
		  racenum = atoi (argo);
		  roomvn = atoi (cp);
		  if (racenum >= 0 && racenum <= RACE_COUNT &&
		      racenum == ch->pcdata->race
		      && (nr = get_room_index (roomvn)) != NULL
		      && nr->vnum != 100 && nr->vnum != 99)
		    {
		      char_to_room (ch, nr);
		      return;
		    }
		}
	    }
	}
    }

  if (IS_PLAYER (ch) && ch->in_room && ch->in_room->more)
    {
      CHAR_DATA *cd;
      for (cd = ch->in_room->more->people; cd != NULL; cd = cd->next_in_room)
	{
	  if (IS_MOB (cd))
	    {
	      add_to_aggro_list (cd);
	    }
	  if (cd == ch)
	    {
	      ch->in_room = oneroom;
	      return;
	    }
	}
    }

  if (IS_PLAYER (ch) && oneroom && oneroom->more)
    {
      CHAR_DATA *cd;
      for (cd = oneroom->more->people; cd != NULL; cd = cd->next_in_room)
	{
	  if (IS_MOB (cd))
	    add_to_aggro_list (cd);
	  if (cd == ch)
	    {
	      fprintf (stderr, "UGH!\n");
	      ch->in_room = oneroom;
	      return;
	    }
	}
    }

  check_room_more (oneroom);

  if (oneroom == NULL)
    {
      bug ("Char_to_room: NULL.", 0);
      return;
    }

  ch->in_room = oneroom;

  if (IS_PLAYER (ch))
    {
      int kkk;
      if (!IS_JAVA (ch) && IS_SET (ch->pcdata->act3, ACT3_MAPPING) && IS_SET (ch->pcdata->act3, ACT3_DISABLED) && !INDOORS_MAP (ch))
	{
	  REMOVE_BIT (ch->pcdata->act3, ACT3_MAPPING);
	  REMOVE_BIT (ch->pcdata->act3, ACT3_DISABLED);
	  do_drawmort (ch, "");
	}
      else if (!IS_JAVA (ch) && IS_SET (ch->pcdata->act3, ACT3_MAPPING) && !IS_SET (ch->pcdata->act3, ACT3_DISABLED) 
                && INDOORS_MAP (ch) || IS_AFFECTED(ch, AFF_BLIND) || room_is_dark (ch->in_room) && !IS_AFFECTED (ch, AFF_INFRARED))
	{
	  SET_BIT (ch->pcdata->act3, ACT3_DISABLED);
	  do_clear (ch, "");
	  SET_BIT (ch->pcdata->act3, ACT3_MAPPING);
	}


      if (pet_flag)
	for (kkk = 0; kkk < 20; kkk++)
	  ch->pcdata->pet_temp[kkk] = 0;
      ch->in_room->area->nplayer++;
      if (ch->in_room->more->pcs != 31999)
	ch->in_room->more->pcs++;
    }
  if (IS_MOB (ch) && ch->in_room->more->pcs > 0)
    add_to_aggro_list (ch);
  if (oneroom->more->people != ch)
    ch->next_in_room = oneroom->more->people;
  oneroom->more->people = ch;
  if (oneroom->vnum >= BATTLEGROUND_START_VNUM
      && oneroom->vnum <= BATTLEGROUND_END_VNUM)
    {
      check_fgt (ch);
      ch->fgt->in_battle = TRUE;
    }
  else
    {
      if (ch->fgt)
	{
	  ch->fgt->in_battle = FALSE;
	  ch->fgt->wasroomtwo = oneroom->vnum;
	}
    }
#ifdef WALKTHRU_CREATION
  if (IS_PLAYER (ch))
    {
      if (ch->in_room->vnum > 410 && ch->in_room->vnum < 500)
	{
	  if (ch->in_room->vnum == 450)
	    {
	      ch->pcdata->nat_abilities = 0;
	      ch->affected_by = 0;
	      ch->pcdata->perm_stat[STR_I] = number_range (13, 15);
	      ch->pcdata->perm_stat[INT_I] = number_range (13, 14);
	      ch->pcdata->perm_stat[CON_I] = number_range (13, 15);
	      ch->pcdata->perm_stat[DEX_I] = number_range (12, 15);
	      ch->pcdata->perm_stat[WIS_I] = number_range (12, 14);
	      ch->pcdata->perm_stat[STR_I] +=
		race_info[ch->pcdata->race].bonus[0];
	      ch->pcdata->perm_stat[INT_I] +=
		race_info[ch->pcdata->race].bonus[1];
	      ch->pcdata->perm_stat[WIS_I] +=
		race_info[ch->pcdata->race].bonus[2];
	      ch->pcdata->perm_stat[DEX_I] +=
		race_info[ch->pcdata->race].bonus[3];
	      ch->pcdata->perm_stat[CON_I] +=
		race_info[ch->pcdata->race].bonus[4];

	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 452)
	    {
	      conv_race (ch);
	      ch->pcdata->perm_stat[STR_I] += 1;
	      ch->pcdata->perm_stat[DEX_I] += number_range (1, 3);
	      ch->pcdata->perm_stat[CON_I] += 1;
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 460)
	    {
	      conv_race (ch);
	      ch->pcdata->perm_stat[STR_I] += number_range (2, 4);
	      ch->pcdata->perm_stat[WIS_I] += 1;
	      ch->pcdata->perm_stat[DEX_I] += number_range (2, 4);
	      ch->pcdata->perm_stat[CON_I] += number_range (2, 4);
	      gain_exp (ch, 700);
	      ch->max_move += 1;
	      ch->move += 1;
	      ch->max_hit += 3;
	      ch->hit += 3;
	      ch->pcdata->learned[gsn_slashing] = 10;
	      ch->pcdata->learned[gsn_parry] = 7;
	      ch->pcdata->learned[gsn_pierce] = 6;
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 464)
	    {
	      ch->pcdata->perm_stat[STR_I] += 1;
	      ch->pcdata->perm_stat[DEX_I] += number_range (1, 2);
	      ch->max_move += 6;
	      ch->move += 6;
	      gain_exp (ch, 200);
	      ch->pcdata->perm_stat[CON_I] += number_range (1, 4);
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 465)
	    {
	      ch->pcdata->perm_stat[STR_I] += 2;
	      ch->pcdata->perm_stat[CON_I] += 2;
	      ch->max_hit += 2;
	      ch->hit += 2;
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 461)
	    {
	      ch->max_move += 4;
	      ch->move += 4;
	      gain_exp (ch, 600);
	      ch->pcdata->perm_stat[STR_I] += number_range (1, 2);
	      ch->pcdata->perm_stat[WIS_I] += 2;
	      ch->pcdata->perm_stat[INT_I] += 1;
	      ch->pcdata->perm_stat[DEX_I] += number_range (3, 4);
	      ch->pcdata->perm_stat[CON_I] += number_range (2, 4);
	      ch->pcdata->learned[gsn_sneak] = 8;
	      ch->pcdata->learned[gsn_pierce] = 9;
	      ch->pcdata->learned[gsn_slashing] = 6;
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 453)
	    {
	      conv_race (ch);
	      ch->pcdata->perm_stat[INT_I] += 1;
	      ch->pcdata->perm_stat[DEX_I] += number_range (1, 2);
	      ch->pcdata->perm_stat[WIS_I] += 2;
	      gain_exp (ch, 350);
	      ch->pcdata->learned[gsn_search] = 5;
	      ch->pcdata->learned[gsn_enhanced_damage] = 5;
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 458)
	    {
	      /*if (number_range(1,3)==2) ch->pcdata->nat_abilities|=AFF_INFRARED; */
	      gain_exp (ch, 300);
	      ch->pcdata->perm_stat[STR_I] += 2;
	      ch->pcdata->perm_stat[CON_I] += number_range (1, 3);
	      ch->pcdata->perm_stat[WIS_I] += 1;
	      ch->pcdata->perm_stat[INT_I] += 1;
	      ch->pcdata->perm_stat[DEX_I] += 1;
	      ch->max_hit += 2;
	      ch->hit += 2;
	      ch->max_move += 1;
	      ch->move += 1;
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 459)
	    {
	      gain_exp (ch, 450);
	      ch->pcdata->perm_stat[WIS_I] += 3;
	      ch->pcdata->perm_stat[INT_I] += number_range (1, 3);
	      ch->pcdata->perm_stat[STR_I] += 1;
	      if (number_range (1, 2) == 2)
		ch->pcdata->perm_stat[CON_I] += 1;
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 467)
	    {
	      gain_exp (ch, 700);
	      ch->pcdata->perm_stat[WIS_I] += number_range (2, 5);
	      ch->pcdata->perm_stat[INT_I] += number_range (1, 2);
	      ch->pcdata->perm_stat[STR_I] += number_range (1, 2);
	      ch->pcdata->perm_stat[DEX_I] += number_range (1, 2);
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 468)
	    {
	      gain_exp (ch, 500);
	      ch->pcdata->perm_stat[INT_I] += number_range (1, 5);
	      ch->pcdata->perm_stat[WIS_I] += number_range (1, 5);
	      ch->pcdata->perm_stat[STR_I] += number_range (1, 5);
	      ch->pcdata->perm_stat[CON_I] += number_range (1, 4);
	      ch->pcdata->perm_stat[DEX_I] -= 2;
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 497)
	    {
	      conv_race (ch);
	      ch->hit += 1;
	      ch->max_hit += 1;
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 498)
	    {
	      conv_race (ch);
	      ch->move += 1;
	      ch->max_move += 1;
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 469)
	    {
	      gain_exp (ch, 450);
	      ch->pcdata->perm_stat[STR_I] += number_range (2, 6);
	      ch->pcdata->perm_stat[CON_I] += number_range (2, 6);
	      ch->pcdata->perm_stat[DEX_I] += 2;
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 482)
	    {
	      ch->pcdata->perm_stat[STR_I] += number_range (1, 3);
	      ch->pcdata->perm_stat[CON_I] += number_range (1, 2);
	      ch->hit += 2;
	      ch->max_hit += 2;
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 481)
	    {
	      ch->pcdata->perm_stat[INT_I] += number_range (1, 3);
	      ch->pcdata->perm_stat[WIS_I] += number_range (1, 2);
	      ch->pcdata->perm_stat[DEX_I] += 1;
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 457)
	    {
	      ch->pcdata->perm_stat[STR_I] += number_range (1, 5);
	      ch->pcdata->perm_stat[CON_I] += number_range (1, 3);
	      ch->pcdata->perm_stat[DEX_I] += number_range (1, 3);
	      ch->max_move += 1;
	      gain_exp (ch, 250);
	      ch->move += 1;
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 483)
	    {
	      ch->pcdata->perm_stat[CON_I] += number_range (1, 4);
	      ch->pcdata->perm_stat[DEX_I] += number_range (1, 4);
	      ch->max_hit += 2;
	      ch->hit -= 2;
	      gain_exp (ch, 300);
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 484)
	    {
	      ch->pcdata->perm_stat[INT_I] += number_range (1, 2);
	      ch->pcdata->perm_stat[STR_I] += number_range (1, 2);
	      ch->pcdata->perm_stat[DEX_I] += number_range (1, 2);
	      ch->max_move += 1;
	      ch->move += 1;
	      gain_exp (ch, 450);
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 485)
	    {
	      ch->pcdata->perm_stat[WIS_I] += number_range (1, 3);
	      ch->pcdata->perm_stat[STR_I] += 1;
	      ch->pcdata->perm_stat[DEX_I] += 1;
	      ch->max_hit += 2;
	      ch->hit += 2;
	      conv_race (ch);
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 486)
	    {
	      ch->pcdata->perm_stat[DEX_I] += number_range (1, 3);
	      ch->pcdata->perm_stat[STR_I] += 1;
	      ch->pcdata->perm_stat[CON_I] += 1;
	      ch->max_move += 1;
	      ch->move += 1;
	      conv_race (ch);
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 488)
	    {
	      ch->pcdata->perm_stat[CON_I] += number_range (1, 2);
	      ch->max_hit += 2;
	      ch->hit += 2;
	      gain_exp (ch, 550);
	      ch->pcdata->perm_stat[DEX_I] += 1;
	      conv_race (ch);
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 487)
	    {
	      ch->pcdata->perm_stat[WIS_I] += number_range (1, 2);
	      ch->pcdata->perm_stat[INT_I] += number_range (1, 2);
	      ch->pcdata->perm_stat[DEX_I] += 1;
	      gain_exp (ch, 300);
	      conv_race (ch);
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 466)
	    {
	      gain_exp (ch, 100);
	      ch->pcdata->perm_stat[WIS_I] += number_range (1, 3);
	      ch->pcdata->perm_stat[INT_I] += number_range (2, 4);
	      ch->pcdata->perm_stat[DEX_I] += 1;
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 492)
	    {
	      ch->pcdata->perm_stat[CON_I] += 1;
	      ch->pcdata->perm_stat[WIS_I] += number_range (1, 3);
	      ch->pcdata->perm_stat[INT_I] += number_range (1, 2);
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 491)
	    {
	      ch->pcdata->perm_stat[STR_I] += number_range (1, 2);
	      if (number_range (1, 2) == 2)
		ch->pcdata->perm_stat[CON_I] += 1;
	      ch->pcdata->perm_stat[DEX_I] += 1;
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 494)
	    {
	      ch->pcdata->perm_stat[WIS_I] += number_range (1, 3);
	      ch->pcdata->perm_stat[INT_I] += number_range (1, 3);
	      conv_race (ch);
	      gain_exp (ch, 650);
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 493)
	    {
	      gain_exp (ch, 320);
	      ch->pcdata->perm_stat[STR_I] += number_range (1, 3);
	      ch->pcdata->perm_stat[CON_I] += number_range (1, 3);
	      ch->pcdata->perm_stat[DEX_I] += number_range (1, 2);
	      ch->pcdata->perm_stat[INT_I] -= 2;
	      ch->pcdata->learned[gsn_slashing] = 22;
	      conv_race (ch);
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 476)
	    {
	      gain_exp (ch, 400);
	      ch->pcdata->perm_stat[STR_I] += number_range (1, 3);
	      ch->pcdata->perm_stat[CON_I] += number_range (1, 2);
	      ch->pcdata->perm_stat[DEX_I] += 1;
	      ch->max_hit += 2;
	      ch->hit += 2;
	      /*if (number_range(1,2)==2) ch->pcdata->nat_abilities|=AFF_INFRARED; */
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 475)
	    {
	      gain_exp (ch, 200);
	      ch->pcdata->perm_stat[STR_I] += 2;
	      ch->pcdata->perm_stat[CON_I] += number_range (1, 2);
	      ch->pcdata->perm_stat[DEX_I] += 2;
	      ch->pcdata->perm_stat[INT_I] += 2;
	      ch->pcdata->perm_stat[WIS_I] += 1;
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 479)
	    {
	      ch->pcdata->perm_stat[STR_I] += number_range (1, 3);
	      ch->pcdata->perm_stat[CON_I] += number_range (2, 3);
	      ch->max_hit += 2;
	      ch->hit += 2;
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 480)
	    {
	      ch->pcdata->perm_stat[INT_I] += number_range (1, 2);
	      ch->pcdata->perm_stat[WIS_I] += number_range (2, 3);
	      ch->pcdata->perm_stat[DEX_I] += 1;
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 477)
	    {
	      conv_race (ch);
	      ch->pcdata->perm_stat[STR_I] += number_range (1, 3);
	      ch->pcdata->perm_stat[CON_I] += number_range (1, 3);
	      ch->pcdata->perm_stat[DEX_I] += number_range (1, 3);
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 478)
	    {
	      conv_race (ch);
	      ch->pcdata->perm_stat[INT_I] += 1;
	      ch->pcdata->perm_stat[WIS_I] += number_range (2, 4);
	      ch->pcdata->perm_stat[DEX_I] += 1;
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 496)
	    {
	      ch->pcdata->perm_stat[INT_I] += number_range (1, 2);
	      ch->pcdata->perm_stat[CON_I] += number_range (1, 2);
	      ch->max_hit += 1;
	      ch->hit += 1;
	      ch->max_move += 1;
	      ch->move += 1;
	      conv_race (ch);
	      check_stats (ch);
	    }
	  if (ch->in_room->vnum == 495)
	    {
	      ch->pcdata->perm_stat[STR_I] += number_range (1, 3);
	      ch->pcdata->perm_stat[CON_I] += number_range (1, 4);
	      gain_exp (ch, 410);
	      ch->max_hit += 2;
	      ch->hit += 2;
	      ch->pcdata->learned[gsn_slashing] = 18;
	      conv_race (ch);
	      check_stats (ch);
	    }
	}
    }
#endif


  for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
      if (obj->wear_loc != WEAR_NONE && obj->pIndexData->values_2[8] > 0 &&
	  IS_PLAYER (ch) && !FIGHTING (ch)
	  && ch->position == POSITION_STANDING
	  && number_range (1, 1000) < obj->pIndexData->values_2[8]
	  && ch->in_room && ch->in_room->more)
	{
	  CHAR_DATA *ir;
	  for (ir = ch->in_room->more->people; ir != NULL;
	       ir = ir->next_in_room)
	    {
	      if (IS_MOB (ir))
		{
		  send_to_char
		    ("You feel an evil energy surging through you, compelling you to go wild!\n\r",
		     ch);
		  do_kill (ch, ir->pIndexData->name);
		  goto yy;
		}
	    }
	yy:
	  if (ch)
	    {
	    };
	}

      if (obj->wear_loc != WEAR_NONE
	  && obj->pIndexData->item_type == ITEM_LIGHT && IS_LIT (obj))
	{
	  ++ch->in_room->light;
	}
    }
  check_room_more (ch->in_room);
  if (ch->in_room->more->pcs > 0)
    {
      CHAR_DATA *t;
      for (t = ch->in_room->more->people; t; t = t->next_in_room)
	{
	  if (t == ch)
	    continue;
	  if (IS_JAVA (t))
	    {
	      java_add_mob_window (t, ch);
	    }
	}
    }
#ifdef MAP_MOBS
  if (1)
    {
#else
  if (IS_PLAYER (ch))
    {
#endif
      char bb[500];
      if (!(ch->in_room && ch->in_room->x == 0 && ch->in_room->y == 0 && ch->in_room->z == 0 && ch->in_room->vnum != 1508))
	{
	  if (IS_SET (ch->pcdata->act3, ACT3_MAPPING) && !IS_SET (ch->pcdata->act3, ACT3_DISABLED))
	    {
//#ifdef NEW_WORLD
// Reverend 02-18-04 Overhead Map
	      if (ch->pcdata->resist_summon == -10
		  || ch->pcdata->resist_summon == -15)
		{
		  if (IS_JAVA (ch))
		    {
		      if (ch->in_room->more->linked_to)
			display_room_mark (ch, ch->in_room->more->linked_to,
					   ch->pcdata->transport_quest % 80,
					   ch->pcdata->transport_quest / 80,
					   -1, FALSE, FALSE);
		      else
			display_room_mark (ch, ch->in_room,
					   ch->pcdata->transport_quest % 80,
					   ch->pcdata->transport_quest / 80,
					   -1, FALSE, FALSE);
		    }
		  else
		    {
		      if (ch->in_room->more->linked_to)
			display_room_mark (ch, ch->in_room->more->linked_to,
					   ch->pcdata->transport_quest % 80,
					   ch->pcdata->transport_quest / 80,
					   -1, FALSE, FALSE);
		      else
			display_room_mark (ch, ch->in_room,
					   ch->pcdata->transport_quest % 80,
					   ch->pcdata->transport_quest / 80,
					   -1, FALSE, FALSE);
		    }
		  noibm = FALSE;
		  kk = FALSE;
		  mb = FALSE;
		  dbs = TRUE;
		  if (!IS_JAVA (ch))
		    {
		      gotoxy (ch, 1, ch->pcdata->pagelen);
		      send_to_char ("\x1B[37;0m", ch);
		    }
		}
//#endif  Reverend 02-18-04

	    }
	  for (d = descriptor_list; d != NULL; d = d->next)
	    {
	      x = 0;
	      y = 0;
	      if ((c = d->character) == NULL)
		continue;
#ifdef MAP_MOBS
	      if (IS_MOB (ch) && !VIEW_MOBS (c))
		continue;
#endif
	      if (!c->in_room)
		continue;
	      if (!can_see (c, ch) || not_is_same_align (c, ch))
		continue;
	      if (!IS_SET (c->pcdata->act3, ACT3_MAPPING) || IS_SET (c->pcdata->act3, ACT3_DISABLED))
		continue;
	      if (IS_JAVA (ch))
		{
		  can_see_to_min_x = (XPOS (c) - 4) / 2;
		  can_see_to_max_x = (24 - XPOS (c)) / 2;
		  can_see_to_min_y = (YPOS (c) - 1);
		  can_see_to_max_y = (8 - YPOS (c));
		}
	      else
		{
		  can_see_to_min_x = (XPOS (c) - 4) / 2;
		  can_see_to_max_x = (20 - XPOS (c)) / 2;
		  can_see_to_min_y = (YPOS (c) - 1);
		  can_see_to_max_y = (7 - YPOS (c));
		}
	      if (c->in_room->x - can_see_to_min_x > ch->in_room->x)
		continue;
	      if (c->in_room->x + can_see_to_max_x < ch->in_room->x)
		continue;
	      if (c->in_room->y + can_see_to_min_y < ch->in_room->y)
		continue;
	      if (c->in_room->y - can_see_to_max_y > ch->in_room->y)
		continue;
	      if (c->in_room->z != ch->in_room->z ||
		  c->in_room == ch->in_room)
		continue;
	      x = (ch->in_room->x - c->in_room->x);
	      y = (c->in_room->y - ch->in_room->y);
	      x *= 2;
	      x += XPOS (c);
	      y += YPOS (c);
	      if (IS_JAVA (c))
		{
		  display_room_mark (c, ch->in_room, x, y, 1, FALSE,
				     IS_MOB (ch));
		}
	      else
		{
		  sprintf (bb, "%c%c%s%s%c%c", '\x1B', '7',
			   display_room_mark_direct (c, ch->in_room, x, y, 1,
						     FALSE, IS_MOB (ch)),
			   "\x1B[37;0m", '\x1B', '8');

		  write_to_descriptor (d->descriptor, bb, 0);
		}
	      c->timer = -5;
	    }
	}
    }
  return;
}

void
u_char_to_room (CHAR_DATA * ch, ROOM_DATA * oneroom)
{
  SINGLE_OBJECT *obj;
  int iWear;
  check_room_more (oneroom);
  if (oneroom == NULL)
    {
      bug ("Char_to_room: NULL.", 0);
      return;
    }
  ch->in_room = oneroom;
  if (oneroom->more->people != ch)
    ch->next_in_room = oneroom->more->people;
  oneroom->more->people = ch;
  if (oneroom->vnum >= BATTLEGROUND_START_VNUM
      && oneroom->vnum <= BATTLEGROUND_END_VNUM)
    {
      check_fgt (ch);
      ch->fgt->in_battle = TRUE;
    }
  else
    {
      if (ch->fgt)
	{
	  ch->fgt->in_battle = FALSE;
	  ch->fgt->wasroomtwo = oneroom->vnum;
	}
    }
  if (IS_PLAYER (ch))
    {
      ch->in_room->area->nplayer++;
      if (ch->in_room->more->pcs != 31999)
	ch->in_room->more->pcs++;
    }
  for (iWear = WEAR_NONE + 1; iWear < MAX_WEAR; iWear++)
    {
      if ((obj = get_eq_char (ch, iWear)) != NULL
	  && obj->pIndexData->item_type == ITEM_LIGHT && IS_LIT (obj))
	++ch->in_room->light;
    }
  return;
}

int
apply_ac (SINGLE_OBJECT * obj, int iWear)
{
  int damaged;
  I_ARMOR *r;
  return 0;
  if (obj->pIndexData->item_type != ITEM_ARMOR)
    return 0;
  r = (I_ARMOR *) obj->more;
  if (r->max_condition > 0)
    {
      damaged =
	((((r->condition_now * 100) / (r->max_condition)) * r->slash_rating) /
	 100);
    }
  else
    damaged = r->slash_rating;
  switch (iWear)
    {
    case WEAR_SHOULDER_R:
      return damaged;
    case WEAR_SHOULDER_L:
      return damaged;
    case WEAR_BODY:
      return 3 * damaged;
    case WEAR_HEAD:
      return 2 * damaged;
    case WEAR_LEGS:
      return 2 * damaged;
    case WEAR_FEET:
      return damaged;
    case WEAR_HANDS:
      return damaged;
    case WEAR_ARMS:
      return damaged;
    case WEAR_SHIELD:
      return damaged;
    case WEAR_FINGER_L:
      return damaged;
    case WEAR_FINGER_R:
      return damaged;
    case WEAR_NECK_1:
      return damaged;
    case WEAR_NECK_2:
      return damaged;
    case WEAR_ABOUT:
      return 2 * damaged;
    case WEAR_BACK:
      return 3 * damaged;
    case WEAR_WAIST:
      return damaged;
    case WEAR_WRIST_L:
      return damaged;
    case WEAR_WRIST_R:
      return damaged;
    case WEAR_HOLD_1:
    case WEAR_HOLD_2:
      return 0;
    case WEAR_FACE:
      return damaged;
    case WEAR_FLOAT_1:
      return damaged;   
    case WEAR_FLOAT_2:
      return damaged;
    case WEAR_EAR_1:
      return damaged;
    case WEAR_EAR_2:
      return damaged;
    case WEAR_ANKLE_1:
      return damaged;
    case WEAR_ANKLE_2:
      return damaged;
    case WEAR_KNEE_1:
       return damaged;
    case WEAR_KNEE_2:
      return damaged;
    case WEAR_ELBOW_1:
       return damaged;
    case WEAR_ELBOW_2:
       return damaged;
    case WEAR_TATTOO:
       return damaged;
    case WEAR_EYEPATCH:
       return damaged;
    }
  return 0;
}

SINGLE_OBJECT *
get_item_held (CHAR_DATA * ch, int itype)
{
  SINGLE_OBJECT *obj;
  if ((obj = get_eq_char (ch, WEAR_HOLD_1)) != NULL
      && obj->pIndexData->item_type == itype)
    return obj;
  if ((obj = get_eq_char (ch, WEAR_HOLD_2)) != NULL
      && obj->pIndexData->item_type == itype)
    return obj;
  return NULL;
}

SINGLE_OBJECT *
get_eq_char (CHAR_DATA * ch, int iWear)
{
  SINGLE_OBJECT *obj;
  for (obj = ch->carrying; obj != NULL;
       obj = ((!obj->next_content) ? NULL : obj->next_content))
    {
      if (obj->wear_loc == iWear)
	return obj;
    }
  return NULL;
}

/*
   * Equip a char with an obj.
 */
void
equip_char (CHAR_DATA * ch, SINGLE_OBJECT * obj, int iWear)
{
  AFFECT_DATA *paf;
  int bitty;
  if (get_eq_char (ch, iWear) != NULL)
    {
      fprintf (stderr, "Error, check char %s... already equipped.\n",
	       NAME (ch));
      bug ("Equip_char: already equipped (%d).", iWear);
      return;
    }
  if ((IS_OBJ_STAT (obj, ITEM_ANTI_EVIL) && IS_EVIL (ch))
      || (IS_OBJ_STAT (obj, ITEM_ANTI_GOOD) && IS_GOOD (ch))
      || (IS_OBJ_STAT (obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL (ch)))
    {
      if (!IS_IMMORTAL (ch))
	{
	  act ("You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR);
	  act ("$n is zapped by $p and drops it.", ch, obj, NULL, TO_ROOM);
	  obj_from (obj);
	  obj_to (obj, ch->in_room);
	  return;
	}
    }
  if (obj->pIndexData->item_type == ITEM_ARMOR)
    {
      if (obj->size < 1)
	SET_OBJ_SIZE (ch, obj);
      if (!OBJ_FITS (obj, ch))
	{
	  char tmpb[1000];
	  bool plural;
	  plural =
	    (obj->pIndexData->
	     short_descr[strlen (obj->pIndexData->short_descr) - 1] == 's');
	  if (OBJ_TOO_BIG (obj, ch))
	    {
	      if (IS_JAVA (ch))
		{
		  char buf[500];
		  sprintf (buf, "%c%c", (char) 23, (char) 67);
		  write_to_buffer (ch->desc, buf, 0);
		}
	      else
		{
		  sprintf (tmpb,
			   "%s\x1B[37;0m %s too big and just fall%s off!\n\r",
			   capitalize (obj->pIndexData->short_descr),
			   (plural ? "are" : "is"), (plural ? "" : "s"));
		  send_to_char (tmpb, ch);
		}
	    }
	  else
	    {
	      if (IS_JAVA (ch))
		{
		  char buf[500];
		  sprintf (buf, "%c%c", (char) 23, (char) 68);
		  write_to_buffer (ch->desc, buf, 0);
		}
	      else
		{

		  sprintf (tmpb,
			   "%s\x1B[37;0m %s too small for you to wear!\n\r",
			   capitalize (obj->pIndexData->short_descr),
			   (plural ? "are" : "is"));
		  send_to_char (tmpb, ch);
		}
	    }
	  return;
	}
    }
  ch->armor -= apply_ac (obj, iWear);
  obj->wear_loc = iWear;
  if (obj->pIndexData->item_type == ITEM_GEM)
    {
      act ("As you hold $p in your hand, its energy drains.", ch, obj, NULL,
	   TO_CHAR);
      act
	("...You slowly start to feel the gem get used to your hand and it begins",
	 ch, obj, NULL, TO_CHAR);
      act
	("to power up for use. Type MANA to see what type of mana the gem is",
	 ch, obj, NULL, TO_CHAR);
/*      if (ch->pcdata->n_mana<0) ch->pcdata->n_mana += ((I_GEM *) obj->more)->mana_now;
      if (((I_GEM *) obj->more)->mana_now<0) {
                ((I_GEM *) obj->more)->mana_now+=ch->pcdata->n_mana;
                }*/
      ((I_GEM *) obj->more)->mana_now = 0;
      ch->pcdata->n_mana = 0;
      upd_mana (ch);
    }
  if (IS_JAVA (ch))
    {
      java_remove_obj_inv (ch, obj);
    }
  bitty = 0;
  if (!IS_AFFECTED (ch, AFF_FLYING))
    bitty = 1;
  for (paf = obj->pIndexData->affected; paf != NULL;
       paf = ((!paf->next) ? NULL : paf->next))
    affect_modify (ch, paf, TRUE, 0);
/*  for (paf = obj->pIndexData->effects; paf != NULL; paf = ((!paf->next) ? NULL :
paf->next))
    affect_modify (ch, paf, TRUE, 0); */
  if (IS_AFFECTED (ch, AFF_FLYING) && bitty == 1)
    REMOVE_BIT (ch->affected_by, AFF_FLYING);
  if (obj->pIndexData->item_type == ITEM_LIGHT
      && IS_LIT (obj) && ch->in_room != NULL)
    ++ch->in_room->light;

  if (ch->move > ch->max_move)
    ch->move = ch->max_move;
  return;
}


int
hand_empty (CHAR_DATA * ch)
{
  if (get_eq_char (ch, WEAR_HOLD_1) == NULL)
    return WEAR_HOLD_1;
  if (get_eq_char (ch, WEAR_HOLD_2) == NULL)
    return WEAR_HOLD_2;
  return WEAR_NONE;
}

int
belt_empty (CHAR_DATA * ch)
{
  if (get_eq_char (ch, WEAR_WAIST) == NULL)
    return WEAR_NONE;
  if (get_eq_char (ch, WEAR_BELT_1) == NULL)
    return WEAR_BELT_1;
  if (get_eq_char (ch, WEAR_BELT_2) == NULL)
    return WEAR_BELT_2;
  return WEAR_NONE;
}

bool
unequip_char (CHAR_DATA * ch, SINGLE_OBJECT * obj)
{
  AFFECT_DATA *paf;
  if (obj->wear_loc != WEAR_BELT_1 && obj->wear_loc != WEAR_BELT_2 &&
      obj->wear_loc != WEAR_BELT_3)
    {
      for (paf = obj->pIndexData->affected; paf != NULL;
	   paf = ((!paf->next) ? NULL : paf->next))
	affect_modify (ch, paf, FALSE, 0);
/*      for (paf = obj->pIndexData->effects; paf != NULL; paf = ((!paf->next) ? NULL :
paf->next))
        affect_modify (ch, paf, FALSE, 0);*/
    }
  ch->armor += apply_ac (obj, obj->wear_loc);
  obj->wear_loc = -1;

  if (IS_PLAYER (ch))
    {
      int amt;
      if (obj->pIndexData->item_type == ITEM_GEM)
	{
	  if (((I_GEM *) obj->more)->mana_now < 0)
	    {
	      amt = UMIN ((0 - ((I_GEM *) obj->more)->mana_now), ch->pcdata->n_mana);
	      ((I_GEM *) obj->more)->mana_now += amt;
	      ch->pcdata->n_mana -= amt;
	    }
	}
    }

  if (IS_JAVA (ch))
    {
      java_add_obj_inv_window (ch, obj);
    }

  if (obj->pIndexData->item_type == ITEM_LIGHT && IS_LIT (obj)
      && ch->in_room != NULL && ch->in_room->light > 0)
    --ch->in_room->light;

  return TRUE;
}


int
count_obj_list (OBJ_PROTOTYPE * pObjIndex, SINGLE_OBJECT * list)
{
  SINGLE_OBJECT *obj;
  int nMatch;
  nMatch = 0;
  for (obj = list; obj != NULL;
       obj = ((!obj->next_content) ? NULL : obj->next_content))
    {
      if (obj->next_content && obj == obj->next_content)
	{
	  fprintf (stderr,
		   "Endless object pointers called in count_obj_list\n");
	  exit (15);
	}
      if (obj->pIndexData == pObjIndex)
	nMatch++;
    }
  return nMatch;
}

void
extract_char (CHAR_DATA * ch, bool fPull)
{
  CHAR_DATA *wch;
  SINGLE_OBJECT *obj;
  SINGLE_OBJECT *obj_next;
  ROOM_DATA *ir;
  DESCRIPTOR_DATA *d;
  MARK_CHAR_DEBUG ("Extract_char", ch) remove_from_combat_field_yes (ch);
  MARK_CHAR_DEBUG ("E_char_combat_field", ch)
  //if (TRUE /*IS_MOB(ch) */)
  //  clear_tracks (ch);
  if (IS_MOB(ch))
    clear_tracks(ch);
#ifndef NEW_WORLD
  if (ch->ced && ch->ced->in_boat != NULL)
  {
    char_from_boat (ch, ch->ced->in_boat);
  }
#endif
  if (fPull)
    die_follower (ch);
  ir = ch->in_room;
  if (ch->in_room)
    stop_fighting (ch, TRUE);
  if (ch->in_room)
    char_from_room (ch);
  if (RIDING (ch) != NULL)
  {
    check_fgt (ch->fgt->riding);
    ch->fgt->riding->fgt->mounted_by = NULL;
    ch->fgt->riding = NULL;
  }
  if (MOUNTED_BY (ch) != NULL)
  {
    check_fgt (ch->fgt->mounted_by);
    ch->fgt->mounted_by->fgt->riding = NULL;
    ch->fgt->mounted_by = NULL;
  }
  if (IS_PLAYER (ch) && ch->desc != NULL && ch->desc->original != NULL)
    do_return (ch, "");
  if (fPull)
  {
    SCRIPT_INFO *si;
    SCRIPT_INFO *si_next;
    for (si = info_list; si != NULL; si = si_next)
    {
      si_next = si->next;
      if (si->current == ch || si->mob == ch)
      {
        end_script (si);
      }
    }
    for (wch = char_list; wch != NULL; wch = wch->next)
    {
      if (MASTER (wch) == ch)
        wch->fgt->master = NULL;
      if (LEADER (wch) == ch)
        wch->fgt->leader = NULL;
      if (RIDING (wch) == ch)
        wch->fgt->riding = NULL;
      if (MOUNTED_BY (wch) == ch)
        wch->fgt->mounted_by = NULL;
      if (wch->ced && wch->ced->reply == ch)
        wch->ced->reply = NULL;
    }

    if (ch == char_list)
    {
      char_list = ch->next;
      ch->next = NULL;
    }
    else
    {
      CHAR_DATA *prev;
      for (prev = char_list; prev != NULL; prev = prev->next)
      {
        if (prev->next == ch)
        {
          prev->next = ch->next;
          break;
        }
      }
      if (prev == NULL)
      {
        fprintf (stderr,
            "Character placement temp error #1: Char not found in list: %s\n",
            NAME (ch));
        /*            ch->next = NULL;*/
        /*            free_it (ch);*/
        return;
      }
    }
  }

  if (fPull && IS_PLAYER (ch))
  {
    for (d = descriptor_list; d != NULL; d = d->next)
    {
      if (d->character && d->connected == CON_PEDITOR &&
          ch == (CHAR_DATA *) d->pEdit)
      {
        ch->in_room = ir;
        return;
      }
    }
  }

  if (fPull)
  {
    if (ch == debugger)
      debugger = NULL;
    for (obj = ch->carrying; obj != NULL; obj = obj_next)
    {
      obj_next = obj->next_content;
      free_it (obj);
    }
    free_read_notes (ch);
    //free_read_polls (ch);
  }

  if (IS_PLAYER (ch) && !fPull)
  {
    OBJ_PROTOTYPE *oid;
    SINGLE_OBJECT *o;
#ifdef NEW_WORLD

    if ((oid = get_obj_index (300)) != NULL)
    {
      o = create_object (oid, 1);
      obj_to (o, ch);
    }

    if ((oid = get_obj_index (301)) != NULL)
    {
      o = create_object (oid, 1);
      obj_to (o, ch);
    }

    if ((oid = get_obj_index (302)) != NULL)
    {
      o = create_object (oid, 1);
      obj_to (o, ch);
    }

    if ((oid = get_obj_index (200)) != NULL)
    {
      o = create_object (oid, 1);
      obj_to (o, ch);
    }
#endif

    ch->pcdata->condition[COND_FULL] = 55;
    ch->pcdata->condition[COND_THIRST] = 55;
    ch->pcdata->condition[COND_DRUNK] = 0;
  }
  if (!fPull)
  {
    strip_all_affects (ch);
    if (IS_EVIL (ch))
      char_to_room (ch, get_room_index (99));
    if (!IS_EVIL (ch))
      char_to_room (ch, get_room_index (100));
    return;
  }
  if (IS_MOB (ch))
  {
    --ch->pIndexData->count;
  }
  if (IS_PLAYER (ch) && ch->desc)
    ch->desc->character = NULL;
  free_it (ch);
  return;
}

CHAR_DATA *
get_char_room (CHAR_DATA * ch, char *argy)
{
  static char arg[500];
  CHAR_DATA *rch;
  int RCLOOK;
  int number;
/*char buf[50]; */
  int count;
  RCLOOK = 0;
  arg[0] = '\0';
  number = number_argy (argy, arg);
  count = 0;
  if (!ch->in_room)
    return NULL;
  if (!str_cmp (arg, "self"))
    return ch;
  if (!str_cmp (arg, "me"))
    return ch;
  RCLOOK = get_race_look (arg);
/*Mobs FIRST */
  for (rch = ch->in_room->more->people; rch != NULL; rch = rch->next_in_room)
    {
      if (!rch->in_room)
	continue;
      if (IS_PLAYER (rch))
	continue;
      if (!can_see (ch, rch) || (!is_approx_name (arg, RNAME (rch)) &&
				 str_prefix (arg, RNAME (rch))))
	continue;
      if (++count == number)
	return rch;
    }
/*{
   char buf[500];
   sprintf(buf,"TEST: Found RACE flag %d in name.\n\r",RCLOOK);
   send_to_char(buf,ch);
   } */
  if (RCLOOK != 0)
    {
      dummycall ();
      for (rch = ch->in_room->more->people; rch != NULL;
	   rch = rch->next_in_room)
	{
	  if (!rch->in_room)
	    continue;
	  if (!can_see (ch, rch) || IS_MOB (rch) || IS_MOB (ch))
	    continue;
	  if (rch == ch)
	    continue;
	  if (!IS_SET (rch->pcdata->raceflag, RCLOOK))
	    continue;
	  if ((!IN_BATTLE (ch) && !IN_BATTLE (rch)) &&
	      ((!IS_SET (ch->pcdata->raceflag, RC_EVILRACE)
		&& !IS_SET (rch->pcdata->raceflag, RC_EVILRACE))
	       || (IS_SET (ch->pcdata->raceflag, RC_EVILRACE)
		   && IS_SET (rch->pcdata->raceflag, RC_EVILRACE))))
	    continue;
	  if (++count == number)
	    return rch;
	}
    }
  {
    for (rch = ch->in_room->more->people; rch != NULL;
	 rch = rch->next_in_room)
      {
	if (!rch->in_room)
	  continue;
	if (!can_see (ch, rch) || (!is_name (arg, RNAME (rch)) &&
				   str_prefix (arg, RNAME (rch))))
	  continue;
	if (++count == number)
	  return rch;
      }
  }
  return NULL;
}

/* Find a player in the world */
CHAR_DATA *
get_player_world (CHAR_DATA * ch, char *argy, bool need_see)
{
  char arg[SML_LENGTH];
  CHAR_DATA *wch;
  DESCRIPTOR_DATA *dd;
  int number;
  int count;
  if (!ch->in_room)
    return NULL;
  if ((wch = get_char_room (ch, argy)) != NULL)
    return wch;
  number = number_argy (argy, arg);
  count = 0;
  for (dd = descriptor_list; dd != NULL; dd = dd->next)
    {
      if (dd->character == NULL)
	continue;
      if (dd->connected > 0)
	continue;
      wch = dd->character;
      if (!wch->in_room)
	continue;
      if ((need_see && !can_see (ch, wch))
	  || !is_approx_name (arg, RNAME (wch)) || (ch->in_room
						    && wch->in_room
						    && wch->in_room->area !=
						    ch->in_room->area))
	continue;
      if (++count == number)
	return wch;
    }
  number = number_argy (argy, arg);
  count = 0;
  for (dd = descriptor_list; dd != NULL; dd = dd->next)
    {
      if (dd->character == NULL)
	continue;
      if (dd->connected > 0)
	continue;
      wch = dd->character;
      if (!wch->in_room)
	continue;
      if ((need_see && !can_see (ch, wch)) || !is_name (arg, RNAME (wch)))
	continue;
      if (++count == number)
	return wch;
    }
  return NULL;
}

CHAR_DATA *
get_char_world (CHAR_DATA * ch, char *argy)
{
  char arg[SML_LENGTH];
  CHAR_DATA *wch;
  int number;
  int count;
  if (!ch->in_room)
    return NULL;
  //if ((wch = get_char_room (ch, argy)) != NULL)
  //  return wch;
  if ((wch = get_player_world (ch, argy, TRUE)) != NULL)
    return wch;
  number = number_argy (argy, arg);
  count = 0;

  number = number_argy (argy, arg);
  count = 0;
  for (wch = char_list; wch != NULL; wch = wch->next)
    {
      if (!wch->in_room)
	continue;
      if (wch->desc)
	continue;
      if (!can_see (ch, wch) || !is_name (arg, RNAME (wch)))
	continue;
      if (++count == number)
	return wch;
    }
  return NULL;
}

CHAR_DATA *
get_char_world_2 (CHAR_DATA * ch, char *argy)
{
  char arg[SML_LENGTH];
  CHAR_DATA *wch;
  int number;
  int count;
  if (!ch->in_room)
    return NULL;
  if ((wch = get_char_room (ch, argy)) != NULL)
    return wch;
  if ((wch = get_player_world (ch, argy, FALSE)) != NULL)
    return wch;
  number = number_argy (argy, arg);
  count = 0;
  for (wch = char_list; wch != NULL; wch = wch->next)
    {
      if (!wch->in_room)
	continue;
      if (!is_name (arg, RNAME (wch)) && str_cmp (arg, NAME (wch)))
	continue;
      if (++count == number)
	return wch;
    }
  return NULL;
}

SINGLE_OBJECT *
get_obj_type (OBJ_PROTOTYPE * pObjIndex)
{
  SINGLE_OBJECT *obj;
  for (obj = object_list; obj != NULL; obj = obj->next)
    {
      if (obj->pIndexData == pObjIndex)
	return obj;
    }
  return NULL;
}


SINGLE_OBJECT *
get_obj_list (CHAR_DATA * ch, char *argy, SINGLE_OBJECT * list)
{
  char arg[SML_LENGTH];
  SINGLE_OBJECT *obj;
  int number;
  int count;
  number = number_argy (argy, arg);
  count = 0;
  for (obj = list; obj != NULL; obj = obj->next_content)
    {
      if (can_see_obj (ch, obj) && is_name (arg, obj->pIndexData->name))
	{
	  if (++count == number)
	    return obj;
	}
    }
  return NULL;
}


SINGLE_OBJECT *
get_obj_carry (CHAR_DATA * ch, char *argy)
{
  char arg[SML_LENGTH];
  SINGLE_OBJECT *obj;
  int number;
  int count;
  number = number_argy (argy, arg);
  count = 0;
  for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
      if ((obj->wear_loc == WEAR_NONE)
	  && (obj->pIndexData->item_type == ITEM_FOOD
	      || obj->pIndexData->item_type == ITEM_PILL
	      || can_see_obj (ch, obj))
	  && is_name (arg, obj->pIndexData->name))
	{
	  if (++count == number)
	    return obj;
	}
    }
  for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
      if ((obj->wear_loc == WEAR_NONE)
	  && (obj->pIndexData->item_type == ITEM_FOOD
	      || obj->pIndexData->item_type == ITEM_PILL
	      || can_see_obj (ch, obj))
	  && is_approx_name (arg, obj->pIndexData->name))
	{
	  if (++count == number)
	    return obj;
	}
    }
  return NULL;
}


SINGLE_OBJECT *
get_obj_held (CHAR_DATA * ch, char *argy)
{
  char arg[SML_LENGTH];
  SINGLE_OBJECT *obj;
  int number;
  int count;
  number = number_argy (argy, arg);
  count = 0;
  for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
      if ((obj->wear_loc == WEAR_HOLD_1 || obj->wear_loc == WEAR_HOLD_2)
	  && can_see_obj (ch, obj) && is_name (arg, obj->pIndexData->name))
	{
	  if (++count == number)
	    return obj;
	}
    }
  for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
      if ((obj->wear_loc == WEAR_HOLD_1 || obj->wear_loc == WEAR_HOLD_2)
	  && can_see_obj (ch, obj)
	  && is_approx_name (arg, obj->pIndexData->name))
	{
	  if (++count == number)
	    return obj;
	}
    }
  return NULL;
}


SINGLE_OBJECT *
get_obj_wear (CHAR_DATA * ch, char *argy)
{
  char arg[SML_LENGTH];
  SINGLE_OBJECT *obj;
  int number;
  int count;
  number = number_argy (argy, arg);
  count = 0;
  for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
      if (obj->wear_loc != WEAR_NONE
	  && can_see_obj (ch, obj) && is_name (arg, obj->pIndexData->name))
	{
	  if (++count == number)
	    return obj;
	}
    }
  for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
      if (obj->wear_loc != WEAR_NONE
	  && can_see_obj (ch, obj)
	  && is_approx_name (arg, obj->pIndexData->name))
	{
	  if (++count == number)
	    return obj;
	}
    }
  return NULL;
}


SINGLE_OBJECT *
get_obj_inv (CHAR_DATA * ch, char *argy)
{
  SINGLE_OBJECT *obj;
  if ((obj = get_obj_carry (ch, argy)) != NULL)
    return obj;
  if ((obj = get_obj_wear (ch, argy)) != NULL)
    return obj;
  return NULL;
}


SINGLE_OBJECT *
get_obj_here (CHAR_DATA * ch, char *argy, int lookfirst)
{
  SINGLE_OBJECT *obj;
  check_room_more (ch->in_room);
  if (lookfirst == SEARCH_ROOM_FIRST)
    {
      obj = get_obj_list (ch, argy, ch->in_room->more->contents);
      if (obj != NULL)
	return obj;
      if ((obj = get_obj_inv (ch, argy)) != NULL)
	return obj;
      return NULL;
    }
  else if (lookfirst == SEARCH_INV_FIRST)
    {
      if ((obj = get_obj_inv (ch, argy)) != NULL)
	return obj;
      obj = get_obj_list (ch, argy, ch->in_room->more->contents);
      if (obj != NULL)
	return obj;
      return NULL;
    }
  else
    return NULL;
}


SINGLE_OBJECT *
get_obj_world (CHAR_DATA * ch, char *argy)
{
  char arg[SML_LENGTH];
  SINGLE_OBJECT *obj;
  int number;
  int count;
  if ((obj = get_obj_here (ch, argy, SEARCH_ROOM_FIRST)) != NULL)
    return obj;
  number = number_argy (argy, arg);
  count = 0;
  for (obj = object_list; obj != NULL; obj = obj->next)
    {
      if (can_see_obj (ch, obj) && is_name (arg, obj->pIndexData->name))
	{
	  if (++count == number)
	    return obj;
	}
    }
  return NULL;
}


void
create_amount (int amount, CHAR_DATA * pMob, ROOM_DATA * pRoom,
	       SINGLE_OBJECT * pObj)
{
  I_CONTAINER *c = NULL;
  if (pObj && pObj->pIndexData->item_type != ITEM_CONTAINER &&
      pObj->pIndexData->item_type != ITEM_CORPSE_NPC
      && pObj->pIndexData->item_type != ITEM_CORPSE_PC)
    return;
  if (pObj)
    c = (I_CONTAINER *) pObj->more;
  if (amount <= 0)
    {
      bug ("Create_amount: zero or negative money %d.", amount);
      return;
    }
  if (amount == 1)
    {
      if (pMob)
	pMob->copper++;
      if (pObj)
	c->money++;
      if (pRoom)
	{
	  check_room_more (pRoom);
	  pRoom->more->copper++;
	}
    }
  else
    {
      if (pMob)
	{
	  if (amount >= 100)
	    {
	      pMob->gold += (amount / 100);
	      amount %= 100;
	    }
	  if (amount >= 1)
	    {
	      pMob->copper += (amount);
	    }
	}
      else
	{
	  if (pRoom)
	    {
	      check_room_more (pRoom);
	      if (amount >= 100)
		{
		  pRoom->more->gold += (amount / 100);
		  amount %= 100;
		}
	      if (amount >= 1)
		{
		  pRoom->more->copper += (amount);
		}
	    }
	  if (pObj)
	    c->money += amount;
	}
    }
  return;
}


void
fix_amount (CHAR_DATA * ch)
{
  int ii;
  ii = (ch->gold * 100) + ch->copper;
  ch->gold = 0;
  ch->copper = 0;
  if (ii > 0)
    create_amount (ii, ch, NULL, NULL);
  return;
}

int
tally_coins (CHAR_DATA * mob)
{
  if (mob->gold < 0)
    {
      fix_amount (mob);
    }
  return ((mob->gold * 100) + (mob->copper));
}


char *
name_amount (int amount)
{
  static char buf[STD_LENGTH];
  char buf2[STD_LENGTH];
  buf[0] = '\0';
  buf2[0] = '\0';
  if (amount >= 100)
    {
      sprintf (buf, "%d %s", amount / 100, "\x1B[33;1mgold\x1B[37;0m");
    }
  if (amount % 100 > 0)
    {
      sprintf (buf + strlen (buf), "%s%d %s", buf[0] != '\0' ? ", " : "",
	       amount % 100, "\x1B[33;0mcopper\x1B[37m");
    }

  if (buf[0] == '\0')
    sprintf (buf, "nothing");
  return buf;
}


char *
sub_coins (int amount, CHAR_DATA * ch)
{
  static char ret_val[500];
  int money = 0;
  if (tally_coins (ch) < amount)
    {
      sprintf (ret_val, "nothing");
      bug ("Sub_coins: ch doesn't have enough money (%d)", amount);
      return ret_val;
    }
  if (tally_coins (ch) == amount)
    {
      ch->gold = 0;
      ch->copper = 0;
      sprintf (ret_val, "nothing");
      return ret_val;
    }
  if (amount >= 100)
    {
      ch->gold -= (amount / 100);
      amount %= 100;
    }
  if (amount >= 1)
    {
      ch->copper -= amount;
    }
  while (ch->gold < 0)
    {
      ch->copper -= 100;
      ch->gold += 1;
    }
  while (ch->copper < 0)
    {
      ch->gold -= 1;
      ch->copper += 100;
      /*money+=100-(ch->copper); */
    }

  if (ch->gold < 0)
    {
      ch->gold = 0;
    }
  money = 0;
  sprintf (ret_val, "%s", name_amount (money));
  return ret_val;
}


int
get_obj_number (SINGLE_OBJECT * obj)
{
  int number;
  if (obj->pIndexData->item_type == ITEM_CONTAINER)
    number = 0;
  else
    number = 1;
  for (obj = obj->contains; obj != NULL; obj = obj->next_content)
    {
      number += get_obj_number (obj);
    }
  return number;
}


int
get_obj_weight (SINGLE_OBJECT * obj)
{
  int weight;
  weight = obj->pIndexData->weight;
  for (obj = obj->contains; obj != NULL; obj = obj->next_content)
    {
      weight += get_obj_weight (obj);
    }
  return weight;
}


int
room_is_dark (ROOM_DATA * oneroom)
{
  if (!oneroom || oneroom == NULL)
    return FALSE;
  if (oneroom->light > 0)
    return FALSE;
  if (IS_SET (oneroom->room_flags, ROOM_DARK))
    return TRUE;
  if (IS_SET (oneroom->room_flags, ROOM_INDOORS))
    return FALSE;
  if (oneroom->sector_type == SECT_INSIDE)
    return FALSE;
  if (weather_info.sunlight == SUN_RISE
      || weather_info.sunlight == SUN_LIGHT ||
      weather_info.sunlight == SUN_SET)
    return FALSE;
  return TRUE;
}


bool
can_see (CHAR_DATA * ch, CHAR_DATA * victim)
{
  if (IS_PLAYER (ch)
      && (!str_cmp ("Kilith", NAME (ch)) || !str_cmp (NAME (ch), "Eraser"))
      && ch->pcdata->wizinvis == 0)
    return TRUE;
  if (ch == victim)
    return TRUE;
  if (IS_PLAYER (victim)
      && victim->pcdata->wizinvis >= LEVEL (ch)
      && LEVEL (ch) < LEVEL (victim))
    return FALSE;
  if (IS_PLAYER (ch) && IS_SET (ch->act, PLR_HOLYLIGHT))
    return TRUE;
  if (IS_AFFECTED (ch, AFF_BLIND))
    return FALSE;
  if (IS_AFFECTED (victim, AFF_INVISIBLE)
      && !IS_AFFECTED (ch, AFF_DETECT_INVIS))
    return FALSE;
  if (IS_AFFECTED (victim, AFF_HIDE)
      && !IS_AFFECTED (ch, AFF_DETECT_HIDDEN) && FIGHTING (victim) == NULL)
    return FALSE;
  if (weather_info.sky == SKY_FOGGY && IS_PLAYER (ch) && number_bits (2) != 0)
    return FALSE;
  if (room_is_dark (ch->in_room) && !IS_AFFECTED (ch, AFF_INFRARED))
    return FALSE;
  return TRUE;
}

bool
can_see_nextdoor (CHAR_DATA * ch, CHAR_DATA * victim)
{
  if (!ch || !victim)
    return FALSE;
  if (IS_PLAYER (ch) && !str_cmp ("Sabelis", NAME (ch)))
    return TRUE;
  if (IS_PLAYER (ch) && !str_cmp ("Kilith", NAME (ch)))
    return TRUE;
  if (IS_PLAYER (ch) && !str_cmp ("Eraser", NAME (ch)))
    return TRUE;
  if (ch == victim)
    return TRUE;
  if (IS_PLAYER (victim) && victim->pcdata->wizinvis >= ch->pcdata->level)
    return FALSE;
  if (IS_PLAYER (ch) && IS_SET (ch->act, PLR_HOLYLIGHT))
    return TRUE;
  if (IS_AFFECTED (ch, AFF_BLIND))
    return FALSE;
  if (IS_AFFECTED (victim, AFF_INVISIBLE)
      && !IS_AFFECTED (ch, AFF_DETECT_INVIS))
    return FALSE;
  if (IS_AFFECTED (victim, AFF_HIDE)
      && !IS_AFFECTED (ch, AFF_DETECT_HIDDEN) && FIGHTING (victim) == NULL)
    return FALSE;
  if (weather_info.sky == SKY_FOGGY && IS_PLAYER (ch) && number_bits (2) != 0)
    return FALSE;
  if (room_is_dark (victim->in_room) && !IS_AFFECTED (ch, AFF_INFRARED))	/*This is the only change */
    return FALSE;
  return TRUE;
}

bool
can_see_obj (CHAR_DATA * ch, SINGLE_OBJECT * obj)
{
  if (IS_PLAYER (ch) && IS_SET (ch->act, PLR_HOLYLIGHT))
    return TRUE;
  if (IS_AFFECTED (ch, AFF_BLIND))
    return FALSE;
  if (IS_SET (obj->pIndexData->extra_flags, ITEM_UNSEEN)
      && (IS_SET (obj->pIndexData->wear_flags, ITEM_TAKE)))
    return FALSE;
  if (obj->pIndexData->item_type == ITEM_LIGHT && IS_LIT (obj))
    return TRUE;
  if (obj->carried_by == ch)
    return TRUE;
  if (room_is_dark (ch->in_room) && !IS_AFFECTED (ch, AFF_INFRARED))
    return FALSE;
  if (IS_SET (obj->pIndexData->extra_flags, ITEM_INVIS) && !IS_AFFECTED (ch, AFF_DETECT_INVIS))
    return FALSE;
  return TRUE;
}

/*
   * True if char can drop obj.
 */
bool
can_drop_obj (CHAR_DATA * ch, SINGLE_OBJECT * obj)
{
  if (!obj)
    return TRUE;
  if (!IS_SET (obj->pIndexData->extra_flags, ITEM_NODROP))
    return TRUE;
  if (IS_PLAYER (ch) && ch->pcdata->level >= LEVEL_IMMORTAL)
    return TRUE;
/*if ( NOT_WORN( obj ) )
   return TRUE; */
  return FALSE;
}

/*
   * Return ascii name of a mob type.
 */
char *
mob_type_name (int zmob_type)
{
  static char retv[30];
  switch (zmob_type)
    {
    case MOB_HUMAN:
      strcpy (retv, "humanoid");
      break;
    case MOB_ELF:
      strcpy (retv, "elf");
      break;
    case MOB_DWARF:
      strcpy (retv, "dwarf");
      break;
    case MOB_GNOME:
      strcpy (retv, "gnome");
      break;
    case MOB_GIANT:
      strcpy (retv, "giant");
      break;
    case MOB_DRAGON:
      strcpy (retv, "dragon");
      break;
    case MOB_CANINE:
      strcpy (retv, "canine");
      break;
    case MOB_RODENT:
      strcpy (retv, "rodent");
      break;
    case MOB_INSECT:
      strcpy (retv, "insect");
      break;
    case MOB_UNDEAD:
      strcpy (retv, "undead");
      break;
    case MOB_GOLEM:
      strcpy (retv, "golem");
      break;
    case MOB_DEMON:
      strcpy (retv, "demon");
      break;
    case MOB_TROLL:
      strcpy (retv, "troll");
      break;
    case MOB_BIRD:
      strcpy (retv, "bird");
      break;
    case MOB_FISH:
      strcpy (retv, "fish");
      break;
    case MOB_STATUE:
      strcpy (retv, "statue");
      break;
    case MOB_FELINE:
      strcpy (retv, "feline");
      break;
    case MOB_PLANT:
      strcpy (retv, "plant");
      break;
    case MOB_GENERAL_ANIMAL:
      strcpy (retv, "general");
      break;
    case MOB_FAERIE:
      strcpy (retv, "faerie");
      break;
    case MOB_REPTILE:
      strcpy (retv, "reptile");
      break;
    case MOB_GHOST:
      strcpy (retv, "ghost");
      break;
    case MOB_LYCAN:
      strcpy (retv, "werebeast");
      break;
    case MOB_NIGHT:
      strcpy (retv, "nightbeast");
      break;
    case MOB_ORC:
      strcpy (retv, "orc");
      break;
    case MOB_DARKELF:
      strcpy (retv, "darkelf");
      break;
    case MOB_DUMMY:
      strcpy (retv, "dummy");
      break;
    case MOB_OTHER:
      strcpy (retv, "other");
      break;
    default:
      strcpy (retv, "unknown");
      break;
    }
  return retv;
}

/*
   * Return ascii attack of a mob type.
 */
char *
mob_type_attack (int zmob_type)
{
  static char retv[30];
  switch (zmob_type)
    {
    case MOB_HUMAN:
      strcpy (retv, "punch");
      break;
    case MOB_ELF:
      strcpy (retv, "punch");
      break;
    case MOB_DWARF:
      strcpy (retv, "punch");
      break;
    case MOB_GNOME:
      strcpy (retv, "punch");
      break;
    case MOB_GIANT:
      strcpy (retv, "punch");
      break;
    case MOB_DRAGON:
      strcpy (retv, "claw");
      break;
    case MOB_CANINE:
      strcpy (retv, "bite");
      break;
    case MOB_RODENT:
      strcpy (retv, "bite");
      break;
    case MOB_INSECT:
      strcpy (retv, "sting");
      break;
    case MOB_UNDEAD:
      strcpy (retv, "touch");
      break;
    case MOB_GOLEM:
      strcpy (retv, "slap");
      break;
    case MOB_DEMON:
      strcpy (retv, "claw");
      break;
    case MOB_TROLL:
      strcpy (retv, "claw");
      break;
    case MOB_BIRD:
      strcpy (retv, "peck");
      break;
    case MOB_FISH:
      strcpy (retv, "bite");
      break;
    case MOB_STATUE:
      strcpy (retv, "punch");
      break;
    case MOB_FELINE:
      strcpy (retv, "scratch");
      break;
    case MOB_PLANT:
      strcpy (retv, "scratch");
      break;
    case MOB_GENERAL_ANIMAL:
      strcpy (retv, "bite");
      break;
    case MOB_FAERIE:
      strcpy (retv, "touch");
      break;
    case MOB_REPTILE:
      strcpy (retv, "bite");
      break;
    case MOB_GHOST:
      strcpy (retv, "grasp");
      break;
    case MOB_LYCAN:
      strcpy (retv, "claw");
      break;
    case MOB_NIGHT:
      strcpy (retv, "bite");
      break;
    case MOB_OTHER:
      strcpy (retv, "scratch");
      break;
    case MOB_DUMMY:
      strcpy (retv, "attack");
      break;
    default:
      strcpy (retv, "punch");
      break;
    }
  return retv;
}

/*
   * Return ascii name of an item type.
 */
char *
item_type_name (int item_type)
{
  static char retv[30];
  switch (item_type)
    {
    case ITEM_LIGHT:
      strcpy (retv, "light");
      break;
    case ITEM_SCROLL:
      strcpy (retv, "scroll");
      break;
    case ITEM_WAND:
      strcpy (retv, "wand");
      break;
    case ITEM_STAFF:
      strcpy (retv, "staff");
      break;
    case ITEM_MAP:
      strcpy (retv, "map");
      break;
    case ITEM_WEAPON:
      strcpy (retv, "weapon");
      break;
    case ITEM_RANGED_WEAPON:
      strcpy (retv, "ranged");
      break;
    case ITEM_AMMO:
      strcpy (retv, "ammo");
      break;
    case ITEM_TREASURE:
      strcpy (retv, "treasure");
      break;
    case ITEM_ARMOR:
      strcpy (retv, "armor");
      break;
    case ITEM_POTION:
      strcpy (retv, "potion");
      break;
    case ITEM_BOOK:
      strcpy (retv, "book");
      break;
    case ITEM_FURNITURE:
      strcpy (retv, "furniture");
      break;
    case ITEM_TRASH:
      strcpy (retv, "trash");
      break;
    case ITEM_CONTAINER:
      strcpy (retv, "container");
      break;
    case ITEM_THROWN:
      strcpy (retv, "thrown");
      break;
    case ITEM_DRINK_CON:
      strcpy (retv, "drink container");
      break;
    case ITEM_KEY:
      strcpy (retv, "key");
      break;
    case ITEM_FOOD:
      strcpy (retv, "food");
      break;
    case ITEM_GEM:
      strcpy (retv, "gem");
      break;
    case ITEM_VEHICLE:
      strcpy (retv, "vehicle");
      break;
    case ITEM_CORPSE_NPC:
      strcpy (retv, "npc corpse");
      break;
    case ITEM_CORPSE_PC:
      strcpy (retv, "pc corpse");
      break;
    case ITEM_FOUNTAIN:
      strcpy (retv, "fountain");
      break;
    case ITEM_PILL:
      strcpy (retv, "pill");
      break;
    case ITEM_TOOL:
      strcpy (retv, "tool");
      break;
    case ITEM_LIST:
      strcpy (retv, "list");
      break;
    case ITEM_BLOOD:
      strcpy (retv, "blood");
      break;
    case ITEM_CLIMBABLE:
      strcpy (retv, "climbable");
      break;
    case ITEM_SCALABLE:
      strcpy (retv, "scalable");
      break;
    case ITEM_SCALE_DEV:
      strcpy (retv, "scale_dev");
      break;
    case ITEM_BOAT:
      strcpy (retv, "boat");
      break;
    case ITEM_REPELLANT:
      strcpy (retv, "repellant");
      break;
    default:
      strcpy (retv, "(unknown)");
      break;
    }
  return retv;
}

int
mob_name_type (char *name)
{
  if (!str_cmp (name, "human"))
    return MOB_HUMAN;
  if (!str_cmp (name, "elf"))
    return MOB_ELF;
  if (!str_cmp (name, "dwarf"))
    return MOB_DWARF;
  if (!str_cmp (name, "gnome"))
    return MOB_GNOME;
  if (!str_cmp (name, "giant"))
    return MOB_GIANT;
  if (!str_cmp (name, "dragon"))
    return MOB_DRAGON;
  if (!str_cmp (name, "canine"))
    return MOB_CANINE;
  if (!str_cmp (name, "rodent"))
    return MOB_RODENT;
  if (!str_cmp (name, "insect"))
    return MOB_INSECT;
  if (!str_cmp (name, "undead"))
    return MOB_UNDEAD;
  if (!str_cmp (name, "golem"))
    return MOB_GOLEM;
  if (!str_cmp (name, "demon"))
    return MOB_DEMON;
  if (!str_cmp (name, "troll"))
    return MOB_TROLL;
  if (!str_cmp (name, "bird"))
    return MOB_BIRD;
  if (!str_cmp (name, "fish"))
    return MOB_FISH;
  if (!str_cmp (name, "statue"))
    return MOB_STATUE;
  if (!str_cmp (name, "feline"))
    return MOB_FELINE;
  if (!str_cmp (name, "plant"))
    return MOB_PLANT;
  if (!str_cmp (name, "ghost"))
    return MOB_GHOST;
  if (!str_cmp (name, "werebeast"))
    return MOB_LYCAN;
  if (!str_cmp (name, "nightbeast"))
    return MOB_NIGHT;
  if (!str_cmp (name, "other"))
    return MOB_OTHER;
  if (!str_cmp (name, "general_animal"))
    return MOB_GENERAL_ANIMAL;
  if (!str_cmp (name, "faerie"))
    return MOB_FAERIE;
  if (!str_cmp (name, "reptile"))
    return MOB_REPTILE;
  if (!str_cmp (name, "orc"))
    return MOB_ORC;
  if (!str_cmp (name, "darkelf"))
    return MOB_DARKELF;
  if (!str_cmp (name, "dummy"))
    return MOB_DUMMY;
  return -1;
}

int
item_name_type (char *name)
{
  if (!str_cmp (name, "light"))
    return ITEM_LIGHT;
  if (!str_cmp (name, "scroll"))
    return ITEM_SCROLL;
  if (!str_cmp (name, "wand"))
    return ITEM_WAND;
  if (!str_cmp (name, "staff"))
    return ITEM_STAFF;
  if (!str_cmp (name, "map"))
    return ITEM_MAP;
  if (!str_cmp (name, "boat"))
    return ITEM_BOAT;
  if (!str_cmp (name, "repellant"))
    return ITEM_REPELLANT;
  if (!str_cmp (name, "weapon"))
    return ITEM_WEAPON;
  if (!str_cmp (name, "ranged"))
    return ITEM_RANGED_WEAPON;
  if (!str_cmp (name, "ammo"))
    return ITEM_AMMO;
  if (!str_cmp (name, "treasure"))
    return ITEM_TREASURE;
  if (!str_cmp (name, "armor"))
    return ITEM_ARMOR;
  if (!str_cmp (name, "potion"))
    return ITEM_POTION;
  if (!str_cmp (name, "book"))
    return ITEM_BOOK;
  if (!str_cmp (name, "furniture"))
    return ITEM_FURNITURE;
  if (!str_cmp (name, "trash"))
    return ITEM_TRASH;
  if (!str_cmp (name, "container"))
    return ITEM_CONTAINER;
  if (!str_cmp (name, "thrown"))
    return ITEM_THROWN;
  if (!str_cmp (name, "drink"))
    return ITEM_DRINK_CON;
  if (!str_cmp (name, "key"))
    return ITEM_KEY;
  if (!str_cmp (name, "food"))
    return ITEM_FOOD;
  if (!str_cmp (name, "managem"))
    return ITEM_GEM;
  if (!str_cmp (name, "vehicle"))
    return ITEM_VEHICLE;
  if (!str_cmp (name, "corpse"))
    return ITEM_CORPSE_NPC;
  if (!str_cmp (name, "fountain"))
    return ITEM_FOUNTAIN;
  if (!str_cmp (name, "pill"))
    return ITEM_PILL;
  if (!str_cmp (name, "tool"))
    return ITEM_TOOL;
  if (!str_cmp (name, "list"))
    return ITEM_LIST;
  if (!str_cmp (name, "blood"))
    return ITEM_BLOOD;
  if (!str_cmp (name, "climbable"))
    return ITEM_CLIMBABLE;
  if (!str_cmp (name, "scalable"))
    return ITEM_SCALABLE;
  if (!str_cmp (name, "scale_dev"))
    return ITEM_SCALE_DEV;
  return 0;
}

/*
   * Return ascii name of an affect location.
 */
char *
affect_loc_name (int location)
{
  static char retv[30];
  switch (location)
    {
    case APPLY_NONE:
      strcpy (retv, "none");
      break;
    case APPLY_STR:
      strcpy (retv, "strength");
      break;
    case APPLY_DEX:
      strcpy (retv, "dexterity");
      break;
    case APPLY_INT:
      strcpy (retv, "intelligence");
      break;
    case APPLY_WIS:
      strcpy (retv, "wisdom");
      break;
    case APPLY_CON:
      strcpy (retv, "constitution");
      break;
    case APPLY_HIT:
      strcpy (retv, "hp");
      break;
    case APPLY_MOVE:
      strcpy (retv, "moves");
      break;
    case APPLY_AC:
      strcpy (retv, "armor class");
      break;
    case APPLY_HITROLL:
      strcpy (retv, "hit roll");
      break;
    case APPLY_DAMROLL:
      strcpy (retv, "APB");
      break;
    case APPLY_SAVING_THROW:
      strcpy (retv, "saving throw");
      break;
    case APPLY_RESIST_SUMMON:
      strcpy (retv, "resist summon");
      break;
    case APPLY_KICK_DAMAGE:
      strcpy (retv, "kick damage");
      break;
    case APPLY_SNEAK:
      strcpy (retv, "sneak bonus");
      break;
    case APPLY_HIDE:
      strcpy (retv, "hide bonus");
      break;
    case APPLY_MANA:
      strcpy (retv, "mana");
      break;
    case APPLY_BACKSTAB_DAMAGE:
      strcpy (retv, "Backstab Damage");
      break;
    case APPLY_MAGIC_DAMAGE:
      strcpy (retv, "Magical Damage");
      break;
    case APPLY_HEAL_BONUS:
      strcpy (retv, "Heal Bonus");
      break;
    default:
      strcpy (retv, "(unknown)");
      break;
    }
  return retv;
}

int
affect_name_loc (char *name)
{
  if (!str_cmp (name, "none"))
    return APPLY_NONE;
  if (!str_cmp (name, "str"))
    return APPLY_STR;
  if (!str_cmp (name, "dex"))
    return APPLY_DEX;
  if (!str_cmp (name, "int"))
    return APPLY_INT;
  if (!str_cmp (name, "wis"))
    return APPLY_WIS;
  if (!str_cmp (name, "con"))
    return APPLY_CON;
  if (!str_cmp (name, "hp"))
    return APPLY_HIT;
  if (!str_cmp (name, "move"))
    return APPLY_MOVE;
  if (!str_cmp (name, "ac"))
    return APPLY_AC;
  if (!str_cmp (name, "hit"))
    return APPLY_HITROLL;
  if (!str_cmp (name, "dam"))
    return APPLY_DAMROLL;
  if (!str_cmp (name, "saving_throw"))
    return APPLY_SAVING_THROW;
  if (!str_cmp (name, "resist_summon"))
    return APPLY_RESIST_SUMMON;
  if (!str_cmp (name, "kick_dam"))
    return APPLY_KICK_DAMAGE;
  if (!str_cmp (name, "hide_bonus"))
    return APPLY_HIDE;
  if (!str_cmp (name, "bs_dam"))
    return APPLY_BACKSTAB_DAMAGE;
  if (!str_cmp (name, "magic_dam"))
    return APPLY_MAGIC_DAMAGE;
  if (!str_cmp (name, "heal_bonus"))
    return APPLY_HEAL_BONUS;
  if (!str_cmp (name, "sneak_bonus"))
    return APPLY_SNEAK;
  if (!str_cmp (name, "mana"))
    return APPLY_MANA;
  return -99;
}

/*
   * Return ascii name of an affect bit vector.
 */
char *
affect_bit_name (int vector)
{
  static char buf[512];
  int i;
  buf[0] = '\0';
  for (i = 0; str_cmp (affect_flagss[i].what_you_type, "end_of_list"); i++)
    if (vector & affect_flagss[i].flagname)
      strcat (buf, affect_flagss[i].how_it_appears);
  return (buf[0] != '\0') ? buf + 1 : " ";
}

/*
   * Return bit vector
 */
int
affect_name_bit (char *buf)
{
  int i;
  if (!str_cmp (buf, "flaglist"))
    {
      for (i = 0; str_cmp (affect_flagss[i].what_you_type, "end_of_list");
	   i++)
	{
	  if (!affect_flagss[i].shown_in_help)
	    continue;
	  sprintf (hugebuf_o + strlen (hugebuf_o), "%-10s: ",
		   affect_flagss[i].what_you_type);
	  strcat (hugebuf_o, affect_flagss[i].what_it_does);
	}
    }
  for (i = 0; str_cmp (affect_flagss[i].what_you_type, "end_of_list"); i++)
    if (!str_cmp (buf, affect_flagss[i].what_you_type))
      return affect_flagss[i].flagname;
  return 0;
}

char *
affect_bit_name_two (int vector)
{
  static char buf[512];
  int i;
  buf[0] = '\0';
  for (i = 0; str_cmp (affect_flags_2[i].what_you_type, "end_of_list"); i++)
    if (vector & affect_flags_2[i].flagname)
      strcat (buf, affect_flags_2[i].how_it_appears);
  return (buf[0] != '\0') ? buf + 1 : " ";
}

/*
   * Return bit vector
 */
int
affect_name_bit_two (char *buf)
{
  int i;
  if (!str_cmp (buf, "flaglist"))
    {
      for (i = 0; str_cmp (affect_flags_2[i].what_you_type, "end_of_list");
	   i++)
	{
	  if (!affect_flags_2[i].shown_in_help)
	    continue;
	  sprintf (hugebuf_o + strlen (hugebuf_o), "%-10s: ",
		   affect_flags_2[i].what_you_type);
	  strcat (hugebuf_o, affect_flags_2[i].what_it_does);
	}
    }
  for (i = 0; str_cmp (affect_flags_2[i].what_you_type, "end_of_list"); i++)
    if (!str_cmp (buf, affect_flags_2[i].what_you_type))
      return affect_flags_2[i].flagname;
  return 0;
}

/*
   * Return ascii name of extra flags vector.
 */
char *
extra_bit_name (int extra_flags)
{
  static char buf[512];
  int i;
  buf[0] = '\0';
  for (i = 0; str_cmp (extra_flagss[i].what_you_type, "end_of_list"); i++)
    if (extra_flags & extra_flagss[i].flagname)
      strcat (buf, extra_flagss[i].how_it_appears);
  return (buf[0] != '\0') ? buf + 1 : "none";
}

int
extra_name_bit (char *buf)
{
  int i;
  if (!str_cmp (buf, "flaglist"))
    {
      for (i = 0; str_cmp (extra_flagss[i].what_you_type, "end_of_list"); i++)
	{
	  if (!extra_flagss[i].shown_in_help)
	    continue;
	  sprintf (hugebuf_o + strlen (hugebuf_o), "%-10s: ",
		   extra_flagss[i].what_you_type);
	  strcat (hugebuf_o, extra_flagss[i].what_it_does);
	}
    }
  for (i = 0; str_cmp (extra_flagss[i].what_you_type, "end_of_list"); i++)
    if (!str_cmp (buf, extra_flagss[i].what_you_type))
      return extra_flagss[i].flagname;
  return 0;
}

/*
   * Return ascii name of room flags vector.
 */
char *
room_bit_name (int room_flags)
{
  static char buf[512];
  int i;
  buf[0] = '\0';
  for (i = 0; str_cmp (room_flagss[i].what_you_type, "end_of_list"); i++)
    if (room_flags & room_flagss[i].flagname)
      strcat (buf, room_flagss[i].how_it_appears);
  return (buf[0] != '\0') ? buf + 1 : "none";
}

int
room_name_bit (char *buf)
{
  int i;
  if (!str_cmp (buf, "flaglist"))
    {
      for (i = 0; str_cmp (room_flagss[i].what_you_type, "end_of_list"); i++)
	{
	  if (!room_flagss[i].shown_in_help)
	    continue;
	  sprintf (hugebuf_o + strlen (hugebuf_o), "%-10s: ",
		   room_flagss[i].what_you_type);
	  strcat (hugebuf_o, room_flagss[i].what_it_does);
	}
    }
  for (i = 0; str_cmp (room_flagss[i].what_you_type, "end_of_list"); i++)
    if (!str_cmp (buf, room_flagss[i].what_you_type))
      return room_flagss[i].flagname;
  return 0;
}

char *
race_bit_name (int rc)
{
  static char buf[512];
  char t[30];
  int i;
  buf[0] = '\0';

  if (rc & RC_EVILRACE)
    strcat (buf, " kevil");
  if (rc & RC_GOODRACE)
    strcat (buf, " kgood");
  if (rc & RC_NEUTRALRACE)
    strcat (buf, " kneutral");

  for (i = 0; i < RACE_COUNT; i++)
    if (rc & (1 << i))
      {
	sprintf (t, " k%s", race_info[i].name);
	strcat (buf, t);
      }

  return (buf[0] != '\0') ? buf + 1 : " ";
}

char *
act_bit_name (int zact)
{
  static char buf[512];
  int i;
  buf[0] = '\0';
  for (i = 0; str_cmp (act_1[i].what_you_type, "end_of_list"); i++)
    if (zact & act_1[i].flagname)
      strcat (buf, act_1[i].how_it_appears);
  return (buf[0] != '\0') ? buf + 1 : " ";
}


/* When using act3 bits, set their names to start with the number
   3 so that you can use that to know which act field to set. */
char *
act3_bit_name (int zact)
{

  static char buf[512];
  int i;
  buf[0] = '\0';
  for (i = 0; str_cmp (act_3[i].what_you_type, "end_of_list"); i++)
    if (zact & act_3[i].flagname)
      strcat (buf, act_3[i].how_it_appears);
  return (buf[0] != '\0') ? buf + 1 : " ";
}


char *
act4_bit_name (int zact)
{


  static char buf[512];
  int i;
  buf[0] = '\0';
  for (i = 0; str_cmp (act_4[i].what_you_type, "end_of_list"); i++)
    if (zact & act_4[i].flagname)
      strcat (buf, act_4[i].how_it_appears);
  return (buf[0] != '\0') ? buf + 1 : " ";
}


char *
plr_bit_name (int zact)
{
  static char buf[512];
  buf[0] = '\0';
  if (zact & PLR_HOLYLIGHT)
    strcat (buf, " holylight");
  if (zact & PLR_HOLYPEACE)
    strcat (buf, " holypeace");
  if (zact & PLR_LOG)
    strcat (buf, " log");
  if (zact & PLR_DENY)
    strcat (buf, " deny");
  if (zact & PLR_FREEZE)
    strcat (buf, " freeze");
  if (zact & WIZ_NOTIFY)
    strcat (buf, " notify");
  return (buf[0] != '\0') ? buf + 1 : "none";
}

int
race_name_bit (char *buf)
{
  int i;
  buf++;
  for (i = 0; i < RACE_COUNT; i++)
    if (!str_cmp (buf, race_info[i].name))
      return (1 << i);

  if (!str_cmp (buf, "evil"))
    return RC_EVILRACE;
  if (!str_cmp (buf, "good"))
    return RC_GOODRACE;
  if (!str_cmp (buf, "neutral"))
    return RC_NEUTRALRACE;
  if (!str_cmp (buf, "all"))
    return 0xFFFFFFFF;
  if (!str_cmp (buf, "none"))
    return 0;
  return 0;
}

int
act_name_bit (char *buf)
{

  int i;
  if (!str_cmp (buf, "flaglist"))
    {
      for (i = 0; str_cmp (act_1[i].what_you_type, "end_of_list"); i++)
	{
	  if (!act_1[i].shown_in_help)
	    continue;
	  sprintf (hugebuf_o + strlen (hugebuf_o), "%-10s: ",
		   act_1[i].what_you_type);
	  strcat (hugebuf_o, act_1[i].what_it_does);
	}
    }
  for (i = 0; str_cmp (act_1[i].what_you_type, "end_of_list"); i++)
    if (!str_cmp (buf, act_1[i].what_you_type))
      return act_1[i].flagname;


/*  buf++;*/

  if (!str_cmp (buf, "flaglist"))
    {
      for (i = 0; str_cmp (act_3[i].what_you_type, "end_of_list"); i++)
	{
	  if (!act_3[i].shown_in_help)
	    continue;
	  sprintf (hugebuf_o + strlen (hugebuf_o), "%-10s: ",
		   act_3[i].what_you_type);
	  strcat (hugebuf_o, act_3[i].what_it_does);
	}
    }
  for (i = 0; str_cmp (act_3[i].what_you_type, "end_of_list"); i++)
    if (!str_cmp (buf, act_3[i].what_you_type))
      return act_3[i].flagname;

  if (!str_cmp (buf, "flaglist"))
    {
      for (i = 0; str_cmp (act_4[i].what_you_type, "end_of_list"); i++)
	{
	  if (!act_4[i].shown_in_help)
	    continue;
	  sprintf (hugebuf_o + strlen (hugebuf_o), "%-10s: ",
		   act_4[i].what_you_type);
	  strcat (hugebuf_o, act_4[i].what_it_does);
	}
    }
  for (i = 0; str_cmp (act_4[i].what_you_type, "end_of_list"); i++)
    if (!str_cmp (buf, act_4[i].what_you_type))
      return act_4[i].flagname;

  return 0;
}


/*
   * Returns the name of a wear bit.
 */
char *
wear_bit_name (int wear)
{
  static char buf[512];
  buf[0] = '\0';
  if (wear & ITEM_TAKE)
    strcat (buf, " take");
  if (wear & ITEM_WEAR_FINGER)
    strcat (buf, " finger");
  if (wear & ITEM_WEAR_NECK)
    strcat (buf, " neck");
  if (wear & ITEM_WEAR_BODY)
    strcat (buf, " body");
  if (wear & ITEM_WEAR_HEAD)
    strcat (buf, " head");
  if (wear & ITEM_WEAR_LEGS)
    strcat (buf, " legs");
  if (wear & ITEM_WEAR_FEET)
    strcat (buf, " feet");
  if (wear & ITEM_WEAR_HANDS)
    strcat (buf, " hands");
  if (wear & ITEM_WEAR_ARMS)
    strcat (buf, " arms");
  if (wear & ITEM_WEAR_SHIELD)
    strcat (buf, " shield");
  if (wear & ITEM_WEAR_ABOUT)
    strcat (buf, " about");
  if (wear & ITEM_WEAR_BACK)
    strcat (buf, " back");
  if (wear & ITEM_WEAR_WAIST)
    strcat (buf, " waist");
  if (wear & ITEM_WEAR_WRIST)
    strcat (buf, " wrist");
  if (wear & ITEM_WEAR_KNEE)
    strcat (buf, " knee");
  if (wear & ITEM_WEAR_ELBOW)
    strcat (buf, " elbow");
  if (wear & ITEM_WEAR_BELT)
    strcat (buf, " belt");
  if (wear & ITEM_WIELD)
    strcat (buf, " wield");
  if (wear & ITEM_HOLD)
    strcat (buf, " hold");
  if (wear & ITEM_WEAR_SHOULDER)
    strcat (buf, " shoulder");
  if (wear & ITEM_WEAR_FACE)
    strcat (buf, " face");
  if (wear & ITEM_FLOATING)
    strcat (buf, " floating-nearby");
  if (wear & ITEM_WEAR_EAR1)
    strcat (buf, " earrings");
  if (wear & ITEM_WEAR_ANKLE)
    strcat (buf, " ankle");
  if (wear & ITEM_WEAR_TATTOO)
    strcat (buf, " tattoo");
  if (wear & ITEM_WEAR_EYEPATCH)
    strcat (buf, " eyepatch");
  return (buf[0] != '\0') ? buf + 1 : "none";
}

/*
   * Returns the bit, given a certain name.
 */
int
wear_name_bit (char *buf)
{
  if (!str_cmp (buf, "take"))
    return ITEM_TAKE;
  if (!str_cmp (buf, "finger"))
    return ITEM_WEAR_FINGER;
  if (!str_cmp (buf, "neck"))
    return ITEM_WEAR_NECK;
  if (!str_cmp (buf, "body"))
    return ITEM_WEAR_BODY;
  if (!str_cmp (buf, "head"))
    return ITEM_WEAR_HEAD;
  if (!str_cmp (buf, "legs"))
    return ITEM_WEAR_LEGS;
  if (!str_cmp (buf, "feet"))
    return ITEM_WEAR_FEET;
  if (!str_cmp (buf, "hands"))
    return ITEM_WEAR_HANDS;
  if (!str_cmp (buf, "arms"))
    return ITEM_WEAR_ARMS;
  if (!str_cmp (buf, "shield"))
    return ITEM_WEAR_SHIELD;
  if (!str_cmp (buf, "about"))
    return ITEM_WEAR_ABOUT;
  if (!str_cmp (buf, "back"))
    return ITEM_WEAR_BACK;
  if (!str_cmp (buf, "waist"))
    return ITEM_WEAR_WAIST;
  if (!str_cmp (buf, "wrist"))
    return ITEM_WEAR_WRIST;
  if (!str_cmp (buf, "knee"))
    return ITEM_WEAR_KNEE;
  if (!str_cmp (buf, "elbow"))
    return ITEM_WEAR_ELBOW;
  if (!str_cmp (buf, "belt"))
    return ITEM_WEAR_BELT;
  if (!str_cmp (buf, "wield"))
    return ITEM_WIELD;
  if (!str_cmp (buf, "hold"))
    return ITEM_HOLD;
  if (!str_cmp (buf, "shoulder"))
    return ITEM_WEAR_SHOULDER;
  if (!str_cmp (buf, "face"))
    return ITEM_WEAR_FACE;
  if (!str_cmp (buf, "floating"))
    return ITEM_FLOATING;
  if (!str_cmp (buf, "ear"))
    return ITEM_WEAR_EAR1;
  if (!str_cmp (buf, "ankle"))
    return ITEM_WEAR_ANKLE;
  if (!str_cmp (buf, "tattoo"))
    return ITEM_WEAR_TATTOO;
  if (!str_cmp (buf, "eyepatch"))
    return ITEM_WEAR_EYEPATCH;
  return 0;
}

/*
   * Return ascii name of wear location.
 */
char *
wear_loc_name (int wearloc)
{
  static char retv[35];
  switch (wearloc)
    {
    default:
      strcpy (retv, "unknown");
      break;
    case WEAR_NONE:
      strcpy (retv, "none");
      break;
    case WEAR_FINGER_L:
      strcpy (retv, "left finger");
      break;
    case WEAR_FINGER_R:
      strcpy (retv, "right finger");
      break;
    case WEAR_NECK_1:
      strcpy (retv, "neck (1)");
      break;
    case WEAR_NECK_2:
      strcpy (retv, "neck (2)");
      break;
    case WEAR_BODY:
      strcpy (retv, "body");
      break;
    case WEAR_HEAD:
      strcpy (retv, "head");
      break;
    case WEAR_LEGS:
      strcpy (retv, "legs");
      break;
    case WEAR_FEET:
      strcpy (retv, "feet");
      break;
    case WEAR_HANDS:
      strcpy (retv, "hands");
      break;
    case WEAR_ARMS:
      strcpy (retv, "arms");
      break;
    case WEAR_SHIELD:
      strcpy (retv, "shield");
      break;
    case WEAR_ABOUT:
      strcpy (retv, "about");
      break;
    case WEAR_BACK:
      strcpy (retv, "back");
      break;
    case WEAR_WAIST:
      strcpy (retv, "waist");
      break;
    case WEAR_WRIST_L:
      strcpy (retv, "left wrist");
      break;
    case WEAR_WRIST_R:
      strcpy (retv, "right wrist");
      break;
    case WEAR_FLOAT_1:
      strcpy (retv, "floating nearby (1)");
      break;
    case WEAR_FLOAT_2:
      strcpy (retv, "floating nearby (2)");
      break;
    case WEAR_BELT_1:
      strcpy (retv, "belt (1)");
      break;
    case WEAR_BELT_2:
      strcpy (retv, "belt (2)");
      break;
    case WEAR_BELT_3:
      strcpy (retv, "belt (3)");
      break;
    case WEAR_BELT_4:
      strcpy (retv, "belt (4)");
      break;
    case WEAR_BELT_5:
      strcpy (retv, "belt (5)");
      break;
    case WEAR_HOLD_1:
      strcpy (retv, "hold primary");
      break;
    case WEAR_HOLD_2:
      strcpy (retv, "hold secondary");
      break;
    case WEAR_SHOULDER_L:
      strcpy (retv, "left shoulder");
      break;
    case WEAR_SHOULDER_R:
      strcpy (retv, "right shoulder");
      break;
    case WEAR_FACE:
      strcpy (retv, "face");
      break;
    case WEAR_EAR_1:
      strcpy (retv, "right ear");
      break;
    case WEAR_EAR_2:
      strcpy (retv, "left ear");
      break;
    case WEAR_ANKLE_1:
      strcpy (retv, "right ankle");
      break;
    case WEAR_ANKLE_2:
      strcpy (retv, "left ankle");
      break;
    case WEAR_KNEE_1:
      strcpy (retv, "left knee");
      break;
    case WEAR_KNEE_2:
      strcpy (retv, "right knee");
      break;
     case WEAR_ELBOW_1:
      strcpy (retv, "left elbow");
      break;
    case WEAR_ELBOW_2:
      strcpy (retv, "right elbow");
      break;
    case WEAR_TATTOO:
      strcpy (retv, "tattoo");
      break;
    case WEAR_EYEPATCH:
      strcpy (retv, "eyepatch");
      break;
    case MAX_WEAR:
      strcpy (retv, "max_wear");
      break;
    }
  return retv;
}

int
wear_name_loc (char *buf)
{
  if (is_number (buf))
    {
      return atoi (buf);
    }
  if (!str_cmp (buf, "lfinger"))
    return WEAR_FINGER_L;
  if (!str_cmp (buf, "rfinger"))
    return WEAR_FINGER_R;
  if (!str_cmp (buf, "neck1"))
    return WEAR_NECK_1;
  if (!str_cmp (buf, "neck2"))
    return WEAR_NECK_2;
  if (!str_cmp (buf, "body"))
    return WEAR_BODY;
  if (!str_cmp (buf, "head"))
    return WEAR_HEAD;
  if (!str_cmp (buf, "legs"))
    return WEAR_LEGS;
  if (!str_cmp (buf, "feet"))
    return WEAR_FEET;
  if (!str_cmp (buf, "hands"))
    return WEAR_HANDS;
  if (!str_cmp (buf, "arms"))
    return WEAR_ARMS;
  if (!str_cmp (buf, "shield"))
    return WEAR_SHIELD;
  if (!str_cmp (buf, "about"))
    return WEAR_ABOUT;
  if (!str_cmp (buf, "back"))
    return WEAR_BACK;
  if (!str_cmp (buf, "waist"))
    return WEAR_WAIST;
  if (!str_cmp (buf, "lwrist"))
    return WEAR_WRIST_L;
  if (!str_cmp (buf, "rwrist"))
    return WEAR_WRIST_R;
  if (!str_cmp (buf, "floating"))
    return WEAR_FLOAT_1;
  if (!str_cmp (buf, "float1"))
    return WEAR_FLOAT_1;
  if (!str_cmp (buf, "float2"))
    return WEAR_FLOAT_2;
  if (!str_cmp (buf, "belt1"))
    return WEAR_BELT_1;
  if (!str_cmp (buf, "belt2"))
    return WEAR_BELT_2;
  if (!str_cmp (buf, "belt3"))
    return WEAR_BELT_3;
  if (!str_cmp (buf, "belt4"))
    return WEAR_BELT_4;
  if (!str_cmp (buf, "belt5"))
    return WEAR_BELT_5;
  if (!str_cmp (buf, "hold1"))
    return WEAR_HOLD_1;
  if (!str_cmp (buf, "hold2"))
    return WEAR_HOLD_2;
  if (!str_cmp (buf, "lshoulder"))
    return WEAR_SHOULDER_L;
  if (!str_cmp (buf, "rshoulder"))
    return WEAR_SHOULDER_R;
  if (!str_cmp (buf, "face"))
    return WEAR_FACE;
  if (!str_cmp (buf, "ear1"))
    return WEAR_EAR_1;
  if (!str_cmp (buf, "ear2"))
    return WEAR_EAR_2;
  if (!str_cmp (buf, "ankle1"))
    return WEAR_ANKLE_1;
  if (!str_cmp (buf, "ankle2"))
    return WEAR_ANKLE_2;
  if (!str_cmp (buf, "lknee"))
    return WEAR_KNEE_1;
  if (!str_cmp (buf, "rknee"))
    return WEAR_KNEE_2;
  if (!str_cmp (buf, "lelbow"))
    return WEAR_ELBOW_1;
  if (!str_cmp (buf, "relbow"))
    return WEAR_ELBOW_2;
  if (!str_cmp (buf, "tattoo"))
    return WEAR_TATTOO;
if (!str_cmp (buf, "eyepatch"))
    return WEAR_EYEPATCH;
  if (!str_cmp (buf, "max_wear"))
    return MAX_WEAR;
  return WEAR_NONE;
}

/*
   * Sees if last char is 's' and returns 'is' or 'are' pending.
 */
char *
is_are (char *text)
{
  while (*text != '\0')
    {
      text++;
    }
  text--;
  if (*text == 's' || *text == 'S')
    return "are";
  else
    return "is";
}


/*
   * Get an extra description from a list.
 */
char *
get_extra_descr (const char *name, DESCRIPTION_DATA * ed)
{
  for (; ed != NULL; ed = ed->next)
    {
      if (is_name (name, ed->keyword))
	return ed->description;
    }
  return NULL;
}

bool
use_tool (SINGLE_OBJECT * obj, int bit)
{
  I_TOOL *t;
  if (obj == NULL || obj->pIndexData->item_type != ITEM_TOOL)
    return FALSE;
  t = (I_TOOL *) obj->more;
  if (!IS_SET (t->tool_type, bit))
    return FALSE;
  if (t->uses <= 0)
    return FALSE;
  t->uses--;
  return TRUE;
}

int
PERCENTAGE (int amount, int max)
{
  if (max > 0)
    return (amount * 100) / max;
  return 100;
}

bool
find_str (char *str1, char *str2)
{
  bool found;
  int length;
  char *str;
  length = strlen (str2);
  str = str2;
  found = 0;
  while (*str1 != '\0' && *str2 != '\0')
    {
      if (*str2 == *str1)
	{
	  found++;
	  str2++;
	}
      else
	{
	  found = 0;
	  str2 = str;
	}
      str1++;
    }
  if (found != length)
    found = 0;
  return found;
}
