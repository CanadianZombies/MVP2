#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"

bool diggy = FALSE;

char * verify_room_name (char *rn)
{
  char *t;
  char *tp;
  int len = 0;
  rn[0] = UPPER (rn[0]);
  if (rn[(strlen (rn) - 1)] == '.')
    {
      rn[(strlen (rn) - 1)] = '\0';
    }
  tp = rn;

  for (t = rn; *t != '\0'; t++)
    {
      if (*t == ' ')
	{
	  tp = t + 1;
	  len = 0;
	  continue;
	}
      len++;
      if (len > 3 && *tp != 'n')
	{
	  len = 0;
	  *tp = UPPER (*tp);
	}
    }
  return rn;
}

int sector_toggle (char *inpt)
{
  int i = 0;
  if ((i = get_sector_number (inpt)) == SECT_MAX)
    return 0;
  return (1 << i);
}

char * sector_fname (int flag)
{
  static char rbbv[200];
  rbbv[0] = '\0';
  if (IS_SET (flag, (1 << SECT_INSIDE)))
    strcat (rbbv, "inside ");
  if (IS_SET (flag, (1 << SECT_CITY)))
    strcat (rbbv, "city ");
  if (IS_SET (flag, (1 << SECT_FIELD)))
    strcat (rbbv, "field ");
  if (IS_SET (flag, (1 << SECT_FOREST)))
    strcat (rbbv, "forest ");
  if (IS_SET (flag, (1 << SECT_HILLS)))
    strcat (rbbv, "hills ");
  if (IS_SET (flag, (1 << SECT_MOUNTAIN)))
    strcat (rbbv, "mountain ");
  if (IS_SET (flag, (1 << SECT_WATER_SWIM)))
    strcat (rbbv, "swim ");
  if (IS_SET (flag, (1 << SECT_WATER_NOSWIM)))
    strcat (rbbv, "noswim ");
  if (IS_SET (flag, (1 << SECT_UNDERWATER)))
    strcat (rbbv, "underwater ");
  if (IS_SET (flag, (1 << SECT_AIR)))
    strcat (rbbv, "air ");
  if (IS_SET (flag, (1 << SECT_DESERT)))
    strcat (rbbv, "desert ");
  if (IS_SET (flag, (1 << SECT_SNOW)))
    strcat (rbbv, "snow ");
  if (IS_SET (flag, (1 << SECT_TROPICAL)))
    strcat (rbbv, "tropical ");
  if (IS_SET (flag, (1 << SECT_ICE)))
    strcat (rbbv, "ice ");
  if (IS_SET (flag, (1 << SECT_MARSH)))
    strcat (rbbv, "marsh ");
  return rbbv;
}

void redit (CHAR_DATA * ch, char *argy)
{
  AREA_DATA *pArea;
  ROOM_DATA *pRoom;
  EXIT_DATA *pExit;
  DESCRIPTION_DATA *ed;
  char arg[STD_LENGTH];
  char arg1[STD_LENGTH];
  char arg2[STD_LENGTH];
  char buf[STD_LENGTH];
  int value = 0;
  int iHash = 0;
  int door = 0;
  strcpy (arg, argy);
  smash_tilde (argy);
  argy = one_argy (argy, arg1);
  strcpy (arg2, argy);
  pRoom = ch->in_room;
  pArea = pRoom->area;
  check_room_more (ch->in_room);
  hugebuf_o[0] = '\0';

  if (!IS_BUILDER (ch, pArea))
    {
      send_to_char ("AEdit: Insufficient security to modify room.\n\r", ch);
      interpret (ch, arg);
      return;
    }

  if (!str_cmp (arg1, "show") || arg1[0] == '\0')
    {
      sprintf (buf, "%d", pRoom->vnum);
      do_rstat (ch, buf);
      return;
    }

  if (!str_cmp (arg1, "done"))
    {
      ch->desc->pEdit = NULL;
      ch->desc->connected = CON_PLAYING;
      return;
    }

  if (!str_cmp (arg1, "?"))
    {
      do_help (ch, "redit");
      return;
    }

  if (!IS_BUILDER (ch, pArea))
    {
      interpret (ch, arg);
      return;
    }

  if (!str_cmp (arg1, "}") && pRoom->vnum < top_vnum_room && IS_REAL_GOD (ch))
    {
      int jj = 0;
      ROOM_DATA *rid;
      rid = NULL;
      jj = pRoom->vnum;
      while (rid == NULL)
	{
	  jj++;
	  rid = get_room_index (jj);
	}
      char_from_room (ch);
      char_to_room (ch, rid);
      do_look (ch, "");
      return;
    }
  if (!str_prefix ("app", arg1))
    {
      pRoom->a = arg2[0];
      send_to_char ("Appearance set.\n\r", ch);
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      return;
    }
  if (!str_prefix ("java", arg1))
    {
      char arg3[200];
      int n1, n2;
      char *t = arg2;
      t = one_argy (t, arg3);
      if ((n1 = atoi (arg3)) < 0)
	return;
      if ((n2 = atoi (t)) < 0)
	return;
      pRoom->img[0] = (char) n1;
      pRoom->img[1] = (char) n2;
      send_to_char ("Java appearance set.\n\r", ch);
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      return;
    }

  if (!str_prefix ("can_travel_sect", arg1))
    {
      int ii = 0;
      if ((ii = sector_toggle (arg2)) > 0)
	{
	  pRoom->more->can_travel_sectors ^= ii;
	  send_to_char ("Toggled.\n\r", ch);
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  return;
	}
    }
  if (!str_prefix ("col", arg1))
    {
      int isit = -1;
      if (arg2[0] == '\0')
	{
	  pRoom->c = '\0';
	  return;
	}
      else
	{
	  isit = atoi (arg2);
	  if (isit < 1 || isit > 15)
	    {
	      send_to_char ("Bad color!\n\r", ch);
	      return;
	    }
	}
      pRoom->c = isit;
      send_to_char ("Color set.\n\r", ch);
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      return;
    }
  if (!str_cmp ("orig_room", arg1))
    {
      int vk;
      vk = atoi (arg2);
      if (vk < 0)
	return;
      if (!IS_SET (pRoom->room_flags, ROOM_ISVEHICLE))
	return;
      pRoom->more->orig_room = vk;
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      return;
    }
  if (!str_cmp ("linkit", arg1))
    {
      int vk;
      ROOM_DATA *rrs;
      vk = atoi (arg2);
      if ((rrs = get_room_index (vk)) == NULL)
	{
	  send_to_char ("That room doesn't exist!\n\r", ch);
	  return;
	}
      if (vk <= 1)
	return;
      if (!IS_SET (pRoom->room_flags, ROOM_ISVEHICLE))
	return;
      if (pRoom->more->linked_to)
	room_from_room (pRoom);
      room_to_room (pRoom, rrs, TRUE);
      send_to_char ("Link set.\n\r", ch);
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      return;
    }
  if (!str_cmp (arg1, "tps") && !str_cmp (NAME (ch), "Odie"))
    {
      int tt;
      if (!is_number (arg2) || (tt = atoi (arg2)) < 0)
	{
	  send_to_char ("Syntax is 'tps <tps_in_this_room>'.\n\r", ch);
	  return;
	}
      pRoom->tps = tt;
      send_to_char ("Travel points in room set.\n\r", ch);
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      return;
    }

  if (!str_cmp (arg1, "sh"))
    {
      if (is_number (arg2))
	{
	  int vv;
	  int tt;
	  tt = atoi (arg2);
	  if (tt <= pRoom->vnum)
	    return;
	  for (vv = pRoom->vnum; vv <= tt; vv++)
	    {
	      ROOM_DATA *rid;
	      if ((rid = get_room_index (vv)) == NULL || rid->area != pArea)
		continue;
	      rid->shade = TRUE;
	    }
	  send_to_char
	    ("In-shade set for current vnum to specified vnum.\n\r", ch);
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  return;
	}
      pRoom->shade = TRUE;
      send_to_char ("This room is now marked as shaded.\n\r", ch);
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      return;
    }

  if (!str_cmp (arg1, "li"))
    {
      if (is_number (arg2))
	{
	  int vv;
	  int tt;
	  tt = atoi (arg2);
	  if (tt <= pRoom->vnum)
	    return;
	  for (vv = pRoom->vnum; vv <= tt; vv++)
	    {
	      ROOM_DATA *rid;
	      if ((rid = get_room_index (vv)) == NULL || rid->area != pArea)
		continue;
	      rid->shade = FALSE;
	    }
	  send_to_char ("Lighted set for current vnum to specified vnum.\n\r",
			ch);
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  return;
	}
      pRoom->shade = FALSE;
      send_to_char ("This room is now marked as lighted.\n\r", ch);
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      return;
    }



  if (room_name_bit (arg1) != ROOM_NONE)
    {
      pRoom->room_flags ^= room_name_bit (arg1);
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      send_to_char ("Room flag toggled.\n\r", ch);
      return;
    }

  if (get_sector_number (arg1) != SECT_MAX)
    {
      pRoom->sector_type = get_sector_number (arg1);
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      send_to_char ("Sector type set.\n\r", ch);
      return;
    }

  if (!str_cmp (arg1, "copyfrom"))
    {
      ROOM_DATA *rid;
      value = atoi (arg2);
      if (arg2[0] == '\0' || !is_number (arg2))
	{
	  send_to_char ("Syntax: copyfrom <copyfrom vnum>\n\r", ch);
	  return;
	}
      if ((rid = get_room_index (value)) == NULL)
	{
	  send_to_char ("That room doesn't exist.\n\r", ch);
	  return;
	}
      if (!IS_BUILDER (ch, rid->area))
	{
	  send_to_char ("Sorry, you don't have access to that room.\n\r", ch);
	  return;
	}
      if (pRoom->description != NULL)
	free_string (pRoom->description);
      pRoom->description = str_dup (rid->description);
      send_to_char ("Copy Successful.\n\r", ch);
      return;
    }

  if (!str_cmp (arg1, "nameto"))
    {
      ROOM_DATA *rid;
      value = atoi (arg2);
      if (arg2[0] == '\0' || !is_number (arg2))
	{
	  send_to_char ("Syntax: nameto <nameto vnum>\n\r", ch);
	  return;
	}
      if ((rid = get_room_index (value)) == NULL)
	{
	  send_to_char ("That room doesn't exist.\n\r", ch);
	  return;
	}
      if (!IS_BUILDER (ch, rid->area))
	{
	  send_to_char ("Sorry, you don't have access to that room.\n\r", ch);
	  return;
	}
      if (rid->name != NULL)
	free_string (rid->name);
      rid->name = str_dup (pRoom->name);
      rid->sector_type = pRoom->sector_type;
      send_to_char ("Namecopy and sectorcopy Successful.\n\r", ch);
      return;
    }

  if (!str_cmp (arg1, "namefrom"))
    {
      ROOM_DATA *rid;
      value = atoi (arg2);
      if (arg2[0] == '\0' || !is_number (arg2))
	{
	  send_to_char ("Syntax: namefrom <namefrom vnum>\n\r", ch);
	  return;
	}
      if ((rid = get_room_index (value)) == NULL)
	{
	  send_to_char ("That room doesn't exist.\n\r", ch);
	  return;
	}
      if (!IS_BUILDER (ch, rid->area))
	{
	  send_to_char ("Sorry, you don't have access to that room.\n\r", ch);
	  return;
	}
      if (pRoom->name != NULL)
	free_string (pRoom->name);
      pRoom->name = str_dup (rid->name);
      pRoom->sector_type = rid->sector_type;
      send_to_char ("Namecopy and sectorcopy Successful.\n\r", ch);
      return;
    }



  if (!str_cmp (arg1, "copyto"))
    {
      ROOM_DATA *rid;
      value = atoi (arg2);
      if (arg2[0] == '\0' || !is_number (arg2))
	{
	  send_to_char ("Syntax: copyto <destination vnum>\n\r", ch);
	  return;
	}
      if ((rid = get_room_index (value)) == NULL)
	{
	  send_to_char ("The destination room doesn't exist.\n\r", ch);
	  return;
	}
      if (!IS_BUILDER (ch, rid->area))
	{
	  send_to_char ("Sorry, you don't have access to that room.\n\r", ch);
	  return;
	}
      if (rid->description != NULL)
	free_string (rid->description);
      rid->description = str_dup (pRoom->description);
      send_to_char ("Copy Successful.\n\r", ch);
      return;
    }

  if (!str_cmp (arg1, "objdescript") || !str_cmp (arg1, "obj_descript"))
    {
      check_room_more (pRoom);
      if (!pRoom->more->obj_description)
	{
	  pRoom->more->obj_description =
	    str_dup ("This is the description of the room object.\n\r");
	}
      if (arg2[0] == '\0')
	{
	  string_append (ch, &pRoom->more->obj_description);
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  return;
	}
      return;
    }



  if (!str_cmp (arg1, "movemess") || !str_cmp (arg1, "movemessage"))
    {
      check_room_more (pRoom);
      if (arg2[0] == '\0')
	{
	  string_append (ch, &pRoom->more->move_message);
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  return;
	}
      return;
    }

  if ((door = get_direction_number (arg1)) != DIR_MAX && arg2[0] != '\0')
    {
      argy = one_argy (argy, arg1);
      strcpy (arg2, argy);
      if (!str_cmp (arg1, "delete"))
	{
	  if (pRoom->exit[door] == NULL)
	    {
	      send_to_char ("That exit does not exist!\n\r", ch);
	      return;
	    }
	  free_exit (pRoom->exit[door]);
	  pRoom->exit[door] = NULL;
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  send_to_char
	    ("Exit unlinked.  Please remember to unlink the other side, too.\n\r",
	     ch);

	  if (!(pRoom->x == 0 && pRoom->y == 0 && pRoom->z == 0))
	    assign_coordinates (ch, "");
	  return;
	}
      if (!str_cmp (arg1, "destroy"))
	{
	  ROOM_DATA *rd;
	  if (pRoom->exit[door] == NULL)
	    {
	      send_to_char ("That exit does not exist!\n\r", ch);
	      return;
	    }
	  if (pRoom->exit[door]->to_room == NULL ||
	      pRoom->exit[door]->to_room->exit[rev_dir[door]] == NULL ||
	      pRoom->exit[door]->to_room->exit[rev_dir[door]]->to_room !=
	      pRoom)
	    {
	      send_to_char ("There is no exit from the other room.  Please use delete.\n\r", ch);
	      return;
	    }
	  rd = pRoom->exit[door]->to_room;
	  free_exit (pRoom->exit[door]);
	  pRoom->exit[door] = NULL;
	  free_exit (rd->exit[rev_dir[door]]);
	  rd->exit[rev_dir[door]] = NULL;
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  send_to_char ("Exit and opposite side unlinked.\n\r", ch);
	  if (!(pRoom->x == 0 && pRoom->y == 0 && pRoom->z == 0))
	    assign_coordinates (ch, "");
	  return;
	}

      if (!str_cmp (arg1, "notransport"))
	{
	  check_room_more (pRoom);
	  pRoom->more->move_dir = 0;
	  if (IS_SET (pRoom->room_flags, ROOM_MOVING))
	    pRoom->room_flags -= ROOM_MOVING;
	  send_to_char ("Transport removed.\n\r", ch);
	  return;
	}
      if (!str_cmp (arg1, "transport"))
	{
	  check_room_more (pRoom);
	  pRoom->more->move_dir = door;
	  SET_BIT (pRoom->room_flags, ROOM_MOVING);
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  send_to_char ("Transport made.\n\r", ch);
	  return;
	}
      if (!str_cmp (arg1, "link"))
	{
	  ROOM_DATA *rd;
	  if (arg2[0] == '\0' || !is_number (arg2))
	    {
	      send_to_char ("Syntax: [direction] link [vnum]\n\r", ch);
	      return;
	    }
	  value = atoi (arg2);
	  if ((rd = get_room_index (value)) == NULL)
	    {
	      if (!(pRoom->x == 0 && pRoom->y == 0 && pRoom->z == 0))
		{
		  if (door == DIR_EAST)
		    rd = find_coord ((pRoom->x) + 1, (pRoom->y), (pRoom->z));
		  if (door == DIR_WEST)
		    rd = find_coord ((pRoom->x) - 1, (pRoom->y), (pRoom->z));
		  if (door == DIR_SOUTH)
		    rd = find_coord ((pRoom->x), (pRoom->y) - 1, (pRoom->z));
		  if (door == DIR_NORTH)
		    rd = find_coord ((pRoom->x), (pRoom->y) + 1, (pRoom->z));
		  if (door == DIR_UP)
		    rd = find_coord ((pRoom->x), (pRoom->y), (pRoom->z) + 1);
		  if (door == DIR_DOWN)
		    rd = find_coord ((pRoom->x), (pRoom->y), (pRoom->z) - 1);
		}
	      if (rd == NULL)
		{
		  send_to_char ("That room doesn't exist...\n\r", ch);
		  return;
		}
	    }
	  if (!IS_BUILDER (ch, rd->area))
	    {
	      send_to_char ("You don't have access to the remote side.\n\r", ch);
	      return;
	    }
	  if (rd->exit[rev_dir[door]] != NULL)
	    {
	      send_to_char ("The other side already has an exit!\n\r", ch);
	      return;
	    }
	  if (!(pRoom->x == 0 && pRoom->y == 0 && pRoom->z == 0))
	    {
	      ROOM_DATA *rt = NULL;
	      if (!(rd->x == 0 && rd->y == 0 && rd->z == 0))
		{
		  int nx, ny, nz;
		  nx = pRoom->x;
		  ny = pRoom->y;
		  nz = pRoom->z;
		  if (door == DIR_EAST)
		    nx++;
		  if (door == DIR_WEST)
		    nx--;
		  if (door == DIR_SOUTH)
		    ny--;
		  if (door == DIR_NORTH)
		    ny++;
		  if (door == DIR_UP)
		    nz++;
		  if (door == DIR_DOWN)
		    nz--;
		  if (nx != rd->x || ny != rd->y || nz != rd->z)
		    {
		      send_to_char ("There is already a room occupying that physical coordinate.\n\r", ch);
		      send_to_char ("To link to this room, use <direction> link.\n\r", ch);
		      return;
		    }
		}
	      if (door == DIR_EAST)
		rt = find_coord ((pRoom->x) + 1, (pRoom->y), (pRoom->z));
	      if (door == DIR_WEST)
		rt = find_coord ((pRoom->x) - 1, (pRoom->y), (pRoom->z));
	      if (door == DIR_SOUTH)
		rt = find_coord ((pRoom->x), (pRoom->y) - 1, (pRoom->z));
	      if (door == DIR_NORTH)
		rt = find_coord ((pRoom->x), (pRoom->y) + 1, (pRoom->z));
	      if (door == DIR_UP)
		rt = find_coord ((pRoom->x), (pRoom->y), (pRoom->z) + 1);
	      if (door == DIR_DOWN)
		rt = find_coord ((pRoom->x), (pRoom->y), (pRoom->z) - 1);
	      if (rt && rt != rd)
		{
		  send_to_char ("There is already a room occupying that physical coordinate.\n\r", ch);
		  send_to_char ("To link to this room, use <direction> link.\n\r", ch);
		  return;
		}
	    }

	  if (pRoom->exit[door] == NULL)
	    {
	      pRoom->exit[door] == new_exit ();
	    }
	  (ROOM_DATA *) pRoom->exit[door]->to_room == rd;
	  pRoom->exit[door]->vnum = rd->vnum;
	  pRoom = rd;
	  door = rev_dir[door];
	  pExit = new_exit ();
	  (ROOM_DATA *) pExit->to_room == ch->in_room;
	  pExit->vnum = ch->in_room->vnum;
	  pRoom->exit[door] = pExit;
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  send_to_char ("Exit linked.\n\r", ch);
	  pRoom = ch->in_room;
	  if (!(rd->x == 0 && rd->y == 0 && rd->z == 0) ||
	      !(pRoom->x == 0 && pRoom->y == 0 && pRoom->z == 0))
	    {
	      assign_coordinates (ch, "");
	    }
	  return;
	}
      if (!str_cmp (arg1, "dig"))
	{
	  if (arg2[0] == '\0' || !is_number (arg2))
	    {
	      send_to_char ("Syntax: [direction] dig <vnum>\n\r", ch);
	      return;
	    }
	  diggy = TRUE;
	  sprintf (buf, "create %s", arg2);
	  redit (ch, buf);
	  if (!diggy)
	    return;
	  sprintf (buf, "%s link %s", dir_name[door], arg2);
	  redit (ch, buf);
	  diggy = FALSE;
	  return;
	}
      if (!str_cmp (arg1, "room"))
	{
	  if (arg2[0] == '\0' || !is_number (arg2))
	    {
	      send_to_char ("Syntax: [direction] room [vnum]\n\r", ch);
	      return;
	    }
	  value = atoi (arg2);
	  if (get_room_index (value) == NULL)
	    {
	      send_to_char ("That room doesn't exist.\n\r", ch);
	      return;
	    }
	  if (pRoom->exit[door] == NULL)
	    {
	      pRoom->exit[door] = new_exit ();
	    }
	  (ROOM_DATA *) pRoom->exit[door]->to_room == get_room_index (value);
	  /*pRoom->exit[door]->vnum = value; */
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  send_to_char ("Exit set.\n\r", ch);
	  return;
	}
      if (!str_cmp (arg1, "str"))
	{
	  if (arg2[0] == '\0' || !is_number (arg2))
	    {
	      send_to_char ("Syntax: [direction] str [val]\n\r", ch);
	      return;
	    }
	  if (pRoom->exit[door] == NULL)
	    {
	      send_to_char ("You must first make this an exit.\n\r", ch);
	      return;
	    }
	  if (pRoom->exit[door]->d_info == NULL
	      || !IS_SET (pRoom->exit[door]->d_info->exit_info, EX_ISDOOR))
	    {
	      send_to_char ("You must have a door first.\n\r", ch);
	      return;
	    }
	  value = atoi (arg2);
	  pRoom->exit[door]->d_info->str = value;
	  pRoom->exit[door]->d_info->maxstr = value;
	  if (((ROOM_DATA *) pRoom->exit[door]->to_room)->
	      exit[rev_dir[door]] != NULL
	      && ((ROOM_DATA *) pRoom->exit[door]->to_room)->
	      exit[rev_dir[door]]->d_info != NULL)
	    {
	      ((ROOM_DATA *) pRoom->exit[door]->to_room)->
		exit[rev_dir[door]]->d_info->str = value;
	      ((ROOM_DATA *) pRoom->exit[door]->to_room)->
		exit[rev_dir[door]]->d_info->maxstr = value;
	    }
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  send_to_char ("Exit str set.\n\r", ch);
	  return;
	}
      if (!str_cmp (arg1, "key"))
	{
	  if (arg2[0] == '\0' || !is_number (arg2))
	    {
	      send_to_char ("Syntax: [direction] key [vnum]\n\r", ch);
	      return;
	    }
	  if (pRoom->exit[door] == NULL || pRoom->exit[door]->d_info == NULL)
	    {
	      send_to_char ("You must first make this an exit.\n\r", ch);
	      return;
	    }
	  value = atoi (arg2);
	  if (get_obj_index (value) == NULL)
	    {
	      send_to_char ("There is no key with that vnum.\n\r", ch);
	      return;
	    }
	  pRoom->exit[door]->d_info->key = value;
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  send_to_char ("Exit key set.\n\r", ch);
	  return;
	}
      if (!str_cmp (arg1, "name"))
	{
	  if (arg2[0] == '\0')
	    {
	      send_to_char ("Syntax: [direction] name [string]\n\r", ch);
	      return;
	    }
	  if (pRoom->exit[door] == NULL || pRoom->exit[door]->d_info == NULL)
	    {
	      send_to_char ("You must set <direction> DOOR first.\n\r", ch);
	      return;
	    }
	  if (pRoom->exit[door]->d_info->keyword != NULL &&
	      pRoom->exit[door]->d_info->keyword[0] != '\0')
	    free_string (pRoom->exit[door]->d_info->keyword);
	  pRoom->exit[door]->d_info->keyword = str_dup (arg2);
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  send_to_char ("Exit name set.\n\r", ch);
	  return;
	}
      if (!str_cmp (arg1, "desc") || !str_cmp (arg1, "description"))
	{
	  if (pRoom->exit[door] == NULL)
	    {
	      pRoom->exit[door] = new_exit ();
	    }
	  if (pRoom->exit[door]->d_info == NULL)
	    {
	      pRoom->exit[door]->d_info = new_door ();
	    }
	  if (arg2[0] == '\0')
	    {
	      string_append (ch, &pRoom->exit[door]->d_info->description);
	      SET_BIT (pArea->area_flags, AREA_CHANGED);
	      return;
	    }
	  if (arg2[0] == '+')
	    {
	      string_append (ch, &pRoom->exit[door]->d_info->description);
	      SET_BIT (pArea->area_flags, AREA_CHANGED);
	      return;
	    }
	  send_to_char ("Syntax: [direction] desc - line edit\n\r", ch);
	  send_to_char (" [direction] desc + - line append\n\r", ch);
	  return;
	}
      if (get_exit_flag_number (arg1) != EX_NONE)
	{
	  if (pRoom->exit[door] == NULL)
	    {
	      send_to_char
		("You must LINK this room to another before setting this.\n\r",
		 ch);
	      return;
	    }
	  if (pRoom->exit[door]->d_info == NULL)
	    {
	      pRoom->exit[door]->d_info = new_door ();
	    }
	  pRoom->exit[door]->d_info->rs_flags ^= get_exit_flag_number (arg1);
	  pRoom->exit[door]->d_info->exit_info ^= get_exit_flag_number (arg1);
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  send_to_char ("Exit flag toggled.\n\r", ch);
	  return;
	}
    }

  if (!str_cmp (arg1, "ed"))
    {
      if (arg2[0] == '\0')
	{
	  send_to_char ("Syntax: ed add [keyword]\n\r", ch);
	  send_to_char (" ed delete [keyword]\n\r", ch);
	  send_to_char (" ed edit [keyword]\n\r", ch);
	  send_to_char (" ed format [keyword]\n\r", ch);
	  return;
	}
      argy = one_argy (argy, arg1);
      strcpy (arg2, argy);
      if (!str_cmp (arg1, "add"))
	{
	  if (arg2[0] == '\0')
	    {
	      send_to_char ("Syntax: ed add [keyword]\n\r", ch);
	      return;
	    }
	  ed = new_extra_descr ();
	  ed->keyword = str_dup (arg2);
	  ed->next = pRoom->more->extra_descr;
	  pRoom->more->extra_descr = ed;
	  string_append (ch, &ed->description);
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  return;
	}
      if (!str_cmp (arg1, "fadd"))
	{
	  if (arg2[0] == '\0')
	    {
	      send_to_char ("Syntax: ed fadd [keyword]\n\r", ch);
	      return;
	    }
	  ed = new_extra_descr ();
	  ed->keyword = str_dup (arg2);
	  ed->next = pRoom->more->extra_descr;
	  pRoom->more->extra_descr = ed;
	  fullscreen_editor (ch, &ed->description);
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  return;
	}
      if (!str_cmp (arg1, "edit"))
	{
	  if (arg2[0] == '\0')
	    {
	      send_to_char ("Syntax: ed edit [keyword]\n\r", ch);
	      return;
	    }
	  for (ed = pRoom->more->extra_descr; ed != NULL; ed = ed->next)
	    {
	      if (is_name (arg2, ed->keyword))
		break;
	    }
	  if (ed == NULL)
	    {
	      send_to_char ("Extra description keyword not found!\n\r", ch);
	      return;
	    }
	  string_append (ch, &ed->description);
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  return;
	}
      if (!str_cmp (arg1, "fedit"))
	{
	  if (arg2[0] == '\0')
	    {
	      send_to_char ("Syntax: ed fedit [keyword]\n\r", ch);
	      return;
	    }
	  for (ed = pRoom->more->extra_descr; ed != NULL; ed = ed->next)
	    {
	      if (is_name (arg2, ed->keyword))
		break;
	    }
	  if (ed == NULL)
	    {
	      send_to_char ("Extra description keyword not found!\n\r", ch);
	      return;
	    }
	  fullscreen_editor (ch, &ed->description);
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  return;
	}
      if (!str_cmp (arg1, "append"))
	{
	  if (arg2[0] == '\0')
	    {
	      send_to_char ("Syntax: ed edit [keyword]\n\r", ch);
	      return;
	    }
	  for (ed = pRoom->more->extra_descr; ed != NULL; ed = ed->next)
	    {
	      if (is_name (arg2, ed->keyword))
		break;
	    }
	  if (ed == NULL)
	    {
	      send_to_char ("Extra description keyword not found!\n\r", ch);
	      return;
	    }
	  string_append (ch, &ed->description);
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  return;
	}
      if (!str_cmp (arg1, "delete"))
	{
	  DESCRIPTION_DATA *ped;
	  if (arg2[0] == '\0')
	    {
	      send_to_char ("Syntax: ed delete [keyword]\n\r", ch);
	      return;
	    }
	  ped = NULL;
	  for (ed = pRoom->more->extra_descr; ed != NULL; ed = ed->next)
	    {
	      if (is_name (arg2, ed->keyword))
		break;
	      ped = ed;
	    }
	  if (ed == NULL)
	    {
	      send_to_char ("Extra description keyword not found!\n\r", ch);
	      return;
	    }
	  if (ped == NULL)
	    {
	      pRoom->more->extra_descr = ed->next;
	    }
	  else
	    {
	      ped->next = ed->next;
	    }
	  free_extra_descr (ed);
	  send_to_char ("Extra description deleted.\n\r", ch);
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  return;
	}
      if (!str_cmp (arg1, "format"))
	{
	  DESCRIPTION_DATA *ped;
	  if (arg2[0] == '\0')
	    {
	      send_to_char ("Syntax: ed format [keyword]\n\r", ch);
	      return;
	    }
	  ped = NULL;
	  for (ed = pRoom->more->extra_descr; ed != NULL; ed = ed->next)
	    {
	      if (is_name (arg2, ed->keyword))
		break;
	      ped = ed;
	    }
	  if (ed == NULL)
	    {
	      send_to_char ("Extra description keyword not found!\n\r", ch);
	      return;
	    }
	  ed->description = format_string (ed->description);
	  send_to_char ("Extra description formatted.\n\r", ch);
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  return;
	}
    }

  if (!str_cmp (arg1, "create"))
    {
      value = atoi (arg2);
      if (arg2[0] == '\0' || value == 0)
	{
	  send_to_char ("Syntax: create [vnum]\n\r", ch);
	  return;
	}
      pArea = get_vnum_area (value);
      if (pArea == NULL)
	{
	  send_to_char ("That vnum is not assigned an area.\n\r", ch);
	  return;
	}
      if (!IS_BUILDER (ch, pArea))
	{
	  send_to_char ("Vnum in an area you can't build in.\n\r", ch);
	  return;
	}
      if (get_room_index (value) != NULL)
	{
	  send_to_char ("Room vnum already exists.\n\r", ch);
	  diggy = FALSE;
	  return;
	}
      pRoom = new_room_index ();
      pRoom->area = pArea;
      pRoom->vnum = value;
      if (value > top_vnum_room)
	top_vnum_room = value;
      iHash = value % HASH_MAX;
      pRoom->next = room_hash[iHash];
      room_hash[iHash] = pRoom;
      ch->desc->pEdit = (void *) pRoom;
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      send_to_char ("Room created.\n\r", ch);
      pArea->room_count++;
      return;
    }

  if (!str_cmp (arg1, "name"))
    {
      if (arg2[0] == '\0')
	{
	  send_to_char ("Syntax: name [name]\n\r", ch);
	  return;
	}
      free_string (pRoom->name);
      pRoom->name = str_dup (verify_room_name (arg2));
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      send_to_char ("Name set.\n\r", ch);
      return;
    }

  if (!str_cmp (arg1, "fdesc") || !str_cmp (arg1, "fdescription"))
    {
      char *s;
      if (pRoom->description != NULL && pRoom->description[0] == 1)
	{
	  s = str_dup (decompress (pRoom->description));
	  free_string (pRoom->description);
	  pRoom->description = s;
	}
      fullscreen_editor (ch, &pRoom->description);
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      return;
    }

  if (!str_cmp (arg1, "desc") || !str_cmp (arg1, "description"))
    {
      char *s;
      if (pRoom->description != NULL && pRoom->description[0] == 1)
	{
	  s = str_dup (decompress (pRoom->description));
	  free_string (pRoom->description);
	  pRoom->description = s;
	}
      if (arg2[0] == '\0')
	{
	  string_append (ch, &pRoom->description);
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  return;
	}
      if (arg2[0] == '+')
	{
	  string_append (ch, &pRoom->description);
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  return;
	}
      send_to_char ("Syntax: desc - line edit\n\r", ch);
      send_to_char (" desc + - line append\n\r", ch);
      return;
    }

  if (!str_cmp (arg1, "format"))
    {
      char *s;
      if (pRoom->description != NULL && pRoom->description[0] == 1)
	{
	  s = str_dup (decompress (pRoom->description));
	  free_string (pRoom->description);
	  pRoom->description = s;
	}
      pRoom->description = format_string (pRoom->description);
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      send_to_char ("String formatted.\n\r", ch);
      return;
    }

  if (!str_cmp (arg1, "ocean"))
    {
      if (IS_SET (pRoom->room_flags_2, ROOM2_OCEAN_D))
	{
	  REMOVE_BIT (pRoom->room_flags_2, ROOM2_OCEAN_D);
	  send_to_char ("Toggled OFF - no longer an ocean descript.\n\r", ch);
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  return;
	}
      SET_BIT (pRoom->room_flags_2, ROOM2_OCEAN_D);
      send_to_char ("Toggled ON - this is now an ocean descript.\n\r", ch);
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      return;
    }


  if (!str_cmp (arg1, "fieldd"))
    {
      if (IS_SET (pRoom->room_flags_2, ROOM2_FIELD_D))
	{
	  REMOVE_BIT (pRoom->room_flags_2, ROOM2_FIELD_D);
	  send_to_char ("Toggled OFF - no longer a field descript.\n\r", ch);
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  return;
	}
      SET_BIT (pRoom->room_flags_2, ROOM2_FIELD_D);
      send_to_char ("Toggled ON - this is now a field descript.\n\r", ch);
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      return;
    }

  if (!str_cmp (arg1, "lforestd"))
    {
      if (IS_SET (pRoom->room_flags_2, ROOM2_LIGHTF_D))
	{
	  REMOVE_BIT (pRoom->room_flags_2, ROOM2_LIGHTF_D);
	  send_to_char
	    ("Toggled OFF - no longer a light forest descript.\n\r", ch);
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  return;
	}
      SET_BIT (pRoom->room_flags_2, ROOM2_LIGHTF_D);
      send_to_char ("Toggled ON - this is now a light forest descript.\n\r",
		    ch);
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      return;
    }



  if (!str_cmp (arg1, "forestd"))
    {
      if (IS_SET (pRoom->room_flags_2, ROOM2_FOREST_D))
	{
	  REMOVE_BIT (pRoom->room_flags_2, ROOM2_FOREST_D);
	  send_to_char ("Toggled OFF - no longer a forest descript.\n\r", ch);
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  return;
	}
      SET_BIT (pRoom->room_flags_2, ROOM2_FOREST_D);
      send_to_char ("Toggled ON - this is now a forest descript.\n\r", ch);
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      return;
    }

  if (!str_cmp (arg1, "rforestd"))
    {
      if (IS_SET (pRoom->room_flags_2, ROOM2_RFOREST_D))
	{
	  REMOVE_BIT (pRoom->room_flags_2, ROOM2_RFOREST_D);
	  send_to_char ("Toggled OFF - no longer a rainforest descript.\n\r",
			ch);
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  return;
	}
      SET_BIT (pRoom->room_flags_2, ROOM2_RFOREST_D);
      send_to_char ("Toggled ON - this is now a rainforest descript.\n\r",
		    ch);
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      return;
    }

  if (!str_prefix ("ancient", arg1))
    {
      if (IS_SET (pRoom->room_flags_2, ROOM2_ANCIENTROAD))
	{
	  REMOVE_BIT (pRoom->room_flags_2, ROOM2_ANCIENTROAD);
	  send_to_char
	    ("Toggled OFF - no longer an ancient road descript.\n\r", ch);
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  return;
	}
      SET_BIT (pRoom->room_flags_2, ROOM2_ANCIENTROAD);
      send_to_char ("Toggled ON - this is now an ancient road descript.\n\r",
		    ch);
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      return;
    }

  if (!str_prefix ("hilld", arg1))
    {
      if (IS_SET (pRoom->room_flags_2, ROOM2_HILL_D))
	{
	  REMOVE_BIT (pRoom->room_flags_2, ROOM2_HILL_D);
	  send_to_char ("Toggled OFF - no longer a hill descript.\n\r", ch);
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  return;
	}
      SET_BIT (pRoom->room_flags_2, ROOM2_HILL_D);
      send_to_char ("Toggled ON - this is now a hill descript.\n\r", ch);
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      return;
    }



  if (!str_prefix ("earthcave", arg1))
    {
      if (IS_SET (pRoom->room_flags_2, ROOM2_EARTHCAVE_D))
	{
	  REMOVE_BIT (pRoom->room_flags_2, ROOM2_EARTHCAVE_D);
	  send_to_char ("Toggled OFF - no longer an earthcave descript.\n\r",
			ch);
	  SET_BIT (pArea->area_flags, AREA_CHANGED);
	  return;
	}
      SET_BIT (pRoom->room_flags_2, ROOM2_EARTHCAVE_D);
      send_to_char ("Toggled ON - this is now an earthcave descript.\n\r",
		    ch);
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      return;
    }

  if (!str_cmp (arg1, "light"))
    {
      if (arg2[0] == '\0' || !is_number (arg2))
	{
	  send_to_char ("Syntax: light [number]\n\r", ch);
	  return;
	}
      value = atoi (arg2);
      pRoom->light = value;
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      send_to_char ("Light set.\n\r", ch);
      return;
    }
  if (hugebuf_o[0] != '\0')
    {
      page_to_char (hugebuf_o, ch);
      return;
    }
  interpret (ch, arg);
  return;
}
