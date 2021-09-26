#include <sys/types.h>
#ifdef Linux
#include <sys/time.h>
#endif
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"


bool
check_pr (char *txt)
{
  char *t;
  bool last_a = FALSE;
  for (t = txt; *t != '\0'; t++)
    {
      if (*t == '%')
	return FALSE;
      if (*t == '$')
	{
	  last_a = TRUE;
	  continue;
	}
      if (last_a && (*t < '0' || *t > '9') && *t != 'B' && *t != 'R')
	{
	  return FALSE;
	}
      last_a = FALSE;
    }
  return TRUE;
}


extern bool prmpt;

void
print_prompt (CHAR_DATA * ch)
{
  static char buf[STD_LENGTH];
  static char hugebuf[2048];
  const char *i;
  int xii;
  hugebuf[0] = '\0';
  i = hugebuf;

  /*if (IS_PLAYER (ch) && IS_SET (ch->pcdata->act3, ACT3_MAPPING) && !IS_SET (ch->pcdata->act3, ACT3_DISABLED) && IS_SET (ch->pcdata->act2, PLR_PROMPT_UP))
    {
      if (ch->desc && ch->desc->host[29] != 1)
	gotoxy (ch, 25, 3);
      send_to_char ("\x1B[K", ch);
      if (FIGHTING (ch) != NULL)
	{
	  sprintf (buf, "\x1B[0;36m[Oppnt: %s\x1B[0;36m]\x1B[37;0m ", capitalize (STRING_HITS (FIGHTING (ch))));
	  send_to_char (buf, ch);
	  if (FIGHTING (FIGHTING (ch)) && FIGHTING (FIGHTING (ch)) != ch)
	    {
	      sprintf (buf, "\x1B[0;36m[\x1B[1;37m%s: %s\x1B[0;36m]\x1B[37;0m ", NAME (FIGHTING (FIGHTING (ch))), capitalize (STRING_HITS (FIGHTING (FIGHTING (ch)))));
	      send_to_char (buf, ch);
	    }
	}
      if (ch->desc && ch->desc->host[29] != 1)
	gotoxy (ch, 25, 2);
    }*/
  if (ch->wait > 1)
    send_to_char ("\x1B[1;30m[\x1B[1;31m*\x1B[1;30m] \x1B[0;37m", ch);
  if (IS_PLAYER (ch) && ch->pcdata->tickcounts > 11)
    send_to_char ("\x1B[1;30m[\x1B[1;31m+\x1B[1;30m] \x1B[0;37m", ch);
  if (IS_PLAYER (ch) && ch->fgt->ears > 10)
    {
      sprintf (buf, "\x1B[1;36m[\x1B[1;37m%d\x1B[1;36m]\x1B[0;37m ", ch->fgt->ears - 10);
      send_to_char (buf, ch);
    }
  if (ch->position == POSITION_CASTING && FIGHTING (ch) != NULL)
    {
      send_to_char ("(Casting) ", ch);
    }
    if (ch->position == POSITION_BACKSTAB)
    {
      send_to_char ("(BSing) ", ch);
    }
    if (ch->position == POSITION_CIRCLE)
    {
      send_to_char ("(Circling) ", ch);
    }
  if (ch->position == POSITION_BASHED)
    {
      send_to_char ("(Bashed) ", ch);
    }
  if (!pow.player_prompt)
    {
      if (IS_PLAYER (ch) && IS_SET (ch->pcdata->act2, PLR_BLANKPROMPT))
	{
	  char tmpss[500];
	  sprintf (tmpss, "\x1B[%d;52H\x1B[1K\x1B[%d;1f",
		   ch->pcdata->pagelen - 1, ch->pcdata->pagelen - 1);
	  send_to_char (tmpss, ch);
	}
      if (ch->desc)
	{
	  /*if (ch->desc->connected==CON_ROOMEDITOR) 
	     {
	     send_to_char("<Room Editor> ",ch);
	     } */
	  if (ch->desc->connected == CON_SEDITOR)
	    {
	      send_to_char ("[Spell Editor] ", ch);
	    }
	  if (ch->desc->connected == CON_PEDITOR)
	    {
	      send_to_char ("[Peditor] ", ch);
	    }
	}

      if (ch->position == POSITION_CASTING)
	{
	  if (IS_PLAYER (ch) && IS_SET (ch->pcdata->act2, PLR_BLANKPROMPT))
	    {
	      char tmpss[500];
	      sprintf (tmpss, "\x1B[%d;1f> ", ch->pcdata->pagelen - 3);
	      write_to_buffer (ch->desc, tmpss, 0);
	    }
	  return;
	}

      if (ch->position == POSITION_SEARCHING)
	{
	  if (IS_PLAYER (ch) && IS_SET (ch->pcdata->act2, PLR_BLANKPROMPT))
	    {
	      char tmpss[500];
	      sprintf (tmpss, "\x1B[%d;1f> ", ch->pcdata->pagelen - 3);
	      write_to_buffer (ch->desc, tmpss, 0);
	    }
	  /*if (IS_PLAYER (ch) && IS_SET (ch->pcdata->act3, ACT3_MAPPING) && !IS_SET (ch->pcdata->act3, ACT3_DISABLED))
	    {
	      send_to_char ("\x1B[K", ch);
	      gotoxy (ch, 1, ch->pcdata->pagelen);
	      send_to_char ("> ", ch);
	    }*/
	  return;
	}
      send_to_char ("\x1B[37;0m", ch);
      if (!pow.player_prompt)
	{
	  if (ch->hit < ch->max_hit)
	    {
	      sprintf (buf, "Hp: %s ", capitalize (STRING_HITS (ch)));
	      send_to_char (buf, ch);
	    }
	  if (FIGHTING (ch) != NULL)
	    {
	      sprintf (buf, "\x1B[0;36m[Oppnt: %s\x1B[0;36m]\x1B[37;0m ", capitalize (STRING_HITS (FIGHTING (ch))));
	      send_to_char (buf, ch);
	      if (FIGHTING (FIGHTING (ch)) && FIGHTING (FIGHTING (ch)) != ch)
		{
		  sprintf (buf, "\x1B[0;36m[\x1B[1;37m%s: %s\x1B[0;36m]\x1B[37;0m ", NAME (FIGHTING (FIGHTING (ch))),
			   capitalize (STRING_HITS (FIGHTING (FIGHTING (ch)))));
		  send_to_char (buf, ch);
		}
	      if (ch->move < ch->max_move - (ch->max_move / 5))
		send_to_char ("(", ch);
	    }
	  if (ch->move < ch->max_move - (ch->max_move / 5)
	      && ch->pcdata->prompt == 0)
	    {
	      if (FIGHTING (ch) != NULL)
		sprintf (buf, "Mv: %s) ", capitalize (STRING_MOVES (ch)));
	      else if (FIGHTING (ch) == NULL && ch->hit < ch->max_hit)
		sprintf (buf, "(Mv: %s) ", capitalize (STRING_MOVES (ch)));
	      else
		sprintf (buf, "Mv: %s ", capitalize (STRING_MOVES (ch)));
	      send_to_char (buf, ch);
	    }
	  if (IS_PLAYER (ch))
	    {
	      int xxx;
	      xii = max_mana (ch);
	      xxx = mana_now (ch);
	      if (xxx < (xii - (xii / 5)) && ch->pcdata->prompt == 0)
		{
		  if (LEVEL (ch) > 100)
		    sprintf (buf, "Mana: %s ", capitalize (string_mana_p (xxx)));
		  else
		    sprintf (buf, "Mana: %s", capitalize (string_mana_p (xxx)));
		  send_to_char (buf, ch);
		}
	      if (LEVEL (ch) > 100)
		{
		  sprintf (buf, "V-%d", ch->in_room->vnum);
		  send_to_char (buf, ch);
		}
	    }
	  send_to_char ("\x1B[37;0m> ", ch);
	  if (IS_PLAYER (ch) && IS_SET (ch->pcdata->act2, PLR_BLANKPROMPT))
	    {
	      char tmpss[500];
	      if (ch->desc != NULL)
		{
		  sprintf (tmpss, "\x1B[%d;1f", ch->pcdata->pagelen - 3);
		  write_to_descriptor2 (ch->desc, tmpss, 0);
		  sprintf (tmpss, "\x1B[%d;1f> %s", ch->pcdata->pagelen - 3, ch->desc->inbuf);
		  write_to_buffer (ch->desc, tmpss, 0);
		}
	    }
	  /*if (IS_PLAYER (ch) && IS_SET (ch->pcdata->act3, ACT3_MAPPING) && !IS_SET (ch->pcdata->act3, ACT3_DISABLED))
	    {
	      send_to_char ("\x1B[K", ch);
	      gotoxy (ch, 1, ch->pcdata->pagelen);
	      send_to_char ("> ", ch);
	    }*/
	  return;
	}

      if (IS_PLAYER (ch) && IS_SET (ch->pcdata->act2, PLR_BLANKPROMPT))
	{
	  char tmpss[500];
	  if (ch->desc != NULL)
	    {
	      sprintf (tmpss, "\x1B[%d;1f", ch->pcdata->pagelen - 3);
	      write_to_descriptor2 (ch->desc, tmpss, 0);
	      sprintf (tmpss, "\x1B[%d;1f> %s", ch->pcdata->pagelen - 3,
		       ch->desc->inbuf);
	      write_to_buffer (ch->desc, tmpss, 0);
	    }
	}
      /*if (IS_PLAYER (ch) && IS_SET (ch->pcdata->act3, ACT3_MAPPING) && !IS_SET (ch->pcdata->act3, ACT3_DISABLED))
	{
	  send_to_char ("\x1B[K", ch);
	  gotoxy (ch, 1, ch->pcdata->pagelen);
	  send_to_char ("> ", ch);
	}*/
      return;
    }
  else
    {
      int ii;
      char intl[3000];
      char *t;
      wwrap = FALSE;
      ii = ch->position;
      ch->position = POSITION_STANDING;
      ch->pcdata->convo[199] = 0;
      prmpt = TRUE;
      strcpy (intl, interpret_line (ch->pcdata->rprompt, ch));
      prmpt = FALSE;
      for (t = intl; *t != '\0'; t++)
	{
	  if (*t == '$')
	    {
	      t++;
	      continue;
	    }
	  if (*t == '\x1B')
	    {
	      t++;
	      while (*t != 'm' && *t != 'f' && *t != 'H' && *t != 'r'
		     && *t != 'J' && *t != '\0')
		t++;
	      continue;
	    }
	  if (*t == '\n' || ch->pcdata->convo[199] > 79)
	    {
	      ch->pcdata->convo[199] = 0;
	    }
	  ch->pcdata->convo[199]++;
	}
      ch->pcdata->convo[199]++;
      act (intl, ch, NULL, ch, TO_CHAR);
      ch->position = ii;
      wwrap = TRUE;
    }
  if (ch->desc && ch->desc->host[29] == 1)
    gotoxy (ch, ch->pcdata->convo[199], ch->pcdata->pagelen);

  /*if (IS_PLAYER (ch) && IS_SET (ch->pcdata->act3, ACT3_MAPPING) && !IS_SET (ch->pcdata->act3, ACT3_DISABLED) && IS_SET (ch->pcdata->act2, PLR_PROMPT_UP))
    {
      send_to_char ("\x1B[K", ch);
      if (ch->desc && ch->desc->host[29] != 1)
	{
	  gotoxy (ch, 1, ch->pcdata->pagelen);
	  send_to_char ("> ", ch);
	}
    }*/
  if (ch->desc && ch->desc->host[29] == 1)
    {
      send_to_char (ch->desc->inbuf, ch);
      ch->pcdata->convo[199] += strlen (ch->desc->inbuf);
    }
  return;
}

void
try_prompt (char *tx, CHAR_DATA * ch)
{
  static char buf[STD_LENGTH];
  static char hugebuf[2048];
  int ii;
  const char *i;
  int xii;
  hugebuf[0] = '\0';
  i = hugebuf;
  send_to_char ("\x1B[37;0m", ch);
  wwrap = FALSE;
  ii = ch->position;
  ch->position = POSITION_STANDING;
  act (interpret_line (tx, ch), ch, NULL, ch, TO_CHAR);
  ch->position = ii;
  wwrap = TRUE;
  send_to_char ("\n\r", ch);
  return;
}


char *prompts[] = {
  "$B$1*hn*/*hm*$R$7hp$B$2 *mn*/*mm*$R$7mv $B$5*mnn*/*mmm*$R$7m *fi**tk*> ",
  "$R$3*hn*$7/$3*hm*$7h$2 *mn*$7/$2*mm*$7mv $5*mnn*$7/$5*mmm*$7m *fi**tk*> ",
  "$B$1*hn*/*hm*$R$7hp$B$7(*hw*) $B$2*mn*/*mm*$R$7mv $B$3*mnn*/*mmm*$R$7m $B$5(*xl*)$R$7xp $B$7*fi* *tk*$R> ",
  "$B$2*hn*/*hm*$7hp$R $B$6*mn*/*mm*$7mv$R $B$3*mnn*/*mmm*$7ma$R $B$5(*xl*)$R $B$1*ob*$7ob$R $B$6*eb*$7eb$R $B$3*apb*$7ap$R $B$7*fi* *tk*$R> ",
  "$B$0{$2*hn*$0/$2*hm*$B$0}hp {$2*mn*$B$0/$2*mm*$B$0}mv {$2*mnn*$0/$2*mmm*$0}ma {$2*xl*$0}xp $B$0*fi* *tk*$R> ",
  "$B$7{$5*hn*$7/$R$5*hm*$B$7hp}-{$3*mn*$7/$R$3*mm*$B$7mv}-{$6*mnn*$7/$R$6*mmm*$B$7mn}-{$1*xl*$7xp} $R$2*ap*apb $B*fi* *tk*$R> ",
  "$7$BL:*lvl* [*hw*] $4*hn*$7|$R$4*hm* $B$3*mn*$7|$R$3*mm* $1$B*mnn*$7|$R$1*mmm* $B$4*ap*$0-$R$6*wp*$0$B-$7$B*kw* $R$6-{*xl*}-$B $2*tk* $R$3*fi*$R> ",
  "$7$BL:*lvl* [*hw*] $4*hn*$7|$R$4*hm* $B$3*mn*$7|$R$3*mm* $1$B*mnn*$7|$R$1*mmm* $B$7*kw*$0-$R$6*wp*$0$B-$7$B*wim* $R$6-{*xl*}-$B $2*tk* $R$3*fi*$R> ",
  "$B$2*hn*$R$7hp $B$6*mn*$R$7mv $B$3*mnn*$R$7ma $B$5(*xl*)$R$7 *fi**tk*> ",
  "$R$3*hn*$7/$3*hm*$7hp $5*mn*$7/$5*mm*mvs $6*mnn*$7/$6*mmm*m $7(*xl*xp) $4(*kw*wgt)$7*tk*$4|$$7*fi*$R> ",
  "$RH:*hn* M:*mn* Ma:*mnn* XL:*xl* *fi**tk*> ",
  "$B$2*hn*$R$7h $B$6*mn*$R$7m $B$1(*xl*)$R$7 *fi**tk*> ",
  "$R$2*hn*$7h $6*mn*$7m $5(*xl*)$7 *fi**tk*> ",
  "$R$2*hn*$7h $6*mn*$7mv $B$7*mnn*$R$7ma $5(*xl*)$7 *fi**tk*> ",
  "$R$2H:*hn*/*hm* $3M:*mn*/*mm* $5XL:*xl*$7 *fi**tk*> ",
  "$B$2H:*hn*/*hm* $B$6M:*mn*/*mm* $B$1XL:*xl*$R$7 *fi**tk*> ",
  "END"
};


void
set_default_prompt (CHAR_DATA * ch, char option)
{
  int num;
  int cnt;

  if (!ch)
    return;
  option = UPPER (option);
  if (option < 'A' || option > 'P')
    return;

  if (ch->pcdata->rprompt)
    free_string (ch->pcdata->rprompt);
  ch->pcdata->rprompt = str_dup (prompts[(option - 'A')]);

  return;
}
