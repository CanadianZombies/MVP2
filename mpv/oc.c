#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"

AREA_DATA *area_free;
DESCRIPTION_DATA *extra_descr_free;
ROOM_DATA *room_index_free;
OBJ_PROTOTYPE *obj_index_free;
SHOP_DATA *shop_free;
MOB_PROTOTYPE *mob_index_free;
RESET_DATA *reset_free;

int
find_next_obj (int cur)
{
  int i;
  i = cur + 1;
  while (i < top_vnum_obj && get_obj_index (i) == NULL)
    i++;
  if (i == top_obj_index)
    return 1;
  return i;
}

int
find_next_mob (int cur)
{
  int i;
  i = cur + 1;
  while (i < top_vnum_mob && get_mob_index (i) == NULL)
    i++;
  if (i == top_mob_index)
    return 1;
  return i;
}

int
find_prev_obj (int cur)
{
  int i;
  i = cur - 1;
  while (i > 1 && get_obj_index (i) == NULL)
    i--;
  if (i == 1)
    return 1;
  return i;
}

int
find_prev_mob (int cur)
{
  int i;
  i = cur - 1;
  while (i > 1 && get_mob_index (i) == NULL)
    i--;
  if (i == 1)
    return 1;
  return i;
}


char *
strip_enters (char *s)
{
  char *t;
  int i;
  static char b[500];
  strcpy (b, s);
  i = strlen (b);
  t = b + i;
  while (*t != '\n' && t > b)
  {
    t--;
  }
  *t = '\0';
  return b;
}

char *
javaspecfun (MOB_PROTOTYPE * mob)
{
  static char ss[50];
  strcpy (ss, "None");
  if (!mob->spec_name)
    return ss;
  if (!str_cmp (mob->spec_name, "spec_warrior"))
    strcpy (ss, "Warrior");
  if (!str_cmp (mob->spec_name, "spec_thief"))
    strcpy (ss, "Thief");
  if (!str_cmp (mob->spec_name, "spec_poison"))
    strcpy (ss, "Poison");
  if (!str_cmp (mob->spec_name, "spec_eatcorpse"))
    strcpy (ss, "EatCorpse");
  if (!str_cmp (mob->spec_name, "spec_janitor"))
    strcpy (ss, "Janitor");
  if (!str_cmp (mob->spec_name, "spec_thief"))
    strcpy (ss, "Thief");
  if (!str_cmp (mob->spec_name, "spec_barbarian"))
    strcpy (ss, "Barbarian");
  return ss;
}

void
java_oedit (CHAR_DATA * ch, OBJ_PROTOTYPE * obj)
{
  char buf[5000];
  char b2[500];
  int i;
  sprintf (buf, "%c%c%c%d\n", (char) 19, (char) 29, (char) 1, obj->vnum);
  strcpy (b2, strip_enters (obj->description));

  sprintf (buf + strlen (buf), "%c%s\n%c%s\n",
      (char) 2, obj->name, (char) 3, obj->short_descr);
  sprintf (buf + strlen (buf), "%c%s\n%c%d\n",
      (char) 4, b2, (char) 5, obj->cost);
  sprintf (buf + strlen (buf), "%c%d\n%c%d\n",
      (char) 6, obj->weight, (char) 7, obj->how_many);
  sprintf (buf + strlen (buf), "%c%d\n", (char) 8, obj->item_type);
  sprintf (buf + strlen (buf), "%c%d\n", (char) 10, obj->value[0]);
  sprintf (buf + strlen (buf), "%c%d\n", (char) 11, obj->value[1]);
  sprintf (buf + strlen (buf), "%c%d\n", (char) 12, obj->value[2]);
  sprintf (buf + strlen (buf), "%c%d\n", (char) 20, obj->value[3]);
  sprintf (buf + strlen (buf), "%c%d\n", (char) 14, obj->value[4]);
  sprintf (buf + strlen (buf), "%c%d\n", (char) 15, obj->value[5]);
  sprintf (buf + strlen (buf), "%c%d\n", (char) 16, obj->value[6]);
  sprintf (buf + strlen (buf), "%c%d\n", (char) 17, obj->value[7]);
  sprintf (buf + strlen (buf), "%c%d\n", (char) 18, obj->value[8]);
  sprintf (buf + strlen (buf), "%c%d\n", (char) 19, obj->value[9]);
  sprintf (buf + strlen (buf), "%c", (char) 200);
  sprintf (buf + strlen (buf), "%c%c%d\n", (char) 19, (char) 30,
      (((unsigned int) obj->java[0]) * 256) +
      (int) ((unsigned char) obj->java[1]));
  write_to_descriptor2 (ch->desc, buf, 0);
  return;
}



void
java_medit (CHAR_DATA * ch, MOB_PROTOTYPE * mob)
{
  char buf[5000];
  char b2[500];
  int i;
  sprintf (buf, "%c%c%c%d\n", (char) 19, (char) 26, (char) 1, mob->vnum);
  strcpy (b2, strip_enters (mob->long_descr));
  sprintf (buf + strlen (buf), "%c%s\n%c%s\n%c%s\n",
      (char) 6, mob->name, (char) 7, b2, (char) 17, mob->short_descr);
  sprintf (buf + strlen (buf), "%c%s\n",
      (char) 5,
      (mob->sex ==
       0 ? "Neuter" : (mob->sex == SEX_MALE ? "Male" : "Female")));
  sprintf (buf + strlen (buf), "%c%d\n", (char) 9, mob->yeller_number);
  sprintf (buf + strlen (buf), "%c%d\n", (char) 10, mob->hpmult % 1000);
  sprintf (buf + strlen (buf), "%c%d\n", (char) 11, mob->hpmult / 1000);
  sprintf (buf + strlen (buf), "%c%d\n", (char) 12, mob->level);
  sprintf (buf + strlen (buf), "%c%d\n", (char) 15, mob->will_help);
  sprintf (buf + strlen (buf), "%c%d\n", (char) 16, mob->money);
  sprintf (buf + strlen (buf), "%c%d\n", (char) 14, mob->guard);
  sprintf (buf + strlen (buf), "%c%d\n", (char) 18, mob->clan_guard_1);
  sprintf (buf + strlen (buf), "%c%d\n", (char) 19, mob->clan_guard_2);
  sprintf (buf + strlen (buf), "%c%d\n", (char) 21, mob->parry);
  sprintf (buf + strlen (buf), "%c%d\n", (char) 22, mob->hitroll);
  sprintf (buf + strlen (buf), "%c%d\n", (char) 23, mob->damroll);
  sprintf (buf + strlen (buf), "%c%d\n", (char) 24, mob->dodge);
  sprintf (buf + strlen (buf), "%c%d\n", (char) 25, mob->max_in_world);

  sprintf (buf + strlen (buf), "%c%s\n", (char) 3, javaspecfun (mob));
  if (mob->mobtype >= 0)
  {
    sprintf (buf + strlen (buf), "%c%s\n",
        (char) 4, capitalize (mob_type[mob->mobtype]));
  }
  i = 0;
  while (str_cmp (affect_flagss[i].what_you_type, "end_of_list"))
  {
    if (IS_SET (mob->affected_by, affect_flagss[i].flagname))
    {
      sprintf (buf + strlen (buf), "%c%s\n",
          (char) 2, affect_flagss[i].what_you_type);
    }
    i++;
  }
  i = 0;
  while (str_cmp (act_1[i].what_you_type, "end_of_list"))
  {
    if (IS_SET (mob->act, act_1[i].flagname))
    {
      sprintf (buf + strlen (buf), "%c%s\n",
          (char) 2, act_1[i].what_you_type);
    }
    i++;
  }
  i = 0;
  while (str_cmp (act_3[i].what_you_type, "end_of_list"))
  {
    if (IS_SET (mob->act3, act_3[i].flagname))
    {
      sprintf (buf + strlen (buf), "%c%s\n",
          (char) 2, act_3[i].what_you_type);
    }
    i++;
  }
  i = 0;
  while (str_cmp (act_4[i].what_you_type, "end_of_list"))
  {
    if (IS_SET (mob->act4, act_4[i].flagname))
    {
      sprintf (buf + strlen (buf), "%c%s\n",
          (char) 2, act_4[i].what_you_type);
    }
    i++;
  }

  sprintf (buf + strlen (buf), "%c", (char) 200);
  sprintf (buf + strlen (buf), "%c%c%d\n", (char) 19, (char) 28,
      (((unsigned int) mob->java[0]) * 256) +
      (int) ((unsigned char) mob->java[1]));
  write_to_descriptor2 (ch->desc, buf, 0);
  return;
}

void
send_pic_to_medit (CHAR_DATA * ch, MOB_PROTOTYPE * mob)
{
  char buf[500];
  buf[0] = '\0';
  sprintf (buf + strlen (buf), "%c%c%d\n", (char) 19, (char) 28,
      (((unsigned int) mob->java[0]) * 256) +
      (int) ((unsigned char) mob->java[1]));
  write_to_descriptor2 (ch->desc, buf, 0);
  return;
}

void
send_pic_to_oedit (CHAR_DATA * ch, OBJ_PROTOTYPE * obj)
{
  char buf[500];
  buf[0] = '\0';
  sprintf (buf + strlen (buf), "%c%c%d\n", (char) 19, (char) 30,
      (((unsigned int) obj->java[0]) * 256) +
      (int) ((unsigned char) obj->java[1]));
  write_to_descriptor2 (ch->desc, buf, 0);
  return;
}



void
send_edit_char (char *st, CHAR_DATA * ch)
{
  char *t;
  int i = 0;
  hugebuf_o[0] = '\0';
  for (t = st; *t != '\0'; t++)
  {
    if (*t == ':' && *(t + 1) >= '0' && *(t + 1) <= '9')
    {
      hugebuf_o[i] = '\n';
      i++;
      hugebuf_o[i] = '\r';
      i++;
      hugebuf_o[i] = ':';
      i++;
      continue;
    }
    hugebuf_o[i] = *t;
    i++;
  }
  hugebuf_o[i] = '\0';
  send_to_char (hugebuf_o, ch);
  return;
}

MOB_PROTOTYPE *
new_mob_index (void)
{
  MOB_PROTOTYPE *pMob;
  if (mob_index_free == NULL)
  {
    pMob = mem_alloc (sizeof (*pMob));
    top_mob_index++;
  }
  else
  {
    pMob = mob_index_free;
    mob_index_free = mob_index_free->next;
  }
  pMob->next = NULL;
  pMob->spec_fun = NULL;
  pMob->clan_guard_1 = 0;
  pMob->clan_guard_2 = 0;
  pMob->spec_name = NULL;
  pMob->pShop = NULL;
  pMob->area = NULL;
  pMob->attackname = str_dup ("N/A");
  pMob->name = str_dup ("newmob");
  pMob->short_descr = str_dup ("(please set short description)");
  pMob->long_descr = str_dup ("(please set long description)\n\r");
  pMob->description = str_dup ("");
  pMob->script_fn = NULL;
  pMob->yeller_number = 0;
  pMob->max_in_world = 1;
  pMob->will_help = 0;
  pMob->vnum = 0;
  pMob->guard = 0;
  pMob->height = 0;
  pMob->triggers = NULL;
  pMob->trig_list = NULL;
  pMob->count = 0;
  pMob->killed = 0;
  pMob->sex = 1;
  pMob->level = 0;
  pMob->act = 0;
  pMob->act3 = 0;
  pMob->act4 = 0;
  pMob->affected_by = 0;
  pMob->more_affected_by = 0;
  pMob->alignment = 0;
  pMob->alt_vnum = 0;
  pMob->money = 0;
  pMob->mobtype = -1;
  pMob->hpmult = 10;
  pMob->opt = NULL;
  pMob->race_hate = 0;
  pMob->armcls = 101;
  pMob->hitroll = 0;
  pMob->damroll = 0;
  pMob->dodge = 0;
  pMob->parry = 0;
  return pMob;
}

PLAYERBASE_DATA *
new_player (void)
{
  PLAYERBASE_DATA *play;
  play = mem_alloc (sizeof (*play));
  play->player_name[0] = '\0';
  play->email_address[0] = '\0';
  play->next = playerbase_zero;
  playerbase_zero = play;
  return play;
}

void
free_temp (CHAR_DATA * ch)
{
  if (!ch->pcdata || !ch->pcdata->temp)
    return;
  ch->pcdata->temp->next = temp_free;
  temp_free = ch->pcdata->temp;
  ch->pcdata->temp = NULL;
  return;
}

void
check_temp (CHAR_DATA * ch)
{
  if (!ch || !ch->pcdata)
    return;
  if (ch->pcdata->temp == NULL)
    ch->pcdata->temp = new_temp ();
  return;
}

TEMP_STR *
new_temp (void)
{
  TEMP_STR *temp;
  if (temp_free == NULL)
  {
    temp = mem_alloc (sizeof (*temp));
    top_temp++;
  }
  else
  {
    temp = temp_free;
    temp_free = temp_free->next;
  }
  temp->next = temp_first;
  temp_first = temp;
  temp->temp_string[0] = '\0';
  return temp;
}

void
do_setroom (CHAR_DATA * ch, char *argy)
{
#ifdef NEW_WORLD
  char arg1[500];
  char arg2[500];
  DEFINE_COMMAND ("superroom", do_setroom, POSITION_DEAD, 110, LOG_ALWAYS,
      "Superroom feature for Kilith only.") 
      if (str_cmp (NAME (ch), "Sabelis")
      && str_cmp (NAME (ch), "Kilith")) return;
      if (argy == NULL || argy == "" || argy[0] == '\0')
      {
        current_vnum = 0;
        ch->pcdata->arena_msg = 0;
        ch->pcdata->warpoints = 0;
        send_to_char ("Toggled OFF!\n\r", ch); return;
      }
      current_vnum = ch->in_room->vnum;
      if (ch->pcdata->short_descr)
        free_string (ch->pcdata->short_descr);
      if (ch->pcdata->long_descr)
        free_string (ch->pcdata->long_descr);
      if (ch->pcdata->ignore[9])
        free_string (ch->pcdata->ignore[9]);
      ch->pcdata->short_descr = &str_empty[0];
      ch->pcdata->long_descr = &str_empty[0];
      argy = one_argy (argy, arg1);
      argy = one_argy (argy, arg2);
      if (arg1[0] != '\0')
        ch->pcdata->short_descr = str_dup (arg1);
      if (arg2[0] != '\0')
        ch->pcdata->ignore[9] = str_dup (arg2);
      if (argy[0] != '\0')
        ch->pcdata->long_descr = str_dup (argy);
      ch->pcdata->warpoints = 5; ch->pcdata->arena_msg = 5;
#endif
      return;
}

void free_ced (CHAR_DATA * ch)
{
  CED_DATA * f;
  if (ch->ced == NULL) return; if (ced_first == ch->ced)
  {
    ced_first = ch->ced->next;
  }
  else
    for (f = ced_first; f != NULL; f = f->next)
    {
      if (f->next == ch->ced)
      {
        f->next = ch->ced->next; break;
      }
    }
  ch->ced->pnote = NULL;
  //ch->ced->poll = NULL;
  ch->ced->attackers[0] = NULL;
  ch->ced->attackers[1] = NULL;
  ch->ced->attackers[2] = NULL; ch->ced->attackers[3] = NULL;
#ifndef NEW_WORLD
  ch->ced->in_boat = NULL; ch->ced->came_from_boat = NULL;
#endif
  ch->ced->wasroom = 0;
  ch->ced->pnote = NULL;
  ch->ced->original = NULL;
  ch->ced->has_yelled = FALSE;
  ch->ced->scanning = FALSE;
  ch->ced->was_in_room = NULL;
  ch->ced->reply = NULL;
  ch->ced->fight_ops = 0xFFFFFFFF;
  ch->ced->ground_pos = 0;
  ch->ced->wimpy = 5;
  ch->ced->cur_line = 0;
  ch->ced->scr_wait = 0;
  ch->ced->wait_type = 0;
  ch->ced->next = ced_free;
  ced_free = ch->ced; ch->ced = NULL; return;
}

void check_ced (CHAR_DATA * ch)
{
  if (!ch)
    return; if (ch->ced == NULL) ch->ced = new_ced (); return;
}

CED_DATA * new_ced (void)
{
  CED_DATA * ced; if (ced_free == NULL)
  {
    ced = mem_alloc (sizeof (*ced)); top_ced++;
  }
  else
  {
    ced = ced_free; ced_free = ced_free->next;
  }
  ced->next = ced_first;
  ced_first = ced;
  ced->attackers[0] = NULL;
  ced->attackers[1] = NULL;
  ced->attackers[2] = NULL; ced->attackers[3] = NULL;
#ifndef NEW_WORLD
  ced->in_boat = NULL; ced->came_from_boat = NULL;
#endif
  ced->wasroom = 0;
  ced->pnote = NULL;
  //ced->poll = NULL;
  ced->original = NULL;
  ced->has_yelled = FALSE;
  ced->scanning = FALSE;
  ced->was_in_room = NULL;
  ced->reply = NULL;
  ced->fight_ops = 0xFFFFFFFF;
  ced->ground_pos = 0;
  ced->wimpy = 5;
  ced->cur_line = 0;
  ced->scr_wait = 0; ced->wait_type = 0; return ced;
}

EXIT_DATA * new_exit (void)
{
  EXIT_DATA * pExit;
  pExit = mem_alloc (sizeof (*pExit));
  top_exit++;
  pExit->to_room = NULL;
  pExit->vnum = 0; pExit->d_info = NULL;
  /*pExit->exit_info = 0;
    pExit->key = 0;
    pExit->keyword = NULL;
    pExit->description = str_dup("");
    pExit->rs_flags = 0;
    pExit->str = 30;
    pExit->maxstr = 30; */
  return pExit;
}

ROOM_DATA * new_room_index (void)
{
  ROOM_DATA * pRoom;
  int door;
  pRoom = new_room ();
  pRoom->next = NULL;
  pRoom->area = NULL;
  for (door = 0; door < MAX_DIR; door++)
    pRoom->exit[door] = NULL;
  pRoom->name = str_dup ("");
  pRoom->data_type = K_ROOM;
  pRoom->description = str_dup ("");
  pRoom->vnum = 0;
  pRoom->room_flags = 0;
  pRoom->room_flags_2 = 0;
  pRoom->light = 0; pRoom->sector_type = 1; return pRoom;
}

AREA_DATA * get_vnum_area (int vnum)
{
  AREA_DATA * pArea;
  for (pArea = area_first; pArea != NULL; pArea = pArea->next)
  {
    if (vnum >= pArea->lvnum
        && vnum <= pArea->uvnum) return pArea;
  }
  return NULL;
}

void do_2forward (CHAR_DATA * ch, char *argy)
{
  int tmp_vnum;
  char buffy[STD_LENGTH];
  DEFINE_COMMAND ("f>", do_2forward, POSITION_DEAD, 100,
      LOG_NORMAL,
      "Advances forward in medit or oedit, creating a mob/object if the next vnum is blank.")
    buffy[0] = '\0'; if (ch->desc->connected == CON_MEDITOR)
    {
      MOB_PROTOTYPE * pMob;
      MOB_PROTOTYPE * tMob;
      pMob = (MOB_PROTOTYPE *) ch->desc->pEdit;
      if (!pMob)
        return;
      tmp_vnum = pMob->vnum + 1;
      if ((tMob = get_mob_index (tmp_vnum)) != NULL)
      {
        do_forward (ch, ""); return;
      }
      sprintf (buffy, "create %d", tmp_vnum);
      do_medit (ch, buffy); do_forward (ch, ""); return;
    }
  if (ch->desc->connected == CON_OEDITOR)
  {
    OBJ_PROTOTYPE * pObj;
    OBJ_PROTOTYPE * tObj;
    pObj = (OBJ_PROTOTYPE *) ch->desc->pEdit;
    if (!pObj)
      return;
    tmp_vnum = pObj->vnum + 1;
    if ((tObj = get_obj_index (tmp_vnum)) != NULL)
    {
      do_forward (ch, ""); return;
    }
    sprintf (buffy, "create %d", tmp_vnum);
    do_oedit (ch, buffy);
    sprintf (buffy, "%d", tmp_vnum);
    if (!IS_JAVA (ch)) do_oindex (ch, buffy); return;
  }
  return;
}

void do_2backward (CHAR_DATA * ch, char *argy)
{
  int tmp_vnum;
  char buffy[STD_LENGTH];
  DEFINE_COMMAND ("b<", do_2backward, POSITION_DEAD, 100,
      LOG_NORMAL,
      "Goes backward in medit or oedit, creating a mob/object if the previous vnum is blank.")
    buffy[0] = '\0'; if (ch->desc->connected == CON_MEDITOR)
    {
      MOB_PROTOTYPE * pMob;
      MOB_PROTOTYPE * tMob;
      pMob = (MOB_PROTOTYPE *) ch->desc->pEdit;
      if (!pMob)
        return;
      tmp_vnum = pMob->vnum - 1;
      if ((tMob = get_mob_index (tmp_vnum)) != NULL)
      {
        do_backward (ch, ""); return;
      }
      sprintf (buffy, "create %d", tmp_vnum);
      do_medit (ch, buffy); do_backward (ch, ""); return;
    }
  if (ch->desc->connected == CON_OEDITOR)
  {
    OBJ_PROTOTYPE * pObj;
    OBJ_PROTOTYPE * tObj;
    pObj = (OBJ_PROTOTYPE *) ch->desc->pEdit;
    if (!pObj)
      return;
    tmp_vnum = pObj->vnum - 1;
    if ((tObj = get_obj_index (tmp_vnum)) != NULL)
    {
      do_backward (ch, ""); return;
    }
    sprintf (buffy, "create %d", tmp_vnum);
    do_oedit (ch, buffy);
    sprintf (buffy, "%d", tmp_vnum);
    if (!IS_JAVA (ch)) do_oindex (ch, buffy); return;
  }
  return;
}

void do_forward (CHAR_DATA * ch, char *argy)
{
  int tmp_vnum;
  char buffy[STD_LENGTH];
  DEFINE_COMMAND (">", do_forward, POSITION_DEAD, 100, LOG_NORMAL, "Advances 1 vnum forward in medit or oedit.")
    buffy[0] = '\0'; if (ch->desc->connected == CON_MEDITOR)
    {
      MOB_PROTOTYPE * pMob; pMob = (MOB_PROTOTYPE *) ch->desc->pEdit; if (!pMob) return; tmp_vnum = find_next_mob (pMob->vnum);	/*1; */
      sprintf (buffy, "%d", tmp_vnum);
      do_medit (ch, buffy); return;
    }
  if (ch->desc->connected == CON_OEDITOR)
  {
    OBJ_PROTOTYPE * pObj; pObj = (OBJ_PROTOTYPE *) ch->desc->pEdit; if (!pObj) return; tmp_vnum = find_next_obj (pObj->vnum);	/*1; */
    sprintf (buffy, "%d", tmp_vnum);
    do_oedit (ch, buffy);
    if (!IS_JAVA (ch)) do_oindex (ch, buffy); return;
  }
  return;
}

void do_backward (CHAR_DATA * ch, char *argy)
{
  int tmp_vnum;
  char buffy[STD_LENGTH];
  DEFINE_COMMAND ("<", do_backward, POSITION_DEAD, 100, LOG_NORMAL, "Goes backward 1 vnum in medit or oedit.")
    buffy[0] = '\0'; if (ch->desc->connected == CON_MEDITOR)
    {
      MOB_PROTOTYPE * pMob;
      pMob = (MOB_PROTOTYPE *) ch->desc->pEdit;
      if (!pMob)
        return;
      tmp_vnum = find_prev_mob (pMob->vnum);
      sprintf (buffy, "%d", tmp_vnum);
      do_medit (ch, buffy); return;
    }
  if (ch->desc->connected == CON_OEDITOR)
  {
    OBJ_PROTOTYPE * pObj;
    pObj = (OBJ_PROTOTYPE *) ch->desc->pEdit;
    if (!pObj)
      return;
    tmp_vnum = find_prev_obj (pObj->vnum);
    sprintf (buffy, "%d", tmp_vnum);
    do_oedit (ch, buffy);
    if (!IS_JAVA (ch)) do_oindex (ch, buffy); return;
  }
  return;
}

RESET_DATA * new_reset_data (void)
{
  RESET_DATA * pReset; if (reset_free == NULL)
  {
    pReset = mem_alloc (sizeof (*pReset)); top_reset++;
  }
  else
  {
    pReset = reset_free; reset_free = reset_free->next;
  }
  pReset->next = NULL;
  pReset->command = 'M';
  pReset->loc = -1;
  pReset->percent = 0;
  pReset->rs_vnum = 0; pReset->vnum = 0; return pReset;
}

void free_reset_data (RESET_DATA * pReset)
{
  pReset->next = reset_free; reset_free = pReset; return;
}

AREA_DATA * new_area (void)
{
  AREA_DATA * pArea;
  char buf[SML_LENGTH]; if (area_free == NULL)
  {
    pArea = mem_alloc (sizeof (*pArea)); top_area++;
  }
  else
  {
    pArea = area_free; area_free = area_free->next;
  }
  pArea->next = NULL;
  pArea->name = str_dup ("New area");
  pArea->repop = str_dup (" \x1B[0;36mContact an Admin to have this changed when you come up with a messege!\x1B[37;0m\n\r");
  pArea->area_flags = AREA_CHANGED;
  /*pArea->area_flags = AREA_ADDED; */
  pArea->open = 0; pArea->desc_rooms = 0; pArea->weather_change = 0; pArea->more_rainy = FALSE; pArea->more_dry = FALSE; pArea->more_fog = FALSE; pArea->security = 1; pArea->builders = str_dup ("None"); pArea->lvnum = 0; pArea->has_popped_this_reboot = TRUE; pArea->uvnum = 0; pArea->nplayer = 0; sprintf (buf, "area%d.are", pArea->vnum); pArea->filename = str_dup (buf); pArea->vnum = top_area;	/* -1 */
  pArea->room_count = 0;
  pArea->mob_count = 0;
  pArea->obj_count = 0;
  pArea->repop_rate = 24;
  pArea->repop_counter = number_range (20, 30); return pArea;
}

void free_area (AREA_DATA * pArea)
{
  free_string (pArea->name);
  free_string (pArea->filename);
  free_string (pArea->repop);
  free_string (pArea->builders);
  pArea->next = area_free->next; area_free = pArea; return;
}

DESCRIPTION_DATA * new_extra_descr (void)
{
  DESCRIPTION_DATA * pExtra; if (extra_descr_free == NULL)
  {
    pExtra = mem_alloc (sizeof (*pExtra)); top_ed++;
  }
  else
  {
    pExtra = extra_descr_free;
    extra_descr_free = extra_descr_free->next;
  }
  pExtra->keyword = str_dup ("d");	/*TESTING IF THIS FIXES */
  pExtra->description = str_dup ("");
  pExtra->next = NULL; return pExtra;
}

void free_extra_descr (DESCRIPTION_DATA * pExtra)
{
  free_string (pExtra->keyword);
  free_string (pExtra->description);
  pExtra->next = extra_descr_free;
  extra_descr_free = pExtra; return;
}

void free_exit (EXIT_DATA * pExit)
{
  if (pExit->d_info)
  {
    if (pExit->d_info->keyword)
      free_string (pExit->d_info->keyword);
    if (pExit->d_info->description)
      free_string (pExit->d_info->description);
    pExit->d_info = NULL;
  }
  /*free_m(pExit); */
  return;
}

AFFECT_DATA * new_affect (void)
{
  AFFECT_DATA * pAf; if (affect_free == NULL)
  {
    pAf = mem_alloc (sizeof (*pAf)); top_affect++;
  }
  else
  {
    pAf = affect_free; affect_free = affect_free->next;
  }
  pAf->next = NULL;
  pAf->location = 0;
  pAf->modifier = 0;
  pAf->type = 0;
  pAf->duration = 0; pAf->bitvector = 0; return pAf;
}

void free_affect (AFFECT_DATA * pAf)
{
  pAf->next = affect_free; affect_free = pAf; return;
}

OBJ_PROTOTYPE * new_obj_index (void)
{
  OBJ_PROTOTYPE * pObj; int value; if (obj_index_free == NULL)
  {
    pObj = mem_alloc (sizeof (*pObj)); top_obj_index++;
  }
  else
  {
    pObj = obj_index_free;
    obj_index_free = obj_index_free->next;
  }
  pObj->next = NULL;
  pObj->extra_descr = NULL;
  pObj->action_descr[0] = NULL;
  pObj->action_descr[1] = NULL;
  pObj->action_descr[2] = NULL;
  pObj->affected = NULL;
  pObj->area = NULL; pObj->name = str_dup ("no name");
#ifdef OLD_LIMIT_FORMAT
  strcpy (pObj->item_limited, "UNLIMITED");
#else
  pObj->how_many = 0;
#endif
  pObj->short_descr = str_dup ("(no short description)");
  pObj->description = str_dup ("(no description)");
  pObj->vnum = 0;
  pObj->item_type = ITEM_TRASH;
  pObj->max_in_room = 1;
  pObj->timer = 0;
  pObj->extra_flags = 0;
  pObj->wear_flags = 0;
  pObj->count = 0;
  pObj->weight = 0;
  pObj->level = 0;
  pObj->cost = 0;
  pObj->effects = 0;
  for (value = 0; value < 10; value++)
    pObj->value[value] = 0;
  for (value = 0; value < 10; value++)
    pObj->values_2[value] = 0; return pObj;
}


OPTIONAL_DATA * new_optional (void)
{
  OPTIONAL_DATA * optional;
  int i;
  optional = mem_alloc (sizeof (*optional));
  for (i = 0; i < 30; i++)
  {
    optional->skltaught[i] = 0;
  }
  for (i = 0; i < 10; i++)
  {
    optional->cast_spells[i] = NULL;
  }
  optional->cast_percent = 0;
  optional->kicks = FALSE;
  optional->tackles = FALSE;
  optional->flurries = FALSE;
  optional->warrior_percent = 0; return optional;
}

void new_shop (MOB_PROTOTYPE * m)
{
  SHOP_DATA * pShop; int buy; if (shop_free == NULL)
  {
    pShop = mem_alloc (sizeof (*pShop)); top_shop++;
  }
  else
  {
    pShop = shop_free; shop_free = shop_free->next;
  }
  pShop->next = NULL;
  pShop->keeper = 0;
  for (buy = 0; buy < MAX_TRADE; buy++)
    pShop->buy_type[buy] = ITEM_NONE;
  pShop->decrease = 5;
  pShop->items_until_decrease = 4;
  pShop->profit_buy = 100;
  pShop->profit_sell = 100;
  pShop->open_hour = 0;
  pShop->close_hour = 23;
  pShop->creates_vnum = 0;
  pShop->creates_hours = 1;
  pShop->creates_message = NULL;
  m->pShop = pShop;
  pShop->keeper = m->vnum;
  if (shop_last) shop_last->next = pShop;
  else
  {
    shop_first = pShop; shop_last = pShop;
  }
  return;
}

void free_shop (SHOP_DATA * pShop)
{
  pShop->next = shop_free; shop_free = pShop; return;
}

void free_mob_index (MOB_PROTOTYPE * pMob)
{
  free_string (pMob->name);
  free_string (pMob->short_descr);
  free_string (pMob->long_descr);
  free_string (pMob->description);
  free_shop (pMob->pShop);
  pMob->next = mob_index_free; mob_index_free = pMob; return;
}

int arg_count (char *argy)
{
  int total; char *s; total = 0; s = argy; while (*s != '\0')
  {
    if (*s != ' ')
    {
      total++; while (*s != ' ' && *s != '\0') s++;
    }
    else
    {
      s++;
    }
  }
  return total;
}

AREA_DATA * get_area_data (int vnum)
{
  AREA_DATA * pArea;
  for (pArea = area_first; pArea != NULL; pArea = pArea->next)
  {
    if (pArea->vnum == vnum) return pArea;
  }
  return NULL;
}

char *area_bit_name (int area_flags)
{
  static char buf[512];
  buf[0] = '\0';
  if (area_flags & AREA_CHANGED) strcat (buf, " CHANGED");
  else
    strcat (buf, " No Change");
  if (area_flags & AREA_ADDED) strcat (buf, " ADDED");
  else
    strcat (buf, ""); return buf + 1;
}

int get_area_flags_number (char *argy)
{
  if (!str_cmp (argy, "no-change"))
    return AREA_CHANGED;
  if (!str_cmp (argy, "added"))
    return AREA_ADDED; return AREA_NONE;
}

char *get_sector_name (int sector_type)
{
  switch (sector_type)
  {
    case SECT_INSIDE:
      return "(0) Inside - room always lit unless DARK is set."; case SECT_CITY:
        return "(1) City or Road - dark at night, minimal mp loss."; case SECT_FIELD:
        return "(2) Field - can see in the dark on moon-lit nights."; case SECT_FOREST:
        return "(3) Forest - Is dark if sky is cloudy."; case SECT_HILLS:
        return "(4) Hills - hard travelling."; case SECT_MOUNTAIN:
        return "(5) Mountain - extremely hard travelling."; case SECT_WATER_SWIM:
        return "(6) Swimable water sector."; case SECT_WATER_NOSWIM:
        return "(7) Unswimmable water sector."; case SECT_UNDERWATER:
        return "(8) Underwater - requires vehicle underwtr"; case SECT_AIR:
        return "(9) Air sector - fly spell required."; case SECT_DESERT:
        return "(10) Desert - can't recall from this room."; case SECT_SNOW:
        return "(11) Snow - High mp loss without vehicle."; case SECT_TROPICAL:
        return "(12) Tropical - Hot and dense."; case SECT_JAVA:
        return "(13) Impassable."; case SECT_ICE:
        return "(14) Ice/Ice field."; case SECT_MARSH:
        return "(15) Marsh/Swampland";
  }
  return "unknown sector";
}

int get_sector_number (char *argy)
{
  if (!str_cmp (argy, "inside"))
    return SECT_INSIDE;
  if (!str_cmp (argy, "city"))
    return SECT_CITY;
  if (!str_cmp (argy, "field"))
    return SECT_FIELD;
  if (!str_cmp (argy, "forest"))
    return SECT_FOREST;
  if (!str_cmp (argy, "hills"))
    return SECT_HILLS;
  if (!str_cmp (argy, "mountain"))
    return SECT_MOUNTAIN;
  if (!str_cmp (argy, "swim"))
    return SECT_WATER_SWIM;
  if (!str_cmp (argy, "noswim"))
    return SECT_WATER_NOSWIM;
  if (!str_cmp (argy, "underwater"))
    return SECT_UNDERWATER;
  if (!str_cmp (argy, "air"))
    return SECT_AIR;
  if (!str_cmp (argy, "desert"))
    return SECT_DESERT;
  if (!str_cmp (argy, "snow"))
    return SECT_SNOW;
  if (!str_cmp (argy, "tropical"))
    return SECT_TROPICAL;
  if (!str_cmp (argy, "impassable"))
    return SECT_JAVA;
  if (!str_cmp (argy, "ice"))
    return SECT_ICE;
  if (!str_cmp (argy, "marsh"))
    return SECT_MARSH; return SECT_MAX;
}

int get_direction_number (char *arg)
{
  if (!str_cmp (arg, "n") || !str_cmp (arg, "north"))
    return DIR_NORTH;
  if (!str_cmp (arg, "e") || !str_cmp (arg, "east"))
    return DIR_EAST;
  if (!str_cmp (arg, "s") || !str_cmp (arg, "south"))
    return DIR_SOUTH;
  if (!str_cmp (arg, "w") || !str_cmp (arg, "west"))
    return DIR_WEST;
  if (!str_cmp (arg, "u") || !str_cmp (arg, "up"))
    return DIR_UP;
  if (!str_cmp (arg, "d") || !str_cmp (arg, "down"))
    return DIR_DOWN; return DIR_MAX;
}

int get_exit_flag_number (char *arg)
{
  if (!str_cmp (arg, "door"))
    return EX_ISDOOR;
  if (!str_cmp (arg, "closed"))
    return EX_CLOSED;
  if (!str_cmp (arg, "locked"))
    return EX_LOCKED;
  if (!str_cmp (arg, "pickproof"))
    return EX_PICKPROOF;
  if (!str_cmp (arg, "hidden"))
    return EX_HIDDEN;
  if (!str_cmp (arg, "bashproof"))
    return EX_BASHPROOF;
  if (!str_cmp (arg, "fixdoor"))
    return EX_RESET; return EX_NONE;
}

int get_mob_sex_number (char *arg)
{
  if (!str_cmp (arg, "neuter"))
    return SEX_NEUTRAL;
  if (!str_cmp (arg, "male"))
    return SEX_MALE;
  if (!str_cmp (arg, "female"))
    return SEX_FEMALE; return SEX_NONE;
}

SPEC_FUNCTION * get_mob_spec (char *arg)
{
  char buf[STD_LENGTH];
  buf[0] = '\0';
  strcat (buf, "spec_");
  strcat (buf, arg); return spec_lookup (buf);
}

void do_aedit (CHAR_DATA * ch, char *argy)
{
  AREA_DATA * pArea;
  int value;
  DEFINE_COMMAND ("aedit", do_aedit, POSITION_DEAD, 110, LOG_ALWAYS, "This command allows you to enter the area editor.")
    if (IS_MOB (ch)) return; pArea = ch->in_room->area;
  if (is_number (argy))
  {
    value = atoi (argy);
    if ((pArea = get_area_data (value)) == NULL)
    {
      send_to_char ("That area vnum does not exist.\n\r", ch);
      return;
    }
  }
  else
  {
    if (!str_cmp (argy, "create"))
    {
      /*     pArea = new_area();
             area_last->next = pArea;
             SET_BIT( pArea->area_flags, AREA_ADDED ); */
      pArea = new_area ();
      top_area++; area_sort (pArea); save_area_list ();
    }
  }
  ch->desc->pEdit = (void *) pArea;
  ch->desc->connected = CON_AEDITOR; return;
}

void do_soc_edit (CHAR_DATA * ch, char *argy)
{
  SOCIAL * s;
  int value;
  int iHash;
  char arg1[STD_LENGTH];
  DEFINE_COMMAND ("socedit", do_soc_edit, POSITION_DEAD, 110, LOG_ALWAYS, "This command allows you to modify/add socials")
    if (IS_MOB (ch)) return; if (!ch->desc) return;
  argy = one_argy (argy, arg1); if (arg1[0] == '\0') return;
  if (!str_cmp (arg1, "create"))
  {
    s = (SOCIAL *) new_social ();
    strcpy (s->name, "none");
    ch->desc->pEdit = s;
    ch->desc->connected = CON_SOCEDIT;
    send_to_char
      ("Social created.  Entering social editor.\n\r", ch);
    return;
  }
  for (s = social_list; s != NULL; s = s->next)
  {
    if (!str_cmp (s->name, arg1))
    {
      ch->desc->pEdit = s;
      ch->desc->connected = CON_SOCEDIT;
      send_to_char ("Social found.  Entering social editor.\n\r",
          ch); return;
    }
  }
  send_to_char ("That social could not be found.\n\r", ch);
  return;
}

void do_redit (CHAR_DATA * ch, char *argy)
{
  ROOM_DATA * pRoom;
  AREA_DATA * pArea;
  int value;
  int iHash;
  char arg1[STD_LENGTH];
  DEFINE_COMMAND ("redit", do_redit, POSITION_DEAD, 103, LOG_NORMAL, "This command allows you to enter the room editor.")
    if (IS_MOB (ch)) return; argy = one_argy (argy, arg1);
  pRoom = ch->in_room; if (is_number (arg1))
  {
    value = atoi (arg1);
    if ((pRoom = get_room_index (value)) == NULL)
    {
      send_to_char ("REdit: That vnum does not exist.\n\r", ch);
      return;
    }
  }
    else
    {
      if (!str_cmp (arg1, "reset"))
      {
        reset_room (pRoom);
        send_to_char ("Room reset.\n\r", ch); return;
      }
      else
        if (!str_cmp (arg1, "create"))
        {
          value = atoi (argy); if (argy[0] == '\0' || value == 0)
          {
            send_to_char ("Syntax: redit create [vnum]\n\r", ch);
            return;
          }
          pArea = get_vnum_area (value); if (pArea == NULL)
          {
            send_to_char ("REdit: That vnum is not assigned an area.\n\r", ch);
            return;
          }
          if (!IS_BUILDER (ch, pArea))
          {
            send_to_char
              ("REdit: Vnum in an area you can't build in.\n\r", ch);
            return;
          }
          if (get_room_index (value) != NULL)
          {
            send_to_char ("REdit: Room vnum already exists.\n\r", ch);
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
          SET_BIT (pArea->area_flags, AREA_CHANGED);
          pArea->room_count++;
          char_from_room (ch); char_to_room (ch, pRoom);
        }
    }
  ch->desc->connected = CON_REDITOR; return;
}


void do_oedit (CHAR_DATA * ch, char *argy)
{
  OBJ_PROTOTYPE * pObj;
  AREA_DATA * pArea;
  int value;
  int iHash;
  char arg1[STD_LENGTH];
  DEFINE_COMMAND ("oedit", do_oedit, POSITION_DEAD, 103, LOG_NORMAL, "This command allows you to enter the object editor.")
    argy = one_argy (argy, arg1); if (IS_MOB (ch)) return;
  if (is_number (arg1))
  {
    value = atoi (arg1);
    if ((pObj = get_obj_index (value)) == NULL)
    {
      send_to_char ("OEdit: That vnum does not exist.\n\r", ch);
      return;
    }
    /*pArea=get_vnum_area(value); */
    pArea = pObj->area; if (!IS_BUILDER (ch, pArea))
    {
      send_to_char ("OEdit: It is not nice to edit other people's objects!\n\r", ch); 
      return;
    }
    ch->desc->pEdit = (void *) pObj;
    if (IS_JAVA (ch)) java_oedit (ch, pObj);
    ch->desc->connected = CON_OEDITOR; return;
  }
  else
  {
    if (!str_cmp (arg1, "create"))
    {
      value = atoi (argy); if (argy[0] == '\0' || value == 0)
      {
        send_to_char ("Syntax: oedit create [vnum]\n\r", ch);
        return;
      }
      pArea = get_vnum_area (value); if (pArea == NULL)
      {
        send_to_char ("OEdit: That vnum is not assigned an area.\n\r", ch);
        return;
      }
      if (!IS_BUILDER (ch, pArea))
      {
        send_to_char("OEdit: Vnum in an area you can't build in.\n\r", ch);
        return;
      }
      if (get_obj_index (value) != NULL)
      {
        char tmp[50];
        send_to_char ("OEdit: Object vnum already exists.\n\r", ch);
        sprintf (tmp, "%d", value); do_oedit (ch, tmp); return;
      }
      pObj = new_obj_index ();
      pObj->vnum = value;
      pObj->area = pArea;
      if (value > top_vnum_obj)
        top_vnum_obj = value;
      iHash = value % HASH_MAX;
      pObj->next = obj_index_hash[iHash];
      obj_index_hash[iHash] = pObj;
      ch->desc->pEdit = (void *) pObj;
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      pArea->obj_count++;
      if (IS_JAVA (ch)) java_oedit (ch, pObj);
      ch->desc->connected = CON_OEDITOR; return;
    }
  }
  send_to_char ("OEdit: Syntax is OEDIT <vnum> or OEDIT CREATE <vnum>.\n\r", ch); 
	return;
}

void do_medit (CHAR_DATA * ch, char *argy)
{
  MOB_PROTOTYPE * pMob;
  AREA_DATA * pArea;
  int value;
  int iHash;
  char arg1[STD_LENGTH];
  DEFINE_COMMAND ("medit", do_medit, POSITION_DEAD, 103, LOG_NORMAL, "This command allows you to enter the mob editor.")
    if (IS_MOB (ch)) return; argy = one_argy (argy, arg1);
  if (is_number (arg1))
  {
    value = atoi (arg1);
    if ((pMob = get_mob_index (value)) == NULL)
    {
      send_to_char ("MEdit: That vnum does not exist.\n\r", ch);
      return;
    }
    /*pArea = get_vnum_area( value ); */
    pArea = pMob->area;
    if (pArea != NULL) if (!IS_BUILDER (ch, pArea))
    {
      send_to_char
        ("MEdit: Vnum in an area you can't build in.\n\r", ch);
      return;
    }
    ch->desc->pEdit = (void *) pMob;
    ch->desc->connected = CON_MEDITOR;
    mindex_window (ch, pMob, -1);
    if (IS_JAVA (ch)) java_medit (ch, pMob); return;
  }
  else
  {
    if (!str_cmp (arg1, "create"))
    {
      value = atoi (argy); if (argy[0] == '\0' || value == 0)
      {
        send_to_char ("Syntax: medit create [vnum]\n\r", ch);
        return;
      }
      pArea = get_vnum_area (value); if (pArea == NULL)
      {
        send_to_char
          ("MEdit: That vnum is not assigned an area.\n\r", ch);
        return;
      }
      if (!IS_BUILDER (ch, pArea))
      {
        send_to_char
          ("MEdit: Vnum in an area you can't build in.\n\r", ch);
        return;
      }
      if (get_mob_index (value) != NULL)
      {
        send_to_char ("MEdit: Mob vnum already exists.\n\r", ch);
        return;
      }
      pMob = new_mob_index ();
      pMob->vnum = value;
      pMob->area = pArea;
      if (value > top_vnum_mob)
        top_vnum_mob = value;
      iHash = value % HASH_MAX;
      pMob->next = mob_index_hash[iHash];
      mob_index_hash[iHash] = pMob;
      ch->desc->pEdit = (void *) pMob;
      mindex_window (ch, pMob, -1);
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      pArea->mob_count++;
      ch->desc->connected = CON_MEDITOR;
      if (IS_JAVA (ch)) java_medit (ch, pMob); return;
    }
  }
  send_to_char
    ("MEdit: Syntax is MEDIT <vnum> or MEDIT CREATE <vnum>.\n\r",
     ch); return;
}

void do_cedit (CHAR_DATA * ch, char *argy)
{
  CLAN_DATA * ClanData;
  int value;
  int iHash;
  char arg1[STD_LENGTH];
  DEFINE_COMMAND ("cedit", do_cedit, POSITION_DEAD, 111, LOG_ALWAYS, "This command allows you to enter the clan editor.")
    ch->fgt->attacker_turn = FALSE; if (IS_MOB (ch)) return;
  argy = one_argy (argy, arg1); if (is_number (arg1))
  {
    value = atoi (arg1);
    if ((ClanData = get_clan_index (value)) == NULL)
    {
      send_to_char ("CEdit: That vnum does not exist.\n\r", ch);
      return;
    }
    ch->desc->pEdit = (void *) ClanData;
    ch->desc->connected = CON_CEDITOR; return;
  }
  else
  {
    if (!str_cmp (arg1, "create"))
    {
      value = atoi (argy); if (argy[0] == '\0' || value == 0)
      {
        send_to_char ("Syntax: cedit create [vnum]\n\r", ch);
        return;
      }
      if (get_clan_index (value) != NULL)
      {
        send_to_char ("CEdit: CLAN vnum already exists.\n\r", ch);
        return;
      }
      ClanData = new_clan_index ();
      ClanData->vnum = value;
      if (value > top_clan)
        top_clan = value;
      iHash = value % 20;
      ClanData->next = clan_index_hash[iHash];
      clan_index_hash[iHash] = ClanData;
      ch->desc->pEdit = (void *) ClanData;
      ch->desc->connected = CON_CEDITOR; return;
    }
  }
  send_to_char
    ("CEdit: Syntax is CEDIT <vnum> or CEDIT CREATE <vnum>.\n\r",
     ch); return;
}


void do_cedit2 (CHAR_DATA * ch, char *argy)
{
  CLAN_DATA * ClanData;
  int value;
  int iHash;
  char arg1[STD_LENGTH];
  DEFINE_COMMAND ("cedit2", do_cedit2, POSITION_DEAD, 111, LOG_ALWAYS, "This command allows you to enter the clan editor for alternative 'clans'.")
    if (IS_MOB (ch)) return; ch->fgt->attacker_turn = TRUE;
  argy = one_argy (argy, arg1); if (is_number (arg1))
  {
    value = atoi (arg1);
    if ((ClanData = get_clan_index_2 (value)) == NULL)
    {
      send_to_char ("CEdit2: That vnum does not exist.\n\r", ch);
      return;
    }
    ch->desc->pEdit = (void *) ClanData;
    ch->desc->connected = CON_CEDITOR; return;
  }
    else
    {
      if (!str_cmp (arg1, "create"))
      {
        value = atoi (argy); if (argy[0] == '\0' || value == 0)
        {
          send_to_char ("Syntax: cedit2 create [vnum]\n\r", ch);
          return;
        }
        if (get_clan_index_2 (value) != NULL)
        {
          send_to_char ("CEdit: CLAN vnum already exists.\n\r", ch);
          return;
        }
        ClanData = new_clan_index_2 ();
        ClanData->vnum = value;
        if (value > top_clan_2)
          top_clan_2 = value;
        iHash = value % 20;
        ClanData->next = clan_index_hash_2[iHash];
        clan_index_hash_2[iHash] = ClanData;
        ch->desc->pEdit = (void *) ClanData;
        ch->desc->connected = CON_CEDITOR; return;
      }
    }
  send_to_char
    ("CEdit2: Syntax is CEDIT2 <vnum> or CEDIT2 CREATE <vnum>.\n\r",
     ch); return;
}




void do_mindex (CHAR_DATA * ch, char *argy)
{
  DEFINE_COMMAND ("mindex", do_mindex, POSITION_DEAD, 103, LOG_NORMAL, "This command shows a complete mob template.")
    do_mindexc (ch, argy, FALSE); return;
}

void
mindex_window (CHAR_DATA * ch, MOB_PROTOTYPE * victim,
    int update)
{
  static char buf[1024]; if (IS_JAVA (ch))
  {
    if (update != -1) return;
    sprintf (buf, "%d", victim->vnum);
    do_mindexc (ch, buf, FALSE); return;
  }
  /*
     1 Vnum [48374] Level: 114 Sex: Male Money: 4834 Hp: 2.4 Ktr: 484 J1 J2
     2 Name 'eastern sentry gate large' Align: 9324 AC: Auto Area: 85
     3 Short 'a large eastern gate sentry' Spec : Spec_warrior Alt: 39238
     4 Act: sentinel 3guard_door_good
     4 Act: kevil kneutral Height: 56 (4' 8")
     5 Aff: detect-invis detect-hidden
     6 Guarding: -1 Yell#: 13 Willhelp: 1 in 0 Attack: punch [Human(oid)]
     7 Long 'Joe blow stands here, guarding the eastern gate with his life!'
     -----------------------------------------------------------------------------
   */
  if (update == -1)
  {
    sprintf (buf,
        "\x1B[1;1f\x1B[K\x1B[2;1f\x1B[K\x1B[3;1f\x1B[K\x1B[4;1f\x1B[K\x1B[5;1f\x1B[K\x1B[6;1f\x1B[K\x1B[7;1f\x1B[K\x1B[8;1f\x1B[K\x1B[9;1f\x1B[K\x1B[10;%dr",
        ch->pcdata->pagelen); send_to_char (buf, ch);
  }
  /*Vnum [48374] Level: 114 Sex: Male Money: 4834 Hp: 2.4 Ktr: 484 */
  if (update == -1 || update == 1)
  {
    sprintf (buf,
        "\x1B[1;1f\x1B[KVnum [\x1B[32;1m%d\x1B[37;0m] Level: \x1B[1m%-3d\x1B[0m Sex: %-7s Money: \x1B[1m%-5d\x1B[0m Hp: \x1B[1m%2.1f\x1B[0m Ktr: \x1B[1m%d\x1B[0m [%d %d]\n\r",
        victim->vnum, victim->level,
        (victim->sex == SEX_MALE ? "male" : victim->sex ==
         SEX_FEMALE ? "female" : "neutral"), victim->money,
        ((double) victim->hpmult / (double) 10),
        victim->killed, victim->java[0], victim->java[1]);
    send_to_char (buf, ch); if (update != -1)
    {
      sprintf (buf, "\x1B[%d;1f\x1B[K", ch->pcdata->pagelen);
      send_to_char (buf, ch); return;
    }
  }
  /*Name 'eastern sentry gate large' Align: 9324 AC: Auto Area: 85 */
  if (update == -1 || update == 2)
  {
    char minibuf[100];
    if (victim->armcls == 101) sprintf (minibuf, "Auto");
    else
      sprintf (minibuf, "%d", victim->armcls);
    sprintf (buf,
        "\x1B[2;1f\x1B[KName '%s'\x1B[2;39f\x1B[K Align: \x1B[1m%-4d\x1B[0m AC: \x1B[1m%-4s\x1B[0m Area: %d\n\r",
        victim->name, victim->alignment, minibuf,
        victim->area->vnum); send_to_char (buf, ch);
        if (update != -1)
        {
          sprintf (buf, "\x1B[%d;1f\x1B[K", ch->pcdata->pagelen);
          send_to_char (buf, ch); return;
        }
  }
  /*Short 'a large eastern gate sentry' Spec : Spec_warrior Alt: 39238 */
  if (update == -1 || update == 3)
  {
    sprintf (buf,
        "\x1B[3;1f\x1B[KShort '%s'\x1B[3;41f\x1B[KSpec : \x1B[1m%-15s\x1B[0m Alt: \x1B[1m%d\x1B[0m\n\r",
        victim->short_descr,
        (victim->spec_name ==
         NULL ? "None" : victim->spec_name),
        victim->alt_vnum); send_to_char (buf, ch);
        if (update != -1)
        {
          sprintf (buf, "\x1B[%d;1f\x1B[K", ch->pcdata->pagelen);
          send_to_char (buf, ch); return;
        }
  }
  /*Act: sentinel 3guard_door_good
Act: kevil kneutral */
  if (update == -1 || update == 4)
  {
    sprintf (buf,
        "\x1B[4;1f\x1B[KAct: \x1B[32;1m%s\x1B[5;1f\x1B[K\x1B[37;0mAct: \x1B[31;1m%s \x1B[37m%s \x1B[30;1m%s\x1B[37;0m",
        act_bit_name (victim->act),
        act3_bit_name (victim->act3),
        act4_bit_name (victim->act4),
        race_bit_name (victim->race_hate));
    send_to_char (buf, ch);
    sprintf (buf, "\x1B[4;50fMaxIW: %d", victim->max_in_world);
    send_to_char (buf, ch);
    if (victim->height == -1) sprintf (buf,
        "\x1B[5;49fHeight: <Autocalc>");
    else
      sprintf (buf,
          "\x1B[5;49fHeight: \x1B[37;1m%d\x1B[37;0m (%d' %d\")",
          victim->height, victim->height / 12,
          victim->height % 12); send_to_char (buf, ch);
          if (update != -1)
          {
            sprintf (buf, "\x1B[%d;1f\x1B[K", ch->pcdata->pagelen);
            send_to_char (buf, ch); return;
          }
  }
  /*Aff: detect-invis detect-hidden */
  if (update == -1 || update == 5)
  {
    sprintf (buf,
        "\x1B[6;1f\x1B[KAff: \x1B[1m%s \x1B[1;33m%s\x1B[0m\n\r",
        affect_bit_name (victim->affected_by),
        affect_bit_name_two (victim->more_affected_by));
    send_to_char (buf, ch); if (update != -1)
    {
      sprintf (buf, "\x1B[%d;1f\x1B[K", ch->pcdata->pagelen);
      send_to_char (buf, ch); return;
    }
  }
  /*Guarding: -1 Yell#: 13 Willhelp: 1 in 0 Attack: punch [Human(oid)] */
  if (update == -1 || update == 6)
  {
    char minibuf[200];
    if (victim->attackname
        && str_cmp ("N/A",
          victim->attackname)) sprintf (minibuf, "%s",
          victim->
          attackname);
    else
      sprintf (minibuf, "%s", mob_type_attack (victim->mobtype));
    sprintf (buf,
        "\x1B[7;1f\x1B[KGuarding: \x1B[1m%-2d\x1B[0m Yell#: \x1B[1m%-2d\x1B[0m Willhelp: 1 in \x1B[1m%-2d\x1B[0m Attack: \x1B[1m%s\x1B[0m [\x1B[34;1m%s\x1B[37;0m]\n\r",
        victim->guard, victim->yeller_number,
        victim->will_help, minibuf,
        mob_type_name (victim->mobtype));
    send_to_char (buf, ch); if (update != -1)
    {
      sprintf (buf, "\x1B[%d;1f\x1B[K", ch->pcdata->pagelen);
      send_to_char (buf, ch); return;
    }
  }
  /*Long 'Joe blow stands here, guarding the eastern gate with his life!' */
  if (update == -1 || update == 7)
  {
    sprintf (buf, "\x1B[8;1f\x1B[KLong: %s",
        victim->long_descr);
    send_to_char (buf, ch); if (update != -1)
    {
      sprintf (buf, "\x1B[%d;1f\x1B[K", ch->pcdata->pagelen);
      send_to_char (buf, ch); return;
    }
  }
  send_to_char
    ("\x1B[31;1m-----------------------------------------------------------------------------\x1B[37;0m\n\r",
     ch);
  sprintf (buf, "\x1B[%d;1f\x1B[K", ch->pcdata->pagelen);
  send_to_char (buf, ch); return;
}

void do_mindexc (CHAR_DATA * ch, char *argy, bool windowed)
{
  bool f1 = FALSE;
  char buf[STD_LENGTH];
  char arg[SML_LENGTH];
  MOB_PROTOTYPE * victim;
  int cnt;
  int linect; one_argy (argy, arg); if (arg[0] == '\0')
  {
    send_to_char ("Mindex whom?\n\r", ch); return;
  }
  if ((victim = get_mob_index (atoi (arg))) == NULL)
  {
    send_to_char ("Invalid mob index VNUM.\n\r", ch); return;
  }
  if (LEVEL (ch) > 100 && LEVEL (ch) < 110
      && victim->area->open != 0)
  {
    send_to_char
      ("Due to cheat reasons, you cannot mindex open areas' mobs.\n\r",
       ch); return;
  }
  linect = 0;
  sprintf (buf,
      "Name (Keywords): '\x1B[1m%s\x1B[0m' (%d)\n\r",
      victim->name, victim->points);
  send_to_char (buf, ch); linect++; if (!windowed)
  {
    sprintf (buf, "Area %d: '\x1B[1m%s\x1B[0m'\n\r",
        victim->area->vnum,
        victim->area->name); send_to_char (buf, ch);
  }
  if (victim->race_hate != 0)
  {
    sprintf (buf, "Race_hate: \x1B[1m%s\x1B[0m\n\r",
        race_bit_name (victim->race_hate));
    send_to_char (buf, ch); linect++;
  }
  if (victim->yeller_number != 0)
  {
    if (windowed)
      sprintf (buf,
          "\x1B[sYell #: \x1B[1m%d\x1B[0m (Willhelp 1 in %d) 0=never.\x1B[u",
          victim->yeller_number, victim->will_help);
    else
      sprintf (buf,
          "Yeller_number: \x1B[1m%d\x1B[0m (Will_help chance: 1 in %d) <-- 0 means never.\n\r",
          victim->yeller_number, victim->will_help);
    send_to_char (buf, ch); linect++;
  }
  sprintf (buf, "Act: \x1B[1m%s\x1B[0m\n\r",
      act_bit_name (victim->act));
  send_to_char (buf, ch); linect++; if (victim->act3 != 0)
  {
    if (victim->act4 == 0)
      sprintf (buf, "Act3: \x1B[1m%s\x1B[0m\n\r",
          act3_bit_name (victim->act3));
    else
      sprintf (buf,
          "Act3: \x1B[1m%s\x1B[0m Act4: \x1B[1m%s\x1B[0m\n\r",
          act3_bit_name (victim->act3),
          act4_bit_name (victim->act4));
    send_to_char (buf, ch); linect++;
  }
  if (victim->act4 != 0 && victim->act3 == 0)
  {
    sprintf (buf, "Act4: \x1B[1m%s\x1B[0m\n\r",
        act4_bit_name (victim->act4));
    send_to_char (buf, ch); linect++;
  }
  if (victim->height == -1) sprintf (buf,
      "Height: <Autocalc>\n\r");
  else
    sprintf (buf, "Height: \x1B[37;1m%d\x1B[37;0m (%d' %d\")",
        victim->height,
        victim->height / 12,
        victim->height % 12);
  send_to_char (buf, ch); if (victim->guard != -1)
  {
    if (str_cmp ("N/A", victim->attackname)
        && victim->attackname != NULL) sprintf (buf,
          "Guarding: \x1B[1m%d\x1B[0m Attack String: %s\n\r",
          victim->guard,
          victim->
          attackname);
    else
      sprintf (buf, "Guarding: \x1B[1m%d\x1B[0m\n\r",
          victim->guard); send_to_char (buf, ch); linect++;
  }
  if (victim->attackname
      && str_cmp ("N/A", victim->attackname)
      && victim->guard == -1)
  {
    sprintf (buf, "Attack String: %s\n\r", victim->attackname);
    send_to_char (buf, ch);
  }
  sprintf (buf,
      "Vnum: \x1B[1m%d\x1B[0m Kills this Reboot: \x1B[1m%d\x1B[0m Sex: \x1B[1m%s\x1B[0m\n\r",
      victim->vnum, victim->killed,
      victim->sex == SEX_MALE ? "male" : victim->sex ==
      SEX_FEMALE ? "female" : "neutral");
  send_to_char (buf, ch); linect++;
  sprintf (buf,
      "Level: \x1B[1m%d\x1B[0m Align: \x1B[1m%d\x1B[0m Money: \x1B[1m%d\x1B[0m\n\r",
      victim->level, victim->alignment, victim->money);
  send_to_char (buf, ch); linect++;
  if (victim->affected_by != 0)
  {
    sprintf (buf, "Affected by: \x1B[1m%s\x1B[0m\n\r",
        affect_bit_name (victim->affected_by));
    send_to_char (buf, ch); linect++;
  }
  if (victim->more_affected_by != 0)
  {
    sprintf (buf, "Also affected by: \x1B[1m%s\x1B[0m\n\r",
        affect_bit_name_two (victim->more_affected_by));
    send_to_char (buf, ch); linect++;
  }
  sprintf (buf,
      "Short description: \x1B[1m%s\x1B[0m\n\rLong description:\n\r\x1B[1m%s\x1B[0m",
      victim->short_descr, victim->long_descr);
  send_to_char (buf, ch); linect += 2;
  if (victim->clan_guard_1 != 0 || victim->clan_guard_2 != 0)
  {
    sprintf (buf,
        "ClanGuard #%d.  Sect/Religion Guard #%d.\n\r",
        victim->clan_guard_1, victim->clan_guard_2);
    send_to_char (buf, ch); linect++;
  }
  if (victim->mobtype >= 0 && victim->mobtype < MAX_MOB_TYPE)
  {
    sprintf (buf, "Category: \x1B[1m%s.\x1B[0m\n\r",
        mob_type_name (victim->mobtype));
    send_to_char (buf, ch); linect++;
  }
  if (victim->alt_vnum !=
      0 /*victim->mobtype == MOB_LYCAN */ )
  {
    sprintf (buf,
        "This mob will transform into mob vnum %d\n\r",
        victim->alt_vnum); send_to_char (buf, ch);
        linect++;
  }
  if (victim->hpmult != 10)
  {
    if (victim->hpmult > 1000)
    {
      sprintf (buf,
          "This mob will have %d0%% norm exp and %2.1f times norm hp.\n\r",
          (victim->hpmult / 1000),
          ((double) (victim->hpmult % 1000) / (double) 10));
    }
    else
    {
      sprintf (buf,
          "This mob will have %2.1f times its normal hp.\n\r",
          (double) victim->hpmult / (double) 10);
    }
    send_to_char (buf, ch); linect++;
  }
  if (victim->hitroll > 0)
  {
    sprintf (buf, "Hitroll: %d.  ", victim->hitroll);
    send_to_char (buf, ch); f1 = TRUE;
  }
  if (victim->damroll != 0)
  {
    sprintf (buf, "Damroll: %d.  ", victim->damroll);
    send_to_char (buf, ch); f1 = TRUE;
  }
  if (victim->dodge > 0)
  {
    sprintf (buf, "Dodge: %d.  ", victim->dodge);
    send_to_char (buf, ch); f1 = TRUE;
  }
  if (victim->parry > 0)
  {
    sprintf (buf, "Parry: %d.  ", victim->parry);
    send_to_char (buf, ch); f1 = TRUE;
  }
  if (f1)
  {
    send_to_char ("\n\r", ch); linect++;
  }

  if (victim->spec_fun != NULL)
  {
    sprintf (buf, "Spec fun: \x1B[1m%s\x1B[0m ",
        victim->spec_name); send_to_char (buf, ch);
  }
  /*send_to_char("ARMOR <num> changes armor class... ARMOR 101 means auto-calculate.\n\r",ch); */
  if (victim->armcls == 101)
  {
    send_to_char ("Armor Class Automatically Calculated.\n\r",
        ch); linect++;
  }
  else
  {
    sprintf (buf, "Armor Class: (%d)\n\r", victim->armcls);
    send_to_char (buf, ch); linect++;
  }
  if (IS_SET (victim->act, ACT_PRACTICE) && victim->opt)
  {
    SPELL_DATA * spell;
    send_to_char ("Skills Taught: ", ch);
    cnt = 0; while (victim->opt->skltaught[cnt] != 0)
    {
      if (victim->opt->skltaught[cnt] == 0)
        sprintf (buf, "None\n\r");
      else
      {
        sprintf (buf, "%d: ", victim->opt->skltaught[cnt]);
        send_to_char (buf, ch);
        if ((spell =
              skill_lookup (NULL,
                victim->opt->skltaught[cnt])) == NULL)
        {
          send_to_char ("BAD SPELL/SKILL TAUGHT!!\n\r", ch);
        }
        else
        {
          sprintf (buf, "(%s) ", spell->spell_funky_name);
          send_to_char (buf, ch);
        }
      }
      cnt++; linect += 3;
    }
    send_to_char ("\n\r", ch);
  }
  if (victim->opt)
  {
    short i;
    short col = 1; char buff[100]; for (i = 0; i < 10; i++)
    {
      if (victim->opt->cast_spells[i] != NULL)
      {
        if (col == 3)
        {
          col = 1; send_to_char ("\n\r", ch);
        }
        sprintf (buff, "Casts %-30s", victim->opt->cast_spells[i]);
        send_to_char (buff, ch); col++;
      }
    }
    if (col != 1)
      send_to_char ("\n\r", ch);
    sprintf (buff,
        "Cast_P = %d. Warrior_P = %d. Flu: %s Kick: %s Tack: %s.\n\r",
        victim->opt->cast_percent,
        victim->opt->warrior_percent,
        (victim->opt->flurries ? "Yes" : "No"),
        (victim->opt->kicks ? "Yes" : "No"),
        (victim->opt->tackles ? "Yes" : "No"));
    send_to_char (buff, ch);
  }
  if (victim->script_fn != NULL)
  {
    sprintf (buf, "Script file: \x1B[1m%s\x1B[0m\n\r",
        victim->script_fn);
    send_to_char (buf, ch); linect++;
  }
  if (victim->pShop != NULL)
  {
    SHOP_DATA * pShop;
    int iTrade;
    pShop = victim->pShop;
    sprintf (buf,
        "Shop data (for %d):\n\r People buy at %d%%, and sell at %d%%.\n\r",
        pShop->keeper, pShop->profit_buy,
        pShop->profit_sell); send_to_char (buf, ch);
        linect++;
        sprintf (buf,
            " Opens at %d, closes at %d (also closed at 12 noon).\n\r",
            pShop->open_hour, pShop->close_hour);
        send_to_char (buf, ch); linect++;
        sprintf (buf,
            " Decrease in amount paid for specific item by %d percent aftr %d items.\n\r",
            pShop->decrease, pShop->items_until_decrease);
        send_to_char (buf, ch); linect++;
        if (pShop->creates_vnum != 0)
        {
          sprintf (buf,
              " This shop creates item vnum %d every %d game hour%s.\n\r",
              pShop->creates_vnum, pShop->creates_hours,
              (pShop->creates_hours == 1 ? "" : "s"));
          send_to_char (buf, ch); linect++;
          sprintf (buf, " Message: %s\n\r",
              (pShop->creates_message ==
               NULL ? "(No message)" : pShop->creates_message));
          send_to_char (buf, ch); linect++;
        }
        for (iTrade = 0; iTrade < MAX_TRADE; iTrade++)
        {
          if (pShop->buy_type[iTrade] != ITEM_NONE)
          {
            sprintf (buf, " Buys %ss (#%d)\n\r",
                item_type_name (pShop->buy_type[iTrade]), iTrade);
            send_to_char (buf, ch); linect++;
          }
        }
  }
  return;
}

void do_oindex (CHAR_DATA * ch, char *argy)
{
  char buf[STD_LENGTH];
  char arg[SML_LENGTH];
  char tmps[500];
  AFFECT_DATA * paf;
  OBJ_PROTOTYPE * obj;
  DEFINE_COMMAND ("oindex", do_oindex, POSITION_DEAD, 103, LOG_NORMAL, "This command allows you to view a complete object prototype.")
    one_argy (argy, arg); if (arg[0] == '\0')
    {
      send_to_char ("Oindex what?\n\r", ch); return;
    }
  if ((obj = get_obj_index (atoi (arg))) == NULL)
  {
    send_to_char ("Invalid VNUM reference.\n\r", ch); return;
  }
  if (LEVEL (ch) > 100 && LEVEL (ch) < 110
      && obj->area->open != 0)
  {
    send_to_char
      ("Due to cheat reasons, you cannot oindex open areas' objects.\n\r",
       ch); return;
  }
  if (obj->made_of == 0)
  {
    tmps[0] = '\0';
  }
  else
  {
    sprintf (tmps, "(%d : \x1B[37;1m%s\x1B[37;0m)", obj->made_of, materials[obj->made_of - 1].name);
  }
  sprintf (buf, "\x1B[0;37mName: '\x1B[1;31m%s\x1B[0;37m' %s\n\rArea \x1B[1;31m%d\x1B[0;37m: '\x1B[1;31m%s\x1B[0;37m'\n\r", obj->name, tmps, obj->area->vnum, obj->area->name);
  send_to_char (buf, ch);
  sprintf (buf, "\x1B[0;37mVnum: \x1B[1;31m%d\n\r\x1B[0;37mType: \x1B[1;31m%s\n\r", obj->vnum, item_type_name (obj->item_type));
  send_to_char (buf, ch);
  sprintf (buf, "\x1B[0;37mShort description: \x1B[37;0m%s\x1B[0m\n\r\x1B[0;37mLong description: \x1B[37;0m%s\x1B[0m\n\r", obj->short_descr, obj->description);
  send_to_char (buf, ch);
  sprintf (buf, "\x1B[0;37mLimited to \x1B[37;1m%d\x1B[0;37m in existance. \x1B[1;30m(0 = unlimited)\n\r", obj->how_many); 
  send_to_char (buf, ch);
  sprintf (buf, "\x1B[0;37mWear flags: \x1B[37;1m%s\n\r\x1B[0;37mExtra flags: \x1B[37;1m%s\n\r", wear_bit_name (obj->wear_flags), extra_bit_name (obj->extra_flags));
      send_to_char (buf, ch); if (obj->effects != 0)
      {
        sprintf (buf, "\x1B[0;37mSets player/mob bits: \x1B[37;1m%s\n\r", affect_bit_name (obj->effects));
        send_to_char (buf, ch);
      }
      sprintf (buf, "\x1B[0;37mWeight: \x1B[37;1m%d \x1B[0;37mCost in copper: \x1B[0;33m%d \x1B[0;37mTimer: \x1B[37;1m%d\x1B[0;37m\n\r", obj->weight, obj->cost, obj->timer);
      send_to_char (buf, ch); if (obj->max_in_room != 1)
      {
        sprintf (buf, "Max of this object that pops in a room before no more pop: %d\n\r", obj->max_in_room); send_to_char (buf, ch);
      }
      sprintf 
(buf, "\x1B[0;37mValues: \x1B[0;37mv0 \x1B[1;31m%d \x1B[0;37mv1 \x1B[1;31m%d \x1B[0;37mv2 \x1B[1;31m%d \x1B[0;37mv3 \x1B[1;31m%d \x1B[0;37mv4 \x1B[1;31m%d \x1B[0;37mv5 \x1B[1;31m%d \x1B[0;37mv6 \x1B[1;31m%d \x1B[0;37mv7 \x1B[1;31m%d \x1B[0;37mv8 \x1B[1;31m%d \x1B[0;37mv9 \x1B[1;31m%d\x1B[0;37m\n\r",
          obj->value[0], obj->value[1], obj->value[2],
          obj->value[3], obj->value[4], obj->value[5],
          obj->value[6], obj->value[7], obj->value[8],
          obj->value[9]); 
      send_to_char (buf, ch);
          /*sprintf (buf, "Value2: %d %d %d %d %d %d %d %d %d %d.\n\r",
              obj->values_2[0], obj->values_2[1],
              obj->values_2[2], obj->values_2[3],
              obj->values_2[4], obj->values_2[5],
              obj->values_2[6], obj->values_2[7],
              obj->values_2[8], obj->values_2[9]);
          send_to_char (buf, ch);*/ 
       if (obj->action_descr[0])
          {
            sprintf (buf, "Action Descr1: %s\n\r",
                obj->action_descr[0]); send_to_char (buf, ch);
          }
          if (obj->action_descr[1])
          {
            sprintf (buf, "Action Descr2: %s\n\r",
                obj->action_descr[1]); send_to_char (buf, ch);
          }
          if (obj->action_descr[2])
          {
            sprintf (buf, "Action Descr3: %s\n\r",
                obj->action_descr[2]); send_to_char (buf, ch);
          }
          if (obj->item_type == ITEM_WEAPON)
          {
            send_to_char ("\n\r\x1B[1;30mIn weapons you only use v1 v3 v7 and v8.\n\r", ch);
            send_to_char ("v1 - damage weapon can do before it has a chance to damage means,\n\r", ch);
            send_to_char ("v1 - 5 = if the weapon is a 3d2 and does 5 or 6 damage it could dent.\n\r", ch);
	    send_to_char ("for v3 see help: weapon type or v3.\n\r", ch);
	    send_to_char ("v7 and v8 is the damage the weapon can do. Total of v7 and v8 should \x1B[1;31mNOT \x1B[1;30mbe over 28\x1B[37;0m\n\r", ch);
	  }
          if (obj->item_type == ITEM_ARMOR)
          {
            send_to_char ("\n\r\x1B[1;31mv0 \x1B[1;30m- \x1B[1;37mLeg AC\x1B[1;30m, \x1B[1;31mv1 \x1B[1;30m- \x1B[1;37mBody AC\x1B[1;30m, \x1B[1;31mv2 \x1B[1;30m- \x1B[1;37mHead AC\x1B[1;30m, \x1B[1;31mv3 \x1B[1;30m- \x1B[1;37mArm AC\x1B[1;30m,\n\r", ch);
            send_to_char ("\x1B[1;31mv4 \x1B[1;30m- Pound rating.  10 = normal, 15 = 1.5x protection against pound, etc.\n\r", ch);
            send_to_char ("so v4 10 is normal vs pound, v5 10 is normal vs slash, v8 10 normal vs pierce.\n\r", ch);
            send_to_char ("v5 - Slash rating, \x1B[1;31mv6 \x1B[1;30m- Max condition, \x1B[1;31mv7 \x1B[1;30m- Condition Now, v8 - Pierce rating,\n\r", ch); 
            send_to_char ("v9 - Warmth. \x1B[1;30mAny questions please ask an admin.\n\r", ch);
            send_to_char ("Max armor is \x1B[1;30m10 \x1B[1;30mon single location armor and \x1B[1;30m5 \x1B[1;30mon double location armor.\n\r", ch);
            send_to_char ("\x1B[1;34mSingle location armor \x1B[1;30m= \x1B[1;31mBody, legs, feet, head, arms.\n\r", ch);
            send_to_char ("\x1B[1;34mDouble location armor \x1B[1;30m= \x1B[1;31mearrings, neck, knee, elbow, finger.\x1B[37;0m\n\r", ch);
          }
          if (obj->extra_descr != NULL)
          {
            DESCRIPTION_DATA * ed;
            send_to_char ("Extra description keywords: ", ch);
            for (ed = obj->extra_descr; ed != NULL; ed = ed->next)
            {
              send_to_char ("'", ch);
              send_to_char (ed->keyword, ch); send_to_char ("'", ch);
            }
            send_to_char ("\n\r", ch);
          }
          for (paf = obj->affected; paf != NULL; paf = paf->next)
          {
            sprintf (buf, "Affects %s by %d.\n\r",
                affect_loc_name (paf->location), paf->modifier);
            send_to_char (buf, ch);
          }
          return;
}

void display_resets (CHAR_DATA * ch)
{
  char final[STD_LENGTH];
  char buf[STD_LENGTH];
  ROOM_DATA * pRoom = ch->in_room;
  RESET_DATA * pReset;
  MOB_PROTOTYPE * pMob = NULL;
  MOB_PROTOTYPE * LastMob = NULL;
  OBJ_PROTOTYPE * obj = NULL;
  OBJ_PROTOTYPE * LastObj = NULL;
  OBJ_PROTOTYPE * LastObjPrev = NULL;
  MOB_PROTOTYPE * pMobIndex;
  OBJ_PROTOTYPE * pObjIndex;
  bool last;
  int iReset = 0;
  int olevel = 2;
  if (pRoom == NULL)
    return;
  pMob = NULL; last = TRUE; final[0] = '\0'; if (!pRoom->more)
  {
    send_to_char ("No resets here.\n\r", ch); return;
  }
  for (pReset = pRoom->more->reset_first; pReset != NULL;
      pReset = pReset->next)
  {
    send_to_char (final, ch);
    final[0] = '\0';
    sprintf (final, "\x1B[1m%2d\x1B[0m- ", ++iReset);
    switch (pReset->command)
    {
      default:
        strcat (final, "Invalid Reset Command\n\r"); break; case 'M':
          if ((pMobIndex = get_mob_index (pReset->rs_vnum)) == NULL)
          {
            sprintf (buf, "Load Mobile - Bad Vnum %d - DELETE ME!\n\r",
                pReset->rs_vnum); strcat (final, buf); continue;
          }
        pMob = pMobIndex; sprintf (buf, "Loads \x1B[1m%s\x1B[0m (\x1B[1;36m%d\x1B[0m) in room (max \x1B[1;32m%d\x1B[0m at \x1B[1;32m%d%%\x1B[0m chance)\n\r", pMob->short_descr, pReset->rs_vnum, pReset->loc, pReset->percent); strcat (final, buf); LastObj = NULL; LastObjPrev = NULL; LastMob = pMob; olevel = pMob->level; break; case 'O':
          if ((pObjIndex = get_obj_index (pReset->rs_vnum)) == NULL)
          {
            sprintf (buf, "Load Object - Bad Vnum %d - DELETE ME!\n\r",
                pReset->rs_vnum); strcat (final, buf); continue;
          }
        obj = pObjIndex;
        if (pReset->loc == RESET_LOC_INOBJ && LastObj != NULL)
        {
          sprintf (buf,
              " Loads \x1B[0m%s\x1B[0m (\x1B[1m%d\x1B[0m) inside %s\x1B[0m",
              obj->short_descr, pReset->rs_vnum,
              LastObj ? LastObj->short_descr : "!NO OBJ!");
          strcat (final, buf);
        }
        else
          if (pReset->loc == RESET_LOC_PREVOBJ)
          {
            sprintf (buf, "Drops out to %s\x1B[0m\n\r",
                LastObjPrev ? LastObjPrev->short_descr :
                "!NO OBJ!");
            strcat (final, buf);
            LastObj = LastObjPrev; LastObjPrev = NULL;
          }
          else
            if (pReset->loc == RESET_LOC_ONOBJ && LastObj != NULL)
            {
              sprintf (buf,
                  " Loads %s\x1B[0m (\x1B[1m%d\x1B[0m) on top of %s\x1B[0m",
                  obj->short_descr, pReset->rs_vnum,
                  LastObj ? LastObj->short_descr : "!NO OBJ!");
              strcat (final, buf);
            }
            else
              if (pReset->loc == RESET_LOC_INROOM)
              {
                sprintf (buf, " Loads %s\x1B[0m (\x1B[1m%d\x1B[0m) in room",
                    obj->short_descr, pReset->rs_vnum);
                strcat (final, buf);
              }
              else
                if (LastMob != NULL)
                {
                  if (LastMob->pShop == NULL)
                    sprintf (buf,
                        " Loads %s\x1B[0m (\x1B[1m%d\x1B[0m) on \x1B[1;36m%s\x1B[0m of \x1B[1m%s\x1B[0m",
                        obj->short_descr, pReset->rs_vnum,
                        wear_loc_name (pReset->loc),
                        LastMob ? LastMob->short_descr : "!NO MOB!");
                  else
                    sprintf (buf,
                        " Loads %s\x1B[0m (\x1B[1m%d\x1B[0m) times \x1B[1;36m%d\x1B[0m of \x1B[1m%s\x1B[0m",
                        obj->short_descr, pReset->rs_vnum, pReset->loc,
                        LastMob ? LastMob->short_descr : "!NO MOB!");
                  strcat (final, buf);
                }
                else
                  strcat (final, "Incorrect Assignment of Object\n\r");
        if (!strstr (final, "Incorrect") && !strstr (final, "Drop"))
        {
          sprintf (buf, " \x1B[1;31m%d%%\x1B[0m chance\n\r",
              pReset->percent); strcat (final, buf);
        }
        if (LastObj == NULL || obj->item_type == ITEM_CONTAINER)
        {
          LastObjPrev = LastObj; LastObj = obj;
        }
        break;
    }
  }
  send_to_char (final, ch); final[0] = '\0'; return;
}

void check_reset (CHAR_DATA * ch, RESET_DATA * pReset)
{
  return;
}

void
add_reset (ROOM_DATA * room, RESET_DATA * pReset, int idx)
{
  RESET_DATA * reset;
  int iReset = 0;
  check_room_more (room); if (room->more->reset_first == NULL)
  {
    room->more->reset_first = pReset;
    room->more->reset_last = pReset;
    pReset->next = NULL; return;
  }
  idx--; if (idx == 0)
  {
    pReset->next = room->more->reset_first;
    room->more->reset_first = pReset; return;
  }
  for (reset = room->more->reset_first; reset->next != NULL;
      reset = reset->next)
  {
    if (++iReset == idx) break;
  }
  pReset->next = reset->next;
  reset->next = pReset;
  if (pReset->next == NULL)
    room->more->reset_last = pReset; return;
}

void do_resets (CHAR_DATA * ch, char *argy)
{
  char arg1[SML_LENGTH];
  char arg2[SML_LENGTH];
  char arg3[SML_LENGTH];
  char arg4[SML_LENGTH];
  char arg5[SML_LENGTH];
  char arg6[SML_LENGTH];
  char arg7[SML_LENGTH];
  char arg8[SML_LENGTH];
  char arg9[SML_LENGTH];
  char arg10[SML_LENGTH];
  RESET_DATA * pReset;
  AREA_DATA * pArea;
  int range1, range2, vnum;
  ROOM_DATA * oneroom;
  DEFINE_COMMAND ("resets", do_resets, POSITION_DEAD, 100, LOG_NORMAL, "This command allows you to view or modify resets in the current room.")
    if (IS_MOB (ch)) return;
  if (LEVEL (ch) > 100 && !IS_REAL_GOD (ch)
      && ch->in_room->area->open == 1) return;
  pArea = ch->in_room->area; argy = one_argy (argy, arg1);
  argy = one_argy (argy, arg2); argy = one_argy (argy, arg3);
  if (is_number (arg1) && is_number (arg3)
      && !str_cmp (arg2, "objects"))
  {
    int pcnt = atoi (arg3); while (argy && argy[0] != '\0')
    {
      argy = one_argy (argy, arg4);
      pReset = new_reset_data ();
      pReset->command = 'O';
      pReset->rs_vnum = atoi (arg4);
      pReset->loc = WEAR_NONE;
      pReset->percent = pcnt;
      add_reset (ch->in_room, pReset, atoi (arg1));
    }
    send_to_char ("Resets added.  Please check them.\n\r", ch);
    SET_BIT (pArea->area_flags, AREA_CHANGED); return;
  }

  argy = one_argy (argy, arg4);
  argy = one_argy (argy, arg5);
  argy = one_argy (argy, arg6);
  argy = one_argy (argy, arg7);
  argy = one_argy (argy, arg8);
  argy = one_argy (argy, arg9);
  argy = one_argy (argy, arg10); if (arg1[0] == '\0')
  {
    if (ch->in_room->more
        && ch->in_room->more->reset_first != NULL)
    {
      send_to_char ("Resets:\n\r", ch); display_resets (ch);
    }
    else
      send_to_char ("No resets in this room.\n\r", ch);
  }
  if (!IS_BUILDER (ch, ch->in_room->area))
  {
    send_to_char
      ("Resets: Invalid security for editing this room.\n\r", ch);
    return;
  }
  if (!str_cmp (arg1, "area"))
  {
    if (!ch->in_room || !ch->in_room->area)
      return;
    range1 = ch->in_room->area->lvnum;
    range2 = ch->in_room->area->uvnum;
    for (vnum = range1; vnum <= range2; vnum++)
      if ((oneroom = get_room_index (vnum)) != NULL)
        reset_room (oneroom);
    send_to_char (ch->in_room->area->repop, ch); return;
  }
  if (!str_cmp (arg1, "room") && arg2[0] == '\0')
  {
    reset_room (ch->in_room); return;
  }
  if (is_number (arg1))
  {
    ROOM_DATA * pRoom = ch->in_room;
    check_room_more (ch->in_room);
    if (!str_cmp (arg2, "delete"))
    {
      int insert_loc = atoi (arg1);
      if (ch->in_room->more->reset_first == NULL)
      {
        send_to_char ("No resets in this room.\n\r", ch); return;
      }
      if (insert_loc - 1 <= 0)
      {
        pReset = pRoom->more->reset_first;
        pRoom->more->reset_first = pRoom->more->reset_first->next;
        if (pRoom->more->reset_first == NULL)
          pRoom->more->reset_last = NULL;
      }
      else
      {
        int iReset = 0;
        RESET_DATA * prev = NULL;
        for (pReset = pRoom->more->reset_first; pReset != NULL;
            pReset = pReset->next)
        {
          if (++iReset == insert_loc) break; prev = pReset;
        }
        if (pReset == NULL)
        {
          send_to_char ("Reset not found.\n\r", ch); return;
        }
        if (prev != NULL) prev->next = prev->next->next;
        else
          pRoom->more->reset_first = pRoom->more->reset_first->next;
        for (pRoom->more->reset_last = pRoom->more->reset_first;
            pRoom->more->reset_last->next != NULL;
            pRoom->more->reset_last =
            pRoom->more->reset_last->next);
      }
      free_reset_data (pReset);
      SET_BIT (pArea->area_flags, AREA_CHANGED);
      send_to_char ("Reset deleted.\n\r", ch);
    }
    else
      if ((!str_cmp (arg2, "mob") && is_number (arg3))
          || (!str_cmp (arg2, "obj") && is_number (arg3))
          || !str_cmp (arg2, "back"))
      {
        pReset = new_reset_data (); if (!str_cmp (arg2, "mob"))
        {
          pReset->command = 'M';
          pReset->rs_vnum = atoi (arg3);
          pReset->percent = is_number (arg4) ? atoi (arg4) : 100;
          pReset->loc = is_number (arg5) ? atoi (arg5) : 1;
        }
        else
          if (!str_cmp (arg2, "obj"))
          {
            pReset->command = 'O';
            pReset->rs_vnum = atoi (arg3);
            if (!str_cmp (arg4, "inside") || !str_cmp (arg4, "in"))
              pReset->loc = RESET_LOC_INOBJ;
            else
              if (!str_cmp (arg4, "on")) pReset->loc = RESET_LOC_ONOBJ;
              else
                if (!str_cmp (arg4, "room")) pReset->loc = RESET_LOC_INROOM;
                else
                  pReset->loc = wear_name_loc (arg4);
            pReset->percent = is_number (arg5) ? atoi (arg5) : 75;
          }
          else
            if (!str_cmp (arg2, "back"))
            {
              pReset->command = 'O';
              pReset->vnum = 1;
              pReset->loc = RESET_LOC_PREVOBJ; pReset->percent = 100;
            }
        add_reset (ch->in_room, pReset, atoi (arg1));
        SET_BIT (pArea->area_flags, AREA_CHANGED);
        send_to_char ("Reset added.\n\r", ch);
      }
      else
      {
        send_to_char
          ("Syntax: RESET <number> OBJ <vnum> <location> [chance]\n\r", ch);
        send_to_char
          (" RESET <number> MOB <vnum> [<chance ><max #>]\n\r", ch);
        send_to_char (" RESET <number> DELETE\n\r", ch);
      }
  }
  return;
}

void do_astat (CHAR_DATA * ch, char *argy)
{
  char arg1[SML_LENGTH];
  char buf[STD_LENGTH];
  AREA_DATA * pArea;
  DEFINE_COMMAND ("astat", do_astat, POSITION_DEAD, 110, LOG_NORMAL, "Shows the specified area's complete stats.")
    smash_tilde (argy); strcpy (arg1, argy);
  if (is_number (arg1)) pArea = get_area_data (atoi (arg1));
  else
    pArea = ch->in_room->area;
  if (!pArea)
    pArea = ch->in_room->area;
  sprintf (buf, "Name: '%s'\n\r", pArea->name);
  send_to_char (buf, ch);
  sprintf (buf, "File: '%s'\n\r", pArea->filename);
  send_to_char (buf, ch);
  sprintf (buf, "Can_quit: %s\n\r",
      (pArea->can_quit ==
       1 ? "Players can quit anywhere in this area."
       : (pArea->can_quit ==
         2 ? "Players cannot quit in this area at all."
         : "Normal quit rules within this area.")));
  send_to_char (buf, ch);
  sprintf (buf, "JAIL vnum: %d. OUTJAIL vnum: %d.\n\r",
      pArea->jail, pArea->outjail);
  send_to_char (buf, ch);
  sprintf (buf, "Clan Zone? %s.\n\r",
      (pArea->clanzone ? "YES" : "NO"));
  send_to_char (buf, ch);
  sprintf (buf, "Players: %d\n\r", pArea->nplayer);
  send_to_char (buf, ch);
  sprintf (buf, "Descripted Rooms: %d Weather change: %d\n\r",
      pArea->desc_rooms, pArea->weather_change);
  send_to_char (buf, ch);
  sprintf (buf, "More_rainy: %s More_dry: %s More_foggy: %s\n\r",
      (pArea->more_rainy ? "Yes" : "No"),
      (pArea->more_dry ? "Yes" : "No"),
      (pArea->more_fog ? "Yes" : "No"));
  send_to_char (buf, ch);
  sprintf (buf, "Security: %d; Builders '%s'\n\r",
      pArea->security, pArea->builders);
  send_to_char (buf, ch);
  sprintf (buf, "Vnums: %d-%d\n\r", pArea->lvnum,
      pArea->uvnum); send_to_char (buf, ch);
      sprintf (buf, "\x1B[1;34mRepop: \x1B[1;30m%s\x1B[0;37m\n\r", pArea->repop);
      send_to_char (buf, ch);
      sprintf (buf, "Flags: %s\n\r",
          area_bit_name (pArea->area_flags));
      send_to_char (buf, ch);
      sprintf (buf, "Open: %s\n\r", (pArea->open ? "Yes" : "No"));
      send_to_char (buf, ch);
      sprintf (buf, "Room count: %d\n\r", pArea->room_count);
      send_to_char (buf, ch);
      sprintf (buf, "Mob count: %d\n\r", pArea->mob_count);
      send_to_char (buf, ch);
      sprintf (buf, "Obj count: %d\n\r", pArea->obj_count);
      send_to_char (buf, ch);
      sprintf (buf, "Repop: %d/%d\n\r", pArea->repop_counter,
          pArea->repop_rate); send_to_char (buf, ch);
          return;
}

void save_area_list ()
{
  FILE * fp;
  AREA_DATA * pArea;
  if ((fp = fopen ("playarea.lst", "w")) == NULL)
  {
    bug ("Save_area_list: fopen", 0); perror ("area.lst");
  }
  else
  {
    fprintf (fp, "help.are\n");
    for (pArea = area_first; pArea != NULL; pArea = pArea->next)
    {
      fprintf (fp, "%s\n", fix_string (pArea->filename));
    }
    fprintf (fp, "$\n"); fclose (fp);
  }
  return;
}

char *fix_string (const char *str)
{
  static char strfix[10000];
  char *s = NULL;
  char *t;
  int i; int o; if (str == NULL) return '\0'; if (str[0] == 1)
  {
    s = decompress ((char *) str);
  }
  if (s) t = s;
  else
    t = (char *) str; for (o = i = 0; t[i + o] != '\0'; i++)
    {
      if (t[i + o] == '\r' || t[i + o] == '~')
        o++; strfix[i] = t[i + o];
    }
  strfix[i] = '\0'; return strfix;
}

void save_mobiles (FILE * fp, AREA_DATA * pArea)
{
  int iHash;
  MOB_PROTOTYPE * pMobIndex;
  int iTrade;
  int tmm;
  fprintf (fp, "#MOBDATA\n");
  for (iHash = 0; iHash < HASH_MAX; iHash++)
  {
    for (pMobIndex = mob_index_hash[iHash]; pMobIndex != NULL;
        pMobIndex = pMobIndex->next)
    {
      if (pMobIndex != NULL && pMobIndex->area == pArea &&
          pMobIndex->guard != -32000)
      {
        fprintf (fp, "#%d\n", pMobIndex->vnum);
        fprintf (fp, "Name %s~\n", fix_string (pMobIndex->name));
        fprintf (fp, "Short %s~\n",
            fix_string (pMobIndex->short_descr));
        fprintf (fp, "Long \n%s~\n",
            fix_string (pMobIndex->long_descr));
        fprintf (fp, "Descr\n%s~\n",
            fix_string (pMobIndex->description));
        if (pMobIndex->attackname
            && str_cmp ("N/A", pMobIndex->attackname)) fprintf (fp,
            "Attackmsg %s~\n",
            fix_string
            (pMobIndex->
             attackname));
        fprintf (fp, "AMax %d\n", pMobIndex->max_in_world);
        if (pMobIndex->points != 0)
        {
          fprintf (fp, "JPts %d\n", pMobIndex->points);
        }
        if (pMobIndex->java[0] != '\0')
        {
          fprintf (fp, "JavO %d\n", pMobIndex->java[0]);
          fprintf (fp, "JavT %d\n", pMobIndex->java[1]);
        }
        if (pMobIndex->yeller_number != 0)
        {
          fprintf (fp, "AYeller %d\n", pMobIndex->yeller_number);
          fprintf (fp, "AWillhelp %d\n", pMobIndex->will_help);
        }
        if (pMobIndex->height != -1)
          fprintf (fp, "NHeight %d\n", pMobIndex->height);
        if (pMobIndex->clan_guard_1 > 0)
          fprintf (fp, "AClan %d\n", pMobIndex->clan_guard_1);
        if (pMobIndex->clan_guard_2 > 0)
          fprintf (fp, "AClan2 %d\n", pMobIndex->clan_guard_2);
        if (pMobIndex->act != 0)
          fprintf (fp, "ActBits %d\n", pMobIndex->act);
        if (pMobIndex->act3 != 0)
          fprintf (fp, "ActBits3 %d\n", pMobIndex->act3);
        if (pMobIndex->act4 != 0)
          fprintf (fp, "ActBits4 %d\n", pMobIndex->act4);
        if (pMobIndex->affected_by != 0)
          fprintf (fp, "Affect_By %d\n", pMobIndex->affected_by);
        if (pMobIndex->more_affected_by != 0)
          fprintf (fp, "AffectTWO %d\n", pMobIndex->more_affected_by);
        if (pMobIndex->alignment != 0)
          fprintf (fp, "Alignment %d\n", pMobIndex->alignment);
        if (pMobIndex->money != 0)
          fprintf (fp, "Money %d\n", pMobIndex->money);
        fprintf (fp, "Level %d\n", pMobIndex->level);
        if (pMobIndex->sex != 0)
          fprintf (fp, "Sex %d\n", pMobIndex->sex);
        if (pMobIndex->mobtype != -1)
          fprintf (fp, "MobType %d\n", pMobIndex->mobtype);
        if (pMobIndex->alt_vnum != 0)
          fprintf (fp, "Alt_vnum %d\n", pMobIndex->alt_vnum);
        if (pMobIndex->hpmult != 10)
          fprintf (fp, "Hp %d\n", pMobIndex->hpmult);
        if (pMobIndex->guard != -1)
          fprintf (fp, "Guard %d\n", pMobIndex->guard);
        if (pMobIndex->hitroll != 0)
          fprintf (fp, "GHitR %d\n", pMobIndex->hitroll);
        if (pMobIndex->damroll != 0)
          fprintf (fp, "GDamR %d\n", pMobIndex->damroll);
        if (pMobIndex->dodge != 0)
          fprintf (fp, "GDod %d\n", pMobIndex->dodge);
        if (pMobIndex->parry != 0)
          fprintf (fp, "GPar %d\n", pMobIndex->parry);
        if (pMobIndex->spec_fun != NULL)
          fprintf (fp, "Spec %s\n",
              fix_string (pMobIndex->spec_name));
        if (pMobIndex->pShop != NULL)
        {
          fprintf (fp, "Shop ");
          for (iTrade = 0; iTrade < MAX_TRADE; iTrade++)
          {
            fprintf (fp, "%4d", pMobIndex->pShop->buy_type[iTrade]);
          }
          fprintf (fp, " %d %d %d %d\n", pMobIndex->pShop->profit_buy,
              pMobIndex->pShop->profit_sell,
              pMobIndex->pShop->open_hour,
              pMobIndex->pShop->close_hour);
          fprintf (fp, "Shopd2 %d\n", pMobIndex->pShop->decrease);
          fprintf (fp, "Shopd3 %d\n",
              pMobIndex->pShop->items_until_decrease);
          if (pMobIndex->pShop->creates_vnum != 0) fprintf (fp,
              "ShopCr %d\n",
              pMobIndex->
              pShop->
              creates_vnum);
          if (pMobIndex->pShop->creates_message != NULL) fprintf (fp,
              "ShopCS %s~\n",
              fix_string
              (pMobIndex->
               pShop->
               creates_message));
          if (pMobIndex->pShop->creates_hours != 1) fprintf (fp,
              "ShCrh %d\n",
              pMobIndex->
              pShop->
              creates_hours);
        }
        if (pMobIndex->race_hate != 0) fprintf (fp, "RaceHate %d\n",
            pMobIndex->
            race_hate);
        if (pMobIndex->armcls != 101) fprintf (fp,
            "ArmorClass %d\n",
            pMobIndex->armcls);
        if (pMobIndex->opt)
        {
          tmm = 0; while (tmm < 10)
          {
            if (pMobIndex->opt->cast_spells[tmm] != NULL)
            {
              fprintf (fp, "BCasts %s~\n",
                  fix_string (pMobIndex->opt->cast_spells[tmm]));
            }
            tmm++;
          }
          if (pMobIndex->opt->cast_percent > 0)
          {
            fprintf (fp, "BCast_P %d\n", pMobIndex->opt->cast_percent);
          }
          if (pMobIndex->opt->kicks)
          {
            fprintf (fp, "BKicks %d\n", pMobIndex->opt->kicks);
          }
          if (pMobIndex->opt->flurries)
          {
            fprintf (fp, "BFlurries %d\n", pMobIndex->opt->flurries);
          }
          if (pMobIndex->opt->tackles)
          {
            fprintf (fp, "BTackles %d\n", pMobIndex->opt->tackles);
          }
          if (pMobIndex->opt->warrior_percent > 0)
          {
            fprintf (fp, "BWarrior_P %d\n",
                pMobIndex->opt->warrior_percent);
          }
          tmm = 0; while (tmm < 30)
          {
            if (pMobIndex->opt->skltaught[tmm] != 0)
            {
              fprintf (fp, "ASklTaught %d %d\n", tmm,
                  pMobIndex->opt->skltaught[tmm]);
            }
            tmm++;
          }
        }

        if (pMobIndex->script_fn != NULL)
          fprintf (fp, "SFT %s\n", fix_string (pMobIndex->script_fn));
        fprintf (fp, "End\n\n");
      }
    }
  }
  fprintf (fp, "#0\n\n\n\n"); return;
}

void save_objects (FILE * fp, AREA_DATA * pArea)
{
  int iHash;
  OBJ_PROTOTYPE * pObjIndex;
  AFFECT_DATA * pAf;
  DESCRIPTION_DATA * pEd;
  int tmpi;
  bool fnd;
  fprintf (fp, "#OBJDATA\n");
  for (iHash = 0; iHash < HASH_MAX; iHash++)
  {
    for (pObjIndex = obj_index_hash[iHash]; pObjIndex != NULL;
        pObjIndex = pObjIndex->next)
    {
      if (pObjIndex != NULL && pObjIndex->area == pArea &&
          pObjIndex->item_type != 99999)
      {
        fprintf (fp, "#%d\n", pObjIndex->vnum);
        fprintf (fp, "Name %s~\n", fix_string (pObjIndex->name));
        fprintf (fp, "Short %s~\n",
            fix_string (pObjIndex->short_descr));
        if (pObjIndex->effects != 0) fprintf (fp, "LEffect %d\n",
            pObjIndex->effects);
        if (pObjIndex->java[0] != '\0')
        {
          fprintf (fp, "JavO %d\n", pObjIndex->java[0]);
          fprintf (fp, "JavT %d\n", pObjIndex->java[1]);
        }
        if (pObjIndex->made_of != 0)
        {
          fprintf (fp, "DMat %d\n", pObjIndex->made_of);
        }
        fprintf (fp, "Descr\n%s~\n",
            fix_string (pObjIndex->description));
        if (pObjIndex->action_descr[0]) fprintf (fp,
            "DescA1\n%s~\n",
            fix_string
            (pObjIndex->
             action_descr[0]));
        if (pObjIndex->action_descr[1]) fprintf (fp,
            "DescA2\n%s~\n",
            fix_string
            (pObjIndex->
             action_descr[1]));
        if (pObjIndex->action_descr[2]) fprintf (fp,
            "DescA3\n%s~\n",
            fix_string
            (pObjIndex->
             action_descr[2]));
        fprintf (fp, "Type %d\n", pObjIndex->item_type);
        if (pObjIndex->extra_flags != 0) fprintf (fp, "Extra %d\n",
            pObjIndex->
            extra_flags);
        if (pObjIndex->max_in_room != 1) fprintf (fp,
            "Maxinroom %d\n",
            pObjIndex->
            max_in_room);
        fprintf (fp, "Wear %d\n", pObjIndex->wear_flags);
        if (pObjIndex->timer != 0) fprintf (fp, "Timer %d\n",
            pObjIndex->timer);
        fnd = FALSE;
        fprintf (fp, "Values2 %d %d %d %d %d %d %d %d %d %d\n",
            pObjIndex->values_2[0], pObjIndex->values_2[1],
            pObjIndex->values_2[2], pObjIndex->values_2[3],
            pObjIndex->values_2[4], pObjIndex->values_2[5],
            pObjIndex->values_2[6], pObjIndex->values_2[7],
            pObjIndex->values_2[8], pObjIndex->values_2[9]);
        for (tmpi = 0; tmpi < 10;
            tmpi++) if (pObjIndex->value[tmpi] != 0)
            {
              fnd = TRUE; break;
            }
            if (fnd)
            {
              fprintf (fp, "Values %d %d %d %d %d %d %d %d %d %d\n",
                  pObjIndex->value[0],
                  pObjIndex->value[1],
                  pObjIndex->value[2],
                  pObjIndex->value[3],
                  pObjIndex->value[4],
                  pObjIndex->value[5],
                  pObjIndex->value[6],
                  pObjIndex->value[7],
                  pObjIndex->value[8], pObjIndex->value[9]);
            }
            /*end if fnd */
            fprintf (fp, "Weight %d\n", pObjIndex->weight);
            fprintf (fp, "Cost %d\n", pObjIndex->cost);
            for (pAf = pObjIndex->affected; pAf != NULL;
                pAf = pAf->next)
            {
              fprintf (fp, "Affect %d %d %d %d %d\n", pAf->location,
                  pAf->modifier,
                  pAf->type, pAf->duration, pAf->bitvector);
            }
            for (pEd = pObjIndex->extra_descr; pEd != NULL;
                pEd = pEd->next)
            {
              fprintf (fp, "ExtraDescr %s~\n%s~\n", pEd->keyword,
                  pEd->description);
            }
            if (pObjIndex->how_many > 0)
              fprintf (fp, "Limtd %d\n", pObjIndex->how_many);
            fprintf (fp, "End\n\n");
      }
    }
  }
  fprintf (fp, "#0\n\n\n\n"); return;
}

void save_rooms (FILE * fp, AREA_DATA * pArea)
{
  int iHash;
  ROOM_DATA * oneroom;
  DESCRIPTION_DATA * pEd;
  RESET_DATA * pReset;
  EXIT_DATA * pExit;
  int door;
  fprintf (fp, "#ROOMDATA\n");
  for (iHash = 0; iHash < HASH_MAX; iHash++)
  {
    for (oneroom = room_hash[iHash]; oneroom != NULL;
        oneroom = oneroom->next)
    {
      if ((!oneroom->more
            || ((oneroom->more && oneroom->more->pcs != 31999)
              && oneroom->area != NULL))
          && oneroom->area == pArea)
      {
        fprintf (fp, "#%d\n", oneroom->vnum);
        fprintf (fp, "Name %s~\n", fix_string (oneroom->name));
        if (oneroom->img[1] != 0)
        {
          fprintf (fp, "NJAO %d\n", oneroom->img[0]);
          fprintf (fp, "NJAT %d\n", oneroom->img[1]);
        }
        if (oneroom->a != '\0')
          fprintf (fp, "NAP %d\n", oneroom->a);
        if (oneroom->c != '\0')
          fprintf (fp, "NAC %d\n", oneroom->c);
        /*             if (oneroom->tps!=0)
                       {
                       fprintf( fp, "NTPS %d\n", oneroom->tps);
                       }
         */
        if (oneroom->description == NULL)
          oneroom->description = str_dup ("Blank Description.");
        fprintf (fp, "Descr\n%s~\n",
            fix_string (oneroom->description));
        if (oneroom->room_flags_2 != 0) fprintf (fp, "Flags2 %d\n",
            oneroom->
            room_flags_2);
        if (oneroom->room_flags != 0) fprintf (fp, "Flags %d\n",
            oneroom->room_flags);
        if (oneroom->shade) fprintf (fp, "Shd %d\n",
            oneroom->shade);
        if (oneroom->sector_type != 1) fprintf (fp, "Sector %d\n",
            oneroom->
            sector_type);
        if (oneroom->more) for (pEd = oneroom->more->extra_descr;
            pEd != NULL; pEd = pEd->next)
        {
          fprintf (fp, "ExtraDescr %s~\n%s~\n", pEd->keyword,
              pEd->description);
        }
        if (oneroom->more)
          for (pReset = oneroom->more->reset_first; pReset != NULL;
              pReset = pReset->next)
          {
            if (pReset != NULL)
            {
              fprintf (fp, "Reset %c %d %d %d\n", pReset->command,
                  pReset->rs_vnum, pReset->loc, pReset->percent);
            }
          }

        if (oneroom->more)
        {
          if (oneroom->more->move_dir != 0)
            fprintf (fp, "Move_dir %d\n", oneroom->more->move_dir);
          if (oneroom->more->obj_description
              && oneroom->more->obj_description[0] !=
              '\0') fprintf (fp, "Mdescript\n%s~\n",
                fix_string (oneroom->more->
                  obj_description));
          if (oneroom->more->can_travel_sectors)
          {
            fprintf (fp, "Mcts %d\n",
                oneroom->more->can_travel_sectors);
          }
          if (oneroom->more->orig_room > 1)
          {
            fprintf (fp, "Mor %d\n", oneroom->more->orig_room);
          }
          if (oneroom->more->linked_to)
          {
            fprintf (fp, "Mlt %d\n", oneroom->more->linked_to->vnum);
          }
          if (oneroom->more->move_message
              && str_cmp (oneroom->more->move_message, "None.")
              && oneroom->more->move_message[0] != '\0') fprintf (fp,
                "Movemess\n%s~\n",
                fix_string
                (oneroom->
                 more->
                 move_message));
        }
        for (door = 0; door <= 5; door++)
        {
          if ((pExit = oneroom->exit[door]) != NULL
              /*&& pExit->vnum > 0
                && get_room_index( pExit->vnum ) != NULL */ )
          {
            fprintf (fp, "Door %d %d %d %d %d %d\n", door,
                (pExit->d_info ? pExit->d_info->rs_flags : 0),
                (pExit->d_info ? pExit->d_info->key : 0),
                (pExit->to_room ==
                 NULL ? 0 : ((ROOM_DATA *) pExit->to_room)->vnum),
                (pExit->d_info ? pExit->d_info->maxstr : 0),
                (pExit->d_info ? pExit->d_info->maxstr : 0));
            if (pExit->d_info
                && pExit->d_info->description ==
                NULL) pExit->d_info->description = str_dup ("");
            if (pExit->d_info) fprintf (fp, "%s~\n",
                fix_string (pExit->d_info->
                  description));
            else
              fprintf (fp, "~\n"); if (pExit->d_info)
              {
                if (pExit->d_info->keyword == NULL) fprintf (fp, "~\n");
                else
                  fprintf (fp, "%s~\n", fix_string (pExit->d_info->keyword));
              }
            else
              fprintf (fp, "~\n");
          }
        }
        fprintf (fp, "End\n\n");
      }
    }
  }
  fprintf (fp, "#0\n\n\n\n"); return;
}

void save_area (AREA_DATA * pArea)
{
  FILE * fp; if (builder_port && pArea->open) return;	/* Don't save open areas if we're on builder port! */
  if ((fp = fopen (pArea->filename, "w")) == NULL)
  {
    bug ("Open_area: fopen", 0); perror (pArea->filename);
  }
  fprintf (fp, "#AREADATA \n");
  fprintf (fp, "Name %s~\n", pArea->name);
  fprintf (fp, "Repop \x1B[1;30m%s~\x1B[0;37m\n", fix_string (pArea->repop));
  if (pArea->repop_rate != 24)
    fprintf (fp, "SRepop %d\n", pArea->repop_rate);
  fprintf (fp, "SJail %d\n", pArea->jail);
  fprintf (fp, "SOutJail %d\n", pArea->outjail);
  fprintf (fp, "SCZone %d\n", pArea->clanzone);
  fprintf (fp, "Builders %s~\n",
      fix_string (pArea->builders));
  fprintf (fp, "VNUMs %d %d\n", pArea->lvnum, pArea->uvnum);
  fprintf (fp, "NQuit %d\n", pArea->can_quit);
  fprintf (fp, "Open %d\n", pArea->open);
  fprintf (fp, "End\n\n\n\n"); save_mobiles (fp, pArea);
  save_objects (fp, pArea); save_rooms (fp, pArea);
  fprintf (fp, "#$\n"); fclose (fp); return;
}

void save_playerbase (void)
{
  FILE * fp;
  PLAYERBASE_DATA * player;
  if ((fp = fopen ("pbase.dat", "w")) == NULL)
  {
    fprintf (stderr, "Error on playerbase write.\n"); exit (2);
  }
  for (player = playerbase_zero; player != NULL;
      player = player->next)
  {
    if (player->player_level == -1)
      continue;
    if (!str_cmp (player->player_name, "Sabelis"))
      continue;
    if (!str_cmp (player->player_name, "Kilith"))
      continue;
    fprintf (fp, "%s\n", player->player_name);
    fprintf (fp, "%s\n", player->email_address);
    fprintf (fp, "%ld\n", player->player_last_login);
    fprintf (fp, "%d\n", player->player_good);
    fprintf (fp, "%d\n", player->player_level);
    fprintf (fp, "%d\n", player->player_nodelete);
  }
  fprintf (fp, "\nEND\n"); fclose (fp); return;
}

void save_topten (void)
{
  FILE * fp;
  int k; if ((fp = fopen ("topten.dat", "w")) == NULL)
  {
    fprintf (stderr, "Error on topten write.\n"); exit (2);
  }
  for (k = 0; k < MAX_TOPTEN; k++)
  {
    fprintf (fp, "%s\n", topten.topten_name[k]);
    fprintf (fp, "%d\n", topten.topten_warpoints[k]);
    fprintf (fp, "%d\n", topten.topten_good[k]);
  }
  fprintf (fp, "\nEND\n");
  fclose (fp);
  return;
}

void save_rating (void)
{
  FILE * fp;
  int k; if ((fp = fopen ("rating.dat", "w")) == NULL)
  {
    fprintf (stderr, "Error on rating write.\n"); exit (2);
  }
  for (k = 0; k < MAX_RATING; k++)
  {
    fprintf (fp, "%s\n", ratingd.rating_name[k]);
    fprintf (fp, "%d\n", ratingd.rating_rating[k]);
    fprintf (fp, "%d\n", ratingd.rating_good[k]);
  }
  fprintf (fp, "\nEND\n"); 
  fclose (fp); 
  return;
}

void save_banlist (void)
{
  FILE * fp;
  BAN_DATA * pBan;
  if ((fp = fopen ("bansite.dat", "w")) == NULL)
  {
    fprintf (stderr, "Error on bansite write.\n"); exit (2);
  }
  for (pBan = ban_list; pBan != NULL; pBan = pBan->next)
  {
    fprintf (fp, "%s\n", pBan->name);
    fprintf (fp, "%d\n", pBan->newbie);
  }
  fprintf (fp, "\nEND\n");
  fclose (fp);
  return;
}

#ifdef OLD_LIMIT_FORMAT
void save_limited (void)
{
  FILE * fpt;
  int k; if ((fpt = fopen ("limited.dat", "w")) == NULL)
  {
    fprintf (stderr, "Error on limited write.\n"); exit (2);
  }
  for (k = 0; k < MAXLIMITED; k++)
  {
    fprintf (fpt, "%s\n", limited.limitkey[k]);
  }
  fprintf (fpt, "\nEND\n"); fclose (fpt); return;
}

#else
void save_limited (int vnum)
{
  FILE * fpt;
  char fn[60];
  sprintf (fn, "../limited/%d", vnum);
  if ((fpt = fopen (fn, "w")) == NULL)
  {
    fprintf (stderr, "Error on limited write.\n"); exit (2);
  }
  fprintf (fpt, "%d\n", limited.limited_array[vnum]);
  fclose (fpt); return;
}

#endif
void save_clans (void)
{
  int iHash;
  CLAN_DATA * clan;
  int i;
  FILE * fp; if ((fp = fopen ("pclans.dat", "w")) == NULL)
  {
    bug ("Open_clans: fopen", 0); perror ("clans.dat");
  }
  for (iHash = 0; iHash < 20; iHash++)
  {
    for (clan = clan_index_hash[iHash]; clan != NULL;
        clan = clan->next)
    {
      if (clan != NULL)
      {
        fprintf (fp, "#%d\n", clan->vnum);
        fprintf (fp, "Name %s~\n", fix_string (clan->name));
        fprintf (fp, "Leader %s\n", clan->leader);
        fprintf (fp, "Minlev %d\n", clan->minlev);
        fprintf (fp, "MStartV %d\n", clan->start_vnum);
        fprintf (fp, "Alignment %d\n", clan->alignment);
        for (i = 0; i < 100; i++)
        {
          if (clan->members[i] != NULL && clan->members[i][0] != ' ')
          {
            fprintf (fp, "CMember %s\n", clan->members[i]);
          }
        }
        fprintf (fp, "End\n\n");
      }
    }
  }
  fprintf (fp, "#0\n\n\n\n");
  fprintf (fp, "#$\n"); fclose (fp); return;
}

void save_clans_2 (void)
{
  int iHash;
  CLAN_DATA * clan;
  int i;
  FILE * fp; if ((fp = fopen ("pclans2.dat", "w")) == NULL)
  {
    bug ("Open_clans_2: fopen", 0); perror ("pclans2.dat");
  }
  for (iHash = 0; iHash < 20; iHash++)
  {
    for (clan = clan_index_hash_2[iHash]; clan != NULL;
        clan = clan->next)
    {
      if (clan != NULL)
      {
        fprintf (fp, "#%d\n", clan->vnum);
        fprintf (fp, "Name %s~\n", fix_string (clan->name));
        fprintf (fp, "Leader %s\n", clan->leader);
        fprintf (fp, "Minlev %d\n", clan->minlev);
        fprintf (fp, "MStartV %d\n", clan->start_vnum);
        fprintf (fp, "Alignment %d\n", clan->alignment);
        for (i = 0; i < 100; i++)
        {
          if (clan->members[i] != NULL && clan->members[i][0] != ' ')
          {
            fprintf (fp, "CMember %s\n", clan->members[i]);
          }
        }
        fprintf (fp, "End\n\n");
      }
    }
  }
  fprintf (fp, "#0\n\n\n\n");
  fprintf (fp, "#$\n"); fclose (fp); return;
}



void do_asave (CHAR_DATA * ch, char *argy)
{
  char arg1[SML_LENGTH];
  AREA_DATA * pArea;
  FILE * fp;
  int value;
  DEFINE_COMMAND ("asave", do_asave, POSITION_DEAD, 103, LOG_ALWAYS, "This command is used to save the world, or sections of it.")
  fp = NULL; 
  if (saving != NULL)
  {
    send_to_char ("A world save is in progress. You may not save at this time.\n\r", ch); return;
  }
  if (ch == NULL)	/* Do an autosave */
  {
    save_area_list ();
    for (pArea = area_first; pArea != NULL; pArea = pArea->next)
    {
      save_area (pArea);
      REMOVE_BIT (pArea->area_flags, AREA_CHANGED);
    }
    return;
  }
  smash_tilde (argy);
  strcpy (arg1, argy); if (arg1[0] == '\0')
  {
    send_to_char ("Syntax:\n\r", ch);
    send_to_char (" asave changed  - saves all changed areas, code, triggers and spells\n\r", ch);
    send_to_char (" asave zone     - saves the area list and the area you're in\n\r", ch);
    send_to_char (" asave world    - begins a background world save\n\r", ch);
    send_to_char (" asave complete - begins a foreground world save (LAGGY)\n\r", ch);
    send_to_char (" asave list     - saves the area list\n\r", ch);
    send_to_char (" asave clans    - saves all clans\n\r", ch);
    send_to_char (" asave rand     - saves all random pops\n\r", ch);
    send_to_char (" asave code     - saves all code labels\n\r", ch);
    send_to_char (" asave triggers - saves all triggers\n\r", ch);
    send_to_char ("\n\r", ch); return;
  }
  /*
  * Snarf the value (which need not be numeric).
   */
  value = atoi (arg1);
  if ((pArea = get_area_data (value)) == NULL
      && is_number (arg1))
  {
    send_to_char ("That area does not exist.\n\r", ch); return;
  }
  if (is_number (arg1))
  {
    save_area_list (); save_area (pArea); return;
  }
  /*
  if (!str_cmp ("tps", arg1))
  {
    save_tps ();
    send_to_char ("Successfully saved tps status.\n\r", ch);
    return;
  }
  */
  if (!str_cmp ("code", arg1))
  {
    save_code ();
    send_to_char ("Successfully saved all code labels.\n\r",
        ch); return;
  }
  if (!str_cmp ("triggers", arg1))
  {
    save_triggers ();
    send_to_char ("Successfully saved all triggers.\n\r", ch);
    return;
  }
  if (!str_cmp ("rand", arg1))
  {
    save_random_pops ();
    send_to_char ("Successfully saved random pop status.\n\r",
        ch); return;
  }
  if (!str_cmp ("world", arg1) || !str_cmp ("all", arg1))
  {
    if (LEVEL (ch) < 110 || str_cmp ("Kilith", NAME (ch)))
    {
      send_to_char ("This command has been disabled.\n\r", ch);
      return;
    }
    save_area_list (); saving = area_first;
    /*
       for( pArea = area_first; pArea != NULL; pArea = pArea->next )
       {
       save_area( pArea );
       REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
       }
       send_to_char( "You saved the world.\n\r", ch );
     */
    do_global
      ("Full World/Database save has started... Please pardon any slight lag.\n\r",
       0, 0); return;
  }
  if (!str_cmp ("complete", arg1))
  {
    save_code ();
    save_triggers ();
    do_asave (ch, "clans");
    save_random_pops (); if (changed_spells)
    {
      send_to_char (" \x1B[1;32m-------> \x1B[1;34mSpell Changes Have Now Been Saved.\x1B[37;0m\n\r", ch);
      changed_spells = FALSE; write_spells ();
    }
    //do_asave (ch, "tps");
    save_area_list ();
    for (pArea = area_first; pArea != NULL; pArea = pArea->next)
    {
      save_area (pArea);
      REMOVE_BIT (pArea->area_flags, AREA_CHANGED);
    }
    send_to_char ("World save complete.\n\r", ch); return;
  }

  if (!str_cmp ("clans", arg1))
  {
    save_clans ();
    save_clans_2 ();
    send_to_char ("\x1B[1;34mAll clans have now been saved..\x1B[37;0m\n\r", ch); return;
  }
  if (!str_cmp ("changed", arg1))
  {
    char buf[SML_LENGTH];
    /*save_area_list(); */
    save_code ();
    save_triggers ();
    //do_asave (ch, "tps");
    send_to_char ("\x1B[1;31mSaved zones\x1B[1;32m:\x1B[37;0m\n\r", ch);
    if (changed_spells)
    {
      send_to_char ("\n\r\x1B[1;32m -------> \x1B[1;34mSpell Changes Have Now Been Saved.\n\r\x1B[37;0m", ch);
      changed_spells = FALSE;
      write_spells ();
    }
    sprintf (buf, "\x1B[1;32mNone\x1B[37;0m.\n\r"); for (pArea = area_first;
        pArea != NULL;
        pArea = pArea->next)
    {
      if (IS_SET (pArea->area_flags, AREA_CHANGED))
      {
        save_area (pArea);
        sprintf (buf, "\x1B[1;34m%24s \x1B[0;37m- '\x1B[1;31m%s\x1B[0;37m'\x1B[37;0m\n\r", pArea->name,
            pArea->filename); send_to_char (buf, ch);
            REMOVE_BIT (pArea->area_flags, AREA_CHANGED);
      }
    }
    if (!str_cmp (buf, "\x1B[1;32mNone\x1B[37;0m.\n\r"))
      send_to_char (buf, ch); return;
  }
  if (!str_cmp (arg1, "list"))
  {
    save_area_list (); return;
  }
  if (!str_cmp (arg1, "zone"))
  {
    if (LEVEL (ch) < 110)
    {
      send_to_char ("Please use ASAVE CHANGED!\n\r", ch); return;
    }
    save_area_list ();
    save_area (ch->in_room->area);
    REMOVE_BIT (pArea->area_flags, AREA_CHANGED);
    send_to_char ("Zone saved.\n\r", ch); return;
  }
  do_asave (ch, ""); return;
}
