#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"

void
allocate_correct_datatype (SINGLE_OBJECT * obj, int *val)
{
  OBJ_PROTOTYPE *oi = obj->pIndexData;

  if (oi == NULL)
    {
      fprintf (stderr, "Error with null prototype in allocating object datatype.\n");
      exit (99);
    }
  if (oi->item_type == ITEM_REPELLANT)
    {
      I_SPRAY *data;
      int i;
      data = mem_alloc (sizeof (*data));
      data->tag = oi->item_type;
      data->sprays = val[0];
      for (i = 1; i < 10; i++)
	data->repels[i - 1] = val[i];
      obj->more = data;
      return;
    }
  if (oi->item_type == ITEM_WEAPON)
    {
      I_WEAPON *data;
      data = mem_alloc (sizeof (*data));
      data->tag = oi->item_type;
      data->firstdice = val[7];
      data->seconddice = val[8];
      data->attack_type = val[3];
      data->damage_p = (val[0] == 0 ? 100 : val[0]);
      data->strength = val[1];
      obj->more = data;
      return;
    }
  if (oi->item_type == ITEM_ARMOR)
    {
      I_ARMOR *data;
      data = mem_alloc (sizeof (*data));
      data->tag = oi->item_type;
      data->warmth = val[9];
      data->protects_legs = val[0];
      data->protects_body = val[1];
      data->protects_head = val[2];
      data->protects_arms = val[3];
      data->pound_rating = val[4];
      data->slash_rating = val[5];
      data->pierce_rating = val[8];
      data->max_condition = val[6];
      data->condition_now = val[7];
      obj->more = data;
      return;
    }
  if (oi->item_type == ITEM_DRINK_CON)
    {
      I_DRINK *data;
      data = mem_alloc (sizeof (*data));
      data->tag = oi->item_type;
      data->max_liquid = val[1];
      data->liquid_now = val[0];
      data->liquid_type = val[2];
      data->not_poison = val[3];
      obj->more = data;
      return;
    }
  if (oi->item_type == ITEM_LIGHT)
    {
      I_LIGHT *data;
      data = mem_alloc (sizeof (*data));
      data->tag = oi->item_type;
      data->light_fillable = val[2];
      data->light_lit = val[3];
      data->light_now = val[0];
      data->max_light = val[1];
      obj->more = data;
      return;
    }
  if (oi->item_type == ITEM_TOOL)
    {
      I_TOOL *data;
      data = mem_alloc (sizeof (*data));
      data->tag = oi->item_type;
      data->tool_type = val[0];
      data->uses = val[1];
      data->max_uses = val[2];
      obj->more = data;
      return;
    }
  if (oi->item_type == ITEM_VEHICLE)
    {
      I_VEHICLE *data;
      data = mem_alloc (sizeof (*data));
      data->tag = oi->item_type;
      data->sector_use = val[0];
      obj->more = data;
      return;
    }
  if (oi->item_type == ITEM_FOUNTAIN)
    {
      I_FOUNTAIN *data;
      data = mem_alloc (sizeof (*data));
      data->tag = oi->item_type;
      data->liquid = val[0];
      data->spells[0] = val[1];
      data->spells[1] = val[2];
      data->spells[2] = val[3];
      obj->more = data;
      return;
    }
  if (oi->item_type == ITEM_POTION || oi->item_type == ITEM_PILL
      || oi->item_type == ITEM_SCROLL)
    {
      I_POTION *data;
      data = mem_alloc (sizeof (*data));
      data->tag = oi->item_type;
      data->spell_level = val[0];
      data->spells[0] = val[1];
      data->spells[1] = val[2];
      data->spells[2] = val[3];
      obj->more = data;
      return;
    }
  if (oi->item_type == ITEM_FOOD)
    {
      I_FOOD *data;
      data = mem_alloc (sizeof (*data));
      data->tag = oi->item_type;
      data->hours = val[0];
      data->not_poison = val[3];
      obj->more = data;
      return;
    }
  if (oi->item_type == ITEM_GEM)
    {
      I_GEM *data;
      data = mem_alloc (sizeof (*data));
      data->tag = oi->item_type;
      data->gem_type = val[0];
      data->mana_now = val[1];
      data->max_mana = val[2];
      data->max_level = val[3];
      obj->more = data;
      return;
    }
  if (oi->item_type == ITEM_WAND || oi->item_type == ITEM_STAFF)
    {
      I_WAND *data;
      data = mem_alloc (sizeof (*data));
      data->tag = oi->item_type;
      data->spell_level = val[0];
      data->max_charges = val[1];
      data->current_charges = val[2];
      data->spells[0] = val[3];
      data->spells[1] = val[4];
      data->spells[2] = val[5];
      obj->more = data;
      return;
    }
  if (oi->item_type == ITEM_FURNITURE)
    {
      I_FURNITURE *data;
      data = mem_alloc (sizeof (*data));
      data->tag = oi->item_type;
      data->to_room_vnum = val[1];
      data->type = val[0];
      data->strip_possessions = val[5];
      obj->more = data;
      return;
    }
  if (oi->item_type == ITEM_CONTAINER || oi->item_type == ITEM_CORPSE_PC ||
      oi->item_type == ITEM_CORPSE_NPC)
    {
      I_CONTAINER *data;
      data = mem_alloc (sizeof (*data));
      data->tag = oi->item_type;
      data->max_weight = val[0];
      data->flags = val[1];
      data->key_vnum = val[2];
      data->money = val[8];
      data->reason_for_death[0] = '\0';
      data->looted_by[0] = '\0';
      data->looted_by_two[0] = '\0';
      obj->more = data;
      return;
    }
  if (oi->item_type == ITEM_CLIMBABLE)
    {
      I_CLIMBABLE *data;
      data = mem_alloc (sizeof (*data));
      data->tag = oi->item_type;
      data->up_down = val[0];
      data->above = val[2];
      data->below = val[3];
      obj->more = data;
      return;
    }
  if (oi->vnum == OBJ_VNUM_BLOOD)
    {
      I_BLOOD *data;
      data = mem_alloc (sizeof (*data));
      data->tag = oi->item_type;
      data->door = 0;
      obj->more = data;
      return;
    }
  if (oi->item_type == ITEM_BOOK)
    {
      I_BOOK *data;
      data = mem_alloc (sizeof (*data));
      data->tag = oi->item_type;
      data->teaches = val[0];
      obj->more = data;
      return;
    }
  obj->more = NULL;
  return;
}




SINGLE_OBJECT *
new_obj (void)
{
  SINGLE_OBJECT *obj;
  if (obj_free == NULL)
    {
      obj = mem_alloc (sizeof (*obj));
      obj->data_type = K_OBJ;
    }
  else
    {
      obj = obj_free;
      obj_free = obj_free->next;
      obj->data_type = K_OBJ;
    }
  clear_data (obj);
  return obj;
}



void
obj_to (SINGLE_OBJECT * obj, void *thing)
{
  ROOM_DATA *in_room;
  SINGLE_OBJECT *obj_t;
  CHAR_DATA *ch;
  char *t = thing;
  if (!obj || !thing)
    return;
  if (*t == K_OBJ)
    {
      obj_t = (SINGLE_OBJECT *) thing;
      if (obj->carried_by && obj->wear_loc != -1)
	unequip_char (obj->carried_by, obj);
      obj->next_content = obj_t->contains;
      obj_t->contains = obj;
      obj->in_obj = obj_t;
      obj->in_room = NULL;
      obj->carried_by = NULL;
      obj->wear_loc = WEAR_NONE;
      for (; obj_t != NULL; obj_t = obj_t->in_obj)
	{
	  if (obj_t->carried_by != NULL && IS_PLAYER (obj_t->carried_by))
	    {
	      obj_t->carried_by->pcdata->carry_number += get_obj_number (obj);
	      obj_t->carried_by->pcdata->carry_weight += get_obj_weight (obj);
	    }
	}
    }
  else if (*t == K_ROOM)
    {
      in_room = (ROOM_DATA *) thing;
      check_room_more (in_room);

      if (obj->pIndexData->item_type == ITEM_LIGHT && IS_LIT (obj))
	++in_room->light;
      obj->next_content = in_room->more->contents;
      in_room->more->contents = obj;
      obj->in_room = in_room;
      obj->carried_by = NULL;
      obj->in_obj = NULL;
      obj->wear_loc = WEAR_NONE;
      if (obj->in_room->more && obj->in_room->more->pcs > 0)
	{
	  CHAR_DATA *tch;
	  for (tch = obj->in_room->more->people; tch; tch = tch->next_in_room)
	    {
	      if (IS_JAVA (tch))
		{
		  java_add_obj_window (tch, obj);
		}
	    }
	}
      return;
    }
  else if (*t == K_CHAR)
    {
      ch = (CHAR_DATA *) thing;
      if (IS_PLAYER (ch) && obj->pIndexData->how_many > 0 && got)
	check_write_limited (obj, obj->pIndexData->vnum);
      obj->next_content = ch->carrying;
      ch->carrying = obj;
      obj->carried_by = ch;
      obj->in_room = NULL;
      obj->in_obj = NULL;
      if (IS_PLAYER (ch))
	{
	  ch->pcdata->carry_number += get_obj_number (obj);
	  ch->pcdata->carry_weight += get_obj_weight (obj);
	}
      if (IS_JAVA (ch))
	{
	  java_add_obj_inv_window (ch, obj);
	}
    }
  return;
}



void
obj_from (SINGLE_OBJECT * obj)
{
  CHAR_DATA *ch;
  ROOM_DATA *in_room;
  SINGLE_OBJECT *obj_frm;
  ch = obj->carried_by;
  in_room = obj->in_room;
  obj_frm = obj->in_obj;
  if (ch != NULL)
    {
      if (!quit_flag && obj->pIndexData->vnum < MAX_LIMITED && IS_PLAYER (ch)
	  && obj->pIndexData->how_many > 0)
	{
	  if (limited.limited_array[obj->pIndexData->vnum] > 0)
	    {
	      limited.limited_array[obj->pIndexData->vnum]--;
	      fprintf (stderr, "Reduced limited on %s to %d.\n",
		       obj->pIndexData->short_descr,
		       limited.limited_array[obj->pIndexData->vnum]);
	      save_limited (obj->pIndexData->vnum);
	    }
	}
      if (obj->wear_loc != WEAR_NONE)
	unequip_char (ch, obj);
      if (ch->carrying == obj)
	{
	  ch->carrying = obj->next_content;
	}
      else
	{
	  SINGLE_OBJECT *prev;
	  for (prev = ch->carrying; prev != NULL;
	       prev = ((!prev->next_content) ? NULL : prev->next_content))
	    if (prev->next_content == obj)
	      {
		prev->next_content = obj->next_content;
		break;
	      }
	}
      obj->carried_by = NULL;
      obj->next_content = NULL;
      ch->pcdata->carry_number -= get_obj_number (obj);
      ch->pcdata->carry_weight -= get_obj_weight (obj);
      if (IS_JAVA (ch))
	{
	  java_remove_obj_inv (ch, obj);
	}
    }
  else if (in_room != NULL)
    {
      if (obj == in_room->more->contents)
	{
	  in_room->more->contents = obj->next_content;
	}
      else
	{
	  SINGLE_OBJECT *prev;
	  for (prev = in_room->more->contents; prev;
	       prev = prev->next_content)
	    if (prev->next_content == obj)
	      {
		prev->next_content = obj->next_content;
		break;
	      }
	}
      if (obj->pIndexData->item_type == ITEM_LIGHT && IS_LIT (obj))
	--obj->in_room->light;
      if (obj->pIndexData->item_type == ITEM_BOAT && obj->boat)
	{
	  BLIST *bt;
	  BLIST *bt_n;
	  if (base_blist && base_blist->vnum == obj->in_room->vnum)
	    {
	      bt = base_blist;
	      base_blist = base_blist->next;
	      save_boats ();
	      free_m (bt);
	    }
	  else
	    for (bt = base_blist; bt != NULL; bt = bt_n)
	      {
		bt_n = bt->next;
		if (bt_n && bt_n->vnum == obj->in_room->vnum)
		  {
		    bt->next = bt_n->next;
		    free_m (bt_n);
		    save_boats ();
		    goto ttj;
		  }
	      }
	}
    ttj:
      if (obj->in_room->more && obj->in_room->more->pcs > 0)
	{
	  CHAR_DATA *t;
	  for (t = obj->in_room->more->people; t; t = t->next_in_room)
	    {
	      if (IS_JAVA (t))
		{
		  java_remove_obj_stuff (t, obj);
		}
	    }
	}
      check_clear_more (obj->in_room);
      obj->in_room = NULL;
      obj->next_content = NULL;
    }
  else if (obj_frm != NULL)
    {
      if (obj == obj_frm->contains)
	{
	  obj_frm->contains = obj->next_content;
	}
      else
	{
	  SINGLE_OBJECT *prev;
	  for (prev = obj_frm->contains; prev; prev = prev->next_content)
	    if (prev->next_content == obj)
	      {
		prev->next_content = obj->next_content;
		break;
	      }
	}
      obj->next_content = NULL;
      obj->in_obj = NULL;
      for (; obj_frm != NULL; obj_frm = obj_frm->in_obj)
	if (obj_frm->carried_by != NULL && IS_PLAYER (obj_frm->carried_by))
	  {
	    obj_frm->carried_by->pcdata->carry_number -= get_obj_number (obj);
	    obj_frm->carried_by->pcdata->carry_weight -= get_obj_weight (obj);
	  }
    }
  return;
}
