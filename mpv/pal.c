#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"

void 
shield_rush (CHAR_DATA * ch, char * argy) 
   {
  CHAR_DATA *victim;
  SINGLE_OBJECT *obj;
  SPELL_DATA *spl;
  int chan;
  bool notft = FALSE;
  int tmpi;
  char buf[STD_LENGTH];
  int dam;
  int damm;	  
  DEFINE_COMMAND("rush", shield_rush, POSITION_FIGHTING, 0, LOG_ALWAYS, "Shield Rush")
  check_fgt (ch);
  chan = 0;
 
  if (IS_MOB(ch) || !is_member(ch,GUILD_PALADIN)) 
    {
      send_to_char("This skill is only available to paladins.\n\r",ch);
      return;
        }
  if (!ch) 
      return;
  if ((spl = skill_lookup ("Shield Rush", -1)) == NULL) 
        {
      send_to_char ("Huh?\n\r", ch);
       return;
        }
  if (ch->wait > 2) 
	  return;
  if ((obj = get_eq_char(ch,WEAR_SHIELD)) == NULL) 
        {
      send_to_char("You can't rush without a shield or buckler on your arm.\n\r",ch);
       return;
        }
  if (!FIGHTING(ch)) 
        {
      send_to_char("You cannot start combat with a rush.  Use KILL instead.\n\r",ch);
       return;
        } 
  if ( (ch->move < 20) || ( ( ch->move - 20 ) < 0) ) 
        {
      send_to_char("You don't have enough moves to rush!\n\r",ch);
       return;
        }
  if (ch->fgt->ears > 9) 
	{
      send_to_char ("You can't shield rush at this moment.\n\r", ch);	  
       return; 
      }  
  if (ch->in_room->sector_type == SECT_UNDERWATER)
	{
      send_to_char ("Rush someone underwater? I don't think so...\n\r", ch);
       return;
  	} 
   if ((FIGHTING (ch) == NULL || ch->position != POSITION_FIGHTING) && (argy == "" || argy[0] == '\0')) 
	{
      send_to_char ("Rush whom?\n\r", ch);	 
       return;
      }
   if (FIGHTING (ch) == NULL)
    {
     notft = TRUE;
	if ((victim = get_char_room (ch, argy)) == NULL)
        {
        if (argy[0] == '\0')
	   {
         send_to_char ("Rush who?\n\r", ch);
          return;
           }
         send_to_char ("They aren't here.\n\r", ch);
          return;
  	   }
	if ((obj = get_eq_char(ch,WEAR_SHIELD)) == NULL) 
           {
	 send_to_char("You can't rush without a shield or buckler on your arm.\n\r",ch);
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
      send_to_char ("You can't rush someone when you are riding!\n\r", ch);
       return;
      }
  if (is_safe (ch, victim))
	   return;
  check_ced (ch);
  check_ced (victim);
    
  if (!(IS_IN_MELEE (ch, victim)))
	      {
         send_to_char ("You aren't on the front lines of that battle!\n\r", ch);     
	 return;		      
	      }

 if (FIGHTING (ch) != victim)
    {
     if (!pkill(ch, victim))
     return;
     check_pkill (ch, victim);
    }

  if (!can_see (ch, victim)) 
      {
    send_to_char ("How can you rush someone you can't see!?\n\r", ch);	  
     return;
    }
  if (ch->position == POSITION_GROUNDFIGHTING) 
     {
    act ("You are wrestling on the ground! You can't rush $N!", ch, NULL, victim, TO_CHAR);
     return;
    }
 
  if (victim->position == POSITION_GROUNDFIGHTING) 
      {
    send_to_char ("Your victim is groundfighting!\n\r", ch);
     return;
    }
  
  if (ch->position == POSITION_BASHED) 
     { 
    act ("You are bashed.. how do you expect to rush $N?", ch, NULL, victim, TO_CHAR);
     return;
    }
   
  if (!bashable (victim)) 
     {
    act ("You can't rush that kind of creature!", ch, NULL, victim, TO_CHAR);
     return;
    }
   check_fgt(victim);   
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
  if (number_range (1, 11) == 3 && IS_AWAKE (victim) && IS_AFFECTED (victim, AFF_FLYING) && !IS_AFFECTED (ch, AFF_FLYING))
      {
     act ("$N quickly flies out of the way of $n's rush!", ch, NULL, victim, TO_NOTVICT + 1000);	        
     act ("$N quickly flies out of the way of your rush!", ch, NULL, victim, TO_CHAR + 1000);
     act ("You quickly fly out of the way of $n's rush!", ch, NULL, victim, TO_VICT + 1000);           
        WAIT_STATE (ch, 4 * PULSE_VIOLENCE);
	 return;
        }
  if (IS_MOB (victim))
       tmpi = 10;
    else
       tmpi = (15 + get_curr_dex (victim) + (victim->pcdata->learned[gsn_dodge] / 6));
     if (IS_MOB (ch)) 
         chan = (LEVEL (ch) * 2);
       else    
         chan = ((ch->pcdata->learned[gsn_rush]) + (get_curr_str (ch) * 2)) - tmpi;
  if (IS_PLAYER (ch) && ch->pcdata->learned[gsn_rush] < 5)
	     chan = 5;
       if (chan < 5)
	      chan = 5;
         if (chan > 155)
	       chan = number_range (130, 160);
	 
    if (number_range (0, 210) > chan && IS_AWAKE (victim)) 
            {                                       
       act ("You miss the rush and almost fall on the ground!", ch, NULL, victim, TO_CHAR + 1000);
       act ("$n tries to rush you, but almost falls over instead!", ch, NULL, victim, TO_VICT + 1000);
       act ("$n misses a rush at $N and almost falls over!", ch, NULL, victim, TO_NOTVICT + 1000);
      if (notft) 
          {
	WAIT_STATE (ch, (double) 7.3 * PULSE_VIOLENCE);
          }
      else
	WAIT_STATE (ch, (double) 5.3 * PULSE_VIOLENCE);
      
      SUBMOVE(ch, 5);
      
      if (FIGHTING (victim) == NULL)
	    set_fighting (victim, ch);
            set_fighting (ch, victim);
	    return;
        }

 if (RIDING (victim) != NULL) 
       {
      sprintf (buf, "$B$4Your rush sends $N flying off of %s!$R$7", PERS (RIDING (victim), ch)); act (buf, ch, NULL, victim, TO_CHAR);
      sprintf (buf, "$B$4$n's rush sends $N flying off of %s!$R$7", NAME (RIDING (victim))); act (buf, ch, NULL, victim, TO_NOTVICT);
      sprintf (buf, "$B$4You fly off of %s as $n rushes you!$R$7", PERS (RIDING (victim), victim));
       SUBMOVE(ch,25);
       SUBHIT(ch,15);
  int level = LEVEL(ch);
  int str = get_curr_str(ch);
  int dmr = GET_DAMROLL (ch);
  int wis = get_curr_wis(ch);
  damm = ((level/3)
          + (str/2)
          + (dmr/2)
          + (wis/2)
          + number_range (10, 20));
	show_hitdam (gsn_rush, "rush", damm, ch, victim);
        victim->hit -= dam;
       //damage (ch, victim, dam, gsn_rush); 
     
      act (buf, ch, NULL, victim, TO_VICT);
          check_fgt (victim->fgt->riding);
      victim->fgt->riding->fgt->mounted_by = NULL;
         victim->fgt->riding = NULL;
	    }
  else if (RIDING (victim) == NULL)
    {
 if (number_range (1, 18) != 4 || !can_groundfight (victim) || LEVEL (victim) < 12 || !can_groundfight (ch) || LEVEL (ch) < 12 || !pow.bash_slip_to_tackle)
	        {
 char buf[400];
      act ("$B$4Using your $R$p $B$4you send $0$N $4sprawling onto the ground!$R$7", ch, obj, victim, TO_CHAR + 1000);
      act ("$B$4$n sends $0$N $4sprawling backwards on to the ground with $s $R$p!$R$7",ch, obj, victim, TO_NOTVICT + 1000);
      act ("$B$4$n sends you sprawling backwards on to the ground with $s $R$p!$R$7",ch, obj, victim, TO_VICT + 1000);
   SUBMOVE(ch,25);
   SUBHIT(ch,15);
  int level = LEVEL(ch);
  int str = get_curr_str(ch);
  int dmr = GET_DAMROLL (ch);
  int wis = get_curr_wis(ch);
  dam = ((level/3)
          + (str/2)
          + (dmr/2)
          + (wis/2)
          + number_range (1, 10));

        show_hitdam (gsn_rush, "rush", dam, ch, victim);
        victim->hit -= dam;

      //damage (ch, victim, dam, gsn_rush);
       
      if (IS_PLAYER (ch))
	skill_gain (ch, gsn_rush, TRUE);
          if (CHALLENGE (ch) == 10)
	  {
      sprintf (buf, "($B$1Arena$N$7) $B%s just sent %s flying with a rush!", NAME (ch), NAME (victim));              
      do_arenaecho (ch, buf, TRUE);
            }
	      }
           else
	       {
	 char buf[500];
	 int i;
         act ("$B$5You rush $N, but trip and stumble! You are now groundfighting!$R$7", ch, NULL, victim, TO_CHAR);
         act ("$B$5$N rushs $N, but trips and stumbles! They are now groundfighting!$R$7", ch, NULL, victim, TO_NOTVICT);
         act ("$B$5$n rushes you, but trips and looses his balance and falls on top of you. You begin to ground-fight!!$R$7", ch, NULL, victim, TO_VICT);
         if (CHALLENGE (ch) == 10)
           {
	   i = number_range (1, 3);
	   if (i == 1)
sprintf (buf, "($B$1Arena$N$7) $B%s tries to rush %s but they both fall to the\n\r($B$1Arena$N$7) $Bground in a bloody tangle of elbows and teeth...", NAME (ch), NAME (victim));
	   if (i == 2)
sprintf (buf, "($B$1Arena$N$7) $B%s falls on his face, trying to rush %s.\n\r($B$1Arena$N$7) $BThey both fall over and begin clawing at each other!", NAME (ch), NAME (victim));
           if (i == 3)
sprintf (buf, "($B$1Arena$N$7) $B%s tries to rush %s, but %s catches %s\n\r($B$1Arena$N$7) $Bwith an elbow and they both fall to the ground in a bloody mess!", NAME (ch), NAME (victim), NAME (victim), NAME (ch));
	   do_arenaecho (ch, buf, TRUE);
	    }
       set_fighting (ch, victim);
       victim->fgt->fighting = ch;
       NEW_POSITION(victim, POSITION_GROUNDFIGHTING);
       NEW_POSITION(ch, POSITION_GROUNDFIGHTING);
       return;
       }
    }
  {
   SPELL_DATA *s;
    if ((s = skill_lookup (NULL, gsn_rush)) == NULL)
            {
            fprintf (stderr, "Rush not found!\n");
            return;
           }
//Crash Area 

    if(victim != NULL && victim->fgt != NULL && victim->fgt->fighting != NULL) 
     {
   victim->fgt->combat_delay_count = 4;
  
  if (victim->position != POSITION_FIGHTING)
   victim->fgt->fighting = ch;
   victim->position = POSITION_BASHED;
   set_fighting (ch, victim);
   victim->fgt->ears = 10 + (s->casting_time);
   ch->fgt->ears = 10 + (s->casting_time * 2);
        }
   }
  return;

  
} //closing statement
