/* Check that mobs yell when you cast a spell at them */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"

#define CHECK_CHAR(ch)   if ( (ch)->data_type==50) return

bool hitop = FALSE;
bool using_weapon = FALSE;
bool using_shield = FALSE;

int ghit_pos = 0;
char reasonfd[30];

extern void mob_undef (int, CHAR_DATA *, CHAR_DATA *); // Kenor '03

void
do_tackle (CHAR_DATA * ch, char *argy)
{
  CHAR_DATA *victim;
  int chn;
  SINGLE_OBJECT *obj;
  int xn;
  char buf[STD_LENGTH];
  DEFINE_COMMAND ("tackle", do_tackle, POSITION_FIGHTING, 0, LOG_NORMAL, "This command makes your character attempt to tackle his/her opponent.")
    check_fgt (ch);
  if (ch->pcdata->learned[gsn_tackle] < 20)
   {
      send_to_char ("You can't tackle!!!! You do not know how!!!!\n\r", ch);
      return;
    } 
  if (ch->hit < ch->max_hit / 4)
   {
      send_to_char ("You have less then 1/4 of your hitpoints left!! That would not be a very wise thing to do!!!!\n\r", ch);
      return;
    }  
  if (ch->fgt->ears > 9)
    {
      send_to_char ("You can't tackle this soon after giving or recieving a bash.\n\r", ch);
      return;
    }
 chn = 0;
  if (ch->wait > 2)
    return;
  if (FIGHTING (ch) == NULL)
    {
      if (argy[0] == '\0' || argy == "")
	{
	  send_to_char ("Tackle who?\n\r", ch);
	  return;
	}
      if ((victim = get_char_room (ch, argy)) == NULL)
	{
	  send_to_char ("Tackle who?\n\r", ch);
	  return;
	}
      if (victim == ch)
	{
	  send_to_char ("You really do look like an idiot!\n\r", ch);
	  return;
	}
    }
  else
    victim = FIGHTING (ch);
  if (RIDING (ch) != NULL)
    {
      send_to_char ("You can't tackle someone when you are riding!\n\r", ch);
      return;
    }
  if (is_safe (ch, victim))
    {
      return;
    }
  if (FIGHTING (ch) != victim)
    {
      if (!pkill (ch, victim))
	return;
      check_pkill (ch, victim);
    }
  if (!can_see (ch, victim))
    {
      send_to_char ("How can you tackle someone you can't see!?\n\r", ch);
      return;
    }
  if (ch->position == POSITION_BASHED)
    {
      act ("You are bashed on the ground, how do you expect to tackle!?", ch, NULL, victim, TO_CHAR);
      return;
    }
 if (ch->position == POSITION_STOPCAST)
    {
    send_to_char ("You can not try to tackle someone rigth after you tried to cast, YOU WOULD GET SMOKED!!\n\r", ch);
    NEW_POSITION (ch, POSITION_STANDING);
    return;
    }
  if (ch->position == POSITION_GROUNDFIGHTING)
    {
      act ("You are on the ground; how do you expect to tackle $N?", ch, NULL, victim, TO_CHAR);
      return;
    }
  if (!can_groundfight (victim))
    {
      act ("You do not want to tackle that kind of creature!", ch, NULL, victim, TO_CHAR);
      return;
    }
  if (victim->position == POSITION_GROUNDFIGHTING)
    {
      act ("$N is already on the ground, tackled!", ch, NULL, victim, TO_CHAR);
      return;
    }
  if (IS_PLAYER (ch) && IS_PLAYER (victim) && (IS_EVIL (ch) == IS_EVIL (victim)) && HP_NOW (ch) < 100)
    {
      send_to_char ("Maybe that's not the right thing to do.\n\r", ch);
      return;
    }
  check_ced (ch);
  check_ced (victim);
  if (!(IS_IN_MELEE (ch, victim)))
    {
      send_to_char ("You are not in the main melee!\n\r", ch);
      return;
    }
  if (!pow.tackle_with_weapon)
    {
      for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
	if (obj->pIndexData->item_type == ITEM_WEAPON && (obj->wear_loc == WEAR_HOLD_1 || obj->wear_loc == WEAR_HOLD_2))
	  {
	    send_to_char ("You cannot tackle with a weapon in hand!\n\r", ch);
	    return;
	  }
    }
  if (!pow.tackle_person_with_weapon)
    {
      for (obj = victim->carrying; obj != NULL; obj = obj->next_content)
	if (obj->pIndexData->item_type == ITEM_WEAPON
	    && (obj->wear_loc == WEAR_HOLD_1 || obj->wear_loc == WEAR_HOLD_2))
	  {
	    send_to_char ("Are you CRAZY!? Tackle someone with a weapon!??\n\r", ch);
	    send_to_char ("You'd surely get whacked on your approach...\n\r", ch);
	    return;
	  }
    }

#ifndef OLD_FIGHT
  if (!ch->fgt || is_fighting_near (ch, victim) == -1)
    {
      send_to_char ("You must be next to that opponent to tackle him/her/it.\n\r", ch);
      return;
    }

  if (ch->fgt && ch->fgt->field_ticks > 0)
    {
      char bb[500];
      sprintf (bb, "tackle %s", argy);
      setnext (ch, bb);
      return;
    }
#endif



  if (IS_MOB (ch))
    chn = (LEVEL (ch) * 2) + 15;
  else
    chn = (get_curr_dex (ch) * 2) + (LEVEL (ch) * 2 - (get_curr_dex (victim)));
  chn -= (LEVEL (victim) * 2) + (IS_MOB (victim) ? 15 : 0);
  if (chn > 110)
    chn = 110;
  if (victim->ced && IS_SET (victim->ced->fight_ops, F_GROUND))
    xn = 150;
  else
    xn = 370;
if (IS_PLAYER (ch) && (number_percent () + (get_curr_dex (victim) + 10) + (LEVEL (victim)/40)) > (ch->pcdata->learned[gsn_tackle] + (get_curr_dex (ch)/2) - 15))
    {
      send_to_char ("\x1B[1;31mYou miss the tackle and almost fall on the ground!\x1B[0;37m\n\r", ch);
      act ("$n tries to tackle you, but almost falls over instead!", ch, NULL, victim, TO_VICT);
      act ("$n misses a tackle at $N and almost falls over!", ch, NULL, victim, TO_NOTVICT);
      WAIT_STATE (ch, 7 * PULSE_VIOLENCE);
      SUBHIT (ch, 10);
  if (IS_PLAYER (ch) && number_range(1,5) == 3)
   {
     skill_gain (ch, gsn_tackle, TRUE);
   }
      if (FIGHTING (victim) == NULL)
	set_fighting (victim, ch);
      set_fighting (ch, victim);
      return;
    }
  check_ced (victim);
  check_ced (ch);
  victim->ced->ground_pos = 0;
  ch->ced->ground_pos = 0;
  NEW_POSITION (victim, POSITION_GROUNDFIGHTING);
  NEW_POSITION (ch, POSITION_GROUNDFIGHTING);
  if (RIDING (victim) != NULL)
    {
      sprintf (buf, "As you jump up and tackle $N, $S flies off of %s and lands hard on the ground!", PERS (RIDING (victim), ch));
      act (buf, ch, NULL, victim, TO_CHAR);
      sprintf (buf, "$n's flying tackle makes $N fall off of %s. $N lands on his back on the ground.", NAME (RIDING (victim)));
      act (buf, ch, NULL, victim, TO_NOTVICT);
      sprintf (buf, "You fall off of %s as $n tackles you!", PERS (RIDING (victim), victim));
      act (buf, ch, NULL, victim, TO_VICT);
      check_fgt (victim->fgt->riding);
      victim->fgt->riding->fgt->mounted_by = NULL;
      victim->fgt->riding = NULL;
    }
  else if (RIDING (victim) == NULL)
    {
      act ("You tackle $B$4$N! $RThe fight is taken to the ground!", ch, NULL, victim, TO_CHAR);
      act ("$B$4$n $Rtackles $B$4$N $Rand they both end up on the ground!", ch, NULL, victim, TO_NOTVICT);
      act ("$B$4$n $Rtackles you! The fight is taken to the ground!", ch, NULL, victim, TO_VICT);
    }
  set_fighting (ch, victim);
  set_fighting (victim, ch);
  victim->fgt->fighting = ch;
  ch->fgt->fighting = victim;
  victim->ced->ground_pos = 0;
  ch->ced->ground_pos = 0;
  if (IS_PLAYER (ch) && number_range(1,5) == 3)
   {
     skill_gain (ch, gsn_tackle, TRUE);
   }
  NEW_POSITION (victim, POSITION_GROUNDFIGHTING);
  NEW_POSITION (ch, POSITION_GROUNDFIGHTING);
  WAIT_STATE (victim, 2 * PULSE_VIOLENCE);
  WAIT_STATE (ch, (5 * PULSE_VIOLENCE));
  SUBHIT (ch , 5);
  return;
}

void
violence_update (bool flaggy)
{
  CHAR_DATA *ch;
  CHAR_DATA *ch_next;
  CHAR_DATA *victim;
  CHAR_DATA *rch;
  CHAR_DATA *rch_next;
  int posorg;
  combat_state++;
  if (combat_state == 4)
    combat_state = 0;
  for (ch = f_first; ch != NULL; ch = ch_next)
    {
      if (ch->data_type == 50)
	{
	  fprintf (stderr, "That was it! Is this the problem? This is in fgt.c.\n");
	  remove_from_fighting_list (ch);
	  return;
	}
      if (FIGHTING (ch) && FIGHTING (ch)->data_type == 50)
	{
	  fprintf (stderr, "That was it2! Is this the problem? This is in fgt.c.\n");
	  ch->fgt->fighting = NULL;
	  return;
	}
      ch_next = ch->next_fighting;
      check_fgt (ch);
      if (!ch->in_room)
	continue;
      if (ch->position == POSITION_FIGHTING && !FIGHTING (ch))
	NEW_POSITION (ch, POSITION_STANDING);
      if (!flaggy)
	{
      if (IS_MOB (ch) && IS_SET (ch->pIndexData->act4, ACT4_FASTHUNT) && ch->fgt 
          && ch->fgt->hunting != NULL && FIGHTING (ch) == NULL && ch->move > 5 && ch->position == POSITION_STANDING)
      if (number_range (1, 3) == 2 && IS_MOB (ch))
	 hunt_victim (ch);
      if (ch->fgt->ears == 10)
	    {
	      ch->fgt->ears = 2;
	      if (ch->position == POSITION_BASHED)
		do_stand (ch, "");
	    }
	  else if (ch->fgt->ears > 10)
	    ch->fgt->ears--;
	  if (IS_MOB (ch) && FIGHTING (ch) == NULL && (IS_AFFECTED (ch, AFF_CHARM)
		  /*|| (ch->npcdata->hire && ch->npcdata->hire!=NULL) */ ))
	    {
/*if (ch->npcdata->hire!=NULL && ch->npcdata->hire->fighting!=NULL) multi_hit(ch,ch->npcdata->hire->fighting, TYPE_UNDEFINED); */
if (IS_AFFECTED (ch, AFF_CHARM) && ch->fgt->master && ch->fgt->master != NULL && (ch->fgt->master->fgt == NULL || ch->fgt->master->fgt-> fighting != NULL))
#ifdef OLD_FIGHT
		multi_hit (ch, ch->fgt->master->fgt->fighting, TYPE_UNDEFINED);
#else
		set_fighting (ch, ch->fgt->master->fgt->fighting);
#endif
	    }
	}
      /*End if !flaggy */
      if ((victim = FIGHTING (ch)) == NULL)	/*Below applies to fighting only */
	continue;

      if (FIGHTING (victim) == NULL)
	{
	 /* victim->fgt->fighting = ch;*/
         set_fighting (victim, ch);
	}
      if (IS_AWAKE (ch) && ch->in_room == victim->in_room)
	{
	  if (!flaggy && IS_MOB (ch) && IS_SET (ch->pIndexData->act4, ACT4_MOBSWITCH))
	    {
	      CHAR_DATA *cir;
	      int percent = 100;
	      int count = -1;
	      cir = ch->in_room->more->people;
	      while (cir != NULL)
		{
		  cir = cir->next_in_room;
		  count++;
		}
	      if (count != 0)
		percent = 100 / count;
	      for (cir = ch->in_room->more->people; cir != NULL;
		   cir = cir->next_in_room)
		{
		  if (FIGHTING (cir) == ch && (number_percent () <= percent) && FIGHTING (ch) != cir)
		    {
		      act ("$n turns and attacks $N!", ch, NULL, cir, TO_NOTVICT);
		      act ("$n turns towards you and attacks!", ch, NULL, cir, TO_CHAR);
		      set_fighting (ch, cir);
		      break;
		    }
		}
	    }
	  posorg = victim->position;
	  if (ch->fgt->challenge == 10 && number_range (1, 9) == 2)
	    {
	      char bufr[500];
	      sprintf (bufr, "($B$1Arena$N$7) $B%s: $4%s $7%s: $4%s", NAME (ch), STRING_HITS (ch), NAME (victim), STRING_HITS (victim));
	      do_arenaecho (ch, bufr, TRUE);
	    }
	  if (ch != victim && ch->position == POSITION_GROUNDFIGHTING
	      && victim->position == POSITION_GROUNDFIGHTING)
	    {
	      if (!flaggy)
		ground_hit (ch, victim, TYPE_UNDEFINED);
	    }
	  else if (ch->position != POSITION_CASTING)
	    {
	      if (ch->fgt->challenge == 10 && number_range (1, 6) == 2)
		{
		  char buf[500];
		  int i;
		  i = number_range (1, 5);
		  if (i == 1)
		    sprintf (buf, "($B$1Arena$N$7) $BBlood runs freely as %s and %s exchange blows.", NAME (ch), NAME (victim));
		  if (i == 2)
		    sprintf (buf, "($B$1Arena$N$7) $BThe two fighters circle, flailing at each other wildly.");
		  if (i == 3)
		    sprintf (buf, "($B$1Arena$N$7) $BPieces of flesh fly everywhere as blows are exchanged.");
		  if (i == 4)
		    sprintf (buf, "($B$1Arena$N$7) $B%s lets out a loud warcry before attacking!", NAME (ch));
		  if (i == 5)
		    sprintf (buf, "($B$1Arena$N$7) $BYou can almost hear the crunching of bones...");
		  do_arenaecho (ch, buf, TRUE);
		}
	      if (IS_MOB (ch) && ch->pIndexData && ch->pIndexData->opt)
		{
		  if (number_range (1, 101) <
		      ch->pIndexData->opt->warrior_percent)
		    {
		      int cnt = 0;
		      if (ch->pIndexData->opt->flurries)
			cnt++;
		      if (ch->pIndexData->opt->tackles)
			cnt++;
		      if (ch->pIndexData->opt->kicks)
			cnt++;
		      if (number_range (1, cnt) == 1 && ch->pIndexData->opt->flurries && ch->move > 40)
			{
			  do_flurry (ch, "");
			}
		      else if (number_range (1, cnt) == 1 && ch->pIndexData->opt->tackles)
			{
			  do_tackle (ch, "");
			}
		      else if (number_range (1, cnt) == 1 && ch->pIndexData->opt->kicks)
			{
			  do_kick (ch, "");
			}
		    }
		  else
		    {
		      if (number_range (1, 101) < ch->pIndexData->opt->cast_percent)
			{
			  int iil;
			  int cnt = 0;
			  for (iil = 0; iil < 10; iil++)
			    if (ch->pIndexData->opt->cast_spells[iil] != NULL)
			      cnt++;
			  for (iil = 0; iil < 10; iil++)
			    {
			      if (number_range (1, cnt) == 1
				  && ch->pIndexData->opt->cast_spells[iil] != NULL)
				{
				  SPELL_DATA *spl;
				  if ((spl = skill_lookup (ch->pIndexData->opt->cast_spells[iil], -1)) == NULL)
				    continue;
				  if (LEVEL (ch) < spl->spell_level)
				    continue;
				  if (spl->spell_type == TAR_CHAR_DEFENSIVE ||
				      spl->spell_type == TAR_CHAR_SELF)
				    {
				      general_spell (spl, LEVEL (ch), ch, ch);
				    }
				  else
				    general_spell (spl, LEVEL (ch), ch,
						   victim);
				  break;
				}
			    }
			}
		    }
		}
#ifdef OLD_FIGHT
	      if (pow.old_attack)
		{
		  multi_hit (ch, victim, TYPE_UNDEFINED);
		}
	      else
#endif

	      if (!flaggy)
		{
		  if (FIGHTING (ch) && FIGHTING (FIGHTING (ch)) == ch
		      && !ch->fgt->attacker_turn)
		    continue;
		  if (FIGHTING (ch) && FIGHTING (FIGHTING (ch)) != ch
		      && number_range (1, 2) == 2)
		    continue;
#ifdef OLD_FIGHT
		  multi_hit (ch, victim, TYPE_UNDEFINED);
#endif
		}
	      else
		{
		  /*if (ch->fgt->attacks<2) continue; */
		  ch->fgt->attacks = 0;
#ifdef OLD_FIGHT
		  multi_hit (ch, victim, TYPE_UNDEFINED + 20000);
#endif
		  ch->fgt->attacks = 1;
		}
	    }
	  if (posorg == POSITION_CASTING
	      && victim->position != POSITION_CASTING)
	    {
	      send_to_char ("\x1B[5;31mOUCH! You just lost your concentration!\x1b[37;0m\n\r", victim);
	      if (IS_PLAYER (victim))
		victim->pcdata->tickcounts = 0;
	    }
	}
      else
	stop_fighting (ch, FALSE);
      if ((victim = FIGHTING (ch)) == NULL)
	continue;
      for (rch = ch->in_room->more->people; rch != NULL; rch = rch_next)
	{
	  rch_next = rch->next_in_room;
	  if (IS_AWAKE (rch) && FIGHTING (rch) == NULL)
	    {
	      if (IS_PLAYER (ch) || IS_AFFECTED (ch, AFF_CHARM))
		{
		  if ((IS_PLAYER (rch) || IS_AFFECTED (rch, AFF_CHARM))
		      && is_same_group (ch, rch)
		      && IS_SET (rch->pcdata->act2, PLR_ASSIST)
		      && !IN_BATTLE (rch) && !IN_BATTLE (ch)
		      && FIGHTING (ch) != NULL && IS_MOB (FIGHTING (ch))
		      && rch->position == POSITION_STANDING && (!ch->ced || ch->hit > ch->ced->wimpy))
		    {
		      act ("You join the fight!", rch, NULL, rch, TO_CHAR);
		      act ("$n joins the fight!", rch, NULL, rch, TO_ROOM);
#ifdef OLD_FIGHT
		      multi_hit (rch, victim, TYPE_UNDEFINED);
#endif
		      set_fighting (rch, victim);
		    }
		  continue;
		}
	      if (IS_MOB (rch) && !IS_AFFECTED (rch, AFF_CHARM))
		{
		  if (rch->pIndexData == ch->pIndexData)
		    {
		      CHAR_DATA *vch;
		      CHAR_DATA *target;
		      int number;
		      target = NULL;
		      number = 0;
		      for (vch = ch->in_room->more->people; vch;
			   vch = vch->next)
			{
			  if (can_see (rch, vch)
			      && is_same_group (vch, victim)
			      && number_range (0, number) == 0)
			    {
			      target = vch;
			      number++;
			    }
			}
		      if (target != NULL)
			{
			  if (LEVEL (rch) > 6 && FIGHTING (rch) == NULL
			      && can_see (rch, target))
			    {
			      check_fgt (rch);
			      rch->fgt->fighting = target;
			      NEW_POSITION (rch, POSITION_FIGHTING);
			      act ("$n CHARGES into battle!", rch, NULL, NULL, TO_ROOM);
#ifdef OLD_FIGHT
			      multi_hit (rch, target, TYPE_UNDEFINED);
#else
			      set_fighting (rch, target);
#endif
			    }
			}
		    }
		}
	    }
	}
    }
  if (!flaggy && !pow.old_attack)
    {
      CHAR_DATA *rch_nxt;
      for (rch = f_first; rch != NULL; rch = rch_nxt)
	{
	  rch_nxt = rch->next_fighting;
	  check_fgt (rch);
	  if (rch->fgt->fighting)
	    check_fgt (rch->fgt->fighting);
	  if (FIGHTING (rch) == NULL
	      || (rch->fgt->upd && !rch->fgt->fighting->fgt->upd))
	    continue;
	  if (FIGHTING (FIGHTING (rch)) && FIGHTING (FIGHTING (rch)) == rch)
	    {
	      rch->fgt->upd = FALSE;
	      rch->fgt->fighting->fgt->upd = TRUE;
	      if (rch->fgt->attacker_turn
		  && rch->fgt->fighting->fgt->attacker_turn)
		rch->fgt->attacker_turn = FALSE;
	      else if (!rch->fgt->attacker_turn
		       && !rch->fgt->fighting->fgt->attacker_turn)
		rch->fgt->attacker_turn = TRUE;
	      else if (rch->fgt->attacker_turn)
		{
		  rch->fgt->attacker_turn = FALSE;
		  rch->fgt->fighting->fgt->attacker_turn = TRUE;
		}
	      else if (!rch->fgt->attacker_turn)
		{
		  rch->fgt->attacker_turn = TRUE;
		  rch->fgt->fighting->fgt->attacker_turn = FALSE;
		}
	    }
	}
    }
  return;
}

void
do_circle (CHAR_DATA * ch, char *argy)
{
  SINGLE_OBJECT *obj;
  int dt;
  SPELL_DATA *spl;
  DEFINE_COMMAND ("circle", do_circle, POSITION_FIGHTING, 0, LOG_NORMAL, "This command allows you to circle behind an opponent (who isn't fighting you), and thrust a dagger in his/her/its back during combat.")
    check_fgt (ch);
  if ((spl = skill_lookup ("Circle", -1)) == NULL)
    {
      send_to_char ("Huh?\n\r", ch);
      return;
    }
  if (ch->fgt->ears > 9)
    {
      send_to_char ("You can't circle this soon after giving or recieving a bash.\n\r", ch);
      return;
    }
  if (IS_MOB (ch))
    return;
  if (ch->pcdata->tickcounts > 2)
    {
      send_to_char ("You are off-balance from your last maneuver; you can't circle at the moment.\n\r", ch);
      return;
    }
  if (ch->wait > 1)
    return;
  if (ch->position != POSITION_FIGHTING)
    return;
  if (FIGHTING (ch) == NULL)
    return;
  if (FIGHTING (FIGHTING (ch)) == NULL)
    return;
  if (FIGHTING (FIGHTING (ch)) == ch)
    {
      send_to_char ("You can't circle around behind the mob who is fighting you!\n\r",ch);
      return;
    }
  if (ch->pcdata->learned[gsn_circle] < 2)
    {
      send_to_char ("You do not know how to circle an opponent!\n\r", ch);
      return;
    }
  if ((obj = get_item_held (ch, ITEM_WEAPON)) == NULL)
    {
      send_to_char ("You need to wield a weapon.\n\r", ch);
      return;
    }
  dt = ((I_WEAPON *) obj->more)->attack_type;
  if (attack_table[dt].hit_type != TYPE_PIERCE)
    {
      send_to_char ("You need to wield a piercing weapon.\n\r", ch);
      return;
    }
  if (FIGHTING (ch)->position == POSITION_GROUNDFIGHTING)
    {
      send_to_char ("That person is entangled in mortal combat on the ground!!\n\r", ch);
      return;
    }
  check_ced (ch);
  check_ced (FIGHTING (ch));
  if (!(IS_IN_MELEE (ch, FIGHTING (ch))))
    {
      send_to_char ("You aren't on the front lines of that battle!\n\r", ch);
      return;
    }
  send_to_char ("You attempt to circle around your victim...\n\r", ch);
  ch->pcdata->tickcounts = spl->mana;	/*13; */
  NEW_POSITION (ch, POSITION_CIRCLE);
  return;
}

void
do_actual_circle (CHAR_DATA * ch, char *argy, bool used_flash)
{
  SINGLE_OBJECT *obj;
  int dt;
  CHAR_DATA *c;
  DESCRIPTOR_DATA *d;
  SPELL_DATA *spl;
  if (IS_MOB (ch))
    return;
  if (ch->wait > 1)
    return;
  if (FIGHTING (ch) == NULL)
    return;
  if (FIGHTING (FIGHTING (ch)) == NULL)
    return;
  if (ch->position != POSITION_CIRCLE && !used_flash)
    return;
  NEW_POSITION (ch, POSITION_STANDING);
  if (IS_PLAYER (ch) && number_range(1,5) == 3)
   {
     skill_gain (ch, gsn_circle, TRUE);
     skill_gain (ch, gsn_backstab, TRUE);
   }
  if ((spl = skill_lookup ("Circle", -1)) == NULL)
    {
      send_to_char ("Huh?\n\r", ch);
      return;
    }
  if (FIGHTING (FIGHTING (ch)) == ch && !used_flash)
    {
      send_to_char ("You can't circle around behind the mob who is fighting you!\n\r", ch);
      return;
    }
  if (ch->pcdata->learned[gsn_circle] < 2)
    {
      send_to_char ("You do not know how to circle an opponent!\n\r", ch);
      return;
    }
  if ((obj = get_item_held (ch, ITEM_WEAPON)) == NULL)
    {
      send_to_char ("You need to wield a weapon.\n\r", ch);
      return;
    }
  dt = ((I_WEAPON *) obj->more)->attack_type;
  if (attack_table[dt].hit_type != TYPE_PIERCE)
    {
      send_to_char ("You need to wield a piercing weapon.\n\r", ch);
      return;
    }
  if (FIGHTING (ch)->position == POSITION_GROUNDFIGHTING)
    {
      send_to_char ("That person is entangled in mortal combat on the ground!!\n\r", ch);
      return;
    }
  for (d = descriptor_list; d != NULL; d = d->next)
    {
      if (!d->character || d->connected != CON_PLAYING)
	continue;
      c = d->character;
      if (FIGHTING (c) == FIGHTING (ch) && c->position == POSITION_CIRCLE && !used_flash)
	{
	  act ("$B$4You bump into $N, who is also trying to circle at the same time!", ch, NULL, c, TO_CHAR);
	  act ("$B$4$n bumps into $N, who are both trying to circle at the same time!  What fools!", ch, NULL, c, TO_NOTVICT);
	  act ("$B$4$n bumps into you as you both crowd behind the same mob, trying to circle!", ch, NULL, c, TO_VICT);
	  NEW_POSITION (c, POSITION_FIGHTING);
	  free_temp (c);
	  SUBHIT (c, 4);
	  SUBHIT (ch, 5);
	  return;
	}
    }
  if (number_range (0, 110) < ch->pcdata->learned[gsn_circle])
    {
      set_fighting (FIGHTING (ch), ch);
      set_fighting (ch, FIGHTING (ch));
      if (IS_PLAYER (ch))
	ch->pcdata->tickcounts = spl->casting_time;
      multi_hit (ch, FIGHTING (ch), gsn_circle);
    }
  else
    send_to_char ("You failed to circle your opponent.\n\r", ch);
      if (number_range (1,5)==1)
    skill_gain (ch, gsn_circle, TRUE);
      if (number_range (1,5)==3)
    skill_gain (ch, gsn_backstab, TRUE);
      if (number_range (1,3)==2)
    skill_gain (ch, gsn_pierce, TRUE);
  return;
}

void
check_pkill (CHAR_DATA * ch, CHAR_DATA * victim)
{
  if (CHALLENGE (ch) == 10 || (IN_BATTLE (ch)))
    return;
  return;
}

void
do_flash (CHAR_DATA * ch, char *argy)
{
  char arg[SML_LENGTH];
  CHAR_DATA *victim;
  I_TOOL *pow;
  char *argg;
  SINGLE_OBJECT *powder;
  DEFINE_COMMAND ("flash", do_flash, POSITION_FIGHTING, 0, LOG_NORMAL,"This command allows a thief to flash behind someone and circle them.")
if (ch->pcdata->learned[gsn_flash] < 1)
    {
      send_to_char ("Huh?\n\r", ch);
      return;
    }
    if (!FIGHTING (ch))
    {
      send_to_char ("You must be fighting to flash.\n\r", ch);
      return;
    }
  argg = str_dup (argy);
  one_argy (argg, arg);
  if ((victim = FIGHTING(ch)) == NULL)
    {
      send_to_char("You must be fighting someone to flash behind them!\n\r", ch);
      return;
    }
  if (ch->fgt->ears > 9)
    {
      send_to_char ("You can't flash this soon after giving or recieving a bash.\n\r", ch);
      return;
    }

  if (ch->position == POSITION_GROUNDFIGHTING)
    {
      send_to_char ("You can't flash while groundfighting!\n\r", ch);
      return;
    }
  if (ch->position == POSITION_BASHED)
    {
      send_to_char ("You can't flash when you have been bashed!\n\r", ch);
      return;
    }

  if (FIGHTING (ch) == NULL)
    {
      send_to_char ("You can't begin a fight with a flash; you must be already fighting!\n\r", ch);
      return;
    }
  powder = get_item_held(ch, ITEM_TOOL);
  if (powder == NULL)
    {
      send_to_char("You must be holding flash powder to do this!\n\r",ch);
      return;
    }

  SUBMOVE(ch, 40);
  pow = (I_TOOL *) powder->more;
  if (use_tool (powder, TOOL_FLASH_POWDER))
    {
      skill_gain(ch, gsn_flash, TRUE);
      if (number_percent() > ch->pcdata->learned[gsn_flash] - 35)
         {
           act ("$R$7You disappear in a bright $B$6FLASH$R$7, but reappear in the same spot.", ch, NULL, victim, TO_CHAR);
           act ("$R$7$n disappears in a bright $B$6FLASH$R$7, but reappears in the same spot.", ch, NULL, victim, TO_VICT);
           act ("$R$7$n disappears in a bright $B$6FLASH$R$7, but reappears in the same spot.", ch, NULL, victim, TO_NOTVICT);
           WAIT_STATE(ch, 3*PULSE_VIOLENCE);
if (((I_TOOL *) powder->more)->uses < 1)
    {
      send_to_char ("The bag is empty, and magically disappears!\n\r", ch);
      free_it (powder);
     }
           return;
         }
      else
         {
           act ("$R$7You disappear in a bright $B$6FLASH$R$7, and reappear right behind $N!", ch, NULL, victim, TO_CHAR);
           act ("$R$7$n disappears in a bright $B$6FLASH$R$7, and reappears behind YOU!", ch, NULL, victim, TO_VICT);
           act ("$R$7$n disappears in a bright $B$6FLASH$R$7, and reappears behind $N!", ch, NULL, victim, TO_NOTVICT);
           do_actual_circle(ch, argy, TRUE);
           WAIT_STATE(ch, 8*PULSE_VIOLENCE);

if (((I_TOOL *) powder->more)->uses < 1)
    {
      send_to_char ("The bag is empty, and magically disappears!\n\r", ch);
      free_it (powder);
     }
           return;
         }
     }
   else
     {
       send_to_char("You need flash powder to do this.\n\r", ch);
       return;
      }
  return;
}

void
multi_hit (CHAR_DATA * ch, CHAR_DATA * victim, int dt)
{
  int chance;
  int gmembers;
  bool cons;
  bool castsp = FALSE;
  bool second;
  SINGLE_OBJECT *action_object = NULL;
  CHAR_DATA *vch;
  SINGLE_OBJECT *wield;
  cons = FALSE;
  second = FALSE;
  CHECK_CHAR (ch);
  CHECK_CHAR (victim);
  check_fgt (ch);
  check_fgt (victim);
  using_weapon = FALSE;
  //using_shield = FALSE;
  /* Not in same room */
  if (ch->in_room != victim->in_room)
    {
      ch->fgt->fighting = NULL;
      victim->fgt->fighting = NULL;
      ch->position = POSITION_STANDING;
      victim->position = POSITION_STANDING;
      return;
    }
  if (dt > 19000)
    {
      dt -= 20000;		/*second=TRUE; */
    }
  if (dt > 8000)
    {
      dt -= 10000;
      cons_damage = 0;
      cons = TRUE;
    }
  if (!cons)
    {
      /*AM I JUST CONSIDERING THE BATTLE? */
      check_ced (ch);
      check_ced (victim);
      add_to_fighting_list (ch);
      add_to_fighting_list (victim);
      if (!FIGHTING (victim))
	{
	  closeto = TRUE;
	  set_fighting (victim, ch);
	}
      if (!FIGHTING (ch))
	{
	  closeto = TRUE;
	  set_fighting (ch, victim);
	}
      closeto = FALSE;

      if (IS_MOB (victim) && victim && IS_SET (victim->act, ACT_UNTOUCHABLE))
	{
	  send_to_char ("Your physical attacks don't seem to hurt this creature!\n\r", ch);
	  return;
	}
      if (ch->position == POSITION_SLEEPING)
	return;
      if (victim == NULL)
	return;
      if (victim->in_room == NULL || ch->in_room == NULL)
	return;
      /* Kill self capabilities here */
      if (pow.can_fight_self)
	{
	  if (IS_PLAYER (ch) && ch->desc != NULL && FIGHTING (ch) == NULL)
	    {
	      //write_to_descriptor2 (ch->desc, "\n\r", 2);
	      ch->fgt->fighting = victim;
	    }
	  else
	    {
	      check_fgt (ch);
	      ch->fgt->fighting = victim;
	    }
	}
      if (FIGHTING (FIGHTING (ch)) == NULL)
	{
	  ch->ced->attackers[0] = victim;
	  victim->ced->attackers[0] = ch;
	}
      if (ch == FIGHTING (victim) && !(IS_IN_MELEE (ch, victim)))
	{
	  /*victim is being hit, but not in melee?? */
	  victim->ced->attackers[0] = ch;	/*Set him in position 1 attacking */
	}
      for (vch = victim->in_room->more->people; vch != NULL;
	   vch = vch->next_in_room)
	{
	  if (FIGHTING (vch) == victim)
	    {
	      check_ced (vch);
	      if (!IS_IN_MELEE (vch, victim))
		{
		  for (gmembers = 0; gmembers < 4; gmembers++)
		    {
		      if (victim->ced->attackers[gmembers] == NULL ||
			  victim->ced->attackers[gmembers]->in_room !=
			  victim->in_room)
			{
			  victim->ced->attackers[gmembers] = vch;
			  break;
			}
		    }
		}
	    }
	}
      if (dt != gsn_backstab && !IS_IN_MELEE (ch, victim))
	{
	  send_to_char ("\x1B[1;35m<-> \x1B[0;35(\x1B[1;32mYou are not within melee range!!\x1B[0;35m) \x1B[1;35m<->\x1B[37;0m\n\r", ch);
	  return;
	}
      if (ch->position <= POSITION_STUNNED && ch->position >= 0)
	return;
      if (victim->position == POSITION_DEAD)
	{
	  stop_fighting (ch, TRUE);
	  ch->wait = 0;
	  stop_fighting (victim, TRUE);
	}
      if (is_safe (ch, victim))
	{
	  return;
	}
      if (!second)
	{
	  if (ch->fgt->combat_delay_count <= -5)
	    {
	      ch->fgt->combat_delay_count--;
	      if (ch->fgt->combat_delay_count < -8)
		ch->fgt->combat_delay_count = -1;
	    }
	  if (ch->fgt->combat_delay_count > 0)
	    {
	      ch->fgt->combat_delay_count--;
	      return;
	    }
	  if (ch->position != POSITION_BASHED
	      && ch->fgt->combat_delay_count == 0)
	    {
	      ch->fgt->combat_delay_count = -1;
	      send_to_char ("You scramble to your feet!\n\r", ch);
	      act ("$n stands up quickly, trying not to get whacked again!",
		   ch, NULL, NULL, TO_ROOM);
	      NEW_POSITION (ch, POSITION_FIGHTING);
	    }
	  if (IS_MOB (ch) && ch->position == POSITION_BASHED
	      && ch->fgt->combat_delay_count < 1)
	    {
	      ch->fgt->combat_delay_count = -5;
	      send_to_char ("You scramble to your feet!\n\r", ch);
	      act ("$n stands up, trying not to get bashed down again!", ch,
		   NULL, NULL, TO_ROOM);
	      NEW_POSITION (ch, POSITION_FIGHTING);
	    }
	  if (ch->position == POSITION_BASHED)
	    {
	      update_pos (ch);
	      return;
	    }
	  if (ch->fgt->combat_delay_count < 1
	      && ch->position == POSITION_STANDING)
	    {
	      NEW_POSITION (ch, POSITION_FIGHTING);
	    }
	  if (number_range (1, 40) == 3 && RIDING (ch) == NULL && ch->hit > 15
	      && victim->hit > 15 && !IS_AFFECTED (ch, AFF_FLYING)
	      && can_groundfight (ch) && can_groundfight (victim)
	      && (ch->hit * (double) 1.7 < ch->max_hit
		  || victim->hit * (double) 1.7 < victim->max_hit))
	    {
	      if (number_range (1, 2) == 1
		  || !IS_SET (ch->ced->fight_ops, F_GROUND)
		  || !IS_SET (victim->ced->fight_ops, F_GROUND)
		  || LEVEL (ch) < 10 || LEVEL (victim) < 10
		  || victim->position == POSITION_GROUNDFIGHTING
		  || ch->position == POSITION_GROUNDFIGHTING
		  || FIGHTING (victim) != ch)
		{
		  act ("You slip on a pool of blood that has formed on the ground here!\n\rYou hit your head hard as your face meets the ground!",
		     ch, NULL, victim, TO_CHAR);
		  act ("$n slips on a pool of blood and falls on $s face!",
		       ch, NULL, victim, TO_ROOM);
		  SUBHIT (ch, (7 + (UMIN (10, LEVEL (ch) / 7))));
		  WAIT_STATE (ch, PULSE_VIOLENCE);
		  return;
		}
	      else
		{
		  act ("You slip on some blood and $N falls on top of you!",
		       ch, NULL, victim, TO_CHAR);
		  act ("$n slips on some blood and drags $N down too!", ch,
		       NULL, victim, TO_NOTVICT);
		  act ("$n slips on a pool of blood on the ground, but as $n\n\rgoes down, $n grabs you, taking you down too!",
		     ch, NULL, victim, TO_VICT);
		  act ("The fight is taken to the ground!", ch, NULL, victim,
		       TO_ROOM);
		  act ("The fight is taken to the ground!", ch, NULL, victim,
		       TO_CHAR);
		  NEW_POSITION (ch, POSITION_GROUNDFIGHTING);
		  NEW_POSITION (victim, POSITION_GROUNDFIGHTING);
		  WAIT_STATE (ch, PULSE_VIOLENCE);
		  WAIT_STATE (victim, PULSE_VIOLENCE);
		  return;
		}
	    }
	}
      /*End if !second */
    }
  /*End is considering */
  if (victim)
    {
      SINGLE_OBJECT *o;
      for (o = victim->carrying; o != NULL; o = o->next_content)
	{
	  if (o->wear_loc == WEAR_HOLD_1
	      && o->pIndexData->item_type == ITEM_WEAPON)
	    {
	      using_weapon = TRUE;
	    }
	  if (o->wear_loc == WEAR_HOLD_2
	      && o->pIndexData->item_type == ITEM_WEAPON)
	    {
	      using_weapon = TRUE;
	    }
	  /*if (o->wear_loc == WEAR_SHIELD)
	    {
	      using_shield = TRUE;
	    }*/
	}
    }

  if (dt == gsn_backstab || dt == gsn_circle)
    {
      SINGLE_OBJECT *wield1;
      SINGLE_OBJECT *wield2;
      wield1 = get_eq_char (ch, WEAR_HOLD_1);
      wield2 = get_eq_char (ch, WEAR_HOLD_2);
      if (wield1 && wield1->pIndexData->item_type == ITEM_WEAPON
	  && IS_SET (wield1->extra_flags, ITEM_PIERCE))
	{
	  one_hit (ch, victim, dt, wield1, cons);
	}
      else if (wield2 && wield2->pIndexData->item_type == ITEM_WEAPON
	       && IS_SET (wield2->extra_flags, ITEM_PIERCE))
	{
	  one_hit (ch, victim, dt, wield2, cons);
	}
      return;
    }

  if (dt == gsn_dual_backstab)
    {
      SINGLE_OBJECT *wield3;	// Looking at second slot only.
      wield3 = get_eq_char (ch, WEAR_HOLD_2);
      if (wield3 && wield3->pIndexData->item_type == ITEM_WEAPON
	  && IS_SET (wield3->extra_flags, ITEM_PIERCE))
	{
	  one_hit (ch, victim, dt, wield3, cons);
	}
      return;
    }

  if (dt == TYPE_UNDEFINED)
    {
      SINGLE_OBJECT *o;
      SINGLE_OBJECT *wield1 = NULL;
      SINGLE_OBJECT *wield2 = NULL;
      if (IS_MOB (ch) && (ch->pIndexData->mobtype == MOB_DUMMY))
	return;

      for (o = ch->carrying; o != NULL; o = o->next_content)
	{
	  if (!cons && !action_object)
	    {
	      if (o->wear_loc > WEAR_NONE && o->wear_loc != WEAR_BELT_1 &&
		  o->wear_loc != WEAR_BELT_2 && o->pIndexData->values_2[0] > 0
		  && number_range (1, 1000) < o->pIndexData->values_2[1])
		{
		  action_object = o;
		}
	      if (o->wear_loc > WEAR_NONE && o->wear_loc != WEAR_BELT_1 &&
		  o->wear_loc != WEAR_BELT_2 && o->pIndexData->values_2[2] > 0
		  && number_range (1, 1000) < o->pIndexData->values_2[3])
		{
		  action_object = o;
		  castsp = TRUE;
		}
	    }
	  if (o->wear_loc == WEAR_HOLD_1
	      && o->pIndexData->item_type == ITEM_WEAPON)
	    {
	      wield1 = o;
	    }
	  if (o->wear_loc == WEAR_HOLD_2
	      && o->pIndexData->item_type == ITEM_WEAPON)
	    {
	      wield2 = o;
	    }
	}


      if (IS_MOB (victim) && (victim->pIndexData->mobtype == MOB_GHOST))
	{
	  bool can_hit_it;
	  can_hit_it = FALSE;
	  if (wield1 && (IS_OBJ_STAT (wield1, ITEM_GLOW) || IS_OBJ_STAT (wield1, ITEM_HUM)))
	    can_hit_it = TRUE;
	  if (wield2 && (IS_OBJ_STAT (wield2, ITEM_GLOW) || IS_OBJ_STAT (wield2, ITEM_HUM)))
	    can_hit_it = TRUE;
	  if (!can_hit_it)
	    {
	      if (!cons)
		{
		  char buf[STD_LENGTH];
		  sprintf (buf, "\x1B[1;33m%s tries to hit you, but the attack goes right through you!\x1B[0;37m\n\r", cap (rNAME (ch, victim)));
		  send_to_char (buf, victim);
		  sprintf (buf, "\x1B[1;31mYour attack goes right through %s!\x1B[0;37m\n\r", cap (rNAME (victim, ch)));
		  send_to_char (buf, ch);
		  return;
		}
	    }
	}
      if (wield1 && IS_AFFECTED_EXTRA (ch, AFF_SPEED))
	{
	  if (number_range (1, 100) < ((get_curr_dex (ch) - 4) * 2))
	    {
	      one_hit (ch, victim, dt, wield1, cons);
	    }
	}
      if (wield1)
	{
    one_hit (ch, victim, dt, wield1, cons);
    if (wield2 && ((IS_PLAYER (ch) && ch->pcdata->learned[gsn_dual_wield] > number_range (0, 148)) 
        || ((IS_MOB (ch)) && number_range (1, 120) < LEVEL (ch))))
	    {
	      if (wield2)
		{
		  one_hit (ch, victim, dt, wield2, cons);
		}
	    }
	}
      else if (wield2 != NULL)
	{
	  one_hit (ch, victim, dt, wield2, cons);
	}
      else
	{
	  one_hit (ch, victim, TYPE_HIT, NULL, cons);
	}
      if (FIGHTING (ch) != victim || dt == gsn_backstab || dt == gsn_circle)
	return;
      wield = (wield1 ? wield1 : wield2);
      if (second)
	{
	  chance = 0;
	}
      else
	chance = IS_MOB (ch) ? LEVEL (ch) / 3 : ((ch->pcdata->learned[gsn_second_attack] / 4) + LEVEL (ch) / 5);
      if (number_percent () < chance)
	{
	  if (wield)
	    one_hit (ch, victim, dt, wield, cons);
	  else
	    one_hit (ch, victim, TYPE_HIT, NULL, cons);
	  if (IS_PLAYER (ch) && !cons)
	    skill_gain (ch, gsn_second_attack, TRUE);
            skill_gain (ch, gsn_dual_wield, TRUE);
            skill_gain (ch, gsn_dual_daggers, TRUE);
	  if (FIGHTING (ch) != victim)
	    return;
	}
      if (second)
	chance = 0;
      else
	chance = IS_MOB (ch) ? LEVEL (ch) / 6 : ch->pcdata->learned[gsn_third_attack] / 7;
      if (number_percent () < chance)
	{
	  if (wield)
	    one_hit (ch, victim, dt, wield, cons);
	  else
	    one_hit (ch, victim, TYPE_HIT, NULL, cons);
	  if (IS_PLAYER (ch) && !cons)
	    skill_gain (ch, gsn_third_attack, TRUE);
	  if (FIGHTING (ch) != victim)
	    return;
	}
      /* chance = IS_MOB(ch) ? LEVEL(ch) / 10 : 0;
         if ( number_percent( ) < chance )
         one_hit( ch, victim, dt, wield, cons ); */

    }
  if (victim->position == POSITION_RESTING
      || victim->position == POSITION_SLEEPING
      || victim->position == POSITION_STANDING)
    set_fighting (victim, ch);
  if (action_object)
    {
      SPELL_DATA *s;
      int spell_level;
      if (castsp)
	{
	  if ((s = skill_lookup (NULL, action_object->pIndexData->values_2[2])) != NULL)
	    {
	      if (s->spell_level <= LEVEL (ch))
		{
		  if (action_object->pIndexData->values_2[4] > 0)
		    spell_level = action_object->pIndexData->values_2[4];
		  else
		    spell_level = LEVEL (ch) / 2;
		  if (s->spell_type == TAR_CHAR_DEFENSIVE)
		    {
		      general_spell (s, spell_level, ch, ch);
		    }
		  else
		    {
		      general_spell (s, spell_level, ch, victim);
		    }
		}
	    }
	}
      if (!castsp)
	{
	  if (action_object->pIndexData->action_descr[0])
	    {
	      act (action_object->pIndexData->action_descr[0], ch,
		   action_object, victim, TO_CHAR);
	    }
	  if (action_object->pIndexData->action_descr[1])
	    {
	      act (action_object->pIndexData->action_descr[1], ch,
		   action_object, victim, TO_VICT);
	    }
	  if (action_object->pIndexData->action_descr[2])
	    {
	      act (action_object->pIndexData->action_descr[2], ch,
		   action_object, victim, TO_NOTVICT);
	    }
	  SUBHIT (victim, number_range (1, action_object->pIndexData->values_2[0]));
	  if (victim->hit < 1)
	    victim->hit = 1;
	}
    }
  using_weapon = FALSE;
  //using_shield = FALSE;
  return;
}

int
addoffense (CHAR_DATA * ch, SINGLE_OBJECT * obj)
{
  if (IS_MOB (ch))
    return 0;
  if (IS_SET (obj->extra_flags, ITEM_LANCE))
    {
      if (ch->pcdata->learned[gsn_lance] < 2)
	{
	  return -100;
	}
      else
	return (ch->pcdata->learned[gsn_lance] / 4);
    }
  if (IS_SET (obj->extra_flags, ITEM_PIERCE))
    return (ch->pcdata->learned[gsn_pierce] ==
	    -100 ? 0 : ch->pcdata->learned[gsn_pierce] / 4);
  if (IS_SET (obj->extra_flags, ITEM_CONCUSSION))
    return (ch->pcdata->learned[gsn_concussion] ==
	    -100 ? 0 : ch->pcdata->learned[gsn_concussion] / 5);
  if (IS_SET (obj->extra_flags, ITEM_WHIP))
    return (ch->pcdata->learned[gsn_whip] ==
	    -100 ? 0 : ch->pcdata->learned[gsn_whip] / 4);
  if (ch->pcdata->learned[gsn_slashing] == -100)
    return 0;
  if (IS_SET (obj->extra_flags, ITEM_HEAVY_SLASH))
    {
      if (get_curr_str (ch) < 11)
	return -3;
      if (get_curr_str (ch) < 16)
	return 0;
      if (get_curr_str (ch) < 18)
	return (ch->pcdata->learned[gsn_slashing] / 7);
      if (get_curr_str (ch) < 21)
	return (ch->pcdata->learned[gsn_slashing] / 6);
      if (get_curr_str (ch) < 24)
	return (ch->pcdata->learned[gsn_slashing] / 5);
      else
	return (ch->pcdata->learned[gsn_slashing] / 4);
    }
  if (IS_SET (obj->extra_flags, ITEM_LIGHT_SLASH))
    return ((ch->pcdata->learned[gsn_slashing] / 4) +
	    ((get_curr_str (ch) - 5) / 2));
  if (IS_SET (obj->extra_flags, ITEM_SLASHING))
    return (ch->pcdata->learned[gsn_slashing] / 5);
  return 0;
}


void
one_hit (CHAR_DATA * ch, CHAR_DATA * victim, int dt, SINGLE_OBJECT * obj, bool consid)
{
  SINGLE_OBJECT *wield;
  int to_hit;
  int bonushit;
  int max_absorb;
  int actual_absorb;
  int dam;
  int diceroll;
  int iWear;
  int hit_pos;
  max_absorb = 0;
  actual_absorb = 0;
  bonushit = 0;
  CHECK_CHAR (ch);
  CHECK_CHAR (victim);

  check_ced (ch);
  check_ced (victim);
  check_fgt (ch);
  check_fgt (victim);

  if (ch->in_room != victim->in_room)
  {
    ch->fgt->fighting = NULL;
    victim->fgt->fighting = NULL;
    ch->position = POSITION_STANDING;
    victim->position = POSITION_STANDING;
    return;
  }


  hit_pos = number_range (1, 4);
  if (victim && ch && victim->height > ch->height + 22
      && hit_pos == STRIKE_HEAD)
  {
    if (number_range (1, 2) == 2)
      hit_pos = STRIKE_LEGS;
    else
      hit_pos = STRIKE_BODY;
  }
  if (!consid)
  {
    /*If I'm considering, don't do the following */
    if (victim->position == POSITION_DEAD || ch->in_room != victim->in_room)
    {
      ch->fgt->fighting = NULL;
      victim->fgt->fighting = NULL;
      ch->wait = 0;
      return;
    }
    if (ch && victim && IS_MOB (ch) && IS_PLAYER (victim) &&
        (LEVEL (victim) > 5) && (!HUNTING (ch) || (HUNTING (ch) && !is_number (HUNTING (ch)))))

    {
      if (strlen (NAME (victim)) < 16)
      {
        if (HUNTING (ch) != NULL)
        {
          free_string (ch->fgt->hunting);
          ch->fgt->hunting = NULL;
        }
        ch->fgt->hunting = str_dup (NAME (victim));
      }
    }
    if (MOUNTED_BY (victim) == ch)
    {
      ch->fgt->riding = NULL;
      victim->fgt->mounted_by = NULL;
    }
  }
  /*End is consid */
  /*
   * Figure out the type of damage message.
   */
  wield = obj;
  if (wield && wield->pIndexData->item_type != ITEM_WEAPON)
    wield = NULL;
  if (dt == TYPE_UNDEFINED)
  {
    dt = TYPE_HIT;
    if (wield != NULL && wield->pIndexData->item_type == ITEM_WEAPON)
    {
      I_WEAPON *w = (I_WEAPON *) wield->more;
      {
        if (w->attack_type == 3 && number_range (1, 15) == 4)
          dt += 2;
        else if (w->attack_type == 11 && number_range (1, 15) == 5)
          dt += 1;
        else
          dt += w->attack_type;
      }
      if (IS_PLAYER (ch))
        bonushit = addoffense (ch, wield);
    }
  }

  to_hit = ((get_curr_dex (ch) + 20 + bonushit + GET_HITROLL (ch) +
        LEVEL (ch) / 5 +
        (IS_SET (ch->ced->fight_ops, F_AGGRESSIVE) ? 5 : 0)) -
      (LEVEL (victim) / 4 + (get_curr_dex (victim) - 5) +
       (!IS_SET (victim->ced->fight_ops, F_AGGRESSIVE) ? 8 : 0)));

  if (IS_MOB (ch))
    to_hit += ch->pIndexData->hitroll;
  if (HAS_ARMS_DEXTERITY_IMPLANT (ch))
    to_hit += number_range (5, 14);

  if (number_percent () >= to_hit && number_range (0, 9) >= 6 &&
      dt != gsn_backstab && dt != gsn_circle && IS_AWAKE (victim))
  {
    /* Shit! Missed! */
    if (!consid)
    {
      ghit_pos = hit_pos;
      damage (ch, victim, 0, dt);
      ghit_pos = 0;
    }
    else
      cons_damage += 0;
    return;
  }
  if (!consid && IS_PLAYER (ch) && wield != NULL
      && wield->pIndexData->item_type == ITEM_WEAPON)
  {
    if (IS_SET (wield->extra_flags, ITEM_LANCE))
      skill_gain (ch, gsn_lance, TRUE);
    if (IS_SET (wield->extra_flags, ITEM_PIERCE))
      skill_gain (ch, gsn_pierce, TRUE);
      skill_gain (ch, gsn_dual_daggers, TRUE);
      skill_gain (ch, gsn_dual_wield, TRUE);
    if (IS_SET (wield->extra_flags, ITEM_CONCUSSION))
      skill_gain (ch, gsn_concussion, TRUE);
      skill_gain (ch, gsn_dual_wield, TRUE);
    if (IS_SET (wield->extra_flags, ITEM_WHIP))
      skill_gain (ch, gsn_whip, TRUE);
      skill_gain (ch, gsn_dual_wield, TRUE);
    if (IS_SET (wield->extra_flags, ITEM_HEAVY_SLASH) ||
        IS_SET (wield->extra_flags, ITEM_LIGHT_SLASH) ||
        IS_SET (wield->extra_flags, ITEM_SLASHING))
      skill_gain (ch, gsn_slashing, TRUE);
      skill_gain (ch, gsn_dual_wield, TRUE);
  }
  diceroll = number_range (0, 19);
  dam = 0;
  if (IS_MOB (ch))
  {
#ifdef NEW_NEW
    if (wield != NULL && wield->pIndexData->item_type == ITEM_WEAPON)
    {
      dam = (dice (FIRSTDICE (wield), SECONDDICE (wield)) / 2) +
        (number_range (LEVEL (ch) / 3, (LEVEL (ch)) / 2));
    }
    else
    {
      dam = number_range ((LEVEL (ch) * 2) / 5, (LEVEL (ch) * 4) / 5);
    }
#else
    if (wield != NULL && wield->pIndexData->item_type == ITEM_WEAPON)
    {
      dam = (dice (FIRSTDICE (wield), SECONDDICE (wield)) / 2) +
        (number_range (LEVEL (ch) / 4, (LEVEL (ch)) / 2));
    }
    else
    {
      dam = number_range (LEVEL (ch) / 4, (LEVEL (ch)) / 2);
    }
#endif
    dam += ch->pIndexData->damroll;
    if (dam <= 0)
      dam = 1;
  }

  else
  {
    if (wield != NULL && wield->pIndexData->item_type == ITEM_WEAPON)
    {
      dam = dice (FIRSTDICE (wield), SECONDDICE (wield));
    }
    else
    {
      /*FISTS of HIGH-LEVS now do more damage */
      if (IS_SET (ch->ced->fight_ops, F_AGGRESSIVE))
      {
        if (LEVEL (ch) < 10)
          dam = number_range (1, 3);
        else if (LEVEL (ch) < 20)
          dam = number_range (1, 4);
        else if (LEVEL (ch) < 35)
          dam = number_range (1, 6);
        else if (LEVEL (ch) < 50)
          dam = number_range (1, 7);
        else if (LEVEL (ch) < 65)
          dam = number_range (2, 9);
        else if (LEVEL (ch) < 90)
          dam = number_range (3, 11);
        else
          dam = number_range (6, 14);
      }
      else
        dam = number_range (1, 3);
      if (HAS_HANDS_STRENGTH_IMPLANT (ch))
        dam += number_range (2, 5);
      if (HAS_BODY_STRENGTH_IMPLANT (ch))
        dam += number_range (1, 3);
    }
  }

  if (LEVEL (ch) > 8 && IS_SET (ch->ced->fight_ops, F_POWER))
    dam += 1;
//Kilith 06
  if (IS_PLAYER (ch) && is_member(ch, GUILD_WARRIOR) && is_member(ch, GUILD_TATICS) && is_member(ch, GUILD_RANGER))
    dam += (number_range (1, GET_DAMROLL (ch)) / 4);
  else
    dam += (number_range (1, GET_DAMROLL (ch)) / 5);
  if (!consid && IS_PLAYER (ch)
      && ch->pcdata->learned[gsn_enhanced_damage] > 0)
  {
    dam += (dam * ch->pcdata->learned[gsn_enhanced_damage]) / 400;
    if (number_range (1, 18) == 10)
      skill_gain (ch, gsn_enhanced_damage, TRUE);
  }
if (HAS_ARMS_STRENGTH_IMPLANT (ch))
  {
  dam += (number_range (1,2));
  }
if (HAS_BODY_STRENGTH_IMPLANT (ch))
  { 
  dam += (number_range (1,4));
  }
if (HAS_HANDS_STRENGTH_IMPLANT (ch))
  {
  dam += (number_range (1,2));
  }

  if (IS_SET (ch->ced->fight_ops, F_AGGRESSIVE) && number_range (1, 2) == 2)
    dam += 1;

  {
    int b4 = dam;
    int aftr;
    int dif;
    dam = new_damage (victim, dam, hit_pos, dt);
    aftr = dam;
    dif = b4 - aftr;
    if (!consid && IS_PLAYER (ch) && dif > 4 && number_range (1, 100) < ch->pcdata->learned[gsn_penetration])
    {
      if (number_range (1, 4) == 2)
        skill_gain (ch, gsn_penetration, TRUE);
      dif /= 2;
      dam += dif;
    }
  }

  if (!consid)
  {
    if (ch->position != POSITION_BASHED
        && victim->position != POSITION_BASHED && number_range (1, 7) == 4
        && IS_AFFECTED (ch, AFF_FLYING)
        && !IS_AFFECTED (victim, AFF_FLYING))
    {
      act ("$n swoops down and attacks $N!", ch, NULL, victim, TO_NOTVICT);
      act ("$n swoops down from the air and attacks you!", ch, NULL, victim, TO_VICT);
      act ("You swoop down from the air and attack $N!", ch, NULL, victim, TO_CHAR);
      dam += 2;
      if (number_range (1, 12) == 5)
      {
        if (number_range (1, 2) == 1)
        {
          act ("$N catches $n with an elbow while $e is approaching!", ch, NULL, victim, TO_NOTVICT);
          act ("You catch $n with an elbow as $e approaches!", ch, NULL, victim, TO_VICT);
          act ("$N smashes $S elbow into your face as you swoop down!", ch, NULL, victim, TO_CHAR);
        }
        else
        {
          act ("$N jumps out of the way and $n smashes into the ground!", ch, NULL, victim, TO_NOTVICT);
          act ("You jump out of the way and $n smashes into the ground!", ch, NULL, victim, TO_VICT);
          act ("$N steps out of the way and you smash into the ground!", ch, NULL, victim, TO_CHAR);
        }
        if (ch->hit > 7)
          SUBHIT (ch, 7);
        return;
      }
    }
    if (ch->position != POSITION_BASHED
        && victim->position != POSITION_BASHED && number_range (1, 11) == 4
        && !IS_AFFECTED (ch, AFF_FLYING)
        && IS_AFFECTED (victim, AFF_FLYING))
    {
      /*hitter not flying, victim flying */
      act ("$N quickly flies out of the way, dodging your attack!", ch, NULL, victim, TO_CHAR);
      act ("$N flies out of $n's attack path.", ch, NULL, victim, TO_NOTVICT);
      act ("You fly out of the path of $n's attack.", ch, NULL, victim, TO_VICT);
      return;
    }
    if (ch->position != POSITION_BASHED
        && victim->position != POSITION_BASHED && number_range (1, 34) == 6
        && IS_AFFECTED (ch, AFF_FLYING) && IS_AFFECTED (victim, AFF_FLYING))
    {
      /*both hitter and victim flying */
      act ("$N and $n get tangled and smash into the ground!", ch, NULL, victim, TO_NOTVICT);
      act ("You smash into the ground as you struggle with $n!", ch, NULL, victim, TO_VICT);
      act ("You smash into the ground as you struggle with $N!", ch, NULL, victim, TO_CHAR);
      if (ch->hit > 10)
      {
        SUBHIT (ch, 9);
        SUBHIT (victim, 9);
      }
      return;
    }
    if (!IS_AWAKE (victim))
    {
      if (dam < 2)
        dam = 2;
      dam = (dam * 2);
    }
    if (victim->position == POSITION_RESTING)
    {
      if (dam < 2)
        dam = 2;
      dam = (dam * 3) / 2;
    }
  }

  /*end if !consid */
//Follows Circle Damage and BS damage
  if (dt == gsn_circle)
  {
   int idam = 0;
   if (wield != NULL && wield->pIndexData->item_type == ITEM_WEAPON)
       {
      idam = dice (FIRSTDICE (wield), SECONDDICE (wield));
       }
    int remort = (ch->pcdata->remort_times);
    int level = LEVEL(ch);
    int dex = get_curr_dex(ch);
    int str = get_curr_str(ch);
    int tguild = (is_member(ch, GUILD_THIEFG));
    int aguild = (is_member(ch, GUILD_ASSASSIN));
    int mguild = (is_member(ch, GUILD_MARAUDER));
    int pbs = (ch->pcdata->plus_bs/2);
    int sleeping = (!IS_AWAKE(victim));

    dam = ((idam * 2) 
        + (pbs) 
	+ (str * tguild)
        + (str/2 * aguild)
        + (remort * 2)                      // 9 = max +18
        + (dex * 2)                        //35 = max +40 
        + ((str - 10) * 2)                  //28 = max +36
        + (35 * tguild)                     //   = max +20
        + (number_range(10, 45 * aguild))   //   = max +35
        + (number_range(1, 20 * mguild)) 
        + (level / 3)                       //90 = max +23
        + number_range(1, 15));             //   = max +15
    dam = dam * (sleeping + 1);
    if (dam > 250) dam = (250 + number_range (5, (dex+15)));
   if (HAS_ARMS_DEXTERITY_IMPLANT (ch))
    {
    dam += 35;
    }
  }

  if (dt == gsn_backstab || dt == gsn_dual_backstab)
  {
//BSDAM Backstab Damage Formula
   int idam = 0;
   if (wield != NULL && wield->pIndexData->item_type == ITEM_WEAPON)
       {
      idam = dice (FIRSTDICE (wield), SECONDDICE (wield));
       }
    int remort = (ch->pcdata->remort_times);
    int level = LEVEL(ch);
    int dex = get_curr_dex(ch);
    int str = get_curr_str(ch);
    int tguild = (is_member(ch, GUILD_THIEFG));
    int aguild = (is_member(ch, GUILD_ASSASSIN));
    int mguild = (is_member(ch, GUILD_MARAUDER));
    int pbs = (ch->pcdata->plus_bs);
    int dmr = GET_DAMROLL (ch);
    int sleeping = (!IS_AWAKE(victim));

    dam = ((idam * 4)
        + (pbs * 2)
        + ((str * 2) * tguild)
        + (dex * aguild)
        + (dmr/4)
        + (remort * 2)
        + ((dex - 10) * 2 + dex) 
        + ((str * 3) / 2) 
        + (number_range(10, 30 * tguild)) 
        + (number_range(10, 25 * aguild))
        + (number_range(1, 20 * mguild)) 
        + level + 25 
        + number_range(10, 35));
    dam = dam * ((sleeping / 2) + 1);
   if (HAS_ARMS_DEXTERITY_IMPLANT (ch))
    {
    dam += 50;
    }
  }
  if (dt == gsn_dual_backstab)
    dam /= 3;			// Reduce damage for a dual backstab. Kenor '03

  if (dam <= 0)
    dam = 1;
  if (!consid)
  {
    if (number_range (1, 5) == 1 && ch->move > 0)
    {
      SUBMOVE (ch, 1);
      upd_mps (ch);
    }
  }

  ghit_pos = hit_pos;
  /*end if !consid */
  if (wield && ((I_WEAPON *) wield->more)->damage_p < 3)
  {
    if (!consid)
      damage (ch, victim, dam / 10, dt);
    else
      cons_damage += 1;
    if (!consid)
      send_to_char ("Your weapon is so bashed up it hardly does any damage!\n\r", ch);
  }

  else if (wield && ((I_WEAPON *) wield->more)->damage_p < 15)
  {
    if (!consid)
      damage (ch, victim, dam / 3, dt);
    else
      cons_damage += (dam / 3);
  }

  else if (wield && ((I_WEAPON *) wield->more)->damage_p < 44)
  {
    if (!consid)
      damage (ch, victim, dam / 2, dt);
    else
      cons_damage += (dam / 2);
  }

  else if (wield && ((I_WEAPON *) wield->more)->damage_p < 73)
  {
    if (dam > 31)
      dam -= 7;
    else if (dam > 22)
      dam -= 6;
    else if (dam > 15)
      dam -= 4;
    else if (dam > 9)
      dam -= 2;
    else if (dam > 4)
      dam -= 1;
    if (!consid)
      damage (ch, victim, dam, dt);
    else
      cons_damage += dam;
  }

  else if (!consid)
  {
    hitop = TRUE;
    damage (ch, victim, dam, dt);
  }
  else
    cons_damage += dam;
  if (victim->data_type == 50)
    return;
  if (!consid)
    if (FIGHTING (ch) == NULL)
      return;
  if (!consid)
  {
    if (wield && (((I_WEAPON *) wield->more)->strength != 0) &&
        (dam >= ((I_WEAPON *) wield->more)->strength))
      if (number_range (1, 12) == 5)
      {
        --((I_WEAPON *) wield->more)->damage_p;
        if (((I_WEAPON *) wield->more)->damage_p == 1)
        {
          send_to_char ("--->ACK! Your weapon just broke!\n\r", ch);
          unequip_char (ch, wield);
          free_it (wield);
          return;
        }
        else if (number_range (1, 4) == 3)	/*only display occasionally */
          send_to_char
            ("Your weapon is damaged slightly by your attack!\n\r", ch);
      }

    ghit_pos = 0;

    if (dam >= 4 && number_range (1, 3) == 1
        && (wield = get_eq_char (victim, (iWear = number_range (WEAR_NONE + 1, MAX_WEAR)))) != NULL
        && wield->pIndexData->item_type == ITEM_ARMOR
        && wield->wear_loc != -1
        && ((I_ARMOR *) wield->more)->max_condition)
    {
      int localle;
      char blah[500];
      char *tmp;
      if (hit_pos == STRIKE_BODY)
      {
        if (wield->wear_loc != WEAR_BODY
            && wield->wear_loc != WEAR_WAIST
            && wield->wear_loc != WEAR_SHIELD
            && wield->wear_loc != WEAR_ABOUT
            && wield->wear_loc != WEAR_BACK)
          goto brkl;
      }
      if (hit_pos == STRIKE_HEAD)
      {
        if (wield->wear_loc != WEAR_HEAD
            && wield->wear_loc != WEAR_NECK_1
            && wield->wear_loc != WEAR_NECK_2
            && wield->wear_loc != WEAR_FACE
            && wield->wear_loc != WEAR_SHIELD
            && wield->wear_loc != WEAR_EAR_1
            && wield->wear_loc != WEAR_EAR_2)
          goto brkl;
      }
      if (hit_pos == STRIKE_ARMS)
      {
        if (wield->wear_loc != WEAR_ARMS
            && wield->wear_loc != WEAR_HANDS
            && wield->wear_loc != WEAR_SHIELD
            && wield->wear_loc != WEAR_WRIST_L
            && wield->wear_loc != WEAR_WRIST_R
            && wield->wear_loc != WEAR_SHOULDER_L
            && wield->wear_loc != WEAR_SHOULDER_R
            && wield->wear_loc != WEAR_ELBOW_1
            && wield->wear_loc != WEAR_ELBOW_2)
          goto brkl;
      }
      if (hit_pos == STRIKE_LEGS)
      {
        if (wield->wear_loc != WEAR_LEGS 
            && wield->wear_loc != WEAR_FEET
            && wield->wear_loc != WEAR_ANKLE_1
            && wield->wear_loc != WEAR_ANKLE_2
            && wield->wear_loc != WEAR_KNEE_1
            && wield->wear_loc != WEAR_KNEE_2)
          goto brkl;
      }
      if (wield->pIndexData->short_descr[0] != '\0')
      {
        for (tmp = wield->pIndexData->short_descr; *tmp != '\0'; tmp++)
        {
        };
        if (*(tmp - 1) == 's')
          sprintf (blah, "$p are damaged.");
        else
          sprintf (blah, "$p is damaged.");
        act (blah, victim, wield, NULL, TO_CHAR);
        localle = wield->wear_loc;
        unequip_char (victim, wield);
        --((I_ARMOR *) wield->more)->condition_now;
        equip_char (victim, wield, localle);
        if (((I_ARMOR *) wield->more)->condition_now <= 0)
        {
          if (*tmp == 's')
            sprintf (blah,
                "The damage was so powerful, $p fall apart!");
          else
            sprintf (blah,
                "The damage was so powerful, $p falls apart!");
          act (blah, victim, wield, NULL, TO_CHAR);
          unequip_char (victim, wield);
          free_it (wield);
          return;
        }
      }
    }
brkl:
    if (wield)
    {
    };
  }

  /*end if !consid */
  return;
}

bool
can_trip (CHAR_DATA * ch)
{
  if (number_range (1, 4) != 1)
    return FALSE;
  if (IS_PLAYER (ch))
    return TRUE;
  if (ch->pIndexData->mobtype == MOB_HUMAN
      || ch->pIndexData->mobtype == MOB_DWARF
      || ch->pIndexData->mobtype == MOB_ELF
      || ch->pIndexData->mobtype == MOB_GIANT
      || ch->pIndexData->mobtype == MOB_DRAGON)
    return TRUE;
  return FALSE;
}

bool
bashable (CHAR_DATA * ch)
{
  if (IS_PLAYER (ch))
    return TRUE;
  if (ch->pIndexData->mobtype == MOB_GHOST)
    return FALSE;
  if (ch->pIndexData->mobtype == MOB_DRAGON)
    return FALSE;
  return TRUE;
}

int
kickable (CHAR_DATA * ch)
{
  if (IS_PLAYER (ch))
    return 1;
  if (ch->pIndexData->mobtype == MOB_GHOST)
    return 0;
  if (IS_AFFECTED (ch, AFF_FLYING))
    return 2;
  return 1;
}

bool
pkill (CHAR_DATA * ch, CHAR_DATA * victim)
{
  return TRUE;
}

bool
is_using_sword (CHAR_DATA * ch)
{
  SINGLE_OBJECT *obj;
  if (!ch)
    return FALSE;
  for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
      if (obj->wear_loc != WEAR_HOLD_1 && obj->wear_loc != WEAR_HOLD_2)
	continue;
      if (obj->pIndexData->item_type != ITEM_WEAPON)
	continue;
      if (((I_WEAPON *) obj->more)->attack_type == 3)
	return TRUE;
    }

  return FALSE;
}

/*
   * Inflict damage from a hit.
 */
void
damage (CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt)
{
  bool display_pr;
  CHECK_CHAR (ch);
  CHECK_CHAR (victim);
  check_fgt (ch);
  check_fgt (victim);
  if (!ch->in_room || !victim->in_room)
    return;
  if (ch->in_room != victim->in_room)
    {
      ch->fgt->fighting = NULL;
      victim->fgt->fighting = NULL;
      ch->position = POSITION_STANDING;
      victim->position = POSITION_STANDING;
      return;
    }

  add_to_fighting_list (ch);
  add_to_fighting_list (victim);
  if (!FIGHTING (ch))
    set_fighting (ch, victim);
  if (!FIGHTING (victim))
    set_fighting (victim, ch);
  if (dt > 19000)
    {
      dt -= 20000;
      display_pr = FALSE;
    }

  else
    display_pr = TRUE;
  if (victim == NULL)
    {
      ch->fgt->fighting = NULL;
      return;
    }

  if (!victim->in_room || !victim->in_room->area ||
      ((IS_PLAYER (ch) && LEVEL (ch) > 99 && !IS_REAL_GOD (ch))
       && victim->in_room && victim->in_room->area
       && victim->in_room->area->open == 1))
    {
      ch->fgt->fighting = NULL;
      victim->fgt->fighting = NULL;
      return;
    }

  if (victim->position == POSITION_DEAD)
    {
      ch->wait = 0;
      stop_fighting (ch, TRUE);
      return;
    }

  if (FIGHTING (victim) == ch && FIGHTING (ch) != victim
      && !(victim->position == POSITION_GROUNDFIGHTING
	   && ch->position == POSITION_GROUNDFIGHTING) && dt != gsn_kick
      && victim->position != POSITION_CASTING
      && victim->position != POSITION_BASHED
      && ch->position != POSITION_CASTING && ch->position != POSITION_BASHED)
    {
      NEW_POSITION (ch, POSITION_FIGHTING);
      NEW_POSITION (victim, POSITION_FIGHTING);
      //if (IS_PLAYER (ch) && ch->desc != NULL)
      //{
      //write_to_descriptor2 (ch->desc, "\n\r", 2);
      //}
      if (ch != victim || pow.can_fight_self)
	{
	  ch->fgt->fighting = victim;
	}
    }

  if (ch->position == POSITION_GROUNDFIGHTING
      && victim->position != POSITION_GROUNDFIGHTING)
    {
      NEW_POSITION (ch, POSITION_FIGHTING);
    }

  update_pos (victim);
  if (victim->position == POSITION_DEAD)
    goto isdedman;
/*
   * Stop up any residual loopholes.
 */
  if (victim != ch)
    {
      if (is_safe (ch, victim))
	{
	  if (IS_MOB (ch))
	    return;
	  act
	    ("\x1B[1;37mYou hear a loud clash of thunder in the sky, sent forth by",
	     ch, NULL, NULL, TO_CHAR);
	  act ("the gods, and you decide not to attack.\x1B[0m", ch, NULL,
	       NULL, TO_CHAR);
	  act ("You hear massive clashes of thunder in the heavens above.",
	       ch, NULL, NULL, TO_NOTVICT);
	  return;
	}

      if (IS_PLAYER (ch) && !IS_IMMORTAL (ch))
	{
	  ch->pcdata->no_quit = 2;
	}

      if (IS_PLAYER (victim) && !IS_IMMORTAL (victim))
	{
	  victim->pcdata->no_quit = 2;
	}

      if (IS_PLAYER (ch) && !IS_MOB (victim) && !IS_IMMORTAL (ch)
	  && !IS_IMMORTAL (victim))
	{
	  /*player killing player */
	  ch->pcdata->no_quit_pk = 5;
	  victim->pcdata->no_quit_pk = 5;
	}
      if (can_yell (victim) && victim->hit < 8 && victim->max_hit > 100
	  && ch->hit > 60)
	{
	  do_fatal (victim, ch, dt);
	}
      if (IS_PLAYER (ch) && dam > 2 && ch && victim && ch != victim)
	{
	  int gn;
#ifdef NEW_WORLD
	  gn = (dam * 2) + (LEVEL (victim) * 3) + ((LEVEL (ch) * LEVEL (ch)) / 4) + number_range (1, (LEVEL (victim) + LEVEL (ch)));
#else
	  gn = (dam * 3) + (LEVEL (victim) * 3) + ((LEVEL (ch) * LEVEL (ch)) / 8) + number_range (1, (LEVEL (victim) + LEVEL (ch)));
#endif
	  if (LEVEL (ch) < 100 && ch->in_room && ch->in_room->vnum >= 1000 &&
	      (!(LEVEL (ch) > 10 && LEVEL (victim) < 4)
	       && !(LEVEL (ch) > 20 && LEVEL (victim) < 8)
	       && !(LEVEL (ch) > 35 && LEVEL (victim) < 14)
	       && !(LEVEL (ch) > 50 && LEVEL (victim) < 20)))
	    {
	      if ((IS_PLAYER (ch) && !IS_PLAYER (victim))
		  || (!IS_PLAYER (ch) && IS_PLAYER (victim)))
		{
		  gain_exp (ch, gn);
		  if (IS_PLAYER (ch))
		    ch->pcdata->voting_on += gn;
		}
	    }
	}
      if (victim->position > POSITION_STUNNED && dam > 0)
	{
	  if (FIGHTING (victim) == NULL)
	    {
	      set_fighting (victim, ch);
	      NEW_POSITION (victim, POSITION_FIGHTING);
	    }
	}
      if (victim->position > POSITION_STUNNED)
	{
	  if (FIGHTING (ch) == NULL && dt != TYPE_SHOT)
	    set_fighting (ch, victim);
	}
/*
   * More charm stuff.
 */
      if (MASTER (victim) == ch)
	stop_follower (victim);
      if (IS_AFFECTED (ch, AFF_HIDE) && dt != TYPE_SHOT)
	{
	  affect_strip (ch, gsn_hide);
	  REMOVE_BIT (ch->affected_by, AFF_HIDE);
	  act ("$n is revealed.", ch, NULL, NULL, TO_ROOM);
	}
/*
   * Damage modifiers.
 */
      if (dt > 999)
	{
	  if (IS_EVIL (ch) && IS_AFFECTED (victim, AFF_PROT_EVIL))
	    dam -= number_range (0, (dam / 3));
	  if (IS_GOOD (ch) && IS_AFFECTED (victim, AFF_PROT_GOOD))
	    dam -= number_range (0, (dam / 3));
	  if (IS_AFFECTED (victim, AFF_SANCTUARY))
	    dam -= number_range (0, (dam / 2));
	  if (IS_AFFECTED (victim, AFF_PROTECT))
	    dam -= dam / 2;
	  if (dam < 0)
	    dam = 0;
	}
      if (!ch || !victim)
	return;
/*
   * Check for disarm, trip, parry, and dodge.
 */
      if (dt >= TYPE_HIT && dt != TYPE_SHOT)
	{
	  if ((IS_MOB (ch) && number_percent () < 20) && ch->move > 10)
	    {
	      if (can_trip (ch) &&
		  victim->position != POSITION_GROUNDFIGHTING
		  && victim->position != POSITION_CASTING)
		{
		  disarm (ch, victim);
		}
	    }
	  if ((IS_MOB (ch) && number_percent () < 8) &&
	      victim->position != POSITION_GROUNDFIGHTING && can_trip (ch) &&
	      victim->position != POSITION_CASTING && can_trip (victim)
	      && victim->fgt->combat_delay_count < 0 && ch->move > 10)
	    {
	      if (number_range (0, 7) == 1)
		trip (ch, victim);
	      else
		{
		  act ("$n tries to trip you, but you quickly jump out of the way!", ch, NULL, victim, TO_VICT_SPAM);
		  act ("You try to trip $N, but $N swiftly jumps out of the way!", ch, NULL, victim, TO_CHAR);
		  act ("$n tries to trip $N, but $N swiftly jumps out of the way!", ch, NULL, victim, TO_NOTVICT_SPAM);
		}
	    }
	  if (!ch || !victim)
	    return;
	  if (dt > 999 && dt != gsn_circle && dt != gsn_backstab)
	    {
	      if (IS_AFFECTED (victim, AFF_INVISIBLE) &&
		  !IS_AFFECTED (ch, AFF_DETECT_INVIS)
		  && number_range (1, 14) == 3)
		{
		  act ("You flail wildly around, trying to hit your invisible opponent...", ch, NULL, victim, TO_CHAR_SPAM);
		  return;
		}
	      if (victim->position == POSITION_GROUNDFIGHTING)
		{
		  if (number_range (1, 3) != 1)
		    {
		      act ("$N is groundfighting and you can't find a line of attack.", ch, NULL, victim, TO_CHAR_SPAM);
		      act ("$n can't find a clear line of attack at the moment.", ch, NULL, victim, TO_NOTVICT_SPAM);
		      return;
		    }
		}
	      else if (number_range (1, 8) > 2)
		{
		  if (!ch || !victim)
		    return;
		  if ((dt == 1003 || dt == 1011) && using_weapon
		      && check_parry (ch, victim))
		    return;
		  if (check_dodge (ch, victim))
		    return;
		  if (check_shield (ch, victim))
		    return;
		}
	    }
	  if (!ch || !victim)
	    return;
	  if (!IS_SET (ch->ced->fight_ops, F_POWER)
	      && number_range (1, 12) == 4
	      && ch->position == POSITION_FIGHTING)
	    {
	      act ("$N quickly steps out of the way of your attack.", ch, NULL, victim, TO_CHAR_SPAM);
	      act ("$N steps out of the way of $n's attack.", ch, NULL, victim, TO_NOTVICT_SPAM);
	      act ("You quickly step out of the way of $n's attack.", ch, NULL, victim, TO_VICT_SPAM);
	      return;
	    }
	}
      /*if (dt == gsn_backstab || dt == gsn_circle || dt == gsn_dual_backstab*/
	if (dt != TYPE_SHOT && dt != gsn_kick && (!(dt > 0 && dt < 300)))
	{
	  dam_message (ch, victim, dam, dt, ghit_pos);
	  if (dt == gsn_backstab || dt == gsn_dual_backstab || dt == gsn_circle)
	    mob_undef (dam, ch, victim);
	}
    }
  SUBHIT (victim, dam);
  upd_hps (victim);
  if (victim->fgt && victim->fgt->field)
    {
      handle_gs_da (victim, victim->fgt->pos_x, victim->fgt->pos_y);
    }
  if (!ch || !victim)
    return;

// absorb damage while casting
// added 8-27-01 by Eraser
  if (IS_PLAYER (victim))
    {
      int absorb;
      int cint = get_curr_int(ch)/2;
      int cwis = get_curr_wis(ch)/2;

      absorb = 4;
      if ((is_member (victim, GUILD_WIZARD) && (LEVEL (victim) >= LEVEL_WIZARD))
	  || (is_member (victim, GUILD_HEALER) && (LEVEL (victim) >= LEVEL_HEALER)))
	{
	  int i;

	  absorb += (LEVEL (victim)/6); //changed asorb see if mages can cast through more Kilith 05
	  absorb += (victim->pcdata->remort_times);
          absorb += cint/2;
          absorb += cwis/2;
          
      if ((is_member (victim, GUILD_NECROMANCER) && (LEVEL (victim) >= LEVEL_NECROMANCER)) || (is_member (victim, GUILD_ELEMENTAL) && (LEVEL (victim) >= LEVEL_ELEMENTAL)))
            {
          absorb += (victim->pcdata->remort_times);
          absorb += (LEVEL (victim)/7);
          absorb += cwis/3;
          absorb += cwis/3;
            }
      if (HAS_HEAD_BRAIN_IMPLANT (ch))
             {
             absorb += 25;
             }
	  i = number_percent ();

	  // 5pct chance that it doesn't work properly
	  if (i <= 5)
	    {
	      absorb = 4;
	    }
	  // 5pct chance that it works, regardless of damage
	  else if (i >= 95)
	    {
	      absorb = 99999;
	    }
	  // Whenever you get more dmg than you could normally absorb, you
	  // have a small chance to prevent losing your concentration
	  // Right now, you have a 5% chance of absorbing (absorb+20) dmg or more
	  else if ((dam > absorb) && (((dam - absorb) * 8) < (i - 5)))
	    {
	      absorb = dam;
	    }
	}
    
      if (victim->position == POSITION_CASTING && dam > absorb)
	{
	  NEW_POSITION (victim, POSITION_FIGHTING);
	  send_to_char ("\x1b[5;31mOUCH! You just lost your concentration!\x1b[37;0m\n\r", victim);
	}
    }

  if (!IS_AWAKE (victim))
    stop_fighting (victim, FALSE);
  if (IS_PLAYER (victim)
      && LEVEL (victim) >= LEVEL_IMMORTAL && victim->hit < 1)
    victim->hit = 1;
  update_pos (victim);
isdedman:
  switch (victim->position)
    {
    case POSITION_MORTAL:
      act ("\x1B[1;31m$n is mortally wounded, and will die soon if not aided.\x1B[37;0m", victim, NULL, NULL, TO_ROOM);
      send_to_char ("\x1B[1;31mYou are mortally wounded, and will die soon if not aided.\x1B[37;0m\n\r", victim);
      break;
    case POSITION_INCAP:
      act ("\x1B[0;31m$n is incapacitated and will suffer a slow, painful death if not aided.\x1B[37;0m", victim, NULL, NULL, TO_ROOM);
      send_to_char ("\x1B[0;31mYou are incapacitated and will suffer a slow, painful death if not aided.\x1B[37;0m\n\r", victim);
      break;
    case POSITION_STUNNED:
      act ("\x1B[0;33m$n is stunned, but will probably recover.\x1B[37;0m", victim, NULL, NULL, TO_ROOM);
      send_to_char ("\x1B[0;33mYou are stunned, but will probably recover.\x1B[37;0m\n\r", victim);
      break;
    case POSITION_DEAD:
      if (IS_PLAYER (ch) && LEVEL (ch) < 30)
	{
	  ch->copper++;
	  send_to_char ("\x1B[0;37mYou just got a \x1B[0;33mcopper \x1B[0;37mcoin from the gods for the kill!\x1B[37;0m\n\r", ch);
	}
      act ("\x1B[1;31m$n has been killed!\x1B[37;0m", victim, 0, 0, TO_ROOM);
      send_to_char ("\n\r\n\r\x1B[1;31mYou have been killed!\x1B[37;0m\n\r", victim);
      ch->wait = 0;
      break;
    default:
      if (dam > victim->max_hit / 5)
	send_to_char ("\x1B[1;31mOuch, that was *not* pleasant!\x1B[37;0m\n\r", victim);
      if (victim->hit < victim->max_hit / 8)
	send_to_char ("\x1B[5;31mYou sure wish the BLEEDING would stop!!!\x1B[37;0m\n\r", victim);
      break;
    }

  if (!ch || !victim)
    return;
  if (victim->position == POSITION_DEAD)
    {
      if (IS_PLAYER (victim) && IN_BATTLE (victim) == TRUE)
	{
	  stop_fighting (victim, TRUE);
	  NEW_POSITION (victim, POSITION_STANDING);
	  NEW_POSITION (ch, POSITION_STANDING);
	  MAXHIT (victim);
	  upd_hps (victim);
	  char_from_room (victim);
	  char_to_room (victim, get_room_index (victim->fgt->wasroomtwo));
	  do_look (victim, "");
	  return;
	}
      if (IS_PLAYER (victim) && CHALLENGE (victim) != 0)
	{
	  stop_fighting (victim, TRUE);
	  NEW_POSITION (victim, POSITION_STANDING);
	  NEW_POSITION (ch, POSITION_STANDING);
	  ch->fgt->fighting = NULL;
	  victim->fgt->fighting = NULL;
	  MAXHIT (ch);
	  MAXHIT (victim);
	  upd_hps (ch);
	  upd_hps (victim);
	  end_arena (ch, victim);
	  return;
	}
      group_gain (ch, victim);
      ch->wait = 0;
      if (IS_PLAYER (victim))
	{
	  int ii;
	  char buf[STD_LENGTH];
	  victim->pcdata->deaths++;
          strip_all_affects (victim);
          send_to_char ("\n\r\n\r\x1B[1;31mYou have been killed!\x1B[37;0m\n\r", victim);
	  victim->pcdata->no_quit = 0;
	  if (!IS_EVIL (victim) && !IS_EVIL (ch) && ch->pcdata->bounty > 10
	      && victim->pcdata->bounty < 10)
	    {
	      fprintf (stderr, "%s killed without provocation by %s at %d", NAME (victim), NAME (ch), victim->in_room->vnum);
	      conv_race (ch);
	    }
	  reasonfd[0] = '\0';
	  if (strlen (NAME (ch)) < 30)
	    strcpy (reasonfd, NAME (ch));
	  sprintf (log_buf, "%s killed by %s at %d", NAME (victim), NAME (ch), victim->in_room->vnum);
	  log_string (log_buf);
	  sprintf (log_buf, "%s just got killed by %s!", NAME (victim), NAME (ch));
/*WORLD ECHO ADDED BY KILITH 05
  THIS ONLY SHOWS ECHO WHEN PLAYER KILLED BY PLAYER
  THIS WILL ALSO SHOW SAME SIDE PKILL SO WILL SHOW OTHER
  SIDE WHO IS THE ASSHOLE ON THE OTHER SIDE!! LOL
  NOT DRUNK WHILE CODING THIS SO WILL WORK JUST FINE!!
*/
        if (IS_PLAYER (victim) && IS_PLAYER (ch))
        {
sprintf (buf, "\x1b[1;31m%s \x1B[0;37mwas just \x1b[1;31mK\x1b[0;31mille\x1b[1;31md \x1B[0;37mby \x1b[1;34m%s\x1B[0;37m!!",  NAME (victim), NAME (ch));
    do_echo (ch, buf);
        }
	  ii = clan_number (victim);
	  if (ii > 0)
	    clan_notify (log_buf, ii);
	  if (LEADER (victim) != NULL)
	    group_notify (log_buf, victim);
	  if (IS_PLAYER (ch))
	    {
sprintf (log_buf, "\x1b[1;31m%s \x1B[0;37mwas just \x1b[1;31mK\x1b[0;31mille\x1b[1;31md \x1B[0;37mby \x1b[1;34m%s\x1B[0;37m!!",  NAME (victim), NAME (ch));
	      ii = clan_number (ch);
	      if (ii > 0)
		clan_notify (log_buf, ii);
	    }
	  sprintf (buf, "Notify> %s", log_buf);
	  NOTIFY (buf, LEVEL_IMMORTAL, WIZ_NOTIFY_DEATH);

	  /*
	     if (IS_PLAYER (ch) && !IS_MOB (victim))
	     {
	     if (victim->pcdata->warpoints - (1 + (LEVEL (victim) / 6)) >= 0)
	     victim->pcdata->warpoints -= (1 + (LEVEL (victim) / 6));
	     }
	   */

// new same align pkill code comin' up, by Eraser 09/14/2001

	  if (IS_PLAYER (ch) && !IS_MOB (victim) && ch != victim &&
	      !IS_IMMORTAL (ch) && !IS_IMMORTAL (victim))
	    {
	      if (IS_PLAYER (victim) && IS_EVIL (ch) != IS_EVIL (victim))
		{
		  if (victim->pcdata->warpoints - (1 + (LEVEL (victim) / 6)) >= 0)
		    {
		      sprintf (buf, "[37;0mYou just lost [31;1m%d[37;0m warpoint%s!\n\r", (1 + (LEVEL (victim) / 6)),
			       (1 + (LEVEL (victim) / 6)) == 1 ? "" : "s");
		      send_to_char (buf, victim);
		      victim->pcdata->warpoints -= (1 + (LEVEL (victim) / 6));
		    }
		  else
		    {
		      if (victim->pcdata->warpoints > 0)
			{
			  sprintf (buf,
				   "[37;0mYou just lost [31;1m%d[37;0m warpoint%s!\n\r",
				   victim->pcdata->warpoints,
				   victim->pcdata->warpoints == 1 ? "" : "s");
			  send_to_char (buf, victim);
			  victim->pcdata->warpoints = 0;
			}
		    }
		}
	      else if (IS_PLAYER (victim))	// same align pkill
		{
		  int reduce = 10;
		  if (ch->pcdata->warpoints < reduce)
		    reduce = ch->pcdata->warpoints;
		  if (reduce > 0)
		    {
		      ch->pcdata->warpoints -= reduce;
		      sprintf (buf, "Notify> %s just lost %d warpoints for same align pkill!", NAME (ch), reduce);
		      NOTIFY (buf, LEVEL_IMMORTAL, WIZ_NOTIFY_DEATH);
		      sprintf (buf, "[37;0mYou just lost [31;1m%d[37;0m warpoints for same align pkill!\n\r", reduce);
		      send_to_char (buf, ch);
		    }
		  ch->pcdata->no_quit_pk = 10;
		  victim->pcdata->no_quit_pk = 0;
		}
	    }
// end new code

	  if (IS_MOB (ch) && IS_PLAYER (victim))
	    {
	      pc_death_penalty (ch, victim);
	    }
	  else
	    gain_exp (victim, -(LEVEL (victim) * 1423));
	}

      raw_kill (victim, (dt == 1003));
      if (IS_JAVA (victim))
	do_clear (victim, "");
      if (IS_PLAYER (ch) && IS_MOB (victim))
	{
	  int old_gold = 0;
	  CHAR_DATA *others;
	  char buf[STD_LENGTH];
	  bool RNQ = TRUE;
	  for (others = ch->in_room->more->people; others != NULL;
	       others = others->next_in_room)
	    {
	      if (FIGHTING (others) == ch || FIGHTING (ch) != NULL)
		{
		  RNQ = FALSE;
		  break;
		}
	    }
	  if (RNQ)
	    ch->pcdata->no_quit = 0;
	  old_gold = tally_coins (ch);
	  if (IS_SET (ch->pcdata->act2, PLR_AUTOLOOT))
	    do_get (ch, "all from corpse");
	  else if (IS_SET (ch->pcdata->act2, PLR_AUTOGOLD))
	    do_get (ch, "all.coins from corpse");
	  if (IS_SET (ch->pcdata->act2, PLR_AUTOSAC))
	    do_sacrifice (ch, "corpse");
	  if (IS_SET (ch->pcdata->act2, PLR_AUTOSPLIT)
	      && tally_coins (ch) - old_gold > 1)
	    {
	      sprintf (buf, "%d", tally_coins (ch) - old_gold);
	      do_split (ch, buf);
	    }
	}
      ch->wait = 0;
      return;
    }

  if (victim == ch)
    return;
/*
   * Take care of link dead people.
 */
  if (IS_PLAYER (victim) && victim->desc == NULL)
    {
      if (number_range (0, victim->wait) < 2)
	{
	  do_flee (victim, "");
	  return;
	}
    }

/*
   * Wimp out?
 */
/*
  if (IS_MOB (victim) && dam > 0 && dt != TYPE_SHOT)
    {
      if ((IS_SET (victim->act, ACT_WIMPY) && number_bits (1) == 0
	   && victim->hit < victim->max_hit / 6))
	do_flee (victim, "");
      return;
    }
*/
  if (IS_PLAYER (victim) && dam > 0 && dt != TYPE_SHOT)
    if (victim->ced && victim->hit <= victim->ced->wimpy
	&& victim->position != POSITION_BASHED)
      do_flee (victim, "");
  return;
}

bool
is_safe (CHAR_DATA * ch, CHAR_DATA * victim)
{
  CHAR_DATA *tch;
  bool god_present;
  if (!victim->in_room)
    return TRUE;
#ifndef NEW_WORLD
  if (IN_BOAT (ch) && IN_BOAT (victim) && !is_in_same_boat (ch, victim))
  {
    send_to_char ("They aren't here.\n\r", ch);
    return TRUE;
  }

  if ((IN_BOAT (ch) || IN_BOAT (victim)) && !is_in_same_boat (ch, victim))
  {
    send_to_char ("You must be either both in the same boat, or out of your boat to attack.\n\r", ch);
    return TRUE;
  }
#endif

  if (IS_REAL_GOD (ch))
    return FALSE;
  god_present = FALSE;
  // don't hunt into portal, remort room, or old style creation area
  if (ch->in_room && ((ch->in_room->vnum < 101 && ch->in_room->vnum > 95) ||
        (ch->in_room->vnum >= 400 && ch->in_room->vnum < 500)))
    return TRUE;
  for (tch = victim->in_room->more->people; tch != NULL;
      tch = tch->next_in_room)
    if (tch != ch && tch != victim && IS_REAL_GOD (tch) && IS_SET (tch->act, PLR_HOLYPEACE)) god_present = TRUE;
  if (god_present)
  {
    if (FIGHTING (victim) != NULL)
    {
      victim->fgt->fighting = NULL;
      NEW_POSITION (victim, POSITION_STANDING);
    }
    if (FIGHTING (ch) != NULL)
    {
      ch->fgt->fighting = NULL;
      NEW_POSITION (ch, POSITION_STANDING);
    }
    send_to_char ("No fighting in the presence of Gods!\n\r", ch);
    return TRUE;
  }

  if (IN_BATTLE (ch) || CHALLENGE (ch) == 10)
    return FALSE;

  if (IS_PLAYER (ch) && IS_PLAYER (victim) &&
      (IS_EVIL (ch) == IS_EVIL(victim)) && LEVEL (victim) < 10)
  {
    send_to_char ("That newbie is still protected, sorry.\n\r", ch);
    return TRUE;
  }
  /*
  if (IS_PLAYER (ch) && IS_PLAYER (victim) && !IS_EVIL (ch)
      && IS_EVIL (victim) && LEVEL (ch) < 10)
  {
    send_to_char ("You may not engage in pkill combat until level 10, sorry.\n\r", ch);
    return TRUE;
  }
#ifdef ALIENPK
  if (not_within_levels (ch, victim))
    return TRUE;
#endif

  if (IS_PLAYER (ch) && IS_PLAYER (victim) && NO_PKILL (ch))
   {
   send_to_char ("You can not PK IF you want to join in PK you must delete and reroll.\n\r", ch);
    return TRUE;
   }
  if (IS_PLAYER (victim) && IS_PLAYER (ch) && NO_PKILL (victim))
    send_to_char ("That person is protected from PK.\n\r", ch);
    return TRUE;
*/

  /*
     if (IS_PLAYER(ch) && IS_PLAYER (victim) && !IS_EVIL(ch) &&
     !IS_EVIL(victim)
   */
  //ALLOW_UNMOTIVATED_PK (yes)
  if (!pow.good_kill_good && IS_PLAYER (ch) && IS_PLAYER (victim)
      && !IS_EVIL (ch) && !IS_EVIL (victim))
  {
    return TRUE;
  }
  if (!pow.evil_kill_evil && IS_PLAYER (ch) && IS_PLAYER (victim)
      && IS_EVIL (ch) && IS_EVIL (victim))
  {
    return TRUE;
  }


  return FALSE;
}


bool
check_parry (CHAR_DATA * ch, CHAR_DATA * victim)
{
  int chance;
  CHAR_DATA *mob;
  int number = 0;
  if (victim->move <= 5)
    return FALSE;
  if (!IS_AWAKE (victim) || victim->position <= POSITION_STUNNED)
    return FALSE;

/* Optimize this! */
  if (!using_weapon)
    return FALSE;


  if (IS_AFFECTED (ch, AFF_INVISIBLE) && !IS_AFFECTED (victim, AFF_DETECT_INVIS))
    {
      act ("You try to parry, but you can't see your opponent and fail!", victim, NULL, ch, TO_CHAR);
      return FALSE;
    }

/* If more than one fighting you, the less chance of parrying */
  for (mob = ch->in_room->more->people; mob != NULL; mob = mob->next_in_room)
    {
      if (FIGHTING (mob) == victim && mob != victim)
	number++;
    }

  if (number < 1)
    number = 1;

  if (IS_MOB (victim))
    {
      chance = victim->pIndexData->parry +
	(UMIN (60, (LEVEL (victim) * 2)) / (number * 2));
    }
  else
    {
      chance = victim->pcdata->learned[gsn_parry] / (2 * number);
    }

  if (IS_PLAYER (victim) && is_member (victim, GUILD_WARRIOR))
    chance += 32;

  if (number_range (1, 1000) >= chance)
    return FALSE;
  if (number_range (1, 2) == 2)
    {
      act ("You skillfully parry $n's attack.", ch, NULL, victim, TO_VICT);
      act ("$N skillfully parries your attack.", ch, NULL, victim, TO_CHAR);
      act ("$N skillfully parries $n's attack.", ch, NULL, victim, TO_NOTVICT);
    }
  else
    {
      act ("You forcefully deflect $n's blade, parrying the attack.", ch, NULL, victim, TO_VICT);
      act ("$N forcefully deflects your blade, parrying the attack.", ch, NULL, victim, TO_CHAR);
      act ("$N deflects $n's attack with his weapon.", ch, NULL, victim, TO_NOTVICT);
    }

  if (number_range (1, 3) == 2)
    skill_gain (victim, gsn_parry, TRUE);
  SUBMOVE (victim, 1);
  return TRUE;
}


bool
check_dodge (CHAR_DATA * ch, CHAR_DATA * victim)
{
  int chance;
  CHAR_DATA *mob;
  int number = 0;
  if (RIDING (victim) != NULL)
    return FALSE;
  if (victim->move <= 5)
    return FALSE;


  if (IS_PLAYER (victim))
    {
      if (victim->pcdata->carry_weight > (get_curr_str (victim) * 2))
	if (number_range (1, 6) == 2)
	  return FALSE;
      if (victim->pcdata->carry_weight > (get_curr_str (victim) * 3))
	if (number_range (1, 5) == 2)
	  return FALSE;
      if (victim->pcdata->carry_weight > (get_curr_str (victim) * 4))
	if (number_range (1, 4) == 2)
	  return FALSE;
      if (victim->pcdata->carry_weight > (get_curr_str (victim) * 5))
	if (number_range (1, 3) == 2)
	  return FALSE;
      if (victim->pcdata->carry_weight > (get_curr_str (victim) * 6))
	if (number_range (1, 3) == 2)
	  return FALSE;
    }

  if (!IS_AWAKE (victim) || victim->position <= POSITION_STUNNED)
    return FALSE;
/* If more than one fighting you, the less chance of parrying */
  for (mob = ch->in_room->more->people; mob != NULL; mob = mob->next_in_room)
    {
      if (FIGHTING (mob) == victim && mob != victim)
	number++;
    }

  if (number < 1)
    number = 1;

  if (IS_MOB (victim))
    {
      chance = victim->pIndexData->dodge + ((LEVEL (victim) / 3) / (number)); //took out UMIN(75,
    }
  else
    {
      chance = victim->pcdata->learned[gsn_dodge] + ((LEVEL (victim) / 3) / (number));
    }

  if (IS_PLAYER (victim) && is_member (victim, GUILD_THIEFG))
    chance += 20 ;
   if (IS_PLAYER (victim) && is_member (victim, GUILD_MARAUDER))
    chance += 10;
   if (IS_PLAYER (victim) && is_member (victim, GUILD_WARRIOR))
    chance += 15;
  if (number_range (0, 500) >= chance)
    return FALSE;

  if (IS_AFFECTED (ch, AFF_INVISIBLE)
      && !IS_AFFECTED (victim, AFF_DETECT_INVIS))
    {
      if (number_range (1, 10) > 3)
	{
	  act ("You try to dodge, but since you can't see where the attacker is, you fail!", victim, NULL, victim, TO_VICT);
	  return FALSE;
	}
      else
	{
	  act ("Although you can't see the attacker, you get lucky and dodge the attack!", ch, NULL, victim, TO_VICT);
	  act ("$N gets lucky and quickly dodges your attack.", ch, NULL, victim, TO_CHAR);
	  act ("$N gets lucky and dodges $n's attack.", ch, NULL, victim, TO_NOTVICT);
	  if (number_range (1, 2) == 2)
	    skill_gain (victim, gsn_dodge, TRUE);
	  SUBMOVE (victim, 1);
	  return TRUE;
	}
    }

  else
    {
      if (victim->position != POSITION_BASHED)
	{
	  if (number_range (1, 2) == 2)
	    {
	      act ("You step aside, dodging $n's attack.", ch, NULL, victim, TO_VICT);
	      act ("$N quickly steps aside, dodging your attack.", ch, NULL, victim, TO_CHAR);
	      act ("$N steps aside, dodging $n's attack.", ch, NULL, victim, TO_NOTVICT);
	    }
	  else
	    {
	      if (number_range (1, 2) == 2)
		{
		  act ("You duck right, dodging $n's attack.", ch, NULL, victim, TO_VICT);
		  act ("$N ducks right, dodging your attack.", ch, NULL, victim, TO_CHAR);
		  act ("$N ducks right, dodging $n's attack.", ch, NULL, victim, TO_NOTVICT);
		}
	      else
		{
		  act ("You duck left, dodging $n's attack.", ch, NULL, victim, TO_VICT);
		  act ("$N ducks left, dodging your attack.", ch, NULL, victim, TO_CHAR);
		  act ("$N ducks left, dodging $n's attack.", ch, NULL, victim, TO_NOTVICT);
		}
	    }
	}
      else
	{
	  act ("You roll out of the way of $n's attack.", ch, NULL, victim, TO_VICT);
	  act ("$N rolls out of the way of your attack.", ch, NULL, victim, TO_CHAR);
	  act ("$N rolls out of the way of $n's attack.", ch, NULL, victim, TO_NOTVICT);
	}
      if (number_range (1, 2) == 2)
	skill_gain (victim, gsn_dodge, TRUE);
      SUBMOVE (victim, 1);
      return TRUE;
    }


  return FALSE;
}

/*
   * Check for shield block.
 */
bool
check_shield (CHAR_DATA * ch, CHAR_DATA * victim)
{
  int chance, chancey;
  CHAR_DATA *mob;
  CHAR_DATA *vict;
  SINGLE_OBJECT *Attshield = NULL;
  int number = 0;
  vict = FIGHTING (ch);
  if (!IS_AWAKE (victim))
    return FALSE;

  /* Optimize this */
  if (!using_shield)
    return FALSE;

  /* If more than one fighting you, the less chance of parrying */
  for (mob = ch->in_room->more->people; mob != NULL; mob = mob->next_in_room)
    {
      if (FIGHTING (mob) == victim && mob != victim)
	number++;
    }

  if (number < 1)
    number = 1;

  if (IS_MOB (victim))
    chance = UMIN (60, LEVEL (victim));
  else
    {
      chance = victim->pcdata->learned[gsn_shield_block] / (number / 2.0);
    }

  if (number_range (1, 800) >= chance)
    return FALSE;
  act ("You block $n's attack with your shield.", ch, NULL, victim, TO_VICT);
  act ("$N blocks your attack with $S shield.", ch, NULL, victim, TO_CHAR);
  act ("$N blocks $n's attack with $S shield.", ch, NULL, victim, TO_NOTVICT);
  skill_gain (victim, gsn_shield_block, TRUE);
  chancey = (get_curr_dex (ch));
  if (Attshield == NULL)
    return TRUE;
  else if (number_range (1, 30) <= chancey)
    {
      int dam;
      act ("You slam into $N with your shield.", ch, NULL, vict, TO_CHAR);
      act ("$N slams into you with $s shield.", ch, NULL, vict, TO_VICT);
      act ("$N slams $S shield into $n's body.", ch, NULL, vict, TO_NOTVICT);
      dam = (number_range ((get_curr_str (ch) / 3), (get_curr_str (ch))));
      SUBHIT (vict, dam);
      if (vict->hit < 1)
	vict->hit = 1;
    }
  return TRUE;
}

/*
   * Set position of a victim.
 */
void
update_pos (CHAR_DATA * victim)
{
  if (victim == NULL)
    return;
  if (victim->position == POSITION_STANDING && victim->fgt
      && victim->fgt->field)
    {
      remove_from_combat_field (victim);
    }
  if (victim->hit > 0)
    {
      if (victim->position <= POSITION_STUNNED && victim->position >= 0)
	{
	  if (FIGHTING (victim) == NULL)
	    NEW_POSITION (victim, POSITION_STANDING);
	}
      return;
    }

  if (IS_MOB (victim) || victim->hit <= -5)
    {
      NEW_POSITION (victim, POSITION_DEAD);
      return;
    }

  if (victim->hit <= -3)
    NEW_POSITION (victim, POSITION_MORTAL);
  else if (victim->hit <= -1)
    NEW_POSITION (victim, POSITION_INCAP);
  else
    NEW_POSITION (victim, POSITION_STUNNED);
  return;
}

/*
   * Start fights.
 */
void
set_fighting (CHAR_DATA * ch, CHAR_DATA * victim)
{
  check_ced (ch);
  check_ced (victim);
  check_fgt (ch);
  check_fgt (victim);
  if (ch == NULL || victim == NULL)
    return;
  if (ch == victim && !pow.can_fight_self)
    return;
/*if ( IS_AFFECTED(ch, AFF_SLEEP) )
   affect_strip( ch, gsn_sleep ); */
  ch->fgt->fighting = victim;
  if (ch->position != POSITION_GROUNDFIGHTING
      && ch->position != POSITION_BASHED)
    NEW_POSITION (ch, POSITION_FIGHTING);
  add_to_fighting_list (ch);
  add_to_fighting_list (victim);
  if (!FIGHTING (victim))
    {
      set_fighting (victim, ch);
    }
#ifndef OLD_FIGHT
  add_to_combat_field (ch, victim);
  add_to_combat_field (victim, ch);
#endif
  if (1)			/*!second) */
    {
      if (IS_MOB (victim) && FIGHTING (victim) != NULL && IS_PLAYER (FIGHTING (victim)) && !victim->ced->has_yelled
	  && victim->pIndexData->yeller_number != 0 && (HUNTING (victim) == NULL || FIGHTING (FIGHTING (victim)) == NULL))
	{
	  CHAR_DATA *tempch;
	  CHAR_DATA *witness;
	  if (IS_SET (victim->pIndexData->act4, ACT4_SOLOYELL))
	    {
	      witness = NULL;
	      for (witness = victim->in_room->more->people; witness != NULL;
		   witness = witness->next_in_room)
		{
		  if (witness == victim || (witness->pIndexData && witness->pIndexData->yeller_number < 1))
		    continue;
		  check_ced (witness);
		  if (IS_MOB (witness) && !witness->ced->has_yelled && witness->pIndexData->yeller_number == victim->pIndexData->yeller_number)
		    break;
		}
	      if (witness != NULL)
		{
		  for (tempch = char_list; tempch != NULL;
		       tempch = tempch->next)
		    {
		      if (tempch->in_room && tempch->in_room->area != victim->in_room->area)
			continue;
		      if (IS_MOB (tempch) && tempch->pIndexData->yeller_number == victim->pIndexData->yeller_number 
			  && tempch->in_room && tempch->in_room->area == victim->in_room->area && tempch->pIndexData->will_help > 0)
			{
			  if (tempch->pIndexData->will_help == 1
			      || number_range (1, tempch->pIndexData->will_help) == 2)
			    {
			      if (HUNTING (tempch) != NULL)
				{
				  free_string (tempch->fgt->hunting);
				  tempch->fgt->hunting = NULL;
				}
			      check_fgt (tempch);
			      tempch->fgt->hunting =
				str_dup (NAME (FIGHTING (victim)));
			      add_to_fighting_list (tempch);
			    }
			}
		    }
		  check_ced (victim);
		  check_ced (witness);
		  victim->ced->has_yelled = TRUE;
		  witness->ced->has_yelled = TRUE;
		  if (can_yell (witness))
		    {
		      char buffr[500];
		      if (number_range (1, 2) == 1)
			sprintf (buffr, "Help! %s is attacking %s!", NAME (FIGHTING (victim)), SD (victim));
		      else if (number_range (1, 2) == 2)
			sprintf (buffr, "%s is trying to kill %s! Send help!", NAME (FIGHTING (victim)), SD (victim));
		      do_yell (witness, buffr);
		    }
		}
	    }
	  else
	    {
	      for (tempch = char_list; tempch != NULL; tempch = tempch->next)
		{
		  if (tempch->in_room && tempch->in_room->area != victim->in_room->area)
		    continue;
		  if (IS_MOB (tempch) && tempch->pIndexData->yeller_number == victim->pIndexData->yeller_number 
			&& tempch->in_room && tempch->in_room->area == victim->in_room->area && tempch->pIndexData->will_help > 0)
		    {
		      if (tempch->pIndexData->will_help == 1
			  || number_range (1, tempch->pIndexData->will_help) == 2)
			{
			  if (HUNTING (tempch) != NULL)
			    {
			      free_string (tempch->fgt->hunting);
			      tempch->fgt->hunting = NULL;
			    }
			  check_fgt (tempch);
			  tempch->fgt->hunting = str_dup (NAME (FIGHTING (victim)));
			  add_to_fighting_list (tempch);
			}
		    }
		}
	      victim->ced->has_yelled = TRUE;
	      if (can_yell (victim))
		{
		  char buffr[500];
		  if (number_range (1, 2) == 1)
		    sprintf (buffr, "%s just attacked me!", NAME (FIGHTING (victim)));
		  else
		    sprintf (buffr, "%s is trying to kill me!!", NAME (FIGHTING (victim)));
		  do_yell (victim, buffr);
		}
	    }
	}
    }
  /*End if !second */ return;
}

/*
   * Stop fights.
 */
void
stop_fighting (CHAR_DATA * ch, bool fBoth)
{
  CHAR_DATA *fch;
  CHAR_DATA *fch_next;

  if (FIGHTING (ch) == FIGHTING (FIGHTING (ch)))
    fBoth = TRUE;

  if (fBoth && FIGHTING (ch))
    {
      remove_from_combat_field (FIGHTING (ch));
      NEW_POSITION (ch->fgt->fighting, POSITION_STANDING);
      if (FIGHTING (FIGHTING (ch)))
	ch->fgt->fighting->fgt->fighting = NULL;
      update_pos (FIGHTING (ch));
    }

  remove_from_combat_field (ch);

  for (fch = f_first; fch != NULL; fch = fch_next)
    {
      fch_next = fch->next_fighting;
      if (fch == ch
	  || (fBoth && FIGHTING (fch) != NULL && FIGHTING (fch) == ch))
	{
	  fch->fgt->fighting = NULL;
	  NEW_POSITION (fch, POSITION_STANDING);
	  fch->fgt->combat_delay_count = -1;
	  update_pos (fch);
	}
    }

  if (ch->fgt)
    ch->fgt->fighting = NULL;
  NEW_POSITION (ch, POSITION_STANDING);
  update_pos (ch);
  return;
}

bool
IS_CORPSE_MOB (CHAR_DATA * ch)
{
  if (IS_PLAYER (ch))
    return TRUE;
  switch (ch->pIndexData->mobtype)
    {
    case MOB_GHOST:
      return FALSE;
      break;
    case MOB_STATUE:
      return FALSE;
      break;
    case MOB_PLANT:
      return FALSE;
      break;
    case MOB_FISH:
      return FALSE;
      break;
    }
  return TRUE;
}

int
xp_compute (CHAR_DATA * gch, CHAR_DATA * victim)
{
  int before;
  int kkk;
  if (IS_MOB (gch) || !pow.evil_good)
    return 0;
  if (!race_info[gch->pcdata->race].switch_aligns)
    return 0;
  before = gch->pcdata->alignment;
  kkk = ALIGN (victim) / 70;
  if (kkk < -3)
    kkk = -3;
  if (kkk > 3)
    kkk = 3;
  gch->pcdata->alignment -= kkk;
  if (ALIGN (gch) < -100 && ALIGN (gch) > -150 && IS_PLAYER (gch))
    send_to_char ("You feel a strange evil presence creeping into your soul...\n\r", gch);
  if (ALIGN (gch) > 100 && ALIGN (gch) < 150)
    send_to_char ("You feel a strange devotion to virtue creeping into your soul...\n\r", gch);
  conv_race (gch);
  clan_check_align (gch);
  if (!IS_EVIL (gch) && before < -149)
    {
      gch->pcdata->alignment = 5000;
      gch->pcdata->warpoints = 0;
    }

  if (IS_EVIL (gch) && before > -150)
    {
      gch->pcdata->alignment = -5000;
      gch->pcdata->warpoints = 0;
    }

  return 2;
}

void
disarm (CHAR_DATA * ch, CHAR_DATA * victim)
{
  SINGLE_OBJECT *obj;
  if ((obj = get_item_held (ch, ITEM_WEAPON)) == NULL)
    return;
  if ((obj = get_item_held (victim, ITEM_WEAPON)) == NULL)
    return;
  if (number_range (0, MAX_STAT) < get_curr_str (victim))
    {
      act ("$n tries to disarm you, but your strength allows you to hold on!", ch, NULL, victim, TO_VICT_SPAM + 1000);
      act ("You try to disarm $N, but $N's strength won't allow it!", ch, NULL, victim, TO_CHAR + 1000);
      act ("$n tries to disarm $N, but $N's strength won't allow it!", ch, NULL, victim, TO_NOTVICT_SPAM + 1000);
      return;
    }

  if (number_range (1, 3) > 1)
    return;
  if (IS_SET (obj->extra_flags, ITEM_NOREMOVE) || IS_IMMORTAL (ch))
    {
      act ("$B$n tries to disarm you, but $p stays firmly in your sweaty hand!$R", ch, obj, victim, TO_VICT);
      act ("You try to knock the weapon from $S's hand, but you fail miserably!", ch, obj, victim, TO_CHAR);
      return;
    }

  act ("$B$4$n disarms you and sends your weapon flying!$R$7", ch, NULL, victim, TO_VICT);
  act ("$B$4You have managed to disarm $N!$R$7", ch, NULL, victim, TO_CHAR);
  act ("$B$4$n disarms $N!$R$7", ch, NULL, victim, TO_NOTVICT);
  unequip_char (victim, obj);
  obj_from (obj);
  if (IS_MOB (victim))
    obj_to (obj, victim);
  else
    obj_to (obj, victim->in_room);
  return;
}

void
do_bash (CHAR_DATA * ch, char *argy)
{
  CHAR_DATA *victim;
  int chn;
  bool notft = FALSE;
  int tmpi;
  char buf[STD_LENGTH];
  DEFINE_COMMAND ("bash", do_bash, POSITION_FIGHTING, 0, LOG_NEVER, "This command allows you to bash an opponent to the ground.")
    check_fgt (ch);
  chn = 0;
  if (ch->wait > 2)
    return;
  if (ch->fgt->ears > 9)
    {
      send_to_char ("You can't bash this soon again after giving or recieving a bash.\n\r", ch);
      return;
    }
  if (ch->in_room->sector_type == SECT_UNDERWATER)
    {
      send_to_char ("Bash someone underwater? I don't think so...\n\r", ch);
      return;
    }
  if ((FIGHTING (ch) == NULL || ch->position != POSITION_FIGHTING)
      && (argy == "" || argy[0] == '\0'))
    {
      send_to_char ("Bash whom?\n\r", ch);
      return;
    }
  if (FIGHTING (ch) == NULL)
    {
      notft = TRUE;
      if ((victim = get_char_room (ch, argy)) == NULL)
	{
	  if (argy[0] == '\0')
	    {
	      send_to_char ("Bash who?\n\r", ch);
	      return;
	    }
	  send_to_char ("They aren't here.\n\r", ch);
	  return;
	}
      if (victim == ch)
	{
	  send_to_char ("You really do look like an idiot!\n\r", ch);
	  return;
	}
    }

  else
    victim = ch->fgt->fighting;
  if (RIDING (ch) != NULL)
    {
      send_to_char ("You can't bash someone when you are riding!\n\r", ch);
      return;
    }
  if (is_safe (ch, victim))
    {
      return;
    }
  if (IS_PLAYER (ch) && IS_PLAYER (victim) && (IS_EVIL (ch) == IS_EVIL (victim)) && HP_NOW (ch) < 100)
    {
      send_to_char ("Maybe that's not the right thing to do.\n\r", ch);
      return;
    }
  check_ced (ch);
  check_ced (victim);
  if (!(IS_IN_MELEE (ch, victim)))
    {
      send_to_char ("You aren't on the front lines of that battle!\n\r", ch);
      return;
    }
  if (FIGHTING (ch) != victim)
    {
      if (!pkill (ch, victim))
	return;
      check_pkill (ch, victim);
    }
  if (!can_see (ch, victim))
    {
      send_to_char ("How can you bash someone you can't see!?\n\r", ch);
      return;
    }
 if (ch->position == POSITION_STOPCAST)
    {
    send_to_char ("You can not bash while concentrating!!\n\r", ch);
    NEW_POSITION (ch, POSITION_FIGHTING);
    return;
    }
  if (ch->position == POSITION_GROUNDFIGHTING)
    {
      act ("You are wrestling on the ground! You can't bash $N!", ch, NULL, victim, TO_CHAR);
      return;
    }
  if (victim->position == POSITION_GROUNDFIGHTING)
    {
      send_to_char ("Your victim is groundfighting!\n\r", ch);
      return;
    }
  if (ch->position == POSITION_BASHED)
    {
      act ("You yourself are bashed.. how do you expect to bash $N?", ch, NULL, victim, TO_CHAR);
      return;
    }
  if (!bashable (victim))
    {
      act ("You can't bash that kind of creature!", ch, NULL, victim, TO_CHAR);
      return;
    }
  check_fgt (victim);

  if (victim->fgt->combat_delay_count < -2)
    {
      act ("$N is alert from $S last bash to the ground. You nearly fall over!", ch, NULL, victim, TO_CHAR);
      WAIT_STATE (ch, 2 * PULSE_VIOLENCE);
      return;
    }
  if (victim->position == POSITION_BASHED)
    {
      act ("$N has already been bashed to the ground!", ch, NULL, victim, TO_CHAR);
      return;
    }
  if (number_range (1, 11) == 3 && IS_AWAKE (victim)
      && IS_AFFECTED (victim, AFF_FLYING) && !IS_AFFECTED (ch, AFF_FLYING))
    {
/*basher is not flying, victim is */
      act ("$N quickly flies out of the way of $n's bash!", ch, NULL, victim, TO_NOTVICT + 1000);
      act ("$N quickly flies out of the way of your bash!", ch, NULL, victim, TO_CHAR + 1000);
      act ("You quickly fly out of the way of $n's bash!", ch, NULL, victim, TO_VICT + 1000);
      WAIT_STATE (ch, 4 * PULSE_VIOLENCE);
      return;
    }

  if (IS_MOB (victim))
    tmpi = 10;
  else
    tmpi = (15 + get_curr_dex (victim) + (victim->pcdata->learned[gsn_dodge] / 6));
  if (IS_MOB (ch))
    chn = (LEVEL (ch) * 2);
  else
    chn = ((ch->pcdata->learned[gsn_bash]) + (get_curr_str (ch) * 2)) - tmpi;
  if (IS_PLAYER (ch) && ch->pcdata->learned[gsn_bash] < 5)
    chn = 5;
  if (chn < 5)
    chn = 5;
  if (chn > 155)
    chn = number_range (130, 160);
  if (HAS_BODY_STRENGTH_IMPLANT (ch))
    chn += number_range (18, 34);
  if (HAS_BODY_STRENGTH_IMPLANT (victim))
    chn -= number_range (10, 20);
  if (number_range (0, 210) > chn && IS_AWAKE (victim))
    {
      act ("You miss the bash and almost fall on the ground!", ch, NULL, victim, TO_CHAR + 1000);
      act ("$n tries to bash you, but almost falls over instead!", ch, NULL, victim, TO_VICT + 1000);
      act ("$n misses a bash at $N and almost falls over!", ch, NULL, victim, TO_NOTVICT + 1000);
      if (notft)
	{
	  WAIT_STATE (ch, (double) 7.3 * PULSE_VIOLENCE);
	}
      else
	WAIT_STATE (ch, (double) 5.3 * PULSE_VIOLENCE);

      if (FIGHTING (victim) == NULL)
	set_fighting (victim, ch);
      set_fighting (ch, victim);
      return;
    }

  if (RIDING (victim) != NULL)
    {
      sprintf (buf, "$B$2Your powerful bash sends $N flying off of %s!$R$7", PERS (RIDING (victim), ch));
      act (buf, ch, NULL, victim, TO_CHAR);
      sprintf (buf, "$B$2$n's bash sends $N flying off of %s!$R$7", NAME (RIDING (victim)));
      act (buf, ch, NULL, victim, TO_NOTVICT);
      sprintf (buf, "$B$2You fly off of %s as $n bashes you!$R$7",
	       PERS (RIDING (victim), victim));
      act (buf, ch, NULL, victim, TO_VICT);
      check_fgt (victim->fgt->riding);
      victim->fgt->riding->fgt->mounted_by = NULL;
      victim->fgt->riding = NULL;
    }

  else if (RIDING (victim) == NULL)
    {
      if (number_range (1, 18) != 4 || !can_groundfight (victim)
	  || LEVEL (victim) < 12 || !can_groundfight (ch) || LEVEL (ch) < 12
	  || !pow.bash_slip_to_tackle)
	{
	  char bufx[400];
	  act ("$B$2Your powerful bash sends $N sprawling onto the ground!$R$7", ch, NULL, victim, TO_CHAR + 1000);
	  act ("$B$2$n sends $N sprawling backwards on to the ground with a powerful bash!$R$7", ch, NULL, victim, TO_NOTVICT + 1000);
	  act ("$B$2$n sends you sprawling backwards on to the ground with a powerful bash!$R$7", ch, NULL, victim, TO_VICT + 1000);
	  if (IS_PLAYER (ch))
	    skill_gain (ch, gsn_bash, TRUE);
	  if (CHALLENGE (ch) == 10)
	    {
	      sprintf (bufx, "($B$1Arena$N$7) $B%s just sent %s flying with a bash!", NAME (ch), NAME (victim));
	      do_arenaecho (ch, bufx, TRUE);
	    }
	}
      else
	{
	  char bufx[500];
	  int i;
	  act ("$B$5You bash $N, but trip and stumble! You are now groundfighting!$R$7", ch, NULL, victim, TO_CHAR);
	  act ("$B$5$n bashes $N, but trips and falls! They begin fighting on the ground!$R$7", ch, NULL, victim, TO_NOTVICT);
	  act ("$B$5$n bashes you, but trips and looses his balance and falls on top of you. You begin to ground-fight!!$R$7", ch, NULL, victim, TO_VICT);
	  if (CHALLENGE (ch) == 10)
	    {
	      i = number_range (1, 3);
	      if (i == 1)
		sprintf (bufx, "($B$1Arena$N$7) $B%s tries to bash %s but they both fall to the\n\r($B$1Arena$N$7) $Bground in a bloody tangle of elbows and teeth...",
			 NAME (ch), NAME (victim));
	      if (i == 2)
		sprintf (bufx, "($B$1Arena$N$7) $B%s falls on his face, trying to bash %s.\n\r($B$1Arena$N$7) $BThey both fall over and begin clawing at each other!",
			 NAME (ch), NAME (victim));
	      if (i == 3)
		sprintf (bufx, "($B$1Arena$N$7) $B%s tries to bash %s, but %s catches %s\n\r($B$1Arena$N$7) $Bwith an elbow and they both fall to the ground in a bloody mess!",
			 NAME (ch), NAME (victim), NAME (victim), NAME (ch));
	      do_arenaecho (ch, bufx, TRUE);
	    }
	  set_fighting (ch, victim);
	  victim->fgt->fighting = ch;
	  NEW_POSITION (victim, POSITION_GROUNDFIGHTING);
	  NEW_POSITION (ch, POSITION_GROUNDFIGHTING);
	  return;
	}
    }
  {
    SPELL_DATA *s;
    if ((s = skill_lookup (NULL, gsn_bash)) == NULL)
      {
	fprintf (stderr, "Bash not found!\n");
	return;
      }
    victim->fgt->combat_delay_count = 4;
    if (victim->position != POSITION_FIGHTING)
      victim->fgt->fighting = ch;
    victim->position = POSITION_BASHED;
    set_fighting (ch, victim);
    victim->fgt->ears = 10 + (s->casting_time);
    ch->fgt->ears = 10 + (s->casting_time * 2);
  }
  return;
}

void
trip (CHAR_DATA * ch, CHAR_DATA * victim)
{
  if (ch->in_room->sector_type == SECT_UNDERWATER)
    {
      return;
    }
  if (RIDING (victim) != NULL)
    return;
  if (IS_AFFECTED (victim, AFF_FLYING) || IS_IMMORTAL (ch))
    {
      act ("$n tries to trip you, but you float over it!", ch, NULL, victim, TO_VICT_SPAM);
      act ("You try to trip $N, but $N floats over it!", ch, NULL, victim, TO_CHAR);
      act ("$n tries to trip $N, but $N floats over it!", ch, NULL, victim, TO_NOTVICT_SPAM);
      return;
    }

  act ("$B$n trips you and you go down!$R", ch, NULL, victim, TO_VICT);
  act ("You trip $N and $E goes down!", ch, NULL, victim, TO_CHAR);
  act ("$n trips $N!", ch, NULL, victim, TO_NOTVICT);
  victim->fgt->combat_delay_count = UMAX (1, dice (1, 5 - get_curr_dex (victim) / 6));
  WAIT_STATE (ch, PULSE_VIOLENCE);
  WAIT_STATE (victim, PULSE_VIOLENCE);
  return;
}

void
do_kill (CHAR_DATA * ch, char *argy)
{
  char arg[SML_LENGTH];
  CHAR_DATA *victim;
  CHAR_DATA *fch, *fch_next;
  DEFINE_COMMAND ("kill", do_kill, POSITION_FIGHTING, 0, LOG_NORMAL, "This command allows you to start combat or switch targets during combat.")
    check_fgt (ch);
  one_argy (argy, arg);
  if (ch->fgt->ears > 9)
    {
      send_to_char ("You are suffering from the ever-popular bash lag (tm), and therefore\n\rcannot attack another mob/player or use the kill command.\n\r", ch);
      return;
    }
  if (arg[0] == '\0')
    {
      send_to_char ("Syntax: Kill <thing/person to kill>\n\r", ch);
      return;
    }
  if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("Who or what is that?\n\r", ch);
      return;
    }
  if (victim == ch)
    {
      send_to_char ("You hit yourself.. moron!\n\r", ch);
      //multi_hit (ch, ch, TYPE_UNDEFINED);
      return;
    }
  if (!can_see (ch, victim))
    {
      send_to_char ("Who or what is that?\n\r", ch);
      return;
    }
  if (is_safe (ch, victim))
    {
      return;
    }
  if (!pkill (ch, victim))
    return;

  if (IS_PLAYER (ch) && IS_PLAYER (victim) && (IS_EVIL (ch) == IS_EVIL (victim)) && HP_NOW (ch) < 100)
    {
      send_to_char ("Maybe that's not the right thing to do.\n\r", ch);
      return;
    }
  check_pkill (ch, victim);
  if (victim->position == POSITION_GROUNDFIGHTING)
    {
      set_fighting (ch, victim);
      return;
    }
  if (IS_AFFECTED (ch, AFF_CHARM) && MASTER (ch) == victim)
    {
      act ("$N is your master!", ch, NULL, victim, TO_CHAR);
      return;
    }
  for (fch = ch->in_room->more->people; fch != NULL; fch = fch_next)
    {
      fch_next = fch->next_in_room;
      if (IS_MOB (fch) && IS_SET (fch->pIndexData->act3, ACT3_GUARD_MOB)
	  && victim->pIndexData != NULL
	  && victim->pIndexData->vnum == fch->pIndexData->guard)
	{
	  do_say (fch, "You'll have to get by me first!");
#ifdef OLD_FIGHT
	  multi_hit (fch, ch, TYPE_UNDEFINED);
#else
	  set_fighting (fch, ch);
#endif
	  return;
	}
    }

  if (FIGHTING (ch) && victim == FIGHTING (ch))
    {
      act ("You're already fighting $N!", ch, NULL, victim, TO_CHAR);
      return;
    }

  check_fgt (victim);
#ifdef OLD_FIGHT
  if (ch->position == POSITION_FIGHTING && FIGHTING (ch) != victim)
#endif
    {
      if (!IS_AWAKE (victim))
	{
	  multi_hit (ch, victim, TYPE_UNDEFINED);
	  return;
	}
      act ("Your new target is $N!", ch, NULL, victim, TO_CHAR);
      act ("$n turns and targets $N!", ch, NULL, victim, TO_ROOM);
      WAIT_STATE (ch, PULSE_VIOLENCE);
      set_fighting (ch, victim);
    }
#ifndef OLD_FIGHT
  return;
#endif
  if ((IS_PLAYER (victim)) && IS_AFFECTED (ch, AFF_CHARM))
    {
      return;
    }

  if (ch->fgt->combat_delay_count >= 0)
    {
      ch->fgt->combat_delay_count = -1;
    }

  if (victim->fgt->combat_delay_count >= 0)
    {
      victim->fgt->combat_delay_count = -1;
    }


  WAIT_STATE (ch, 1 * PULSE_VIOLENCE);
  if (IS_MOB (ch) && IS_SET (ch->act, ACT_KILLGLADIATOR))
    {
      do_say (ch, "You are not permitted to leave!!");
    }

  set_fighting (ch, victim);
  if (victim->position != POSITION_FIGHTING &&
      victim->position != POSITION_GROUNDFIGHTING &&
      victim->position != POSITION_BASHED)
    victim->fgt->fighting = ch;
#ifdef OLD_FIGHT
  multi_hit (ch, victim, TYPE_UNDEFINED);
#endif
  return;
}

void
do_flurry (CHAR_DATA * ch, char *argy)
{
  char arg[SML_LENGTH];
  CHAR_DATA *victim;
  DEFINE_COMMAND ("flurry", do_flurry, POSITION_FIGHTING, 0, LOG_NORMAL, "This command allows you to flail around madly in a wild flurry.  Anyone can do this (it uses moves).")
    if (!FIGHTING (ch))
    {
      send_to_char ("You must be fighting to flurry.\n\r", ch);
      return;
    }
  one_argy (argy, arg);

  if (ch->fgt->ears > 9)
    {
      send_to_char ("You can't flurry this soon after giving or recieving a bash.\n\r", ch);
      return;
    }
 if (ch->position == POSITION_STOPCAST)
    {
    send_to_char ("You can not flurry while you are concentrating!!\n\r", ch);
    NEW_POSITION (ch, POSITION_FIGHTING);
    return;
    }
  if (ch->position == POSITION_GROUNDFIGHTING)
    {
      send_to_char ("You can't flurry while groundfighting!\n\r", ch);
      return;
    }
  if (ch->position == POSITION_BASHED)
    {
      send_to_char ("You can't flurry when you have been bashed!\n\r", ch);
      return;
    }
  if (ch->position == POSITION_STOPCAST)
    {
      send_to_char ("You can't flurry after concentrating that hard!\n\r", ch);
      return;
    }
  if (FIGHTING (ch) == NULL)
    {
      send_to_char ("You can't begin a fight with a flurry; you must be already fighting!\n\r", ch);
      return;
    }
  if (ch->move < pow.flurry_mps)
    {
      send_to_char ("You are too exhausted to flurry.\n\r", ch);
      return;
    }


  victim = ch->fgt->fighting;


#ifndef OLD_FIGHT
  if (!ch->fgt || is_fighting_near (ch, victim) == -1)
    {
      send_to_char ("You must be next to your opponent to flurry into him/her/it.\n\r", ch);
      return;
    }

  if (ch->fgt && ch->fgt->field_ticks > 0)
    {
      setnext (ch, "flurry");
      return;
    }
#endif

  SUBMOVE (ch, pow.flurry_mps);
  upd_mps (ch);
  WAIT_STATE (ch, 3 * PULSE_VIOLENCE);
  act ("You open up a wild flurry of multiple attacks!", ch, NULL, victim, TO_CHAR + 1000);
  act ("$n opens up in a wild flurry of multiple attacks!", ch, NULL, victim, TO_ROOM + 1000);
  hitop = FALSE;
  multi_hit (ch, victim, TYPE_UNDEFINED);
  if (victim->data_type != 50)
    multi_hit (ch, victim, TYPE_UNDEFINED);
  if (hitop && ch->fgt && ch->fgt->field && victim && victim->fgt
      && victim->fgt->field)
    {
      java_hit_field (ch->fgt->field, ch, ch->fgt->pos_x, ch->fgt->pos_y,
		      victim->fgt->pos_x, victim->fgt->pos_y);
    }
  hitop = FALSE;
  return;
}

void
do_bs (CHAR_DATA * ch, char *argy)
{
  DEFINE_COMMAND ("bs", do_bs, POSITION_STANDING, 0, LOG_NORMAL, "See backstab.") do_backstab (ch, argy);
  return;
}

void
do_backstab (CHAR_DATA * ch, char *argy)
{
  char arg2[SML_LENGTH];
  CHAR_DATA *victim;
  SINGLE_OBJECT *obj;
  int dt;
  SPELL_DATA *spl;
  DEFINE_COMMAND ("backstab", do_backstab, POSITION_STANDING, 0, LOG_NORMAL, "This command allows you to sneak behind an opponent and place a piercing weapon in his/her/its back.")
    if ((spl = skill_lookup ("Backstab", -1)) == NULL)
    {
      send_to_char ("Huh?\n\r", ch);
      return;
    }
  one_argy (argy, arg2);
  if (arg2[0] == '\0')
    {
      send_to_char ("Backstab whom?\n\r", ch);
      return;
    }
  if ((victim = get_char_room (ch, arg2)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (arg2[0] == '\0')
      {
    if ((victim = get_char_room (ch, arg2)) == MASTER (ch))
        {
      send_to_char ("Why would you be so dirty and backstab someone you are following!?\n\r", ch);
      return;
    }
  }
  if (victim == ch)
    {
      send_to_char ("How can you sneak up on yourself?\n\r", ch);
      return;
    }
  if (is_safe (ch, victim))
    {
      return;
    }
  if (!pkill (ch, victim))
    return;
  check_pkill (ch, victim);
  if ((obj = get_item_held (ch, ITEM_WEAPON)) == NULL)
    {
      send_to_char ("You need to wield a weapon.\n\r", ch);
      return;
    }
  dt = ((I_WEAPON *) obj->more)->attack_type;
  if (attack_table[dt].hit_type != TYPE_PIERCE)
    {
      send_to_char ("You need to wield a piercing weapon.\n\r", ch);
      return;
    }
  if (FIGHTING (victim) != NULL)
    {
      send_to_char ("You can't backstab a person who is already *fighting*.\n\r", ch);
      return;
    }
  if (IS_MOB (victim) && (IS_SET (victim->pIndexData->act3, ACT3_GUARD_DOOR) 
      || IS_SET (victim->pIndexData->act3, ACT3_GUARD_DOOR_EVIL) 
      || IS_SET (victim->pIndexData->act3, ACT3_GUARD_DOOR_GOOD) 
      || victim->pIndexData->clan_guard_1 || victim->pIndexData->clan_guard_2))
    {
      send_to_char ("But you can't get behind the mob to backstab; it's guarding an exit.\n\r", ch);
      return;
    }
  if (!IS_AWAKE (victim))
    {
      do_actual_backstab (ch, argy);
      return;
    }
  send_to_char ("You begin to stealthily move behind your target...\n\r", ch);
  if (IS_PLAYER (ch))
    ch->pcdata->tickcounts = spl->mana;
  if (IS_MOB (ch))
    {
      do_actual_backstab (ch, argy);
      return;
    }

  if (strlen (argy) > 140)
    return;
  check_temp (ch);
  strcpy (ch->pcdata->temp->temp_string, argy);
  NEW_POSITION (ch, POSITION_BACKSTAB);
  return;
}
void
do_actual_backstab (CHAR_DATA * ch, char *argy)
{
  char arg[SML_LENGTH];
  CHAR_DATA *victim;
  SINGLE_OBJECT *obj;
  SINGLE_OBJECT *obj2;
  SINGLE_OBJECT *obj3;
  int dt;
  SPELL_DATA *spl;
  bool gotDualBackstab = FALSE;	// Kenor '03
  bool gotFirstBackstab = FALSE;
  if ((spl = skill_lookup ("Backstab", -1)) == NULL)
    {
      send_to_char ("Huh?\n\r", ch);
      return;
    }
  one_argy (argy, arg);
  if (ch->position == POSITION_FIGHTING)
    return;
  NEW_POSITION (ch, POSITION_STANDING);
  if (arg[0] == '\0')
    {
      send_to_char ("Backstab whom?\n\r", ch);
      return;
    }
  if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (victim == ch)
    {
      send_to_char ("How can you sneak up on yourself?\n\r", ch);
      return;
    }
  if (is_safe (ch, victim))
    {
      return;
    }
  if ((obj = get_item_held (ch, ITEM_WEAPON)) == NULL)
    {
      send_to_char ("You need to wield a weapon.\n\r", ch);
      return;
    }
  /* We can't use obj because it is referenced in the future. */
  if ((obj2 = get_eq_char (ch, WEAR_HOLD_1)) != NULL && (obj3 = get_eq_char (ch, WEAR_HOLD_2)) != NULL)
  if (obj2->pIndexData->item_type == ITEM_WEAPON && obj3->pIndexData->item_type == ITEM_WEAPON)
  if (attack_table[((I_WEAPON *) obj2->more)->attack_type].hit_type == TYPE_PIERCE 
   && attack_table[((I_WEAPON *) obj3->more)->attack_type].hit_type == TYPE_PIERCE)
	gotDualBackstab = TRUE;

  dt = ((I_WEAPON *) obj->more)->attack_type;
  if (attack_table[dt].hit_type != TYPE_PIERCE)
    {
      send_to_char ("You need to wield a piercing weapon.\n\r", ch);
      return;
    }
  if (FIGHTING (victim) != NULL)
    {
      send_to_char ("You can't backstab a person who is already *fighting*.\n\r", ch);
      return;
    }
  if (IS_AFFECTED (victim, AFF_FLYING) && !IS_AFFECTED (ch, AFF_FLYING))
    {
/*victim is flying, char is not */
      gotDualBackstab = FALSE;	// No way you can dual someone who is flying.
      if (number_range (1, 4) > 1)
	{
	  act ("You manage to jump up high enough to get your dagger to $S back...", ch, NULL, victim, TO_CHAR);
	}
      else
	{
	  act ("You can't reach $N because $E is flying too high...", ch, NULL, victim, TO_CHAR);
	  return;
	}
    }
  if (IS_PLAYER (ch))
    ch->pcdata->tickcounts = spl->casting_time;
/*  set_fighting (ch, victim);
  set_fighting (victim, ch);
*/ 
  if (IS_PLAYER (ch))
      if (number_range(1,3) == 3)
    skill_gain (ch, gsn_backstab, TRUE);
      if (number_range (1,3)==2)
    skill_gain (ch, gsn_pierce, TRUE);
  if (IS_PLAYER (ch) && gotDualBackstab && ch->pcdata->learned[gsn_dual_backstab] > 0)  // Sigh, I did this all backwards.
      if (number_range(1,3) == 3)
    skill_gain (ch, gsn_dual_backstab, TRUE);
      if (number_range (1,3)==2)
    skill_gain (ch, gsn_pierce, TRUE);
  if (!IS_AWAKE (victim)
      || IS_MOB (ch)
      || (IS_PLAYER (ch) && number_percent () < ch->pcdata->learned[gsn_backstab]))
    {
      multi_hit (ch, victim, gsn_backstab);
      gotFirstBackstab = TRUE;
    }
  else
    damage (ch, victim, 0, gsn_backstab);

  if (gotDualBackstab && gotFirstBackstab)
    if (!IS_AWAKE (victim) || IS_MOB (ch) || (IS_PLAYER (ch) 
        && number_percent () < ch->pcdata->learned[gsn_dual_backstab]))
      multi_hit (ch, victim, gsn_dual_backstab);
  return;
}
void
do_flee (CHAR_DATA * ch, char *argy)
{
  ROOM_DATA *was_in;
  ROOM_DATA *now_in;
  int fled_dir;
  CHAR_DATA *victim;
  char buf[STD_LENGTH];
  int attempt, tattempt;
  bool double_flee;
  DEFINE_COMMAND ("flee", do_flee, POSITION_FIGHTING, 0, LOG_NORMAL, "This command allows you to flee from combat.")
    check_fgt (ch);
#ifndef NEW_WORLD
  if (ch->ced && ch->ced->in_boat)
    {
      do_board (ch, "boat");
      return;
    }
#endif
  if (!FIGHTING (ch))
    {
      send_to_char ("You cannot flee if you are not fighting!\n\r", ch);
      return;
    }
  if (ch->position == POSITION_DEAD)
    return;
  if (ch->fgt->ears > 9)
    {
      send_to_char ("You are too off balance from your last combat action to flee!\n\r", ch);
      return;
    }
  double_flee = TRUE;
  if (IS_PLAYER (ch) && ch->pcdata->tickcounts > 1)
    {
      send_to_char ("You fail to find a pause in combat in which to flee!\n\r", ch);
      return;
    }
  if (ch->wait > 1)
    {
      send_to_char ("You are too off-balance and cannot flee.\n\r", ch);
      return;
    }
  WAIT_STATE (ch, 10);

  if (ch->position == POSITION_BASHED)
    {
      send_to_char ("You try to crawl away, but you are bashed to the ground!\n\rBetter stand up first...\n\r", ch);
      return;
    }
    if (number_range (1,4) > 3) 
        {
	send_to_char("You couldn't manage to flee from combat!\n\r",ch);
	return;
	}
  if ((victim = FIGHTING (ch)) == NULL)
    {
      if (ch->position == POSITION_BASHED)
	{
	  NEW_POSITION (ch, POSITION_STANDING);
	  update_pos (ch);
	  return;
	}
      NEW_POSITION (ch, POSITION_STANDING);
      update_pos (ch);
      send_to_char ("You don't have to flee if you're not fighting.\n\r", ch);
      return;
    }

  if (ch->in_room && ch->in_room->more && ch->in_room->more->linked_to)
    {
      if (number_range (1, 3) != 2)
	{
	  send_to_char ("You couldn't manage to get out of the vehicle!\n\r", ch);
	  return;
	}
      stop_fighting (ch, TRUE);
      NEW_POSITION (ch, POSITION_STANDING);
      send_to_char ("You managed to flee!\n\r", ch);
      do_leave (ch, "");
      return;
    }

  if (IS_MOB (ch))
    {
      if (number_range (1, 2) == 2)
	{
	  act ("$n tries to escape, but can't get away!", ch, NULL, ch, TO_ROOM);
	  return;
	}
      if (victim->position == POSITION_DEAD)
	{
	  stop_fighting (victim, TRUE);
	  ch->wait = 0;
	  return;
	}
      if (ch->position == POSITION_FIGHTING)
	NEW_POSITION (ch, POSITION_STANDING);
    }

  if (ch->position == POSITION_GROUNDFIGHTING)
    {
      send_to_char ("You attempt to stand up...\n\r", ch);
      do_stand (ch, "");
      return;
    }
  if (IS_PLAYER (ch) && rchars_in_group (ch) > 1)
    double_flee = FALSE;
  was_in = ch->in_room;
  for (attempt = 0; attempt < 6; attempt++)
    {
      EXIT_DATA *pexit;
      int door;
      door = number_door ();
      if ((pexit = was_in->exit[door]) == 0
	  || (ROOM_DATA *) pexit->to_room == NULL
	  || ((ROOM_DATA *) pexit->to_room)->sector_type == SECT_WATER_NOSWIM
	  /*|| ((ROOM_DATA *) pexit->to_room)->sector_type == SECT_WATER_SWIM*/
	  || (pexit->d_info && IS_SET (pexit->d_info->exit_info, EX_CLOSED))
	  || (IS_MOB (victim) && (IS_SET (victim->pIndexData->act3, ACT3_GUARD_DOOR)
		  || (IS_SET (victim->pIndexData->act3, ACT3_GUARD_DOOR_EVIL) && IS_EVIL (ch))
		  || (IS_SET (victim->pIndexData->act3, ACT3_GUARD_DOOR_GOOD) && !IS_EVIL (ch))) 
                     && victim->pIndexData && victim->pIndexData->guard == door) 
		  || (IS_MOB (ch) && IS_SET (((ROOM_DATA *) pexit->to_room)->room_flags, ROOM_NO_MOB)))
	continue;
      if (RIDING (ch) != NULL)
	{
	  send_to_char ("You fall from your mount as you try to escape!\n\r", ch);
	  check_fgt (ch->fgt->riding);
	  ch->fgt->riding->fgt->mounted_by = NULL;
	  ch->fgt->riding = NULL;
	}
      if (MOUNTED_BY (ch) != NULL)
	{
	  send_to_char ("Your rider falls off as you try to escape!\n\r", ch);
	  send_to_char ("Your mount bolts from under you!\n\r",	MOUNTED_BY (ch));
	  check_fgt (ch->fgt->mounted_by);
	  ch->fgt->mounted_by->fgt->riding = NULL;
	  ch->fgt->mounted_by = NULL;
	}
      move_char (ch, door + 20);
      if (IS_JAVA (ch))
	do_clear (ch, "");
      if ((now_in = ch->in_room) == was_in)
	continue;
      ch->in_room = was_in;
      sprintf (buf, "\x1B[1;32m%s flees %s in panic!\x1B[0m\n\r", capitalize (NAME (ch)), dir_name[door]); //might change to rNAME KILITH 05
      act (buf, ch, NULL, NULL, TO_ROOM);
      make_blood (ch, door);
      ch->in_room = now_in;
      if (IS_PLAYER (ch))
	{
	  fled_dir = door;
	  was_in = ch->in_room;
	  for (tattempt = 0; tattempt < 6; tattempt++)
	    {
	      EXIT_DATA *tpexit;
	      int tdoor;
	      tdoor = number_door ();
	      if ((tpexit = was_in->exit[tdoor]) == 0
		  || (ROOM_DATA *) tpexit->to_room == NULL
		  || ((ROOM_DATA *) tpexit->to_room)->sector_type ==
		  SECT_WATER_NOSWIM
		  || ((ROOM_DATA *) tpexit->to_room)->sector_type ==
		  SECT_WATER_SWIM 
		  || (tpexit->d_info && IS_SET (tpexit->d_info->exit_info, EX_CLOSED))
		  || (IS_MOB (victim)
		      && (IS_SET (victim->pIndexData->act3, ACT3_GUARD_DOOR)
			  || (IS_SET (victim->pIndexData->act3, ACT3_GUARD_DOOR_EVIL) && IS_EVIL (ch))
			  || (IS_SET (victim->pIndexData->act3, ACT3_GUARD_DOOR_GOOD) && !IS_EVIL (ch))) && victim->pIndexData && victim->pIndexData->guard == tdoor) 
			  || (IS_MOB (ch) && IS_SET (((ROOM_DATA *) tpexit->to_room)->room_flags, ROOM_NO_MOB)))
		continue;
	      if (fled_dir == rev_dir[tdoor])
		continue;
	      if (double_flee)
		{
		  move_char (ch, tdoor + 20);
		  if (IS_JAVA (ch))
		    do_clear (ch, "");
		  if ((now_in = ch->in_room) == was_in)
		    continue;
		}
	      break;
	    }
	  send_to_char ("Run away!  You flee head over heels!\n\r", ch);
	  gain_exp (ch, -10);
	}
      if (FIGHTING (FIGHTING (ch)) == ch)
	stop_fighting (ch, TRUE);
      else
	stop_fighting (ch, FALSE);
      return;
    }
  act ("$N won't let you get away!", ch, NULL, FIGHTING (ch), TO_CHAR);
  return;
}

void
do_rescue (CHAR_DATA * ch, char *argy)
{
  char arg[SML_LENGTH];
  CHAR_DATA *victim;
  CHAR_DATA *fch;
  int temp;
  int temp2;
  DEFINE_COMMAND ("rescue", do_rescue, POSITION_FIGHTING, 0, LOG_NORMAL, "This command allows you to rescue someone getting whacked.  Success is based on your rescue skill.")
    one_argy (argy, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Who is that?\n\r", ch);
      return;
    }
  if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("Who is that?\n\r", ch);
      return;
    }
  if ((IS_EVIL (ch) && !IS_EVIL (victim)) || (!IS_EVIL (ch) && IS_EVIL (victim)))
    {
      send_to_char ("Why would you want to do that?\n\r", ch);
      return;
    }
  if (ch->position != POSITION_STANDING && ch->position != POSITION_FIGHTING && ch->position != POSITION_STOPCAST)
    {
      send_to_char ("You must be fighting or standing to rescue someone.\n\r", ch);
      return;
    }
  if (victim == ch)
    {
      send_to_char ("You rescue yourself.  That did a lot of good.\n\r", ch);
      return;
    }
  if (IS_PLAYER (ch) && IS_MOB (victim))
    {
      send_to_char ("That person doesn't want your help!\n\r", ch);
      return;
    }
  if (FIGHTING (ch) == victim)
    {
      send_to_char ("You rescue yourself.  That was useful.\n\r", ch);
      return;
    }
  if ((fch = FIGHTING (victim)) == NULL)
    {
      send_to_char ("That person is not fighting right now.\n\r", ch);
      return;
    }
  if (victim->position != POSITION_FIGHTING)
    {
      send_to_char ("That person isn't in a normal fighting position.\n\r", ch);
      return;
    }
  if (fch->position != POSITION_FIGHTING)
    {
      send_to_char ("That person's opponent is not in a normal fighting position!\n\r", ch);
      return;
    }
  if (victim->position == POSITION_GROUNDFIGHTING)
    {
      send_to_char ("That person is groundfighting! You can't rescue!\n\r", ch);
      return;
    }
  if (victim->position == POSITION_GROUNDFIGHTING)
    {
      if (ch->position == POSITION_GROUNDFIGHTING)
	{
	  send_to_char ("You are groundfighting! You can't rescue!\n\r", ch);
	  return;
	}
      set_fighting (ch, FIGHTING (victim));
      return;
    }
  WAIT_STATE (ch, 15);
  if (IS_PLAYER (ch) && number_percent () > ch->pcdata->learned[gsn_rescue])
    {
      send_to_char ("You attempt the rescue but fail.\n\r", ch);
      return;
    }
  act ("You rescue $N!", ch, NULL, victim, TO_CHAR);
  act ("$n rescues you!", ch, NULL, victim, TO_VICT);
  act ("$n rescues $N!", ch, NULL, victim, TO_NOTVICT);
  check_fgt (fch);
  check_fgt (ch);
  check_fgt (victim);
  temp = victim->fgt->combat_delay_count;
  temp2 = fch->fgt->combat_delay_count;

  if (IS_PLAYER (ch))
    {
      stop_fighting (fch, FALSE);
      stop_fighting (victim, FALSE);
    }

  check_fgt (fch);
  check_fgt (victim);
  set_fighting (ch, fch);
  set_fighting (fch, ch);
  check_fgt (fch);
  check_fgt (victim);
  victim->fgt->combat_delay_count = temp;
  fch->fgt->combat_delay_count = temp2;
  WAIT_STATE (ch, 22);
  if (IS_PLAYER (ch))
    {
      skill_gain (ch, gsn_rescue, TRUE);
    }
  victim->fgt->combat_delay_count = temp;
  fch->fgt->combat_delay_count = temp2;
  return;
}

void
do_kick (CHAR_DATA * ch, char *argy)
{
  CHAR_DATA *victim;
  int ii, i;
  bool notft = FALSE;
  int dl = 0;
  int iii = 1;
  bool gotSecondKick = FALSE;	// Kenor '03
  DEFINE_COMMAND ("kick", do_kick, POSITION_FIGHTING, 0, LOG_NORMAL, "This command allows you to kick an opponent.  Success is based on kicking skill, power is based on profession and level.")
    check_fgt (ch);
  if (ch->wait > 2)
    return;
  if (ch->fgt->ears > 9)
    {
      send_to_char ("You can't kick this soon after a bash.\n\r", ch);
      return;
    }
  if (FIGHTING (ch) == NULL)
    {
      notft = TRUE;
      if (argy[0] == '\0' || argy == "")
	{
	  send_to_char ("Who?\n\r", ch);
	  return;
	}
      if ((victim = get_char_room (ch, argy)) == NULL)
	{
	  send_to_char ("Who?\n\r", ch);
	  return;
	}
      if (victim == ch)
	{
	  send_to_char ("You kick yourself in the butt, but it doesn't hurt that much.\n\r", ch);
	  return;
	}
    }
  else
    victim = ch->fgt->fighting;

  if (victim->data_type == 50)
    return;

  if (is_safe (ch, victim))
    {
      return;
    }
  if (FIGHTING (ch) != victim)
    {
      if (!pkill (ch, victim))
	return;
      check_pkill (ch, victim);
    }
  check_ced (ch);
  check_ced (victim);
  if (!(IS_IN_MELEE (ch, victim)))
    {
      send_to_char ("You aren't on the front lines of that battle!\n\r", ch);
      return;
    }
 if (ch->position == POSITION_STOPCAST)
    {
    send_to_char ("You can not try to kick someone after trying to cast! You would fall on your ass!!!\n\r", ch);
    NEW_POSITION (ch, POSITION_STANDING);
    return;
    }  
  if (ch->position == POSITION_BASHED)
    {
      send_to_char ("You try to kick, but from the ground you can't hit your opponent.\n\r", ch);
      return;
    }
  if (ch->position == POSITION_GROUNDFIGHTING)
    {
      send_to_char ("You can't kick when you are locked into mortal combat on the ground!\n\r", ch);
      return;
    }
  if (kickable (victim) == 0)
    {
      act ("You can't kick that type of creature!", ch, NULL, victim, TO_CHAR);
      return;
    }
  if ((IS_PROF (ch, PROF_MONK)) && (LEVEL (ch) > 19))
    dl = 50;
  else
    dl = 33;

  if (notft)
    dl += (2 * PULSE_VIOLENCE);
  WAIT_STATE (ch, dl);
  if (kickable (victim) == 2)
    {
      act ("You try to kick $N, but $E flies out of the way!", ch, NULL, victim, TO_CHAR);
      return;
    }
  set_fighting (ch, victim);
  /*set_fighting(victim,ch); */
  if (IS_PROF (ch, PROF_MONK) && (LEVEL (ch) >= 45))
    {
      if (number_range (1, 10) < 4)
	i = 3;
      else
	i = 2;
    }
  else if (IS_PROF (ch, PROF_MONK) && (LEVEL (ch) < 45) && (LEVEL (ch) > 19))
    {
      if (number_range (1, 10) < 6)
	i = 2;
      else
	i = 1;
    }
  else
    i = 1;
  for (; i > 0; i--)
    {
      if (IS_MOB (ch) || (!IS_AWAKE (victim) || (IS_PLAYER (ch) && number_percent () < ch->pcdata->learned[gsn_kick])))
	{
	  char buf[500];
	  int jj;
	  int damm;
	  int damm2;		// Kenor '03
	  damm = translate (damt.kick_dam, LEVEL (ch), ch);

	  /*  Second Kick - Kenor '03 
	   *  This is nested in the above
	   *  if check because it requires
	   *  you to hit the first time.
	   */
	  if (number_percent () < ch->pcdata->learned[gsn_second_kick])
	    {
	      gotSecondKick = TRUE;
	      damm2 = translate (damt.kick_dam2, LEVEL (ch), ch);
	    }

	  if (ch->pcdata->learned[gsn_second_kick] > 0)
	    {
	      if (IS_PLAYER (ch) && number_range (1, 3) == 2)
		skill_gain (ch, gsn_second_kick, TRUE);
	    }
	  if (HAS_LEGS_STRENGTH_IMPLANT (ch))
	    {
	      damm += number_range (3, 4);
	      damm2 += number_range (3, 4);
	    }
          if (HAS_FEET_IMPLANT (ch))
            {
              damm += number_range (3, 4);
              damm2 += number_range (3, 4);
            }
	  if (IS_PLAYER (ch) && IS_PROF (ch, PROF_MONK))
	    {
	      damm += 1;
	      damm2 += 1;
	    }
	  if (LEVEL (ch) > 7 && number_range (1, 2) == 2)
	    {
	      damm += 1;
	      damm2 += 1;
	    }
	  if (LEVEL (ch) > 16)
	    {
	      damm += 1;
	      damm2 += 1;
	    }
	  if (LEVEL (ch) > 31)
	    {
	      damm += 1;
	      damm2 += 1;
	    }
	  if (LEVEL (ch) > 54)
	    {
	      damm += 1;
	      damm2 += 1;
	    }
	  if (LEVEL (ch) > 67)
	    {
	      damm += 1;
	      damm2 += 1;
	    }
	  if (LEVEL (ch) > 81)
	    {
	      damm += 1;
	      damm2 += 1;
	    }

	  if (IS_PLAYER (ch) && number_range (1, 3) == 2)
	    skill_gain (ch, gsn_kick, TRUE);

	  if (CHALLENGE (ch) == 10)
	    {
	      jj = number_range (1, 3);
	      if (jj == 1)
		sprintf (buf, "($B$1Arena$N$7) $B%s kicks %s in the ribs... snap crackle pop!", NAME (ch), NAME (victim));
	      if (jj == 2)
		sprintf (buf, "($B$1Arena$N$7) $B%s kicks %s's knee; that's gotta hurt!", NAME (ch), NAME (victim));
	      if (jj == 3)
		sprintf (buf, "($B$1Arena$N$7) $B%s's kick catches %s in the side of the head!", NAME (ch), NAME (victim));
	      do_arenaecho (ch, buf, TRUE);
	    }
	  if (can_yell (victim))
	    {
	      if (damm < 5)
		{
		  act ("\x1B[35;1mYour weak kick barely bruises $N's shin.\x1B[37;0m", ch, NULL, victim, TO_CHAR);
		  act ("\x1B[37;1m$n's weak kick barely bruises $N's shin.\x1B[0m", ch, NULL, victim, TO_NOTVICT);
		  act ("\x1B[36;1m$n's weak kick bruises your shin.\x1B[37;0m", ch, NULL, victim, TO_VICT);
		}
	      else
		{
		  jj = number_range (1, 5);
		  if (jj == 1)
		    {
		      act ("\x1B[35;1mYour kick slams into $N's shin!\x1B[37;0m", ch, NULL, victim, TO_CHAR);
		      act ("\x1B[37;1m$n's kick slams into $N's shin.\x1B[0m", ch, NULL, victim, TO_NOTVICT);
		      act ("\x1B[36;1m$n's kick slams into your shin!\x1B[37;0m", ch, NULL, victim, TO_VICT);
		    }
		  else if (jj == 2)
		    {
		      act ("\x1B[35;1mYour kick smashes into $N's solar plexus!\x1B[37;0m", ch, NULL, victim, TO_CHAR);
		      act ("\x1B[37;1m$n kicks $N directly in the solar plexus!\x1B[0m", ch, NULL, victim, TO_NOTVICT);
		      act("\x1B[36;1m$n kicks you in the solar plexus! Ouch!\x1B[37;0m", ch, NULL, victim, TO_VICT);
		    }
		  else if (jj == 3)
		    {
		      act ("\x1B[35;1mYour kick smashes into $N's knee!\x1B[37;0m", ch, NULL, victim, TO_CHAR);
		      act ("\x1B[37;1m$n kicks $N's knee very hard!\x1B[0m", ch, NULL, victim, TO_NOTVICT);
		      act ("\x1B[36;1m$n kicks your knee, and you nearly fall over...\x1B[37;0m", ch, NULL, victim, TO_VICT);
		    }
		  else if (jj == 4)
		    {
		      act ("\x1B[35;1mYou lift your leg high, and kick $N in the head!!\x1B[37;0m", ch, NULL, victim, TO_CHAR);
		      act ("\x1B[37;1m$n kicks $N's right in the face! Ouch...\x1B[0m", ch, NULL, victim, TO_NOTVICT);
		      act ("\x1B[36;1m$n kicks you in the face!\x1B[37;0m", ch, NULL, victim, TO_VICT);
		      act ("\x1B[31;1mBlood spurts from $N's nose!\x1B[37;0m", ch, NULL, victim, TO_CHAR);
		      act ("\x1B[31;1mBlood spurts from $N's nose!\x1B[37;0m", ch, NULL, victim, TO_NOTVICT);
		      act ("\x1B[31;1mBlood pours forth from your injured nose!\x1B[37;0m", ch, NULL, victim, TO_VICT);
		    }
		  else if (jj == 5)
		    {
		      act ("\x1B[35;1mYour kick connects with $N's ribs!\x1B[37;0m", ch, NULL, victim, TO_CHAR);
		      act ("\x1B[37;1m$n kicks $N in the ribs!\x1B[0m", ch, NULL, victim, TO_NOTVICT);
		      act ("\x1B[36;1m$n kicks you in the ribs!! Ouch!\x1B[37;0m", ch, NULL, victim, TO_VICT);
		    }
		}
	    }
	  else
	    {
	      act ("\x1B[35;1mWham! Your kick slams into $N!\x1B[37;0m", ch, NULL, victim, TO_CHAR);
	      act ("\x1B[37;1m$n's kick slams into $N!\x1B[37;0m", ch, NULL, victim, TO_NOTVICT);
	      act ("\x1B[37;1m$n's kick slams into you hard!\x1B[37;0m", ch, NULL, victim, TO_VICT);
	    }

	  show_hitdam (gsn_kick, "kick", damm, ch, victim);
	  damage (ch, victim, damm, gsn_kick);
	  if (gotSecondKick == TRUE)
         
	    {
	      damage (ch, victim, damm2, gsn_second_kick);
	    }
	}
      else
	{
	  set_fighting (ch, victim);
	  if (FIGHTING (victim) == NULL)
	    victim->fgt->fighting = ch;
	  act ("You miss a kick at $N.", ch, NULL, victim, TO_CHAR + 1000);
	  act ("$n misses a kick at $N.", ch, NULL, victim, TO_NOTVICT + 1000);
	  act ("$n tries to kick you but misses.", ch, NULL, victim, TO_VICT + 1000);
	}
    }
  return;
}
//Thrust code for Marauders
void
do_thrust (CHAR_DATA * ch, char *argy)
{
  CHAR_DATA *victim;
   SINGLE_OBJECT *wield;
  int dam;
  int damm;
  DEFINE_COMMAND ("thrust", do_thrust, POSITION_FIGHTING, 0, LOG_NORMAL, "This command lets you impale an opponent with a polearm.")
    check_fgt (ch);
  if (ch->wait > 4)
    return;
  if (FIGHTING(ch) == NULL)
        {
      send_to_char("You cannot start combat by thrusting your dagger.  Use KILL instead.\n\r",ch);
       return;
        }
  WAIT_STATE (ch, PULSE_VIOLENCE);
  if (ch->fgt->ears > 9)
    {
      send_to_char ("You can't a dagger at someone this soon after a bash.\n\r", ch);
      return;
    }

  if (victim == ch)
    {
      send_to_char("No. Are you nuts?\n\r", ch);
      WAIT_STATE(ch, 10 * PULSE_VIOLENCE);
      return;
    }
  if (FIGHTING (ch) == NULL)
    {
      if (argy[0] == '\0' || argy == "")
        {
          send_to_char ("Who?\n\r", ch);
          WAIT_STATE(ch, 1*PULSE_VIOLENCE);
          return;
        }
      if ((victim = get_char_room (ch, argy)) == NULL)
        {
          send_to_char ("Who?\n\r", ch);
          WAIT_STATE(ch, PULSE_VIOLENCE);
          return;
        }
    }
  else
    victim = ch->fgt->fighting;

  check_ced (ch);
  check_ced (victim);
  if (ch->position == POSITION_BASHED)
    {
      send_to_char ("You can not take a thrust with your dagger at someone when you are sitting on your ass!\n\r", ch);
      WAIT_STATE(ch, 4 * PULSE_VIOLENCE);
      return;
    }
  if (ch->position == POSITION_GROUNDFIGHTING)
    {
      send_to_char ("You can't thrust your dagger at someone while you are groundfighting!\n\r", ch);
      WAIT_STATE(ch, PULSE_VIOLENCE);
      return;
    }
  if((wield = get_item_held(ch, ITEM_WEAPON)) == NULL)
    {
      send_to_char ("You must be wielding a weapon!\n\r", ch);
      WAIT_STATE(ch, 1 * PULSE_VIOLENCE);
      return;
    }
   if (wield != NULL && wield->pIndexData->item_type == ITEM_WEAPON)
    {
      dam = dice (FIRSTDICE (wield), SECONDDICE (wield));
    }

  if ((wield == NULL) || !IS_OBJ_STAT(wield, ITEM_PIERCE))
    {
      send_to_char ("You must be wielding a dagger!\n\r", ch);
      WAIT_STATE(ch, 1 * PULSE_VIOLENCE);
      return;
    }
  if (number_percent() >= ch->pcdata->learned[gsn_thrust])
    {
      send_to_char("\x1B[1;30mYou attempt to thrust your weapon into your victim but fail!\n\r\x1B[37;0m", ch);
      WAIT_STATE(ch, 8 * PULSE_VIOLENCE);
      return;
     }

  if (FIGHTING(ch) != victim);
  set_fighting (ch, victim);

  act("$R$4You lunge toward $N with $p.$R$7", ch, wield, victim,TO_CHAR);
  act("$R$4$n lunges toward you with $s $p.$R$7", ch, wield, victim,TO_VICT);
  act("$R$4$n lunges toward $N with $s $p.$R$7", ch, wield,victim,TO_NOTVICT);

      int remort = (ch->pcdata->remort_times);
      int level = LEVEL(ch);
      int str = get_curr_str(ch);
      int dmr = GET_DAMROLL (ch);
      int dex = get_curr_dex(ch);
      int rand = number_range(10, 20);
      int idam = dam;

    damm = ((idam) + (remort * 2) + (str/2) + (level/4) + (dmr * 3 / 2) + (dex/2) + (rand));
if (HAS_ARMS_DEXTERITY_IMPLANT (ch))
   {
   damm += 15;
   }
//show_hitdam (gsn_thrust, "thrust", damm, ch, victim);
//victim->hit -= damm;
damage (ch, victim, damm, gsn_thrust);

  if (IS_PLAYER (ch)) skill_gain (ch, gsn_thrust, TRUE);
  WAIT_STATE(ch, 8 * PULSE_VIOLENCE);

    {
      if (((I_WEAPON *)wield->more)->damage_p > 20)
        ((I_WEAPON *)wield->more)->damage_p -= 1;
      else
        {
          send_to_char ("\x1B[1;31mYour dagger just broke under the stress.\x1B[37;0m\n\r",ch);
          unequip_char (ch, wield);
          obj_from(wield);
          free_it(wield);
        }
    }
  return;
}
//end Thrust
//Charge code
/*void
do_charge (CHAR_DATA * ch, char *argy)
{
  CHAR_DATA *victim;
   SINGLE_OBJECT *wield;
  int dam;
  int damm;
  DEFINE_COMMAND ("charge", do_charge, POSITION_FIGHTING, 0, LOG_NORMAL, "This command lets you impale an opponent with a polearm.")
    check_fgt (ch);
  if (ch->wait > 4)
    return;
  if (FIGHTING(ch) == NULL)
        {
      send_to_char("You cannot start combat with a rush.  Use KILL instead.\n\r",ch);
       return;
        }
  WAIT_STATE (ch, PULSE_VIOLENCE);
  if (ch->fgt->ears > 9)
    {
      send_to_char ("You can't charge this soon after a bash.\n\r", ch);
      return;
    }
  if (victim == ch)
    {
      send_to_char("No. Are you nuts?\n\r", ch);
      WAIT_STATE(ch, 10 * PULSE_VIOLENCE);
      return;
    }
  if (FIGHTING (ch) == NULL)
    {
      if (argy[0] == '\0' || argy == "")
        {
          send_to_char ("Who?\n\r", ch);
          WAIT_STATE(ch, 1*PULSE_VIOLENCE);
          return;
        }
      if ((victim = get_char_room (ch, argy)) == NULL)
        {
          send_to_char ("Who?\n\r", ch);
          WAIT_STATE(ch, PULSE_VIOLENCE);
          return;
        }
    }
  else
    victim = ch->fgt->fighting;

  check_ced (ch);
  check_ced (victim);
  if (ch->position == POSITION_BASHED)
    {
      send_to_char ("You can not charge at someone when you are sitting on your ass!\n\r", ch);
      WAIT_STATE(ch, 4 * PULSE_VIOLENCE);
      return;
    }
  if (ch->position == POSITION_GROUNDFIGHTING)
    {
      send_to_char ("You can't charge someone while you are groundfighting!\n\r", ch);
      WAIT_STATE(ch, PULSE_VIOLENCE);
      return;
    }
  if((wield = get_item_held(ch, ITEM_WEAPON)) == NULL)
    {
      send_to_char ("You must be wielding a weapon!\n\r", ch);
      WAIT_STATE(ch, 1 * PULSE_VIOLENCE);
      return;
    }
   if (wield != NULL && wield->pIndexData->item_type == ITEM_WEAPON)
    {
      dam = dice (FIRSTDICE (wield), SECONDDICE (wield));
    }

  if ((wield == NULL) || !IS_OBJ_STAT(wield, ITEM_LANCE))
    {
      send_to_char ("You must be wielding a lance!\n\r", ch);
      WAIT_STATE(ch, 1 * PULSE_VIOLENCE);
      return;
    }
  if (number_percent() >= ch->pcdata->learned[gsn_charge])
    {
      send_to_char("\x1B[1;30mYou attempt to thrust your weapon into your victim but fail!\n\r\x1B[37;0m", ch);
      WAIT_STATE(ch, 8 * PULSE_VIOLENCE);
      return;
     }

  if (FIGHTING(ch) != victim);
  set_fighting (ch, victim);

  act("$R$4You lunge toward $N with $p.$R$7", ch, wield, victim,TO_CHAR);
  act("$R$4$n lunges toward you with $s $p.$R$7", ch, wield, victim,TO_VICT);
  act("$R$4$n lunges toward $N with $s $p.$R$7", ch, wield,victim,TO_NOTVICT);

      int remort = (ch->pcdata->remort_times);
      int level = LEVEL(ch);
      int str = get_curr_str(ch);
      int dmr = GET_DAMROLL (ch);
      int wis = get_curr_wis(ch);
      int rand = number_range(10, 20);
      int ride = (RIDING (ch) != NULL);
      int idam = dam; 

    damm = ((idam * 5)
        + (remort * 2)
        + ((str * 3) / 4)
        + (level/2)
        + (dmr * 3 / 2)
        + ((wis * 3) / 4)
        + (ride * 60)
        + (rand));
if (HAS_ARMS_STRENGTH_IMPLANT (ch))
   {
   damm += 25;
   }
show_hitdam (gsn_charge, "charge", damm, ch, victim);
victim->hit -= damm;

  if (IS_PLAYER (ch)) skill_gain (ch, gsn_charge, TRUE);
  WAIT_STATE(ch, 8 * PULSE_VIOLENCE);

    {
      if (((I_WEAPON *)wield->more)->damage_p > 15)
        ((I_WEAPON *)wield->more)->damage_p -= 5;
      else
        {
          send_to_char ("\x1B[1;31mYour lance just snapped under the stress.\x1B[37;0m\n\r",ch);
          unequip_char (ch, wield);
          obj_from(wield);
          free_it(wield);
        }
    }
  return;
}*/ //removing the charge code from the game
//End thrust

void
do_disarm (CHAR_DATA * ch, char *argy)
{
  CHAR_DATA *victim;
  SINGLE_OBJECT *obj;
  int percent;
  DEFINE_COMMAND ("disarm", do_disarm, POSITION_FIGHTING, 0, LOG_NORMAL, "This command allows you to disarm an opponent.  Success is based on the disarm skill.")
    if (CHALLENGE (ch) == 10)
    {
      send_to_char ("You cannot disarm in the arena.\n\r", ch);
      return;
    }
  if (ch->wait > 2)
    {
      send_to_char ("You are too off balance to disarm right now.\n\r", ch);
      return;
    }
  if (IS_PLAYER (ch) && ch->pcdata->tickcounts > 3)
    {
      send_to_char ("You are too off balance to disarm right now.\n\r", ch);
      return;
    }

  if (IN_BATTLE (ch) == TRUE)
    {
      send_to_char ("You cannot disarm while in the battleground.\n\r", ch);
      return;
    }
  if (get_item_held (ch, ITEM_WEAPON) == NULL)
    {
      send_to_char ("You must wield a weapon to disarm.\n\r", ch);
      return;
    }
  if ((victim = FIGHTING (ch)) == NULL)
    {
      send_to_char ("You aren't fighting anybody or anything.\n\r", ch);
      return;
    }
  if ((obj = get_item_held (victim, ITEM_WEAPON)) == NULL)
    {
      send_to_char ("Your opponent is not wielding a weapon.\n\r", ch);
      return;
    }
  WAIT_STATE (ch, 50);
  if (HAS_HANDS_STRENGTH_IMPLANT (victim) && !HAS_HANDS_STRENGTH_IMPLANT (ch))
    {
      act ("$N's incredible, almost unnatural grip remains completely in control of the weapon it holds.", ch, NULL, victim, TO_CHAR);
      act ("Your bionic hands thwart $n's attempt at disarming you.", ch, NULL, victim, TO_VICT);
      return;
    }
  percent = number_percent () + LEVEL (victim) - (LEVEL (ch) * 2);
  if (IS_MOB (ch) || (IS_PLAYER (ch) && percent < ch->pcdata->learned[gsn_disarm] * 2 / 3))
    {
      disarm (ch, victim);
      skill_gain (ch, gsn_disarm, FALSE);
    }
  else
    send_to_char ("You tried, but you could not disarm your opponent.\n\r", ch);
  return;
}

void
do_sla (CHAR_DATA * ch, char *argy)
{
  DEFINE_COMMAND ("sla", do_sla, POSITION_DEAD, 110, LOG_NORMAL, "This is so you don't try to slap someone and slay em instead!")
    send_to_char ("Slay?  Slam?  Sleep?  What do you mean?\n\r", ch);
  return;
}

void
do_slay (CHAR_DATA * ch, char *argy)
{
  CHAR_DATA *victim;
  char arg[SML_LENGTH];
  DEFINE_COMMAND ("slay", do_slay, POSITION_DEAD, 110, LOG_ALWAYS, "This command slays a mob or player.") 
  if (IS_MOB (ch))
    return;
  if (!IS_REAL_GOD (ch))
    {
      send_to_char ("Command has been removed.\n\r", ch);
      return;
    }
  one_argy (argy, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Who is that?\n\r", ch);
      return;
    }
  if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("Who is that?\n\r", ch);
      return;
    }
  if (ch == victim)
    {
      send_to_char ("You aren't allowed to slay yourself.  Go see a psychiatrist.\n\r", ch);
      return;
    }
  if (IS_PLAYER (victim) && LEVEL (ch) < 111)
    {
      send_to_char ("You shouldn't be slaying other players.\n\r", ch);
      return;
    }
  act ("You grab $S head and you squeeze hard... until $N's $B$7skull$R explodes into a $B$4bloody $Rmess!", ch, NULL, victim, TO_CHAR);
  act ("$n grabs your head and squeezes hard... until your $B$7eyes $Rpop out of your head! $B$4Blood $Rflies everywhere and $B$5ch$R$4un$B$5ks $Rof your $B$5br$R$4a$B$5in $Rand then everything goes $B$0dark.$R$1..$R", ch, NULL, victim, TO_VICT);
  act ("$n grabs ahold of $N's head and squeezes hard.  Pretty soon $N's entire head and $B$7skull$R are just $B$4bloody $Rpieces of $R$6flesh$R-and-$B$5br$R$4a$B$5in-$B$5ch$R$4un$B$5ks$R on the ground.", ch, NULL, victim, TO_NOTVICT);
  raw_kill (victim, FALSE);
  return;
}

bool
hit_suck_disarm (CHAR_DATA * ch, CHAR_DATA * victim, int hit, int dam)
{
  SINGLE_OBJECT *obj;
  if (number_range (1, 100) >= LEVEL (ch)
      && (obj = get_item_held (victim, ITEM_WEAPON)) != NULL
      && hand_empty (ch) != WEAR_NONE)
    {
      unequip_char (victim, obj);
      obj->wear_loc = hand_empty (ch);
      obj_from (obj);
      obj_to (obj, ch);
      act ("Your weapon sucks $N's weapon right out of $S hand!", ch, NULL, victim, TO_CHAR);
      act ("$n's weapon sucks your weapon right out of your hand!", ch, NULL, victim, TO_VICT);
      act ("$n's magical weapon sucks $N's weapon right out of $S hand!", ch, NULL, victim, TO_NOTVICT);
      return TRUE;
    }
  return FALSE;
}

bool
hit_vorpal (CHAR_DATA * ch, CHAR_DATA * victim, int hit, int dam)
{
  char buf[STD_LENGTH];
  SINGLE_OBJECT *obj;
  char *name;
  if (!IS_REAL_GOD (ch))
    {
      return FALSE;
    }
  if (number_range (1, 1000) >= 999)
    {
      act ("With a mighty blow, you seperate $N's head from $S body!", ch, 0, victim, TO_CHAR);
      act ("Everything goes black as $n seperates your head from your body with a mighty slash!", ch, 0, victim, TO_VICT);
      act ("$n's might slash seperates $N's head from $S body!", ch, 0, victim, TO_NOTVICT);
      act ("$n's severed head falls to the ground with a thud.", victim, 0, 0, TO_ROOM);
      stop_fighting (victim, TRUE);
      name = NAME (ch);
      obj = create_object (get_obj_index (OBJ_VNUM_SEVERED_HEAD), 0);
      obj->timer = number_range (4, 7);
      sprintf (buf, STR (obj, short_descr), NAME (victim));
      if (obj->short_descr && obj->short_descr != NULL)
	free_string (obj->short_descr);
      obj->short_descr = str_dup (buf);
      sprintf (buf, STR (obj, description), NAME (victim));
      if (obj->description && obj->description != NULL)
	free_string (obj->description);
      obj->description = str_dup (buf);
      obj_to (obj, ch->in_room);
      make_corpse (victim);
      if (IS_MOB (victim))
	{
	  gain_exp (ch, victim->exp);
	  victim->pIndexData->killed++;
	  extract_char (victim, TRUE);
	  return TRUE;
	}
      extract_char (victim, FALSE);
      while (victim->affected)
	affect_remove (victim, victim->affected);
      if (IS_PLAYER (victim))
	victim->affected_by = victim->pcdata->nat_abilities;
      victim->armor = 100;
      NEW_POSITION (victim, POSITION_RESTING);
      victim->hit = UMAX (1, victim->hit);
      victim->move = UMAX (1, victim->move);
      save_char_obj (victim);
      return TRUE;
    }
  return FALSE;
}

void
skill_gain (CHAR_DATA * ch, short gsn, bool report)
{
  char buf[STD_LENGTH];
  bool learnyes;
  int vall[10];
  SPELL_DATA *spell;
  if (IS_MOB (ch))
    return;
  if (!(spell = skill_lookup (NULL, gsn)))
    return;			        // I hope you can still improve on the skills below,
  if (spell->spell_level > LEVEL (ch))  // i.e. I hope they can be found in the has list.
    return;
#ifdef NEW_WORLD
  if (gsn == gsn_slashing || gsn == gsn_whip || gsn == gsn_concussion || gsn == gsn_pierce ||  gsn == gsn_calm || gsn == gsn_rush)
    {
      int ii;
      vall[0] = 550;
      vall[1] = 450;
      vall[2] = 350;
      vall[3] = 300;
      vall[4] = 360;
      vall[5] = 420;
      vall[6] = 500;
      vall[7] = 660;
      vall[8] = 725;
      vall[9] = 890;
      for (ii = 0; ii < 10; ii++)
	vall[ii] += 125;
    }
  else
    {
      spell = skill_lookup (NULL, gsn);
      if (spell == NULL)
	return;
      if (spell->slot != 0)
	{
	  vall[0] = 220;
	  vall[1] = 110;
	  vall[2] = 60;
	  vall[4] = 34;
	  vall[5] = 40;
	  vall[6] = 55;
	  vall[7] = 65;
	  vall[8] = 75;
	  vall[9] = 95;
	}
      else
	{
	  vall[0] = 250;
	  vall[1] = 230;
	  vall[2] = 170;
	  vall[3] = 142;
	  vall[4] = 115;
	  vall[5] = 115;
	  vall[6] = 155;
	  vall[7] = 250;
	  vall[8] = 350;
	  vall[9] = 500;
	}
    }
  learnyes = FALSE;
  if (ch->pcdata->learned[gsn] < 5)
    return;
  if (ch->pcdata->learned[gsn] < 20 && number_range (1, vall[0]) == 29) learnyes = TRUE;
  else if (ch->pcdata->learned[gsn] < 40 && number_range (1, vall[1]) == 29) learnyes = TRUE;
  else if (ch->pcdata->learned[gsn] < 60 && number_range (1, vall[2]) == 29) learnyes = TRUE;
  else if (ch->pcdata->learned[gsn] < 70 && number_range (1, vall[3]) == 29) learnyes = TRUE;
  else if (ch->pcdata->learned[gsn] < 78 && number_range (1, vall[4]) == 20) learnyes = TRUE;
  else if (ch->pcdata->learned[gsn] < 84 && number_range (1, vall[5]) == 29) learnyes = TRUE;
  else if (ch->pcdata->learned[gsn] < 87 && number_range (1, vall[6]) == 29) learnyes = TRUE;
  else if (ch->pcdata->learned[gsn] < 92 && number_range (1, vall[7]) == 29) learnyes = TRUE;
  else if (ch->pcdata->learned[gsn] < 97 && number_range (1, vall[8]) == 29) learnyes = TRUE;
  else if (number_range (1, vall[9]) == 69) learnyes = TRUE;
  if (learnyes && ch->pcdata->learned[gsn] < 100)
    {
      {
	SPELL_DATA *spl;
	spl = skill_lookup (NULL, gsn);
	if (spl == NULL)
	  sprintf (buf, "ERROR!! PLEASE REPORT TO KILITH!\n\r");
	else
	  sprintf (buf, "\x1B[37;1m**** [You have become better at %s!] ****\x1B[0m\n\r", spl->spell_funky_name);
	send_to_char (buf, ch);
        send_to_char ("\x1B[37;1m**** [You have gained 1000 experience!] ****\x1B[0m\n\r", ch);
        gain_exp (ch, 1000);
      }
      ch->pcdata->learned[gsn]++;
    }
#else
  if (gsn == gsn_slashing || gsn == gsn_whip || gsn == gsn_concussion || gsn == gsn_pierce)
    {
      int ii;
      vall[0] = 950;
      vall[1] = 850;
      vall[2] = 720;
      vall[3] = 600;
      vall[4] = 560;
      vall[5] = 620;
      vall[6] = 700;
      vall[7] = 760;
      vall[8] = 825;
      vall[9] = 1190;
      for (ii = 0; ii < 10; ii++)
	vall[ii] += 290;
    }
  else
    {
      spell = skill_lookup (NULL, gsn);
      if (spell == NULL)
	return;
      if (spell->slot != 0)
	{
	  vall[0] = 280;
	  vall[1] = 150;
	  vall[2] = 90;
	  vall[4] = 44;
	  vall[5] = 60;
	  vall[6] = 75;
	  vall[7] = 85;
	  vall[8] = 95;
	  vall[9] = 125;
	}
      else
	{
	  vall[0] = 450;
	  vall[1] = 330;
	  vall[2] = 210;
	  vall[3] = 172;
	  vall[4] = 195;
	  vall[5] = 215;
	  vall[6] = 255;
	  vall[7] = 350;
	  vall[8] = 450;
	  vall[9] = 600;
	}
    }
  learnyes = FALSE;
  if (ch->pcdata->learned[gsn] < 10)
    return;
  if (ch->pcdata->learned[gsn] < 20 && number_range (1, vall[0]) == 69) learnyes = TRUE;
  else if (ch->pcdata->learned[gsn] < 40 && number_range (1, vall[1]) == 69) learnyes = TRUE;
  else if (ch->pcdata->learned[gsn] < 60 && number_range (1, vall[2]) == 69) learnyes = TRUE;
  else if (ch->pcdata->learned[gsn] < 70 && number_range (1, vall[3]) == 69) learnyes = TRUE;
  else if (ch->pcdata->learned[gsn] < 78 && number_range (1, vall[4]) == 40) learnyes = TRUE;
  else if (ch->pcdata->learned[gsn] < 84 && number_range (1, vall[5]) == 69) learnyes = TRUE;
  else if (ch->pcdata->learned[gsn] < 87 && number_range (1, vall[6]) == 69) learnyes = TRUE;
  else if (ch->pcdata->learned[gsn] < 92 && number_range (1, vall[7]) == 69) learnyes = TRUE;
  else if (ch->pcdata->learned[gsn] < 97 && number_range (1, vall[8]) == 69) learnyes = TRUE;
  else if (number_range (1, vall[9]) == 69) learnyes = TRUE;
  if (learnyes && ch->pcdata->learned[gsn] < 100)
    {
      {
	SPELL_DATA *spl;
	spl = skill_lookup (NULL, gsn);
	if (spl == NULL)
	  sprintf (buf, "ERROR!! PLEASE REPORT TO KILITH!\n\r");
	else
	  sprintf (buf, "\x1B[37;1m**** [You have become better at %s!] ****\x1B[0m\n\r", spl->spell_funky_name);
	send_to_char (buf, ch);
        send_to_char ("\x1B[37;1m**** [You have gained 1000 experience!] ****\x1B[0m\n\r", ch);
        gain_exp (ch, 1000);
      }
      ch->pcdata->learned[gsn]++;
    }
#endif
  return;
}
