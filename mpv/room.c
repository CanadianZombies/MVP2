#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"

#ifndef WINDOWS
#include <sys/time.h>
#endif

void
room_to_room (ROOM_DATA * vehicle, ROOM_DATA * room, bool boat)
{
  CHAR_DATA *c;
  if (!vehicle->more)
    return;
  if (!IS_SET (vehicle->room_flags, ROOM_ISVEHICLE))
    return;
  if (IS_SET (room->room_flags, ROOM_ISVEHICLE))
    return;
  if (vehicle->linkage)
    {
      fprintf (stderr, "Error.. room_to_room.. vehicle has non-null linkage!\n");
      return;
    }
  vehicle->linkage = room->linkage;
  room->linkage = vehicle;
  vehicle->more->linked_to = room;
  if (!boat)
    SET_BIT (vehicle->area->area_flags, AREA_CHANGED);
  vehicle->x = room->x;
  vehicle->y = room->y;
  vehicle->z = room->z;
  return;
}

void
room_from_room (ROOM_DATA * vehicle)
{
  ROOM_DATA *r;
  ROOM_DATA *sr;
  if (!vehicle->more)
    return;
  if (!vehicle->more->linked_to)
    return;
  r = vehicle->more->linked_to;
  for (sr = r; sr != NULL; sr = sr->linkage)
    {
      if (sr->linkage == vehicle)
	{
	  sr->linkage = vehicle->linkage;
	  vehicle->linkage = NULL;
	  break;
	}
    }
  vehicle->more->linked_to = NULL;
  if (vehicle->linkage)
    {
      fprintf (stderr, "Vehicle room not found in correct room.\n");
      return;
    }
  return;
}

void
move_vehicles (CHAR_DATA * ch, char *argy)
{
  int hash;
  ROOM_DATA *r;
  ROOM_DATA *to;
  DEFINE_COMMAND ("vehicle_reset", move_vehicles, POSITION_DEAD, 110, LOG_ALWAYS, "This command resets all vehicle locations.") 
  for (hash = 0; hash < HASH_MAX; hash++)
    {
      for (r = room_hash[hash]; r != NULL; r = r->next)
	{
	  if (r->more && r->more->linked_to && r->more->orig_room != 0 &&
	      (to = get_room_index (r->more->orig_room)) != NULL)
	    {
	      room_from_room (r);
	      room_to_room (r, to, TRUE);
	    }
	}
    }
  send_to_char ("Vehicle locations reset.\n\r", ch);
  return;
}

void
z_areachange (CHAR_DATA * ch, char *argy)
{
  char arg1[100];
  char arg2[100];
  int start_vnum = -1;
  int end_vnum = -1;
  int new_area_num = -1;
  int i;
  ROOM_DATA *room;
  AREA_DATA *a;
  AREA_DATA *destination_area = NULL;
  DEFINE_COMMAND ("z_areachange", z_areachange, POSITION_DEAD, 110, LOG_ALWAYS, "This command moves vnums into a new area, without changing the vnums.")
    argy = one_argy (argy, arg1);
  argy = one_argy (argy, arg2);
  start_vnum = atoi (arg1);
  end_vnum = atoi (arg2);
  new_area_num = atoi (argy);
  if (start_vnum <= 0 || end_vnum <= 0 || start_vnum > end_vnum ||
      new_area_num < 0)
    {
      send_to_char ("Invalid.  Syntax: z_areachange <min vnum> <max vnum> <new area>.\n\r", ch);
      return;
    }
  for (a = area_first; a != NULL; a = a->next)
    {
      if (a->vnum == new_area_num)
	{
	  destination_area = a;
	  break;
	}
    }
  if (!destination_area)
    {
      send_to_char ("Destination area vnum not found.\n\r", ch);
      return;
    }

/* Purge the world of mobs */
  for (i = 0; i < HASH_MAX; i++)
    {
      for (room = room_hash[i]; room != NULL; room = room->next)
	{
	  raw_purge (room);
	}
    }

  for (i = start_vnum; i < end_vnum; i++)
    {
      if ((room = get_room_index (i)) == NULL)
	continue;
      room->area = destination_area;
    }
  reset_world ();
  send_to_char ("Operation complete.  Please asave complete at this time.\n\r", ch);
  return;
}

ROOM_DATA *
find_coord (short x, short y, char z)
{
  ROOM_DATA *room;
  int hash;
  for (hash = 0; hash < HASH_MAX; hash++)
    {
      for (room = room_hash[hash]; room != NULL; room = room->next)
	{
	  if (room->x == x && room->y == y && room->z == z)
	    return room;
	}
    }
  return NULL;
}

void
check_room_more (ROOM_DATA * room)
{
  ROOM_MORE *mor;
  if (!room)
    return;
  if (room->more)
    return;
  mor = mem_alloc (sizeof (*mor));
  bzero (mor, sizeof (*mor));
  mor->people = NULL;
  mor->contents = NULL;
  mor->copper = 0;
  mor->extra_descr = NULL;
  mor->move_dir = 0;
  mor->gold = 0;
  mor->move_message = &str_empty[0];
  mor->pcs = 0;
  mor->obj_description = NULL;
  room->more = mor;
  return;
}

void
check_clear_more (ROOM_DATA * room)
{
  ROOM_MORE *mor = room->more;
  if (!mor)
    return;
  if (!mor->move_message && !mor->linked_to && !mor->obj_description
      && mor->move_dir == 0 && !mor->reset_first && !mor->reset_last
      && !mor->people && !mor->contents && !mor->gold && !mor->copper
      && !mor->pcs)
    {
      free_m (mor);
      room->more = NULL;
    }
  return;
}

ROOM_DATA *
new_room (void)
{
  ROOM_DATA *oneroom;
  short door;

  oneroom = mem_alloc (sizeof (*oneroom));
  oneroom->more = NULL;
  oneroom->data_type = K_ROOM;
  oneroom->img[0] = '\0';
  oneroom->img[1] = '\0';
  oneroom->room_flags = 0;
  oneroom->room_flags_2 = 0;
  oneroom->light = 0;
  oneroom->sector_type = 1;
  oneroom->name = &str_empty[0];
  oneroom->description = &str_empty[0];
  oneroom->shade = FALSE;
  for (door = 0; door <= 5; door++)
    oneroom->exit[door] = NULL;
  oneroom->tracks = NULL;


  return oneroom;
}
