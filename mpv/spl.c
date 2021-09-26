#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"

void
show_hitdam (int gsn, char *noun, int dam, CHAR_DATA * ch, CHAR_DATA * victim)
{
  if (gsn == gsn_backstab || gsn == gsn_dual_backstab || gsn == gsn_circle)
    {
      int ty;
      char rvp[1024];
      char rss[1024];
      char *tt;
      ty = 0;
      if (victim->hit < 0)
	return;
      sprintf (rss, "$B$5Your %s *a $N.$R$7", noun);
      for (tt = rss; *tt != '\0'; tt++)
	{
	  if (*tt == '*')
	    {
	      tt++;
	      if (*tt == 'a')
		{
		  strcat (rvp, (star_a (dam, victim->max_hit)));
		  ty = strlen (rvp);
		}
	    }
	  else
	    {
	      rvp[ty] = *tt;
	      rvp[ty + 1] = '\0';
	      ty++;
	    }
	}

      act (rvp, ch, NULL, victim, TO_CHAR);
    }
  else
    dam_message (ch, victim, dam, gsn, 0);
  return;
}

void
spell_summon (int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *vict;

  if (!target_name || target_name[0] == '\0')
    return;
  if ((vict = get_char_world (ch, target_name)) == NULL)
    {
      send_to_char ("The spell failed.\n\r", ch);
      return;
    }
  if (FIGHTING (vict) || LEVEL (vict) > LEVEL (ch) + 5
      || !vict->in_room->area->open || vict->in_room->vnum < 1000
      || vict == ch)
    {
      send_to_char ("The spell failed.\n\r", ch);
      return;
    }
  if (IS_MOB (vict) && !pow.can_summon_mobs)
    {
      send_to_char ("The spell failed.\n\r", ch);
      return;
    }
  if ((vict->in_room && IS_SET (vict->in_room->room_flags, ROOM_NOSUMMON)) ||
      (ch->in_room && IS_SET (ch->in_room->room_flags, ROOM_NOSUMMON)))
    {
      send_to_char ("The spell failed.\n\r", ch);
      return;
    }
  if (IS_PLAYER (vict) && IS_PLAYER (ch) &&
      ((IS_EVIL (vict) && !IS_EVIL (ch))
       || (IS_EVIL (ch) && !IS_EVIL (vict))))
    {
      send_to_char ("The spell failed.\n\r", ch);
      return;
    }
  if ((LEVEL (vict) == 95) && (LEVEL (ch) < 95))
    {
      send_to_char ("The spell failed.\n\r", ch);
      return;
    }
  act ("$n disappears in a flash of bright light!", vict, NULL, vict, TO_ROOM);
  char_from_room (vict);
  char_to_room (vict, ch->in_room);
  act ("$n appears in a blinding flash of light!", vict, NULL, vict, TO_ROOM);
  send_to_char ("You have been summoned!!\n\r", vict);
  do_look (vict, "auto");
  return;
}

void
spell_teleport (int sn, int level, CHAR_DATA * ch, void *vo)
{
  ROOM_DATA *rd;
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if (!victim)
    victim = ch;
  if (FIGHTING (ch))
    {
      send_to_char ("The spell failed!\n\r", ch);
      return;
    }
  while ((rd = get_room_index (number_range (1001, 99999))) == NULL
	 || !rd->area->open || rd->sector_type == SECT_WATER_NOSWIM
	 || rd->sector_type == SECT_WATER_SWIM
	 || IS_SET (rd->room_flags, ROOM_NOSUMMON));

  act ("$n disappears!", victim, NULL, victim, TO_ROOM);
  char_from_room (victim);
  char_to_room (victim, rd);
  return;
}



void
spell_word_of_recall (int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  if (IS_MOB (ch))
    return;
  if (!victim)
    victim = ch;
  if (IS_MOB (victim))
    return;
  if (ch->in_room && IS_SET (ch->in_room->room_flags, ROOM_NO_RECALL))
    {
      send_to_char ("You can not recall from this room!\n\r", ch);
      return;
    }
  if (FIGHTING (ch))
    {
      send_to_char ("The spell failed!\n\r", ch);
      return;
    }

  if ((IS_EVIL (ch) && !IS_EVIL (victim)) || (!IS_EVIL (ch) && IS_EVIL (victim)))
    return;

  if (IS_AFFECTED (victim, AFF_CURSE))
    {
      if (victim == ch)
	{
	  send_to_char ("A powerful curse is preventing you from recalling!\n\r", ch);
	}
      else
	{
	  send_to_char ("A powerful curse prevents your spell from succeeding!\n\r", ch);
	}
      return;
    }

  if (IS_EVIL (victim))
    {
      act ("$n disappears!", victim, NULL, victim, TO_ROOM);
      char_from_room (victim);
      char_to_room (victim, get_room_index (99));
      return;
    }
  if (!IS_EVIL (victim))
    {
      act ("$n disappears!", victim, NULL, victim, TO_ROOM);
      char_from_room (victim);
      char_to_room (victim, get_room_index (100));
      return;
    }

  return;
}



void
scatter_mob (CHAR_DATA * mob)
{
  int just_moved_dir = -1;
  int moved_rooms = 0;
  int move_tries = 0;
  int move_dir;

  if (!mob->in_room)
    return;

  while (move_tries < 20 && moved_rooms < 3)
    {
      move_dir = number_range (0, 5);
      while (move_dir == rev_dir[just_moved_dir])
	move_dir = number_range (0, 5);
      if (move_char (mob, move_dir))
	{
	  move_tries++;
	  moved_rooms++;
	  just_moved_dir = move_dir;
	  continue;
	}
      else
	move_tries++;
    }

  return;
}

void
do_spray (CHAR_DATA * ch, char *argy)
{
  char buf[500];
  char arg1[500];
  SINGLE_OBJECT *spray;
  CHAR_DATA *victim;
  DEFINE_COMMAND ("spray", do_spray, POSITION_FIGHTING, 0, LOG_NORMAL, "Allows you to spray a bottle of repellant at a mob.") 
  argy = one_argy (argy, arg1);
  if (!str_prefix ("with", argy))
    argy = one_argy (argy, buf);

  if (argy == "" || argy[0] == '\0')
    {
      if ((spray = get_item_held (ch, ITEM_REPELLANT)) == NULL)
	{
	  send_to_char ("Spray who with what?\n\r", ch);
	  return;
	}
    }
  else if ((spray = get_obj_carry (ch, argy)) == NULL)
    {
      if ((spray = get_obj_wear (ch, argy)) == NULL)
	{
	  send_to_char ("You don't seem to be carrying that.\n\r", ch);
	  return;
	}
    }

  if (spray->pIndexData->item_type != ITEM_REPELLANT)
    {
      send_to_char ("You can't spray that!\n\r", ch);
      return;
    }

  if ((victim = get_char_room (ch, arg1)) == NULL)
    {
      send_to_char ("I don't see a creature or person like that to spray it on.\n\r", ch);
      return;
    }

/* Spraying it.. remove a 'squirt' */

  ((I_SPRAY *) spray->more)->sprays--;

  if (IS_PLAYER (victim))
    {
      act ("You spray $N, but $E doesn't seem to be affected.", ch, NULL, victim, TO_CHAR);
      act ("$n sprays some liquid in your face, but nothing happens.", ch, NULL, victim, TO_VICT);
      act ("$n sprays some liquid in $N's face, but nothing seems to happen.", ch, NULL, victim, TO_NOTVICT);
    }

  if (IS_MOB (victim))
    {
      int i;
      bool yes_no = FALSE;
      for (i = 0; i < 9; i++)
	if (((I_SPRAY *) spray->more)->repels[i] == victim->pIndexData->vnum)
	  {
	    yes_no = TRUE;
	    break;
	  }
      if (!yes_no)
	{
	  act ("You spray $N, but $E doesn't seem to be affected.", ch, NULL, victim, TO_CHAR);
	  act ("$n sprays some liquid at you, but nothing happens.", ch, NULL, victim, TO_VICT);
	  act ("$n sprays some liquid at $N, but nothing seems to happen.", ch, NULL, victim, TO_NOTVICT);
	}
      else
	{
	  act ("You spray $N.", ch, NULL, victim, TO_CHAR);
	  act ("$n sprays some liquid at you.", ch, NULL, victim, TO_VICT);
	  act ("$n sprays some liquid at $N.", ch, NULL, victim, TO_NOTVICT);
	  if (FIGHTING (victim))
	    {
	      if (HUNTING (victim) && !str_cmp (HUNTING (victim), NAME (ch)))
		{
		  free_string (HUNTING (victim));
		  victim->fgt->hunting = NULL;
		}
	      stop_fighting (FIGHTING (victim), TRUE);
	      stop_fighting (victim, TRUE);
	    }
	  scatter_mob (victim);
	}
    }

  if (((I_SPRAY *) spray->more)->sprays < 1)
    {
      send_to_char ("The spray is empty, and magically disappears!\n\r", ch);
      free_it (spray);
    }

  return;
}


/* Dispell Magic just removes everything from the victim */
void
spell_dispel_magic (int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int affected_by;
  if (victim->affected_by == 0
      || level + 30 < LEVEL (victim)
      || (saves_spell (level, victim) && saves_spell (level, victim)))
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }
 /* for (;;)
    {
      affected_by = 1 << number_bits (5);
      if (IS_SET (victim->affected_by, affected_by))
	break;
    }
  REMOVE_BIT (victim->affected_by, affected_by);*/
  strip_all_affects (victim);
  send_to_char ("You successfully removed all spells from the victim!\n\r", ch);
  act ("$n just removed all spell effects from you!", ch, NULL, victim, TO_VICT);
  return;
}

void
spell_create_object (SPELL_DATA * spell, int level, CHAR_DATA * ch, void *vo)
{
  SINGLE_OBJECT *obj;
  if ((obj = create_object (get_obj_index (spell->creates_obj), 0)) == NULL)
    {
      send_to_char ("A god has set up this spell wrong.. please contact a god.\n\r", ch);
      return;
    }
  if (IS_SET (spell->spell_bits, SPELL_OBJ_TO_ROOM))
    obj_to (obj, ch->in_room);
  else
    {
      obj_to (obj, ch);
      obj->wear_loc = -1;
    }
  return;
}

bool
remove_sa (SPELL_DATA * spell, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  if (!is_affectedt (victim, spell->bitvector))
    {
      send_to_char ("Your spell fails!\n\r", ch);
      return FALSE;
    }
  affect_strip_bits (victim, spell->bitvector);
  return TRUE;
}


void
write_stuff (SPELL_DATA * spell, CHAR_DATA * ch, CHAR_DATA * victim)
{
  if (spell->act_to_ch_1)
    act (spell->act_to_ch_1, ch, NULL, victim, TO_CHAR);
  if (spell->act_to_ch_2)
    act (spell->act_to_ch_2, ch, NULL, victim, TO_CHAR);

  if (victim && spell->act_to_vict_1)
    act (spell->act_to_vict_1, ch, NULL, victim, TO_VICT);
  if (victim && spell->act_to_vict_2)
    act (spell->act_to_vict_2, ch, NULL, victim, TO_VICT);

  if (victim && ch == victim && spell->act_to_vict_1)
    act (spell->act_to_vict_1, ch, NULL, victim, TO_CHAR);
  if (victim && ch == victim && spell->act_to_vict_2)
    act (spell->act_to_vict_2, ch, NULL, victim, TO_CHAR);

  if (spell->act_to_notvict_1)
    act (spell->act_to_notvict_1, ch, NULL, victim, TO_NOTVICT);
  if (spell->act_to_notvict_2)
    act (spell->act_to_notvict_2, ch, NULL, victim, TO_NOTVICT);
  return;
}

void
general_spell (SPELL_DATA * spell, int level, CHAR_DATA * ch, void *vo)
{
  AFFECT_DATA paf;
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  CHAR_DATA *next = NULL;
  bool wrote = FALSE;
  bool once = FALSE;
  int dur = 0;
  bool sskip = FALSE;
  bool cast_at_n = FALSE;
  bzero (&paf, sizeof (paf));
  if (IS_SET (spell->spell_bits, SPELL_NOT_SELF) && ch == victim
      && !IS_SET (spell->spell_bits, SPELL_ALL_IN_ROOM))
    {
      send_to_char ("You find that you cannot cast this spell upon yourself...\n\r", ch);
      return;
    }
  if (IS_SET (spell->spell_bits, SPELL_FAIL_1_3))
    {
      if (number_range (1, 3) == 1)
	{
	  send_to_char ("Kilith does not want to save you at this time.\n\r", ch);
	  return;
	}
      if (number_range (1, 3) == 3)
        {
          send_to_char ("Sabelis feels like a bitch and refuses your request.\n\r", ch);
          return;
        }
    }
  if (IS_SET (spell->spell_bits, SPELL_FAIL_1_4))
    {
      if (number_range (1, 4) == 2)
	{
	  send_to_char ("Sabelis feels like a bitch and refuses your request.\n\r", ch);
	  return;
	}
      if (number_range (1, 4) == 3)
        {
          send_to_char ("Torak? I don't know a Torak.. and if I was here why would I want to help you? oops..\n\r", ch);
          return;
        }
      if (number_range (1, 4) == 1)
        {
          send_to_char ("Kilith is to lazy at this time to want to help you.\n\r", ch);
          return;
        }
    }
  if (IS_SET (spell->spell_bits, SPELL_FAIL_1_2))
    {
      if (number_range (1, 2) == 2)
	{
	  send_to_char ("The gods have stepped out at this time... Please leave a message after the beep..........\n\r", ch);
	  return;
	}
    }
  if (				/*spell->spell_type != TAR_CHAR_OFFENSIVE && */
       IS_SET (spell->spell_bits, SPELL_HURT_GOOD) && IS_EVIL (victim))
    {
      send_to_char ("The spell failed.\n\r", ch);
      return;
    }
  if (				/*spell->spell_type != TAR_CHAR_OFFENSIVE && */
       IS_SET (spell->spell_bits, SPELL_HURT_EVIL) && !IS_EVIL (victim))
    {
      send_to_char ("The spell failed.\n\r", ch);
      return;
    }
  if (spell->creates_obj != 0)
    {
      spell_create_object (spell, level, ch, vo);
    }


  if (spell->spell_type == TAR_CHAR_OFFENSIVE && IS_PLAYER (ch)
      && IS_PLAYER (victim) && (IS_EVIL (ch) == IS_EVIL (victim))
      && HP_NOW (ch) < 100)
    {
      send_to_char ("Maybe that's not the right thing to do.\n\r", ch);
      return;
    }

  if (IS_SET (spell->spell_bits, SPELL_MESSAGE))
    {
      spell_message (spell->gsn, level, ch, vo);
      return;
    }
  if (!IS_SET (spell->spell_bits, SPELL_ONLY_MULTI_MOBS) || IS_MOB (victim))
    {
      if (ch && victim && IS_PLAYER (ch) && !IS_MOB (victim)
	  && IS_SET (spell->spell_bits, SPELL_ALL_IN_ROOM)
	  && spell->spell_type == TAR_CHAR_OFFENSIVE
	  && ((IS_EVIL (ch) && IS_EVIL (victim))
	      || (!IS_EVIL (ch) && !IS_EVIL (victim))))
	{
	  sskip = TRUE;
	}
      if (!sskip && IS_SET (spell->spell_bits, SPELL_ALL_IN_ROOM))
	{
	  cast_at_n = TRUE;
	  victim = ch->in_room->more->people;
	  next = victim->next_in_room;
	}
    }


  for (once = TRUE;
       ((once || next != NULL)
	&& (!victim || (victim && victim->data_type != 50))); victim = next)
    {
      if (victim && IS_PLAYER (victim) && cast_at_n
	  && IS_SET (spell->spell_bits, SPELL_ONLY_MULTI_MOBS))
	{
	  next = victim->next_in_room;
	  continue;
	}
      else if (!sskip && IS_SET (spell->spell_bits, SPELL_ALL_IN_ROOM))
	next = victim->next_in_room;
      else
	next = NULL;
      once = FALSE;
      if (!sskip && IS_SET (spell->spell_bits, SPELL_ONLY_GROUP) && ch &&
	  (((IS_EVIL (ch) && !IS_EVIL (victim))
	    || (!IS_EVIL (ch) && IS_EVIL (victim)))
	   || ((IS_PLAYER (ch) && IS_MOB (victim))
	       || (IS_MOB (ch) && IS_PLAYER (victim)))))
	continue;
      if (!sskip && IS_SET (spell->spell_bits, SPELL_ONLY_NOT_GROUP) && ch &&
	  ((IS_EVIL (ch) && IS_EVIL (victim))
	   || (!IS_EVIL (ch) && !IS_EVIL (victim)))
	  &&
	  ((IS_PLAYER (ch) && IS_MOB (victim)
	    && spell->spell_type != TAR_CHAR_OFFENSIVE) || (IS_MOB (ch) && IS_PLAYER (victim) && spell-> spell_type != TAR_CHAR_OFFENSIVE)
	   || (IS_PLAYER (ch) && !IS_MOB (victim)) || (IS_MOB (ch) && IS_MOB (victim))))
	continue;
      if (IS_SET (spell->spell_bits, SPELL_ALL_IN_ROOM)
	  && IS_SET (spell->spell_bits, SPELL_NOT_SELF) && ch == victim)
	continue;
      if (IS_SET (spell->spell_bits, SPELL_NOT_MOB) && IS_MOB (victim))
	continue;
      if (!victim)
	victim = ch;
/*
    if (IS_SET (spell->spell_bits, SPELL_CHARM_PERSON))
    {
      spell_charm_person (spell->gsn, level, ch, vo);
    }
*/
//raise_undead spell
      if (IS_SET (spell->spell_bits, SPELL_RAISE_UNDEAD))
        {   
          spell_raise_undead (spell, level, ch, vo);
        }
      if (IS_SET (spell->spell_bits, SPELL_SCRY))
	{
	  spell_scry (spell->gsn, level, ch, vo);
	}
      if (IS_SET (spell->spell_bits, SPELL_LOCATE_PERSON))
	{
	  spell_locate_person (spell->gsn, level, ch, vo);
	}
      if (IS_SET (spell->spell_bits, SPELL_REMOVES_BIT))
	{
	  if (!remove_sa (spell, level, ch, vo))
	    {
	    };
	}
      if (IS_SET (spell->spell_bits, SPELL_DISPEL_MAGIC))
	{
	  spell_dispel_magic (spell->gsn, level, ch, vo);
	}
      if (IS_SET (spell->spell_bits, SPELL_IDENTIFY))
	{
	  spell_identify (spell->gsn, level, ch, vo);
	}
      if (IS_SET (spell->spell_bits, SPELL_LOCATE_OBJECT))
	{
	  spell_locate_object (spell->gsn, level, ch, vo);
	}
      if (IS_SET (spell->spell_bits, SPELL_TELEPORT))
	{
	  spell_teleport (spell->gsn, level, ch, vo);
	}
      if (IS_SET (spell->spell_bits, SPELL_SUMMON))
	{
	  spell_summon (spell->gsn, level, ch, vo);
	}
      if (IS_SET (spell->spell_bits, SPELL_RECALL))
	{
	  spell_word_of_recall (spell->gsn, level, ch, vo);
	}

      if (IS_SET (spell->spell_bits, SPELL_ADD_MOVE))
	{
	  victim->move += translate (spell->damage, level, ch);
	  if (victim->move > victim->max_move)
	    {
	      victim->move = victim->max_move;
	    }
	  if (victim->move < 1)
	    {
	      victim->move = 1;
	    }
	}
      if (IS_SET (spell->spell_bits, SPELL_HEALS_DAM))
	{
	  victim->hit += translate (spell->damage, level, ch);
      if (is_member(ch, GUILD_PALADIN) && spell->spell_level > 40
       || is_member(ch, GUILD_ELEMENTAL) && spell->spell_level > 40
       || is_member(ch, GUILD_HEALER) && spell->spell_level > 40)
           {
            victim->hit += ch->pcdata->plus_heal;
           }
	  if (victim->hit > victim->max_hit)
	    victim->hit = victim->max_hit;
	}
      if (IS_SET (spell->spell_bits, SPELL_FIRE))
	{
	  spell_fire_breath (spell->gsn, level, ch, (void *) victim);
	}
      if (IS_SET (spell->spell_bits, SPELL_ACID))
	{
	  spell_acid_breath (spell->gsn, level, ch, (void *) victim);
	}
      if (spell->spell_type == TAR_CHAR_OFFENSIVE && spell->damage != NULL)
	{
	  if (!ch || !victim)
	    return;
	  if (IS_SET (spell->spell_bits, SPELL_HURT_UNDEAD)
	      && (IS_PLAYER (victim)
		  || (IS_MOB (victim)
		      && victim->pIndexData->mobtype != MOB_GHOST
		      && victim->pIndexData->mobtype != MOB_UNDEAD)))
	    {
	      send_to_char ("This spell can only do harm to undead creatures.\n\r", ch);
	      return;
	    }
	  if (IS_SET (spell->spell_bits, SPELL_HURT_GOOD) && IS_EVIL (victim))
	    {
	      send_to_char ("This spell can only do harm to good.\n\r", ch);
	      return;
	    }
	  if (IS_SET (spell->spell_bits, SPELL_HURT_EVIL) && !IS_EVIL (victim))
	    {
	      send_to_char ("This spell can only do harm to evil.\n\r", ch);
	      return;
	    }
	  write_stuff (spell, ch, victim);
	  wrote = TRUE;
//Base spell damage with remort and archmage bonus KILITH 05
	  if (IS_AFFECTED_EXTRA (victim, AFF_MINDSHIELD))
	    { 
                int arch = (ARCHMAGE (ch));
		int remort = (ch->pcdata->remort_times) * 2;
	        int dam = translate (spell->damage, level, ch);
                int dam2 = (((dam + remort) + ((arch * 3) / 3))/2);
      if (is_member(ch, GUILD_WIZARD) && spell->spell_level > 40 
       || is_member(ch, GUILD_ELEMENTAL) && spell->spell_level > 40 
       || is_member(ch, GUILD_BATTLEMAGE) && spell->spell_level > 40)
                {
                dam2 += ch->pcdata->plus_magic/2;
                  }
     if (HAS_HEAD_BRAIN_IMPLANT (ch) && spell->spell_level > 40)
                {
                dam2 += 50;
                }
	        show_hitdam (spell->gsn, spell->noun_damage, dam2, ch, victim);
	        damage (ch, victim, dam2, spell->gsn);
	        if (victim->data_type == 50)
		  continue;
	    }
	  else if (IS_SET (spell->spell_bits, SPELL_HALVED_SAVE) && saves_spell (level, victim))
	    {
                int arch = (ARCHMAGE (ch));
		int remort = (ch->pcdata->remort_times) * 2;
                int dam = translate (spell->damage, level, ch);
                int dam2 = ((dam + remort) + (arch * 3) / 2);
      if (is_member(ch, GUILD_WIZARD) && spell->spell_level > 40
       || is_member(ch, GUILD_ELEMENTAL) && spell->spell_level > 40
       || is_member(ch, GUILD_BATTLEMAGE) && spell->spell_level > 40)
                {
                dam2 += ch->pcdata->plus_magic/2;
                  }
     if (HAS_HEAD_BRAIN_IMPLANT (ch) && spell->spell_level > 40)
                {
                dam2 += 50;
                }
	        show_hitdam (spell->gsn, spell->noun_damage, dam2/2, ch, victim);
	        damage (ch, victim, dam2/2, spell->gsn);
	        if (victim->data_type == 50)
		  continue;
	    }
	  else
	    {
		int arch = (ARCHMAGE (ch));
		int remort = (ch->pcdata->remort_times) * 3/2;
	        int dam = translate (spell->damage, level, ch);
		int dam2 = (dam + remort + (arch * 3));
      if (is_member(ch, GUILD_WIZARD) && spell->spell_level > 40
      || is_member(ch, GUILD_ELEMENTAL) && spell->spell_level > 40
      || is_member(ch, GUILD_BATTLEMAGE) && spell->spell_level > 40)
                {
                dam2 += ch->pcdata->plus_magic;
                  }
     if (HAS_HEAD_BRAIN_IMPLANT (ch) && spell->spell_level > 40)
                {
                dam2 += 50;
                }
	        show_hitdam (spell->gsn, spell->noun_damage, dam2, ch, victim);
	        damage (ch, victim, dam2, spell->gsn);
	        if (victim->data_type == 50)
		  continue;
	    }
	}
      if (str_cmp (spell->duration, "N/A") && translate (spell->duration, level, ch) > 0)
	{
	  dur = translate (spell->duration, level, ch);
	  if (spell->spell_type == TAR_CHAR_OFFENSIVE)
	    {
	      if (victim && (saves_spell (level, victim)) && ch != victim)
		{
		  act ("$N resisted the effects of your spell!", ch, NULL, victim, TO_CHAR);
		  act ("You resisted the effects of $n's spell!", ch, NULL, victim, TO_VICT);
		  if (FIGHTING (victim) == NULL)
		    set_fighting (victim, ch);
		  goto dunny;
		}
	      if (IS_AFFECTED_EXTRA (victim, AFF_MINDSHIELD))
		{
		  dur /= 2;
		}

	    }
	  paf.type = spell->gsn;
	  paf.duration = dur;
	  paf.location = spell->location;
	  paf.modifier = translate (spell->modifier, level, ch);
	  paf.bitvector = spell->bitvector;
	  paf.bitvector2 = spell->bitvector2;
	  paf.long_msg = NULL;
	  if (spell->long_affect_name)
	    paf.long_msg = strdup (spell->long_affect_name);
	  paf.short_msg = NULL;
	  if (spell->short_affect_name)
	    paf.short_msg = strdup (spell->short_affect_name);

	  if (!(IS_MOB (victim) && IS_PLAYER (ch) &&
		(IS_SET (paf.bitvector, AFF_FLYING) ||
		 IS_SET (paf.bitvector, AFF_SANCTUARY) ||
		 IS_SET (paf.bitvector, AFF_INVISIBLE) ||
		 IS_SET (paf.bitvector, AFF_BREATH_WATER)/* ||
		 IS_SET (paf.bitvector, AFF_INSANITY)*/)))
	    {

	      renew_affect (victim, &paf);
	    }
	  if (paf.bitvector == AFF_SLEEP)
	    {
	      do_sleep (victim, "");
	      NEW_POSITION (victim, POSITION_SLEEPING);
	    }
	}
      if (!wrote)
	write_stuff (spell, ch, victim);
    dunny:
      if (ch)
	{
	};
    }
  if (spell && spell->linked_to != NULL)
    {
      SPELL_DATA *sp;
      if ((sp = skill_lookup (spell->linked_to, -1)) != NULL)
	{
	  general_spell (sp, level, ch, vo);
	}
    }
  if (IS_PLAYER (ch))
    skill_gain (ch, spell->gsn, TRUE);
  return;
}


void
spell_message (int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *rec;
  char buf[STD_LENGTH];
  char targname[SML_LENGTH];
  if (!target_name || target_name[0] == '\0')
    {
      send_to_char ("A voice booms from the heavens:\n\r", ch);
      send_to_char
	("\x1B[1mSyntax\x1B[0m: cast 'message' <\x1B[1mtarget\x1B[0m> <\x1B[1mmsg\x1B[0m>\n\r",
	 ch);
      return;
    }
  target_name = one_argy (target_name, targname);
  if ((rec = get_char_world (ch, targname)) == NULL)
    {
      send_to_char
	("You send a voice, but you get the feeling it could not deliver.\n\r",
	 ch);
      return;
    }
  sprintf (buf,
	   "\x1B[1;32mA voice from afar tells you '\x1B[0m%s\x1B[1;32m'\x1B[0m\n\r",
	   target_name);
  send_to_char (buf, rec);
  sprintf (buf, "You send a mystical voice to talk to %s.", NAME (rec));
  send_to_char (buf, ch);
  return;
}

void
spell_charm_person (int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  bzero (&af, sizeof (af));
  if (victim == ch)
    {
      send_to_char ("You like yourself even better!\n\r", ch);
      return;
    }
  if (IS_MOB (victim) && (victim->pIndexData->mobtype == MOB_GHOST))
    {
      act ("$N doesn't have a body to charm.", ch, NULL, victim, TO_CHAR);
      return;
    }
  if (IS_MOB (victim) && IS_SET (victim->act, ACT_UNTOUCHABLE))
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }
  if (IS_AFFECTED (victim, AFF_CHARM)
      || IS_AFFECTED (ch, AFF_CHARM)
      || level < LEVEL (victim) - 6
      || IS_PLAYER (victim)
      || IS_AFFECTED (victim, AFF_NOCHARM) || saves_spell (level, victim))
    {
      act ("$N snarls at you in complete hatred!", ch, NULL, victim, TO_CHAR);
      act ("$n just tried to charm you!!", ch, NULL, victim, TO_VICT);
      return;
    }
  if (MASTER (victim) != NULL)
    stop_follower (victim);
  add_follower (victim, ch);
  af.type = sn;
  af.duration = number_fuzzy (level / 10);
  af.location = 0;
  af.modifier = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char (victim, &af);
  act ("You feel compelled to follow $n.", ch, NULL, victim, TO_VICT);
  if (ch != victim)
    act ("$N gazes at you as if $S life revolved around you.",
	 ch, NULL, victim, TO_CHAR);
  return;
}

// The Actual Raise Undead Spell.
void 
spell_raise_undead (SPELL_DATA * spell, int level, CHAR_DATA * ch, void *vo) {

        CHAR_DATA *undead_creature;
        MOB_PROTOTYPE *the_corpse;
        MOB_PROTOTYPE *the_mobile;
        AFFECT_DATA af;

        char buf[500];

        int sn = spell->gsn;

        // Grab the target corpse object
        SINGLE_OBJECT *target_corpse = (SINGLE_OBJECT *) vo;

        // Return if target corpse is a PC
        if ((target_corpse->pIndexData->item_type != ITEM_CORPSE_NPC))  {
          send_to_char ("You just can't seem to raise that from the dead!\n\r", ch);
          return;
        }

        // Get the mobile number from the spell data
        the_mobile = get_mob_index (spell->creates_obj);

        if (the_mobile == NULL || the_mobile->vnum == 1) { // Nothing there!
          // Get the corpse mobs vnum number and create it
          the_mobile = get_mob_index (((I_CONTAINER *)(target_corpse->more))->key_vnum);
        }

        if (the_mobile == NULL || the_mobile->vnum == 1) {
          send_to_char ("You just can't seem to raise that from the dead!\n\r", ch);
          return;
        }

        undead_creature = create_mobile (the_mobile);

        char_to_room (undead_creature, ch->in_room);

        act ("$n has raised $N from the dead\x1b[0;37m!", ch, NULL, undead_creature, TO_ROOM);
        act ("You raise $N from the dead\x1b[0;37m!", ch, NULL, undead_creature, TO_CHAR);
        free_it (target_corpse);

        if (!(ch->number_of_mob_followers)) ch->number_of_mob_followers = 0;
        else if (ch->number_of_mob_followers > 1) {
          send_to_char ("But it falls *THUD* to the floor and decomposes...too many followers perhaps?\n\r",ch);
          act ("But it falls *THUD* to the floor and decomposes...", ch, NULL, NULL, TO_ROOM);
          free_it (undead_creature);
          return;
        }

        bzero (&af, sizeof (af));

        if (level < (LEVEL (undead_creature)-60) || IS_AFFECTED (undead_creature, AFF_NOCHARM)) {
          act ("$N has been raised but snarls at you in complete hatred and attacks!", ch, NULL, undead_creature, TO_CHAR);
          act ("$N snarls at $n in complete hatred and attacks!", ch, NULL, undead_creature, TO_ROOM);
          check_fgt(undead_creature);
          undead_creature->is_undead = TRUE;
          undead_creature->fgt->fighting = ch;
          NEW_POSITION(undead_creature, POSITION_FIGHTING);
          multi_hit (undead_creature, ch, TYPE_UNDEFINED);
          return;
        }
        // Fix undead stats so it can group, assist, etc
        undead_creature->pcdata->alignment = ch->pcdata->alignment;
        undead_creature->pIndexData->alignment = ch->pcdata->alignment;
        //sprintf(buf,"Setting undead creature alignement to %d\n\r",undead_creature->pcdata->alignment);
        send_to_char (buf,ch);
        undead_creature->pcdata->act2 = PLR_AUTOGOLD | PLR_AUTOSPLIT | PLR_ASSIST;

        add_follower (undead_creature, ch);
        do_group (ch, undead_creature->pcdata->name);
        (ch->number_of_mob_followers)++;
        af.type = sn;
        af.duration = -1;
        af.location = 0;
        af.modifier = 0;
        af.bitvector = AFF_CHARM;
        affect_to_char (undead_creature, &af);
        undead_creature->is_undead = TRUE;

        act ("$N gazes at you as if $S life revolved around you.", ch, NULL, undead_creature, TO_CHAR);
        return;
}

void
spell_scry (int sn, int level, CHAR_DATA * ch, void *vo)
{
  char buf[STD_LENGTH];
  if (target_name == '\0')
    {
      return;
    }
  if (!scry (ch, level, target_name))
    {
      sprintf (buf, "You were unable to scry %s.\n\r", target_name);
      send_to_char (buf, ch);
    }
  return;
}

void
spell_locate_person (int sn, int level, CHAR_DATA * ch, void *vo)
{
  char buf[STD_LENGTH];
  if (target_name == '\0')
    {
      return;
    }
  if (!locate_person (ch, level, target_name))
    {
      sprintf (buf, "You were unable to locate %s.\n\r", target_name);
      send_to_char (buf, ch);
    }
  return;
}

void
spell_identify (int sn, int level, CHAR_DATA * ch, void *vo)
{
  SINGLE_OBJECT *obj = (SINGLE_OBJECT *) vo;
  SPELL_DATA *spp;
  char buf3[500];
  char buf2[STD_LENGTH];
  char buf[STD_LENGTH];
  AFFECT_DATA *paf;
  send_to_char ("\x1B[0m\x1B[1;34m[\x1B[1;32m+\x1B[1;34m]-----------------------------------------------------------------------------[\x1B[1;32m+\x1B[1;34m]\n\r", ch);
  sprintf (buf, "\x1B[1m'\x1B[0m%s\x1B[1m' \x1B[0;36mis some sort of \x1B[1;32m%s\x1B[0;36m.\n\r", STR (obj, short_descr), item_type_name (obj->pIndexData->item_type));
  send_to_char (buf, ch);
  sprintf (buf, "\x1B[0;36mIt weighs \x1B[1;32m%d kg\x1B[0;36m, and is apparantly worth \x1B[0;33m%d\x1B[0;33m copper\x1B[0;36m.\n\r", obj->pIndexData->weight, obj->cost);
  send_to_char (buf, ch);
  sprintf (buf, "\x1B[0;36mIt is affected by \x1B[1;30m[\x1B[1;36m%s\x1B[1;30m]\n\r", extra_bit_name (obj->extra_flags));
  send_to_char (buf, ch);
  sprintf (buf, "\x1B[0;36m- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n\r");
  switch (obj->pIndexData->item_type)
    {
    case ITEM_SCROLL:
    case ITEM_POTION:
      {
	I_POTION *pot = (I_POTION *) obj->more;
	send_to_char (buf, ch);
	sprintf (buf, "Level \x1B[1m%d\x1B[0m spells of:", pot->spell_level);
	send_to_char (buf, ch);
	if (pot->spells[0] >= 0 && pot->spells[0] < SKILL_COUNT)
	  {
	    send_to_char (" '\x1B[1m", ch);
	    if ((spp = skill_lookup (NULL, pot->spells[0])) != NULL)
	      send_to_char (spp->spell_name, ch);
	    send_to_char ("\x1B[0m'", ch);
	  }
	if (pot->spells[1] >= 0 && pot->spells[1] < SKILL_COUNT)
	  {
	    send_to_char (" '\x1B[1m", ch);
	    if ((spp = skill_lookup (NULL, pot->spells[1])) != NULL)
	      send_to_char (spp->spell_name, ch);
	    send_to_char ("\x1B[0m'", ch);
	  }
	if (pot->spells[2] >= 0 && pot->spells[2] < SKILL_COUNT)
	  {
	    send_to_char (" '\x1B[1m", ch);
	    if ((spp = skill_lookup (NULL, pot->spells[2])) != NULL)
	      send_to_char (spp->spell_name, ch);
	    send_to_char ("\x1B[0m'", ch);
	  }
	send_to_char (".\n\r", ch);
      }
      break;
    case ITEM_WAND:
    case ITEM_STAFF:
      {
	I_WAND *wnd = (I_WAND *) obj->more;
	send_to_char (buf, ch);
	sprintf (buf, "Has \x1B[34m%d\x1B[0m of \x1B[1;34m%d\x1B[0m charges of level \x1B[1m%d\x1B[0m", wnd->current_charges, wnd->max_charges, wnd->spell_level);
	send_to_char (buf, ch);
	if (wnd->spells[0] >= 0 && wnd->spells[0] < SKILL_COUNT)
	  {
	    send_to_char (" '\x1B[1m", ch);
	    if ((spp = skill_lookup (NULL, wnd->spells[0])) != NULL)
	      send_to_char (spp->spell_name, ch);
	    send_to_char ("\x1B[0m'", ch);
	  }
	send_to_char (".\n\r", ch);
	break;
      }
    case ITEM_GEM:
      {
	I_GEM *gem = (I_GEM *) obj->more;
	sprintf (buf2, "\x1B[0;36mMana Type(s): ");
	if (IS_SET (gem->gem_type, MANA_FIRE))
	  strcat (buf2, "\x1B[0;31mFIRE ");
	if (IS_SET (gem->gem_type, MANA_WATER))
	  strcat (buf2, "\x1B[0;34mWATER ");
	if (IS_SET (gem->gem_type, MANA_EARTH))
	  strcat (buf2, "\x1B[0;33mEARTH ");
	if (IS_SET (gem->gem_type, MANA_AIR))
	  strcat (buf2, "\x1B[0;36mAIR ");
        if (IS_SET (gem->gem_type, MANA_DEATH))
          strcat (buf2, "\x1B[1;30mDEATH ");
	strcat (buf2, "\x1B[37;0m");
	sprintf (buf3, "\n\r\x1B[0;36mExtra Mana Gained: \x1B[37;1m%d\x1B[0m.", gem->max_mana);
	strcat (buf2, buf3);
	send_to_char (buf2, ch);
	send_to_char ("\n\r\x1B[1;31mGems may not give you their max mana if you have low intelligence or wisdom.\n\r", ch);
	sprintf (buf2, "\n\r\x1B[0;36mThis gem will make your max mana level of: \x1B[1;32m%d\n\r", gem->max_level);
	send_to_char (buf2, ch);
	break;
      }
    case ITEM_WEAPON:
      {
	I_WEAPON *weap = (I_WEAPON *) obj->more;
	send_to_char (buf, ch);
	sprintf (buf, "This weapon is capable of \x1B[1;34m%d\x1B[0md\x1B[1;34m%d\x1B[0m damage.\n\r", weap->firstdice, weap->seconddice);
	send_to_char (buf, ch);
	break;
      }
    case ITEM_ARMOR:
      {
	I_ARMOR *arm = (I_ARMOR *) obj->more;
	send_to_char (buf, ch);
	sprintf (buf, "Leg AC: %d  Body AC: %d  Head AC: %d  Arm AC: %d.\n\r", arm->protects_legs, arm->protects_body, arm->protects_head, arm->protects_arms);
	send_to_char (buf, ch);
	break;
      }
    }
  for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
    {
      if (paf->location != APPLY_NONE && paf->modifier != 0)
	{
	  if (paf->modifier > 0)
	    sprintf (buf, "\x1B[0;36mIt is +\x1B[1;36m%d\x1B[0;36m to \x1B[36m%s\x1B[37;0m.\n\r", paf->modifier, affect_loc_name (paf->location));
	  else
	    sprintf (buf, "\x1B[0;36mIt is -\x1B[1;36m%d\x1B[0;36m to \x1B[36m%s\x1B[37;00m.\n\r", paf->modifier * -1, affect_loc_name (paf->location));
	  send_to_char (buf, ch);
	}
    }

  send_to_char ("\x1B[0m\x1B[1;34m[\x1B[1;32m+\x1B[1;34m]-----------------------------------------------------------------------------[\x1B[1;32m+\x1B[1;34m]\n\r", ch);
  return;
}


void
spell_locate_object (int sn, int level, CHAR_DATA * ch, void *vo)
{
  char buf[SML_LENGTH];
  SINGLE_OBJECT *obj;
  SINGLE_OBJECT *in_obj;
  bool found;
  int number = 0;
  found = FALSE;
  if (level > 100 || LEVEL (ch) > 100)
    return;
  for (obj = object_list; obj != NULL && number <= LEVEL (ch);
       obj = obj->next)
    {
      if (!can_see_obj (ch, obj) || !is_name (target_name, obj->pIndexData->name))
	continue;
      found = TRUE;
      number++;
      for (in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj)
	;
      if (in_obj->carried_by != NULL)
	{
	  sprintf (buf, "%s carried by %s.\n\r", STR (obj, short_descr), PERS (in_obj->carried_by, ch));
	}
      else
	{
	  sprintf (buf, "%s in %s.\n\r", STR (obj, short_descr), in_obj->in_room == NULL ? "somewhere" : in_obj->in_room->name);
	}
      buf[0] = UPPER (buf[0]);
      send_to_char (buf, ch);
    }
  if (!found)
    send_to_char ("Nothing like that in hell, heaven or earth.\n\r", ch);
  return;
}

void
spell_acid_breath (int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  SINGLE_OBJECT *obj_lose;
  SINGLE_OBJECT *obj_next;
  if (number_percent () < 2 * level)
    {
      for (obj_lose = victim->carrying; obj_lose != NULL; obj_lose = obj_next)
	{
	  int iWear;
	  obj_next = obj_lose->next_content;
	  if (number_range (1, 10) > 6)
	    continue;
	  switch (obj_lose->pIndexData->item_type)
	    {
	    case ITEM_ARMOR:
	      if (((I_ARMOR *) obj_lose->more)->condition_now > 0)
		{
		  act ("$p is covered with acid!",
		       victim, obj_lose, NULL, TO_CHAR);
		  if ((iWear = obj_lose->wear_loc) != WEAR_NONE)
		    victim->armor += apply_ac (obj_lose, iWear);
		  if (((I_ARMOR *) obj_lose->more)->condition_now > 3)
		    ((I_ARMOR *) obj_lose->more)->condition_now -= 3;
		  if (iWear != WEAR_NONE)
		    victim->armor -= apply_ac (obj_lose, iWear);
		}
	      break;
	    case ITEM_CONTAINER:
	      {
		act ("$p fumes and dissolves! Anything in your container is now in your inventory!", victim, obj_lose, NULL,
		     TO_CHAR);
		{		// move contents to inv, added 08-27-01 by Eraser
		  SINGLE_OBJECT *cont;
		  SINGLE_OBJECT *cont_next;
		  for (cont = victim->carrying; cont; cont = cont_next)
		    {
		      cont_next = cont->next;
		      if (cont->in_obj == obj_lose)
			{
			  obj_from (cont);
			  obj_to (cont, victim);
			}
		    }
		}
		obj_from (obj_lose);	// player loses container
		free_it (obj_lose);	// container and contents get destroyed
		break;
	      }			// end case container
	    }			// end switch
	}
    }
  return;
}

void
spell_fire_breath (int sn, int level, CHAR_DATA * ch, void *vo)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  SINGLE_OBJECT *obj_lose;
  SINGLE_OBJECT *obj_next;
  if (number_percent () < 2 * level && !saves_spell (level, victim))
    {
      for (obj_lose = victim->carrying; obj_lose != NULL; obj_lose = obj_next)
	{
	  char *msg;
	  obj_next = obj_lose->next_content;
	  if (number_range (1, 100) > 90 )
	    continue;
	  switch (obj_lose->pIndexData->item_type)
	    {
	    default:
	      continue;
	    /*case ITEM_CONTAINER:
	      msg = "$p ignites and burns!";
	      break;*/
	    case ITEM_POTION:
	      msg = "$p bubbles and boils!";
	      break;
	    case ITEM_SCROLL:
	      msg = "$p crackles and burns!";
	      break;
	    case ITEM_STAFF:
	      msg = "$p smokes and chars!";
	      break;
	    case ITEM_WAND:
	      msg = "$p sparks and sputters!";
	      break;
	    case ITEM_FOOD:
	      msg = "$p blackens and crisps!";
	      break;
	    case ITEM_PILL:
	      msg = "$p melts and drips!";
	      break;
	    }
	  act (msg, victim, obj_lose, NULL, TO_CHAR);
	  obj_from (obj_lose);
	  free_it (obj_lose);
	}
    }
  return;
}
