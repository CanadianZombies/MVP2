#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"

bool inventory = FALSE;

//copied showloot from LOC code. Hope it works here. Kilith 05
void
do_showloot (CHAR_DATA * ch, char *argy)
{
  CHAR_DATA *fch;
  DEFINE_COMMAND ("showloot", do_showloot, POSITION_DEAD, 0, LOG_NORMAL, "This command shows what is currently in your inventory.")
  send_to_char("You show off your inventory to the room.\n\r", ch);
  WAIT_STATE(ch, 60);
  for (fch = ch->in_room->more->people; fch != NULL; fch = fch->next_in_room)
  {
    if (((IS_EVIL (ch)) && (IS_EVIL (fch))) || ((!IS_EVIL (ch)) && (!IS_EVIL (fch))))
      show_list_to_char(ch->carrying, fch, TRUE, TRUE);
  }
  return;
} // END SHOWLOOT

void
do_where (CHAR_DATA * ch, char *argy)
{
  char buf[STD_LENGTH];
  char arg[SML_LENGTH];
  CHAR_DATA *victim;
  DESCRIPTOR_DATA *d;
  bool found;
  int pass;
  DEFINE_COMMAND ("where", do_where, POSITION_RESTING, 0, LOG_NORMAL, "This command shows you characters nearby, and what area you are in.")
    one_argy (argy, arg);
  if (!pow.allow_where)
    {
      send_to_char ("Huh?\n\r", ch);
      return;
    }
  if (IS_MOB (ch))
    return;
  sprintf (buf, "\x1B[0;37mYou are in \x1B[1;31m%s\x1B[0;37m, built by \x1B[1;31m%s\x1B[1;31m",
	   ch->in_room->area->name, ch->in_room->area->builders);
  send_to_char (buf, ch);
  send_to_char ("\x1B[0m.\n\r", ch);
  if (LEVEL (ch) > 100 && LEVEL (ch) < 110)
    return;
  send_to_char ("Players near you:\n\r", ch);
  found = FALSE;
  for (pass = 0; pass < 3; pass++)
    {
      for (d = descriptor_list; d; d = d->next)
	{
	  victim = d->character;
	  if (!victim)
	    continue;
	  if (!victim->in_room)
	    continue;
	  if (IS_MOB (victim))
	    continue;
	  if (pass == 0 && !IS_IMMORTAL (victim))
	    continue;
	  if ((pass == 1 || pass == 2) && IS_IMMORTAL (victim))
	    continue;
	  if (pass == 1 && !IS_EVIL (victim))
	    continue;
	  if (pass == 2 && !IS_GOOD (victim))
	    continue;
	  if (can_see (ch, victim) && !not_is_same_align (ch, victim))
	    {
	      if (LEVEL (ch) >= 100)
		{
		  sprintf (buf, "%s %s %-20s [%5d] %s\n\r",
			   victim->in_room->area ==
			   ch->in_room->area ? "(A)" : "   ",
			   IS_IMMORTAL (victim) ? "\e[37;1m[I]\e[37;0m"
			   : (IS_EVIL (victim) ? "\e[31;1m[E]\e[37;0m" :
			      "\e[32;1m[G]\e[37;0m"), NAME (victim),
			   victim->in_room->vnum,
			   strip_ansi_codes (show_room_name
					     (ch, victim->in_room->name)));
		  send_to_char (buf, ch);
		  found = TRUE;
		}
	      else if (d->connected == CON_PLAYING)
		{
		  if ((victim->in_room->area == ch->in_room->area) &&
		      victim != ch && !not_is_same_align (victim, ch))
		    {
		      found = TRUE;
		      sprintf (buf, "%s %-28s %s\n\r",
			       is_same_group (ch,
					      victim) ? "\e[32;1m[G]\e[0;37m"
			       : "   ", NAME (victim),
			       strip_ansi_codes (show_room_name
						 (ch,
						  victim->in_room->name)));
		    }
		  else
		    buf[0] = '\0';
		  send_to_char (buf, ch);
		}
	    }			// if can_see
	}			// for d
    }				// for pass
  if (!found)
    send_to_char ("No one else can be found.\n\r", ch);


  // check if both good and evil in some area
  // code by Eraser
  if (IS_REAL_GOD (ch))
    {
      bool alist[1000][2];
      AREA_DATA *ad;
      for (ad = area_first; ad; ad = ad->next)
	{
	  alist[ad->vnum][0] = FALSE;
	  alist[ad->vnum][1] = FALSE;
	}
      for (d = descriptor_list; d; d = d->next)
	{
	  if ((victim = d->character) && IS_PLAYER (victim) &&
	      !IS_IMMORTAL (victim) && victim->in_room)
	    alist[victim->in_room->area->vnum][IS_EVIL (victim)] = TRUE;
	}
      for (ad = area_first; ad; ad = ad->next)
	{
	  if (!str_cmp (ad->name, "Limbo"))
	    continue;
	  if (alist[ad->vnum][0] && alist[ad->vnum][1])
	    {
	      sprintf (buf,
		       "Both Good and Evil players detected in area %s!\n\r",
		       ad->name);
	      send_to_char (buf, ch);
	    }
	}
    }
  return;
}

/*
   Scan_direction scans up to three rooms away, or until a mob is found
   in the direction passed.
 */
void
scan_direction (CHAR_DATA * ch, int dir)
{
  EXIT_DATA *pexit;
  ROOM_DATA *troom;
  CHAR_DATA *mob;
  char final[STD_LENGTH];
  char buf[STD_LENGTH];
  bool fMultiple;
  int n = 0, max_iteration;
  hugebuf_o[0] = '\0';
  final[0] = '\0';
  max_iteration = 3;
  pexit = ch->in_room->exit[dir];
  if (pexit == NULL || (ROOM_DATA *) pexit->to_room == NULL)
    return;
  troom = (ROOM_DATA *) pexit->to_room;
  if (troom->linkage)
    {
      sprintf (buf,
	       "You can make out some sort of vehicle/vessel to the %s.\n\r",
	       dir_name[URANGE (0, dir, MAX_DIR - 1)]);
      send_to_char (buf, ch);
    }
  if (troom != NULL
      && (!pexit->d_info || (!IS_SET (pexit->d_info->exit_info, EX_CLOSED)))
      && room_is_dark (troom) && !IS_AFFECTED (ch, AFF_INFRARED))
    {
      if (troom->more && troom->more->people != NULL)
	{
	  sprintf (buf, "You can make out some motion to the %s.\n\r",
		   dir_name[URANGE (0, dir, MAX_DIR - 1)]);
	  send_to_char (buf, ch);
	}
      return;
    }
#ifndef NEW_WORLD
  if (troom != NULL
      && !(room_is_dark (troom) && !IS_AFFECTED (ch, AFF_INFRARED)))
    {
      if (troom->more && troom->more->people != NULL
	  && troom->more->people->ced
	  && troom->more->people->ced->in_boat != NULL)
	{
	  sprintf (buf, "You can make out some sort of boat to the %s.\n\r",
		   dir_name[URANGE (0, dir, MAX_DIR - 1)]);
	  send_to_char (buf, ch);
	  return;
	}
    }
#endif
  n = 0;
  while ((pexit != NULL) && (n < max_iteration) &&
	 ((ROOM_DATA *) pexit->to_room != NULL) && (!pexit->d_info
						    ||
						    (!IS_SET
						     (pexit->d_info->
						      exit_info, EX_CLOSED)
						     && pexit->d_info->
						     exit_info != 1)))
    {
      fMultiple = FALSE;
      troom = (ROOM_DATA *) pexit->to_room;
      if (troom != NULL && troom->more && troom->more->people != NULL)
	{
	  int cnnt;
	  cnnt = 0;
	  for (mob = troom->more->people; mob != NULL;
	       mob = mob->next_in_room)
	    {
	      if (!can_see_nextdoor (ch, mob) || !can_see (ch, mob))
		continue;
	      cnnt++;
	      if (cnnt > 1 && mob->next_in_room == NULL
		  && ((mob != troom->more->people)
		      || (troom->more->people->next_in_room != NULL
			  && mob != troom->more->people->next_in_room)))
		{
		  fMultiple = TRUE;
		  sprintf (buf, " and %s", rNAME (mob, ch));
		}
	      else if (mob->next_in_room != NULL && can_see_nextdoor (ch, mob)
		       && mob->next_in_room->next_in_room != NULL)
		sprintf (buf, "%s, ", rNAME (mob, ch));
	      else
		sprintf (buf, "%s", rNAME (mob, ch));
	      strcat (final, buf);
	    }
	  if (cnnt > 0)
	    {
	      sprintf (buf, " %s %s \x1B[37;1m%s\x1B[0m from here.\n\r",
		       fMultiple ? "are" : "is", dist_name[URANGE (0, n, 7)],
		       dir_name[URANGE (0, dir, MAX_DIR - 1)]);
	      strcat (final, buf);
	    }
	  if (final[0] != '\0')
	    {
	      final[0] = UPPER (final[0]);
	      strcpy (hugebuf_o, final);
	      send_to_char (hugebuf_o, ch);
	    }
	  final[0] = '\0';
	  return;
	}
      pexit = troom->exit[dir];
      n++;
    }
  return;
}

void
show_list_to_char (SINGLE_OBJECT * list, CHAR_DATA * ch, bool fShort,
		   bool fShowNothing)
{
#ifdef NEW_WORLD
  char buf[STD_LENGTH];
  char prgpstrShow[200][150];
  int prgnShow[200];
  char pstrShow[150];
#else
  char buf[STD_LENGTH];
  char prgpstrShow[200][250];
  int prgnShow[200];
  char pstrShow[250];
#endif
  SINGLE_OBJECT *obj;
  int nShow;
  int iShow;
  int count;
  bool fCombine;
  count = 0;
  bzero (&prgpstrShow, sizeof (prgpstrShow));
  bzero (&prgnShow, sizeof (prgnShow));
  bzero (&pstrShow, sizeof (pstrShow));

  if (list == NULL)
    return;
  nShow = 0;
  for (obj = list; obj && obj != NULL; obj = obj->next_content)
    {
#ifndef NEW_WORLD
      if (ch->ced && ch->ced->in_boat == obj)
	continue;
#endif
      if (obj->wear_loc == WEAR_NONE && can_see_obj (ch, obj)
	  && ((!IS_SET (obj->extra_flags, ITEM_UNSEEN) &&
	       !IS_SET (obj->pIndexData->extra_flags, ITEM_UNSEEN))
	      || IS_SET (ch->act, PLR_HOLYLIGHT)))
	{

	  if (IS_JAVA (ch))
	    java_obj_window (ch, obj);

	  strcpy (pstrShow, format_obj_to (obj, ch, fShort));
	  fCombine = FALSE;
	  if (IS_MOB (ch) || IS_SET (ch->pcdata->act2, PLR_COMBINE))
	    {
	      for (iShow = nShow - 1; iShow >= 0; iShow--)
		{
		  if (!strcmp (prgpstrShow[iShow], pstrShow))
		    {
		      prgnShow[iShow]++;
		      fCombine = TRUE;
		      break;
		    }
		}
	    }
	  if (!fCombine)
	    {
	      strcpy (prgpstrShow[nShow], pstrShow);
	      prgnShow[nShow] = 1;
	      nShow++;
	      if (nShow >= 200)
		{
		  fprintf (stderr, "Error.  nShow>=200.  Ch: %s.\n",
			   NAME (ch));
		  return;
		}
	    }
	}
    }
  for (iShow = 0; iShow < nShow; iShow++)
    {
      if (IS_MOB (ch) || IS_SET (ch->pcdata->act2, PLR_COMBINE))
	{
	  if (prgnShow[iShow] != 1)
	    {
	      sprintf (buf, "[x%d] ", prgnShow[iShow]);
	      send_to_char (buf, ch);
	    }
	}
      send_to_char (prgpstrShow[iShow], ch);
      if (fShort)
	send_to_char ("\x1B[0m", ch);
    }
  if (fShowNothing && nShow == 0)
    {
      if (IS_MOB (ch) || IS_SET (ch->pcdata->act2, PLR_COMBINE))
	send_to_char ("Nothing.\n\r", ch);
    }
  return;
}

void
show_char_to_char_0 (CHAR_DATA * victim, CHAR_DATA * ch)
{
  char buf[STD_LENGTH * 5];
  char to_show[STD_LENGTH * 5];
  bool tflag;
  bool fll = FALSE;
  bool rcselect;
  buf[0] = '\0';
  rcselect = FALSE;
  tflag = FALSE;
  to_show[0] = '\0';
  buf[0] = '\0';
  if (IS_PLAYER (ch) && IS_PLAYER (victim) && not_is_same_align (ch, victim)
      && (NO_PKILL (ch) || NO_PKILL (victim)))
    return;

  if (IS_JAVA (ch))
    java_mob_window (ch, victim);

#ifndef NEW_WORLD
  if (is_in_same_boat (ch, victim))
    {
      char yyy[500];
      sprintf (yyy, "\x1B[35;1m%s is here, in the boat.\n\r", NAME (victim));
      send_to_char (yyy, ch);
      return;
    }
  if ((!ch->ced || ch->ced->in_boat == NULL)
      && (victim->ced && victim->ced->in_boat != NULL))
    return;
#endif
  if (!IS_SET (ch->act, PLR_HOLYLIGHT) && !can_see (ch, victim))
    return;
  if (MOUNTED_BY (victim) != NULL
      && victim->fgt->mounted_by->in_room == ch->in_room
      && MOUNTED_BY (victim) == ch)
    {
      act ("\x1B[1;37mYou are riding $N.\x1B[0m", ch, NULL, victim,
	   TO_CHAR_SPAM);
      return;
    }
  if (IS_PLAYER (victim) && victim->desc == NULL)
    strcat (buf, "\x1B[1;34m>LINKLESS< \x1B[0m");
  if (IS_PLAYER (victim))
    send_to_char ("\x1B[1;35m", ch);
  else
    send_to_char ("\x1B[1;36m", ch);
  to_show[0] = '\0';
  if (IS_PLAYER (ch) && IS_PLAYER (victim) &&
      ((!IS_EVIL (ch) && IS_EVIL (victim))
       || (IS_EVIL (ch) && !IS_EVIL (victim))))
    {
      strcpy (to_show, rNAME (victim, ch));
      rcselect = TRUE;
    }
  if (MOUNTED_BY (victim) != NULL)
    return;
  if (!rcselect)
    {
      if (victim->position == POSITION_STANDING && RIDING (victim) == NULL
	  && LD (victim) && LD (victim)[0] != '\0')
	{
	  strcpy (to_show, LD (victim));
	  tflag = FALSE;
	}
      else
	{
	  strcpy (to_show, PERS (victim, ch));
	  tflag = TRUE;
	}
    }
  if (IS_PLAYER (ch) && (ch->pcdata->no_spam == 2 || ch->pcdata->no_spam == 1)
      && !tflag)
    {
      AFFECT_DATA *paf;
      if (to_show[strlen (to_show) - 1] == '\n')
	to_show[strlen (to_show) - 1] = '\0';
      if (to_show[strlen (to_show) - 2] == '\r')
	to_show[strlen (to_show) - 3] = '\0';
      if (to_show[strlen (to_show) - 2] == '\n')
	to_show[strlen (to_show) - 2] = '\0';
      if (to_show[strlen (to_show) - 3] == '\r')
	to_show[strlen (to_show) - 4] = '\0';
      if (to_show[strlen (to_show) - 3] == '\n')
	to_show[strlen (to_show) - 3] = '\0';
      if (to_show[strlen (to_show) - 1] == '\r')
	to_show[strlen (to_show) - 2] = '\0';
      for (paf = victim->affected; paf; paf = paf->next)
	{
	  if (paf->short_msg)
	    {
	      strcat (to_show, " ");
	      strcat (to_show, anseval (paf->short_msg, ch));
	    }
	}
    //if (IS_AFFECTED (victim, AFF_CHARM))
     //	strcat (to_show, " \x1B[1;32m(Servant/Trained)\x1B[0m");
      if (victim->is_undead == TRUE)
        strcat (to_show, " \x1B[1;32m(\x1B[0;33mUndead\x1B[1;32m)\x1B[0m");
      if (IS_AFFECTED (victim, AFF_INVISIBLE))
	strcat (to_show, " \x1B[0;36m(Invis)\x1B[0m");
      if (IS_AFFECTED (victim, AFF_HIDE))
	strcat (to_show, " \x1B[1;30m(Hidden)\x1B[0m");
      if (IS_AFFECTED (victim, AFF_SANCTUARY))
	strcat (to_show, " \x1B[1;34m(Sanctuary)\x1B[0m");
      if (is_affected (victim, 75))
	strcat (to_show, " \x1B[0;1m(\x1B[0mStoneskin\x1B[1m)\x1B[0m");
      //if (IS_AFFECTED (victim, AFF_FAERIE_FIRE))
	//strcat (to_show, " \x1B[1;35m(Outlined)\x1B[0m");
      if (IS_AFFECTED (victim, AFF_FLYING))
	strcat (to_show, " \x1B[1;35m(Flying)\x1B[0m");
      strcat (buf, to_show);
      strcat (buf, "\n\r");
      send_to_char (buf, ch);
      return;
    }
  strcat (buf, to_show);
  if (tflag || rcselect)
    {
      buf[0] = UPPER (buf[0]);
      /*
         if (IS_PLAYER (victim) && !IS_SET (ch->pcdata->act2, PLR_BRIEF) && !rcselect &&
         strlen (victim->pcdata->title) < 39)
         {
         strcat (buf, " ");
         strcat (buf, victim->pcdata->title);
         }
       */
      if (RIDING (victim) != NULL
	  && victim->fgt->riding->in_room == victim->in_room)
	{
	  char buf2[STD_LENGTH];
	  sprintf (buf2, " is here, riding %s.", PERS (RIDING (victim), ch));
	  strcat (buf, buf2);
	}
      else
	{
	  switch (victim->position)
	    {
	    case POSITION_DEAD:
	      strcat (buf, " is DEAD!!");
	      break;
	    case POSITION_MORTAL:
	      strcat (buf, " is mortally wounded.");
	      break;
	    case POSITION_INCAP:
	      strcat (buf, " is incapacitated.");
	      break;
	    case POSITION_STUNNED:
	      strcat (buf, " is lying here, stunned.");
	      break;
	    case POSITION_SLEEPING:
	      strcat (buf, " is asleep here.");
	      break;
	    case POSITION_BASHED:
	      strcat (buf, ", is bashed to the ground.\n\r");
	      break;
	    case POSITION_MEDITATING:
	      strcat (buf, " is meditating here.");
	      break;
	    case POSITION_SEARCHING:
	      strcat (buf, " is searching for something here.");
	      break;
	    case POSITION_CASTING:
	      strcat (buf, ", who is beginning to cast a spell, is here.");
	      break;
	    case POSITION_FALLING:
	      strcat (buf, " is falling to the ground!");
	      break;
	    case POSITION_RESTING:
	      strcat (buf, " is resting here.");
	      break;
	    case POSITION_STANDING:
	      strcat (buf, " is here.");
	      break;
	    case POSITION_FIGHTING:
	      strcat (buf, " is here, fighting ");
	      if (FIGHTING (victim) == NULL)
		strcat (buf, "thin air??\n\r");
	      else if (FIGHTING (victim) == ch)
		strcat (buf, "you!\n\r");
	      else if (FIGHTING (victim)
		       && victim->in_room == victim->fgt->fighting->in_room)
		{
		  strcat (buf, rNAME (FIGHTING (victim), ch));
		  strcat (buf, ".\n\r");
		}
	      else
		strcat (buf, "someone who left??\n\r");
	      break;
	    case POSITION_GROUNDFIGHTING:
	      strcat (buf, " is here, fighting on the ground with ");
	      if (FIGHTING (victim) == NULL)
		strcat (buf, "thin air??\n\r");
	      else if (FIGHTING (victim) == ch)
		strcat (buf, "YOU!\n\r");
	      else if (FIGHTING (victim)
		       && victim->in_room == victim->fgt->fighting->in_room)
		{
		  strcat (buf, rNAME (FIGHTING (victim), ch));
		  strcat (buf, ".\n\r");
		}
	      else
		strcat (buf, "someone who left??\n\r");
	      break;
	    }
	}
    }
  if (victim->position != POSITION_FIGHTING &&
      victim->position != POSITION_GROUNDFIGHTING
      && victim->position != POSITION_BASHED)
    {
      if (IS_PLAYER (ch)
	  && (ch->pcdata->no_spam == 2 || ch->pcdata->no_spam == 1))
	{
	  AFFECT_DATA *paf;
	  strcpy (to_show, buf);
	  if (IS_PLAYER (victim) && victim->pcdata->bounty > 0)
	    strcat (to_show, " \x1B[1;37m(Outlaw)\x1B[0;37;0m");
	  for (paf = victim->affected; paf; paf = paf->next)
	    {
	      if (paf->short_msg)
		{
		  strcat (to_show, " ");
		  strcat (to_show, anseval (paf->short_msg, ch));
		}
	    }
	  if (IS_AFFECTED (victim, AFF_CHARM))
	    strcat (to_show, " \x1B[1;32m(\x1B[0;33mUndead\x1B[1;32m)\x1B[0m");
	  if (IS_AFFECTED (victim, AFF_INVISIBLE))
	    strcat (to_show, " \x1B[0;36m(Invis)\x1B[0m");
	  if (IS_AFFECTED (victim, AFF_HIDE))
	    strcat (to_show, " \x1B[1;30m(Hidden)\x1B[0m");
	  if (IS_AFFECTED (victim, AFF_SANCTUARY))
	    strcat (to_show, " \x1B[1;34m(Sanctuary)\x1B[0m");
	  if (is_affected (victim, 75))
	    strcat (to_show, " \x1B[0;1m(\x1B[0mStoneskin\x1B[1m)\x1B[0m");
	  //if (IS_AFFECTED (victim, AFF_FAERIE_FIRE))
	    //strcat (to_show, " \x1B[1;35m(Outlined)\x1B[0m");
	  if (IS_AFFECTED (victim, AFF_FLYING))
	    strcat (to_show, " \x1B[1;35m(Flying)\x1B[0m");
	  strcat (to_show, "\n\r");
	  send_to_char (to_show, ch);
	  return;
	}
    }
  send_to_char (buf, ch);
  if (IS_PLAYER (ch) && ch->pcdata->no_spam == 0)
    {
      AFFECT_DATA *paf = victim->affected;
      if (victim->position != POSITION_FIGHTING
	  && victim->position != POSITION_BASHED
	  && victim->position != POSITION_GROUNDFIGHTING && (tflag
							     || rcselect)
	  && (!fll))
	send_to_char ("\n\r", ch);
      for (paf = victim->affected; paf; paf = paf->next)
	{
	  if (paf->long_msg)
	    act (paf->long_msg, ch, NULL, victim, TO_CHAR);
	}
      if (IS_AFFECTED (victim, AFF_CHARM))
	act ("\x1B[1;32m...$N is \x1B[0;33m>> \x1B[1;32mUndead \x1B[0;33m<<.\x1B[0m", ch, NULL, victim,
	     TO_CHAR);
      if (IS_AFFECTED (victim, AFF_INVISIBLE)
	  && IS_AFFECTED (victim, AFF_HIDE))
	act
	  ("\x1B[1;36m...$N is \x1B[0;36minvisible\x1B[1;36m and \x1B[1;30mhidden\x1B[1;36m.\x1B[0m",
	   ch, NULL, victim, TO_CHAR);
      else
	{
	  if (IS_AFFECTED (victim, AFF_INVISIBLE))
	    act ("\x1B[1;36m...$N is \x1B[0;36minvisible\x1B[1;36m.\x1B[0m",
		 ch, NULL, victim, TO_CHAR);
	  if (IS_AFFECTED (victim, AFF_HIDE))
	    act ("\x1B[1;30m...$N is hidden.\x1B[0m", ch, NULL, victim,
		 TO_CHAR);
	}
      if (IS_AFFECTED (victim, AFF_SANCTUARY))
	{
	  if (IS_EVIL (victim))
	    act ("\x1B[0;36m...$N is protected by a \x1B[1;30mdark\x1B[0;36m aura!\x1B[0m", ch, NULL, victim, TO_CHAR);
	  else
	    act ("\x1B[0;36m...$N is protected by a \x1B[1;37mbright\x1B[0;36m aura!\x1B[0m", ch, NULL, victim, TO_CHAR);
	}
      if (is_affected (victim, 75))
	act ("\x1B[0m...$N's skin looks like it is made of stone.", ch, NULL,
	     victim, TO_CHAR);
      /*if (IS_AFFECTED (victim, AFF_FAERIE_FIRE))
	act ("\x1B[1;35m...$N's body is outlined for easy targeting!\x1B[0m",
	     ch, NULL, victim, TO_CHAR);*/
      if (IS_AFFECTED (victim, AFF_FLYING))
	{
	  if (IS_PLAYER (victim))
	    act ("\x1B[0;1;35m...$N is flying in the air.\x1B[0m",
		 ch, NULL, victim, TO_CHAR);
	  else
	    act ("\x1B[0;1;35m...$N is flying around here.\x1B[0m", ch, NULL,
		 victim, TO_CHAR);
	}
      return;
    }
  return;
}

void
show_char_to_char_1 (CHAR_DATA * victim, CHAR_DATA * ch)
{
  char buf[STD_LENGTH];
  SINGLE_OBJECT *obj;
  int iWear;
  int percent, percent2;
  bool found;

  if (IS_PLAYER (ch) && IS_PLAYER (victim))
    {
      send_to_char (victim->pcdata->name, ch);
      if (LEVEL (victim) < 10)
	send_to_char (" is an inexperienced member of the \x1B[1m", ch);
      else if (LEVEL (victim) < 25)
	send_to_char (" is a member of the \x1B[1m", ch);
      else if (LEVEL (victim) < 45)
	send_to_char (" is well respected amongst the \x1B[1m", ch);
      else if (LEVEL (victim) < 70)
	send_to_char (" is a hero amongst the \x1B[1m", ch);
      else if (LEVEL (victim) < 100)
	send_to_char (" is a legend amongst the \x1B[1m", ch);
      else
	send_to_char (" is a God who belongs to the \x1B[1m", ch);
      send_to_char (race_info[victim->pcdata->race].name, ch);
      send_to_char ("\x1B[0m race.\n\r", ch);
    }
  if (victim == ch)
    {
      sprintf (buf, "You are %d\' %d\" tall.", ch->height / 12,
	       ch->height % 12);
    }
  else
    {
      sprintf (buf, "$E is");
      if (ch->height > victim->height)
	strcat (buf, " shorter than you.");
      else if (ch->height < victim->height)
	strcat (buf, " taller than you.");
      else
	strcat (buf, " the same height as you.");
    }
  act (buf, ch, NULL, victim, TO_CHAR);
  if (DSC (victim) && DSC (victim)[0] != '\0')
    {
      send_to_char (DSC (victim), ch);
    }
  else
    {
      act ("You don't see anything special about $M.", ch, NULL, victim,
	   TO_CHAR);
    }
  if (victim->max_hit > 0)
    percent = (100 * victim->hit) / victim->max_hit;
  else
    percent = 0;
  if (victim->max_move > 0)
    percent2 = (100 * victim->move) / victim->max_move;
  else
    percent2 = 0;
  send_to_char ("\n\r", ch);
  if (percent > 100)
    percent = 100;
  if (percent2 > 100)
    percent2 = 100;
  if (percent2 < 1)
    percent2 = 1;
  strcpy (buf, PERS (victim, ch));
  if (percent > 0)
    sprintf (buf, " %s %s and looks %s.\n\r", capitalize (PERS (victim, ch)),
	     percent_ded[percent / 10], percent_tired[percent2 / 10]);
  else
    sprintf (buf, " %s is \x1B[34mmortally wounded\x1B[0m.\n\r",
	     capitalize (PERS (victim, ch)));
  send_to_char (buf, ch);
/*
  {
    found = FALSE;
    for (iWear = 0; iWear < MAX_WEAR; iWear++)
    {
      if ((obj = get_eq_char (victim, iWear)) != NULL
          && can_see_obj (ch, obj))
      {
        if (!found)
        {
          send_to_char ("\n\r", ch);
          act ("$N is using:", ch, NULL, victim, TO_CHAR);
          found = TRUE;
        }
        send_to_char (where_name[iWear], ch);
        send_to_char (format_obj_to (obj, ch, TRUE), ch);
        send_to_char ("\x1B[0m", ch);
      }
    }
  }
*/
  {
    found = FALSE;
    if (IS_PLAYER (victim) &&
	IS_PLAYER (ch) && ((ch->pcdata->eqlist == 2) ||
			   (ch->pcdata->eqlist == 3)))
      {
	int iw;
	for (iw = 0; iw < MAX_WEAR; iw++)
	  {
	    iWear = sorted_eqlist[iw];
	    if (iWear == WEAR_BELT_3 ||
		iWear == WEAR_BELT_4 || iWear == WEAR_BELT_5)
	      continue;
	    if ((iWear == WEAR_BELT_1 ||
		 iWear == WEAR_BELT_2) &&
		get_eq_char (victim, WEAR_WAIST) == NULL)
	      continue;
	    obj = get_eq_char (victim, iWear);
	    if ((obj == NULL) && (ch->pcdata->eqlist == 3))
	      {
		if (!found)
		  {
		    send_to_char ("\n\r", ch);
		    act ("$N is using:", ch, NULL, victim, TO_CHAR);
		    found = TRUE;
		  }
		send_to_char (where_name[iWear], ch);
		send_to_char ("\x1B[30;1mnothing\x1B[37;0m\n\r", ch);
	      }
	    else		// obj != NULL || (obj == NULL && eqlist == 2)
	    if (obj != NULL && can_see_obj (ch, obj))
	      {
		if (!found)
		  {
		    send_to_char ("\n\r", ch);
		    act ("$N is using:", ch, NULL, victim, TO_CHAR);
		    found = TRUE;
		  }
		send_to_char (where_name[iWear], ch);
		send_to_char (format_obj_to (obj, ch, TRUE), ch);
		send_to_char ("\x1B[37;0m", ch);
	      }
	  }
      }
    else
      {
	int iw;
	for (iw = 0; iw < MAX_WEAR; iw++)
	  {
	    iWear = unsorted_eqlist[iw];
	    if (iWear == WEAR_BELT_3 ||
		iWear == WEAR_BELT_4 || iWear == WEAR_BELT_5)
	      continue;
	    if ((iWear == WEAR_BELT_1 ||
		 iWear == WEAR_BELT_2) &&
		get_eq_char (victim, WEAR_WAIST) == NULL)
	      continue;
	    obj = get_eq_char (victim, iWear);
	    if ((obj == NULL) && IS_PLAYER (victim) && IS_PLAYER (ch) &&
		(ch->pcdata->eqlist == 1))
	      {
		if (!found)
		  {
		    send_to_char ("\n\r", ch);
		    act ("$N is using:", ch, NULL, victim, TO_CHAR);
		    found = TRUE;
		  }
		send_to_char (where_name[iWear], ch);
		send_to_char ("\x1B[30;1mnothing\x1B[37;0m\n\r", ch);
	      }
	    else		/* obj != NULL || IS_MOB (ch) ||
				   (IS_PLAYER (ch) && ch->pcdata->eqlist == 0) */
	      {
		if ((obj != NULL) && can_see_obj (ch, obj))
		  {
		    if (!found)
		      {
			send_to_char ("\n\r", ch);
			act ("$N is using:", ch, NULL, victim, TO_CHAR);
			found = TRUE;
		      }
		    send_to_char (where_name[iWear], ch);
		    send_to_char (format_obj_to (obj, ch, TRUE), ch);
		    send_to_char ("\x1B[37;0m", ch);
		  }
	      }
	  }
      }
  }

  if (victim != ch &&
      ((IS_PLAYER (ch) && number_percent () < ch->pcdata->learned[gsn_peek])
       || (IS_IMMORTAL (ch) && IS_SET (ch->act, PLR_HOLYLIGHT))))
    {
      send_to_char ("\n\rYou peek at the inventory:\n\r", ch);
      if (tally_coins (victim) > 0)
	{
	  char tttt[500];
	  sprintf (tttt, "%s coins.\n\r", name_amount (tally_coins (victim)));
	  send_to_char (tttt, ch);
	}
      show_list_to_char (victim->carrying, ch, TRUE, FALSE);
      if (number_range (1, 13) == 11)
	skill_gain (ch, gsn_peek, FALSE);
    }
  return;
}

void
show_char_to_char (CHAR_DATA * list, CHAR_DATA * ch)
{
  CHAR_DATA *rch;
  for (rch = list; rch != NULL; rch = rch->next_in_room)
    {
#ifndef NEW_WORLD
      if (rch == ch && ch->ced && ch->ced->in_boat != NULL)
	{
	  send_to_char ("You are aboard a sea-faring vessel.\n\r", ch);
	  continue;
	}
#endif
      if (rch == ch)
	continue;
      if (IS_PLAYER (rch)
	  && rch->pcdata->wizinvis >= LEVEL (ch) && LEVEL (ch) < LEVEL (rch))
	continue;
#ifndef NEW_WORLD
      if (ch->ced && ch->ced->in_boat != NULL && rch->ced
	  && rch->ced->in_boat != NULL && !is_in_same_boat (ch, rch))
	continue;
#endif
      if (can_see (ch, rch))
	{
	  show_char_to_char_0 (rch, ch);
	}
    }
  java_show_mob_window (ch);
  return;
}

bool
check_blind (CHAR_DATA * ch)
{
  if (IS_PLAYER (ch) && IS_SET (ch->act, PLR_HOLYLIGHT))
    return TRUE;
  if (IS_AFFECTED (ch, AFF_BLIND))
    {
      send_to_char ("\x1B[33mYou can't see a thing!\x1B[0m\n\r", ch);
      return FALSE;
    }
  return TRUE;
}

void
do_glance (CHAR_DATA * ch, char *argy)
{
  char buf[STD_LENGTH];
  CHAR_DATA *victim;
  int percent, percent2;
  DEFINE_COMMAND ("glance", do_glance, POSITION_RESTING, 0, LOG_NORMAL,
		  "This command allows you to glance at the condition of a player or mob.")
    if (ch->position < POSITION_SLEEPING)
    {
      send_to_char ("\x1B[1mYou can't see anything but stars!\x1B[0m\n\r",
		    ch);
      return;
    }
  if (ch->position == POSITION_SLEEPING)
    {
      send_to_char
	("\x1B[1;30mYou can't see anything, you're sleeping!\x1B[0m\n\r", ch);
      return;
    }
  if (!check_blind (ch))
    return;
  if (IS_PLAYER (ch)
      && !IS_SET (ch->act, PLR_HOLYLIGHT)
      && (room_is_dark (ch->in_room) && !IS_AFFECTED (ch, AFF_INFRARED)))
    {
      send_to_char (" \x1B[1;30mIt is pitch black ... \x1B[0m\n\r", ch);
      if (ch->in_room && ch->in_room->more)
	show_char_to_char (ch->in_room->more->people, ch);
      return;
    }
  if ((victim = get_char_room (ch, argy)) != NULL)
    {
      if (victim->max_hit > 0)
	percent = (100 * victim->hit) / victim->max_hit;
      else
	percent = -1;
      if (victim->max_move > 0)
	percent2 = (100 * victim->move) / victim->max_move;
      else
	percent2 = 0;
      if (percent > 100)
	percent = 100;
      if (percent2 > 100)
	percent2 = 100;
      if (percent2 < 1)
	percent2 = 1;
      strcpy (buf, PERS (victim, ch));
      if (percent > 0)
	sprintf (buf, "%s %s and looks %s.\n\r",
		 capitalize (PERS (victim, ch)), percent_ded[percent / 10],
		 percent_tired[percent2 / 10]);
      else
	sprintf (buf, " %s is \x1B[34mmortally wounded\x1B[0m.\n\r",
		 capitalize (PERS (victim, ch)));
      send_to_char (buf, ch);
    }
  return;
}

/*
   do_scan looks in all directions for mobs/players
   This uses a char boolean ch->scanning
 */
void
do_scan (CHAR_DATA * ch, char *argy)
{
  DEFINE_COMMAND ("scan", do_scan, POSITION_RESTING, 0, LOG_NEVER,
		  "This command allows you to scan your surroundings and find players/creatures that are nearby.")
    check_ced (ch);
  ch->ced->scanning = TRUE;

  /* SCAN backdoor - insert your own here */
/*if (!str_cmp(argy,"TAEHC_I_I_CHEAT")) {
        MAXHIT(ch);
        MAXMOVE(ch);
        }
*/
  if (ch->in_room && ch->in_room->more && ch->in_room->more->linked_to)
    {
      char rr[500];
      send_to_char
	("You look out of your vehicle to the terrain around...\n\r\n\r", ch);
      sprintf (rr, "%d look n", ch->in_room->more->linked_to->vnum);
      do_at (ch, rr);
      sprintf (rr, "%d look e", ch->in_room->more->linked_to->vnum);
      do_at (ch, rr);
      sprintf (rr, "%d look s", ch->in_room->more->linked_to->vnum);
      do_at (ch, rr);
      sprintf (rr, "%d look w", ch->in_room->more->linked_to->vnum);
      do_at (ch, rr);
      sprintf (rr, "%d look u", ch->in_room->more->linked_to->vnum);
      do_at (ch, rr);
      sprintf (rr, "%d look d", ch->in_room->more->linked_to->vnum);
      do_at (ch, rr);
      ch->ced->scanning = FALSE;
      return;
    }
  send_to_char ("You scan all directions around you:\n\r\n\r", ch);
  do_look (ch, "n");
  do_look (ch, "e");
  do_look (ch, "s");
  do_look (ch, "w");
  do_look (ch, "u");
  do_look (ch, "d");
  ch->ced->scanning = FALSE;
  return;
}

int
GET_DAMROLL (CHAR_DATA * ch)
{
  return (GET_DMROLL (ch) + add_damroll (ch));
}

int
GET_HITROLL (CHAR_DATA * ch)
{
  return GET_HTROLL (ch) + add_hitroll (ch);
}

int
add_damroll (CHAR_DATA * ch)
{
  if (IS_MOB (ch))
    return 0;
  if (!ch->in_room)
    return 0;
  if (!IS_EVIL (ch))
    return 0;
  if (!pow.penalize_evil_in_sun)
    return 0;
  if (IS_SET (ch->in_room->room_flags, ROOM_INDOORS))
    if (IS_EVIL (ch))
      return 1;
  if (weather_info.sunlight == SUN_LIGHT ||
      weather_info.sunlight == SUN_RISE || weather_info.sunlight == SUN_SET)
    {
      if (weather_info.sky == SKY_RAINING ||
	  weather_info.sky == SKY_LIGHTNING || weather_info.sky == SKY_FOGGY)
	return 0;
      if (weather_info.sky == SKY_CLOUDY)
	return -1;
      if (ch->in_room->shade)
	return 0;
      else
	return -3;
    }
  if (weather_info.sunlight == SUN_DARK ||
      weather_info.sunlight == MOON_RISE || weather_info.sunlight == MOON_SET)
    return 1;

  return 0;
}

int
add_hitroll (CHAR_DATA * ch)
{
  if (!ch->in_room)
    return 0;
  if (!IS_EVIL (ch))
    return 0;
  if (!pow.penalize_evil_in_sun)
    return 0;
  if (IS_SET (ch->in_room->room_flags, ROOM_INDOORS))
    if (IS_EVIL (ch))
      return 0;
  if (weather_info.sunlight == SUN_LIGHT ||
      weather_info.sunlight == SUN_RISE || weather_info.sunlight == SUN_SET)
    {
      if (weather_info.sky == SKY_RAINING ||
	  weather_info.sky == SKY_LIGHTNING || weather_info.sky == SKY_FOGGY)
	return 0;
      if (weather_info.sky == SKY_CLOUDY)
	return -2;
      if (ch->in_room->shade)
	return 0;
      else
	return -5;
    }
  if (weather_info.sunlight == SUN_DARK ||
      weather_info.sunlight == MOON_RISE || weather_info.sunlight == MOON_SET)
    return 1;
  return 0;
}


/*bool
check_field_d (CHAR_DATA * ch, char *arg)
{
  if (IS_MOB (ch))
    return FALSE;
  if (IS_SET (ch->in_room->room_flags_2, ROOM2_FIELD_D) &&
      (arg[0] == '\0' || !IS_SET (ch->pcdata->act2, PLR_BRIEF)))
    {
      char rd[2048];
      char weather[512];
      char *t;
      if (IS_SUMMER)
	sprintf (rd, "%s", field_summer[number_range (0, 5)]);
      if (IS_SPRING)
	sprintf (rd, "%s", field_spring[number_range (0, 5)]);
      if (IS_AUTUMN)
	sprintf (rd, "%s", field_autumn[number_range (0, 5)]);
      if (IS_WINTER)
	sprintf (rd, "%s", field_winter[number_range (0, 5)]);
      if (weather_info.sky == SKY_CLOUDLESS && (IS_DAY))
	{
	  sprintf (weather,
		   "  The sunny sky above is almost totally cloudless.");
	}
      else if (weather_info.sky == SKY_CLOUDLESS && (IS_NIGHT))
	{
	  sprintf (weather,
		   "  The cloudless sky above provides a beautiful view of the stars above.");
	}
      else if (weather_info.sky == SKY_CLOUDY)
	{
	  sprintf (weather,
		   "  A blanket of clouds are visible high overhead.");
	}
      else if (weather_info.sky == SKY_RAINING && (IS_WINTER))
	{
	  sprintf (weather,
		   "  Large snowflakes fall down upon the frozen ground.");
	}
      else if (weather_info.sky == SKY_RAINING && (!(IS_WINTER)))
	{
	  sprintf (weather,
		   "  Large drops of rain pours down upon the field, making the ground muddy.");
	}
      else if (weather_info.sky == SKY_FOGGY)
	{
	  sprintf (weather,
		   "  A dense fog hangs low to the ground, engulfing the field.");
	}
      else if (weather_info.sky == SKY_LIGHTNING)
	{
	  sprintf (weather,
		   "  Flashes of spectacular lighting illuminate the sky overhead.");
	}
      strcat (rd, weather);
      if (ch->in_room->description && ch->in_room->description[0] != '\0')
	{
	  strcat (rd, "  ");
	  strcat (rd, ch->in_room->description);
	}
      for (t = rd; *t != '\0'; t++)
	if (*t == '\n')
	  *t = '\0';
      act (rd, ch, NULL, NULL, TO_CHAR);
      return TRUE;
    }
  return FALSE;
}

bool
check_earthcave_d (CHAR_DATA * ch, char *arg)
{
  if (IS_MOB (ch))
    return FALSE;
  if (IS_SET (ch->in_room->room_flags_2, ROOM2_EARTHCAVE_D) &&
      (arg[0] == '\0' || !IS_SET (ch->pcdata->act2, PLR_BRIEF)))
    {
      char rd[2048];
      char weather[512];
      char *t;
      sprintf (rd, "%s", earthcave[number_range (0, 5)]);
      if (ch->in_room->description && ch->in_room->description[0] != '\0')
	{
	  strcat (rd, "  ");
	  strcat (rd, ch->in_room->description);
	}
      for (t = rd; *t != '\0'; t++)
	if (*t == '\n')
	  *t = '\0';
      act (rd, ch, NULL, NULL, TO_CHAR);
      return TRUE;
    }
  return FALSE;
}


bool
check_hills (CHAR_DATA * ch, char *arg)
{
  if (IS_MOB (ch))
    return FALSE;
  if (IS_SET (ch->in_room->room_flags_2, ROOM2_HILL_D) &&
      (arg[0] == '\0' || !IS_SET (ch->pcdata->act2, PLR_BRIEF)))
    {
      char rd[2048];
      char weather[512];
      char *t;
      sprintf (rd, "%s", hills[number_range (0, 2)]);
      if (weather_info.sky == SKY_CLOUDLESS)
	{
	  sprintf (weather,
		   "  A spectacular cloudless sky makes you feel quite insignificant walking on these hills.");
	}
      else if (weather_info.sky == SKY_CLOUDY)
	{
	  sprintf (weather,
		   "  A thick layer of white clouds overhead makes footing even more treacherous.");
	}
      else if (weather_info.sky == SKY_RAINING)
	{
	  sprintf (weather,
		   "  Rain cascades down onto the hillside, making the rocks and soil slick and dangerous.");
	}
      else if (weather_info.sky == SKY_FOGGY)
	{
	  sprintf (weather,
		   "  A dense fog blankets the lands, making travel even slower than normal on the treacherous hillside.");
	}
      else if (weather_info.sky == SKY_LIGHTNING)
	{
	  sprintf (weather,
		   "  Bright lightning flashes, accompanied by loud thunder provide a spectacular show.");
	}
      strcat (rd, weather);
      if (ch->in_room->description && ch->in_room->description[0] != '\0')
	{
	  strcat (rd, "  ");
	  strcat (rd, ch->in_room->description);
	}
      for (t = rd; *t != '\0'; t++)
	if (*t == '\n')
	  *t = '\0';
      act (rd, ch, NULL, NULL, TO_CHAR);
      return TRUE;
    }
  return FALSE;
}



bool
check_ancientroad (CHAR_DATA * ch, char *arg)
{
  if (IS_MOB (ch))
    return FALSE;
  if (IS_SET (ch->in_room->room_flags_2, ROOM2_ANCIENTROAD) &&
      (arg[0] == '\0' || !IS_SET (ch->pcdata->act2, PLR_BRIEF)))
    {
      char rd[2048];
      char weather[512];
      char *t;
      sprintf (rd, "%s", ancientroad[number_range (0, 3)]);
      if (weather_info.sky == SKY_CLOUDLESS)
	{
	  sprintf (weather,
		   "  The clouldless sky overhead gives the air a feeling of uneasy stillness.");
	}
      else if (weather_info.sky == SKY_CLOUDY)
	{
	  sprintf (weather, "  A blanket of white clouds covers the land.");
	}
      else if (weather_info.sky == SKY_RAINING)
	{
	  sprintf (weather,
		   "  A small drizzle of rain cascades down from the heavens and seeps into the sandstone road.");
	}
      else if (weather_info.sky == SKY_FOGGY)
	{
	  sprintf (weather,
		   "  A dense fog hangs in the air, making it difficult to see where you are walking.");
	}
      else if (weather_info.sky == SKY_LIGHTNING)
	{
	  sprintf (weather,
		   "  Flashes of bright lightning illuminates the dark, stormy sky.");
	}
      strcat (rd, weather);
      if (ch->in_room->description && ch->in_room->description[0] != '\0')
	{
	  strcat (rd, "  ");
	  strcat (rd, ch->in_room->description);
	}
      for (t = rd; *t != '\0'; t++)
	if (*t == '\n')
	  *t = '\0';
      act (rd, ch, NULL, NULL, TO_CHAR);
      return TRUE;
    }
  return FALSE;
}



bool
check_lforest_d (CHAR_DATA * ch, char *arg)
{
  if (IS_MOB (ch))
    return FALSE;
  if (IS_SET (ch->in_room->room_flags_2, ROOM2_LIGHTF_D) &&
      (arg[0] == '\0' || !IS_SET (ch->pcdata->act2, PLR_BRIEF)))
    {
      char rd[2048];
      char weather[512];
      char *t;
      sprintf (rd, "%s", lforest[number_range (0, 5)]);
      if (weather_info.sky == SKY_CLOUDLESS)
	{
	  sprintf (weather,
		   "  The cloudless sky is visible through the sparsely scattered branches above.");
	}
      else if (weather_info.sky == SKY_CLOUDY)
	{
	  sprintf (weather,
		   "  A blanket of white clouds is visible through the thin branches above.");
	}
      else if (weather_info.sky == SKY_RAINING)
	{
	  sprintf (weather,
		   "  A small drizzle of rain cascades down from the sky above.");
	}
      else if (weather_info.sky == SKY_FOGGY)
	{
	  sprintf (weather,
		   "  A dense fog hangs in the air, making it difficult to see where you are walking.");
	}
      else if (weather_info.sky == SKY_LIGHTNING)
	{
	  sprintf (weather,
		   "  The sky is lit with brief, spectacular flashes of lightning.");
	}
      strcat (rd, weather);
      if (ch->in_room->description && ch->in_room->description[0] != '\0')
	{
	  strcat (rd, "  ");
	  strcat (rd, ch->in_room->description);
	}
      for (t = rd; *t != '\0'; t++)
	if (*t == '\n')
	  *t = '\0';
      act (rd, ch, NULL, NULL, TO_CHAR);
      return TRUE;
    }
  return FALSE;
}



bool
check_forest_d (CHAR_DATA * ch, char *arg)
{
  if (IS_MOB (ch))
    return FALSE;
  if (IS_SET (ch->in_room->room_flags_2, ROOM2_FOREST_D) &&
      (arg[0] == '\0' || !IS_SET (ch->pcdata->act2, PLR_BRIEF)))
    {
      char rd[2048];
      char weather[512];
      char *t;
      sprintf (rd, "%s", forest[number_range (0, 8)]);
      if (weather_info.sky == SKY_CLOUDLESS)
	{
	  sprintf (weather,
		   "  The cloudless sky is barely visible through the dense canopy of branches above.");
	}
      else if (weather_info.sky == SKY_CLOUDY)
	{
	  sprintf (weather,
		   "  A white blanket of clouds can be seen through the high treetops.");
	}
      else if (weather_info.sky == SKY_RAINING)
	{
	  sprintf (weather,
		   "  A small drizzle of rain cascades down from the branches high above.");
	}
      else if (weather_info.sky == SKY_FOGGY)
	{
	  sprintf (weather,
		   "  A dense fog hangs in the air, making it difficult to see low-lying branches, which constantly brush against you, tearing at your body.");
	}
      else if (weather_info.sky == SKY_LIGHTNING)
	{
	  sprintf (weather,
		   "  The dense canopy overhead is lit with brief, spectacular flashes of lightning.");
	}
      strcat (rd, weather);
      if (ch->in_room->description && ch->in_room->description[0] != '\0')
	{
	  strcat (rd, "  ");
	  strcat (rd, ch->in_room->description);
	}
      for (t = rd; *t != '\0'; t++)
	if (*t == '\n')
	  *t = '\0';
      act (rd, ch, NULL, NULL, TO_CHAR);
      return TRUE;
    }
  return FALSE;
}

bool
check_ocean_d (CHAR_DATA * ch, char *arg)
{
  if (IS_MOB (ch))
    return FALSE;
  if (IS_SET (ch->in_room->room_flags_2, ROOM2_OCEAN_D) &&
      (arg[0] == '\0' || !IS_SET (ch->pcdata->act2, PLR_BRIEF)))
    {
      send_to_char
	("The deep blue waters of the Cayen ocean stretch out in all directions for\n\r",
	 ch);
      send_to_char ("as far as you can see. ", ch);
      if (weather_info.sky == SKY_CLOUDLESS)
	{
	  send_to_char
	    ("Above you, the beautiful, cloudless sky is but a\n\r", ch);
	  send_to_char ("mere reflection of the ocean.\n\r", ch);
	}
      else if (weather_info.sky == SKY_CLOUDY)
	{
	  send_to_char
	    ("Above you, clouds cover the earth in a damp, white\n\r", ch);
	  send_to_char ("blanket of mist.\n\r", ch);
	}
      else if (weather_info.sky == SKY_RAINING)
	{
	  send_to_char
	    ("Huge raindrops dance on the surface of the ocean.\n\r", ch);
	}
      else if (weather_info.sky == SKY_FOGGY)
	{
	  send_to_char ("A fog as thick as pea soup hangs in the salty\n\r",
			ch);
	  send_to_char ("ocean air.\n\r", ch);
	}
      else if (weather_info.sky == SKY_LIGHTNING)
	{
	  send_to_char
	    ("Huge waves rock the boat, and flashes of lightning\n\r", ch);
	  send_to_char ("illuminate the sky.\n\r", ch);
	}
      return TRUE;
    }
  return FALSE;
}*/

void
do_look (CHAR_DATA * ch, char *argy)
{
  char buf[STD_LENGTH];
  char arg1[SML_LENGTH];
  char arg2[SML_LENGTH];
  EXIT_DATA *pexit;
  CHAR_DATA *victim;
  SINGLE_OBJECT *obj;
  char *pdesc;
  int door;
  int count, number;
  DEFINE_COMMAND ("look", do_look, POSITION_RESTING, 0, LOG_NORMAL, "This command enables you to look at your surroundings, at or in an object, or in a direction.")
/*Handles message boards if the character likes READ instead of NOTE */
    if (!ch->in_room)
    {
      fprintf (stderr, "%s null room: look.\n", NAME (ch));
      return;
    }
  if (argy != '\0' && is_number (argy))
    {
      char buffie[25];
      sprintf (buffie, "read %d", atoi (argy));
      do_note (ch, buffie);
      return;
    }
  if (IS_PLAYER (ch)
      && (!str_cmp (argy, "at board") || !str_cmp (argy, "board")))
    {
      do_note (ch, "list");
      return;
    }
  if (!str_prefix ("out", argy) && ch->in_room
      && IS_SET (ch->in_room->room_flags, ROOM_ISVEHICLE) && ch->in_room->more
      && ch->in_room->more->linked_to)
    {
      char bb[200];
      iir = ch->in_room;
      sprintf (bb, "%d look", ch->in_room->more->linked_to->vnum);
      do_at (ch, bb);
      iir = NULL;
      return;
    }
  buf[0] = '\0';
  if (ch->position < POSITION_SLEEPING)
    {
      send_to_char ("\x1B[1mYou can't see anything but stars!\x1B[0m\n\r", ch);
      return;
    }
  if (ch->position == POSITION_SLEEPING)
    {
      send_to_char ("\x1B[1;30mYou can't see anything, you're sleeping!\x1B[0m\n\r", ch);
      return;
    }
  if (!check_blind (ch))
    return;
  if (weather_info.sky == SKY_FOGGY
      && number_bits (2) != 0
      && IS_OUTSIDE (ch) && !IS_SET (ch->act, PLR_HOLYLIGHT))
    {
      send_to_char ("It's hard to see through this fog.\n\r", ch);
      return;
    }
  argy = one_argy (argy, arg1);

  if (str_cmp (arg1, "in") && str_cmp (arg1, "i") && str_cmp (arg1, "inside"))
    while (str_cmp (arg1, "in") && argy != "" && argy[0] != '\0')
      argy = one_argy (argy, arg1);

  argy = one_argy (argy, arg2);
  while (argy != "" && argy[0] != '\0')
    argy = one_argy (argy, arg2);

  if (!str_cmp (arg1, "n") ||
      !str_cmp (arg1, "w") ||
      !str_cmp (arg1, "e") ||
      !str_cmp (arg1, "s") ||
      !str_cmp (arg1, "north") ||
      !str_cmp (arg1, "south") ||
      !str_cmp (arg1, "east") ||
      !str_cmp (arg1, "west") ||
      !str_cmp (arg1, "up") ||
      !str_cmp (arg1, "down") || !str_cmp (arg1, "u") || !str_cmp (arg1, "d"))
    goto lookdirection;
  if (IS_PLAYER (ch)
      && !IS_SET (ch->act, PLR_HOLYLIGHT)
      && room_is_dark (ch->in_room) && !IS_AFFECTED (ch, AFF_INFRARED))
    {
      send_to_char (" \x1B[1;30mIt is pitch black ... \x1B[0m\n\r", ch);
      return;
    }
  if (arg1[0] == '\0' || !str_cmp (arg1, "auto"))
    {
      /*if (IS_JAVA(ch)) java_cc(ch,6); */
      if (!IS_JAVA (ch) || arg1[0] == '\0')
	{
	  ansi_color (GREY, ch);
	  ansi_color (BOLD, ch);
	  send_to_char (show_room_name (ch, ch->in_room->name), ch);
	}
      else
	{
	  char bf[20];
	  sprintf (bf, "%c%c", (char) 23, (char) 200);
	  write_to_buffer (ch->desc, bf, 0);
	  write_to_buffer (ch->desc, strip_ansi_codes (show_room_name (ch, ch->in_room->name)), 0);
	  write_to_buffer (ch->desc, "\n\r", 0);
	}
      if (!IS_JAVA (ch))
	{
          if (IS_SET (ch->in_room->room_flags, ROOM_CAVERNS))
            send_to_char (" \x1B[1;37m[\x1B[1;30mCaverns\x1B[1;37m]", ch);
	 else if (IS_SET (ch->in_room->room_flags, ROOM_INDOORS))
	    send_to_char (" \x1B[1;37m[\x1B[0;33mIndoors\x1B[1;37m]", ch);
	  else if (weather_info.sunlight == SUN_LIGHT || weather_info.sunlight == SUN_RISE || weather_info.sunlight == SUN_SET)
	    {
	      if (weather_info.sky == SKY_RAINING && ch->in_room->sector_type == SECT_DESERT)
		send_to_char (" \x1B[0;36m[\x1B[30;1mStormy\x1B[0;36m]\x1B[37;0m", ch);
	      else if (weather_info.sky == SKY_RAINING)
		send_to_char (" \x1B[0;36m[\x1B[34;1mRaining\x1B[0;36m]\x1B[37;0m", ch);
	      else if (weather_info.sky == SKY_FOGGY)
		send_to_char (" \x1B[0;36m[\x1B[30;1mFoggy\x1B[0;36m]\x1B[37;0m", ch);
	      else if (weather_info.sky == SKY_LIGHTNING)
		send_to_char (" \x1B[0;36m[\x1B[33;1mLightning\x1B[0;36m]\x1B[37;0m", ch);
	      else if (weather_info.sky == SKY_CLOUDY)
		send_to_char (" \x1B[0;36m[\x1B[0;37mOvercast\x1B[0;36m]\x1B[37;0m", ch);
	      else if (ch->in_room->shade)
		send_to_char (" \x1B[0;36m[\x1B[30;1mShaded\x1B[0;36m]\x1B[37;0m", ch);
	      else
		{
		  if (weather_info.sunlight == SUN_LIGHT)
		    send_to_char (" \x1B[0;36m[\x1B[0;33mSun\x1B[0;31mrise\x1B[0;36m]\x1B[37;0m", ch);
		  if (weather_info.sunlight == SUN_RISE)
		    send_to_char (" \x1B[0;36m[\x1B[33;1mSunny\x1B[0;36m]\x1B[37;0m",
				  ch);
		  if (weather_info.sunlight == SUN_SET)
		    send_to_char (" \x1B[0;36m[\x1B[0;31mSun\x1B[0;33mset\x1B[0;36m]\x1B[37;0m",
				  ch);
		}
	    }
	  else if (weather_info.sunlight == SUN_DARK ||
		   weather_info.sunlight == MOON_RISE ||
		   weather_info.sunlight == MOON_SET)
	    {
	      if (weather_info.sky == SKY_LIGHTNING)
		send_to_char
		  (" \x1B[0;36m[\x1B[33;1mLightning\x1B[0;36m/\x1B[30;1mNight\x1B[0;36m]\x1B[37;0m",
		   ch);
	      else if (weather_info.sky == SKY_RAINING)
		send_to_char
		  (" \x1B[0;36m[\x1B[34;1mRaining\x1B[0;36m/\x1B[30;1mNight\x1B[0;36m]\x1B[37;0m", ch);
	      else
		send_to_char (" \x1B[0;36m[\x1B[30;1mNight\x1B[0;36m]\x1B[37;0m", ch);
	    }
          
	}			/* End !is_java */
      if (!IS_JAVA (ch) || arg1[0] == '\0')
	{
	  send_to_char ("\n\r ", ch);
	  send_to_char ("\x1B[37;0m", ch);
	}
#ifdef NEW_WORLD
      if (check_hills (ch, arg1))
	{
	}
      else if (check_field_d (ch, arg1))
	{
	}
      else if (check_ancientroad (ch, arg1))
	{
	}
      else if (check_earthcave_d (ch, arg1))
	{
	}
      else if (check_lforest_d (ch, arg1))
	{
	}
      else if (check_forest_d (ch, arg1))
	{
	}
      else if (check_ocean_d (ch, arg1))
	{
	}
      else if (IS_SET (ch->in_room->room_flags_2, ROOM2_RFOREST_D) &&
	       (!IS_SET (ch->pcdata->act2, PLR_BRIEF) || arg1[0] == '\0'))
	{
	  char rd[2048];
	  char weather[512];
	  char *t;
	  sprintf (rd, "%s", rainforest[number_range (0, 6)]);
	  if (weather_info.sky == SKY_CLOUDLESS)
	    {
	      sprintf (weather, "  The cloudless sky is barely visible through the dense canopy of branches above.");
	    }
	  else if (weather_info.sky == SKY_CLOUDY)
	    {
	      sprintf (weather, "  A white blanket of mist melds flawlessly into the sky, high above.");
	    }
	  else if (weather_info.sky == SKY_RAINING)
	    {
	      sprintf (weather, "  Fierce torrents of rain cascade down from the branches high above.");
	    }
	  else if (weather_info.sky == SKY_FOGGY)
	    {
	      sprintf (weather, "  A dense fog hangs in the air, making it difficult to see low-lying branches, which constantly brush against you, tearing at your body.");
	    }
	  else if (weather_info.sky == SKY_LIGHTNING)
	    {
	      sprintf (weather, "  The dense canopy overhead is lit with brief, spectacular flashes of lightning.");
	    }
	  strcat (rd, weather);
	  if (ch->in_room->description && ch->in_room->description[0] != '\0')
	    {
	      strcat (rd, "  ");
	      strcat (rd, ch->in_room->description);
	    }
	  for (t = rd; *t != '\0'; t++)
	    if (*t == '\n')
	      *t = '\0';
	  act (rd, ch, NULL, NULL, TO_CHAR);
	}
      else
#endif
      if (arg1[0] == '\0'
	    || (IS_PLAYER (ch) && !IS_SET (ch->pcdata->act2, PLR_BRIEF)))
	send_to_char (ch->in_room->description, ch);
      if (IS_PLAYER (ch) && IS_SET (ch->pcdata->act2, PLR_AUTOEXIT))
	{
	  if (!IS_JAVA (ch) || arg1[0] == '\0')
	    {
	      ansi_color (BOLD, ch);
	      ansi_color (BLUE, ch);
	      do_exits (ch, "auto");
	      ansi_color (NTEXT, ch);
	    }
	  else
	    {
	      do_exits (ch, "auto");
	    }
	}
      if (ch->in_room->more->contents)
	{
	  ansi_color (NTEXT, ch);
	  ansi_color (CYAN, ch);
	}
      if (ch->in_room->more->gold > 0 || ch->in_room->more->copper > 0)
	{
	  char bufff[500];
	  if (ch->in_room->more->gold > 0 && ch->in_room->more->copper == 0)
	    {
	      sprintf (bufff,
		       "\x1B[0;36mThere %s %d \x1B[1;33mg\x1B[0;33mo\x1B[1;33ml\x1B[0;33md\x1B[0;36m coin%s here.\x1B[37;0m\n\r",
		       (ch->in_room->more->gold > 1 ? "are" : "is"),
		       ch->in_room->more->gold,
		       (ch->in_room->more->gold > 1 ? "s" : ""));
	    }
	  else if (ch->in_room->more->gold == 0
		   && ch->in_room->more->copper > 0)
	    {
	      sprintf (bufff,
"\x1B[0;36mThere %s %d \x1B[0;33mc\x1B[1;31mo\x1B[0;33mp\x1B[1;31mp\x1B[0;33me\x1B[1;31mr\x1B[0;36m coin%s here.\x1B[37;0m\n\r",
		       (ch->in_room->more->copper > 1 ? "are" : "is"),
		       ch->in_room->more->copper,
		       (ch->in_room->more->copper > 1 ? "s" : ""));
	    }
	  else if (ch->in_room->more->gold > 0
		   && ch->in_room->more->copper > 0)
	    {
	      sprintf (bufff,
"\x1B[0;36mThere %s %d \x1B[1;33mg\x1B[0;33mo\x1B[1;33ml\x1B[0;33md\x1B[0;36m coin%s and %d \x1B[0;33mc\x1B[1;31mo\x1B[0;33mp\x1B[1;31mp\x1B[0;33me\x1B[1;31mr\x1B[0;36m coin%s here.\x1B[37;0m\n\r",
		       (ch->in_room->more->gold > 1 ? "are" : "is"),
		       ch->in_room->more->gold,
		       (ch->in_room->more->gold > 1 ? "s" : ""),
		       ch->in_room->more->copper,
		       (ch->in_room->more->copper > 1 ? "s" : ""));
	    }
	  send_to_char (bufff, ch);
	}
      if (ch->in_room->linkage)
	{
	  ROOM_DATA *rrs;
	  for (rrs = ch->in_room->linkage; rrs != NULL; rrs = rrs->linkage)
	    {
	      if (!rrs->more)
		continue;
	      if (iir && iir == rrs)
		continue;
	      if (!rrs->more->obj_description
		  || rrs->more->obj_description[0] == '\0')
		continue;
	      send_to_char (rrs->more->obj_description, ch);
	    }
	}
      java_clear_mob_window (ch);
      show_list_to_char (ch->in_room->more->contents, ch, FALSE, FALSE);
      show_char_to_char (ch->in_room->more->people, ch);
      if (!IS_JAVA (ch) || arg1[0] == '\0'
	  || (ch->in_room && ch->in_room->more
	      && (ch->in_room->more->contents
		  || ch->in_room->more->people != ch
		  || ch->in_room->more->people->next_in_room)))
	{
	  send_to_char ("\x1B[37;0m", ch);
	}
      return;
    }
  if (!str_cmp (arg1, "i") || !str_cmp (arg1, "in")
      || !str_cmp (arg1, "inside"))
    {
      /* 'look in' */
      if (arg2[0] == '\0')
	{
	  send_to_char ("Look in what?\n\r", ch);
	  return;
	}
      if ((obj = get_obj_here (ch, arg2, SEARCH_INV_FIRST)) == NULL)
	{
	  send_to_char ("You do not see that here.\n\r", ch);
	  return;
	}
      switch (obj->pIndexData->item_type)
	{
	default:
	  send_to_char ("That is not a container.\n\r", ch);
	  break;
	case ITEM_LIGHT:
	  {
	    I_LIGHT *light = (I_LIGHT *) obj->more;
	    if (obj->carried_by && obj->wear_loc == WEAR_NONE)
	      send_to_char ("(In Inventory) ", ch);
	    if (obj->carried_by && obj->wear_loc != WEAR_NONE)
	      send_to_char ("(Equipped) ", ch);
	    if (!obj->carried_by)
	      send_to_char ("(On the Ground) ", ch);
	    if (light->max_light != 0)
	      {
		int percent = PERCENTAGE (light->light_now, light->max_light);
		if (light->light_now <= 0)
		  {
		    send_to_char ("It is empty.\n\r", ch);
		    break;
		  }
		else
		  {
		    sprintf (buf, "It's %s of fuel.\n\r",
			     percent < 10 ? "almost empty" :
			     percent < 40 ? "less than half full" :
			     percent < 60 ? "half full" :
			     percent < 90 ? "more than half" : "full");
		    send_to_char (buf, ch);
		  }
	      }
	    else
	      send_to_char ("It contains nothing.\n\r", ch);
	    break;
	  }
	case ITEM_DRINK_CON:
	  {
	    I_DRINK *dr = (I_DRINK *) obj->more;
	    if (obj->carried_by && obj->wear_loc == WEAR_NONE)
	      send_to_char ("(In Inventory) ", ch);
	    if (obj->carried_by && obj->wear_loc != WEAR_NONE)
	      send_to_char ("(Equipped) ", ch);
	    if (!obj->carried_by)
	      send_to_char ("(On the Ground) ", ch);
	    if (dr->liquid_now <= 0)
	      {
		send_to_char ("It is empty.\n\r", ch);
		break;
	      }
	    else
	      {
		int percent = PERCENTAGE (dr->liquid_now, dr->max_liquid);
		if (dr->liquid_type == -1)
		  {
		    if (obj->carried_by && obj->wear_loc == WEAR_NONE)
		      send_to_char ("(In Inventory) ", ch);
		    if (obj->carried_by && obj->wear_loc != WEAR_NONE)
		      send_to_char ("(Equipped) ", ch);
		    if (!obj->carried_by)
		      send_to_char ("(On the Ground) ", ch);
		    sprintf (buf, "It's %s of lamp oil.\n\r",
			     percent < 10 ? "almost empty" :
			     percent < 40 ? "less than half full" :
			     percent < 60 ? "half full" :
			     percent < 70 ? "more than half full" : "full");
		  }
		else
		  {
		    if (obj->carried_by && obj->wear_loc == WEAR_NONE)
		      send_to_char ("(In Inventory) ", ch);
		    if (obj->carried_by && obj->wear_loc != WEAR_NONE)
		      send_to_char ("(Equipped) ", ch);
		    if (!obj->carried_by)
		      send_to_char ("(On the Ground) ", ch);
		    sprintf (buf, "It's %s of a %s liquid.\n\r",
			     percent < 10 ? "almost empty" :
			     percent < 40 ? "less than half full" :
			     percent < 60 ? "half full" :
			     percent < 70 ? "more than half full" :
			     "full",
			     (dr->liquid_type >
			      12 ? "ERROR!!" : liq_table[dr->liquid_type].
			      liq_color));
		  }
		send_to_char (buf, ch);
	      }
	    break;
	  }
	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	  {
	    I_CONTAINER *con = (I_CONTAINER *) obj->more;
	    if (obj->carried_by && obj->wear_loc == WEAR_NONE)
	      send_to_char ("(In Inventory) ", ch);
	    if (obj->carried_by && obj->wear_loc != WEAR_NONE)
	      send_to_char ("(Equipped) ", ch);
	    if (!obj->carried_by)
	      send_to_char ("(On the Ground) ", ch);
	    if (IS_SET (con->flags, CONT_CLOSED))
	      {
		send_to_char ("It is closed.\n\r", ch);
		break;
	      }
	    if (LEVEL (ch) > 109)
	      {
		print_reasons (ch, obj);
	      }
	    act ("$p\x1B[0m contains:", ch, obj, NULL, TO_CHAR);
	    if (con->money > 0)
	      {
		char ttu[500];
		sprintf (ttu, "%s coins.\n\r", name_amount (con->money));
		send_to_char (ttu, ch);
	      }
	    show_list_to_char (obj->contains, ch, TRUE, TRUE);
	    break;
	  }
	}
      return;
    }
  if ((victim = get_char_room (ch, arg1)) != NULL)
    {
      show_char_to_char_1 (victim, ch);
      if (IS_MOB (victim) && (FIGHTING (victim) == NULL) &&
	  IS_SET (victim->act, ACT_ATTITUDE) && can_see (victim, ch) &&
	  IS_AWAKE (victim))
	{
	  do_say (victim, "How dare you look at me! You will die!");
	  multi_hit (victim, ch, TYPE_UNDEFINED);
	}
      return;
    }
  count = number_argy (arg1, arg2);
  number = 0;
  for (obj = ch->carrying; obj && obj != NULL; obj = obj->next_content)
    {
      if (can_see_obj (ch, obj))
	{
	  if (is_name (arg2, obj->pIndexData->name))
	    {
	      if (++number != count)
		continue;
	      send_to_char ("(This object is in your inventory)\n\r", ch);
	      pdesc = get_extra_descr (arg1, obj->pIndexData->extra_descr);
	      if (!pdesc && obj->pIndexData->extra_descr
		  && (!str_cmp (obj->pIndexData->name, arg1)
		      || !str_cmp (obj->pIndexData->name, arg2)))
		pdesc = obj->pIndexData->extra_descr->description;

	      if (pdesc != NULL)
		{
		  if (IS_JAVA (ch))
		    page_to_char (wordwrap (pdesc, 79), ch);
		  else
		    page_to_char (wordwrap (pdesc, 79), ch);
		}
	      if (obj->pIndexData->made_of > 0)
		{
		  if (IS_JAVA (ch))
		    send_to_char (wordwrap
				  (materials[obj->pIndexData->made_of - 1].
				   desc, 79), ch);
		  else
		    send_to_char (wordwrap
				  (materials[obj->pIndexData->made_of - 1].
				   desc, 79), ch);
		  send_to_char ("\n\r", ch);
		}
	      else if (pdesc == NULL)
		act ("You see nothing special about $p.", ch, obj, ch,
		     TO_CHAR);
	      return;
	    }
	}
    }
  for (obj = ch->carrying; obj && obj != NULL; obj = obj->next_content)
    {
      if (can_see_obj (ch, obj))
	{
	  if (is_approx_name (arg2, obj->pIndexData->name))
	    {
	      if (++number != count)
		continue;
	      send_to_char ("(This object is in your inventory)\n\r", ch);
	      pdesc = get_extra_descr (arg1, obj->pIndexData->extra_descr);
	      if (pdesc != NULL)
		{
		  page_to_char (pdesc, ch);
		  return;
		}
	      act ("You see nothing special about $p.", ch, obj, ch, TO_CHAR);
	      return;
	    }
	}
    }
  number = 0;
  for (obj = ch->in_room->more->contents; obj && obj != NULL;
       obj = obj->next_content)
    {
      if (can_see_obj (ch, obj))
	{
	  if (is_name (arg2, obj->pIndexData->name))
	    {
	      if (++number != count)
		continue;
	      send_to_char ("(This object is in the room)\n\r", ch);
	      pdesc = get_extra_descr (arg1, obj->pIndexData->extra_descr);
	      if (pdesc != NULL)
		{
		  page_to_char (pdesc, ch);
		  return;
		}
	      act ("You don't see anything special about $p.", ch, obj, ch,
		   TO_CHAR);
	      return;
	    }
	}
    }
  for (obj = ch->in_room->more->contents; obj && obj != NULL;
       obj = obj->next_content)
    {
      if (can_see_obj (ch, obj))
	{
	  if (is_approx_name (arg2, obj->pIndexData->name))
	    {
	      if (++number != count)
		continue;
	      send_to_char ("(This object is in the room)\n\r", ch);
	      pdesc = get_extra_descr (arg1, obj->pIndexData->extra_descr);
	      if (pdesc != NULL)
		{
		  page_to_char (pdesc, ch);
		  return;
		}
	      act ("You don't see anything special about $p.", ch, obj, ch,
		   TO_CHAR);
	      return;
	    }
	}
    }
  pdesc = NULL;
  if (ch->in_room->more)
    pdesc = get_extra_descr (arg1, ch->in_room->more->extra_descr);
  if (pdesc != NULL)
    {
      page_to_char (pdesc, ch);
      return;
    }
  send_to_char ("You do not see that here.\n\r", ch);
  return;
lookdirection:
  if (!str_cmp (arg1, "n") || !str_cmp (arg1, "north"))
    door = 0;
  else if (!str_cmp (arg1, "e") || !str_cmp (arg1, "east"))
    door = 1;
  else if (!str_cmp (arg1, "s") || !str_cmp (arg1, "south"))
    door = 2;
  else if (!str_cmp (arg1, "w") || !str_cmp (arg1, "west"))
    door = 3;
  else if (!str_cmp (arg1, "u") || !str_cmp (arg1, "up"))
    door = 4;
  else if (!str_cmp (arg1, "d") || !str_cmp (arg1, "down"))
    door = 5;
  else
    {
      send_to_char ("You can't look that way.\n\r", ch);
      return;
    }
/* 'look direction' */
  if ((pexit = ch->in_room->exit[door]) == NULL)
    {
      if (!ch->ced || !ch->ced->scanning)
	send_to_char ("You do not see anything in that direction.\n\r", ch);
      return;
    }
  if (!ch->ced || !ch->ced->scanning)
    {
      if (pexit->d_info && pexit->d_info->str < 0)
	{
	  send_to_char
	    ("There used to be a door there, but now it's just rubble.\n\r",
	     ch);
	  goto doscan;
	}
      if (pexit->d_info && pexit->d_info->description != NULL
	  && pexit->d_info->description[0] != '\0')
	{
	  char *t;
	  char ed[20];
	  int ct = 0;
	  for (t = pexit->d_info->description; *t != '\0' && ct < 20; t++)
	    {
	      if (*t != '\n' && *t != '\r')
		{
		  ed[ct] = *t;
		  ct++;
		}
	    }
	  ed[ct] = '\0';
	  if (is_number (ed))
	    {
	      int lvnum;
	      ROOM_DATA *lr;
	      lvnum = atoi (ed);
	      if ((lr = get_room_index (lvnum)) != NULL)
		{
		  char ttt[200];
		  send_to_char ("You look and see........\n\r", ch);
		  sprintf (ttt, "%d look", lvnum);
		  do_at (ch, ttt);
		}
	    }
	  else
	    send_to_char (pexit->d_info->description, ch);
	}
      else
	send_to_char ("You do not see anything in that direction.\n\r", ch);
      if ((ROOM_DATA *) pexit->to_room == NULL)
	return;
      if (pexit->d_info && pexit->d_info->keyword != NULL
	  && pexit->d_info->keyword[0] != '\0'
	  && pexit->d_info->keyword[0] != ' ')
	{
	}
      /*end if sc->scanning */
      if (pexit->d_info && IS_SET (pexit->d_info->exit_info, EX_HIDDEN)
	  && IS_SET (pexit->d_info->exit_info, EX_CLOSED))
	{
	  return;
	}
      if (!ch->ced || !ch->ced->scanning)
	{
	  if (pexit->d_info && IS_SET (pexit->d_info->exit_info, EX_CLOSED)
	      && pexit->d_info->keyword != NULL
	      && pexit->d_info->keyword[0] != '\0')
	    {
	      act ("The $T\x1B[0m is closed.", ch, NULL,
		   pexit->d_info->keyword, TO_CHAR);
	      if (!str_cmp ("door", pexit->d_info->keyword)
		  || !str_prefix ("door", pexit->d_info->keyword)
		  || !str_suffix ("door", pexit->d_info->keyword))
		{
		  if (pexit->d_info->str > 50)
		    send_to_char ("The door looks impenetrable!\n\r", ch);
		  else if (pexit->d_info->str > 40)
		    send_to_char ("The door looks very solid!\n\r", ch);
		  else if (pexit->d_info->str > 30)
		    send_to_char ("The door could hold up pretty well.\n\r",
				  ch);
		  else if (pexit->d_info->str > 20)
		    send_to_char
		      ("It wouldn't take a big man long to break.\n\r", ch);
		  else if (pexit->d_info->str > 10)
		    send_to_char ("What could this thing keep out?!\n\r", ch);
		  else
		    send_to_char ("You call that a door?!?!\n\r", ch);
		  return;
		}
	    }
	  else if (pexit->d_info
		   && IS_SET (pexit->d_info->exit_info, EX_ISDOOR)
		   && pexit->d_info->keyword != NULL
		   && pexit->d_info->keyword[0] != '\0')
	    {
	      act ("The $T\x1B[0m is open.", ch, NULL, pexit->d_info->keyword,
		   TO_CHAR);
	      act
		("As you peer for signs of life, you find that you cannot make out any\n\rlifeforms due to the limited line of sight behind the open $T.",
		 ch, NULL, pexit->d_info->keyword, TO_CHAR);
	      return;
	    }
	}
      else
	{
	  if (pexit && (ROOM_DATA *) pexit->to_room != NULL)
	    scan_direction (ch, door);
	  return;
	}
    }
/*end if !ch->scanning */
doscan:
  if ((ROOM_DATA *) pexit->to_room != NULL)
    scan_direction (ch, door);
  return;
}

void
do_examine (CHAR_DATA * ch, char *argy)
{
  char buf[STD_LENGTH];
  char arg[SML_LENGTH];
  SINGLE_OBJECT *obj;
  DEFINE_COMMAND ("examine", do_examine, POSITION_RESTING, 0, LOG_NORMAL,
		  "This command first looks at an object, then looks in the object.")
    one_argy (argy, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Examine what?\n\r", ch);
      return;
    }
  if ((obj = get_obj_wear (ch, arg)) != NULL)
    {
      do_look (ch, obj->pIndexData->name);
      switch (obj->pIndexData->item_type)
	{
	default:
	  break;
	case ITEM_LIGHT:
	  {
	    I_LIGHT *light = (I_LIGHT *) obj->more;
	    if (light->max_light != 0)
	      {
		int percent = PERCENTAGE (light->light_now, light->max_light);
		sprintf (buf, "\n\r%s\x1B[0m %s %s\n\r",
			 capitalize (OOSTR (obj, short_descr)),
			 is_are (OOSTR (obj, short_descr)),
			 !IS_LIT (obj) ? "extinguished." :
			 percent < 10 ? "flickering and sputtering." :
			 percent < 20 ? "flickering." :
			 percent < 30 ? "flickering slightly." :
			 percent < 40 ? "providing ample light." :
			 "glowing brightly.");
		send_to_char (buf, ch);
	      }
	    break;
	  }
	case ITEM_GEM:
	  {
	    I_GEM *gem = (I_GEM *) obj->more;
	    if (gem->mana_now)
	      {
		int percent;
		percent = gem->mana_now;
		sprintf (buf, "\n\r%s\x1B[0m %s\n\r",
			 capitalize (OOSTR (obj, short_descr)),
			 percent < 10 ? "is nearly drained." :
			 percent < 30 ? "glows softly." :
			 percent < 50 ? "glows with moderate energy." :
			 percent < 70 ? "glows brightly." :
			 percent < 90 ? "blinds you with its intense glow." :
			 percent < 110 ? "radiates with a powerful energy." :
			 "glows with an amazing white light.");
		send_to_char (buf, ch);
		return;
	      }
	    break;
	  }
	case ITEM_DRINK_CON:
	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	  send_to_char ("(Equipped) You look inside and see:\n\r", ch);
	  sprintf (buf, "in %s", arg);
	  do_look (ch, buf);
	}
      return;
    }
  if ((obj = get_obj_here (ch, arg, SEARCH_INV_FIRST)) != NULL)
    {
      do_look (ch, obj->pIndexData->name);
      switch (obj->pIndexData->item_type)
	{
	default:
	  break;
	case ITEM_DRINK_CON:
	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	  if (obj->carried_by)
	    send_to_char
	      ("\x1B[37;0m(In Inventory) You look inside and see:\n\r", ch);
	  else
	    send_to_char
	      ("\x1B[37;0m(On the Ground) You look inside and see:\n\r", ch);
	  sprintf (buf, "in %s", arg);
	  do_look (ch, buf);
	  break;
	}
      return;
    }
  do_look (ch, arg);
  return;
}

void
do_dir (CHAR_DATA * ch, char *argy)
{
  DEFINE_COMMAND ("dir", do_dir, POSITION_RESTING, 0, LOG_NORMAL,
		  "See 'exits'.") do_exits (ch, argy);
  return;
}

void
do_exits (CHAR_DATA * ch, char *argy)
{
  extern char *const dir_name[];
  char buf[STD_LENGTH];
  EXIT_DATA *pexit;
  bool found;
  bool fAuto;
  int door;
  char javas[50];
  DEFINE_COMMAND ("exits", do_exits, POSITION_RESTING, 0, LOG_NORMAL,
		  "This displays the available exit directions.  It may not show doors or unaccessable exits.")
    javas[0] = '\0';
/*if (IS_JAVA(ch)) {
        java_exits(ch);
        return;
        }*/
  buf[0] = '\0';
  fAuto = !str_cmp (argy, "auto");
  if (!check_blind (ch))
    return;
  strcpy (buf, fAuto ? "\r [Exits:" : "Obvious exits:\n\r\n\r");
  found = FALSE;
  for (door = 0; door <= 5; door++)
    {
      if (ch->in_room->exit[door] && ch->in_room->exit[door]->to_room &&
	  ch->in_room->exit[door]->to_room->sector_type == SECT_JAVA)
	continue;
      if ((pexit = ch->in_room->exit[door]) != NULL
	  && (ROOM_DATA *) pexit->to_room != NULL)
	{
	  if (fAuto)
	    {
	      if ((ROOM_DATA *) pexit->to_room != NULL
		  && ((ROOM_DATA *) pexit->to_room)->sector_type == SECT_AIR)
		{
		  char buf1[50];
		  if (IS_AFFECTED (ch, AFF_FLYING)
		      || IS_SET (ch->act, PLR_HOLYWALK))
		    {
		      strcat (buf, " ");
		      sprintf (buf1, "<%s>", dir_name[door]);
		      found = TRUE;
		      strcat (buf, buf1);
		      sprintf (javas + strlen (javas), "%c",
			       (char) (door + 1 + 10));
		    }
		  else
		    if (IS_SET
			(((ROOM_DATA *) pexit->to_room)->room_flags,
			 ROOM_FALLSFAST))
		    {
		      strcat (buf, " ");
		      found = TRUE;
		      strcat (buf, dir_name[door]);
		      sprintf (javas + strlen (javas), "%c",
			       (char) (door + 1));
		    }
		}
	      else if ((ROOM_DATA *) pexit->to_room != NULL
		       && ((ROOM_DATA *) pexit->to_room)->sector_type ==
		       SECT_WATER_SWIM)
		{
		  char buf1[50];
		  strcat (buf, " ");
		  sprintf (buf1, "/%s\\", dir_name[door]);
		  found = TRUE;
		  sprintf (javas + strlen (javas), "%c",
			   (char) (door + 1 + 20));
		  strcat (buf, buf1);
		}
	      else if ((ROOM_DATA *) pexit->to_room != NULL
		       && ((ROOM_DATA *) pexit->to_room)->sector_type ==
		       SECT_UNDERWATER)
		{
		  char buf1[50];
		  if (IS_AFFECTED (ch, AFF_BREATH_WATER)
		      || IS_SET (ch->act, PLR_HOLYWALK))
		    {
		      strcat (buf, " ");
		      if (pexit->d_info
			  && IS_SET (pexit->d_info->exit_info, EX_CLOSED))
			sprintf (buf1, "{[%s]}", dir_name[door]);
		      else
			sprintf (buf1, "{%s}", dir_name[door]);
		      found = TRUE;
		      sprintf (javas + strlen (javas), "%c",
			       (char) (door + 1 + 50));
		      strcat (buf, buf1);
		    }
		}
	      else if (pexit->d_info
		       && IS_SET (pexit->d_info->exit_info, EX_HIDDEN)
		       && IS_SET (ch->act, PLR_HOLYLIGHT))
		{
		  char buf1[50];
		  strcat (buf, " ");
		  found = TRUE;
		  sprintf (buf1, "(%s)", dir_name[door]);
		  sprintf (javas + strlen (javas), "%c",
			   (char) (door + 1 + 40));
		  strcat (buf, buf1);
		}
	      else if (pexit->d_info
		       && IS_SET (pexit->d_info->exit_info, EX_CLOSED)
		       && !IS_SET (pexit->d_info->exit_info, EX_HIDDEN))
		{
		  char buf1[50];
		  strcat (buf, " ");
		  found = TRUE;
		  sprintf (buf1, "[%s]", dir_name[door]);
		  sprintf (javas + strlen (javas), "%c",
			   (char) (door + 1 + 30));
		  strcat (buf, buf1);
		}
	      else if (pexit->d_info
		       && IS_SET (pexit->d_info->exit_info, EX_HIDDEN)
		       && IS_SET (pexit->d_info->exit_info, EX_ISDOOR)
		       && !IS_SET (pexit->d_info->exit_info, EX_CLOSED))
		{
		  strcat (buf, " ");
		  found = TRUE;
		  strcat (buf, dir_name[door]);
		}
	      else if (!pexit->d_info
		       || !IS_SET (pexit->d_info->exit_info, EX_CLOSED))
		{
		  strcat (buf, " ");
		  found = TRUE;
		  sprintf (javas + strlen (javas), "%c", (char) (door + 1));
		  strcat (buf, dir_name[door]);
		}
	    }
	  /*Might be extra
	     }
	   */
	  else if (!pexit->d_info
		   || (!IS_SET (pexit->d_info->exit_info, EX_HIDDEN)
		       && !IS_SET (pexit->d_info->exit_info, EX_CLOSED))
		   || IS_SET (ch->act, PLR_HOLYLIGHT))
	    {
	      found = TRUE;
	      sprintf (buf + strlen (buf), "%-5s - %s\n\r",
		       capitalize (dir_name[door]),
		       (!IS_SET (ch->act, PLR_HOLYLIGHT)
			&& room_is_dark ((ROOM_DATA *) pexit->
					 to_room)) ? "Too dark to tell"
		       : (show_room_name
			  (ch, ((ROOM_DATA *) pexit->to_room)->name)));
	    }
	}
    }

  if (!found)
    strcat (buf, fAuto ? " none" : "None.\n\r");
  if (fAuto)
    strcat (buf, "]\n\r");
  if (fAuto && IS_JAVA (ch))
    {
      java_exits (ch, javas);
    }
  else
    {
      send_to_char (buf, ch);
    }
  return;
}

void
do_purse (CHAR_DATA * ch, char *argy)
{
  char buffy[500];
  int found;
  DEFINE_COMMAND ("purse", do_purse, POSITION_SLEEPING, 0, LOG_NORMAL,
		  "This command shows a convenient display of the coins you are carrying, and those in your bank.")
    sprintf (buffy, "You have \x1B[37;1m%ld\x1B[0m coins in the bank.\n\r",
	     ch->pcdata->bank);
  send_to_char (buffy, ch);
  send_to_char ("\n\r", ch);
  if (!ch->gold && !ch->copper)
    {
      send_to_char ("You are carrying no coins on your person.\n\r", ch);
      return;
    }
  if (ch->gold > 0)
    {
      found = 1;
      sprintf (buffy, "\x1B[1;33m%d \x1B[1;33mg\x1B[0;33mo\x1B[1;33ml\x1B[0;33md\x1B[0;37m coins.\n\r", ch->gold);
      send_to_char (buffy, ch);
    }
  if (ch->copper > 0)
    {
      found = 1;
      sprintf (buffy, "\x1B[0;33m%d \x1B[0;33mc\x1B[0;31mo\x1B[0;33mp\x1B[0;31mp\x1B[0;33me\x1B[0;31mr\x1B[0;370m coins.\n\r", ch->copper);
      send_to_char (buffy, ch);
    }
  return;
}


void
do_inventory (CHAR_DATA * ch, char *argy)
{
  char buffy[1000];
  int found;
  DEFINE_COMMAND ("inventory", do_inventory, POSITION_DEAD, 0, LOG_NORMAL, "This command shows what is currently in your inventory.")
    inventory = TRUE;
  if (IS_JAVA (ch))
    {
      send_to_char ("You are carrying:\n\r", ch);
      inventory = TRUE;
      java_clear_inv_window (ch);
      show_list_to_char (ch->carrying, ch, TRUE, TRUE);
      java_nomore_inv_window (ch);
      inventory = FALSE;
    }
  else
    {
      send_to_char ("You are carrying:\n\r", ch);
      found = 0;
      if (ch->gold > 0)
	{
	  found = 1;
	  sprintf (buffy, "\x1B[1;33m%d \x1B[1;33mg\x1B[0;33mo\x1B[1;33ml\x1B[0;33md\x1B[0;37m coins.\n\r", ch->gold);
	  send_to_char (buffy, ch);
	}
      if (ch->copper > 0)
	{
	  found = 1;
	  sprintf (buffy, "\x1B[0;33m%d \x1B[0;33mc\x1B[0;31mo\x1B[0;33mp\x1B[0;31mp\x1B[0;33me\x1B[0;31mr\x1B[0;370m coins.\n\r",
		   ch->copper);
	  send_to_char (buffy, ch);
	}

      show_list_to_char (ch->carrying, ch, TRUE, TRUE);
      if (!found && ch->carrying == NULL)
	send_to_char (" Nothing.\n\r", ch);
    }
  return;
}

/*
void
do_vequipment (CHAR_DATA * ch, char *argy)
{
  SINGLE_OBJECT *obj;
  int iWear;
  bool found;
  DEFINE_COMMAND ("vequipment", do_vequipment, POSITION_DEAD, 0, LOG_NORMAL, "This command shows what objects you currently are wearing/have equipped.")

if (IS_JAVA(ch)) {
        java_send_profile(ch);
        return;
        }

    send_to_char ("You are using:\n\r", ch);
  found = FALSE;
  for (iWear = 0; iWear < MAX_WEAR; iWear++)
    {
      if ((obj = get_eq_char (ch, iWear)) == NULL)
        continue;
      send_to_char (where_name[iWear], ch);
      if (can_see_obj (ch, obj))
        {
          send_to_char (format_obj_to (obj, ch, TRUE), ch);
          send_to_char ("\x1B[0m", ch);
        }
      else
        {
          send_to_char ("\x1B[1;30msomething\x1B[0m.\n\r", ch);
        }
      found = TRUE;
    }

  if (!found)
    send_to_char (" Nothing.\n\r", ch);
  return;
}
*/


void
do_equipment (CHAR_DATA * ch, char *argy)
{
  SINGLE_OBJECT *obj;
  int iWear;
  bool found;
  int where;
  short eqlist;
  DEFINE_COMMAND ("equipment", do_equipment, POSITION_DEAD, 0, LOG_NORMAL, "This command shows what objects you currently are wearing/have equipped.")
    if (IS_PLAYER (ch))
    eqlist = ch->pcdata->eqlist;
  else
    eqlist = 0;

  send_to_char ("You are using:\n\r", ch);
  found = FALSE;
/*
  for (iWear = 0; iWear < MAX_WEAR; iWear++)
  {
    if ((obj = get_eq_char (ch, iWear)) == NULL)
      continue;
    send_to_char (where_name[iWear], ch);
    if (can_see_obj (ch, obj))
    {
      send_to_char (format_obj_to (obj, ch, TRUE), ch);
      send_to_char ("\x1B[0m", ch);
    }
    else
    {
      send_to_char ("\x1B[1;30msomething\x1B[0m.\n\r", ch);
    }
    found = TRUE;
  }
*/
  for (iWear = 0; iWear < MAX_WEAR; iWear++)
    {
      where = (eqlist == 2
	       || eqlist ==
	       3) ? sorted_eqlist[iWear] : unsorted_eqlist[iWear];
      if (where == WEAR_BELT_3 || where == WEAR_BELT_4
	  || where == WEAR_BELT_5)
	continue;		// unused
      if ((obj = get_eq_char (ch, where)) == NULL)
	{
	  if (eqlist == 1 || eqlist == 3)
	    {
	      send_to_char (where_name[where], ch);
	      send_to_char ("\x1B[1;30mnothing\x1B[37;0m\n\r", ch);
	      found = TRUE;
	    }
	  continue;
	}

      send_to_char (where_name[where], ch);
      if (can_see_obj (ch, obj))
	{
	  send_to_char (format_obj_to (obj, ch, TRUE), ch);
	  send_to_char ("\x1B[37;0m", ch);
	}
      else
	{
	  send_to_char ("\x1B[1;30msomething\x1B[37;0m.\n\r", ch);
	}
      found = TRUE;
    }

  if (!found)
    send_to_char (" Nothing.\n\r", ch);
  return;
}


/* MARMAR */
/* Added reporting of level and warpoints */

void
do_report (CHAR_DATA * ch, char *argy)
{
  char buf[SML_LENGTH];
  DEFINE_COMMAND ("report", do_report, POSITION_SLEEPING, 0, LOG_NORMAL, "This command reports your condition to the room you're in.  It works while asleep as well.")
sprintf (buf, "You report: \x1B[0;37mI have \x1B[1;31m%d\x1B[0;37m/\x1B[1;31m%d \x1B[1;35mh\x1B[0;35mp\x1B[1;35ms\x1B[0;37m, and \x1B[1;31m%d\x1B[0;37m/\x1B[1;31m%d \x1B[1;31mm\x1B[0;31move\x1B[1;31ms\x1B[0;37m. I am \x1B[1;36ml\x1B[0;36meve\x1B[1;36ml \x1B[1;31m%d\x1B[0;37m, have \x1B[1;31m%d \x1B[1;31mw\x1B[0;31marpoint\x1B[1;31ms\x1B[0;37m, and have \x1B[1;31m%d \x1B[1;37mr\x1B[1;30memort\x1B[1;37ms\x1B[1;37m.\n\r", 
	     ch->hit, ch->max_hit, ch->move, ch->max_move, ch->pcdata->level, ch->pcdata->warpoints, ch->pcdata->remort_times);
  send_to_char (buf, ch);
  sprintf (buf,"$n reports: \x1B[0;37mI have \x1B[1;31m%d\x1B[0;37m/\x1B[1;31m%d \x1B[1;35mh\x1B[0;35mp\x1B[1;35ms\x1B[0;37m, and \x1B[1;31m%d\x1B[0;37m/\x1B[1;31m%d \x1B[1;31mm\x1B[0;31move\x1B[1;31ms\x1B[0;37m. I am \x1B[1;36ml\x1B[0;36meve\x1B[1;36ml  \x1B[1;31m%d\x1B[0;37m, have \x1B[1;31m%d \x1B[1;31mw\x1B[0;31marpoint\x1B[1;31ms\x1B[0;37m, and have \x1B[1;31m%d \x1B[1;37mr\x1B[1;30memort\x1B[1;37ms\x1B[0;37m.\n\r", 
 ch->hit, ch->max_hit, ch->move, ch->max_move, ch->pcdata->level, ch->pcdata->warpoints, ch->pcdata->remort_times );
  act (buf, ch, NULL, NULL, TO_ROOM);
  return;
}
void
do_boast (CHAR_DATA * ch, char *argy)
{ 
  char buf[SML_LENGTH];
  DEFINE_COMMAND ("boast", do_boast, POSITION_SLEEPING, 0, LOG_NORMAL, "This command Boast to other players. It works while asleep as well.")
  
    sprintf (buf,
             "You smile widely and boast: I have \x1B[1m\x1B[31m%d\x1B[37;0m max hp I am level \x1B[1m\x1B[31m%d\x1B[37;0m and have \x1B[1m\x1B[31m%d\x1B[37;0m warpoints.\n\r",
             ch->max_hit, ch->pcdata->level, ch->pcdata->warpoints);
  send_to_char (buf, ch);
  sprintf (buf, "$n smiles widley and boasts: I have \x1B[1m\x1B[31m%d\x1B[37;0m max hp  I am also level \x1B[1m\x1B[31m%d\x1B[37;0m and have \x1B[1m\x1B[31m%d\x1B[37;0m warpoints.",
           ch->max_hit, ch->pcdata->level, ch->pcdata->warpoints);
  act (buf, ch, NULL, NULL, TO_ROOM);
  return;
}
 

void
do_password (CHAR_DATA * ch, char *argy)
{
  char arg1[SML_LENGTH];
  char arg2[SML_LENGTH];
  char *pArg;
  char *pwdnew;
  char *p;
  char cEnd;
  DEFINE_COMMAND ("password", do_password, POSITION_DEAD, 0, LOG_NEVER, "This command allows you to change your password.")
    if (IS_MOB (ch))
    return;
/*
   * Can't use one_argy here because it smashes case.
   * So we just steal all its code. Bleagh.
 */
  pArg = arg1;
  while (isspace (*argy))
    argy++;
  cEnd = ' ';
  if (*argy == '\'' || *argy == '"')
    cEnd = *argy++;
  while (*argy != '\0')
    {
      if (*argy == cEnd)
	{
	  argy++;
	  break;
	}
      *pArg++ = *argy++;
    }

  *pArg = '\0';
  pArg = arg2;
  while (isspace (*argy))
    argy++;
  cEnd = ' ';
  if (*argy == '\'' || *argy == '"')
    cEnd = *argy++;
  while (*argy != '\0')
    {
      if (*argy == cEnd)
	{
	  argy++;
	  break;
	}
      *pArg++ = *argy++;
    }

  *pArg = '\0';
  if (arg1[0] == '\0' || arg2[0] == '\0')
    {
      send_to_char ("Syntax: password <old> <new>.\n\r", ch);
      return;
    }

  if (strcmp (crypt (arg1, ch->pcdata->pwd), ch->pcdata->pwd))
    {
      WAIT_STATE (ch, 40);
      send_to_char ("Wrong password. Wait 10 seconds.\n\r", ch);
      return;
    }

  if (strlen (arg2) < 5)
    {
      send_to_char ("New password must be at least five characters long.\n\r",
		    ch);
      return;
    }

/*
   * No tilde allowed because of player file format.
 */
  pwdnew = crypt (arg2, NAME (ch));
  for (p = pwdnew; *p != '\0'; p++)
    {
      if (*p == '~')
	{
	  send_to_char ("New password not acceptable, try again.\n\r", ch);
	  return;
	}
    }

  if (ch->pcdata->pwd != NULL)
    free_string (ch->pcdata->pwd);
  ch->pcdata->pwd = str_dup (pwdnew);
  save_char_obj (ch);
  send_to_char ("Password set.\n\r", ch);
  return;
}

/*void do_channels( CHAR_DATA *ch, char *argy )
   {
   char arg[SML_LENGTH];
   DEFINE_COMMAND("channels",do_channels,POSITION_DEAD,0,LOG_NORMAL,"This command allows you to toggle channels on/off using channel -name or channel +name.")

   one_argy( argy, arg );
   if ( arg[0] == '\0' )
   {
   if ( IS_PLAYER(ch) && IS_SET(ch->act, PLR_SILENCE) )
   {
   send_to_char( "You are silenced.\n\r", ch );
   return;
   }
   send_to_char( "\x1B[33;1m\n\rChannels:\x1B[37;0m\n\r\n\r", ch );
   #ifdef GLOBAL_GOSSIP

   send_to_char( !IS_SET(ch->pcdata->deaf, CHANNEL_AUCTION)
   ? "\x1B[37;1m*\x1B[0mAUCTION - You are listening to the auction channel.\n\r"
   : " auction - You are ignoring the auction channel.\n\r",
   ch );
   #endif

   send_to_char( !IS_SET(ch->pcdata->deaf, CHANNEL_NOTIFY)
   ? "\x1B[37;1m*\x1B[0mNOTIFY - You are listening to the game event notification channel.\n\r"
   : " notify - You are ignoring the game event notification channel.\n\r",
   ch );
   send_to_char( !IS_SET(ch->pcdata->deaf, CHANNEL_CHAT)
   ? "\x1B[37;1m*\x1B[0mCHAT - You are listening to the chat channel.\n\r"
   : " chat - You are ignoring the chat channel.\n\r",
   ch );
   if ( LEVEL(ch)>=100 )
   {
   send_to_char( !IS_SET(ch->pcdata->deaf, CHANNEL_IMMTALK)
   ? "\x1B[37;1m*\x1B[0mIMMTALK - You are listening to immortal talk.\n\r"
   : " immtalk - You are ignoring immortal talk.\n\r",
   ch );
   }
   if (clan_number(ch)>0)
   {
   send_to_char( !IS_SET(ch->pcdata->deaf, CHANNEL_CLANTALK)
   ? "\x1B[37;1m*\x1B[0mCLANTALK - You are listening to clan telepathy.\n\r"
   : " clantalk - You are ignoring clan telepathy.\n\r",
   ch );
   }
   send_to_char( !IS_SET(ch->pcdata->deaf, CHANNEL_YELL)
   ? "\x1B[37;1m*\x1B[0mYELL - You are listening to yells/gossips.\n\r"
   : " yell - You are ignoring yells.\n\r",
   ch );
   }

   else
   {
   bool fClear;
   int bit;
   if ( arg[0] == '+' ) fClear = TRUE;
   else if ( arg[0] == '-' ) fClear = FALSE;
   else
   {
   send_to_char( "Channels -channel or +channel?\n\r", ch );
   return;
   }
   if ( !str_cmp( arg+1, "chat" ) ) bit = CHANNEL_CHAT;
   else if ( !str_cmp( arg+1, "immtalk" ) ) bit = CHANNEL_IMMTALK;
   else if ( !str_cmp( arg+1, "auction" ) ) bit = CHANNEL_AUCTION;
   else if ( !str_cmp( arg+1, "clantalk" ) ) bit = CHANNEL_CLANTALK;
   else if ( !str_cmp( arg+1, "notify" ) ) bit = CHANNEL_NOTIFY;
   else if ( !str_cmp( arg+1, "yell" ) ) bit = CHANNEL_YELL;
   else
   {
   send_to_char( "Set or clear which channel?\n\r", ch );
   return;
   }
   if ( fClear )
   {
   char buf[50];
   REMOVE_BIT (ch->pcdata->deaf, bit);
   sprintf(buf,"You now are aware of %ss.\n\r",arg+1);
   send_to_char(buf,ch);
   }
   else
   {
   char buf[50];
   SET_BIT (ch->pcdata->deaf, bit);
   sprintf(buf,"You now ignore %ss.\n\r",arg+1);
   send_to_char(buf,ch);
   }
   }
   return;
   }*/

void
do_actual_search (CHAR_DATA * ch, char *argy)
{
  char buf[STD_LENGTH];
  SINGLE_OBJECT *obj;
  CHAR_DATA *oo;
  char arg1[STD_LENGTH];
  short door;
  int found = 0;
  door = -1;
  if (IS_MOB (ch))
    return;
  NEW_POSITION (ch, POSITION_STANDING);
  sprintf (buf, "You search the room, but find nothing.\n\r");
  if (argy[0] != '\0')
    strcpy (arg1, argy);
  if (!str_cmp (arg1, "n") || !str_cmp (arg1, "north"))
    door = 0;
  else if (!str_cmp (arg1, "e") || !str_cmp (arg1, "east"))
    door = 1;
  else if (!str_cmp (arg1, "s") || !str_cmp (arg1, "south"))
    door = 2;
  else if (!str_cmp (arg1, "w") || !str_cmp (arg1, "west"))
    door = 3;
  else if (!str_cmp (arg1, "u") || !str_cmp (arg1, "up"))
    door = 4;
  else if (!str_cmp (arg1, "d") || !str_cmp (arg1, "down"))
    door = 5;
  if (door >= 0)
    {
      /* 'look direction' */
      if ((ch->in_room->exit[door]) == NULL)
	{
	  send_to_char ("You see nothing special in that direction.\n\r", ch);
	  return;
	}
      if (!ch->in_room->exit[door]->d_info
	  || ch->in_room->exit[door]->d_info->keyword == NULL
	  || ch->in_room->exit[door]->d_info->keyword[0] == '\0')
	{
	  send_to_char ("You see nothing special in that direction.\n\r", ch);
	  return;
	}
      if (ch->in_room->exit[door]->d_info && !IS_SET (ch->in_room->exit[door]->d_info->exit_info, EX_HIDDEN))
	{
	  arg1[1] = '\0';
	  do_look (ch, arg1);
	  return;
	}
      if (ch->in_room->exit[door]->d_info
	  && ch->in_room->exit[door]->d_info->keyword != NULL
	  && ch->in_room->exit[door]->d_info->keyword[0] != '\0')
	{
	  if (number_range (0, 120) < UMAX (8, ch->pcdata->learned[gsn_search]) || dice (1, 500) == 149)
	    {
	      sprintf (buf, "You discovered a %s!\n\r", ch->in_room->exit[door]->d_info->keyword);
	      send_to_char (buf, ch);
	      skill_gain (ch, gsn_search, FALSE);
	      return;
	    }
	  else
	    {
	      sprintf (buf, "You see nothing special in that direction.\n\r");
	      send_to_char (buf, ch);
	      return;
	    }
	}
      return;
    }

  for (obj = ch->in_room->more->contents; obj != NULL;
       obj = obj->next_content)
    {
      if (number_percent () < ch->pcdata->learned[gsn_search]
      /*if (number_range (0, 120) < UMAX (8, ch->pcdata->learned[gsn_search])*/
           && IS_SET (obj->extra_flags, ITEM_UNSEEN) && IS_SET (obj->pIndexData->wear_flags, ITEM_TAKE))
	{
	  sprintf (buf, "You search the room and find %s.\n\r", OOSTR (obj, short_descr));
	  send_to_char (buf, ch);
	  REMOVE_BIT (obj->extra_flags, ITEM_UNSEEN);
	  get_obj (ch, obj, NULL);
	  skill_gain (ch, gsn_search, FALSE);
	  found = 1;
	  return;
	}
    }

  for (door = 0; door < 5; door++)
    {
      if (number_percent () < ch->pcdata->learned[gsn_search])
      /*if (number_range (0, 120) > UMAX (15, ch->pcdata->learned[gsn_search] + 10))*/
	continue;
      if (ch->in_room->exit[door] != NULL && ch->in_room->exit[door]->d_info && IS_SET (ch->in_room->exit[door]->d_info->exit_info, EX_HIDDEN))
	{
	  send_to_char ("You seem to notice something strange; an exit might be hidden here.\n\r", ch);
	  send_to_char ("It might be wise to search <direction> to try to find the hidden exit.\n\r", ch);
	  return;
	}
    }



  for (oo = ch->in_room->more->people; oo != NULL; oo = oo->next_in_room)
    if (number_percent () < ch->pcdata->learned[gsn_search])
    /*if (number_range (0, 120) < UMAX (8, ch->pcdata->learned[gsn_search]))*/
      {
	if (IS_AFFECTED (oo, AFF_HIDE) && number_range (1, 2) == 2)
	  {
	    act ("$n search the room and find $N!", ch, NULL, oo, TO_NOTVICT);
	    act ("You search the room and find $N!", ch, NULL, oo, TO_CHAR);
	    act ("$n searches the room and finds your hiding spot!", ch, NULL,
		 oo, TO_VICT);
	    REMOVE_BIT (oo->affected_by, AFF_HIDE);
	    skill_gain (ch, gsn_search, FALSE);
	    return;
	  }
      }


  if (!found)
    send_to_char (buf, ch);
  return;
}

void
do_search (CHAR_DATA * ch, char *argy)
{
  DEFINE_COMMAND ("search", do_search, POSITION_STANDING, 0, LOG_NORMAL,
		  "This command lets you search <direction> or search the room for players and/or hidden obejcts.")
    if (IS_MOB (ch))
    return;
  if (RIDING (ch) != NULL)
    {
      send_to_char ("You cannot search while riding!\n\r", ch);
      return;
    }

  if (ch->position == POSITION_SEARCHING)
    {
      send_to_char ("You're already doing a search!\n\r", ch);
      return;
    }

  send_to_char ("You begin to search...\n\r", ch);
  act ("$n looks as if $e is searching for something...", ch, NULL, ch, TO_ROOM);
  ch->pcdata->tickcounts = 9;
  if (strlen (argy) > 30)
    return;
  check_temp (ch);
  if (argy[0] != '\0')
    strcpy (ch->pcdata->temp->temp_string, argy);
  if (argy[0] == '\0')
    ch->pcdata->temp->temp_string[0] = '\0';
  NEW_POSITION (ch, POSITION_SEARCHING);
  return;
}
