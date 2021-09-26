#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"

CHAR_DATA *
findmob (void)
{
  CHAR_DATA *cd;
  for (cd = char_list; cd != NULL; cd = cd->next)
    {
      if (IS_MOB (cd))
	return cd;
    }
  return char_list;
}

char *
itoa (int n)
{
  static char t[50];
  sprintf (t, "%d", n);
  return t;
}


void
createobject (CHAR_DATA * ch, int vnum)
{
  OBJ_PROTOTYPE *oid;
  SINGLE_OBJECT *obj;
  if ((oid = get_obj_index (vnum)) == NULL)
    return;
  obj = create_object (oid, 1);
  obj_to (obj, ch);
  return;
}

void
do_destroy (CHAR_DATA * mob, int i)
{
  SINGLE_OBJECT *oo;
  for (oo = mob->carrying; oo != NULL; oo = oo->next_content)
    {
      if (oo->pIndexData->vnum == i)
	{
	  free_it (oo);
	  break;
	}
    }
  return;
}

void
do_scriptr (CHAR_DATA * ch, char *argy)
{
  char arg1[500];
  char arg2[500];
  char arg3[500];
  int vnum;
  MOB_PROTOTYPE *mid;
  DEFINE_COMMAND ("rscript", do_scriptr, POSITION_DEAD, 110, LOG_NORMAL,
		  "Format is rscript <vnum> reload.") argy =
    one_argy (argy, arg1);
  argy = one_argy (argy, arg2);
  argy = one_argy (argy, arg3);
  if ((mid = get_mob_index (vnum = atoi (arg1))) == NULL)
    {
      send_to_char ("Invalid mob vnum specified.\n\r", ch);
      return;
    }
  if (!mid->script_fn)
    {
      send_to_char ("This mob has no scriptfile.\n\r", ch);
      return;
    }
  if (!str_prefix ("reload", arg2))
    {
      load_scripts_2 (mid->script_fn, mid);
      send_to_char ("Script reloaded.\n\r", ch);
    }
  return;
}

void
do_list_done_quests (CHAR_DATA * ch, char *argy)
{
  CHAR_DATA *victim;
  int i;
  bool all_flag = FALSE;
  bool list_flag = FALSE;
  bool clear_flag = FALSE;
  DEFINE_COMMAND ("oldquests", do_list_done_quests, POSITION_DEAD, 0,
		  LOG_NORMAL,
		  "Shows you completed quests, etc...") hugebuf_o[0] = '\0';
  if (IS_MOB (ch))
    return;
  if (!str_prefix ("clear", argy) && ch->pcdata->level >= 110)
    {
      clear_flag = TRUE;
      argy += 5;
      send_to_char ("Cleared.\n\r", ch);
    }
  if (!str_prefix ("all", argy) && ch->pcdata->level >= 110)
    {
      all_flag = TRUE;
      argy += 3;
    }
  if (!str_prefix ("list", argy) && ch->pcdata->level >= 110)
    {
      list_flag = TRUE;
      argy += 4;
    }
  if (*argy == ' ')
    argy++;
  victim = ch;
  if (ch->pcdata->level >= 110 && strlen (argy) > 2)
    {
      if ((victim = get_char_world (ch, argy)) == NULL)
	{
	  victim = ch;
	}
    }
  for (i = 0; i < 1000; i++)
    {
      if (clear_flag)
	{
	  victim->pcdata->complete[i] = FALSE;
	  continue;
	}
      if ((list_flag || victim->pcdata->complete[i]) && questname[i] != NULL)
	{
	  if (!list_flag && *questname[i] == '*' && !all_flag)
	    continue;
	  if (victim->pcdata->complete[i])
	    sprintf (hugebuf_o + strlen (hugebuf_o),
		     "[%3d] '%s' - Complete.\n\r", i, questname[i]);
	  else
	    sprintf (hugebuf_o + strlen (hugebuf_o),
		     "[%3d] '%s' - Incomplete.\n\r", i, questname[i]);
	}
    }
  page_to_char (hugebuf_o, ch);
  return;
}

char *
cscr (char *txt, CHAR_DATA * mob, char *player, SINGLE_OBJECT * obj)
{
  static char strr[2048];
  char lastword[256];
  char *runthru;
  int posi_w = 0;
  strr[0] = '\0';
  for (runthru = txt; *runthru != '\0'; runthru++)
    {
      lastword[posi_w] = *runthru;
      posi_w++;
      if (*runthru == ' ')
	{
	  lastword[posi_w] = '\0';
	  if (!str_cmp (lastword, "@mobn "))
	    {
	      strcat (strr, NAME (mob));
	    }
	  else if (!str_cmp (lastword, "@playern "))
	    {
	      strcat (strr, player);
	    }
	  else if (!str_cmp (lastword, "@objn ") && obj)
	    {
	      strcat (strr, obj->pIndexData->short_descr);
	    }
	  else
	    strcat (strr, lastword);
	  posi_w = 0;
	}
    }
  lastword[posi_w] = '\0';
  strcat (strr, lastword);
  return strr;
}

int
grab_num (char *arg)
{
  int cur_num = 0;
  char *look;
  for (look = arg; *look != '\0'; look++)
    {
      if ((*look) < '0' || (*look) > '9')
	continue;
      cur_num *= 10;
      cur_num += ((*look) - '0');
    }
  return cur_num;
}

int
evaluate_random (char *arg)
{
  int num1 = 0;
  int num2 = 0;
  char *look;
  int cur_num = 0;
  bool on_one = TRUE;
  bool fsp = FALSE;
  for (look = arg; *look != '\0'; look++)
    {
      if (!fsp && ((*look) < '0' || (*look) > '9'))
	continue;
      if (!fsp)
	{
	  on_one = TRUE;
	  fsp = TRUE;
	}
      if (*look == ' ' || *look == ',')
	{
	  num1 = cur_num;
	  cur_num = 0;
	  on_one = FALSE;
	  continue;
	}
      if ((*look) < '0' || (*look) > '9')
	continue;
      cur_num *= 10;
      cur_num += ((*look) - '0');
    }
  num2 = cur_num;
  return number_range (num1, num2);
}

CHAR_DATA *
find_player (char *pname)
{
  DESCRIPTOR_DATA *d;
  for (d = descriptor_list; d != NULL; d = d->next)
    {
      if (d->character)
	if (!str_cmp (d->character->pcdata->name, pname))
	  return d->character;
    }
  return NULL;
}

void
do_scriptline (CHAR_DATA * mob, CHAR_DATA * player, SINGLE_OBJECT * obj)
{
  SCRIPT_DAT *sdat;
  TRIGGY_DATA *trig;
  char plr[500];
  if (IS_PLAYER (mob))
    return;
  check_ced (mob);
  if (player)
    {
      check_ced (player);
    }
  mob->ced->wait_type = 0;
  if (player)
    {
      strcpy (plr, NAME (player));
    }
  for (;;)
    {
      if (mob->ced->scr_wait != 0)
	{
	  goto done;
	}
      /* A waitstate has been induced */
      mob->ced->cur_line++;	/* Get ready to read the next line */
      /* Find the next line */
      for (trig = mob->pIndexData->trig_list; trig != NULL; trig = trig->next)
	{
	  for (sdat = trig->head; sdat != NULL; sdat = sdat->next)
	    {
	      if (sdat->label == mob->ced->cur_line)
		goto found_it;
	    }
	}
      mob->ced->cur_line = 0;
      return;			/*Scriptline not found, return */
    found_it:			/*Found the script line! Begin execution... */
      if (!player && sdat->player != NULL)
	{
	  strcpy (plr, sdat->player);
	  free_m (sdat->player);
	  sdat->player = NULL;
	}
      /*INTERPRETTING THE SCRIPT BELOW */
      {
	char argy[2048];
	char arg1[200];
	char arg2[200];
	char arg3[200];
	char arg4[200];
	char arg5[200];
	char arg6[200];
	char arg7[200];
	char arg8[200];
	char arg9[200];
	char arg10[200];
	char *argie = argy;
	strcpy (argy, sdat->to_do);
	argie = one_argy (argie, arg1);
	argie = one_argy (argie, arg2);
	argie = one_argy (argie, arg3);
	argie = one_argy (argie, arg4);
	argie = one_argy (argie, arg5);
	argie = one_argy (argie, arg6);
	argie = one_argy (argie, arg7);
	argie = one_argy (argie, arg8);
	argie = one_argy (argie, arg9);
	argie = one_argy (argie, arg10);
	if (!str_cmp (arg1, "donetrig"))
	  goto done;
	if (!str_prefix ("if", arg1))
	  {
	    int left_side;
	    int right_side;
	    char comparison = 0;
	    int num_if;
	    bool result = FALSE;
	    num_if = grab_num (arg1);
	    if (!str_prefix ("temp", arg2))
	      {
		int i;
		i = atoi (arg3);
		if (i >= 0 && i < 1000)
		  {
		    CHAR_DATA *tch;
		    if ((tch = find_player (plr)) != NULL)
		      {
			if (tch->pcdata->temp_flag[i])
			  goto fnn;
		      }
		  }
		goto res_failed;
	      }
	    if (!str_cmp (arg2, "complete"))
	      {
		int i;
		if ((i = lookup_questname (arg3)) >= 0)
		  {
		    CHAR_DATA *tch;
		    if ((tch = find_player (plr)) != NULL)
		      {
			if (tch->pcdata->complete[i])
			  goto fnn;
			if (!tch->pcdata->complete[i])
			  goto res_failed;
		      }
		  }
		i = atoi (arg3);
		if (i >= 0 && i < 1000)
		  {
		    CHAR_DATA *tch;
		    if ((tch = find_player (plr)) != NULL)
		      {
			if (tch->pcdata->complete[i])
			  goto fnn;
			if (tch->pcdata->complete[i])
			  goto res_failed;
		      }
		  }
		goto res_failed;
	      }
	    if (!str_cmp (arg2, "in_progress"))
	      {
		int i;
		i = atoi (arg3);
		if (i >= 0 && i < 1000)
		  {
		    CHAR_DATA *tch;
		    if ((tch = find_player (plr)) != NULL)
		      {
			if (tch->pcdata->in_progress[i])
			  goto fnn;
		      }
		  }
		goto res_failed;
	      }
	    if (!str_cmp (arg2, "=") || !str_prefix ("equal", arg2))
	      comparison = 1;
	    else if (!str_cmp (arg2, "<") || !str_prefix ("less", arg2))
	      comparison = 2;
	    else if (!str_cmp (arg2, ">") || !str_prefix ("great", arg2))
	      comparison = 3;
	    if (!str_prefix ("rand", arg3) || !str_prefix ("'rand", arg3))
	      left_side = evaluate_random (arg3);
	    else
	      left_side = atoi (arg3);
	    if (!str_prefix ("rand", arg4) || !str_prefix ("'rand", arg4))
	      right_side = evaluate_random (arg4);
	    else
	      right_side = atoi (arg4);
	    if (comparison == 1 && left_side == right_side)
	      result = TRUE;
	    else if (comparison == 2 && left_side < right_side)
	      result = TRUE;
	    else if (comparison == 3 && left_side > right_side)
	      result = TRUE;
	  res_failed:
	    if (!result)
	      {
		/* Result failed.. find otherwise# */
		SCRIPT_DAT *st;
		for (st = trig->head; st != NULL; st = st->next)
		  {
		    if ((!str_prefix ("otherwise", st->to_do)
			 || !str_prefix ("else", st->to_do))
			&& grab_num (st->to_do) == num_if)
		      {
			mob->ced->cur_line = st->label;
			goto fnn;
		      }
		  }
	      }
	  fnn:
	    continue;
	  }
	else if (!str_prefix ("jump", arg1))
	  {
	    SCRIPT_DAT *st;
	    for (st = trig->head; st != NULL; st = st->next)
	      {
		if (!str_prefix (arg2, st->to_do))
		  {
		    mob->ced->cur_line = st->label;
		    goto fnnt;
		  }
	      }
	  fnnt:
	    if (mob)
	      {
	      };
	  }
	else if (!str_prefix ("otherwise", arg1)
		 || !str_prefix ("else", arg1))
	  continue;
	if (!str_cmp (arg1, "do"))
	  {
	    interpret (mob, cscr (sdat->to_do + 3, mob, plr, obj));
	  }
	else if (!str_cmp (arg1, "pset"))
	  {
	    do_pset (mob, cscr (sdat->to_do + 4, mob, plr, obj));
	  }
	else if (!str_cmp (arg1, "award"))
	  {
	    char tmp[500];
	    sprintf (tmp, "%s %s %s", arg2, arg3, arg4);
	    do_award_skill (mob, tmp);
	  }
	else if (!str_cmp (arg1, "createobj"))
	  {
	    OBJ_PROTOTYPE *oid;
	    SINGLE_OBJECT *obj2;
	    int vnum;
	    vnum = atoi (arg2);
	    if (vnum > 1)
	      {
		if ((oid = get_obj_index (vnum)) != NULL)
		  {
		    obj2 = create_object (oid, 5);
		    if (!CAN_WEAR (obj2, ITEM_TAKE))
		      {
			obj_to (obj2, mob->in_room);
		      }
		    else
		      obj_to (obj2, mob);
		  }
	      }
	  }
	else if (!str_prefix ("destroy", arg1))
	  {
	    SINGLE_OBJECT *oo;
	    int i;
	    i = atoi (arg2);
	    for (oo = mob->carrying; oo != NULL; oo = oo->next_content)
	      {
		if (oo->pIndexData->vnum == i)
		  {
		    free_it (oo);
		    goto kdj;
		  }
	      }
	  kdj:
	    continue;
	  }
	else if (!str_cmp (arg1, "createmob"))
	  {
	    MOB_PROTOTYPE *mid;
	    CHAR_DATA *new_mob;
	    int vnum;
	    vnum = atoi (arg2);
	    if (vnum > 1)
	      {
		if ((mid = get_mob_index (vnum)) != NULL)
		  {
		    new_mob = create_mobile (mid);
		    char_to_room (new_mob, mob->in_room);
		  }
	      }
	  }
	else if (!str_cmp (arg1, "wait"))
	  {
	    mob->ced->scr_wait = UMAX (0, (atoi (arg2)));
	    if (!str_cmp (arg3, "char_tick"))
	      mob->ced->wait_type = WAIT_TYPE_CHAR_TICK;
	    else if (!str_cmp (arg3, "combat_tick"))
	      mob->ced->wait_type = WAIT_TYPE_COMBAT_TICK;
	    else if (!str_cmp (arg3, "small_tick"))
	      mob->ced->wait_type = WAIT_TYPE_SMALL_TICK;
	    else if (!str_cmp (arg3, "quick"))
	      mob->ced->wait_type = WAIT_TYPE_PULSE;
	    else
	      mob->ced->wait_type = 0;
	    for (trig = mob->pIndexData->trig_list; trig != NULL;
		 trig = trig->next)
	      {
		for (sdat = trig->head; sdat != NULL; sdat = sdat->next)
		  {
		    if (sdat->label == mob->ced->cur_line + 1)
		      {
			if (sdat->player != NULL)
			  {
			    free_m (sdat->player);
			  }
			sdat->player = (char *) mem_alloc (strlen (plr) + 1);
			strcpy (sdat->player, plr);
			goto brk;
		      }
		  }
	      }
	  brk:
	    return;
	  }
	else if (!str_cmp (arg1, "disable"))
	  {
	    trig->active = FALSE;
	    goto done;
	  }
      }
      /*END SCRIPT INTERPRETATION */
    }
done:
  mob->ced->cur_line = 0;
  return;
}
