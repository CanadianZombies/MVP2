#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"

/*destroy_list destroys the linked list of gen_next */
void
destroy_list (CHAR_DATA * ch)
{
  CHAR_DATA *nxt;
  CHAR_DATA *current;
  for (current = ch; current != NULL; current = nxt)
    {
      nxt = current->gen_next;
      current->gen_next = NULL;
    }
  return;
}

int been_to_room[501];
void
clear_been_to_room (void)
{
  int i;
  for (i = 0; i < 501; i++)
    been_to_room[i] = 0;
  return;
}

bool
been_here (int vnum)
{
  int i;
  for (i = 0; i < 501; i++)
    {
      if (been_to_room[i] == vnum)
	return TRUE;
    }
  return FALSE;
}

void
add_room_to_list (int vnum)
{
  int i;
  for (i = 0; i < 501; i++)
    if (been_to_room[i] == 0)
      {
	been_to_room[i] = vnum;
	break;
      }
  return;
}

int
scan_room_for_people (CHAR_DATA * ch, ROOM_DATA * rid)
{
  CHAR_DATA *blah;
  int ii;
  CHAR_DATA *cur = NULL;
  if (been_here (rid->vnum))
    return -1;
  ii = 0;
  for (blah = ch; blah != NULL; blah = blah->gen_next)
    {
      ii++;
      if (blah->gen_next == NULL)
	{
	  cur = blah;
	  break;
	}
      if (ii > 70)
	{
	  fprintf (stderr, "Ch: %s in %d. Rm: %d\n", NAME (ch),
		   ch->in_room->vnum, rid->vnum);
	  ch->gen_next = NULL;
	  return -1;
	}
    }
  if (cur == NULL)
    return 0;
  if (rid->more)
    for (blah = rid->more->people; blah != NULL; blah = blah->next_in_room)
      {
	if (blah == ch)
	  continue;
	if (blah == cur)
	  continue;
	cur->gen_next = blah;
	if (blah->gen_next)
	  blah->gen_next = NULL;
	cur = blah;
      }
  return 0;
}

void
do_it (CHAR_DATA * ch, ROOM_DATA * rid, int strength, short came_from)
{
  int ctr = 0;
  if (strength < 1)
    return;
  if (IS_SET (rid->room_flags, ROOM_NOHEAR))
    return;
  if (been_here (rid->vnum))
    return;
  if (scan_room_for_people (ch, rid) < 0)
    {
      return;
    }
  add_room_to_list (rid->vnum);
  if (rid->exit[DIR_EAST] && came_from != DIR_WEST
      && rid->exit[DIR_EAST]->to_room)
    {
      if (rid->exit[DIR_EAST]->d_info
	  && IS_SET (rid->exit[DIR_EAST]->d_info->exit_info, EX_CLOSED))
	ctr++;
      if (IS_SET (rid->room_flags, ROOM_NOISY))
	ctr++;
      if (rid->exit[DIR_EAST]->d_info
	  && IS_SET (rid->exit[DIR_EAST]->d_info->exit_info, EX_HIDDEN))
	ctr = 9999;
      do_it (ch, rid->exit[DIR_EAST]->to_room, (strength - ctr - 1),
	     DIR_EAST);
    }
  if (rid->exit[DIR_WEST] && came_from != DIR_EAST
      && rid->exit[DIR_WEST]->to_room)
    {
      if (rid->exit[DIR_WEST]->d_info
	  && IS_SET (rid->exit[DIR_WEST]->d_info->exit_info, EX_CLOSED))
	ctr++;
      if (IS_SET (rid->room_flags, ROOM_NOISY))
	ctr++;
      if (rid->exit[DIR_WEST]->d_info
	  && IS_SET (rid->exit[DIR_WEST]->d_info->exit_info, EX_HIDDEN))
	ctr = 9999;
      do_it (ch, rid->exit[DIR_WEST]->to_room, (strength - ctr - 1),
	     DIR_WEST);
    }
  if (rid->exit[DIR_NORTH] && came_from != DIR_SOUTH
      && rid->exit[DIR_NORTH]->to_room)
    {
      if (rid->exit[DIR_NORTH]->d_info
	  && IS_SET (rid->exit[DIR_NORTH]->d_info->exit_info, EX_CLOSED))
	ctr++;
      if (IS_SET (rid->room_flags, ROOM_NOISY))
	ctr++;
      if (rid->exit[DIR_NORTH]->d_info
	  && IS_SET (rid->exit[DIR_NORTH]->d_info->exit_info, EX_HIDDEN))
	ctr = 9999;
      do_it (ch, rid->exit[DIR_NORTH]->to_room, (strength - ctr - 1),
	     DIR_EAST);
    }
  if (rid->exit[DIR_SOUTH] && came_from != DIR_NORTH
      && rid->exit[DIR_SOUTH]->to_room)
    {
      if (rid->exit[DIR_SOUTH]->d_info
	  && IS_SET (rid->exit[DIR_SOUTH]->d_info->exit_info, EX_CLOSED))
	ctr++;
      if (IS_SET (rid->room_flags, ROOM_NOISY))
	ctr++;
      if (rid->exit[DIR_SOUTH]->d_info
	  && IS_SET (rid->exit[DIR_SOUTH]->d_info->exit_info, EX_HIDDEN))
	ctr = 9999;
      do_it (ch, rid->exit[DIR_SOUTH]->to_room, (strength - ctr - 1),
	     DIR_EAST);
    }
  return;
}

void
find_people_n (CHAR_DATA * ch, int n)
{
  if (!ch->in_room)
    {
      return;
    }
  clear_been_to_room ();
  destroy_list (ch);
  if (n > 15)
    {
      send_to_char ("Error in find_people_n: Max depth is 15.\n\r", ch);
      return;
    }
  do_it (ch, ch->in_room, n, 9999);
  return;
}
