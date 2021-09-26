#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"

void say_spell (CHAR_DATA * ch, int sn);
void
renew_affect (CHAR_DATA * ch, AFFECT_DATA * aff)
{
  AFFECT_DATA *paf;
  AFFECT_DATA pasaf;
  bool foundd;
  foundd = FALSE;
  bzero (&pasaf, sizeof (pasaf));
  pasaf.type = aff->type;
  pasaf.duration = aff->duration;
  pasaf.location = aff->location;
  pasaf.modifier = aff->modifier;
  pasaf.bitvector = aff->bitvector;
  pasaf.bitvector2 = aff->bitvector2;
  if (aff->long_msg)
    pasaf.long_msg = strdup (aff->long_msg);
  else
    pasaf.long_msg = NULL;
  if (aff->short_msg)
    pasaf.short_msg = strdup (aff->short_msg);
  else
    pasaf.short_msg = NULL;
  for (paf = ch->affected; paf != NULL; paf = paf->next)
    {
      if (paf->type == aff->type && paf->location == aff->location)
	{
	  paf->duration = aff->duration;
	  foundd = TRUE;
	}
    }
  if (!foundd)
    affect_to_char (ch, &pasaf);
  return;
}


void
mana (CHAR_DATA * ch, char *argy)
{
  char bufff[500];
  int mana_water = 0;
  int mana_earth = 0;
  int mana_fire = 0;
  int mana_air = 0;
  int mana_death =0;
  int water_lev = 0;
  int earth_lev = 0;
  int fire_lev = 0;
  int air_lev = 0;
  int death_lev = 0;
  int i;
  SINGLE_OBJECT *rbj;
  SINGLE_OBJECT *obj = NULL;
  int max_lvl;
  I_GEM *gem;

  DEFINE_COMMAND ("mana", mana, POSITION_DEAD, 0, LOG_NORMAL, "Allows you to see your mana in different element types, and max spell levels you can cast.")
    if (IS_MOB (ch))
    return;

  mana_fire = find_char_mana (ch, MANA_FIRE, 0);
  mana_air = find_char_mana (ch, MANA_AIR, 0);
  mana_water = find_char_mana (ch, MANA_WATER, 0);
  mana_earth = find_char_mana (ch, MANA_EARTH, 0);
  mana_death = find_char_mana (ch, MANA_DEATH, 0);
  max_lvl = 0;
  if ((rbj = get_eq_char (ch, WEAR_HOLD_1)) != NULL
      && rbj->pIndexData->item_type == ITEM_GEM)
    obj = rbj;
  else if ((rbj = get_eq_char (ch, WEAR_HOLD_2)) != NULL
	   && rbj->pIndexData->item_type == ITEM_GEM)
    obj = rbj;

  if (obj)
    {
      gem = (I_GEM *) obj->more;
      if (IS_SET (gem->gem_type, MANA_FIRE))
	{
	  fire_lev = gem->max_level;
	}
      if (IS_SET (gem->gem_type, MANA_AIR))
	{
	  air_lev = gem->max_level;
	}
      if (IS_SET (gem->gem_type, MANA_EARTH))
	{
	  earth_lev = gem->max_level;
	}
      if (IS_SET (gem->gem_type, MANA_WATER))
	{
	  water_lev = gem->max_level;
	}
      if (IS_SET (gem->gem_type, MANA_DEATH))
        {
          death_lev = gem->max_level;
        }
    }
#ifdef NEW_WORLD
  i = 99;
#else
  i = (get_curr_int (ch) + get_curr_wis (ch)) / 2;
#endif
  fire_lev = UMAX (fire_lev, i);
  water_lev = UMAX (water_lev, i);
  air_lev = UMAX (air_lev, i);
  earth_lev = UMAX (earth_lev, i);
  death_lev = UMAX (death_lev, i);

  sprintf (bufff, "\x1B[0;31mFire mana \x1B[0;37mavailable is   : [\x1B[0;31m%3d\x1B[0;37m] (Max spell level \x1B[1;34m%d\x1B[0;37m).\n\r",
	   mana_fire, fire_lev);
  send_to_char (bufff, ch);
  sprintf (bufff, "\x1B[0;36mAir mana \x1B[0;37mavailable is    : [\x1B[0;36m%3d\x1B[0;37m] (Max spell level \x1B[1;34m%d\x1B[0;37m).\n\r",
	   mana_air, air_lev);
  send_to_char (bufff, ch);
  sprintf (bufff, "\x1B[0;34mWater mana \x1B[0;37mavailable is  : [\x1B[0;34m%3d\x1B[0;37m] (Max spell level \x1B[1;34m%d\x1B[0;37m).\n\r",
	   mana_water, water_lev);
  send_to_char (bufff, ch);
  sprintf (bufff, "\x1B[0;33mEarth mana \x1B[0;37mavailable is  : [\x1B[0;33m%3d\x1B[0;37m] (Max spell level \x1B[1;34m%d\x1B[0;37m).\n\r",
	   mana_earth, earth_lev);
  send_to_char (bufff, ch);
  sprintf (bufff, "\x1B[1;30mDeath mana \x1B[0;37mavailable is  : [\x1B[1;30m%3d\x1B[0;37m] (Max spell level \x1B[1;34m%d\x1B[0;37m).\n\r",
           mana_death, death_lev);
  send_to_char (bufff, ch);
  sprintf (bufff, "\n\rGeneral mana available is the max of the above values and spell levels.\n\r");
  send_to_char (bufff, ch);
  return;
}



int
find_char_mana (CHAR_DATA * ch, int bit, int sn)
{
  SINGLE_OBJECT *obj;
  int zmana = 0;
  I_GEM *g;
  SPELL_DATA *spell;
  bool foundgem;
  foundgem = FALSE;
  spell = skill_lookup (NULL, sn);
  if (IS_MOB (ch))
    return 50000;
  if (LEVEL (ch) >= 110)
    return 7777;
  zmana = ch->pcdata->n_mana;
  if (((obj = get_eq_char (ch, WEAR_HOLD_1)) != NULL)
      && obj->pIndexData->item_type == ITEM_GEM
      && IS_SET (((I_GEM *) obj->more)->gem_type, bit))
    {
      g = (I_GEM *) obj->more;
      zmana += g->mana_now;
      foundgem = TRUE;
      if (spell && spell->spell_level > g->max_level)
	{
	  send_to_char ("Your gem is not powerful enough!\n\r", ch);
	  zmana = 0;
	  return zmana;
	}
    }
  else
    if (((obj = get_eq_char (ch, WEAR_HOLD_2)) != NULL)
	&& obj->pIndexData->item_type == ITEM_GEM
	&& IS_SET (((I_GEM *) obj->more)->gem_type, bit))
    {
      g = (I_GEM *) obj->more;
      zmana += g->mana_now;
      foundgem = TRUE;
      if (spell && spell->spell_level > g->max_level)
	{
	  send_to_char ("Your gem is not powerful enough!\n\r", ch);
	  zmana = 0;
	  return zmana;
	}
    }
#ifndef NEW_WORLD
  if (!foundgem && spell
      && spell->spell_level > ((get_curr_int (ch) + get_curr_wis (ch)) / 2))
    {
      send_to_char ("You can't cast such a powerful spell without the proper gem!\n\r", ch);
      return 0;
    }
#endif
  return zmana;
}

/*
   * Lookup a skill by slot number.
   * Used for object loading.
 */
int
slot_lookup (int slot)
{
  extern bool fBootDb;
  int sn;
  SPELL_DATA *spell;
  if (slot <= 0)
    return -1;
  return slot;
  for (sn = 0; sn < SKILL_COUNT; sn++)
    {
      if ((spell = skill_lookup (NULL, sn)) == NULL)
	continue;
      if (slot == spell->gsn /*spell->slot */ )
	return sn;
    }
  if (fBootDb)
    {
      bug ("Slot_lookup: bad slot %d.", slot);
      return 1;
      /*abort( ); */
    }
  return -1;
}

/* bah this crap is useless - taken from other code */

void
say_spell (CHAR_DATA * ch, int sn)
{
  char buf[STD_LENGTH];
  char buf2[STD_LENGTH];
  CHAR_DATA *rch;
  char *pName;
  int iSyl;
  int length;
  SPELL_DATA *spell;
  struct syl_type
  {
    char *old;
    char *new;
  }
   ;
  static const struct syl_type syl_table[] = {
    {
     " ", " "}
    ,
    {
     "ar", "ara"}
    ,
    {
     "au", "ada"}
    ,
    {
     "bless", "fio"}
    ,
    {
     "blind", "ose"}
    ,
    {
     "bur", "moa"}
    ,
    {
     "cu", "ju"}
    ,
    {
     "de", "olo"}
    ,
    {
     "en", "uso"}
    ,
    {
     "light", "is"}
    ,
    {
     "lo", "h"}
    ,
    {
     "mor", "ak"}
    ,
    {
     "move", "sio"}
    ,
    {
     "ness", "atog"}
    ,
    {
     "ning", "ila"}
    ,
    {
     "per", "uda"}
    ,
    {
     "ra", "gu"}
    ,
    {
     "re", "cus"}
    ,
    {
     "son", "bru"}
    ,
    {
     "tect", "ira"}
    ,
    {
     "fire", "antioch"}
    ,

    {
     "tri", "cu"}
    ,
    {
     "ven", "no"}
    ,
    {
     "a", "z"}
    ,
    {
     "b", "y"}
    ,
    {
     "c", "x"}
    ,
    {
     "d", "w"}
    ,
    {
     "e", "v"}
    ,
    {
     "f", "u"}
    ,
    {
     "g", "t"}
    ,
    {
     "h", "s"}
    ,
    {
     "i", "r"}
    ,
    {
     "j", "q"}
    ,
    {
     "k", "p"}
    ,
    {
     "l", "o"}
    ,
    {
     "m", "n"}
    ,
    {
     "n", "m"}
    ,
    {
     "o", "l"}
    ,
    {
     "p", "k"}
    ,
    {
     "q", "j"}
    ,
    {
     "r", "i"}
    ,
    {
     "s", "h"}
    ,
    {
     "t", "g"}
    ,
    {
     "u", "f"}
    ,
    {
     "v", "e"}
    ,
    {
     "w", "d"}
    ,
    {
     "x", "c"}
    ,
    {
     "y", "b"}
    ,
    {
     "z", "a"}
    ,
    {
     "", ""}
  }
   ;
  if ((spell = skill_lookup (NULL, sn)) == NULL)
    return;
  buf[0] = '\0';
  for (pName = spell->spell_funky_name; *pName != '\0'; pName += length)
    {
      for (iSyl = 0; (length = strlen (syl_table[iSyl].old)) != 0; iSyl++)
	{
	  if (!str_prefix (syl_table[iSyl].old, pName))
	    {
	      strcat (buf, syl_table[iSyl].new);
	      break;
	    }
	}
      if (length == 0)
	length = 1;
    }
  sprintf (buf2, "$n utters the words, '%s'", buf);
  sprintf (buf, "$n utters the words, '%s'", spell->spell_funky_name);
  for (rch = ch->in_room->more->people; rch; rch = rch->next_in_room)
  {
    if (rch != ch)
    {
      if (IS_MOB (rch))
        act (buf, ch, NULL, rch, TO_VICT);
      else if (rch->pcdata->learned[sn] > number_range(0, 99))
        act (buf, ch, NULL, rch, TO_VICT);
      else
        act (buf2, ch, NULL, rch, TO_VICT);
    }
  }
  return;
}

/*---------------------------------------------------*/
/*   * Compute a saving throw.                       */ 
/*   * Negative apply's make saving throw better.    */
/*---------------------------------------------------*/
bool
saves_spell (int level, CHAR_DATA * victim)
{
  int save;
  save =
    50 + (((LEVEL (victim) - level) - (victim->pcdata->saving_throw * 5)));
  save = URANGE (5, save, 95);
  return number_percent () < save;
}

int
mana_now (CHAR_DATA * ch)
{
  SINGLE_OBJECT *obj;
  int mm;
  int zmana = 0;
  if (IS_MOB (ch))
    return 50000;
  if (LEVEL (ch) >= 110)
    return 7777;
  mm = max_mana (ch);
  zmana = ch->pcdata->n_mana;
  if ((obj = get_eq_char (ch, WEAR_HOLD_1)) != NULL)
    {
      if (obj->pIndexData->item_type == ITEM_GEM)
	{
	  zmana += ((I_GEM *) obj->more)->mana_now;
	  if (zmana > mm)
	    return mm;
	  return zmana;
	}
    }
  if ((obj = get_eq_char (ch, WEAR_HOLD_2)) != NULL)
    {
      if (obj->pIndexData->item_type == ITEM_GEM)
	{
	  zmana += ((I_GEM *) obj->more)->mana_now;
	  if (zmana > mm)
	    return mm;
	  return zmana;
	}
    }
  if (zmana > mm)
    return mm;
  return zmana;
}

void
take_mana_char (CHAR_DATA * ch, int zmana, int bit)
{
  SINGLE_OBJECT *obj;
  if (IS_MOB (ch))
    return;
  if (LEVEL (ch) >= 110)
    return;
  MARK_CHAR_DEBUG ("Take_mana_char", ch)
    if ((obj = get_eq_char (ch, WEAR_HOLD_1)) != NULL)
    {
      if (obj->pIndexData->item_type == ITEM_GEM &&
	  IS_SET (((I_GEM *) obj->more)->gem_type, bit))
	{
	  ((I_GEM *) obj->more)->mana_now -= zmana;
	  return;
	}
    }
  if ((obj = get_eq_char (ch, WEAR_HOLD_2)) != NULL)
    {
      if (obj->pIndexData->item_type == ITEM_GEM &&
	  IS_SET (((I_GEM *) obj->more)->gem_type, bit))
	{
	  ((I_GEM *) obj->more)->mana_now -= zmana;
	  return;
	}
    }
  if (ch->pcdata->n_mana >= zmana)
    {
      ch->pcdata->n_mana -= zmana;
      return;
    }
  else
    ch->pcdata->n_mana = 0;
  return;
}

char *target_name;

void
do_cast (CHAR_DATA * ch, char *argy)
{
  SINGLE_OBJECT *obj;
  int zmana;
  CHAR_DATA *victim;
  int char_mana;
  SPELL_DATA *spell;
  char arg1[SML_LENGTH];
  char arg2[SML_LENGTH];
  DEFINE_COMMAND ("cast", do_cast, POSITION_FIGHTING, 0, LOG_NORMAL, "Allows you to cast a spell.  Format is: cast 'spellname' <target>.")
    if (IS_MOB (ch))
    return;

  if (IS_EVIL (ch) && !pow.evil_can_cast)
    {
      send_to_char ("You cannot cast spells!\n\r", ch);
      return;
    }
  if (ch->pcdata->tickcounts > 20 && (FIGHTING (ch) != NULL))
    {
      send_to_char ("You can't cast another spell this soon after your last one!\n\r", ch);
      return;
    }
  if (ch->position == POSITION_GROUNDFIGHTING)
    {
      send_to_char ("You can't concentrate enough! You are groundfighting!\n\r", ch);
      return;
    }
  if (ch->position == POSITION_BASHED)
    {
      send_to_char ("You can't cast while you are sitting on your butt!\n\r", ch);
      return;
    }
  if (ch->position == POSITION_CASTING)
    {
      send_to_char ("You are already casting a spell!\n\r", ch);
      return;
    }
  if (strlen (argy) > 40)
    return;
  target_name = one_argy (argy, arg1);
  one_argy (target_name, arg2);
  if (IS_MOB (ch) && ch->desc == NULL)
    return;
  if (IS_SET (ch->in_room->room_flags, ROOM_NOMAGIC))
    {
      send_to_char ("The ground absorbs your magical incantations, preventing you from casting!\n\r", ch);
      return;
    }
  if ((spell = skill_lookup (arg1, -1)) == NULL || (IS_PLAYER (ch) && LEVEL (ch) < spell->spell_level))
    {
      send_to_char ("You can't do that.\n\r", ch);
      return;
    }
  if (spell->slot == 0)
    {
      send_to_char ("That is not a spell!\n\r", ch);
      return;
    }
  if (ch->position < spell->position)
    {
      send_to_char ("You can't concentrate enough.\n\r", ch);
      return;
    }
   /*if ( IS_PLAYER(ch) && get_curr_wis(ch) < spell->min_wis) 
   {
   send_to_char("You aren't wise enough to use this spell.\n\r",ch);
   return;
   }
   if ( IS_PLAYER(ch) && get_curr_int(ch) < spell->min_int) 
   {
   send_to_char("You aren't smart enough to understand how to cast this spell.\n\r",ch);
   return;
   }*/
 
  switch (spell->spell_type)
    {
    default:
      bug ("Do_cast: bad target for sn %d.", spell->gsn);
      return;
    case TAR_IGNORE:
      break;
    case TAR_CHAR_OFFENSIVE:
      if (arg2[0] == '\0')
	{
	  if ((victim = FIGHTING (ch)) == NULL)
	    {
	      send_to_char ("Cast the spell on whom?\n\r", ch);
	      return;
	    }
	  }
      if ((victim = get_char_room (ch, arg2)) == MASTER (ch))
	{
	  send_to_char ("Why would you be nasty and cast a spell on someone you are following?!\n\r", ch);
	  return;
	}


      else if ((victim = get_char_room (ch, arg2)) == NULL)
	{
	  send_to_char ("They aren't here.\n\r", ch);
	  return;
	}

      if (ch == victim && LEVEL (ch) < 10)
	{
	  send_to_char ("You can't do that to yourself.\n\r", ch);
	  return;
	}
      break;
    case TAR_CHAR_DEFENSIVE:
      if (arg2[0] == '\0')
	{
	  victim = ch;
	}
      else
	{
	  if ((victim = get_char_room (ch, arg2)) == NULL)
	    {
	      send_to_char ("They aren't here.\n\r", ch);
	      return;
	    }
	}
      break;
    case TAR_CHAR_SELF:
      {
	if (arg2[0] != '\0' && !is_name (arg2, RNAME (ch)))
	  {
	    send_to_char ("You can't cast this spell on another.\n\r", ch);
	    return;
	  }
	if (arg2[0] != '\0' && is_name (arg2, "1."))
	  {
	    send_to_char ("You attempted to exploit a crash bug this has been logged\n\r", ch);
	    return;
	  }
	break;
      }
    case TAR_OBJ_INV:
      if (arg2[0] == '\0')
	{
	  send_to_char ("What should the spell be cast upon?\n\r", ch);
	  return;
	}
      if ((obj = get_obj_inv (ch, arg2)) == NULL)
	{
	  send_to_char ("You are not carrying that.\n\r", ch);
	  return;
	}
      break;
    case TAR_OBJ_ROOM:
                if (arg2[0] == '\0')
                {
                        send_to_char ("What should the spell be cast upon?\n\r", ch);
                        return;
                }

         obj = get_obj_list (ch, arg2, ch->in_room->more->contents);
            if (obj == NULL || IS_SET (obj->extra_flags, ITEM_UNSEEN))
            {
              send_to_char("Nothing like that on the ground.\n\r", ch);
              return;
            }
      break;
    }
  char_mana = find_char_mana (ch, spell->mana_type, spell->gsn);
  if (2 + LEVEL (ch) - spell->spell_level == 0)
    zmana = spell->mana;
  else
    {
      zmana = IS_MOB (ch) ? 0 : UMAX (spell->mana, 100 / (2 + LEVEL (ch) - spell->spell_level));
    }
  if (IS_PLAYER (ch) && zmana > char_mana)
    {
      send_to_char ("You don't have enough magical energy of the right element to cast that spell.\n\r", ch);
      return;
    }
  if (strlen (argy) > 140)
    return;
  NEW_POSITION (ch, POSITION_CASTING);
  act ("\x1B[0;36mYou begin to utter some strange incantations...\x1B[0;37m", ch, NULL, ch, TO_CHAR);
  act ("\x1B[0;36m$N begins to utter some strange incantations...\x1B[0;37m", ch, NULL, ch, TO_ROOM);
  check_temp (ch);
  strcpy (ch->pcdata->temp->temp_string, argy);
  if (IS_PLAYER (ch))
    if ((LEVEL (ch) < 100) || (LEVEL (ch) > 99 && IS_SET (ch->pcdata->act2, PLR_HOLYLAG)))
      {
	check_fgt (ch);
       if ((is_member (ch, GUILD_WIZARD)) || (is_member (ch, GUILD_HEALER)))
	ch->pcdata->tickcounts = spell->casting_time - (ch->pcdata->remort_times/2) - (LEVEL (ch) / 20);
       else
        ch->pcdata->tickcounts = spell->casting_time - (ch->pcdata->remort_times/2);

        if (is_member (ch, GUILD_ELEMENTAL))
          {
          ch->pcdata->tickcounts += 5;
          }
	if (ARCHMAGE (ch))
	  {
	    ch->pcdata->tickcounts /= 2;
	  }
      }
  return;
}

void
do_actual_cast (CHAR_DATA * ch, char *argy)
{
  char buf[500];
  char arg1[SML_LENGTH];
  SINGLE_OBJECT *o;
  char arg2[SML_LENGTH];
  CHAR_DATA *victim;
  SINGLE_OBJECT *obj;
  void *vo;
  int zmana;
  int char_mana;
  int sn;
  SPELL_DATA *spell;
/*
   * Switched NPC's can cast spells, but others can't.
 */
  MARK_CHAR_DEBUG ("Actual_cast", ch) if (IS_MOB (ch) && ch->desc == NULL)
    return;
  if (ch->position == POSITION_GROUNDFIGHTING)
    return;
  if (FIGHTING (ch) == NULL && ch->position == POSITION_CASTING)
    NEW_POSITION (ch, POSITION_STANDING);
  if (FIGHTING (ch) != NULL && ch->position == POSITION_CASTING)
    NEW_POSITION (ch, POSITION_FIGHTING);
  target_name = one_argy (argy, arg1);
  one_argy (target_name, arg2);
  if (IS_SET (ch->in_room->room_flags, ROOM_NOMAGIC))
    {
      send_to_char ("\x1B[0;37mThe ground absorbs your magical incantations, preventing you from casting!\n\r", ch);
      return;
    }
  if (arg1[0] == '\0')
    {
      send_to_char ("Cast which what where?\n\r", ch);
      return;
    }
  if ((spell = skill_lookup (arg1, -1)) == NULL
      || (IS_PLAYER (ch) && LEVEL (ch) < spell->spell_level))
    {
      send_to_char ("You can't do that.\n\r", ch);
      return;
    }
  sn = spell->gsn;
  if (ch->position == POSITION_CASTING)
    NEW_POSITION (ch, POSITION_STANDING);
  if (ch->position < spell->position)
    {
      send_to_char ("\x1B[1;31mYou can't concentrate enough\x1B[0;37m.\n\r", ch);
      return;
    }
  if (2 + LEVEL (ch) - spell->spell_level == 0)
    zmana = spell->mana;
  else
    {
      zmana = IS_MOB (ch) ? 0 : UMAX (spell->mana,
				      100 / (2 + LEVEL (ch) -
					     spell->spell_level));
    }
/*
   * Locate targets.
 */
  victim = NULL;
  obj = NULL;
  vo = NULL;
  switch (spell->spell_type)
    {
    default:
      bug ("Do_cast: bad target for sn %d.", sn);
      return;
    case TAR_IGNORE:
      break;
    case TAR_CHAR_OFFENSIVE:
      if (arg2[0] == '\0')
	{
	  if ((victim = FIGHTING (ch)) == NULL)
	    {
	      send_to_char ("Cast the spell on whom?\n\r", ch);
	      return;
	    }

	}
      else if ((victim = get_char_room (ch, arg2)) == NULL)
	{
	  send_to_char ("They aren't here.\n\r", ch);
	  return;
	}

      if (ch == victim && LEVEL (ch) < 10)
	{
	  send_to_char ("You can't do that to yourself.\n\r", ch);
	  return;
	}
      if (ch->position != POSITION_FIGHTING || FIGHTING (ch) != victim)
	if (!pkill (ch, victim))
	  return;
      if (IS_PLAYER (ch))
	ch->pcdata->tickcounts = spell->casting_time + 12/2;
      /* if ( IS_PLAYER(ch) )
         {
         * Removed no-char fuzzyness *
         if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
         {
         send_to_char( "You can't do that on your own follower.\n\r", ch );
         return;
         }
         }
       */
      vo = (void *) victim;
      break;
    case TAR_CHAR_DEFENSIVE:
      if (arg2[0] == '\0')
	{
	  victim = ch;
	}
      else
	{
	  if ((victim = get_char_room (ch, arg2)) == NULL)
	    {
	      send_to_char ("They aren't here.\n\r", ch);
	      return;
	    }
	}
      vo = (void *) victim;
      break;
    case TAR_CHAR_SELF:
      if (arg2[0] != '\0' && !is_name (arg2, RNAME (ch)))
	{
	  send_to_char ("You can't cast this spell on another.\n\r", ch);
	  return;
	}
      vo = (void *) ch;
      break;
    case TAR_OBJ_INV:
      if (arg2[0] == '\0')
	{
	  send_to_char ("What should the spell be cast upon?\n\r", ch);
	  return;
	}
      if ((obj = get_obj_inv (ch, arg2)) == NULL)
	{
	  send_to_char ("You are not carrying that in your hand.\n\r", ch);
	  return;
	}
      vo = (void *) obj;
      break;
        
      case TAR_OBJ_ROOM:
                if (arg2[0] == '\0')
                {
                        send_to_char ("What should the spell be cast upon?\n\r", ch);
                        return;
                }

       obj = get_obj_list (ch, arg2, ch->in_room->more->contents);
           if (obj == NULL || IS_SET (obj->extra_flags, ITEM_UNSEEN))
            {
              send_to_char("Nothing like that on the ground...\n\r", ch);
              return;
            }
          vo = (void *) obj;
      break;
        }

  MARK_CHAR_DEBUG ("Actual_cast_find_char_mana", ch)
    char_mana = find_char_mana (ch, spell->mana_type, sn);

  MARK_CHAR_DEBUG ("Actual_cast_after_find_char_mana", ch)
    if (victim != NULL && spell->spell_type == TAR_CHAR_OFFENSIVE &&
	is_safe (ch, victim)
	&&
	((IS_PLAYER (ch) && !IS_MOB (victim)
	  && !not_is_same_align (ch, victim)) || IS_MOB (victim)))
    {
      send_to_char ("You are in a safe zone. No casting nasty spells like that!\n\r", ch);
      return;
    }
  if (IS_PLAYER (ch) && zmana > char_mana)
    {
      switch (spell->mana_type)
	{
	default:
	  send_to_char ("You don't have enough mana to cast that spell.\n\r", ch);
	  break;
	case MANA_FIRE:
	  send_to_char ("You don't have enough mana to contain the Fire.\n\r", ch);
	  break;
	case MANA_AIR:
	  send_to_char ("You require more mana for Air.\n\r", ch);
	  break;
	case MANA_WATER:
	  send_to_char ("You don't have enough mana for Water.\n\r", ch);
	  break;
	case MANA_EARTH:
	  send_to_char ("Your Earth mana is insufficient.\n\r", ch);
	  break;
        case MANA_DEATH:
          send_to_char ("Your Death mana is insufficient.\n\r", ch);
          break;
	}
      return;
    }
  if (IS_PLAYER (ch) && number_percent () > ch->pcdata->learned[spell->gsn]
      + (is_member (ch, GUILD_WIZARD) ? 15 : 0)
      + (is_member (ch, GUILD_HEALER) ? 10 : 0))
    {
      send_to_char ("You lost your concentration.\n\r", ch);
      take_mana_char (ch, zmana / 2, spell->mana_type);
      upd_mana (ch);
    }
  else
    {
      say_spell (ch, sn);
      /*(*skill_table[sn].spell_fun) ( sn, LEVEL(ch), ch, vo ); */
      general_spell (spell, LEVEL (ch), ch, vo);
      take_mana_char (ch, zmana, spell->mana_type);
      if (IS_JAVA (ch))
	java_mana (ch);
      upd_mana (ch);
      if (victim && victim->data_type == 50)
	{
	  return;
	}
    }

  MARK_CHAR_DEBUG ("Actual_cast_label_3", ch)
    if (spell->spell_type == TAR_CHAR_OFFENSIVE
	&& victim != ch && MASTER (victim) != ch)
    {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;
      for (vch = ch->in_room->more->people; vch; vch = vch_next)
	{
	  vch_next = vch->next_in_room;
	  if (victim == vch && FIGHTING (victim) == NULL)
	    {
	      multi_hit (victim, ch, TYPE_UNDEFINED);
	      break;
	    }
	}
    }

/* Bad things (possibly) */
#ifdef NEW_WORLD
  if (IS_PLAYER (ch))
    {
      char bbb[500];
      for (o = ch->carrying; o != NULL; o = o->next_content)
	{
/*    if (o->wear_loc==WEAR_NONE) continue; */
	  if (o->pIndexData->values_2[6] > 0 &&
	      number_range (1, 1000) < o->pIndexData->values_2[7])
	    {
	      if (o->pIndexData->values_2[6] > 0)
		{
		  SPELL_DATA *s;
		  if ((s = skill_lookup (NULL, o->pIndexData->values_2[6])) != NULL)
		    {
		      sprintf (bbb, "Suddenly %s flares with energy!\n\r", o->pIndexData->short_descr);
		      send_to_char (bbb, ch);
		      general_spell (s, LEVEL (ch), ch, ch);
		      return;
		    }
		}
	    }
	  if (o->pIndexData->values_2[5] > 0 &&
	      number_range (1, 1000) < o->pIndexData->values_2[7])
	    {
	      if (IS_SET (o->pIndexData->values_2[5], SUCK_ALL_MANA))
		{
		  SINGLE_OBJECT *gem;
		  sprintf (bbb, "Suddenly %s flares with energy!\n\r", o->pIndexData->short_descr);
		  send_to_char (bbb, ch);
		  send_to_char ("A dark force engulfs you, and you feel drained!\n\r", ch);
		  ch->pcdata->n_mana = 0;
		  for (gem = ch->carrying; gem != NULL;
		       gem = gem->next_content)
		    {
		      if ((gem->wear_loc == WEAR_HOLD_1
			   || gem->wear_loc == WEAR_HOLD_2)
			  && gem->pIndexData->item_type == ITEM_GEM)
			{
			  ((I_GEM *) o->more)->mana_now = 0;
			}
		    }
		}
	      if (IS_SET (o->pIndexData->values_2[5], SUCK_HALF_MANA))
		{
		  SINGLE_OBJECT *gem;
		  sprintf (bbb, "Suddenly %s flares with energy!\n\r", o->pIndexData->short_descr);
		  send_to_char (bbb, ch);
		  send_to_char ("A dark force engulfs you, and you feel drained!\n\r", ch);
		  ch->pcdata->n_mana /= 2;
		  for (gem = ch->carrying; gem != NULL;
		       gem = gem->next_content)
		    {
		      if ((gem->wear_loc == WEAR_HOLD_1
			   || gem->wear_loc == WEAR_HOLD_2)
			  && gem->pIndexData->item_type == ITEM_GEM)
			{
			  ((I_GEM *) o->more)->mana_now /= 2;
			}
		    }
		}
	      if (IS_SET (o->pIndexData->values_2[5], REMOVE_ALL_SPELLS))
		{
		  AFFECT_DATA *a;
		  AFFECT_DATA *a_next;
		  sprintf (bbb, "Suddenly %s flares with energy!\n\r", o->pIndexData->short_descr);
		  send_to_char (bbb, ch);
		  send_to_char ("A cold, magical wind chills you to the bone...\n\r", ch);
		  strip_all_affects (ch);
		}
	      if (IS_SET (o->pIndexData->values_2[5], SUCK_HALF_HITS))
		{
		  sprintf (bbb, "Suddenly %s flares with energy!\n\r",
			   o->pIndexData->short_descr);
		  send_to_char (bbb, ch);
		  send_to_char
		    ("A sharp pain causes you to involuntarily smack yourself on the head!\n\r",
		     ch);
		  ch->hit /= 2;
		}
	      if (IS_SET (o->pIndexData->values_2[5], SUCK_HALF_MOVES))
		{
		  sprintf (bbb, "Suddenly %s flares with energy!\n\r", o->pIndexData->short_descr);
		  send_to_char (bbb, ch);
		  send_to_char ("Your legs suddenly feel heavy and fatigued.\n\r", ch);
		  ch->move /= 2;
		}
	      if (IS_SET (o->pIndexData->values_2[5], DESTROY_RANDOM_OBJECT))
		{
		  SINGLE_OBJECT *oo;
		  sprintf (bbb, "Suddenly %s flares with energy!\n\r", o->pIndexData->short_descr);
		  send_to_char (bbb, ch);
		  for (oo = ch->carrying; oo != NULL; oo = oo->next_content)
		    {
		      if (number_range (1, 10) == 4)
			break;
		    }
		  if (!oo)
		    oo = ch->carrying;
		  if (IS_SET (oo->extra_flags, ITEM_SAVE))
		    break;
		  if (oo == o)
		    break;
		  sprintf (buf, "%s suddenly vanishes in a puff of smelly, sulfer smoke!\n\r", capitalize (oo->pIndexData->short_descr));
		  send_to_char (buf, ch);
		  if (oo->wear_loc != WEAR_NONE)
		    {
		      unequip_char (ch, oo);
		    }
		  obj_from (oo);
		  free_it (oo);
		  return;
		}
	      if (IS_SET (o->pIndexData->values_2[5], DESTROY_OBJECT))
		{
		  sprintf (bbb, "Suddenly %s flares with energy!\n\r", o->pIndexData->short_descr);
		  send_to_char (bbb, ch);
		  sprintf (buf, "%s suddenly vanishes in a puff of smelly, sulfer smoke!\n\r", capitalize (o->pIndexData->short_descr));
		  send_to_char (buf, ch);
		  if (o->wear_loc != WEAR_NONE)
		    {
		      unequip_char (ch, o);
		    }
		  obj_from (o);
		  free_it (o);
		  return;
		}
	    }
	}
    }
#endif
  return;
}

/*
   * Cast spells at targets using a magical object.
 */
void
obj_cast_spell (int sn, int level, CHAR_DATA * ch, CHAR_DATA * victim,
		SINGLE_OBJECT * obj)
{
  void *vo;
  SPELL_DATA *spell;
  if (IS_SET (ch->in_room->room_flags, ROOM_NOMAGIC))
    {
      send_to_char ("The ground seems to absorb the spell...!\n\r", ch);
      return;
    }
  if (sn <= 0 || (spell = skill_lookup (NULL, sn)) == NULL)
    return;
  switch (spell->spell_type)
    {
    default:
      bug ("Obj_cast_spell: bad target for sn %d.", sn);
      return;
    case TAR_IGNORE:
      vo = NULL;
      break;
    case TAR_CHAR_OFFENSIVE:
      if (victim == NULL)
	victim = FIGHTING (ch);
      if (victim == NULL)
	{
	  send_to_char ("You can't do that.\n\r", ch);
	  return;
	}
      if (IS_PLAYER (ch))
	{
	  if (ch->pcdata->tickcounts > 2)
	    {
	      send_to_char ("The wand refuses to respond!\n\r", ch);
	      return;
	    }
	}
      if (ch->position != POSITION_FIGHTING || FIGHTING (ch) != victim)
	if (!pkill (ch, victim))
	  return;
      vo = (void *) victim;
      break;
    case TAR_CHAR_DEFENSIVE:
      if (victim == NULL)
	victim = ch;
      vo = (void *) victim;
      break;
    case TAR_CHAR_SELF:
      vo = (void *) ch;
      break;
    case TAR_OBJ_INV:
      if (obj == NULL)
	{
	  send_to_char ("You can't do that.\n\r", ch);
	  return;
	}
      vo = (void *) obj;
      break;
    }
  target_name = "";
  general_spell (spell, level, ch, vo);
/* (*skill_table[sn].spell_fun) ( sn, level, ch, vo ); */
  if (IS_PLAYER (ch))
    ch->pcdata->tickcounts = spell->casting_time + 12/2;
  if (spell->spell_type == TAR_CHAR_OFFENSIVE
      && victim != ch && MASTER (victim) != ch)
    {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;
      for (vch = ch->in_room->more->people; vch; vch = vch_next)
	{
	  vch_next = vch->next_in_room;
	  if (victim == vch && FIGHTING (victim) == NULL)
	    {
	      check_fgt (victim);
	      multi_hit (victim, ch, TYPE_UNDEFINED);
	      victim->fgt->fighting = ch;
	      NEW_POSITION (victim, POSITION_FIGHTING);
	      break;
	    }
	}
    }
  return;
}
//END OF FILE
