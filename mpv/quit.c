#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"

bool quit_flag = FALSE;

void
check_limited_stuff (CHAR_DATA * ch)
{
  SINGLE_OBJECT *obj;
  int i;
  // equipment and inventory
  for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
  {
    if (obj->pIndexData->how_many > 0 &&
        limited.limited_array[obj->pIndexData->vnum] > 0)
      limited.limited_array[obj->pIndexData->vnum]--;
  }
  // store
  for (i = 0; i < MAXST; i++)
  {
    if ((obj = ch->pcdata->storage[i]) && obj->pIndexData->how_many > 0 &&
        limited.limited_array[obj->pIndexData->vnum] > 0)
      limited.limited_array[obj->pIndexData->vnum]--;
  }
  return;
}

void
clear_tracks (CHAR_DATA * ch)
{
  ROOM_DATA *rid;
  int hash;
  TRACK_DATA *tr;
  TRACK_DATA *tr_n;
  TRACK_DATA *tt;
  if (!ch)
    return;
  for (hash = 0; hash < HASH_MAX; hash++)
  {
    for (rid = room_hash[hash]; rid != NULL; rid = rid->next)
    {
      if (rid->tracks && !str_cmp(rid->tracks->victname, NAME(ch)))
      {
        tt = rid->tracks;
        rid->tracks = rid->tracks->next_track_in_room;
        free_m (tt);
        continue;
      }
      for (tr = rid->tracks; tr != NULL; tr = tr_n)
      {
        tr_n = tr->next_track_in_room;
        if (tr_n && !str_cmp(tr_n->victname, NAME(ch)))
        {
          tt = tr->next_track_in_room;
          tr->next_track_in_room = tr_n->next_track_in_room;
          free_m (tt);
          goto tt;
        }
      }
tt:;
    }
  }

  return;
}

void
do_busy (CHAR_DATA * ch, char *argy)
{
  DEFINE_COMMAND ("busy", do_busy, POSITION_DEAD, 110, LOG_NORMAL,
      "Allows you to enter busy mode.") if (IS_MOB (ch))
        return;
      if (LEVEL (ch) < 109)
      {
        send_to_char ("You may not go into busy mode unless you are a god.\n\r",
            ch);
        return;
      }
      if (ch->pcdata->quiet != 2)
        ch->pcdata->quiet = 2;
      else
        ch->pcdata->quiet = 0;
      if (ch->pcdata->quiet == 0)
        send_to_char ("You are no longer in busy mode! Prepare yourself!\n\r",
            ch);
      if (ch->pcdata->quiet == 2)
        send_to_char ("You have entered busy mode.\n\r", ch);
      return;
}

void
do_action (CHAR_DATA * ch, char *argy)
{
  int k;
  char *t;
  int empty = 0;
  int n1 = 0;
  int n2 = 0;
  char arg1[SML_LENGTH];
  char arg2[SML_LENGTH];
  char general_use[STD_LENGTH];
  int an;
  DEFINE_COMMAND ("action", do_action, POSITION_DEAD, 0, LOG_NORMAL, "Allows you to set up actions, or delete/modify existing actions.")
    send_to_char ("Actions are disabled right now.\n\r", ch);
  return;
  SET_BIT (ch->special, ACTION_OFF);
  if (!ch || IS_MOB (ch))
  {
    return;
  }
  for (k = 0; k < MAXALIAS; k++)
    if (!ch->pcdata->action[k] || ch->pcdata->action[k][0] == '\0'
        || !ch->pcdata->actionname[k] || ch->pcdata->actionname[k][0] == '\0')
      empty++;
  if (!argy || argy[0] == '\0')
  {
    if (empty == MAXALIAS)
    {
      send_to_char
        ("You have no actions. To set one up, use: Action \"<trigger>\" <action to perform>.\n\r",
         ch);
      return;
    }
    send_to_char
      ("\x1B[34;1m---------\x1B[35m[ Actions ]\x1B[34m---------\x1B[37;0m\n\r\n\r",
       ch);
    for (k = 0; k < MAXALIAS; k++)
    {
      if (ch->pcdata->action[k] && ch->pcdata->action[k][0] != '\0'
          && ch->pcdata->actionname[k]
          && ch->pcdata->actionname[k][0] != '\0')
      {
        char *iio;
        for (iio = ch->pcdata->action[k]; *iio != '\0'; iio++)
        {
          if (*iio == '~')
            *iio = '*';
        }
        sprintf (general_use,
            "\x1B[1;30m[\x1B[34m%2d\x1B[30m]\x1B[34m --> \x1B[32m%-22s\x1B[34m Then Do: \x1B[37m\"%s\"\x1B[37;0m\n\r",
            k + 1, ch->pcdata->actionname[k],
            ch->pcdata->action[k]);
        send_to_char (general_use, ch);
        for (iio = ch->pcdata->action[k]; *iio != '\0'; iio++)
        {
          if (*iio == '*')
            *iio = '~';
        }
      }
    }
    return;
  }
  argy = one_argy (argy, arg1);
  one_argy (argy, arg2);
  if (argy[0] == '{' && argy[strlen (argy) - 1] == '}')
  {
    argy[strlen (argy) - 1] = '\0';
    argy++;
  }
  for (t = arg1; *t != '\0'; t++)
  {
    if (*t == '^')
      n1++;
    if (*t == '&')
      n2++;
    if (n1 > 1 || n2 > 1)
    {
      send_to_char
        ("You may not have more than ONE ^ or & in your action TRIGGER.\n\r",
         ch);
      return;
    }
  }
  if (!arg2 || arg2[0] == '\0')
  {
    for (k = 0; k < MAXALIAS; k++)
    {
      if (ch->pcdata->actionname[k]
          && !str_cmp (arg1, ch->pcdata->actionname[k]))
      {
        sprintf (general_use, "Deleted action #%d.\n\r", k + 1);
        send_to_char (general_use, ch);
        free_string (ch->pcdata->actionname[k]);
        free_string (ch->pcdata->action[k]);
        ch->pcdata->actionname[k] = NULL;
        ch->pcdata->action[k] = NULL;
        return;
      }
    }
    an = atoi (arg1);
    if (an > 0)
    {
      for (k = 0; k < MAXALIAS; k++)
      {
        if (an == (k + 1))
        {
          if (!ch->pcdata->actionname[k])
          {
            send_to_char ("That action number does not exist!\n\r",
                ch);
            return;
          }
          sprintf (general_use, "Deleted action #%d.\n\r", k + 1);
          send_to_char (general_use, ch);
          free_string (ch->pcdata->actionname[k]);
          free_string (ch->pcdata->action[k]);
          ch->pcdata->actionname[k] = NULL;
          ch->pcdata->action[k] = NULL;
          return;
        }
      }
    }
    sprintf (general_use, "Action \"%s\" not found.\n\r", arg1);
    send_to_char (general_use, ch);
    return;
  }
  if (empty == 0)
  {
    send_to_char
      ("You have no free action entries. You must delete one before adding\n\rany more.\n\r",
       ch);
    return;
  }
  if (strlen (arg1) < 5)
  {
    send_to_char ("Action triggers must be at least 5 letters long.\n\r",
        ch);
    return;
  }

  for (k = 0; k < MAXALIAS; k++)
  {
    if (ch->pcdata->actionname[k]
        && !str_cmp (arg1, ch->pcdata->actionname[k]))
    {
      char *msk;
      free_string (ch->pcdata->actionname[k]);
      free_string (ch->pcdata->actionname[k]);
      ch->pcdata->actionname[k] = str_dup (arg1);
      ch->pcdata->action[k] = str_dup (argy);
      for (msk = ch->pcdata->action[k]; *msk != '\0'; msk++)
        if (*msk == '*')
          *msk = '~';
      break;
    }
  }
  if (k == MAXALIAS)
    for (k = 0; k < MAXALIAS; k++)
    {
      if (ch->pcdata->actionname[k] == '\0' || !ch->pcdata->actionname[k]
          || !ch->pcdata->action[k] || ch->pcdata->action[k] == '\0')
      {
        char *msk;
        ch->pcdata->actionname[k] = str_dup (arg1);
        ch->pcdata->action[k] = str_dup (argy);
        for (msk = ch->pcdata->action[k]; *msk != '\0'; msk++)
          if (*msk == '*')
            *msk = '~';
        break;
      }
    }
  sprintf (general_use,
      "\x1B[34;1m[\x1B[37mAdded action #%d\x1B[34m]\x1B[37;0m\n\r",
      k + 1);
  send_to_char (general_use, ch);
  return;
}

void
do_alias (CHAR_DATA * ch, char *argy)
{
  int k;
  int empty = 0;
  char *t;
  char arg1[SML_LENGTH];
  char arg2[SML_LENGTH];
  char general_use[STD_LENGTH];
  int an;
  DEFINE_COMMAND ("alias", do_alias, POSITION_DEAD, 0, LOG_NORMAL, "Allows you to set up an alias expansion, or delete/modify existing aliases.")
    if (!ch || IS_MOB (ch))
      return;
  for (k = 0; k < MAXALIAS; k++)
    if (!ch->pcdata->alias[k] || ch->pcdata->alias[k][0] == '\0' || !ch->pcdata->aliasname[k] || ch->pcdata->aliasname[k][0] == '\0')
      empty++;
  if (!argy || argy[0] == '\0')
  {
    if (empty == MAXALIAS)
    {
      send_to_char ("You have no aliases. To set one up, use: Alias <name> <expansion>.\n\r", ch);
      return;
    }
    send_to_char ("\x1B[34;1m---------\x1B[35m[ Aliases ]\x1B[34m---------\x1B[37;0m\n\r\n\r", ch);
    for (k = 0; k < MAXALIAS; k++)
    {
      if (ch->pcdata->alias[k] && ch->pcdata->alias[k][0] != '\0' && ch->pcdata->aliasname[k] && ch->pcdata->aliasname[k][0] != '\0')
      {
        char *iio;
        for (t = ch->pcdata->alias[k]; *t != '\0'; t++)
        {
          if (*t == '%')
            *t = '@';
        }
        for (t = ch->pcdata->aliasname[k]; *t != '\0'; t++)
        {
          if (*t == '%')
            *t = '@';
        }
        for (iio = ch->pcdata->alias[k]; *iio != '\0'; iio++)
        {
          if (*iio == '~')
            *iio = '*';
        }
        sprintf (general_use, "\x1B[1;30m[\x1B[34m%2d\x1B[30m]\x1B[34m --> \x1B[32m%-10s\x1B[34m Expansion: \x1B[37m\"%s\"\x1B[37;0m\n\r",
            k + 1, ch->pcdata->aliasname[k], ch->pcdata->alias[k]);
        send_to_char (general_use, ch);
        for (iio = ch->pcdata->alias[k]; *iio != '\0'; iio++)
        {
          if (*iio == '*')
            *iio = '~';
        }
      }
    }
    return;
  }
  argy = one_argy (argy, arg1);
  one_argy (argy, arg2);
  if (argy[0] == '{' && argy[strlen (argy) - 1] == '}')
  {
    argy[strlen (argy) - 1] = '\0';
    argy++;
  }
  if (!arg2 || arg2[0] == '\0')
  {
    for (k = 0; k < MAXALIAS; k++)
    {
      if (ch->pcdata->aliasname[k]
          && !str_cmp (arg1, ch->pcdata->aliasname[k]))
      {
        sprintf (general_use, "Deleted alias #%d.\n\r", k + 1);
        send_to_char (general_use, ch);
        free_string (ch->pcdata->aliasname[k]);
        free_string (ch->pcdata->alias[k]);
        ch->pcdata->aliasname[k] = NULL;
        ch->pcdata->alias[k] = NULL;
        return;
      }
    }
    an = atoi (arg1);
    if (an > 0)
    {
      for (k = 0; k < MAXALIAS; k++)
      {
        if (an == (k + 1))
        {
          if (!ch->pcdata->aliasname[k])
          {
            send_to_char ("That alias number does not exist!\n\r",
                ch);
            return;
          }
          sprintf (general_use, "Deleted alias #%d.\n\r", k + 1);
          send_to_char (general_use, ch);
          free_string (ch->pcdata->aliasname[k]);
          free_string (ch->pcdata->alias[k]);
          ch->pcdata->aliasname[k] = NULL;
          ch->pcdata->alias[k] = NULL;
          return;
        }
      }
    }
    sprintf (general_use, "Alias \"%s\" not found.\n\r", arg1);
    send_to_char (general_use, ch);
    return;
  }
  if (empty == 0)
  {
    send_to_char
      ("You have no free alias entries. You must delete one before adding\n\rany more.\n\r",
       ch);
    return;
  }
  if (strlen (arg1) == 1
      && (arg1[0] == 'l' || arg1[0] == 'k' || arg1[0] == 'n' || arg1[0] == 's'
        || arg1[0] == 'e' || arg1[0] == 'w' || arg1[0] == 'i'
        || arg1[0] == 'u' || arg1[0] == 'd'))
  {
    send_to_char
      ("Aliases that begin with that letter must be more than 1 character long.\n\r",
       ch);
    return;
  }
  for (k = 0; k < MAXALIAS; k++)
  {
    if (ch->pcdata->aliasname[k] &&
        !aliasok (ch->pcdata->aliasname[k], argy))
    {
      send_to_char ("Recursive aliases are not acceptable!\n\r", ch);
      return;
    }
  }
  if (!aliasok (arg1, argy))
  {
    send_to_char ("Partially recursive aliases are not acceptable!\n\r",
        ch);
    return;
  }
  for (k = 0; k < MAXALIAS; k++)
  {
    if (ch->pcdata->aliasname[k]
        && !str_cmp (arg1, ch->pcdata->aliasname[k]))
    {
      char *msk;
      free_string (ch->pcdata->alias[k]);
      free_string (ch->pcdata->aliasname[k]);
      ch->pcdata->aliasname[k] = str_dup (arg1);
      ch->pcdata->alias[k] = str_dup (argy);
      for (t = ch->pcdata->alias[k]; *t != '\0'; t++)
      {
        if (*t == '%')
          *t = '@';
      }
      for (t = ch->pcdata->aliasname[k]; *t != '\0'; t++)
      {
        if (*t == '%')
          *t = '@';
      }
      for (msk = ch->pcdata->alias[k]; *msk != '\0'; msk++)
        if (*msk == '*')
          *msk = '~';
      break;
    }
  }
  if (k == MAXALIAS)
    for (k = 0; k < MAXALIAS; k++)
    {
      if (ch->pcdata->aliasname[k] == '\0' || !ch->pcdata->aliasname[k]
          || !ch->pcdata->alias[k] || ch->pcdata->alias[k] == '\0')
      {
        char *msk;
        ch->pcdata->aliasname[k] = str_dup (arg1);
        ch->pcdata->alias[k] = str_dup (argy);
        for (t = ch->pcdata->alias[k]; *t != '\0'; t++)
        {
          if (*t == '%')
            *t = '@';
        }
        for (t = ch->pcdata->aliasname[k]; *t != '\0'; t++)
        {
          if (*t == '%')
            *t = '@';
        }
        for (msk = ch->pcdata->alias[k]; *msk != '\0'; msk++)
          if (*msk == '*')
            *msk = '~';
        break;
      }
    }
  sprintf (general_use,
      "\x1B[34;1m[\x1B[37mAdded alias #%d\x1B[34m]\x1B[37;0m\n\r",
      k + 1);
  send_to_char (general_use, ch);
  return;
}

void
do_qui (CHAR_DATA * ch, char *argy)
{
  DEFINE_COMMAND ("qui", do_qui, POSITION_DEAD, 0, LOG_NORMAL, "This command is so you don't type q or qu and quit out by accident.")
    send_to_char ("Huh?\n\r", ch);
  return;
}

void
do_quit (CHAR_DATA * ch, char *argy)
{
  DESCRIPTOR_DATA *d;
  char buf[STD_LENGTH];
  CHAR_DATA *bah_nir;
  CHAR_DATA *bah;
  CHAR_DATA *wch;
  char general_use[STD_LENGTH];
  DEFINE_COMMAND ("quit", do_quit, POSITION_DEAD, 0, LOG_NORMAL, "This command saves your character and terminates the session.")
    if (IS_MOB (ch) && ch->desc != NULL)
    {
      do_return (ch, "");
      return;
    }
  if (ch->in_room && ch->in_room->sector_type == SECT_WATER_NOSWIM)
  {
    send_to_char ("You may not quit in an unswimmable water sector.\n\r", ch);
    return;
  }
  if (ch->in_room && ch->in_room->more && ch->in_room->more->linked_to)
  {
    send_to_char ("You cannot quit while in a vehicle.\n\r", ch);
    return;
  }

  if (ch->desc && !ch->desc->character)
  {
    ch->desc->character = ch;
  }
  if (IS_AFFECTED (ch, AFF_CHARM))
    return;
  check_ced (ch);
#ifndef NEW_WORLD
  if (ch->ced->in_boat != NULL)
  {
    send_to_char ("You cannot quit inside of a boat!!\n\r", ch);
    return;
  }
#endif
  if (IN_BATTLE (ch))
  {
    send_to_char ("You cannot quit in the battleground!\n\r", ch);
    return;
  }

  if (LEVEL (ch) < 100)
  {
    ROOM_DATA *rrd;
    if (ch->in_room && ch->in_room->area && ch->in_room->area->can_quit == 2)
    {
      send_to_char ("You may not quit while in this zone.\n\r", ch);
      return;
    }
}
#ifdef NEW_WORLD
    if (!IS_EVIL (ch) && (rrd = get_room_index (1501)) != NULL &&
        ch->in_room && ch->in_room->area != rrd->area &&
        rrd->vnum > 1200 && ch->in_room->area->can_quit == 0 &&
        IS_SET (ch->in_room->room_flags, ROOM_INDOORS))
    {
      send_to_char
        ("You cannot quit while indoors.  You may only quit in town or outside.\n\r",
         ch);
      return;
    }
    if (IS_EVIL (ch) && ch->in_room
        && IS_SET (ch->in_room->room_flags, ROOM_INDOORS)
        && ch->in_room->area->can_quit == 0)
    {
      send_to_char ("You cannot quit while indoors.\n\r", ch);
      return;
    }
  }
#endif
  if (ch->in_room && ch->in_room->vnum > 440 && ch->in_room->vnum < 700 &&
      !ch->in_room->area->can_quit)
  {
    send_to_char ("You are not in a valid place in the world to quit.\n\r", ch);
    return;
  }
  if (CHALLENGE (ch) == 10)
  {
    send_to_char ("You must wait until after the arena battle!\n\r", ch);
    return;
  }
  if (auction_char == ch || e_auction_char == ch)
  {
    send_to_char ("Not while you are auctioning an item!\n\r", ch);
    return;
  }
  if (auction_tochar == ch || e_auction_tochar == ch)
  {
    send_to_char ("Not while you have the current bid on the auction!\n\r", ch);
    return;
  }
  if (ch->pcdata->no_quit_pk != 0)
  {
    sprintf (general_use, "Our records show you have recently been near or in combat with a player.\n\r");
    send_to_char (general_use, ch);
    sprintf (general_use, "You must wait %d more game hours to quit.\n\r", ch->pcdata->no_quit_pk);
    send_to_char (general_use, ch);
    return;
  }

  if (ch->pcdata->no_quit != 0)
  {
    send_to_char ("Due to possibilities of avoiding hunting mobs and/or players, killing,\n\r", ch);
    send_to_char ("looting, then quitting out right away, you cannot quit from the game\n\r", ch);
    send_to_char ("this soon after combat.  Combat with mobs sets the counter to 3 ticks,\n\r", ch);
    send_to_char ("combat with players sets the counter to 6 ticks, and encountering an\n\r", ch);
    send_to_char ("player opponent, sets the counter to 2 ticks.\n\r", ch);
    sprintf (general_use, "You must wait %d more game hours to quit.\n\r", ch->pcdata->no_quit);
    send_to_char (general_use, ch);
    return;
  }
  if (ch->position == POSITION_FIGHTING && ch->position == POSITION_STOPCAST && ch->position == POSITION_BASHED)
  {
    send_to_char ("You may not quit during combat.\n\r", ch);
    return;
  }
  if (ch->position < POSITION_STUNNED)
  {
    send_to_char ("You must wait until you die or recover to quit.\n\r", ch);
    return;
  }
  /* Pray that they are not auctioning stuff.. or it might crashie! */

  check_ced (ch);
  if (ch->ced->wasroom != 0)
  {
    char_from_room (ch);
    char_to_room (ch, get_room_index (ch->ced->wasroom));
  }
  if (ch->in_room && ch->in_room->vnum > 500 && ch->in_room->vnum < 550)
  {
    fprintf (stderr, "Moved %s back into portal room from arena/waiting rooms.\n", NAME (ch));
    char_from_room (ch);
    if (IS_EVIL (ch))
      char_to_room (ch, get_room_index (99));
    else
      char_to_room (ch, get_room_index (100));
  }
  do_help (ch, "FAREWELL");

  //clear_tracks (ch);
  if (!d)
  {
    write_to_descriptor2 (d, "\x1B[1;34mThanks for playing \x1B[1;31mMidpoint Void\x1B[0;33! \x1B[1;34mCome back soon ! ", 0);
  }
  update_pbase (ch);
  if (IS_PLAYER (ch) && ch->pcdata->prompt != 0)
  {
    char tmps[500];
    sprintf (tmps, "\x1B[1;%dr\x1B[2J",
        ch->pcdata->pagelen);
    write_to_descriptor2 (ch->desc, tmps, 0);
  }
  if (IS_PLAYER (ch) && ch->desc != NULL && IS_SET (ch->pcdata->act2, PLR_BLANKPROMPT))
  {
    char tmps[500];
    sprintf (tmps, "\x1B[1;%dr\x1B[2J",
        ch->pcdata->pagelen);
    write_to_descriptor2 (ch->desc, tmps, 0);
  }
  if (strcasecmp (RNAME (ch), "eraser"))
  {
    sprintf (log_buf, "%s (%s@%s) has left the world.",
        NAME (ch),
        (ch->desc ? ch->desc-> username : "nodesc"),
        (ch->desc ? ch->desc->host : "nodesc"));
    log_string (log_buf);
    sprintf (general_use, "Notify> %s has left the world.", NAME (ch));
    do_global (general_use, LEVEL_IMMORTAL, WIZ_NOTIFY_LOGIN);
    if (LEVEL (ch) < LEVEL_IMMORTAL)
    act ("$n logged off.", ch, NULL, NULL, TO_NOTVICT);
  }
  NEW_POSITION (ch, POSITION_STANDING);
  ch->pcdata->number_pets = 0;
  sprintf (general_use, "%s logged off.\n\r", NAME (ch));
  do_chan_notify (ch, general_use);

/*---------------------------------*/
/*         LOG_OUT MESSEGE         */
/*---------------------------------*/
  for (wch = char_list; wch != NULL; wch = wch->next)
      {
      if (!IS_IMMORTAL(wch))
         {
         if ((IS_EVIL (ch)) && (IS_EVIL (wch)) && !IS_IMMORTAL(ch))
            {
sprintf (buf,"\n\r\x1B[0;36mThe forces of \x1B[1;31mevil \x1B[0;36mweaken as \x1B[1;31m%s\x1B[0;36m steps into the \x1B[1;30mdark.\x1b[0;37m\n\r",ch->pcdata->name);
            send_to_char(buf, wch);
            }
       else if ((!IS_EVIL (ch)) && (!IS_EVIL (wch)) && !IS_IMMORTAL(ch))
            {
sprintf(buf,"\n\r\x1B[0;36mThe forces of \x1B[1;34mgood \x1B[0;36mweaken as \x1B[1;34m%s\x1B[0;36m steps into the \x1B[1;30mdark.\x1B[0;37m\n\r",ch->pcdata->name);
            send_to_char(buf, wch);
            }
          }
      if (IS_IMMORTAL(wch))
         {
         if (!IS_IMMORTAL(ch) && !IS_EVIL (ch))
            {
sprintf(buf,"\n\r\x1B[0;36mThe forces of \x1B[1;34mgood \x1B[0;36mweaken as \x1B[1;34m%s\x1B[0;36m steps into the \x1B[1;30mdark.\x1B[0;37m\n\r",ch->pcdata->name);
            send_to_char(buf, wch);
                }
         if (!IS_IMMORTAL(ch) && IS_EVIL (ch))
           {
sprintf(buf,"\n\r\x1B[0;36mThe forces of \x1B[1;31mevil \x1B[0;36mweaken as \x1B[1;31m%s\x1B[0;36m steps into the \x1B[1;30mdark.\x1B[0;37m\n\r",ch->pcdata->name);
            send_to_char(buf, wch);
              }
            } 
          }
  if (LEVEL (ch) > 112) ch->pcdata->level = 1;
  save_char_obj (ch);
  //save_char_tro (ch);
  if (ch->in_room)
  {
    for (bah = ch->in_room->more->people; bah != NULL;
        bah = bah_nir)
    {
      bah_nir = bah->next_in_room;
      if (IS_MOB (bah) && MASTER (bah) == ch
          && IS_AFFECTED (bah, AFF_CHARM))
      {
        char_from_room (bah);
        check_fgt (bah);
        bah->fgt->master = NULL;
        extract_char (bah, TRUE);
      }
    }
  }
  d = ch->desc; quit_flag = TRUE;
  // do_rawclear (ch);
  // Keep the farewell up there.
  extract_char (ch, TRUE);
  quit_flag = FALSE; if (d != NULL)
  {
    close_socket (d);
  }
  return;
}

void 
do_delete (CHAR_DATA * ch, char *argy)
{
  DESCRIPTOR_DATA * d;
  PLAYERBASE_DATA * playerbase;
  char general_use[STD_LENGTH];
  DEFINE_COMMAND ("delete", do_delete, POSITION_DEAD, 0, LOG_ALWAYS, "This command is used to delete your character permanently and irreversably.")
    if (FIGHTING (ch) != NULL || ch->position == POSITION_FIGHTING) return;
  check_ced (ch);
#ifndef NEW_WORLD
  if (ch->ced->in_boat != NULL)
  {
    send_to_char ("You cannot delete inside of a boat!!\n\r", ch);
    return;
  }
#endif
  if (ch->in_room && ch->in_room->vnum > 75 && ch->in_room->vnum < 999)
  {
    send_to_char ("You are not in a valid place in the world to delete.\n\r", ch); 
    return;
  }
  if (IS_AFFECTED (ch, AFF_CHARM)) return;
  if (IN_BATTLE (ch))
  {
    send_to_char ("You cannot delete in the battleground!\n\r", ch); 
    return;
  }
  if (!str_cmp (argy, "character forever"))
  {
    if (CHALLENGE (ch) == 10)
    {
      send_to_char ("You must wait until after the arena battle!\n\r", ch); 
	return;
    }
    if (auction_char == ch || e_auction_char == ch)
    {
      send_to_char ("Not while you are auctioning an item!\n\r", ch);
      return;
    }
    if (ch->pcdata->remort_times >= 2)
    {
     send_to_char ("Since you have 2 or more remorts.. You can not delete with out the help of an admin!\n\r", ch);
     return;
    }
    if (auction_tochar == ch || e_auction_tochar == ch)
    {
      send_to_char ("Not while you have the current bid on the auction!\n\r", ch);
      return;
    }

    //clear_tracks (ch);
    ch->pcdata->warpoints = 0;
    check_topten (ch);
    save_topten ();
    ch->pcdata->deaths += rating (ch);
    check_rating (ch);
    save_rating ();
    if (clan_number (ch) >= 1)
    {
      int i;
      CLAN_DATA * cd;
      cd = get_clan_index (clan_number (ch));
      for (i = 0; i < 100; i++)
      {
        if (!str_cmp (cd->members[i], RNAME (ch)))
        {
          free_string (cd->members[i]);
          cd->members[i] = NULL; break;
        }
      }
      if (!str_cmp (cd->leader, RNAME (ch)))
        strcpy (cd->leader, "Leaderless!");
    }

    save_char_obj (ch);
    sprintf (general_use, "%s%s.cor", PLAYER_DIR_2, NAME (ch));
    unlink (general_use);
    //sprintf (general_use, "%s%s.bak", PLAYER_DIR_2, NAME (ch)); 
    //unlink (general_use);
    sprintf (general_use, "%s%s.note", PLAYER_DIR_2, NAME (ch)); 
    unlink (general_use);
    sprintf (general_use, "%s%s.poll", PLAYER_DIR_2, NAME (ch)); 
    unlink (general_use);
    sprintf (general_use, "%s%s", PLAYER_DIR_2, NAME (ch)); 
    unlink (general_use);
    for (playerbase = playerbase_zero;
        playerbase != NULL;
        playerbase = playerbase->next)
    {
      if (!str_cmp (playerbase->player_name, NAME (ch)))
      {
        playerbase->player_level = -1;
        save_playerbase (); break;
      }
    }
    sprintf (log_buf, "\x1B[1;31m--> %s deleted their character.\x1B[0;37m", NAME (ch));
    log_string (log_buf);
    sprintf (general_use, "\x1B[1;31mNotify> %s has just typed delete character forever and has been deleted.\x1B[0;37m", NAME (ch));
    do_global (general_use, LEVEL_IMMORTAL, WIZ_NOTIFY_LOGIN);
    d = ch->desc;
    check_limited_stuff (ch); extract_char (ch, TRUE);
    if (d != NULL)
    {
      write_to_descriptor2 (d, "\n\rThanks for playing, hope you had a good time.\n\rYour character has been succesfully deleted.\n\r\n\r", 0);
       close_socket (d);
          return;
    }
  }
  if (argy[0] == '\0')
  {
    send_to_char
      ("To delete this character permanently, type delete character forever\n\r",
       ch); return;
  }
  return;
}

void do_allsave (CHAR_DATA * ch, char *argy)
{
  DESCRIPTOR_DATA * dd;
  DEFINE_COMMAND ("allsave", do_allsave, POSITION_DEAD, 110, LOG_ALWAYS, "This command forces a complete save of all characters.")
    for (dd = descriptor_list; dd != NULL;
        dd = dd->next)
    {
      if (!dd->character || dd->connected != CON_PLAYING) continue;
      write_to_descriptor2 (dd, "Forced save... saving your character!!\n\r", 0);
      save_char_obj (dd->character);
    }
  return;
}

void do_save (CHAR_DATA * ch, char *argy)
{
  DEFINE_COMMAND ("save", do_save, POSITION_DEAD, 0, LOG_NORMAL, "This command is used to save your character.")
  if (IS_MOB (ch)) return;
  save_char_obj (ch);
  if (argy && !str_cmp (argy, "yy2zz11")) return;
  send_to_char ("Okies Dokie :p Saving... Successful.\n\r", ch);
  return;
}

void do_follow (CHAR_DATA * ch, char *argy)
{
  char arg[SML_LENGTH];
  CHAR_DATA * victim;
  DEFINE_COMMAND ("follow", do_follow, POSITION_STANDING, 0, LOG_NORMAL, "This command allows you to follow a player or mob.  See also Ditch and Group.")
    one_argy (argy, arg); if (arg[0] == '\0')
    {
      send_to_char ("Syntax: Follow <person>\n\r", ch);
      return;
    }
  if ((victim = get_char_room (ch, arg)) == NULL)
  {
    send_to_char ("That person is either not here, or not visible to you.\n\r", ch); 
    return;
  }
  if (LEVEL (victim) > 100 && LEVEL (ch) < 100)
  {
    send_to_char ("Mortals may not follow immortals!\n\r", ch);
    return;
  }
  if (IS_AFFECTED (ch, AFF_CHARM) && MASTER (ch) != NULL)
  {
    act ("You'd rather follow your master... $N!", ch, NULL, MASTER (ch), TO_CHAR); 
    return;
  }
/*if (ch->number_of_mob_followers != 0)
  {
          send_to_char ("You are too busy leading your own troops!\n\r", ch);
          return;
  }
   if (victim->number_of_mob_followers != 0)
  {
          send_to_char ("That person is too busy ordering slaves around!\n\r", ch);
          return;
  }*/
  if (victim == ch)
  {
    if (MASTER (ch) == NULL)
    {
      send_to_char ("Okay.  You stop following anyone you were following.\n\r", ch); 
      return;
    }
    stop_follower (ch); 
    return;
  }
  conv_race (victim);
  conv_race (ch);
  if (not_is_same_align (ch, victim))
  {
    if (IS_EVIL (victim))
    {
      send_to_char ("FOLLOW A DARKIE!? Are you kidding me!?!\n\r", ch);
    }
    if (!IS_EVIL (victim))
    {
      send_to_char ("FOLLOW A WHITIE!? You have got to be kidding me!!!\n\r", ch);
    }
    return;
  }
  if (MASTER (ch) != NULL)
    stop_follower (ch);
  add_follower (ch, victim); return;
}

void
add_follower (CHAR_DATA * ch, CHAR_DATA * master)
{
  if (MASTER (ch) != NULL)
  {
    bug ("Add_follower: non-null master.", 0); return;
  }
  check_fgt (ch);
  ch->fgt->master = master;
  ch->fgt->leader = NULL;
  if (can_see (master, ch))
    act ("$n now follows you.", ch, NULL, master, TO_VICT);
    act ("You now follow $N.", ch, NULL, master, TO_CHAR); 
    return;
}

void stop_follower (CHAR_DATA * ch)
{
  char general_use[STD_LENGTH];
  if (MASTER (ch) == NULL)
  {
    bug ("Stop_follower: null master.", 0); 
    return;
  }
  if (IS_AFFECTED (ch, AFF_CHARM))
  {
    (MASTER(ch)->number_of_mob_followers)--;
    REMOVE_BIT (ch->affected_by, AFF_CHARM);
  //raise_undead spell
    affect_strip (ch, gsn_raise_undead);
  }
  if (can_see (MASTER (ch), ch))
    act ("$n stops following you around.", ch, NULL, MASTER (ch), TO_VICT);
  act ("You stop following $N.", ch, NULL, MASTER (ch), TO_CHAR);
  if (LEADER (ch) != NULL)
  {
    sprintf (general_use, "%s has left the group.", NAME (ch));
    group_notify (general_use, ch);
  }
  check_fgt (ch); ch->fgt->master = NULL;
  ch->fgt->leader = NULL; return;
}

void die_follower (CHAR_DATA * ch)
{
  CHAR_DATA * fch;
  if (MASTER (ch) != NULL)
    stop_follower (ch);
  if (!ch->fgt)
    return;
  ch->fgt->leader = NULL;
  for (fch = char_list; fch != NULL;
      fch = fch->next)
  {
    if (MASTER (fch) == ch)
      stop_follower (fch);
    if (LEADER (fch) == ch) fch->fgt->leader = fch;
  }
  return;
}

int chars_in_group (CHAR_DATA * ch)
{
  DESCRIPTOR_DATA * tch; int tally; tally = 0; if (IS_MOB (ch)) return 1;	/*Just dummy value */
  for (tch = descriptor_list; tch != NULL;
      tch = tch->next)
  {
    if (tch->character == NULL
        || tch->connected != CON_PLAYING) continue;
    if (is_same_group (ch, tch->character)) tally++;
  }
  if (tally == 0) tally = 1; return tally;
}

int rchars_in_group (CHAR_DATA * ch)
{
  CHAR_DATA * tch; int tally; tally = 0; if (IS_MOB (ch)) return 1;	/*Just dummy value */
  for (tch = ch->in_room->more->people; tch != NULL;
      tch = tch->next_in_room)
  {
    if (IS_MOB (tch))
      continue; if (is_same_group (ch, tch)) tally++;
  }
  if (tally == 0) tally = 1; return tally;
}

int tchars_in_group (CHAR_DATA * ch)
{
  CHAR_DATA * tch; int tally; tally = 0; if (IS_MOB (ch)) return 1;	/*Just dummy value */
  for (tch = char_list; tch != NULL;
      tch = tch->next)
  {
    if (IS_MOB (tch))
      continue; if (is_same_group (ch, tch)) tally++;
  }
  if (tally == 0) tally = 1; return tally;
}

void do_ditch (CHAR_DATA * ch, char *argy)
{
  CHAR_DATA * ditchee;
  DEFINE_COMMAND ("ditch", do_ditch, POSITION_DEAD, 0, LOG_NORMAL, "This command allows you to ditch a following character.")
    if (argy == "" || argy[0] == '\0')
    {
      send_to_char ("Ditch whom?\n\r", ch); return;
    }
  if ((ditchee = get_char_world (ch, argy)) == NULL)
  {
    send_to_char ("Ditch Whom?\n\r", ch); return;
  }
  if (ditchee == ch)
  {
    send_to_char ("Ditch yourself? Get a clue...\n\r", ch); 
    return;
  }
  if ((!MASTER (ditchee) || MASTER (ditchee) != ch)
      && (!LEADER (ditchee) || LEADER (ditchee) != ch))
  {
    send_to_char ("Ditch Whom?\n\r", ch); 
    return;
  }
  send_to_char ("Player successfully ditched.\n\r", ch); 
      check_fgt (ditchee);
      ditchee->fgt->master = NULL;
      ditchee->fgt->leader = NULL;
      if (IS_AFFECTED (ditchee, AFF_CHARM))
        REMOVE_BIT (ditchee->affected_by, AFF_CHARM);
      send_to_char ("The group leader has ditched you!\n\r", ditchee); 
      return;
}


void do_grwhere (CHAR_DATA * ch, char *argy)
{
  char buf[STD_LENGTH];
  CHAR_DATA * wch;
  CHAR_DATA * leader;
  DEFINE_COMMAND ("grwhere", do_grwhere, POSITION_DEAD, 0, LOG_NORMAL, "Shows character's locations when you are grouped with them.")
    leader = (LEADER (ch) != NULL) ? LEADER (ch) : ch;
  send_to_char ("Group member's locations:\n\r",
      ch); for (wch = char_list;
        wch != NULL;
        wch = wch->next)
      {
        if (is_same_group (wch, ch))
        {
          sprintf (buf, "    %-17s                  %s\n\r",
              NAME (wch),
              strip_ansi_codes (show_room_name
                (ch,
                 wch->in_room->name)));
          send_to_char (buf, ch);
        }

      }
      return;
}

void 
do_group (CHAR_DATA * ch, char *argy)
{
  char buf[STD_LENGTH];
  char arg[SML_LENGTH];
  int cig;
  CHAR_DATA * victim;
  int tcig;
  DEFINE_COMMAND ("group", do_group, POSITION_DEAD, 0, LOG_NORMAL, "This command allows you to group 'all' or playername.  With no arguments, it shows info on your current group.")
    one_argy (argy, arg); if (arg[0] == '\0')
    {
      CHAR_DATA * gch;
      CHAR_DATA * leader;
      leader = (LEADER (ch) != NULL) ? LEADER (ch) : ch;

      sprintf (buf, "Group Leader [\x1B[37;1m%s\x1B[0m]\n\r\n\r", NAME (leader));
      send_to_char (buf, ch);
      sprintf (buf, "%-21s %-14s %-18s %-14s  %s\n\r",
"\x1B[1;30m[\x1B[0;37m -Char Power -\x1B[1;30m]", 
"\x1B[0;32m--\x1B[1;32mName\x1B[0;32m--", 
"             \x1B[0;36m+-Health-+", 
"   \x1B[1;34m*-Stamina-*", 
"   \x1B[1;35m=-Exp2Lvl-=");
          send_to_char (buf, ch);
          for (gch = char_list; gch != NULL;
              gch = gch->next)
          {
            if (is_same_group (gch, ch))
            {
              char tmp[40];
#ifndef NEW_WORLD
              if (gch == ch) sprintf (tmp, "\x1B[1;37m%14d", LEVEL (ch));
              else
                if (LEVEL (gch) < 4) strcpy (tmp,  "\x1B[1;35m    N00bie    ");
              else
                if (LEVEL (gch) < 12) strcpy (tmp, "\x1B[1;37m Knowledgable ");
              else
                if (LEVEL (gch) < 27) strcpy (tmp, "\x1B[1;36m  Experienced ");
              else
                if (LEVEL (gch) < 45) strcpy (tmp, "\x1B[1;33m  Well-Known  ");
              else
                if (LEVEL (gch) < 56) strcpy (tmp, "\x1B[1;32m --<Famous>-- ");
              else
                if (LEVEL (gch) < 80) strcpy (tmp, "\x1B[1;34m- + LEGEND + -");
              else
                if (LEVEL (gch) < 90) strcpy (tmp, "\x1B[1;31m >>> HERO <<< ");
              else
                                      strcpy (tmp, "\x1B[1;30m-=>IMMORTAL<=-");
#endif
#ifdef NEW_WORLD
              sprintf (buf,
                  "[%s\x1B[30;1m%-6d\x1B[37;0m] \x1B[37;1m%-14s\x1B[0m \x1B[31;1m%-18s\x1B[34m %-14s\x1B[37;0m  (%ld)\n\r",
                  (IS_PLAYER (gch) ? (NO_PKILL (gch) ? "\x1B[33;1mNo-PK " : "\x1B[34;1mPKill ") : ""),
                  LEVEL (gch), (IS_MOB (gch) ? gch->pIndexData->short_descr : capitalize (PERS (gch, ch))),
                  STRING_HITS (gch), STRING_MOVES (gch), ((FIND_EXP (LEVEL (gch), gch->race)) - gch->exp)); send_to_char (buf, ch);
#else
                  sprintf (buf,
"\x1B[0;36m[%12s\x1B[0;36m] \x1B[1;37m%-21s  \x1B[1;37m%-15s           \x1B[1;37m%-9s       \x1B[0;35m(\x1B[1;35m%ld\x1B[0;35m)\x1B[37;0m\n\r",
                      tmp, (IS_MOB (gch) ? gch->pIndexData-> short_descr : capitalize (PERS (gch, ch))),
                      STRING_HITS (gch), STRING_MOVES (gch), ((FIND_EXP (LEVEL (gch), gch->race)) - gch->exp)); send_to_char (buf, ch);
#endif
            }
          }
          if (LEADER (ch) == NULL) cig =
            tchars_in_group (ch);
          else
            cig = tchars_in_group (LEADER (ch)); if (cig > 1)
            {
              sprintf (buf,
                  "--Members in group: [\x1B[34;1m%d\x1B[37;0m]\n\r",
                  cig); send_to_char (buf, ch);
            }
          return;
    }
  tcig = tchars_in_group (ch);	/*added substitution, saves TONS of cpu */
  if (!str_cmp (arg, "all"))
  {
    CHAR_DATA * gch; if (LEADER (ch) != NULL)
    {
      send_to_char
        ("You must be the leader of the group someone or ALL!\n\r",
         ch); return;
    }
    for (gch = char_list; gch != NULL;
        gch = gch->next)
    {
      if (ch->in_room != NULL
          && !is_same_group (gch, ch)
          && ch->in_room == gch->in_room
          && MASTER (gch) != NULL && MASTER (gch) == ch
          && LEADER (gch) == NULL && tcig < 16)
      {
        check_fgt (gch);
        gch->fgt->leader = ch;
        act ("\x1B[1;32m$N joins $n's group.\x1B[37;0m", ch, NULL, gch, TO_NOTVICT);
        act ("\x1B[1;32mYou join $n's group.\x1B[37;0m", ch, NULL, gch, TO_VICT);
        act ("\x1B[1;32m$N joins your group.\x1B[37;0m", ch, NULL, gch, TO_CHAR);
        sprintf (buf, "%s has joined the group!", NAME (gch)); group_notify (buf, ch);
      }
    }
    return;
  }
  /*end all */
  if ((victim = get_player_world (ch, arg, FALSE)) == NULL || !can_see (ch, victim))
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  if ((IS_EVIL (ch) && !IS_EVIL (victim)) || (!IS_EVIL (ch) && IS_EVIL (victim)))
  {
    send_to_char ("They aren't here.\n\r", ch);
    return;
  }
  if (MASTER (ch) != NULL || (LEADER (ch) != NULL && LEADER (ch) != ch))
  {
    send_to_char ("You can follow two people at once!\n\r", ch);
    return;
  }
  if (MASTER (victim) != ch && ch != victim)
  {
    act
      ("$N must be following you in order to group up.", ch, NULL, victim, TO_CHAR); 
	return;
  }
  if (ch == victim)
  {
    CHAR_DATA * tch;
    send_to_char ("You disband your group.  All players stop following you.\n\r", ch); 
       for (tch = char_list; tch != NULL;
         tch = tch->next)
       {
         if (is_same_group (tch, ch))
         {
           check_fgt (tch);
           tch->fgt->leader = NULL;
           if (IS_PLAYER (tch))
             tch->fgt->master = NULL;
           send_to_char
             ("The leader has disbanded the group, and you now follow no-one.\n\r",
              tch);
         }
       }
       return;
  }
  if (is_same_group (victim, ch))
  {
    char buffy[200];
    check_fgt (victim);
    victim->fgt->leader = NULL;
    sprintf (buffy, "%s removed %s from the group.", NAME (ch), NAME (victim));
    group_notify (buffy, ch); 
    return;
  }
  if (tcig >= 6)
  {
    send_to_char ("Max limit of 6 players per group.\n\r", ch);
    return;
  }
  check_fgt (victim); victim->fgt->leader = ch;
  sprintf (buf, "%s has joined the group!", NAME (victim)); 
   group_notify (buf, ch);
      return;
}

void do_split (CHAR_DATA * ch, char *argy)
{
  char buf[STD_LENGTH];
  char arg[STD_LENGTH];
  CHAR_DATA * gch;
  char str1[1000];
  char str2[1000];
  int copper;
  int gold;
  int total_coinage;
  int members;
  int amount;
  int share;
  int counter;
  DEFINE_COMMAND ("split", do_split, POSITION_RESTING, 0, LOG_NORMAL, "This command can be used to split up coins.  It will not make change.")
    if (!ch->in_room) return;
  argy = one_argy (argy, arg); if (arg[0] == '\0')
  {
    send_to_char ("Split how much?\n\r", ch); return;
  }
  amount = atoi (arg);
  if (!str_cmp (argy, "gold"))
    amount *= 100; if (amount < 0)
    {
      send_to_char ("Your group wouldn't like that.\n\r", ch);
      return;
    }
  if (amount == 0)
  {
    send_to_char ("Split nothing?\n\r", ch); return;
  }
  if (tally_coins (ch) < amount)
  {
    send_to_char ("You don't have that much money.\n\r", ch);
    return;
  }
  copper = 0; gold = 0;
  sprintf (str1, "%s", name_amount (amount));
  if (amount >= 100)
  {
    gold = amount / 100; amount %= 100;
  }
  if (amount >= 1)
  {
    copper = amount; amount = 0;
  }

  while (ch->gold < gold)
  {
    ch->copper -= 100; ch->gold++;
  }
  while (ch->copper < amount)
  {
    ch->copper += 100; ch->gold--;
  }

  /*if (ch->gold<gold) 
    {
    send_to_char("You don't have that many gold coins!\n\r",ch); return;
    }
    if (ch->copper<copper) 
    {
    send_to_char("You don't have that many copper coins!\n\r",ch); return;
    } */
  members = 0;
  ch->pcdata->voting_on = 0;
  for (gch = ch->in_room->more->people; gch != NULL;
      gch = gch->next_in_room)
  {
    if (is_same_group (gch, ch) && IS_PLAYER (gch))
    {
      gch->pcdata->voting_on = 0; members++;
    }
  }
  if (members < 2)
    return; for (counter = 0; counter < 2; counter++)
    {
      if (counter == 0)
        amount = gold;
      if (counter == 1)
        amount = copper;
      if (amount < 1)
        continue;
      total_coinage = amount; share = 1;
      if (counter == 0)
      {
        amount *= 100; share *= 100;
      }
      sub_coins (amount, ch); while (total_coinage > 0)
      {
        create_amount (share, ch, NULL, NULL);	/*Initial coin to the char */
        ch->pcdata->voting_on += share;
        total_coinage--;
        if (total_coinage == 0)
          goto bah;
        for (gch = ch->in_room->more->people; gch != NULL;
            gch = gch->next_in_room)
        {
          if (IS_MOB (gch))
            continue;
          if (gch != ch && is_same_group (gch, ch))
          {
            total_coinage--;
            gch->pcdata->voting_on += share;
            create_amount (share, gch, NULL, NULL);
            if (total_coinage == 0) goto bah;
          }
        }
      }
bah:
      if (share)
      {
      };
    }
  /*End counter */
  if (ch->pcdata->voting_on != 0)
  {
    sprintf (buf, "You split %s coins.  You keep your share of %s coins.\n\r", str1, name_amount (ch->pcdata->voting_on));
    ch->pcdata->voting_on = 0;
    send_to_char (buf, ch);
  }
  for (gch = ch->in_room->more->people; gch != NULL;
      gch = gch->next_in_room)
  {
    if (IS_MOB (gch))
      continue;
    if (gch != ch && is_same_group (gch, ch))
    {
      if (gch->pcdata->voting_on == 0)
      {
        sprintf (buf, "$n splits %s coins, but there is not enough to go around, and you receive nothing.", str1); act (buf, ch, NULL, gch, TO_VICT);
            continue;
      }
      sprintf (str2, "%s", name_amount (gch->pcdata->voting_on));
      sprintf (buf, "$n splits %s coins.\n\r  Your get your portion... %s coins.", str1, str2); gch->pcdata->voting_on = 0;
      act (buf, ch, NULL, gch, TO_VICT);
    }
  }
  return;
}


bool
is_same_group (CHAR_DATA * ach, CHAR_DATA * bch)
{
  if (LEADER (ach) != NULL)
    ach = LEADER (ach);
  if (LEADER (bch) != NULL)
    bch = LEADER (bch); return ach == bch;
}

