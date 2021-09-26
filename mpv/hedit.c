#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"

HELP_DATA *help_index_free;

void
show_hedit (HELP_DATA * pHelp, CHAR_DATA * ch)
{
  char buf[500];
  if (!ch || IS_MOB (ch))
    return;
  if IS_SET
    (ch->pcdata->act2, PLR_ASCII)
    {
      sprintf (buf,
          "\n\r\x1B[1;30mÄÄÄÄÄÄ\x1B[0;31mÄÄÄÄÄÄÄÄÄÄÄÄÄ\x1B[1;31mÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ\x1B[0;31mÄÄÄÄÄÄÄÄÄÄÄÄÄ\x1B[1;30mÄÄÄÄÄÄ\x1B[0m\n\r");
    }
  else
  {
    sprintf (buf,
        "\n\r\x1B[1;30m------\x1B[0;31m-------------\x1B[1;31m-----------------------------------------\x1B[0;31m-------------\x1B[1;30m------\x1B[0m\n\r");
  }
  send_to_char (buf, ch);

  sprintf (buf,
      "Help name\x1B[1;38m: \x1B[1;32m[\x1B[0;32m%s\x1B[1m]\x1B[0m\n\r",
      pHelp->keyword);
  send_to_char (buf, ch);
  sprintf (buf,
      "Help level\x1B[1;38m: \x1B[1;32m[\x1B[0;32m%d\x1B[1m]\x1B[0m\n\r",
      pHelp->level);
  send_to_char (buf, ch);

  if IS_SET
    (ch->pcdata->act2, PLR_ASCII)
    {
      sprintf (buf,
          "\x1B[1;30mÄÄÄÄÄÄ\x1B[0;31mÄÄÄÄÄÄÄÄÄÄÄÄÄ\x1B[1;31mÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ\x1B[0;31mÄÄÄÄÄÄÄÄÄÄÄÄÄ\x1B[1;30mÄÄÄÄÄÄ\x1B[0m\n\r");
    }
  else
  {
    sprintf (buf,
        "\x1B[1;30m------\x1B[0;31m-------------\x1B[1;31m-----------------------------------------\x1B[0;31m-------------\x1B[1;30m------\x1B[0m\n\r");
  }
  send_to_char (buf, ch);
  return;
}

void
save_helps ()
{
  HELP_DATA *pHelp;
  FILE *fp;
  if ((fp = fopen ("help.are", "w")) == NULL)
  {
    bug ("Save_helps: fopen", 0);
    perror ("help.are");
    return;
  }
  fprintf (fp, "#HELPS\n\n");
  for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
  {
    if (strlen (pHelp->text) > 5 && pHelp->text[0] && pHelp->text[0] != '\0')
    {
      fprintf (fp, "%d %s~", pHelp->level, fix_string (pHelp->keyword));
      fprintf (fp, "\n");
      fprintf (fp, "%s", fix_string (pHelp->text));
      fprintf (fp, "~\n\n");
    }
  }
  fprintf (fp, "\n\n0 $~\n");
  fprintf (fp, "#$\n");
  fclose (fp);
  return;
}

HELP_DATA *
new_help_index (char *argy)
{
  HELP_DATA * pHelp;
  pHelp = mem_alloc (sizeof (*pHelp));
  pHelp->level = 0;
  pHelp->keyword = str_dup (argy);
  pHelp->text = str_dup ("");

  if (help_first == NULL)
    help_first = pHelp;
  if (help_last != NULL)
    help_last->next = pHelp;
  help_last = pHelp;
  pHelp->next = NULL;
  top_help++;
  return pHelp;
}

void
hedit (CHAR_DATA * ch, char *argy)
{
  HELP_DATA *pHelp;
  HELP_DATA *Help;
  char interpret_buff[STD_LENGTH];
  char arg[STD_LENGTH];

  pHelp = (HELP_DATA *) ch->desc->pEdit;

  strcpy (interpret_buff, argy);
  argy = one_argy (argy, arg);

  if (!str_cmp (arg, "show") || arg[0] == '\0')
  {
    show_hedit (pHelp, ch);
    return;
  }
  if (!str_cmp (arg, "level"))
  {
    if (!is_number (argy) || argy[0] == '\0')
    {
      send_to_char ("Syntax: level <level>\n\r", ch);
      return;
    }
    pHelp->level = atoi (argy);
    send_to_char ("Help level set.\n\r", ch);
    show_hedit (pHelp, ch);
    return;
  }
  if (!str_cmp (arg, "keyword"))
  {
    if (argy[0] == '\0')
    {
      send_to_char ("Syntax: keyword <keyword>", ch);
      return;
    }
    for (Help = help_first; Help != NULL; Help = Help->next)
    {
      if (is_name (argy, Help->keyword)
          && !is_name (Help->keyword, pHelp->keyword))
      {
        send_to_char
          ("\n\rA helpfile with that keyword already exists.\n\r", ch);
        return;
      }
    }
    pHelp->keyword = str_dup (argy);
    send_to_char ("\n\rHelp keyword set.\n\r", ch);
    return;
  }
  if (!str_cmp (arg, "done"))
  {
    save_helps ();
    send_to_char ("\n\rHelp file Saved\n\r", ch);
    ch->desc->pEdit = NULL;
    ch->desc->connected = CON_PLAYING;
    return;
  }
  if (!str_cmp (arg, "edit"))
  {
    string_append (ch, &pHelp->text);
    return;
  }
  interpret (ch, interpret_buff);
  return;
}

void
do_hedit (CHAR_DATA * ch, char *argy)
{
  HELP_DATA *pHelp;
  char arg[STD_LENGTH];

  DEFINE_COMMAND ("hedit", do_hedit, POSITION_DEAD, 110, LOG_ALWAYS,
      "This command allow you to enter help editor.");

  if (IS_MOB (ch))
    return;

  argy = one_argy (argy, arg);

  if (!str_cmp (arg, "create"))
  {
    if (argy[0] == '\0')
    {
      send_to_char ("Syntax: create <keyword>\n\r", ch);
      return;
    }
    for (pHelp = help_first; pHelp && pHelp != NULL; pHelp = pHelp->next)
    {
      if (is_name (argy, pHelp->keyword))
      {
        send_to_char
          ("\n\rA helpfile with that keyword already exists.\n\r", ch);
        return;
      }
    }
    pHelp = new_help_index (argy);
    ch->desc->pEdit = (void *) pHelp;
    ch->desc->connected = CON_HEDITOR;
    send_to_char ("Helpfile Created.\n\r", ch);
    show_hedit (pHelp, ch);
    return;
  }
  for (pHelp = help_first; pHelp && pHelp != NULL; pHelp = pHelp->next)
  {
    if (is_name (arg, pHelp->keyword))
    {
      ch->desc->pEdit = pHelp;
      ch->desc->connected = CON_HEDITOR;
      show_hedit (pHelp, ch);
      return;
    }
  }

  send_to_char ("SYNTAX: Hedit <keyword>\n\r", ch);
  return;
}
