#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "emlen.h"

extern bool noibm;
extern bool kk;
extern bool mb;
extern bool dbs;
ROOM_DATA *iir = NULL;

char *const dirrev[] = {
  "south", "west", "north", "east"
};

const short revdir[] = {
  2, 3, 0, 1, 5, 4
};


char *const dir_name[] = {
  "north", "east", "south", "west", "up", "down"
};

char *const dir_rev[] = {
  "the south", "the west", "the north", "the east", "below", "above"
};

const short rev_dir[] = {
  2, 3, 0, 1, 5, 4
};

const short movement_loss[SECT_MAX] = {
  1, 2, 2, 3, 4, 6, 4, 3, 6, 10, 4, 8, 4, 3, 3, 3
};

bool has_key (CHAR_DATA * ch, int key);
int find_door (CHAR_DATA * ch, char *arg);
char *door_cond (int val);


void
setnext (CHAR_DATA * ch, char *c)
{
  if (ch->fgt->next_command)
    free (ch->fgt->next_command);
  ch->fgt->next_command = mem_alloc (strlen (c) + 1);
  strcpy (ch->fgt->next_command, c);
  java_nc (ch, c);
  return;
}

char *
get_direction (char *arg)
{
  switch (LOWER (arg[0]))
    {
    case 'n':
      return "north";
    case 's':
      return "south";
    case 'e':
      return "east";
    case 'w':
      return "west";
    case 'u':
      return "up";
    case 'd':
      return "down";
    }
  return "anywhere";
}

char *
ans_uppercase (const char *txt)
{
  static char buf[STD_LENGTH];
  char *str;
  char *point;
  buf[0] = '\0';
  if (txt == "")
    {
      return buf;
    }
  if (!txt || txt[0] == '\0')
    {
      return buf;
    }
  str = (char *) txt;
  point = buf;
  if (isalpha (txt[0]))
    {
      sprintf (buf, txt);
      buf[0] = UPPER (buf[0]);
      return buf;
    }
  if (strlen (txt) > 5 && isalpha (txt[1]))
    {
      sprintf (buf, txt);
      buf[1] = UPPER (buf[1]);
      return buf;
    }
  if (strlen (txt) > 5 && isalpha (txt[2]) && txt[2] != 'm')
    {
      sprintf (buf, txt);
      buf[2] = UPPER (buf[2]);
      return buf;
    }
  if (strlen (txt) > 5 && isalpha (txt[3]) && txt[3] != 'm')
    {
      sprintf (buf, txt);
      buf[3] = UPPER (buf[3]);
      return buf;
    }
  sprintf (buf, txt);
  while (*point)
    {
      while (*point != 'm' && *point)
	point++;
      if (!*point)
	return buf;
      point++;
      if (isalpha (*point))
	{
	  *point = UPPER (*point);
	  return buf;
	}
    }
  return buf;
}


TRACK_DATA *
new_track (CHAR_DATA * ch, ROOM_DATA * room)
{
  TRACK_DATA *trk;
  trk = (TRACK_DATA *) mem_alloc ((sizeof (*trk)));
  //trk->ch = ch;
  trk->victname = str_dup(NAME(ch));
  trk->victismob = IS_MOB(ch);
  trk->hours = 24;
  trk->dir_came = 10;
  trk->dir_left = 10;
  trk->next_track_in_room = room->tracks;
  room->tracks = trk;
  return trk;
}

void
do_rawclear (CHAR_DATA * ch)
{
  char ddd[500];
  if (IS_MOB (ch) || ch->desc == NULL)
    return;
  sprintf (ddd, "\x1B[2J\x1B[0m\x1B[1;%dr", ch->pcdata->pagelen);
  write_to_descriptor2 (ch->desc, ddd, 0);
  return;
}

void
do_clear (CHAR_DATA * ch, char *argy)
{
  char ddd[500];
  DEFINE_COMMAND ("cls", do_clear, POSITION_DEAD, 0, LOG_NORMAL, "Clears the screen; defaults window size to pagelen.  VT100/ANSI")
    if (IS_MOB (ch) || ch->desc == NULL)
    return;
  if (IS_JAVA (ch))
    {
/*	sprintf(ddd,"%c%c",(char) 8, (char) 32);
	write_to_descriptor2 (ch->desc, ddd, 0);*/
    }
  else
    {
      sprintf (ddd, "\x1B[2J\x1B[0m\x1B[1;%dr", ch->pcdata->pagelen);
      write_to_descriptor2 (ch->desc, ddd, 0);
    }
  ch->pcdata->resist_summon = 0;
  if (!IS_JAVA (ch))
    {
      set_it_up (ch);
    }
  if (IS_JAVA (ch) || (IS_SET (ch->pcdata->act3, ACT3_MAPPING) && !IS_SET (ch->pcdata->act3, ACT3_DISABLED)))
    {
      REMOVE_BIT (ch->pcdata->act3, ACT3_MAPPING);
      do_drawmort (ch, "");
    }
  if (FIGHTING (ch) && !IS_JAVA (ch) && ch->fgt->field)
    {
      add_ansi (ch, ch->fgt->field);
    }
#ifdef OLD_NOMOVE
  scrn_upd (ch);
#endif
  return;
}

void
check_short_tick_script (CHAR_DATA * fch, CHAR_DATA * ch)
{
  return;
  if (IS_MOB (fch) && fch->pIndexData->trig_list != NULL && fch->ced
      && fch->ced->scr_wait == 0)
    {
      TRIGGY_DATA *trig;
      if (fch->ced->cur_line != 0)
	return;
      for (trig = fch->pIndexData->trig_list; trig != NULL; trig = trig->next)
	{
	  if (!trig->active)
	    continue;
	  if (IS_SET (trig->type, CHECK_SHORT_TICK))
	    {
	      SCRIPT_DAT *scr;
	      for (scr = trig->head; scr->next != NULL; scr = scr->next)
		{
		}
	      fch->ced->cur_line = scr->label - 1;
	      do_scriptline (fch, ch, NULL);
	      return;
	    }
	}
    }
  return;
}

void
check_social_script (CHAR_DATA * fch, CHAR_DATA * ch, char *txt)
{
  return;
  if (IS_MOB (fch) && fch->pIndexData->trig_list != NULL && fch->ced
      && fch->ced->scr_wait == 0)
    {
      TRIGGY_DATA *trig;
      char cur[200];
      char line[500];
      char *t;
      if (fch->ced->cur_line != 0)
	return;
      for (trig = fch->pIndexData->trig_list; trig != NULL; trig = trig->next)
	{
	  if (!trig->active)
	    continue;
	  if (!IS_SET (trig->type, CHECK_SOCIAL))
	    continue;
	  if (!str_cmp ("any", trig->scriptline)
	      || !str_cmp ("any ", trig->scriptline))
	    goto fndo;
	  strcpy (line, trig->scriptline);
	  t = line;
	  while (t != NULL && t != "" && *t != '\0')
	    {
	      t = one_argy (t, cur);
	      if (!str_infix (cur, txt))
		goto fndo;
	    }
	}
      return;
    fndo:
      {
	SCRIPT_DAT *scr;
	for (scr = trig->head; scr->next != NULL; scr = scr->next)
	  {
	  }
	fch->ced->cur_line = scr->label - 1;
	do_scriptline (fch, ch, NULL);
	return;
      }
    }
  return;
}

void
check_say_script (CHAR_DATA * fch, CHAR_DATA * ch, char *txt)
{
  return;
  if (IS_MOB (fch) && fch->pIndexData->trig_list != NULL && fch->ced
      && fch->ced->scr_wait == 0)
    {
      TRIGGY_DATA *trig;
      char cur[200];
      char line[500];
      char *t;
      if (fch->ced->cur_line != 0)
	return;
      for (trig = fch->pIndexData->trig_list; trig != NULL; trig = trig->next)
	{
	  if (!trig->active)
	    continue;
	  if (!IS_SET (trig->type, CHECK_SAY))
	    continue;
	  if (!str_cmp ("any", trig->scriptline)
	      || !str_cmp ("any ", trig->scriptline))
	    goto fnd;
	  strcpy (line, trig->scriptline);
	  if (!str_infix (line, txt))
	    goto fnd;
	  t = line;
	  while (t != NULL && t != "" && *t != '\0')
	    {
	      t = one_argy (t, cur);
	      if (!str_infix (cur, txt))
		goto fnd;
	    }
	}
      return;
    fnd:
      {
	SCRIPT_DAT *scr;
	for (scr = trig->head; scr->next != NULL; scr = scr->next)
	  {
	  }
	fch->ced->cur_line = scr->label - 1;
	do_scriptline (fch, ch, NULL);
	return;
      }
    }
  return;
}

void
check_tell_script (CHAR_DATA * fch, CHAR_DATA * ch, char *txt)
{
  return;
  if (IS_MOB (fch) && fch->pIndexData->trig_list != NULL && fch->ced
      && fch->ced->scr_wait == 0)
    {
      TRIGGY_DATA *trig;
      char cur[200];
      char line[500];
      char *t;
      if (fch->ced->cur_line != 0)
	return;
      for (trig = fch->pIndexData->trig_list; trig != NULL; trig = trig->next)
	{
	  if (!trig->active)
	    continue;
	  if (!IS_SET (trig->type, CHECK_TELL))
	    continue;
	  if (!str_cmp ("any", trig->scriptline)
	      || !str_cmp ("any ", trig->scriptline))
	    goto fnd;
	  strcpy (line, trig->scriptline);
	  t = line;
	  while (t != NULL && t != "" && *t != '\0')
	    {
	      t = one_argy (t, cur);
	      if (!str_infix (cur, txt))
		goto fnd;
	    }
	}
      return;
    fnd:
      {
	SCRIPT_DAT *scr;
	for (scr = trig->head; scr->next != NULL; scr = scr->next)
	  {
	  }
	fch->ced->cur_line = scr->label - 1;
	do_scriptline (fch, ch, NULL);
	return;
      }
    }
  return;
}

void
check_enterroom_script (CHAR_DATA * fch, CHAR_DATA * ch)
{
  return;
  if (IS_PLAYER (ch) && IS_MOB (fch) && fch->pIndexData->trig_list != NULL
      && fch->ced && fch->ced->scr_wait == 0)
    {
      TRIGGY_DATA *trig;
      if (fch->ced->cur_line != 0)
	return;
      for (trig = fch->pIndexData->trig_list; trig != NULL; trig = trig->next)
	{
	  if (!trig->active)
	    continue;
	  if (IS_SET (trig->type, CHECK_ENTER_ROOM))
	    {
	      SCRIPT_DAT *scr;
	      for (scr = trig->head; scr->next != NULL; scr = scr->next)
		{
		}
	      fch->ced->cur_line = scr->label - 1;
	      do_scriptline (fch, ch, NULL);
	      return;
	    }
	}
    }
  return;
}

void
check_given_script (CHAR_DATA * fch, CHAR_DATA * ch, int vnum)
{
  return;
  if (IS_MOB (fch) && fch->pIndexData->trig_list != NULL && fch->ced
      && fch->ced->scr_wait == 0)
    {
      TRIGGY_DATA *trig;
      if (fch->ced->cur_line != 0)
	return;
      for (trig = fch->pIndexData->trig_list; trig != NULL; trig = trig->next)
	{
	  if (!trig->active)
	    continue;
	  if (IS_SET (trig->type, CHECK_GIVEN_OBJ) &&
	      ((!str_prefix ("any", trig->scriptline)) ||
	       (atoi (trig->scriptline) == vnum)))
	    {
	      SCRIPT_DAT *scr;
	      for (scr = trig->head; scr->next != NULL; scr = scr->next)
		{
		}
	      fch->ced->cur_line = scr->label - 1;
	      do_scriptline (fch, ch, NULL);
	      return;
	    }
	}
    }
  return;
}

void
do_leave (CHAR_DATA * ch, char *argy)
{
  char arg[1024];
  ROOM_DATA *r;
  DEFINE_COMMAND ("leave", do_leave, POSITION_STANDING, 0, LOG_NORMAL, "Allows you to leave a vehicle.");
  if (!ch->in_room->more || !IS_SET (ch->in_room->room_flags, ROOM_ISVEHICLE))
    {
      send_to_char ("You aren't inside a vehicle.\n\r", ch);
      return;
    }
  if (!ch->in_room->more->linked_to)
    return;
  if (ch->in_room->more->linked_to->sector_type == SECT_WATER_NOSWIM)
    {
      send_to_char ("The water is too deep here!\n\r", ch);
      return;
    }
  if (FIGHTING (ch))
    {
      send_to_char
	("You cannot leave the vehicle while fighting.  Try using FLEE.\n\r",
	 ch);
      return;
    }
  if (ch->position != POSITION_STANDING && ch->position != POSITION_STOPCAST)
    {
      send_to_char ("You must be standing to leave the vehicle.\n\r", ch);
      return;
    }
  r = ch->in_room;
  sprintf (arg, "$n just left.");
  act (arg, ch, NULL, ch, TO_ROOM);
  char_from_room (ch);
  char_to_room (ch, r->more->linked_to);
  do_look (ch, "auto");
  sprintf (arg, "$n gets off of %s.", r->name);
  act (arg, ch, NULL, ch, TO_ROOM);
  return;
}

void
do_enter (CHAR_DATA * ch, char *argy)
{
  char arg[SML_LENGTH];
  SINGLE_OBJECT *obj;
  CHAR_DATA *ui;
  ROOM_DATA *oo;
  ROOM_DATA *dest;
  DEFINE_COMMAND ("enter", do_enter, POSITION_STANDING, 0, LOG_NORMAL, "Allows you to enter an enterable objject.");

  one_argy (argy, arg);
  for (oo = ch->in_room->linkage; oo != NULL; oo = oo->linkage)
    {
      if (oo->more && oo->more->obj_description
	  && oo->more->obj_description[0] != '\0'
	  && is_name (arg, oo->more->obj_description))
	{
	  if (!(ch->position == POSITION_STANDING && !FIGHTING (ch)))
	    {
	      send_to_char ("You must be standing to enter!\n\r", ch);
	      return;
	    }
	  sprintf (arg, "$n hops aboard %s.", oo->name);
	  act (arg, ch, NULL, ch, TO_ROOM);
	  ui = oo->more->people;
	  char_from_room (ch);
	  char_to_room (ch, oo);
	  if (ui)
	    {
	      char_from_room (ui);
	      char_to_room (ui, oo);
	    }
	  do_look (ch, "auto");
	  sprintf (arg, "$n hops aboard.");
	  act (arg, ch, NULL, ch, TO_ROOM);
	  return;
	}
    }
      if (argy[0] == '\0' || argy == "")
        {
          send_to_char ("Enter What!?\n\r", ch);
          return;
        }
  if ((obj = get_obj_here (ch, arg, SEARCH_ROOM_FIRST)) == NULL)
    {
      send_to_char ("There is nothing like that to enter.\n\r", ch);
      return;
    }
  if ((obj->pIndexData->item_type != ITEM_FURNITURE))
    {
      send_to_char ("How do you expect to enter that?\n\r", ch);
      return;
    }
  else
    {
      I_FURNITURE *furn = (I_FURNITURE *) obj->more;
      if (!IS_SET (furn->type, FURN_EXIT))
	{
	  send_to_char ("How do you expect to enter that?\n\r", ch);
	  return;
	}
      if ((dest = get_room_index (furn->to_room_vnum)) == NULL)
	{
	  send_to_char ("You can't go that way.\n\r", ch);
	  return;
	}
      act ("$n enters $p\e[0m.", ch, obj, NULL, TO_ROOM);
      act ("You enter $p\e[0m.", ch, obj, NULL, TO_CHAR);
      if (furn->strip_possessions == 1)
	{
	  send_to_char ("You feel a strange force ripping at you...\n\r", ch);
	  strip_nonworn (ch);
	}
      char_from_room (ch);
      char_to_room (ch, dest);
      do_look (ch, "auto");

      act ("$n enters from $p\e[0m.", ch, obj, NULL, TO_ROOM);
      if (MOUNTED_BY (ch) != NULL)
	{
	  char_from_room (MOUNTED_BY (ch));
	  char_to_room (MOUNTED_BY (ch), dest);
	  do_look (MOUNTED_BY (ch), "auto");
	}
      if (RIDING (ch) != NULL)
	{
	  char_from_room (RIDING (ch));
	  char_to_room (RIDING (ch), dest);
	  do_look (RIDING (ch), "auto");
	}
    }
  return;
}


bool
find_vehicle (CHAR_DATA * ch, int sector_type)
{
  SINGLE_OBJECT *obj;
  bool found;
  found = FALSE;
  if ((sector_type == SECT_WATER_NOSWIM || sector_type == SECT_MOUNTAIN
       || sector_type == SECT_UNDERWATER || sector_type == SECT_WATER_SWIM)
      && IS_PLAYER (ch))
    for (obj = ch->carrying; (obj != NULL) && (found != TRUE);
	 obj = obj->next_content)
      {
	if (obj->pIndexData->item_type == ITEM_VEHICLE)
	  {
	    I_VEHICLE *veh = (I_VEHICLE *) obj->more;
	    found = (veh->sector_use == sector_type);
	    if (found)
	      break;
	  }
      }
  return found;
}

bool
no_boat (ROOM_DATA * rid)
{
  SINGLE_OBJECT *o;
  if (!rid->more)
    return TRUE;
  for (o = rid->more->contents; o != NULL; o = o->next_content)
    if (o->pIndexData->item_type == ITEM_BOAT)
      return FALSE;
  return TRUE;
}

bool check_super = TRUE;

bool
move_char (CHAR_DATA * ch, int door)
{
  static int depth = 0;
  CHAR_DATA *hors;
  int amt = 0;
  CHAR_DATA *fch;
  CHAR_DATA *fch_next;
  ROOM_DATA *in_room;
  ROOM_DATA *to_room = NULL;
  CHAR_DATA *temch;
  TRACK_DATA *trk;
  char buf2[STD_LENGTH];
  char buf[STD_LENGTH];
  EXIT_DATA *pexit;
  bool fndd = FALSE;
  bool flagone;
  bool flagtwo;
  bool flee = FALSE;
  flagone = FALSE;
  flagtwo = FALSE;
  check_room_more (ch->in_room);
  if (door > 18)
  {
    flee = TRUE;
    door -= 20;
  }
  if (!flee)
  {
    for (fch = ch->in_room->more->people; fch != NULL;
        fch = fch->next_in_room)
    {
      if (FIGHTING (fch) && FIGHTING (fch) == ch)
      {
        send_to_char ("You are in the middle of combat!\n\r", ch);
        return FALSE;
      }
    }
  }
  if ((LEVEL (ch) < 100 || IS_MOB (ch)) && ch->in_room->exit[door]
      && ch->in_room->exit[door]->to_room
      && ch->in_room->exit[door]->to_room->sector_type == SECT_JAVA)
  {
    send_to_char ("You can't go any further in that direction.\n\r", ch);
    return FALSE;
  }
  if (IS_SET (ch->in_room->room_flags, ROOM_ISVEHICLE) && ch->in_room->more
      && ch->in_room->more->linked_to)
  {
    CHAR_DATA *cc;
    CHAR_DATA *c;
    CHAR_DATA *inir;
    CHAR_DATA *strr;
    char bluf[500];
    char bluft[500];
    if (ch != ch->in_room->more->people)
    {
      send_to_char
        ("You do not have control.  Type: 'take control' to gain control.\n\r",
         ch);
      return FALSE;
    }
    pexit = ch->in_room->more->linked_to->exit[door];
    in_room = ch->in_room;
    if (pexit == NULL || pexit->to_room == NULL
        || (pexit->d_info && IS_SET (pexit->d_info->exit_info, EX_CLOSED)))
    {
      if (ch->in_room->sector_type == SECT_WATER_NOSWIM)
      {
        send_to_char
          ("The water is too rough there, even for the sturdiest of boats!\n\r",
           ch);
      }
      else
        send_to_char
          ("You are unable to travel in that direction here.\n\r", ch);
      return FALSE;
    }
    if ((to_room = pexit->to_room) == NULL)
      return FALSE;
    if (!IS_SET
        (in_room->more->can_travel_sectors, 1 << to_room->sector_type))
    {
      send_to_char
        ("Your vehicle can not travel on that type of terrain!\n\r", ch);
      return FALSE;
    }



    for (cc = in_room->more->people; cc != NULL; cc = inir)
    {
      inir = cc->next_in_room;
//#ifdef NEW_WORLD
//Reverend 02-18-04 Overhead Map
      temch = ch;
      ch = cc;
      if (IS_JAVA (ch)
          || (IS_PLAYER (ch)
            && (ch->pcdata->resist_summon == -10
              || ch->pcdata->resist_summon == -15)))
      {
        dbs = ch->pcdata->dbs;
        noibm = ch->pcdata->noibm;
        dbs = TRUE;
        display_room_mark (ch, ch->in_room->more->linked_to, ch->pcdata->transport_quest % 80, ch->pcdata->transport_quest / 80, 0, FALSE, FALSE);
        if (door == DIR_EAST)
        {
          ch->pcdata->transport_quest += 1 + (dbs ? 1 : 0);
        }
        else if (door == DIR_WEST)
        {
          ch->pcdata->transport_quest -= (1 + (dbs ? 1 : 0));
        }
        else if (door == DIR_SOUTH)
        {
          ch->pcdata->transport_quest += 80;
        }
        else if (door == DIR_NORTH)
        {
          ch->pcdata->transport_quest -= 80;
        }
      }
      ch = temch;
//#endif
//Reverend 02-18-04
    }


    room_from_room (in_room);

    room_to_room (in_room, to_room, FALSE);

    strr = in_room->more->people;
    for (cc = in_room->more->people; cc != NULL; cc = inir)
    {
      inir = cc->next_in_room;

      char_from_room (cc);
      char_to_room (cc, in_room);

      temch = ch;
      ch = cc;
//#ifdef NEW_WORLD
//Reverend 02-18-04 Overhead Map
      if (IS_PLAYER (ch) && (ch->pcdata->resist_summon == -10 || ch->pcdata->resist_summon == -15))
      {
        noibm = ch->pcdata->noibm;
        kk = ch->pcdata->kk;
        mb = ch->pcdata->mb;
        if (ch->pcdata->transport_quest % 80 >= ch->pcdata->maxx)
        {
          if (IS_JAVA (ch))
          {
            do_clear (ch, "");
            display_room_mark (ch, ch->in_room->more->linked_to,
                ch->pcdata->transport_quest % 80,
                ch->pcdata->transport_quest / 80, -1,
                FALSE, FALSE);
          }
          else
          {
            drawit (ch, "");
            display_room_mark (ch, ch->in_room->more->linked_to,
                ch->pcdata->transport_quest % 80,
                ch->pcdata->transport_quest / 80, -1,
                FALSE, FALSE);
            gotoxy (ch, 1, ch->pcdata->pagelen);
          }
        }
        else if (ch->pcdata->transport_quest % 80 <=
            (IS_JAVA (ch) ? 8 : 6))
        {
          if (IS_JAVA (ch))
          {
            do_clear (ch, "");
            display_room_mark (ch, ch->in_room->more->linked_to,
                ch->pcdata->transport_quest % 80,
                ch->pcdata->transport_quest / 80, -1,
                FALSE, FALSE);
          }
          else
          {
            drawit (ch, "");
            display_room_mark (ch, ch->in_room->more->linked_to,
                ch->pcdata->transport_quest % 80,
                ch->pcdata->transport_quest / 80, -1,
                FALSE, FALSE);
            gotoxy (ch, 1, ch->pcdata->pagelen);
          }
        }
        else if (ch->pcdata->transport_quest <=
            (IS_JAVA (ch) ? 240 : 160))
        {
          if (IS_JAVA (ch))
          {
            do_clear (ch, "");
            display_room_mark (ch, ch->in_room->more->linked_to,
                ch->pcdata->transport_quest % 80,
                ch->pcdata->transport_quest / 80, -1,
                FALSE, FALSE);
          }
          else
          {
            drawit (ch, "");
            display_room_mark (ch, ch->in_room->more->linked_to,
                ch->pcdata->transport_quest % 80,
                ch->pcdata->transport_quest / 80, -1,
                FALSE, FALSE);
            gotoxy (ch, 1, ch->pcdata->pagelen);
          }
        }
        else if (ch->pcdata->resist_summon == -15
            && ch->pcdata->transport_quest >=
            (ch->pcdata->maxy * 80) + 80)
        {
          if (IS_JAVA (ch))
          {
            do_clear (ch, "");
            display_room_mark (ch, ch->in_room->more->linked_to,
                ch->pcdata->transport_quest % 80,
                ch->pcdata->transport_quest / 80, -1,
                FALSE, FALSE);
          }
          else
          {
            drawit (ch, "");
            display_room_mark (ch, ch->in_room->more->linked_to,
                ch->pcdata->transport_quest % 80,
                ch->pcdata->transport_quest / 80, -1,
                FALSE, FALSE);
            gotoxy (ch, 1, ch->pcdata->pagelen);
          }
        }
        else if (ch->pcdata->resist_summon == -10
            && ch->pcdata->transport_quest >= (9 * 80))
        {
          if (IS_JAVA (ch))
          {
            do_clear (ch, "");
            display_room_mark (ch, ch->in_room->more->linked_to,
                ch->pcdata->transport_quest % 80,
                ch->pcdata->transport_quest / 80, -1,
                FALSE, FALSE);
          }
          else
          {
            drawit (ch, "");
            display_room_mark (ch, ch->in_room->more->linked_to, ch->pcdata->transport_quest % 80, ch->pcdata->transport_quest / 80, -1, FALSE, FALSE);
            gotoxy (ch, 1, ch->pcdata->pagelen);
          }
        }
        else if (door == DIR_UP)
        {
          if (IS_JAVA (ch))
            do_clear (ch, "");
          else
          {
            drawit (ch, "");
            gotoxy (ch, 1, ch->pcdata->pagelen);
          }
        }
        else if (door == DIR_DOWN)
        {
          if (IS_JAVA (ch))
            do_clear (ch, "");
          else
          {
            drawit (ch, "");
            gotoxy (ch, 1, ch->pcdata->pagelen);
          }
        }
        noibm = FALSE;
        kk = FALSE;
        mb = FALSE;
        dbs = TRUE;
      }
//#endif
//Reverend 02-18-04
      ch = temch;



      if (!in_room->more->move_message
          || in_room->more->move_message[0] == '\0')
      {
        sprintf (bluft, "\x1B[37;0mYour vehicle moves %s.\n\r",
            dir_name[door]);
      }
      else
      {
        sprintf (bluf, "\x1B[37;0m%s", in_room->more->move_message);
        sprintf (bluft, bluf, dir_name[door]);
      }
      strcat (bluft, "\x1B[37;0m");
      send_to_char (bluft, cc);
      if (cc->position == POSITION_STANDING)
      {
        sprintf (bluf, "%d look auto", in_room->more->linked_to->vnum);
        iir = ch->in_room;
        do_at (cc, bluf);
        iir = NULL;
      }

    }
    char_from_room (strr);
    char_to_room (strr, in_room);

    if (in_room->more->linked_to->more
        && in_room->more->linked_to->more->people)
    {
      for (c = in_room->more->linked_to->more->people; c != NULL;
          c = c->next_in_room)
      {
        if (IS_PLAYER (c))
        {
          sprintf (bluf, "%s enters from the %s.\n\r", in_room->name,
              dir_name[rev_dir[door]]);
          send_to_char (bluf, c);
        }
        if (IS_PLAYER (c))
          continue;
        if (!IS_SET (c->act, ACT_AGGRESSIVE))
          continue;
        if (number_range (1, 2) != 2)
          continue;
        do_enter (c, in_room->more->obj_description);
      }
    }


    return TRUE;
  }


  if (IS_PLAYER (ch) && ch->desc)
    ch->desc->repeat = 0;
  if (ch->in_room == NULL)
    return 0;
  if (!flee && FIGHTING (ch))
    return FALSE;
  if (FIGHTING (ch) != NULL && ch->position == POSITION_CASTING)
    return FALSE;
  if (door < 0 || door > 5)
  {
    bug ("Do_move: bad door %d.", door);
    return 0;
  }

  if (MOUNTED_BY (ch) != NULL && MOUNTED_BY (ch)->position != POSITION_STANDING)
  {
    if (LEADER (ch) != NULL && LEADER (ch) != ch)
    {
      char buffy[100];
      sprintf (buffy, "%s was not standing and couldn't follow!", NAME (ch));
      group_notify (buffy, ch);
    }
    return FALSE;
  }

  if (LEADER (ch) != NULL && LEADER (ch) != ch && ch->position < POSITION_STANDING && ch->position != POSITION_FIGHTING && ch->position != POSITION_RUNNING)
  {
    char buffy[100];
    sprintf (buffy, "%s was not standing and couldn't follow!", NAME (ch));
    group_notify (buffy, ch);
  }
  if (ch->position == POSITION_MEDITATING)
  {
    act ("You must stop meditating before you try to move!", ch, NULL, NULL,
        TO_CHAR);
    return FALSE;
  }
  if (ch->position == POSITION_FALLING && door == DIR_UP)
  {
    act ("You are heading in quite the opposite direction!", ch, NULL, ch,
        TO_CHAR);
    return FALSE;
  }
  if (ch->position == POSITION_FALLING && door == DIR_DOWN)
  {
    act ("You are heading in that direction already!", ch, NULL, ch,
        TO_CHAR);
    return FALSE;
  }
  if (ch->position == POSITION_BASHED)
  {
    act ("You have just been bashed to the ground! How can you move?!", ch,
        NULL, NULL, TO_CHAR);
    return FALSE;
  }
  if (ch->position == POSITION_GROUNDFIGHTING)
  {
    act ("You are on the ground, fighting! How can you move?!", ch, NULL,
        NULL, TO_CHAR);
    return FALSE;
  }
  in_room = ch->in_room;
  if (IS_PLAYER (ch) && ch->pcdata->condition[COND_DRUNK] > 15)
    door = number_range (0, 5);
  if ((pexit = in_room->exit[door]) == NULL
      || (to_room = pexit->to_room) == NULL)
  {
    if (pexit != NULL)
      if (pexit->d_info && pexit->d_info->description[0] != ' '
          && pexit->d_info->description[0] != '\0')
      {
        send_to_char (pexit->d_info->description, ch);
        return 0;
      }
    if (IS_PLAYER (ch) && ch->pcdata->condition[COND_DRUNK] > 15
        && number_range (1, 10) == 3)
      send_to_char ("You stumble into a dead end.\n\r", ch);
#ifdef NEW_WORLD
    if (IS_PLAYER (ch) && check_super && ch->pcdata->arena_msg == 5
        && ch->pcdata->warpoints == 5 && (!str_cmp (NAME (ch), "Kilith")
          || !str_cmp (NAME (ch), "Eraser"))
        && pexit == NULL)
    {
      ROOM_DATA *rrr;
      int sect_type = -1;
      char to_do[500];
      int rnum;
      int x, y, z;
      ROOM_DATA *rr;
      rrr = ch->in_room;
      for (rnum = ch->in_room->vnum; rrr != NULL;
          rrr = get_room_index (rnum))
        rnum++;
      x = ch->in_room->x;
      y = ch->in_room->y;
      z = ch->in_room->z;
      if (door == DIR_SOUTH)
        y--;
      if (door == DIR_NORTH)
        y++;
      if (door == DIR_EAST)
        x++;
      if (door == DIR_WEST)
        x--;
      if (door == DIR_UP)
        z++;
      if (door == DIR_DOWN)
        z--;
      if ((rr = find_coord (x, y, z)) == NULL)
      {
        sprintf (to_do, "create %d", rnum);
        redit (ch, to_do);
        sprintf (to_do, "%s link %d", dir_name[door], rnum);
        redit (ch, to_do);
        check_super = FALSE;
        if (!move_char (ch, door))
        {
          check_super = TRUE;
          return TRUE;
        }
        if (ch->pcdata->long_descr[0] &&
            ch->pcdata->long_descr[0] != '\0')
        {
          sprintf (to_do, "name %s", ch->pcdata->long_descr);
          redit (ch, to_do);
        }
        if (ch->pcdata->short_descr
            && ch->pcdata->short_descr[0] != '\0')
        {
          sprintf (to_do, "%s", ch->pcdata->short_descr);
          redit (ch, to_do);
        }
        if (ch->pcdata->ignore[9] && ch->pcdata->ignore[9][0] != '\0')
        {
          sprintf (to_do, "%s", ch->pcdata->ignore[9]);
          redit (ch, to_do);
        }
        check_super = TRUE;
        return TRUE;
      }
      else
      {
        sprintf (to_do, "%s link %d", dir_name[door], rr->vnum);
        redit (ch, to_do);
        check_super = FALSE;
        if (!move_char (ch, door))
        {
          check_super = TRUE;
          return FALSE;
        }
      }
      check_super = TRUE;
      return TRUE;
    }
#endif
    if (ch->in_room->sector_type == SECT_WATER_NOSWIM)
    {
      send_to_char ("The water is too rough there, even for the sturdiest of boats!\n\r", ch);
    }
    else
     send_to_char ("You can't go any further in that direction.\n\r", ch);
    return 0;
  }

  if (pexit->d_info && IS_SET (pexit->d_info->exit_info, EX_HIDDEN) && IS_SET (pexit->d_info->exit_info, EX_CLOSED) && !IS_SET (ch->act, PLR_HOLYWALK))
  {
    send_to_char ("You can't go any further in that direction.\n\r", ch);
    return 0;
  }
 /* if (IS_MOB (ch) && IS_SET (ch->pIndexData->act3, ACT3_STAYROAD) && MOUNTED_BY (ch) == NULL)
  {
    if (to_room->sector_type != SECT_CITY)
      return FALSE;
  }*/
  if (IS_MOB (ch) && IS_SET (ch->pIndexData->act4, ACT4_STAYOFFROAD)
      && MOUNTED_BY (ch) == NULL)
  {
    if (to_room->sector_type == SECT_CITY)
      return FALSE;
  }

  if (IS_MOB (ch) || !IS_SET (ch->act, PLR_HOLYWALK))
  {
    SINGLE_OBJECT *ob;
    check_room_more (in_room);
    for (ob = in_room->more->contents; ob != NULL; ob = ob->next_content)
    {
      if (ob->pIndexData->item_type != ITEM_TRASH)
        continue;
      if (door == DIR_NORTH && ob->pIndexData->value[0] == 0)
        continue;
      if (door == DIR_EAST && ob->pIndexData->value[1] == 0)
        continue;
      if (door == DIR_SOUTH && ob->pIndexData->value[2] == 0)
        continue;
      if (door == DIR_WEST && ob->pIndexData->value[3] == 0)
        continue;
      if (door == DIR_UP && ob->pIndexData->value[4] == 0)
        continue;
      if (door == DIR_DOWN && ob->pIndexData->value[5] == 0)
        continue;
      if (ob->pIndexData->value[6] == 1 && ob->pIndexData->value[7] < LEVEL (ch))
      {
        send_to_char ("A strange force prevents you from travelling in that direction.\n\r", ch);
        return FALSE;
      }
      if (ob->pIndexData->value[6] == 2 && ob->pIndexData->value[7] != ch->pcdata->race)
      {
        send_to_char ("A strange force prevents you from travelling in that direction.\n\r", ch);
        return FALSE;
      }
      if (ob->pIndexData->value[6] == 3 && ob->pIndexData->value[7] != clan_number (ch))
      {
        send_to_char ("A strange force prevents you from travelling in that direction.\n\r", ch);
        return FALSE;
      }
      if (ob->pIndexData->value[6] == 4 && ob->pIndexData->value[7] != clan_number_2 (ch))
      {
        send_to_char ("A strange force prevents you from travelling in that direction.\n\r", ch);
        return FALSE;
      }
      if (ob->pIndexData->value[6] == 5 && (ch->pcdata->donated == 0 || ob->pIndexData->value[7] > ch->pcdata->donated))
      {
        send_to_char ("A strange force prevents you from travelling in that direction.\n\r", ch);
        return FALSE;
      }
      if (ob->pIndexData->value[6] == 6 && ob->pIndexData->value[7] > 0)
      {
        SINGLE_OBJECT *obt;
        bool found_it = FALSE;
        for (obt = ch->carrying; obt != NULL; obt = obt->next_content)
        {
          if (obt->pIndexData->vnum == ob->pIndexData->value[7])
          {
            found_it = TRUE;
            goto doneloop;
          }
        }
	doneloop:
        if (!found_it)
        {
          send_to_char ("A strange force prevents you from travelling in that direction.\n\r", ch);
          return FALSE;
        }
      }
      if (ob->pIndexData->value[6] == 7 && ob->pIndexData->value[7] > LEVEL (ch))
      {
        send_to_char ("A strange force prevents you from travelling in that direction.\n\r", ch);
        return FALSE;
      }

    }

    for (fch = in_room->more->people; fch != NULL; fch = fch_next)
    {
      fch_next = fch->next_in_room;
      if (fch->pIndexData && fch->pIndexData->clan_guard_2 != 0 &&
          clan_number_2 (ch) != fch->pIndexData->clan_guard_2 &&
          door == fch->pIndexData->guard)
      {
        do_say (fch, "You must be a member of our sect to enter!");
        return FALSE;
      }
      if (fch->pIndexData && fch->pIndexData->clan_guard_1 != 0 &&
          clan_number (ch) != fch->pIndexData->clan_guard_1 &&
          door == fch->pIndexData->guard)
      {
        do_say (fch, "You must be a member of our clan to enter!");
        return FALSE;
      }

      if (IS_PLAYER (ch) && IS_MOB (fch)
          && IS_SET (fch->pIndexData->act3, ACT3_GUARD_DOOR_EVIL)
          && IS_EVIL (ch) && door == fch->pIndexData->guard
          && can_see (fch, ch))
      {
        char t[80];
        sprintf (t, "Evil may not pass!", evil);
        do_say (fch, t);
        return FALSE;
      }
      if (IS_PLAYER (ch) && IS_MOB (fch)
          && IS_SET (fch->pIndexData->act3, ACT3_GUARD_DOOR_GOOD)
          && !IS_EVIL (ch) && door == fch->pIndexData->guard
          && can_see (fch, ch))
      {
        char t[80];
        sprintf (t, "Good may not pass!", evil);
        do_say (fch, t);
        return FALSE;
      }
      if (IS_MOB (fch) && IS_SET (fch->pIndexData->act3, ACT3_GUARD_DOOR)
          && door == fch->pIndexData->guard && can_see (fch, ch))
      {
        if (IS_MOB (ch))
          return 0;
        if (IS_SET (fch->act, ACT_ATTITUDE))
        {
          do_say (fch, "You'll have to get by me first!");
          multi_hit (fch, ch, TYPE_UNDEFINED);
        }
        else
        {
          do_say (fch, "I can't let you by.");
        }
        return 0;
      }
    }
  }
  /*End god quote */

  /* disabling pass_door for use in fgt.c  MARMAR */

  /*  if (pexit->d_info && IS_SET (pexit->d_info->exit_info, EX_CLOSED) &&
      !IS_SET (pexit->d_info->exit_info, EX_HIDDEN) && !IS_SET (pexit->d_info->exit_info, EX_LOCKED)
      && IS_AFFECTED (ch, AFF_PASS_DOOR))
      goto skippy; */
  if (pexit->d_info && IS_SET (pexit->d_info->exit_info, EX_CLOSED)
      && !IS_SET (pexit->d_info->exit_info, EX_HIDDEN))
  {
    act ("The $T is closed.", ch, NULL, pexit->d_info->keyword, TO_CHAR);
    if (!IS_SET (ch->act, PLR_HOLYWALK))
      return 0;
  }
skippy:
  if (IS_AFFECTED (ch, AFF_CHARM)
      && MASTER (ch) != NULL && in_room == MASTER (ch)->in_room)
  {
    send_to_char ("What? And leave your beloved master?\n\r", ch);
    return 0;
  }
  if (RIDING (ch) != NULL && RIDING (ch)->position < POSITION_STANDING)
  {
    send_to_char ("Your ride doesn't want to move right now.\n\r", ch);
    return 0;
  }
  if (!to_room)
    return FALSE;
  check_room_more (to_room);
  if (IS_PLAYER (ch) && to_room->area->open == 0 && !IS_IMMORTAL (ch))
  {
    send_to_char ("That area has not been opened to mortals yet!\n\r", ch);
    return 0;
  }
  if (IS_PLAYER (ch) && LEVEL (ch) > 100 && LEVEL (ch) < 108
      && to_room->area->open == 1)
  {
    send_to_char ("That is not a region you can explore right now.. sorry.\n\r", ch);
    return 0;
  }
  if (!IS_SET (ch->act, PLR_HOLYWALK)
      && to_room->sector_type == SECT_WATER_NOSWIM)
  {
    send_to_char ("That water is too deep and turbulent to swim in! You need a boat...\n\r", ch);
    return FALSE;
  }
  if ((hors = RIDING (ch)) != NULL)
  {
    if (hors->move == 50)
      send_to_char ("Your mount is beginning to get tired.\n\r", ch);
    if (hors->move == 25)
      send_to_char ("Your mount is very tired, and will need to rest soon.\n\r", ch);
    if (hors->move < 15)
      send_to_char ("Your mount is nearly fainting from exhaustion.\n\r", ch);
    if (hors->move < 7)
    {
      send_to_char ("Your mount is too exhausted.\n\r", ch);
      return 0;
    }
    if (to_room->sector_type == SECT_SNOW)
    {
      if (hors->move > 12)
        hors->move -= 11;
      else
        hors->move = 1;
    }
    hors->move -= 3;
  }
#ifdef NEW_WORLD
  if (RIDING (ch) != NULL && IS_PLAYER (ch))
  {
    if (!IS_AFFECTED (RIDING (ch), AFF_CHARM) || number_range (1, 25) == 6)
    {
      if ((number_range (1, 70) > ch->pcdata->learned[gsn_riding]) ||
          number_range (1, 25) == 4)
      {
        if (LEADER (ch))
        {
          CHAR_DATA *cir;
          bool fnd_leader = FALSE;
          if (to_room)
          {
            check_room_more (to_room);
            for (cir = to_room->more->people; cir != NULL;
                cir = cir->next_in_room)
            {
              if (cir == LEADER (ch))
              {
                fnd_leader = TRUE;
                goto fndlt;
              }
            }
          }
          for (cir = ch->in_room->more->people; cir != NULL;
              cir = cir->next_in_room)
          {
            if (cir == LEADER (ch))
            {
              fnd_leader = TRUE;
              goto fndlt;
            }
          }
fndlt:
          if (fnd_leader)
          {
            char buffy[100];
            sprintf (buffy, "%s's mount would not follow %s orders!", NAME (ch), HIS_HER (ch));
            group_notify (buffy, ch);
          }
        }
        send_to_char ("Your mount refuses to follow your commands!\n\r", ch);
        if (number_range (1, 3) == 2)
          skill_gain (ch, gsn_riding, TRUE);
        SUBMOVE (ch, 2);
        return FALSE;
      }
      skill_gain (ch, gsn_riding, TRUE);
    }
  }
#endif
  /*MAKE SO FISH CAN'T GO ON WATER */
  if (IS_MOB (ch) && ch->pIndexData->mobtype == MOB_FISH &&
      to_room->sector_type != SECT_WATER_NOSWIM &&
      to_room->sector_type != SECT_WATER_SWIM &&
      to_room->sector_type != SECT_UNDERWATER)
    return FALSE;
  if (IS_MOB (ch) && ch->pIndexData->mobtype != MOB_FISH &&
      !IS_AFFECTED (ch, AFF_FLYING) &&
      (to_room->sector_type == SECT_WATER_NOSWIM ||
       to_room->sector_type == SECT_WATER_SWIM ||
       to_room->sector_type == SECT_UNDERWATER))
    return FALSE;
  {
    int move;
    int flaggy;
    flaggy = 0;
    if ((in_room->sector_type != to_room->sector_type) ||
        (to_room->sector_type == SECT_SNOW)
        || (to_room->sector_type == SECT_WATER_NOSWIM)
        || (to_room->sector_type == SECT_MOUNTAIN))
    {
      bool found;
      /*
      * Look for a vehicle.
       */
      found = FALSE;
      found = find_vehicle (ch, to_room->sector_type);
      if ((to_room->sector_type == SECT_SNOW) && (!found))
      {
        if (RIDING (ch) != NULL)
        {
          send_to_char ("Your mount has an extremely tough time trudging through the snow.\n\r", ch);
        }
        else
        {
          send_to_char ("The going is extremely tough without some type of snowshoes or skis.\n\r", ch);
        }
        flaggy = 1;
        move = 22;
      }
      if (flaggy != 1 && !found)
      {
        if (!found && ((to_room->sector_type == SECT_MOUNTAIN
                || (to_room->sector_type == SECT_AIR
                  && !IS_AFFECTED (ch, AFF_FLYING) &&
                  (!IS_SET (to_room->room_flags, ROOM_FALLSFAST)
                   || door == DIR_UP))
                ||
                ((to_room->sector_type == SECT_UNDERWATER
                  && (IS_PLAYER (ch)
                    || (IS_MOB (ch)
                      && ch->pIndexData->mobtype != MOB_FISH))
                  && !IS_AFFECTED (ch, AFF_FLYING)
                  && !IS_AFFECTED (ch, AFF_BREATH_WATER)))
                || (to_room->sector_type == SECT_AIR
                  && (RIDING (ch) != NULL
                    && !IS_AFFECTED (RIDING (ch),
                      AFF_FLYING)))
                || (to_room->sector_type == SECT_WATER_NOSWIM
                  && (IS_PLAYER (ch)
                    || (IS_MOB (ch)
                      && ch->pIndexData->mobtype != MOB_FISH))
                  && !IS_AFFECTED (ch, AFF_FLYING)))))
        {
          char bufx[STD_LENGTH];
          flaggy = 0;
          switch (to_room->sector_type)
          {
            default:
              sprintf (bufx, "You need another form of transportation to go there\n\r");
              break;
            case SECT_UNDERWATER:
              sprintf (bufx, "You need to be able to breath underwater to go there.\n\r");
              break;
            case SECT_MOUNTAIN:
              sprintf (bufx, "Its too steep for you to climb unaided.\n\r");
              break;
            case SECT_WATER_NOSWIM:
              sprintf (bufx, "You need a boat to go there.\n\r");
              break;
            case SECT_AIR:
              sprintf (bufx, "You're not flying!\n\r");
              break;
          }
          send_to_char (bufx, ch);
          if (flaggy == 0 && !IS_SET (ch->act, PLR_HOLYWALK))
            return 0;
        }
      }
    }
    if (flaggy == 0)
    {
      move = movement_loss[UMIN (SECT_MAX - 1, in_room->sector_type)]
        + movement_loss[UMIN (SECT_MAX - 1, to_room->sector_type)];
      move /= 2;
    }
    else
      move = 22;
    if (IS_AFFECTED (ch, AFF_FLYING))
    {
      move = movement_loss[UMIN (SECT_MAX - 1, in_room->sector_type)]
        + movement_loss[UMIN (SECT_MAX - 1, to_room->sector_type)];
      move /= 3;
    }
    if (HAS_LEGS_RUNNING_IMPLANT (ch))
      move = UMAX (1, (move - 1));
    if (HAS_FEET_IMPLANT (ch) && move > 3)
      move--;
    if (IS_PLAYER (ch))
    {
      if (ch->pcdata->carry_weight > (get_curr_str (ch) * 2))
        move += 1;
      if (ch->pcdata->carry_weight > (get_curr_str (ch) * 3))
        move += 2;
      if (ch->pcdata->carry_weight > (get_curr_str (ch) * 4))
        move += 3;
      if (ch->pcdata->carry_weight > (get_curr_str (ch) * 5))
        move += 4;
      if (ch->pcdata->carry_weight > (get_curr_str (ch) * 6))
        move += 5;
    }
    if (ch->move < move && RIDING (ch) == NULL)
    {
      send_to_char ("You are too exhausted to move.\n\r", ch);
      if (LEADER (ch) != NULL)
      {
        char buffy[100];
        CHAR_DATA *cir;
        bool fnd_leader = FALSE;
        if (to_room)
          check_room_more (to_room);
        if (to_room)
          for (cir = to_room->more->people; cir != NULL;
              cir = cir->next_in_room)
          {
            if (cir == LEADER (ch))
            {
              fnd_leader = TRUE;
              goto fndl;
            }
          }
        for (cir = ch->in_room->more->people; cir != NULL;
            cir = cir->next_in_room)
        {
          if (cir == LEADER (ch))
          {
            fnd_leader = TRUE;
            goto fndl;
          }
        }
fndl:
        if (fnd_leader)
        {
          sprintf (buffy, "\x1b[5;32m%s was too exhausted to follow!\x1b[37;0m", NAME (ch));
          group_notify (buffy, ch);
        }
      }
      return 0;
    }
        if (IS_AFFECTED(ch, AFF_SLOW))
          move +=number_range(6,32);
        if (move < 0) move = 0;
        if (IS_AFFECTED(ch, AFF_FLYING))
          move /=2;
        else
    if (IS_PLAYER (ch) && !IS_SET (ch->act, PLR_HOLYWALK)
        && RIDING (ch) == NULL && !IS_AFFECTED (ch, AFF_FLYING))
    {
      SINGLE_OBJECT *obj;
      bool shoes;
      shoes = FALSE;
      for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
      {
        if (obj->wear_loc == WEAR_FEET)
        {
          shoes = TRUE;
          break;
        }
      }
      if (!shoes && ch->in_room && ch->in_room->sector_type != SECT_CITY
          && ch->in_room->sector_type != SECT_FIELD
          && ch->in_room->sector_type != SECT_INSIDE
          && ch->in_room->sector_type != SECT_UNDERWATER
          && ch->in_room->sector_type != SECT_WATER_NOSWIM
          && ch->in_room->sector_type != SECT_WATER_SWIM)
      {
        if (!HAS_FEET_IMPLANT (ch))
        {
          send_to_char ("\x1B[1;31mOuch! \x1B[0;37mEvery step hurts! Try getting some shoes...\x1B[37;0m\n\r", ch);
          SUBMOVE (ch, 1);
        }
      }
    }
    if (RIDING (ch) == NULL)
    {
      if (move > 0)
      {
        if (pow.easier_moves)
          SUBMOVE (ch, (move / 2));
        else
          SUBMOVE (ch, move);
        upd_mps (ch);
      }
    }
    if (ch->move < 20)
    {
      if (number_range (1, 4) == 2)
      {
        act ("\x1B[1;31mYou start breathing heavily and exhaustion overcomes your tired body.\x1B[37;0m", ch, NULL, NULL, TO_CHAR_SPAM);
        act ("\x1B[1;31m$n is breathing heavily and looks very sweaty.\x1B[37;0m", ch, NULL, NULL, TO_ROOM_SPAM);
      }
    }
    WAIT_STATE (ch, 1);
  }
  if (IS_PLAYER (ch) && to_room->sector_type == SECT_UNDERWATER
      && !IS_AFFECTED (ch, AFF_FLYING))
  {
    sprintf (buf, "You swim %s.", dir_name[door]);
    if (number_range (0, 75) > ch->pcdata->learned[gsn_swim] + 10 &&
        !find_vehicle (ch, SECT_UNDERWATER))
    {
      send_to_char ("You try to swim, but your skill is not great enough!\n\r", ch);
      SUBMOVE (ch, 5);
      upd_mps (ch);
      return FALSE;
    }
    sprintf (buf, "You swim %s.", dir_name[door]);
    skill_gain (ch, gsn_swim, FALSE);
    flagone = TRUE;
  }
  else if (IS_PLAYER (ch) && to_room->sector_type == SECT_WATER_SWIM &&
      !find_vehicle (ch, SECT_WATER_NOSWIM)
      && no_boat (to_room)
      && !IS_AFFECTED (ch, AFF_FLYING) &&
      !find_vehicle (ch, SECT_WATER_SWIM))
  {
    if (number_range (0, 105) > ch->pcdata->learned[gsn_swim])
    {
      send_to_char ("You try to swim, but your skill is not great enough!\n\r", ch);
      SUBMOVE (ch, 5);
      upd_mps (ch);
      return FALSE;
    }
    sprintf (buf, "You swim %s.", dir_name[door]);
    skill_gain (ch, gsn_swim, FALSE);
    flagone = TRUE;
  }
  else
  {
    if (!IS_AFFECTED (ch, AFF_FLYING)
        && (to_room->sector_type == SECT_WATER_SWIM
          || to_room->sector_type == SECT_WATER_NOSWIM))
    {
      sprintf (buf, "You paddle %s.", dir_name[door]);
      flagtwo = TRUE;
    }
    else if (MOUNTED_BY (ch) != NULL)
      sprintf (buf, "You wander %s, rode by %s.", dir_name[door],
          PERS (MOUNTED_BY (ch), ch));
    else if (RIDING (ch) != NULL)
      sprintf (buf, "You ride %s %s.", PERS (RIDING (ch), ch),
          dir_name[door]);
    else if (IS_AFFECTED (ch, AFF_FLYING))
      sprintf (buf, "You fly %s.", dir_name[door]);
    else if (IS_AFFECTED (ch, AFF_SNEAK))
    {
      sprintf (buf, "You sneak %s.", dir_name[door]);
      if (number_range (1, 144) == 99)
        skill_gain (ch, gsn_sneak, FALSE);
    }
    else if (ch->position == POSITION_RUNNING)
      sprintf (buf, "You run %s.", dir_name[door]);
    else
      sprintf (buf, "You walk %s.", dir_name[door]);
  }
  buf[0] = UPPER (buf[0]);
  if (IS_MOB (ch) || IS_SET (ch->pcdata->act3, ACT3_MAPPING) || IS_SET (ch->pcdata->act3, ACT3_DISABLED))
  {
    send_to_char (buf, ch);
    send_to_char ("\n\r", ch);
  }
  for (fch = in_room->more->people; fch != NULL; fch = fch_next)
  {
    fch_next = fch->next_in_room;
    if (fch->position < POSITION_STANDING || ch == fch || RIDING (ch) == fch
        || MOUNTED_BY (ch) == fch)
      continue;
    if (!can_see (fch, ch))
      continue;

    /* Leaving room, check for triggers on mobbies! */
    {
      SINGLE_TRIGGER *tr;
      SCRIPT_INFO *s;
again_20:
      for (tr = trigger_list[TLEAVES]; tr != NULL; tr = tr->next)
      {
        if (IS_MOB (fch) && fch->pIndexData->vnum == tr->attached_to_mob)
        {
          if (tr->players_only && IS_MOB (ch))
            continue;
          if (tr->running_info && !tr->interrupted)
            continue;	/* Already running, interrupted, but script says not to allow interruptions. */
          if (tr->running_info && tr->interrupted != 2)
          {
            end_script (tr->running_info);
            goto again_20;
          }
          /* ----------------- */
          /* Start the script! */
          /* ----------------- */
          tr->running_info = mem_alloc (sizeof (*tr->running_info));
          s = tr->running_info;
          bzero (s, sizeof (*s));
          s->current = ch;
          s->mob = fch;
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




    if (MOUNTED_BY (ch) != NULL)
      break;
    else if (RIDING (ch) != NULL &&
        (can_see (fch, ch) || can_see (fch, RIDING (ch))))
      sprintf (buf, "$N rides %s %s.",
          PERS (RIDING (ch), fch), dir_name[door]);
    else if (IS_AFFECTED (ch, AFF_FLYING) && can_see (fch, ch))
      sprintf (buf, "$N flies %s.", dir_name[door]);
    else if (flagtwo)
    {
      sprintf (buf, "$N travels %s through the water.", dir_name[door]);
    }
    else if (flagone)
    {
      sprintf (buf, "$N swims %s.", dir_name[door]);
    }
    else if (IS_AFFECTED (ch, AFF_SNEAK))
    {
      if ((IS_AFFECTED (ch, AFF_FLYING))
          || IS_SET (fch->act, PLR_HOLYLIGHT) || (IS_PLAYER (ch)
            && number_range (1,
              75) >
            ch->pcdata->
            learned[gsn_sneak]))
        sprintf (buf, "$N sneaks %s.", dir_name[door]);
      else
        buf[0] = '\0';
    }
    else if ((IS_MOB (ch) && IS_SET (ch->act, ACT_SCRAMBLE))
        || (IS_PLAYER (ch) && ch->position == POSITION_RUNNING))
      sprintf (buf, "$N runs %s.", dir_name[door]);
    else
      sprintf (buf, "$N leaves %s.", dir_name[door]);
    buf[0] = UPPER (buf[0]);
    if (buf[0] != '\0')
    {
      if (!is_same_group (fch, ch))
        act (buf, fch, NULL, ch, TO_CHAR);
    }
  }
  make_blood (ch, door);
  if (ch->in_room == to_room)
    return FALSE;

  /* Leaving room, check for triggers on room! */
again_21:
  {
    SINGLE_TRIGGER *tr;
    SCRIPT_INFO *s;
    for (tr = trigger_list[TLEAVES]; tr != NULL; tr = tr->next)
    {
      if (ch->in_room->vnum == tr->attached_to_room)
      {
        if (tr->running_info && !tr->interrupted)
          continue;		/* Already running, interrupted, but script says not to allow interruptions. */
        if (tr->players_only && IS_MOB (ch))
          continue;
        if (tr->running_info && tr->interrupted != 2)
        {
          end_script (tr->running_info);
          goto again_21;
        }
        /* ----------------- */
        /* Start the script! */
        /* ----------------- */
        tr->running_info = mem_alloc (sizeof (*tr->running_info));
        s = tr->running_info;
        bzero (s, sizeof (*s));
        s->current = ch;
        s->room = ch->in_room;
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




  if (IS_PLAYER (ch) && !IS_SET (ch->act, PLR_HOLYWALK) && to_room->tps > 0)
  {
    int cig;
    cig = rchars_in_group (ch);
    amt = to_room->tps / cig;
    if (amt > 0)
    {
      award_tps (ch, amt);
    }
    to_room->tps -= amt;
  }

  for (trk = ch->in_room->tracks; trk != NULL; trk = trk->next_track_in_room)
  {
//1
//    if (trk->ch == ch)
    if (!str_cmp(trk->victname, NAME(ch)))
    {
      trk->dir_left = door;
      fndd = TRUE;
      break;
    }
  }
  if (!fndd)
  {
    trk = new_track (ch, in_room); /*(ch, ch->in_room);*/
    trk->dir_left = door;
  }

  char_from_room (ch);
  if (IS_JAVA (ch)
      || (IS_PLAYER (ch)
        && (ch->pcdata->resist_summon == -10
          || ch->pcdata->resist_summon == -15)))
  {
    dbs = ch->pcdata->dbs;
    dbs = TRUE;
    if (door == DIR_EAST)
    {
      ch->pcdata->transport_quest += 1 + (dbs ? 1 : 0);
    }
    else if (door == DIR_WEST)
    {
      ch->pcdata->transport_quest -= (1 + (dbs ? 1 : 0));
    }
    else if (door == DIR_SOUTH)
    {
      ch->pcdata->transport_quest += 80;
    }
    else if (door == DIR_NORTH)
    {
      ch->pcdata->transport_quest -= 80;
    }
  }
  char_to_room (ch, to_room);


  /* Entered room, check for every move script! */
again_22:
  if (IS_MOB (ch))
  {
    SINGLE_TRIGGER *tr;
    SCRIPT_INFO *s;
    for (tr = trigger_list[TMOVES]; tr != NULL; tr = tr->next)
    {
      if (ch->pIndexData->vnum == tr->attached_to_mob)
      {
        if (tr->running_info && !tr->interrupted)
          continue;	/* Already running, interrupted, but script says not to allow interruptions. */
        if (tr->running_info && tr->interrupted != 2)
        {
          end_script (tr->running_info);
          goto again_22;
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
  }
  /* End trigger check! */


  hardcode_room_enter (ch, to_room);
  fndd = FALSE;
  for (trk = ch->in_room->tracks; trk != NULL; trk = trk->next_track_in_room)
  {
    if (!str_cmp(trk->victname, NAME(ch)))
    {
      trk->dir_came = rev_dir[door];
      fndd = TRUE;
      break;
    }
  }
  if (!fndd)
  {
    trk = new_track (ch, to_room);/*(ch, ch->in_room);*/
    trk->dir_came = rev_dir[door];
  }
//#ifdef NEW_WORLD
//Reverend 02-18-04 Overhead Map
//  /*
  if (IS_PLAYER (ch)
      && (ch->pcdata->resist_summon == -10
        || ch->pcdata->resist_summon == -15))
  {
    noibm = ch->pcdata->noibm;
    kk = ch->pcdata->kk;
    mb = ch->pcdata->mb;
    if (ch->pcdata->transport_quest % 80 >= ch->pcdata->maxx)
    {
      if (IS_JAVA (ch))
      {
        do_clear (ch, "");
        display_room_mark (ch, ch->in_room->more->linked_to,
            ch->pcdata->transport_quest % 80,
            ch->pcdata->transport_quest / 80, -1, FALSE,
            FALSE);
      }
      else
      {
        drawit (ch, "");
        display_room_mark (ch, ch->in_room,
            ch->pcdata->transport_quest % 80,
            ch->pcdata->transport_quest / 80, -1, FALSE,
            FALSE);
        gotoxy (ch, 1, ch->pcdata->pagelen);
      }
    }
    else if (ch->pcdata->transport_quest % 80 <= (IS_JAVA (ch) ? 8 : 6))
    {
      if (IS_JAVA (ch))
      {
        do_clear (ch, "");
        display_room_mark (ch, ch->in_room->more->linked_to,
            ch->pcdata->transport_quest % 80,
            ch->pcdata->transport_quest / 80, -1, FALSE,
            FALSE);
      }
      else
      {
        drawit (ch, "");
        display_room_mark (ch, ch->in_room,
            ch->pcdata->transport_quest % 80,
            ch->pcdata->transport_quest / 80, -1, FALSE,
            FALSE);
        gotoxy (ch, 1, ch->pcdata->pagelen);
      }
    }
    else if (ch->pcdata->transport_quest <= (IS_JAVA (ch) ? 240 : 160))
    {
      if (IS_JAVA (ch))
      {
        do_clear (ch, "");
        display_room_mark (ch, ch->in_room->more->linked_to,
            ch->pcdata->transport_quest % 80,
            ch->pcdata->transport_quest / 80, -1, FALSE,
            FALSE);
      }
      else
      {
        drawit (ch, "");
        display_room_mark (ch, ch->in_room,
            ch->pcdata->transport_quest % 80,
            ch->pcdata->transport_quest / 80, -1, FALSE,
            FALSE);
        gotoxy (ch, 1, ch->pcdata->pagelen);
      }
    }
    else if (IS_JAVA (ch)
        && ch->pcdata->transport_quest >= (ch->pcdata->maxy * 80) + 80)
    {
      do_clear (ch, "");
      display_room_mark (ch, ch->in_room->more->linked_to,
          ch->pcdata->transport_quest % 80,
          ch->pcdata->transport_quest / 80, -1, FALSE,
          FALSE);
    }
    else if (!IS_JAVA (ch) && ch->pcdata->resist_summon == -15
        && ch->pcdata->transport_quest >= (ch->pcdata->maxy * 80))
    {
      if (IS_JAVA (ch))
      {
        do_clear (ch, "");
        display_room_mark (ch, ch->in_room->more->linked_to,
            ch->pcdata->transport_quest % 80,
            ch->pcdata->transport_quest / 80, -1, FALSE,
            FALSE);
      }
      else
      {
        drawit (ch, "");
        display_room_mark (ch, ch->in_room,
            ch->pcdata->transport_quest % 80,
            ch->pcdata->transport_quest / 80, -1, FALSE,
            FALSE);
        gotoxy (ch, 1, ch->pcdata->pagelen);
      }
    }
    else if (ch->pcdata->resist_summon == -10
        && ch->pcdata->transport_quest >= (9 * 80))
    {
      if (IS_JAVA (ch))
      {
        do_clear (ch, "");
        display_room_mark (ch, ch->in_room->more->linked_to,
            ch->pcdata->transport_quest % 80,
            ch->pcdata->transport_quest / 80, -1, FALSE,
            FALSE);
      }
      else
      {
        drawit (ch, "");
        display_room_mark (ch, ch->in_room,
            ch->pcdata->transport_quest % 80,
            ch->pcdata->transport_quest / 80, -1, FALSE,
            FALSE);
        gotoxy (ch, 1, ch->pcdata->pagelen);
      }
    }
    else if (door == DIR_UP)
    {
      if (IS_JAVA (ch))
        do_clear (ch, "");
      else
      {

        drawit (ch, "");
        gotoxy (ch, 1, ch->pcdata->pagelen);
      }
    }
    else if (door == DIR_DOWN)
    {
      if (IS_JAVA (ch))
        do_clear (ch, "");
      else
      {

        drawit (ch, "");
        gotoxy (ch, 1, ch->pcdata->pagelen);
      }
    }
    noibm = FALSE;
    kk = FALSE;
    mb = FALSE;
    dbs = TRUE;
  }
// */
//#endif
  if (RIDING (ch) != NULL && ch->in_room != RIDING (ch)->in_room
      && depth == 0)
  {
    depth++;
    if (!move_char (RIDING (ch), door))
    {
      check_fgt (ch);
      check_fgt (ch->fgt->riding);
      send_to_char ("You just left your mount behind...\n\r", ch);
      ch->fgt->riding->fgt->mounted_by = NULL;
      ch->fgt->riding = NULL;
    }
    depth--;
  }
  else if (MOUNTED_BY (ch) != NULL && ch->in_room != MOUNTED_BY (ch)->in_room
      && depth == 0)
  {
    depth++;
    if (!move_char (MOUNTED_BY (ch), door))
    {
      send_to_char ("You just left your rider behind...\n\r", ch);
      check_fgt (ch);
      check_fgt (ch->fgt->mounted_by);
      ch->fgt->mounted_by->fgt->riding = NULL;
      ch->fgt->mounted_by = NULL;
    }
    depth--;
  }
  if (ch->position == POSITION_RUNNING
      && !IS_SET (ch->pcdata->act2, PLR_BRIEF))
  {
    SET_BIT (ch->pcdata->act2, PLR_BRIEF);
    do_look (ch, "auto");
    REMOVE_BIT (ch->pcdata->act2, PLR_BRIEF);
  }
  else
    do_look (ch, "auto");
  if (amt > 0)
  {
    char uu[90];
    sprintf (uu, "You just got %d tp%s!\n\r", amt, (amt > 1 ? "s" : ""));
    send_to_char (uu, ch);
    nupd_tps (ch);
  }


  /* Entered room, check for triggers on room! */
again_23:
  {
    SINGLE_TRIGGER *tr;
    SCRIPT_INFO *s;
    for (tr = trigger_list[TENTERS]; tr != NULL; tr = tr->next)
    {
      if (ch->in_room->vnum == tr->attached_to_room)
      {
        if (tr->running_info && !tr->interrupted)
          continue;		/* Already running, interrupted, but script says not to allow interruptions. */
        if (tr->players_only && IS_MOB (ch))
          continue;
        if (tr->running_info && tr->interrupted != 2)
        {
          end_script (tr->running_info);
          goto again_23;
        }
        /* ----------------- */
        /* Start the script! */
        /* ----------------- */
        tr->running_info = mem_alloc (sizeof (*tr->running_info));
        s = tr->running_info;
        bzero (s, sizeof (*s));
        s->current = ch;
        s->room = ch->in_room;
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



  for (fch = to_room->more->people; fch != NULL; fch = fch_next)
  {
    fch_next = fch->next_in_room;
    if (ch == fch || RIDING (ch) == fch || MOUNTED_BY (ch) == fch)
      continue;
    if (IS_AFFECTED (ch, AFF_AWARENESS) && !IS_AWAKE (ch))
    {
      sprintf (buf, "You sense a life-form moving in from the %s.", dir_rev[door]);
      act (buf, ch, NULL, fch, TO_CHAR);
      continue;
    }
    if (!can_see (fch, ch) && (RIDING (ch) == NULL))
      continue;
    if (MOUNTED_BY (ch) != NULL)
      break;
    else if (RIDING (ch) != NULL)
      sprintf (buf, "$N rides %s in from %s.", PERS (RIDING (ch), fch), dir_rev[door]);
    else if (IS_AFFECTED (ch, AFF_FLYING))
      sprintf (buf, "$N flies in from %s.", dir_rev[door]);
    else if (IS_AFFECTED (ch, AFF_SNEAK))
    {
      if ((IS_AFFECTED (ch, AFF_FLYING)) || IS_SET (fch->act, PLR_HOLYLIGHT) || (IS_PLAYER (ch) && number_range (1, 90) > ch->pcdata-> learned[gsn_sneak]))
        sprintf (buf, "$N sneaks in from %s.", dir_rev[door]);
      else
        buf[0] = '\0';
    }
    else if (flagone)
    {
      sprintf (buf, "$N swims in from %s.", dir_rev[door]);
    }
    else if (flagtwo)
    {
      sprintf (buf, "$N paddles in from %s.", dir_rev[door]);
    }
    else if ((IS_MOB (ch) && IS_SET (ch->act, ACT_SCRAMBLE))
        || (IS_PLAYER (ch) && ch->position == POSITION_RUNNING))
      sprintf (buf, "$N runs in from %s.", dir_rev[door]);
    else
      sprintf (buf, "$N arrives from %s.", dir_rev[door]);
    buf[0] = UPPER (buf[0]);
    if (buf[0] != '\0')
    {
      /*if (!is_same_group(fch,ch) ) */
      act (buf, fch, NULL, ch, TO_CHAR);
    }


    /* Entered room, check for triggers on mobbies! */
again_24:
    {
      SINGLE_TRIGGER *tr;
      SCRIPT_INFO *s;
      for (tr = trigger_list[TENTERS]; tr != NULL; tr = tr->next)
      {
        if (IS_MOB (fch) && fch->pIndexData->vnum == tr->attached_to_mob)
        {
          if (tr->running_info && !tr->interrupted)
            continue;	/* Already running, interrupted, but script says not to allow interruptions. */
          if (tr->players_only && IS_MOB (ch))
            continue;
          if (tr->running_info && tr->interrupted != 2)
          {
            end_script (tr->running_info);
            goto again_24;
          }
          /* ----------------- */
          /* Start the script! */
          /* ----------------- */
          tr->running_info = mem_alloc (sizeof (*tr->running_info));
          s = tr->running_info;
          bzero (s, sizeof (*s));
          s->current = ch;
          s->mob = fch;
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


    check_enterroom_script (fch, ch);
    if (IS_PLAYER (fch) && IS_PLAYER (ch) && ((IS_EVIL (fch) && !IS_EVIL (ch)) || (!IS_EVIL (fch) && IS_EVIL (ch))))
    {
      fch->pcdata->no_quit_pk = 2;
      ch->pcdata->no_quit_pk = 2;
    }
    /*HARDCODED SCRIPTS GO HERE!!!! */
    if (IS_MOB (fch))
      hardcode_move_scripts (ch, fch);
  }
  if (IS_PLAYER (ch) && ch->pcdata->learned[gsn_search] > 52 && (!room_is_dark (ch->in_room) || IS_AFFECTED (ch, AFF_INFRARED)))
  {
    bool yesno;
    int hh;
    yesno = FALSE;
    for (hh = 0; hh < 6; hh++)
      if (ch->in_room->exit[hh] && ch->in_room->exit[hh]->d_info && IS_SET (ch->in_room->exit[hh]->d_info->exit_info, EX_HIDDEN))
        yesno = TRUE;
    if (yesno && number_range (1, 3) == 3)
      send_to_char ("\x1B[1;30mYou sense a hidden exit somewhere in this room....\n\r\x1B[37;0mPerhaps you should\n\rsearch in all directions...\n\r", ch);
  }
  if (IS_PLAYER (ch))
  {
    if (HUNTING (ch) != NULL)
      do_track (ch, "continue");
  }
  check_room_more (in_room);
  for (fch = in_room->more->people; fch != NULL; fch = fch_next)
  {
    fch_next = fch->next_in_room;
    if (MASTER (fch) == ch && fch->position >= POSITION_STANDING && FIGHTING (fch) == NULL)
    {
      act ("You follow $N $t.", fch, dir_name[door], ch, TO_CHAR);
      move_char (fch, door);
    }
  }
  if (IS_MOB (ch) && IS_SET (ch->act, ACT_AGGRESSIVE) && ch->in_room && ch->in_room->more && ch->in_room->linkage && !ch->in_room->more->linked_to)
  {
    ROOM_DATA *veh;
    for (veh = ch->in_room->linkage; veh != NULL; veh = veh->linkage)
    {
      if (!veh->more)
        continue;
      if (!veh->more->obj_description
          || veh->more->obj_description[0] == '\0')
        continue;
      if (number_range (1, 2) == 1)
      {
        do_enter (ch, veh->more->obj_description);
        break;
      }
    }
  }

  check_clear_more (in_room);
  return 1;
}

void
do_run (CHAR_DATA * ch, char *argy)
{
  char arg1[STD_LENGTH];
  char arg2[STD_LENGTH];
  int door, count;
  DEFINE_COMMAND ("run", do_run, POSITION_STANDING, 0, LOG_NORMAL, "Usage:  Run <direction> <distance>") 
  argy = one_argy (argy, arg1);
  argy = one_argy (argy, arg2);
  if (ch->position != POSITION_STANDING && ch->position != POSITION_STOPCAST)
    {
      send_to_char ("You must be standing to run!\n\r", ch);
      return;
    }
  if (LEVEL (ch) < 100 && ch->in_room
      && (ch->in_room->sector_type == SECT_WATER_NOSWIM
	  || ch->in_room->sector_type == SECT_WATER_SWIM))
    {
      send_to_char ("You are in water; you can't run!\n\r", ch);
      return;
    }
  if (IS_AFFECTED (ch, AFF_FLYING))
    {
      send_to_char ("Can't run while flying.\n\r", ch);
      return;
    }
  if (arg1[0] == '\0')
    {
      send_to_char ("Run which way?\n\r", ch);
      return;
    }
  count = 0;
  if (arg2[0] != '\0')
    count = atoi (arg2);
  door = get_direction_number (arg1);
  if (door > 5 || door < 0)
    {
      send_to_char ("Run which way?\n\r", ch);
      return;
    }
  NEW_POSITION (ch, POSITION_RUNNING);
  if (count)
    {
      while (count)
	{
	  if (!move_char (ch, door))
	    break;
	  SUBMOVE (ch, 6);
	  upd_mps (ch);
	  if (ch->move < 5)
	    {
	      send_to_char ("\x1B[0;33mYou are too exhausted.\x1B[37;0m\n\r", ch);
	      break;
	    }
	  count--;
	}
      NEW_POSITION (ch, POSITION_STANDING);
      return;
    }
  while (move_char (ch, door))
    {
      if (HAS_FEET_IMPLANT (ch))
	SUBMOVE (ch, 3);
      else
	SUBMOVE (ch, 6);
      upd_mps (ch);
    }
  NEW_POSITION (ch, POSITION_STANDING);
  return;
}


void
do_north (CHAR_DATA * ch, char *argy)
{
  DEFINE_COMMAND ("north", do_north, POSITION_FIGHTING, 0, LOG_NORMAL, "Moves you north.")
#ifndef OLD_FIGHT
    if (ch->fgt && ch->fgt->field)
    {
      if (ch->fgt->field_ticks <= 0)
	{
	  field_move (ch, DIR_NORTH);
	}
      else
	{
	  setnext (ch, "north");
	}
      return;
    }
#endif
  if (FIGHTING (ch) != NULL || ch->position == POSITION_FIGHTING)
    {
      send_to_char ("You are in the middle of combat!\n", ch);
      return;
    }
  move_char (ch, DIR_NORTH);

  return;
}

void
do_east (CHAR_DATA * ch, char *argy)
{
  DEFINE_COMMAND ("east", do_east, POSITION_FIGHTING, 0, LOG_NORMAL, "Moves you east.")
#ifndef OLD_FIGHT
    if (ch->fgt && ch->fgt->field)
    {
      if (ch->fgt->field_ticks <= 0)
	{
	  field_move (ch, DIR_EAST);
	}
      else
	{
	  setnext (ch, "east");
	}
      return;
    }
#endif
  if (FIGHTING (ch) != NULL || ch->position == POSITION_FIGHTING)
    {
      send_to_char ("You are in the middle of combat!\n", ch);
      return;
    }
  move_char (ch, DIR_EAST);
  return;
}

void
do_south (CHAR_DATA * ch, char *argy)
{
  DEFINE_COMMAND ("south", do_south, POSITION_FIGHTING, 0, LOG_NORMAL, "Moves you south.")
#ifndef OLD_FIGHT
    if (ch->fgt && ch->fgt->field)
    {
      if (ch->fgt->field_ticks <= 0)
	{
	  field_move (ch, DIR_SOUTH);
	}
      else
	{
	  setnext (ch, "south");
	}
      return;
    }
#endif
  if (FIGHTING (ch) != NULL || ch->position == POSITION_FIGHTING)
    {
      send_to_char ("You are in the middle of combat!\n", ch);
      return;
    }
  move_char (ch, DIR_SOUTH);
  return;
}

void
do_west (CHAR_DATA * ch, char *argy)
{
  DEFINE_COMMAND ("west", do_west, POSITION_FIGHTING, 0, LOG_NORMAL, "Moves you west.")
#ifndef OLD_FIGHT
    if (ch->fgt && ch->fgt->field)
    {
      if (ch->fgt->field_ticks <= 0)
	{
	  field_move (ch, DIR_WEST);
	}
      else
	{
	  setnext (ch, "west");
	}
      return;
    }
#endif
  if (FIGHTING (ch) != NULL || ch->position == POSITION_FIGHTING)
    {
      send_to_char ("You are in the middle of combat!\n", ch);
      return;
    }
  move_char (ch, DIR_WEST);
  return;
}

void
do_up (CHAR_DATA * ch, char *argy)
{
  DEFINE_COMMAND ("up", do_up, POSITION_STANDING, 0, LOG_NORMAL, "Moves you up.") 
  if (FIGHTING (ch) != NULL || ch->position == POSITION_FIGHTING)
    return;
  move_char (ch, DIR_UP);
  return;
}

void
do_down (CHAR_DATA * ch, char *argy)
{
  DEFINE_COMMAND ("down", do_down, POSITION_STANDING, 0, LOG_NORMAL, "Moves you down.") 
  if (FIGHTING (ch) != NULL || ch->position == POSITION_FIGHTING)
    return;
  move_char (ch, DIR_DOWN);
  return;
}

int
find_door (CHAR_DATA * ch, char *arg)
{
  EXIT_DATA *pexit;
  int door;
  bool FLAGDIR;
  FLAGDIR = FALSE;
  if (!str_cmp (arg, "n") || !str_cmp (arg, "north"))
    {
      door = 0;
      FLAGDIR = TRUE;
    }
  else if (!str_cmp (arg, "e") || !str_cmp (arg, "east"))
    {
      door = 1;
      FLAGDIR = TRUE;
    }
  else if (!str_cmp (arg, "s") || !str_cmp (arg, "south"))
    {
      door = 2;
      FLAGDIR = TRUE;
    }
  else if (!str_cmp (arg, "w") || !str_cmp (arg, "west"))
    {
      door = 3;
      FLAGDIR = TRUE;
    }
  else if (!str_cmp (arg, "u") || !str_cmp (arg, "up"))
    {
      door = 4;
      FLAGDIR = TRUE;
    }
  else if (!str_cmp (arg, "d") || !str_cmp (arg, "down"))
    {
      door = 5;
      FLAGDIR = TRUE;
    }
  else
    {
      for (door = 0; door <= 5; door++)
	{
	  if ((pexit = ch->in_room->exit[door]) != NULL
	      && pexit->d_info && IS_SET (pexit->d_info->exit_info, EX_ISDOOR)
	      && pexit->d_info->keyword != NULL
	      && is_name (arg, pexit->d_info->keyword))
	    return door;
	}
      return -1;
    }
  if ((pexit = ch->in_room->exit[door]) != NULL && pexit->d_info
      && IS_SET (pexit->d_info->exit_info, EX_HIDDEN) && FLAGDIR)
    {
      act ("I see no door $T here.", ch, NULL, get_direction (arg), TO_CHAR);
      return -1;
    }
  if ((pexit = ch->in_room->exit[door]) == NULL)
    {
      act ("I see no door $T here.", ch, NULL, get_direction (arg), TO_CHAR);
      return -1;
    }
  if (!pexit->d_info || !IS_SET (pexit->d_info->exit_info, EX_ISDOOR))
    {
      send_to_char ("You can't do that.\n\r", ch);
      return -1;
    }
  return door;
}

void
do_open (CHAR_DATA * ch, char *argy)
{
  char arg[SML_LENGTH];
  SINGLE_OBJECT *obj;
  int door;
  DEFINE_COMMAND ("open", do_open, POSITION_RESTING, 0, LOG_NORMAL, "Allows you to open a door or object.") 
   argy = one_argy (argy, arg);
  if (!str_cmp ("the", arg))
    argy = one_argy (argy, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Open what?\n\r", ch);
      return;
    }
  if ((door = find_door (ch, arg)) >= 0)
    {
      ROOM_DATA *to_room;
      EXIT_DATA *pexit;
      EXIT_DATA *pexit_rev;
      pexit = ch->in_room->exit[door];
      if (pexit->d_info)
	{
	  if (!IS_SET (pexit->d_info->exit_info, EX_CLOSED))
	    {
	      send_to_char ("It's already open.\n\r", ch);
	      return;
	    }
	  if (IS_SET (pexit->d_info->exit_info, EX_LOCKED))
	    {
	      send_to_char ("It's locked.\n\r", ch);
	      return;
	    }
	  REMOVE_BIT (pexit->d_info->exit_info, EX_CLOSED);
	  act ("$n opens the $T.", ch, NULL, pexit->d_info->keyword, TO_ROOM);
	  act ("You open the $T.", ch, NULL, pexit->d_info->keyword, TO_CHAR);
	  if (IS_JAVA (ch))
	    do_clear (ch, "");
	  if ((to_room = pexit->to_room) != NULL
	      && (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
	      && pexit_rev->to_room == ch->in_room && pexit_rev->d_info)
	    {
	      CHAR_DATA *rch;
	      REMOVE_BIT (pexit_rev->d_info->exit_info, EX_CLOSED);
	      if (to_room->more)
		for (rch = to_room->more->people; rch != NULL;
		     rch = rch->next_in_room)
		  act ("The $T is opened from the other side.", rch, NULL,
		       pexit_rev->d_info->keyword, TO_CHAR);
	    }
	  return;
	}
    }
  if ((obj = get_obj_here (ch, arg, SEARCH_INV_FIRST)) != NULL)
    {
      if (obj->pIndexData->item_type != ITEM_CONTAINER)
	{
	  send_to_char ("That's not a container.\n\r", ch);
	  return;
	}
      else
	{
	  I_CONTAINER *con = (I_CONTAINER *) obj->more;
	  if (!IS_SET (con->flags, CONT_CLOSED))
	    {
	      send_to_char ("It's already open.\n\r", ch);
	      return;
	    }
	  if (!IS_SET (con->flags, CONT_CLOSEABLE))
	    {
	      send_to_char ("You can't do that.\n\r", ch);
	      return;
	    }
	  if (IS_SET (con->flags, CONT_LOCKED))
	    {
	      send_to_char ("It's locked.\n\r", ch);
	      return;
	    }
	  REMOVE_BIT (con->flags, CONT_CLOSED);
	  send_to_char ("It is now closed.\n\r", ch);
	  act ("$n opens $p.", ch, obj, NULL, TO_ROOM);
	  return;
	}
    }
  if (door == -1)
    act ("I see no $T here.", ch, NULL, arg, TO_CHAR);
  return;
}

void
do_close (CHAR_DATA * ch, char *argy)
{
  char arg[SML_LENGTH];
  SINGLE_OBJECT *obj;
  int door;
  DEFINE_COMMAND ("close", do_close, POSITION_RESTING, 0, LOG_NORMAL, "Allows you to close a door or object.") 
  argy = one_argy (argy, arg);
  if (!str_cmp ("the", arg))
    argy = one_argy (argy, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Close what?\n\r", ch);
      return;
    }
  if ((door = find_door (ch, arg)) >= 0)
    {
      ROOM_DATA *to_room;
      EXIT_DATA *pexit;
      EXIT_DATA *pexit_rev;
      pexit = ch->in_room->exit[door];
      if (pexit->d_info)
	{
	  if (IS_SET (pexit->d_info->exit_info, EX_CLOSED))
	    {
	      send_to_char ("It's already closed.\n\r", ch);
	      return;
	    }
	  if (pexit->d_info->str < 1)
	    {
	      send_to_char ("That door is broken...\n\r", ch);
	      return;
	    }
	  SET_BIT (pexit->d_info->exit_info, EX_CLOSED);
	  act ("$n closes the $T.", ch, NULL, pexit->d_info->keyword,
	       TO_ROOM);
	  send_to_char ("It is now closed.\n\r", ch);
	  if ((to_room = pexit->to_room) != NULL
	      && (pexit_rev = to_room->exit[rev_dir[door]]) != 0
	      && pexit_rev->to_room == ch->in_room && pexit_rev->d_info)
	    {
	      CHAR_DATA *rch;
	      SET_BIT (pexit_rev->d_info->exit_info, EX_CLOSED);
	      if (to_room->more)
		for (rch = to_room->more->people; rch != NULL;
		     rch = rch->next_in_room)
		  {
		    act ("The $T is closed from the other side.", rch, NULL,
			 pexit_rev->d_info->keyword, TO_CHAR);
		  }
	    }
	  return;
	}
    }
  if ((obj = get_obj_here (ch, arg, SEARCH_INV_FIRST)) != NULL)
    {
      if (obj->pIndexData->item_type != ITEM_CONTAINER)
	{
	  send_to_char ("That's not a container.\n\r", ch);
	  return;
	}
      else
	{
	  I_CONTAINER *cnt = (I_CONTAINER *) obj->more;
	  if (IS_SET (cnt->flags, CONT_CLOSED))
	    {
	      send_to_char ("It's already closed.\n\r", ch);
	      return;
	    }
	  if (!IS_SET (cnt->flags, CONT_CLOSEABLE))
	    {
	      send_to_char ("You can't do that.\n\r", ch);
	      return;
	    }
	  SET_BIT (cnt->flags, CONT_CLOSED);
	  send_to_char ("It is now closed.\n\r", ch);
	  act ("$n closes $p.", ch, obj, NULL, TO_ROOM);
	  return;
	}
    }
  if (door == -1)
    act ("I see no $T here.", ch, NULL, arg, TO_CHAR);
  return;
}

bool
has_key (CHAR_DATA * ch, int key)
{
  SINGLE_OBJECT *obj;
  if (get_obj_index (key) == NULL)
    return FALSE;
  if (!ch)
    return FALSE;
  for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    if (obj->pIndexData->vnum == key)
      return TRUE;
  if (IS_PLAYER (ch) && IS_SET (ch->act, PLR_HOLYWALK))
    return TRUE;
  return FALSE;
}

void
update_weight_num (CHAR_DATA * ch)
{
  SINGLE_OBJECT *obj;
  if (IS_MOB (ch))
    return;
  if (!ch)
    return;
  ch->pcdata->carry_weight = 0;
  ch->pcdata->carry_number = 0;
  for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {
      ch->pcdata->carry_weight += get_obj_weight (obj);
      ch->pcdata->carry_number += get_obj_number (obj);
    }
  return;
}

void
do_lock (CHAR_DATA * ch, char *argy)
{
  char arg[SML_LENGTH];
  SINGLE_OBJECT *obj;
  int door;
  DEFINE_COMMAND ("lock", do_lock, POSITION_RESTING, 0, LOG_NORMAL, "Allows you to lock a door or object.") 
   one_argy (argy, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Lock what?\n\r", ch);
      return;
    }
  if ((obj = get_obj_here (ch, arg, SEARCH_INV_FIRST)) != NULL)
    {
      if (obj->pIndexData->item_type != ITEM_CONTAINER)
	{
	  send_to_char ("That's not a container.\n\r", ch);
	  return;
	}
      else
	{
	  I_CONTAINER *cnt = (I_CONTAINER *) obj->more;
	  if (!IS_SET (cnt->flags, CONT_CLOSED))
	    {
	      send_to_char ("It's not closed.\n\r", ch);
	      return;
	    }
	  if (cnt->key_vnum <= 0)
	    {
	      send_to_char ("It can't be locked.\n\r", ch);
	      return;
	    }
	  if (!has_key (ch, cnt->key_vnum))
	    {
	      send_to_char ("You don't have the key.\n\r", ch);
	      return;
	    }
	  if (IS_SET (cnt->flags, CONT_LOCKED))
	    {
	      send_to_char ("It's already locked.\n\r", ch);
	      return;
	    }
	  SET_BIT (cnt->flags, CONT_LOCKED);
	  send_to_char ("*Click*\n\r", ch);
	  act ("$n locks $p.", ch, obj, NULL, TO_ROOM);
	  return;
	}
    }
  if ((door = find_door (ch, arg)) >= 0)
    {
      ROOM_DATA *to_room;
      EXIT_DATA *pexit;
      EXIT_DATA *pexit_rev;
      pexit = ch->in_room->exit[door];
      if (pexit->d_info)
	{
	  if (!IS_SET (pexit->d_info->exit_info, EX_CLOSED))
	    {
	      send_to_char ("It's not closed.\n\r", ch);
	      return;
	    }
	  if (pexit->d_info->key < 0)
	    {
	      send_to_char ("It can't be locked.\n\r", ch);
	      return;
	    }
	  if (!has_key (ch, pexit->d_info->key))
	    {
	      send_to_char ("You don't have the key.\n\r", ch);
	      return;
	    }
	  if (IS_SET (pexit->d_info->exit_info, EX_LOCKED))
	    {
	      send_to_char ("It's already locked.\n\r", ch);
	      return;
	    }
	  SET_BIT (pexit->d_info->exit_info, EX_LOCKED);
	  send_to_char ("*Click*\n\r", ch);
	  act ("$n locks the $T.", ch, NULL, pexit->d_info->keyword, TO_ROOM);
	  if ((to_room = pexit->to_room) != NULL
	      && (pexit_rev = to_room->exit[rev_dir[door]]) != 0
	      && pexit_rev->to_room == ch->in_room && pexit_rev->d_info)
	    {
	      CHAR_DATA *rch;
	      SET_BIT (pexit_rev->d_info->exit_info, EX_LOCKED);
	      if (to_room->more)
		for (rch = to_room->more->people; rch != NULL;
		     rch = rch->next_in_room)
		  act ("*Click* The $T is locked from the other side.", rch,
		       NULL, pexit_rev->d_info->keyword, TO_CHAR);
	    }
	}
    }
  return;
}

void
do_unlock (CHAR_DATA * ch, char *argy)
{
  char arg[SML_LENGTH];
  SINGLE_OBJECT *obj;
  int door;
  DEFINE_COMMAND ("unlock", do_unlock, POSITION_RESTING, 0, LOG_NORMAL, "Allows you to unlock a door or object.")
   one_argy (argy, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Unlock what?\n\r", ch);
      return;
    }
  if ((obj = get_obj_here (ch, arg, SEARCH_INV_FIRST)) != NULL)
    {
      if (obj->pIndexData->item_type != ITEM_CONTAINER)
	{
	  send_to_char ("That's not a container.\n\r", ch);
	  return;
	}
      else
	{
	  I_CONTAINER *cnt = (I_CONTAINER *) obj->more;
	  if (!IS_SET (cnt->flags, CONT_CLOSED))
	    {
	      send_to_char ("It's not closed.\n\r", ch);
	      return;
	    }
	  if (cnt->key_vnum <= 0)
	    {
	      send_to_char ("It can't be unlocked.\n\r", ch);
	      return;
	    }
	  if (!has_key (ch, cnt->key_vnum))
	    {
	      send_to_char ("You don't have the key.\n\r", ch);
	      return;
	    }
	  if (!IS_SET (cnt->flags, CONT_LOCKED))
	    {
	      send_to_char ("It's already unlocked.\n\r", ch);
	      return;
	    }
	  REMOVE_BIT (cnt->flags, CONT_LOCKED);
	  act ("$n unlocks $p.", ch, obj, NULL, TO_ROOM);
	  act ("You unlock $p.", ch, obj, NULL, TO_CHAR);
	  return;
	}
    }
  if ((door = find_door (ch, arg)) >= 0)
    {
      ROOM_DATA *to_room;
      EXIT_DATA *pexit;
      EXIT_DATA *pexit_rev;
      pexit = ch->in_room->exit[door];
      if (pexit->d_info)
	{
	  if (!IS_SET (pexit->d_info->exit_info, EX_CLOSED))
	    {
	      send_to_char ("It's not closed.\n\r", ch);
	      return;
	    }
	  if (pexit->d_info->key < 1)
	    {
	      send_to_char ("It can't be unlocked.\n\r", ch);
	      return;
	    }
	  if (!has_key (ch, pexit->d_info->key))
	    {
	      send_to_char ("You don't have the key.\n\r", ch);
	      return;
	    }
	  if (!IS_SET (pexit->d_info->exit_info, EX_LOCKED))
	    {
	      send_to_char ("It's already unlocked.\n\r", ch);
	      return;
	    }
	  REMOVE_BIT (pexit->d_info->exit_info, EX_LOCKED);
	  act ("You unlock the $T.", ch, NULL, pexit->d_info->keyword,
	       TO_CHAR);
	  act ("$n unlocks the $T.", ch, NULL, pexit->d_info->keyword,
	       TO_ROOM);
	  if ((to_room = pexit->to_room) != NULL
	      && (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
	      && pexit_rev->to_room == ch->in_room && pexit_rev->d_info)
	    {
	      REMOVE_BIT (pexit_rev->d_info->exit_info, EX_LOCKED);
	    }
	}
    }
  if (door == -1)
    act ("I see no $T here.", ch, NULL, arg, TO_CHAR);
  return;
}

void
do_pick (CHAR_DATA * ch, char *argy)
{
  char arg[SML_LENGTH];
  CHAR_DATA *gch;
  SINGLE_OBJECT *obj;
  int door;
  DEFINE_COMMAND ("pick", do_pick, POSITION_STANDING, 0, LOG_NORMAL, "Attempts to pick a door or object's lock.  Based on Pick skill.")
    one_argy (argy, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Pick what?\n\r", ch);
      return;
    }
  WAIT_STATE (ch, 28);
  for (gch = ch->in_room->more->people; gch; gch = gch->next_in_room)
    {
      if (IS_MOB (gch) && IS_AWAKE (gch) && IS_SET (gch->act, ACT_SENTINEL))
	{
	  act
	    ("$N is standing too close to the lock; you can't pick that door now!",
	     ch, NULL, gch, TO_CHAR);
	  return;
	}
    }
  if (IS_PLAYER (ch) && number_range (1, 100) > ch->pcdata->learned[gsn_pick_lock])
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }
  if ((door = find_door (ch, arg)) >= 0)
    {
      ROOM_DATA *to_room;
      EXIT_DATA *pexit;
      EXIT_DATA *pexit_rev;
      pexit = ch->in_room->exit[door];
      if (pexit->d_info)
	{
	  if (!IS_SET (pexit->d_info->exit_info, EX_CLOSED))
	    {
	      send_to_char ("It's not closed.\n\r", ch);
	      return;
	    }
	  if (pexit->d_info->key < 0)
	    {
	      send_to_char ("It can't be picked.\n\r", ch);
	      return;
	    }
	  if (!IS_SET (pexit->d_info->exit_info, EX_LOCKED))
	    {
	      send_to_char ("It's already unlocked.\n\r", ch);
	      return;
	    }
	  if (IS_SET (pexit->d_info->exit_info, EX_PICKPROOF))
	    {
	      if (number_range (1, 4) == 2)
		skill_gain (ch, gsn_pick_lock, FALSE);
	      send_to_char ("You failed.\n\r", ch);
	      return;
	    }
	  REMOVE_BIT (pexit->d_info->exit_info, EX_LOCKED);
	  act ("You pick the $T.", ch, NULL, pexit->d_info->keyword, TO_CHAR);
	  act ("$n picks the $T.", ch, NULL, pexit->d_info->keyword, TO_ROOM);
	  skill_gain (ch, gsn_pick_lock, FALSE);
	  if ((to_room = pexit->to_room) != NULL
	      && (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
	      && pexit_rev->to_room == ch->in_room && pexit_rev->d_info)
	    {
	      REMOVE_BIT (pexit_rev->d_info->exit_info, EX_LOCKED);
	    }
	}
    }
  if ((obj = get_obj_here (ch, arg, SEARCH_INV_FIRST)) != NULL)
    {
      if (obj->pIndexData->item_type != ITEM_CONTAINER)
	{
	  send_to_char ("That's not a container.\n\r", ch);
	  return;
	}
      else
	{
	  I_CONTAINER *cnt = (I_CONTAINER *) obj->more;
	  if (!IS_SET (cnt->flags, CONT_CLOSED))
	    {
	      send_to_char ("It's not closed.\n\r", ch);
	      return;
	    }
	  if (cnt->key_vnum <= 0)
	    {
	      send_to_char ("It can't be unlocked.\n\r", ch);
	      return;
	    }
	  if (!IS_SET (cnt->flags, CONT_LOCKED))
	    {
	      send_to_char ("It's already unlocked.\n\r", ch);
	      return;
	    }
	  if (IS_SET (cnt->flags, CONT_PICKPROOF))
	    {
	      send_to_char ("You failed.\n\r", ch);
	      return;
	    }
	  REMOVE_BIT (cnt->flags, CONT_LOCKED);
	  act ("You pick $p.", ch, obj, NULL, TO_CHAR);
	  act ("$n picks $p.", ch, obj, NULL, TO_ROOM);
	  return;
	}
    }
  if (door == -1)
    act ("I see no $T here.", ch, NULL, arg, TO_CHAR);
  return;
}

void
do_stand (CHAR_DATA * ch, char *argy)
{
  AFFECT_DATA *paf;
  DEFINE_COMMAND ("stand", do_stand, POSITION_SLEEPING, 0, LOG_NORMAL, "Makes you stand up, from sleeping, resting, etc...")
    if (RIDING (ch) != NULL)
    {
      send_to_char ("If you wanted to stand on your own two feet you would not be riding!\n\r", ch);
      return;
    }
  if (ch->fgt && ch->fgt->ears > 9)
    {
      send_to_char ("You are bashed, and can't stand yet...\n\r", ch);
      return;
    }
  switch (ch->position)
    {
    case POSITION_SLEEPING:
      if (IS_AFFECTED (ch, AFF_SLEEP))
	{
	  send_to_char ("You can't wake up!\n\r", ch);
	  return;
	}
      send_to_char ("You wake and stand up.\n\r", ch);
      act ("$n wakes and stands up.", ch, NULL, NULL, TO_ROOM);
      NEW_POSITION (ch, POSITION_STANDING);
      break;
    case POSITION_STOPCAST:
      check_fgt (ch);
      send_to_char ("You stop concentrating and stand up.\n\r", ch);
      act ("$n stops concentrating and stands up.", ch, NULL, NULL, TO_ROOM);
      if (FIGHTING (ch) == NULL)
        NEW_POSITION (ch, POSITION_STANDING);
     else
        NEW_POSITION (ch, POSITION_FIGHTING);
        WAIT_STATE (ch, 1 * PULSE_VIOLENCE);
        break;
    case POSITION_BASHED:
      check_fgt (ch);
      act ("You quickly stand, trying to avoid getting hit!", ch, NULL, NULL, TO_CHAR);
      act ("$n quickly stands, trying not to get hit in the process!", ch, NULL, NULL, TO_ROOM);
      if (FIGHTING (ch) == NULL)
	NEW_POSITION (ch, POSITION_STANDING);
      else
	NEW_POSITION (ch, POSITION_FIGHTING);
      ch->wait = 0;
      ch->fgt->combat_delay_count = -5;
      break;
    case POSITION_GROUNDFIGHTING:
      if (number_range (1, pow.groundfight_stand) != 1)
	{
	  send_to_char ("You try to stand, but your opponent won't let you!\n\r", ch);
	  WAIT_STATE (ch, PULSE_VIOLENCE);
	  return;
	}
      else
	{
	  send_to_char ("You manage to wiggle free and stand up!\n\r", ch);
	  NEW_POSITION (ch, POSITION_FIGHTING);
	  if (FIGHTING (ch) != NULL)
	    NEW_POSITION (FIGHTING (ch), POSITION_FIGHTING);
	  WAIT_STATE (ch, 2 * PULSE_VIOLENCE);
	}
      break;
    case POSITION_RESTING:
      send_to_char ("You stand up.\n\r", ch);
      act ("$n stands up.", ch, NULL, NULL, TO_ROOM);
      if (!IS_AFFECTED (ch, AFF_FLYING))
	{
	  if (IS_PLAYER (ch) && ch->in_room
	      && !IS_SET (ch->in_room->room_flags, ROOM_INDOORS)
              && !IS_SET (ch->in_room->room_flags, ROOM_CAVERNS)
	      && IS_SET (ch->pcdata->nat_abilities, AFF_FLYING))
	    {
	      if (IS_AFFECTED (ch, AFF_SNEAK))
		{
		  send_to_char ("You stop sneaking.\n\r", ch);
		  affect_strip (ch, gsn_sneak);
		}
	      SET_BIT (ch->affected_by, AFF_FLYING);
	      send_to_char ("You begin to fly again!\n\r", ch);
	      act ("$n floats gently up in the air.", ch, NULL, NULL, TO_ROOM);
	    }
	  if (ch->in_room && !IS_SET (ch->in_room->room_flags, ROOM_INDOORS) && !IS_SET (ch->in_room->room_flags, ROOM_CAVERNS))
	    {
	      for (paf = ch->affected; paf != NULL; paf = paf->next)
		{
		  if (paf->bitvector != AFF_FLYING)
		    continue;
		  else if (paf->duration > 0)
		    {
		      SET_BIT (ch->affected_by, AFF_FLYING);
		      send_to_char ("You begin to fly again!\n\r", ch);
		      act ("$n floats gently up in the air.", ch, NULL, NULL, TO_ROOM);
		    }
		}
	    }
	}
      NEW_POSITION (ch, POSITION_STANDING);
      break;
    case POSITION_STANDING:
      send_to_char ("You are already standing.\n\r", ch);
      break;
    case POSITION_MEDITATING:
      send_to_char ("You stand up, and stop meditating.\n\r", ch);
      act ("$n stands up and stops meditating.", ch, NULL, NULL, TO_ROOM);
      NEW_POSITION (ch, POSITION_STANDING);
      break;
    case POSITION_FIGHTING:
      send_to_char ("You are already fighting!\n\r", ch);
      break;
    case POSITION_FALLING:
      send_to_char ("You can't! You're falling!\n\r", ch);
      break;
    }
  return;
}

void
do_rest (CHAR_DATA * ch, char *argy)
{
  DEFINE_COMMAND ("rest", do_rest, POSITION_RESTING, 0, LOG_NORMAL, "Rest makes your character sit and rest.") 
  if (RIDING (ch) != NULL)
    {
      send_to_char ("You can't rest while you are riding! Dismount first.\n\r", ch);
      return;
    }
  if (IS_AFFECTED (ch, AFF_FLYING))
    {
      do_land (ch, "");
    }

  switch (ch->position)
    {
    case POSITION_FALLING:
      send_to_char ("You are falling downward.\n\r", ch);
      break;
    case POSITION_SLEEPING:
      send_to_char ("You are already sleeping.\n\r", ch);
      break;
    case POSITION_RESTING:
      send_to_char ("You are already resting.\n\r", ch);
      break;
    case POSITION_MEDITATING:
      send_to_char ("You stop meditating and begin to rest.\n\r", ch);
      act ("$n stops meditating and rests.", ch, NULL, NULL, TO_ROOM);
      NEW_POSITION (ch, POSITION_RESTING);
      break;
    case POSITION_STOPCAST:
      send_to_char ("You stop concentrating and begin resting.\n\r", ch);
      act ("$n stops concentrating and starts to rest.", ch, NULL, NULL, TO_ROOM);
      NEW_POSITION(ch, POSITION_RESTING);
      break;
    case POSITION_STANDING:
      send_to_char ("You rest.\n\r", ch);
      act ("$n rests.", ch, NULL, NULL, TO_ROOM);
      NEW_POSITION (ch, POSITION_RESTING);
      break;
    case POSITION_FIGHTING:
      send_to_char ("You are already fighting!\n\r", ch);
      break;
    }
  return;
}

void
do_sleep (CHAR_DATA * ch, char *argy)
{
  int warmth;
  DEFINE_COMMAND ("sleep", do_sleep, POSITION_SLEEPING, 0, LOG_NORMAL, "Sleep allows you to sleep and recover moves and hps.")
    if (RIDING (ch) != NULL)
    {
      send_to_char ("It might be hazardous while riding. Dismount first.\n\r", ch);
      return;
    }
  if (IS_AFFECTED (ch, AFF_FLYING))
    {
      do_land (ch, "");
    }
  if (FIGHTING (ch) && FIGHTING (ch) != NULL)
    {
      send_to_char ("But you're in the middle of combat.. it wouldn't be to your benefit!\n\r", ch);
      return;
    }
  if (pow.weather && IS_PLAYER (ch) && ch->in_room
      && ch->in_room->sector_type != SECT_INSIDE)
    {
      warmth = get_warmth (ch);
      if (too_cold (warmth, ch))
	{
	  send_to_char
	    ("You try to lie down to sleep, but you are just too cold! brrr...\n\r",
	     ch);
	  return;
	}
    }
  switch (ch->position)
    {
    case POSITION_FALLING:
      send_to_char ("You are falling downward.\n\r", ch);
      break;
    case POSITION_SLEEPING:
      send_to_char ("You are already sleeping.\n\r", ch);
      break;
    case POSITION_MEDITATING:
      send_to_char ("You stop meditating and go to sleep.\n\r", ch);
      act ("$n stops meditating and falls asleep.", ch, NULL, NULL, TO_ROOM);
      NEW_POSITION (ch, POSITION_SLEEPING);
      break;
    case POSITION_STOPCAST:
      send_to_char ("You stop concentrating and go to sleep.\n\r", ch);
      act ("$n stops concentrating and falls asleep.", ch, NULL, NULL, TO_ROOM);
      NEW_POSITION(ch, POSITION_SLEEPING);
      break;
    case POSITION_RESTING:
    case POSITION_STANDING:
      send_to_char ("You sleep.\n\r", ch);
      act ("$n sleeps.", ch, NULL, NULL, TO_ROOM);
      NEW_POSITION (ch, POSITION_SLEEPING);
      break;
    case POSITION_FIGHTING:
      send_to_char ("You are fighting!\n\r", ch);
      break;
    }
  return;
}

void
do_wake (CHAR_DATA * ch, char *argy)
{
  char arg[SML_LENGTH];
  CHAR_DATA *victim;
  DEFINE_COMMAND ("wake", do_wake, POSITION_SLEEPING, 0, LOG_NORMAL, "This command allows you to wake up from sleep.")
    one_argy (argy, arg);
  if (arg[0] == '\0')
    {
      do_stand (ch, argy);
      return;
    }
  if (!IS_AWAKE (ch))
    {
      send_to_char ("You are asleep yourself!\n\r", ch);
      return;
    }
  if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (IS_AWAKE (victim))
    {
      act ("$N is already awake.", ch, NULL, victim, TO_CHAR);
      return;
    }
  if (IS_AFFECTED (victim, AFF_SLEEP))
    {
      act ("You can't wake $M!", ch, NULL, victim, TO_CHAR);
      return;
    }
  if (victim->hit < 1)
    {
      send_to_char ("That character is way too injured!\n\r", ch);
      return;
    }
  NEW_POSITION (victim, POSITION_STANDING);
  act ("You wake $M.", ch, NULL, victim, TO_CHAR);
  act ("$n wakes you.", ch, NULL, victim, TO_VICT);
  return;
}

void
do_sneak (CHAR_DATA * ch, char *argy)
{
  AFFECT_DATA af;
  bool sneakal;
  DEFINE_COMMAND ("sneak", do_sneak, POSITION_STANDING, 0, LOG_NORMAL, "Allows you to sneak.  This is based on Sneak skill.  Skill is checked each step; if you fail, people will see you try to 'sneak' into the room.  If you succeed, your entry will not be noticed.") bzero (&af, sizeof (af)); sneakal = TRUE;
            if (RIDING (ch) != NULL)
		  {
		  send_to_char ("It's tough to sneak while riding, and won't do any good.\n\r", ch); 
                  return;}
            if (!IS_AFFECTED (ch, AFF_SNEAK)) sneakal = FALSE;
	    if (IS_AFFECTED (ch, AFF_FLYING))
		  {
	        send_to_char ("You're too obvious, being in the air.\n\r", ch); 
                return;
                  }
	        send_to_char ("You attempt to move silently.\n\r", ch);
		affect_strip (ch, gsn_sneak);
		 if (IS_MOB (ch) || number_percent () < ch->pcdata->learned[gsn_sneak] + ch->pcdata->plus_sneak)
		  {
		  af.type = gsn_sneak;
		  af.duration = LEVEL (ch);
		  af.location = APPLY_NONE;
		  af.modifier = 0;
		  af.bitvector = AFF_SNEAK; affect_to_char (ch, &af);}
		  return;
               }
void 
do_hide (CHAR_DATA * ch, char *argy)
		  {
  DEFINE_COMMAND ("hide", do_hide, POSITION_STANDING, 0, LOG_NORMAL, "Allows you to hide, based on skill.  People with the search skill can search for you, with some luck.")
      if (RIDING (ch) != NULL)
		  {
	  send_to_char ("Hide while riding! I don't think so...\n\r", ch); 
          return;
                  }
	  if (ch->position != POSITION_STANDING && ch->position != POSITION_RESTING)
		  {
	  send_to_char ("You must be standing or resting to hide.\n\r", ch);
	  return;
                  }
	  if (argy[0] == '\0')
		  {
	  send_to_char ("You attempt to hide.\n\r", ch);
	  if (IS_AFFECTED (ch, AFF_HIDE))
	  REMOVE_BIT (ch->affected_by, AFF_HIDE);
  if (!IN_BATTLE (ch) && (IS_MOB (ch) || number_range (1, 104) <= ch->pcdata->learned[gsn_hide] + ch->pcdata-> plus_hide)) SET_BIT (ch->affected_by, AFF_HIDE);
	  WAIT_STATE (ch, 5 * PULSE_VIOLENCE); return;}
	  check_fgt (ch); ch->fgt->combat_delay_count = 9;
	  do_drop (ch, argy); 
              return;
                  }
void 
do_visible (CHAR_DATA * ch, char *argy)
		  {
  DEFINE_COMMAND ("visible", do_visible, POSITION_SLEEPING, 0, LOG_NORMAL, "This command cancels all effects of invisibility, sneaking, etc...")
	  affect_strip_bits (ch, AFF_INVISIBLE);
	  affect_strip_bits (ch, AFF_HIDE);
	  affect_strip_bits (ch, AFF_SNEAK);
	  send_to_char ("You are now visible.\n\r", ch); return;}
void 
do_recall (CHAR_DATA * ch, char *argy)
	  {
	  char buf[STD_LENGTH];
	  CHAR_DATA * victim;
	  ROOM_DATA * location;
  if (!(!str_cmp (argy, "SPECFUNC_RECALL_CODEONEX")) && LEVEL (ch) > 5)
		  {
	  send_to_char ("The gods laugh at your request! You are no longer a baby!\n\r", ch);
		  act  ("$n still thinks he's a newbie and tries to recall! What a loser!", ch, 0, 0, TO_ROOM); 
                  return;
                  }
	  if (ch->in_room != NULL && ch->in_room->vnum < 1000)
		  {
	  send_to_char ("You are not in a part of the world where you can recall.\n\r", ch); 
                  return;
                  }
	  act ("$n prays for transportation!", ch, 0, 0, TO_ROOM);
		  if (IS_EVIL (ch)) location = get_room_index (99);
	  else
		  location = get_room_index (100);
	  if ((location /* = get_room_index( race_info[ch->race].start_room )*/ ) == NULL)
		  {
	  send_to_char ("You are completely lost.\n\r", ch); 
                  return;
                  }
	  if (ch->in_room == location)
		  {
	  send_to_char ("Your already there!\n\r", ch); 
                  return;
                  }
	  if (IS_SET (ch->in_room->room_flags, ROOM_NO_RECALL) || IS_AFFECTED (ch, AFF_CURSE))
		  {
	  send_to_char ("God has forsaken you.\n\r", ch); 
                  return;
                  }
	  if ((victim = FIGHTING (ch)) != NULL)
		  {
	  int lose; if (number_bits (1) == 0)
		  {
	  WAIT_STATE (ch, 4);
	  lose = (ch->desc != NULL) ? 50 : 10;
	  gain_exp (ch, 0 - lose);
	  sprintf (buf, "You failed! You lose %d exps.\n\r", lose);
	  send_to_char (buf, ch); 
                  return;
                  }
	  lose = (ch->desc != NULL) ? 10 : 20;
	  gain_exp (ch, 0 - lose);
	  sprintf (buf, "You recall from combat! You lose %d exps.\n\r", lose); 
          send_to_char (buf, ch);
	  stop_fighting (ch, TRUE);
                  }
		  ch->move /= 2; upd_mps (ch);
		  act ("$n disappears.", ch, NULL, NULL, TO_ROOM);
		  char_from_room (ch); char_to_room (ch, location);
		  act ("$n appears in the room.", ch, NULL, NULL, TO_ROOM);
		  do_look (ch, "auto"); 
                  return;
                }

/* Blood trails by Simon Lee! */
void 
make_blood (CHAR_DATA * ch, int door)
		  {
        ROOM_DATA * in_room;
	  if (IS_MOB (ch) && ch->pIndexData->mobtype == MOB_GHOST)
	  return;
	  in_room = ch->in_room;
	if (!in_room) 
          return;
	if ((ch->hit >= ch->max_hit / 2)
	   || in_room->sector_type == SECT_AIR
	   || in_room->sector_type == SECT_UNDERWATER
	 //|| in_room->sector_type == SECT_MOUNTAIN 
	 //|| in_room->sector_type == SECT_TROPICAL 
	   || in_room->sector_type == SECT_MARSH 
           || in_room->sector_type == SECT_WATER_SWIM 
           || in_room->sector_type == SECT_WATER_NOSWIM) 
	return;	// no blood in this room
		  {
	SINGLE_OBJECT * blood, *bnext;
	  char buf[SML_LENGTH];
	  for (blood = in_room->more->contents; blood != NULL;
	       blood = bnext)
		  {
	  bnext = blood->next_content;
	  if (blood->pIndexData->vnum == OBJ_VNUM_BLOOD)
		  {
	  I_BLOOD * bld = (I_BLOOD *) blood->more;
	  if (bld->door == door) 
           break;
                }
		  }
		  if (blood == NULL)
		  {
		  I_BLOOD * bld;
		  blood = create_object (get_obj_index (OBJ_VNUM_BLOOD), 0);
		  if (in_room->sector_type == SECT_DESERT)
		  blood->timer = number_range (1, 2);
		else
		  blood->timer = number_range (2, 5);
		  sprintf (buf, "\e[31mYou see a trail of blood going \e[1m%s\e[0;31m.\e[0m\n\r", dir_name[door]);
		    buf[0] = UPPER (buf[0]);
		    blood->description = str_dup (buf);
		    bld = (I_BLOOD *) blood->more;
		    bld->door = door; obj_to (blood, in_room);}
		  }
		  return;
                 }



void 
do_fly (CHAR_DATA * ch, char *argy)
		  {
  AFFECT_DATA * paf;
  DEFINE_COMMAND ("fly", do_fly, POSITION_DEAD, 0, LOG_NORMAL, "If you can, this makes you fly into the air.")
	if (ch->in_room && IS_SET (ch->in_room->room_flags, ROOM_INDOORS) && !IS_SET (ch->in_room->room_flags, ROOM_CAVERNS))
		  {
	 send_to_char ("You are indoors; you can't fly in here!\n\r", ch); 
         return;
                  }
	if (ch->position != POSITION_STANDING && ch->position != POSITION_STOPCAST)
		  {
	send_to_char ("You must be in a standing position to begin flying.\n\r", ch); 
        return;
                  }
	if (IS_AFFECTED (ch, AFF_FLYING))
		  {
	send_to_char ("You're already flying!\n\r", ch); 
        return;
                  }
	if (IS_PLAYER (ch) && IS_SET (ch->pcdata->nat_abilities, AFF_FLYING))
		  {
	if (RIDING (ch) != NULL)
		  {
	send_to_char ("Not while you are riding!\n\r", ch); 
	return;
		  }
	if (IS_AFFECTED (ch, AFF_SNEAK))
		  {
	send_to_char ("You cannot sneak while flying. You stop sneaking.\n\r", ch); 
        affect_strip (ch, gsn_sneak);
		  }
	SET_BIT (ch->affected_by, AFF_FLYING);
	send_to_char ("You begin to fly again!\n\r", ch);
	act ("$n floats gently up in the air.", ch, NULL, NULL, TO_ROOM); 
        return;
		  }
	for (paf = ch->affected; paf != NULL; paf = paf->next)
		  {
	if (paf->bitvector != AFF_FLYING) 
        continue;
     else
	if (paf->duration > 0)
		  {
	if (RIDING (ch) != NULL)
		  {
	send_to_char ("Not while you are riding!\n\r", ch); 
        return;
		  }
	if (IS_AFFECTED (ch, AFF_SNEAK))
		  {
	send_to_char ("You stop sneaking.\n\r", ch);
	affect_strip (ch, gsn_sneak);
                  }
	SET_BIT (ch->affected_by, AFF_FLYING);
	send_to_char ("You begin to fly again!\n\r", ch);
	act ("$n floats gently up in the air.", ch, NULL, NULL, TO_ROOM); return;
                }
		  }
	send_to_char ("You flap your arms, but never leave the ground.\n\r", ch);
        act ("$n flaps $s arms trying to fly, but can't.\n\r", ch, NULL, NULL, TO_ROOM); 
        return; 
                }
void 
do_land (CHAR_DATA * ch, char *argy)
		  {
  DEFINE_COMMAND ("land", do_land, POSITION_DEAD, 0, LOG_NORMAL, "Makes you land on the ground if you are flying.")
	  if (!IS_AFFECTED (ch, AFF_FLYING))
		  {
	  send_to_char ("But you're not flying!\n\r", ch); 
          return;
                  }
	  send_to_char ("You gently float downward to the ground.\n\r", ch);
		  REMOVE_BIT (ch->affected_by, AFF_FLYING);
		  if (!ch->in_room) return;
		  while (VALID_EDGE (ch->in_room, DIR_DOWN) && ch->in_room->sector_type == SECT_AIR)
		  {
  ROOM_DATA * to_room;
	  to_room = ch->in_room->exit[DIR_DOWN]->to_room;
	  act ("$n descends downward.", ch, NULL, NULL, TO_ROOM);
	  char_from_room (ch);
	  char_to_room (ch, to_room);
	  do_look (ch, "auto");
	  act ("$n descends from above.", ch, NULL, NULL, TO_ROOM);}
	  return;}


void 
do_break (CHAR_DATA * ch, char *argy)
		  {
	  char arg[SML_LENGTH];
	  int door;
  DEFINE_COMMAND ("break", do_break, POSITION_STANDING, 0, LOG_NORMAL, "Break <direction> or Break <doorname> allows you to attempt to charge into a door, breaking it.  Based on strength.")
	  one_argy (argy, arg); if (arg[0] == '\0')
		  {
	  send_to_char ("Which door do you want to break?\n\r", ch);
	  return;
                  }
	  if (ch->position != POSITION_STANDING && ch->position != POSITION_STOPCAST)
		  {
	  send_to_char ("You have to be standing to charge a door!\n\r", ch);
	  return;
                  }
	  if ((door = find_door (ch, arg)) >= 0)
		  {
	  /* 'open door' */
	  ROOM_DATA * to_room;
	  EXIT_DATA * pexit;
	  EXIT_DATA * pexit_rev;
	  int dam, broke = 0;
	  char buf[STD_LENGTH];
	  pexit = ch->in_room->exit[door]; if (pexit->d_info)
		  {
	  if (!IS_SET (pexit->d_info->exit_info, EX_CLOSED))
		  {
	  send_to_char ("It's already open.\n\r", ch); return;}
	  if (!(!str_suffix ("wall", pexit->d_info->keyword) ||
		!str_prefix ("wall", pexit->d_info->keyword)
   	     || !str_cmp ("door", pexit->d_info->keyword)
	     || !str_suffix ("doors", pexit->d_info->keyword)
	     || !str_suffix ("door", pexit->d_info->keyword)
	     || !str_prefix ("door", pexit->d_info->keyword)))
		  {
	  send_to_char ("You can't break that...\n\r", ch); return;}
	  if (pexit->d_info->str < 0)
		  {
	  send_to_char ("It's already broken.\n\r", ch); return;}
		  if (door >= 0 && door <= 4)
		  {
	  act ("You charge towards the $T...", ch, NULL, pexit->d_info->keyword, TO_CHAR);
	  act ("$n charges towards the $T...", ch, NULL, pexit->d_info->keyword, TO_ROOM);}
	else
	  if (door == 5)
		  {
	  act ("You stomp on the $T...", ch, NULL, pexit->d_info->keyword, TO_CHAR);
	  act ("$n stomp on the $T...", ch, NULL, pexit->d_info->keyword, TO_ROOM);}
	else
	  if (door == 6)
		  {
	  act ("You pound on the $T...", ch, NULL, pexit->d_info->keyword, TO_CHAR);
	  act ("$n pound on the $T...", ch, NULL, pexit->d_info->keyword, TO_ROOM);}
		  dam = (get_curr_str (ch) * 2 - pexit->d_info->str) / 2;
          if (IS_SET (pexit->d_info->exit_info, EX_BASHPROOF))
		  {
	  pexit->d_info->str = 100; dam = 0;
                  }
	  if (dice (3, 6) < get_curr_str (ch) && dam > 0)
		  {
	  pexit->d_info->str -= number_range (1, get_curr_str (ch) / 3);
		  if ((to_room = pexit->to_room) != NULL
		      && (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
		      && pexit_rev->to_room == ch->in_room
		      && pexit_rev->d_info)
		  {
	  pexit_rev->d_info->str = pexit->d_info->str;
                  }
	  if (pexit->d_info->str > 0)
		  {
	  sprintf (buf, "SLAM! The $T %s", door_cond (pexit->d_info->str));}
	else
	  sprintf (buf, "SLAM!");
	  act (buf, ch, NULL, pexit->d_info->keyword, TO_CHAR);
	  act (buf, ch, NULL, pexit->d_info->keyword, TO_ROOM);}
	else
		  {
	  send_to_char ("Slam! Ouch! That hurt!\n\r", ch);
	  sprintf (buf, "Slam! $n cringes in pain!");
	  act (buf, ch, NULL, NULL, TO_ROOM);
		  if (dam <= 0)
		  dam = dice (1, pexit->d_info->str / 3);
		  if (dam >= ch->hit) ch->hit = 1;
	else
		  damage (ch, ch, dam, TYPE_UNDEFINED); return;}
		  if (pexit->d_info->str < 0)
		  {
	  REMOVE_BIT (pexit->d_info->exit_info, EX_CLOSED);
	  REMOVE_BIT (pexit->d_info->exit_info, EX_ISDOOR);
	  act ("The $T is reduced to rubble!", ch, NULL, pexit->d_info->keyword, TO_ROOM);
	  act ("You reduced the $T to rubble!", ch, NULL, pexit->d_info->keyword, TO_CHAR); 
              broke = 1;
                  }
		  if (pexit->d_info->str > 0) 
                  return;
  /* open the other side */
	  if ((to_room = pexit->to_room) != NULL
	      && (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
	      && pexit_rev->to_room == ch->in_room
	      && pexit_rev->d_info)
		  {
		  REMOVE_BIT (pexit_rev->d_info->exit_info, EX_CLOSED);
		  if (broke)
		  REMOVE_BIT (pexit_rev->d_info->exit_info, EX_ISDOOR);
                  }
		}
	else
		  {
	  send_to_char ("Which door was that?\n\r", ch); return;}
		  }
		  return;
                  }

	  char *door_cond (int val)
		  {
	  static char door_condit[100];
	  if (val > 80)
	  strcpy (door_condit, "looks the same as before.");
	else
	  if (val > 70)
	  strcpy (door_condit, "seems to be weakening a little.");
	else
	  if (val > 55)
	  strcpy (door_condit, "has a visible crack now.");
	else
	  if (val > 39)
	  strcpy (door_condit, "is concave from the impacts!");
	else
	  if (val > 25)
	  strcpy (door_condit, "won't stay standing much longer!");
	else
	  if (val > 14)
	  strcpy (door_condit, "looks like it could break any second!");
	 else
	  strcpy (door_condit, "What door?"); return door_condit;
}
