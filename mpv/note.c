#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"

void save_notes (void);
bool is_note_to (CHAR_DATA * ch, NOTE_DATA * pnote);
void note_attach (CHAR_DATA * ch);
void note_remove (CHAR_DATA * ch, NOTE_DATA * pnote);
void note_delete (NOTE_DATA * pnote);
void add_read_note_stamp (CHAR_DATA * ch, time_t stamp);
void save_read_notes (CHAR_DATA * ch);
bool has_read_note (CHAR_DATA * ch, NOTE_DATA * note);

#define MAX_BOARDS 4
char *boards[MAX_BOARDS] = {
  "Blank",
  "Immortal Notice Board",
  "General Chat Board",
  "Ideas/Complaint Board"
};

bool
is_note_to (CHAR_DATA * ch, NOTE_DATA * pnote)
{
  if (!str_cmp (NAME (ch), pnote->sender))
    return TRUE;
  if (is_name ("all", pnote->to_list) || is_name ("clan", pnote->to_list))
    return TRUE;
  if (IS_IMMORTAL (ch) && is_name ("immortal", pnote->to_list))
    return TRUE;
  if (is_name (NAME (ch), pnote->to_list))
    return TRUE;
  if (is_name ("whities", pnote->to_list) && IS_GOOD (ch))
    return TRUE;
  if (is_name ("darkies", pnote->to_list) && IS_EVIL (ch))
    return TRUE;
  if (LEVEL (ch) >= 110)
    return TRUE;
  return FALSE;
}

void
note_attach (CHAR_DATA * ch)
{
  NOTE_DATA *pnote;
  check_ced (ch);
  if (ch->ced->pnote != NULL)
    return;
  if (note_free == NULL)
    {
      pnote = mem_alloc (sizeof (*ch->ced->pnote));
    }
  else
    {
      pnote = note_free;
      note_free = note_free->next;
    }
  pnote->next = NULL;
  pnote->sender = str_dup (NAME (ch));
  pnote->date = str_dup ("");
  pnote->to_list = str_dup ("");
  pnote->subject = str_dup ("");
  pnote->text = str_dup ("");
  pnote->lines = 0;
  ch->ced->pnote = pnote;
  ch->pcdata->edit_note = FALSE;
  return;
}

void
note_remove (CHAR_DATA * ch, NOTE_DATA * pnote)
{
  char to_new[SML_LENGTH];
  char to_one[SML_LENGTH];
  NOTE_DATA *prev;
  char *to_list;
  /*
   * Build a new to_list.
   * Strip out this recipient.
   */
  to_new[0] = '\0';
  to_list = pnote->to_list;
  while (*to_list != '\0')
    {
      to_list = one_argy (to_list, to_one);
      if (to_one[0] != '\0' && str_cmp (NAME (ch), to_one))
	{
	  strcat (to_new, " ");
	  strcat (to_new, to_one);
	}
    }
  /*
   * Just a simple recipient removal?
   */
  if (LEVEL (ch) < 110 && !is_note_to (ch, pnote))
    {
      if (str_cmp (NAME (ch), pnote->sender) && to_new[0] != '\0')
	{
	  free_string (pnote->to_list);
	  pnote->to_list = str_dup (to_new + 1);
	  return;
	}
    }
  /*
   * Remove note from linked list.
   */
  if (pnote == note_list)
    {
      note_list = pnote->next;
    }
  else
    {
      for (prev = note_list; prev != NULL; prev = prev->next)
	{
	  if (prev->next == pnote)
	    break;
	}
      if (prev == NULL)
	{
	  bug ("Note_remove: pnote not found.", 0);
	  return;
	}
      prev->next = pnote->next;
    }
  free_string (pnote->text);
  free_string (pnote->subject);
  free_string (pnote->to_list);
  free_string (pnote->date);
  free_string (pnote->sender);
  pnote->next = note_free;
  note_free = pnote;
  /*
   * Rewrite entire list.
   */
  save_notes ();
  return;
}

void
save_notes (void)
{
  NOTE_DATA *pnote;
  FILE *fp;
  if ((fp = fopen (NOTE_FILE, "w")) == NULL)
    {
      perror (NOTE_FILE);
    }
  else
    {
      for (pnote = note_list; pnote != NULL; pnote = pnote->next)
	{
	  fprintf (fp, "Sender %s~\n", fix_string (pnote->sender));
	  fprintf (fp, "TBoard %d\n", pnote->board_num);
	  fprintf (fp, "Date %s~\n", pnote->date);
	  fprintf (fp, "Stamp %ld\n", pnote->date_stamp);
	  fprintf (fp, "To %s~\n", fix_string (pnote->to_list));
	  fprintf (fp, "Subject %s~\n", fix_string (pnote->subject));
	  fprintf (fp, "Text\n%s~\n", fix_string (pnote->text));
	  fprintf (fp, "End\n");
	}
      fclose (fp);
    }
  return;
}

void
do_nfrm (CHAR_DATA * ch, char *argy)
{
  int board_here;
  SINGLE_OBJECT *obj;
  NOTE_DATA *pnote;
  int count = 0;
  char buf[STD_LENGTH];
  char buf1[STD_LENGTH * 8];
  int vnum;
  DEFINE_COMMAND ("nfrm", do_nfrm, POSITION_DEAD, 0, LOG_NORMAL, "Shows the number of unread notes.") if (IS_MOB (ch))
    return;
  board_here = 0;
  obj = get_obj_here (ch, "board", SEARCH_ROOM_FIRST);
  if (obj == NULL)
    board_here = 1;
  else
    board_here = obj->pIndexData->value[9];
  buf1[0] = '\0';
  sprintf (buf1,
	   "\x1B[34;1m------------------------------------------------------------------------\x1B[37;0m\n\r");
  sprintf (buf1 + strlen (buf1),
	   "\x1B[36;1m New  ###  Sender/From         Subject\x1B[37;0m\n\r");
  sprintf (buf1 + strlen (buf1),
	   "\x1B[34;1m------------------------------------------------------------------------\x1B[37;0m\n\r");
  for (pnote = note_list; pnote != NULL; pnote = pnote->next)
    {
      if (is_note_to (ch, pnote) &&
	  (board_here == pnote->board_num || IS_IMMORTAL (ch)) &&
	  str_cmp (NAME (ch), pnote->sender) && !has_read_note (ch, pnote))
	{
	  count++;
	  if (pnote->subject && strlen (pnote->subject) > 35)
	    pnote->subject[35] = '\0';
	  sprintf (buf,
		   "\x1B[34;1m(New) \x1B[37;1m%3d\x1B[37;0m  From \x1B[35;1m%-14s\x1B[37;0m '\x1B[37;1m%s\x1B[37;0m'\n\r",
		   vnum, pnote->sender,
		   pnote->subject ? pnote->subject : "(no subject)");
	  strcat (buf1, buf);
	  vnum++;
	}
      else
	vnum++;
      if (strlen (buf1) > (STD_LENGTH * 7))
	{
	  strcat (buf1, "\n\rTOO MANY NOTES.. WAIT UNTIL A GOD REMOVES SOME!\n\r");
	  break;
	}
    }
  if (count == 0)
    {
      sprintf (buf1, "You have no unread notes.\n\r");
      send_to_char (buf1, ch);
    }
  else
    {
      sprintf (buf1 + strlen (buf1),
	       "\x1B[34;1m------------------------------------------------------------------------\x1B[37;0m\n\r");
      sprintf (buf1 + strlen (buf1), "You have %d unread note%s.\n\r", count,
	       (count == 1 ? "" : "s"));
      page_to_char (buf1, ch);
    }
  return;
}

int
count_chars (char *text)
{
  int k = 0;
  char *t;
  if (text == NULL)
    return 0;
  for (t = text; *t != '\0'; t++)
    if (*t != '\0' && *t != '\n' && *t != '\r')
      k++;
  return k;
}

int
count_lines (char *text)
{
  int k = 0;
  char *t;
  if (text == NULL)
    return 0;
  for (t = text; *t != '\0'; t++)
    if (*t == '\n')
      k++;
  return k;
}

double
my_sqrt (double n)
{
  double i;
  if (n <= 0)
    return 0.0;
  for (i = 1.0; i < (n * 500.0); i += 1.0)
    if (i * i >= 1000000.0 * n)
      break;
  return ((double) (i / 1000.0));
};

void
notify_new_note (CHAR_DATA * ch, NOTE_DATA * pnote)
{
  CHAR_DATA *toch;
  char buf[STD_LENGTH];
  for (toch = char_list; toch != NULL; toch = toch->next)
    {
      if (!IS_PLAYER (toch))
	continue;
      if (str_cmp (NAME (toch), "Eraser") && pnote->board_num != 1)
	continue;
      if (toch != ch && is_note_to (toch, pnote) && !IS_IMMORTAL (toch))
	{
	  sprintf (buf, "There is a new note for you on the immortal noteboard.\n\r");
	  send_to_char (buf, toch);
	}
      else if (toch != ch && is_note_to (toch, pnote) && IS_IMMORTAL (toch))
	{
	  if (!str_cmp (NAME (toch), "Eraser") && pnote->board_num != 1)
	    sprintf (buf, "%s just posted a note.\n\r", NAME (ch));
	  else
	    sprintf (buf, "%s just posted a note on the immortal noteboard.\n\r", NAME (ch));
	  send_to_char (buf, toch);
	}
    }
  return;
}

void
do_notestats (CHAR_DATA * ch, char *argy)
{
  typedef struct statinfo STAT_INFO;
  struct statinfo
  {
    STAT_INFO *next;
    char *name;
    int times;
    int sum_chars;
    int min_chars;
    int max_chars;
  };
  NOTE_DATA *note;
  STAT_INFO *stat;
  STAT_INFO *first_stat = NULL;
  STAT_INFO *new_stat;
  bool found;
  char buf[STD_LENGTH];
  int max = 0;
  int sumnotes = 0;

  if (IS_MOB (ch))
    return;

  for (note = note_list; note != NULL; note = note->next)
    {
      sumnotes++;
      found = FALSE;
      for (stat = first_stat; stat != NULL; stat = stat->next)
	{
	  if (!strcmp (stat->name, note->sender))
	    {
	      int c;
	      stat->times++;
	      stat->sum_chars += (c = count_chars (note->text));
	      if (c < stat->min_chars)
		stat->min_chars = c;
	      if (c > stat->max_chars)
		stat->max_chars = c;
	      if (stat->times > max)
		max = stat->times;
	      found = TRUE;
	    }
	}
      if (found)
	continue;
      // create new stat
      {
	int c;
	new_stat = (STAT_INFO *) malloc (sizeof (STAT_INFO));
	new_stat->name = strdup (note->sender);
	new_stat->times = 1;
	if (max < 1)
	  max = 1;
	new_stat->sum_chars = (c = count_chars (note->text));
	new_stat->min_chars = c;
	new_stat->max_chars = c;
	new_stat->next = first_stat;
	first_stat = new_stat;
      }
    }
  send_to_char
    ("\x1B[34;1m--------------------------------------------------\x1B[37;0m\n\r",
     ch);
  send_to_char
    ("\x1B[36;1mSender           ###  MinChars  AvgChars  MaxChars\x1B[37;0m\n\r",
     ch);
  send_to_char
    ("\x1B[34;1m--------------------------------------------------\x1B[37;0m\n\r",
     ch);
  while (max > 0)
    {
      for (stat = first_stat; stat != NULL; stat = stat->next)
	{
	  if (stat->times == max)
	    {			// max > 0 => stat->times >= 0 so no division by zero
	      sprintf (buf,
		       "\x1B[35;1m%-15s  \x1B[36;1m%3d  \x1B[37;0m%8d  \x1B[37;1m%8d  \x1B[37;0m%8d\x1B[37;0m\n\r",
		       stat->name, stat->times, stat->min_chars,
		       (int) (stat->sum_chars / stat->times),
		       stat->max_chars);
	      send_to_char (buf, ch);
	    }
	}
      max--;
    }
  send_to_char
    ("\x1B[34;1m--------------------------------------------------\x1B[37;0m\n\r",
     ch);
  sprintf (buf, "\x1B[36;1mTotal notes:     %3d\x1B[37;0m\n\r", sumnotes);
  send_to_char (buf, ch);
  send_to_char
    ("\x1B[34;1m--------------------------------------------------\x1B[37;0m\n\r",
     ch);
  // free memory
  {
    STAT_INFO *stat_next;
    if (first_stat != NULL)
      stat_next = first_stat->next;
    else
      stat_next = NULL;
    for (stat = first_stat; stat != NULL; stat = stat_next)
      {
	stat_next = stat->next;
	if (stat->name != NULL)
	  free (stat->name);
	free (stat);
	stat = NULL;
      }
  }
  return;
}

bool
note_stamp_exists (time_t stamp)
{
  NOTE_DATA *pnote;
  for (pnote = note_list; pnote; pnote = pnote->next)
    {
      if (pnote->date_stamp == stamp)
	return TRUE;
    }
  return FALSE;
}

void
save_read_notes (CHAR_DATA * ch)
{
  NOTE_STAMP_LIST *list;
  FILE *fp;
  char notefile[100];
  if (!ch || IS_MOB (ch))
    return;
  sprintf (notefile, "%s%s.note", PLAYER_DIR, cap (NAME (ch)));
  if ((fp = fopen (notefile, "w")) == NULL)
    perror (notefile);
  else
    {
      for (list = ch->pcdata->read_notes; list; list = list->next)
	{
	  fprintf (fp, "%ld\n", list->stamp);
	}
      fprintf (fp, "End\n");
      fclose (fp);
    }
  return;
}

void
read_read_notes (CHAR_DATA * ch)
{
  FILE *fp;
  char notefile[100];
  char *word;
  if (!ch || IS_MOB (ch))
    return;
  sprintf (notefile, "%s%s.note", PLAYER_DIR, cap (NAME (ch)));
  if ((fp = fopen (notefile, "r")) == NULL)
    perror (notefile);
  else
    {
      for (;;)
	{
	  if (feof (fp))
	    break;
	  word = fread_word (fp);
	  if (!str_cmp (word, "End"))
	    break;
	  if (!is_number (word))
	    break;
	  if (note_stamp_exists (atoi (word)))
	    add_read_note_stamp (ch, atoi (word));
	}
      fclose (fp);
    }
  return;
}

void
add_read_note_stamp (CHAR_DATA * ch, time_t stamp)
{
  NOTE_STAMP_LIST *list;
  if (!ch || IS_MOB (ch))
    return;
  list = (NOTE_STAMP_LIST *) malloc (sizeof (NOTE_STAMP_LIST));
  list->stamp = stamp;
  list->next = ch->pcdata->read_notes;
  ch->pcdata->read_notes = list;
  return;
}

void
free_read_notes (CHAR_DATA * ch)
{
  NOTE_STAMP_LIST *list;
  NOTE_STAMP_LIST *list_next;
  if (!ch || IS_MOB (ch))
    return;
  list = ch->pcdata->read_notes;
  if (!list)
    return;
  for (; list; list = list_next)
    {
      list_next = list->next;
      if (list)
	{
	  free (list);
	  list = NULL;
	}
    }
  ch->pcdata->read_notes = NULL;
  return;
}

bool
has_read_note (CHAR_DATA * ch, NOTE_DATA * note)
{
  NOTE_STAMP_LIST *list;
  if (!ch || !note || IS_MOB (ch))
    return TRUE;
  for (list = ch->pcdata->read_notes; list; list = list->next)
    {
      if (list->stamp == note->date_stamp)
	return TRUE;
    }
  return FALSE;
}

void
do_note (CHAR_DATA * ch, char *argy)
{
  char buf[STD_LENGTH * 17];
  char buf1[STD_LENGTH * 17];
  char arg[SML_LENGTH];
  NOTE_DATA *pnote;
  int vnum;
  int board_here;		/*Boards and board nums by Owen Emlen */
  SINGLE_OBJECT *obj;
  int anum;
  DEFINE_COMMAND ("note", do_note, POSITION_DEAD, 0, LOG_NORMAL,
		  "This command is used to post/read/remove notes.")
    if (IS_MOB (ch))
    return;
  board_here = 0;
  obj = get_obj_here (ch, "board", SEARCH_ROOM_FIRST);
  if (obj == NULL)
    {
      board_here = 1;
    }
  else
    board_here = obj->pIndexData->value[9];
  buf1[0] = '\0';
  argy = one_argy (argy, arg);
  smash_tilde (argy);
  if (arg[0] == '\0')
    {
      do_note (ch, "read");
      return;
    }
  if (!str_cmp (arg, "new"))
    {
      do_nfrm (ch, "");
      return;
    }
  if (!str_cmp (arg, "list"))
    {
      vnum = 0;
      sprintf (buf1,
	       "\x1B[34;1m------------------------------------------------------------------------\x1B[37;0m\n\r");
      sprintf (buf1 + strlen (buf1),
	       "\x1B[36;1m New  ###  Sender/From         Subject\x1B[37;0m\n\r");
      sprintf (buf1 + strlen (buf1),
	       "\x1B[34;1m------------------------------------------------------------------------\x1B[37;0m\n\r");

      for (pnote = note_list; pnote != NULL; pnote = pnote->next)
	{
	  if (is_note_to (ch, pnote) &&
	      (board_here == pnote->board_num || IS_IMMORTAL (ch)))
	    {
	      if (strlen (pnote->subject) > 35)
		{
		  pnote->subject[35] = '\0';
		}
	      sprintf (buf,
		       "%s \x1B[37;1m%3d\x1B[37;0m  From \x1B[35;1m%-14s\x1B[37;0m '\x1B[37;1m%s\x1B[37;0m'\n\r",
		       (!has_read_note (ch, pnote)
			&& str_cmp (pnote->sender,
				    NAME (ch))) ? "\x1B[34;1m(New)\x1B[37;0m"
		       : "     ", vnum, pnote->sender, pnote->subject);
	      strcat (buf1, buf);
	      vnum++;
	    }
	  else
	    vnum++;
	  if (strlen (buf1) > (STD_LENGTH * 16))
	    {
	      strcat (buf1, "\n\rTOO MANY NOTES.. WAIT UNTIL A GOD REMOVES SOME!\n\r");
	      break;
	    }
	}
      /* MARMAR */
      page_to_char_limited (buf1, ch);
      return;
    }
  if (!str_cmp (arg, "cleanup") && IS_PLAYER (ch) && LEVEL (ch) == 111)
    {
      NOTE_DATA *pnext;
      char argone[100];
      char argtwo[100];
      int count = 0;
      int days;
      if (!argy || argy[0] == '\0')
	{
	  send_to_char ("Syntax: note cleanup <days>\n\r", ch);
	  return;
	}
      argy = one_argy (argy, argone);
      if (!is_number (argone))
	{
	  send_to_char ("Syntax: note cleanup <days>\n\r", ch);
	  return;
	}
      days = atoi (argone);
      for (pnote = note_list; pnote != NULL; pnote = pnext)
	{
	  if (pnote->date_stamp < current_time - days * 86400 &&
	      str_cmp (pnote->sender, "Kilith") &&
	      str_cmp (pnote->sender, "Eraser"))
	    {
	      note_remove (ch, pnote);
	      count++;
	      pnext = note_list;
	    }
	  else
	    pnext = pnote->next;
	}
      if (count > 0)
	{
	  char cb[100];
	  sprintf (cb, "Deleted %d note%s.\n\r", count,
		   count == 1 ? "" : "s");
	  send_to_char (cb, ch);
	}
      else
	send_to_char ("No old notes found.\n\r", ch);
      return;
    }
  if (!str_cmp (arg, "wipe") && IS_PLAYER (ch) && LEVEL (ch) == 111)
    {
      NOTE_DATA *pnext;
      char argone[100];
      char argtwo[100];
      int count = 0;
      if (!argy || argy[0] == '\0')
	{
	  send_to_char ("Note wipe of what player or range?\n\r", ch);
	  return;
	}
      argy = one_argy (argy, argone);
      one_argy (argy, argtwo);
      if (is_number (argone))	// range
	{
	  if (is_number (argtwo))
	    {
	      int nn1 = atoi (argone);
	      int nn2 = atoi (argtwo);
	      int number;
	      int idx;
	      char cb[100];
	      if (nn1 < 0 || nn2 < 0 || nn1 > nn2)
		{
		  send_to_char
		    ("Syntax: note wipe <start_note_no> <end_note_no>\n\r",
		     ch);
		  return;
		}
	      pnote = note_list;
	      idx = 0;
	      while (pnote && nn1 > idx)
		{
		  pnote = pnote->next;
		  idx++;
		}
	      if (!pnote)
		{
		  send_to_char
		    ("Syntax: note wipe <start_note_no> <end_note_no>\n\r",
		     ch);
		  return;
		}
	      number = nn2 - nn1 + 1;
	      count = 0;
	      while (count < number && pnote)
		{
		  pnext = pnote->next;
		  note_remove (ch, pnote);
		  pnote = pnext;
		  count++;
		}
	      sprintf (cb, "Deleted %d note%s.\n\r", count,
		       count == 1 ? "" : "s");
	      send_to_char (cb, ch);
	    }
	  else
	    send_to_char ("Syntax: note wipe <start_note_no> <end_note_no>\n\r", ch);
	  return;		// end wipe range
	}
      if (note_list)
	pnext = note_list->next;
      else
	pnext = NULL;
      // argone is some text
      if (!str_cmp (argone, "mortals"))
	{
	  for (pnote = note_list; pnote != NULL; pnote = pnext)
	    {
	      if (str_cmp (pnote->sender, "Kilith") &&
		  str_cmp (pnote->sender, "Eraser"))
		{
		  note_remove (ch, pnote);
		  count++;
		  pnext = note_list;
		}
	      else
		pnext = pnote->next;
	    }
	  if (count > 0)
	    {
	      char cb[100];
	      sprintf (cb, "Deleted %d note%s.\n\r", count,
		       count == 1 ? "" : "s");
	      send_to_char (cb, ch);
	    }
	  else
	    send_to_char ("No notes of mortals found.\n\r", ch);
	  return;		// end mortal note wipe
	}
      // argone is a name
      for (pnote = note_list; pnote != NULL; pnote = pnext)
	{
	  if (!str_cmp (pnote->sender, argone))
	    {
	      note_remove (ch, pnote);
	      count++;
	      pnext = note_list;
	    }
	  else
	    pnext = pnote->next;
	}
      if (count > 0)
	{
	  char cb[100];
	  sprintf (cb, "Deleted %d note%s.\n\r", count,
		   count == 1 ? "" : "s");
	  send_to_char (cb, ch);
	}
      else
	send_to_char ("No notes found of that player.\n\r", ch);
      return;
    }
  if (!str_cmp (arg, "read"))
    {
      bool fAll;
      if (!str_cmp (argy, "all"))
	{
	  fAll = TRUE;
	  anum = 0;
	}
      else if (argy[0] == '\0' || !str_prefix (argy, "next"))
	/* read next unread note */
	{
	  vnum = 0;
	  for (pnote = note_list; pnote != NULL; pnote = pnote->next)
	    {
	      if (is_note_to (ch, pnote) &&
		  (board_here == pnote->board_num || IS_IMMORTAL (ch))
		  && str_cmp (NAME (ch), pnote->sender)
		  && !has_read_note (ch, pnote))
		{
		  sprintf (buf,
			   "\x1B[33;1m---------------------------------------------------------------------------\x1B[37;0m\n\r");
		  sprintf (buf + strlen (buf),
			   "[Note #\x1B[37;1m%d:%d\x1B[37;0m] From: \x1B[35;1m%s\x1B[37;0m  To: \x1B[35;1m%s\x1B[37;0m\n\r",
			   vnum, pnote->board_num, pnote->sender,
			   pnote->to_list);
		  if (strlen (pnote->subject) > 35)
		    {
		      pnote->subject[35] = '\0';
		    }
		  sprintf (buf + strlen (buf),
			   "%s.  Subject: \x1B[37;1m%s\x1B[37;0m\n\r",
			   pnote->date, pnote->subject);
		  sprintf (buf + strlen (buf),
			   "\x1B[33;1m---------------------------------------------------------------------------\x1B[37;0m\n\r");
		  strcat (buf1, buf);
		  strcat (buf1, pnote->text);
		  add_read_note_stamp (ch, pnote->date_stamp);
		  save_read_notes (ch);
		  page_to_char_limited (buf1, ch);
		  return;
		}
	      else
		vnum++;
	    }
	  send_to_char ("You have no unread notes.\n\r", ch);
	  return;
	}
      else if (is_number (argy))
	{
	  fAll = FALSE;
	  anum = atoi (argy);
	}
      else
	{
	  send_to_char ("Note read which number?\n\r", ch);
	  return;
	}
      vnum = 0;
      for (pnote = note_list; pnote != NULL; pnote = pnote->next)
	{
	  if ((vnum++ == anum || fAll) && is_note_to (ch, pnote) &&
	      (board_here == pnote->board_num || IS_IMMORTAL (ch)))
	    {
	      sprintf (buf,
		       "\x1B[33;1m---------------------------------------------------------------------------\x1B[37;0m\n\r");
	      sprintf (buf + strlen (buf),
		       "[Note #\x1B[37;1m%d:%d\x1B[37;0m] From: \x1B[35;1m%s\x1B[37;0m  To: \x1B[35;1m%s\x1B[37;0m\n\r",
		       vnum - 1, pnote->board_num, pnote->sender,
		       pnote->to_list);
	      if (strlen (pnote->subject) > 35)
		{
		  pnote->subject[35] = '\0';
		}
	      sprintf (buf + strlen (buf),
		       "Dated %s.  Subject: \x1B[37;1m%s\x1B[37;0m\n\r",
		       pnote->date, pnote->subject);
	      sprintf (buf + strlen (buf),
		       "\x1B[33;1m---------------------------------------------------------------------------\x1B[37;0m\n\r");
	      strcat (buf1, buf);
	      strcat (buf1, pnote->text);
	      add_read_note_stamp (ch, pnote->date_stamp);
	      save_read_notes (ch);
	      page_to_char_limited (buf1, ch);
	      return;
	    }
	}
      send_to_char ("No such note.\n\r", ch);
      return;
    }
  if (!str_cmp (arg, "write"))
    {
      if (LEVEL (ch) < 100 && board_here == 1)
	{
	  send_to_char ("You can't write on the immortal notice board.\n\r",
			ch);
	  return;
	}
      check_ced (ch);
      note_attach (ch);
      string_append (ch, &ch->ced->pnote->text);
      return;
    }
  if (!str_cmp (arg, "fwrite"))
    {
      if (LEVEL (ch) < 100 && board_here == 1)
	{
	  send_to_char ("You can't write on the immortal notice board.\n\r",
			ch);
	  return;
	}
      check_ced (ch);
      note_attach (ch);
      fullscreen_editor (ch, &ch->ced->pnote->text);
      return;
    }
  if (!str_cmp (arg, "+"))
    {
      check_ced (ch);
      note_attach (ch);
      strcpy (buf, ch->ced->pnote->text);
      if (strlen (buf) + strlen (argy) >= STD_LENGTH - 200)
	{
	  send_to_char ("Note too long.\n\r", ch);
	  return;
	}
      strcat (buf, argy);
      strcat (buf, "\n\r");
      free_string (ch->ced->pnote->text);
      ch->ced->pnote->text = str_dup (buf);
      ch->ced->pnote->lines = count_lines (ch->ced->pnote->text);
      send_to_char ("Ok.\n\r", ch);
      return;
    }
  if (!str_cmp (arg, "-"))
    {
      int num, cnt;
      char new_buf[STD_LENGTH];
      const char *str;
      check_ced (ch);
      if (ch->ced->pnote == NULL)
	{
	  send_to_char ("You have to start a note first.\n\r", ch);
	  return;
	}
      ch->ced->pnote->lines = count_lines (ch->ced->pnote->text);
      if (ch->ced->pnote->lines < 1)
	{
	  send_to_char ("Nothing to delete.\n\r", ch);
	  return;
	}
      if (ch->ced->pnote->lines == 1)
	{
	  free_string (ch->ced->pnote->text);
	  ch->ced->pnote->text = strdup ("");
	  ch->ced->pnote->lines = 0;
	  send_to_char ("Ok.\n\r", ch);
	  return;
	}
      new_buf[0] = '\0';
      str = ch->ced->pnote->text;
      cnt = 1;
      num = 1;
      while (*str != '\0' && num <= ch->ced->pnote->lines)
	{
	  if (*str == '\r')
	    num++;
	  cnt++;
	  str++;
	}
      strncpy (new_buf, ch->ced->pnote->text, cnt);
      new_buf[cnt] = '\0';
      free_string (ch->ced->pnote->text);
      ch->ced->pnote->text = str_dup (new_buf);
      ch->ced->pnote->lines = count_lines (ch->ced->pnote->text);
      send_to_char ("Ok.\n\r", ch);
      return;
    }				// end -
  if (!str_cmp (arg, "subject"))
    {
      check_ced (ch);
      note_attach (ch);
      free_string (ch->ced->pnote->subject);
      ch->ced->pnote->subject = str_dup (argy);
      send_to_char ("Ok.\n\r", ch);
      return;
    }
  if (!str_cmp (arg, "to"))
    {
      check_ced (ch);
      note_attach (ch);
      free_string (ch->ced->pnote->to_list);
      ch->ced->pnote->to_list = str_dup (argy);
      send_to_char ("Ok.\n\r", ch);
      return;
    }
  if (!str_cmp (arg, "clear"))
    {
      check_ced (ch);
      if (ch->ced->pnote != NULL)
	{
	  if (!ch->pcdata->edit_note)
	    {
	      free_string (ch->ced->pnote->text);
	      free_string (ch->ced->pnote->subject);
	      free_string (ch->ced->pnote->to_list);
	      free_string (ch->ced->pnote->date);
	      free_string (ch->ced->pnote->sender);
	      ch->ced->pnote->next = note_free;
	      note_free = ch->ced->pnote;
	    }
	  ch->ced->pnote = NULL;
	}
      send_to_char ("Ok.\n\r", ch);
      return;
    }
  if (!str_cmp (arg, "show"))
    {
      check_ced (ch);
      if (ch->ced->pnote == NULL)
	{
	  send_to_char ("You have no note in progress.\n\r", ch);
	  return;
	}
      ch->ced->pnote->lines = count_lines (ch->ced->pnote->text);
      sprintf (buf, "From: %s  To: %s\n\rLines: %d  Subject: %s\n\r",
	       ch->ced->pnote->sender,
	       ch->ced->pnote->to_list ? ch->ced->pnote->to_list : "",
	       ch->ced->pnote->lines,
	       ch->ced->pnote->subject ? ch->ced->pnote->subject : "");
      strcat (buf1, buf);
      strcat (buf1, ch->ced->pnote->text);
      page_to_char (buf1, ch);
      return;
    }				// end show
  if (!str_cmp (arg, "post") || !str_prefix (arg, "send"))
    {
      FILE *fp;
      char *strtime;
      check_ced (ch);
      if (ch->ced->pnote == NULL)
	{
	  send_to_char ("You have no note in progress.\n\r", ch);
	  return;
	}
      if (board_here == 1 && LEVEL (ch) < 100)
	{
	  send_to_char
	    ("This board is reserved for immortals posting messages to mortals.\n\r",
	     ch);
	  send_to_char
	    ("Please post on a public board if you wish to write a note.\n\r",
	     ch);
	  return;
	}
      if (ch->ced->pnote->to_list == NULL ||
	  !str_cmp (ch->ced->pnote->to_list, "") ||
	  ch->ced->pnote->to_list[0] == '\0')
	{
	  send_to_char
	    ("You need to provide a recipient (name, all, whities, darkies or immortal).\n\r",
	     ch);
	  return;
	}
      if (ch->ced->pnote->subject == NULL ||
	  !str_cmp (ch->ced->pnote->subject, "") ||
	  ch->ced->pnote->subject[0] == '\0')
	{
	  send_to_char ("You need to provide a subject.\n\r", ch);
	  return;
	}
      if (ch->ced->pnote->text == NULL ||
	  !str_cmp (ch->ced->pnote->text, "") ||
	  ch->ced->pnote->text[0] == '\0' ||
	  count_chars (ch->ced->pnote->text) < 1)
	{
	  send_to_char
	    ("Posting an empty note er? Don't think so, Tim :)\n\r", ch);
	  return;
	}
      if (!ch->pcdata->edit_note)
	{
	  ch->ced->pnote->next = NULL;
	  strtime = ctime (&current_time);
	  strtime[strlen (strtime) - 1] = '\0';
	  ch->ced->pnote->date = str_dup (strtime);
	  ch->ced->pnote->board_num = board_here;
	  ch->ced->pnote->date_stamp = current_time;
	  if (note_list == NULL)
	    {
	      note_list = ch->ced->pnote;
	    }
	  else
	    {
	      for (pnote = note_list; pnote->next != NULL;
		   pnote = pnote->next);
	      pnote->next = ch->ced->pnote;
	    }
	  pnote = ch->ced->pnote;
	  notify_new_note (ch, pnote);
	  if ((fp = fopen (NOTE_FILE, "a")) == NULL)
	    {
	      perror (NOTE_FILE);
	    }
	  else
	    {
	      fprintf (fp, "Sender %s~\n", fix_string (pnote->sender));
	      fprintf (fp, "TBoard %d\n", pnote->board_num);
	      fprintf (fp, "Date %s~\n", pnote->date);
	      fprintf (fp, "Stamp %ld\n", pnote->date_stamp);
	      fprintf (fp, "To %s~\n", fix_string (pnote->to_list));
	      fprintf (fp, "Subject %s~\n", fix_string (pnote->subject));
	      fprintf (fp, "Text\n%s~\n", pnote->text);
	      fprintf (fp, "End\n\n");
	      fclose (fp);
	    }
	}
      else
	save_notes ();		// changed a note
      ch->ced->pnote = NULL;
      ch->pcdata->edit_note = FALSE;
      send_to_char ("Note posted.\n\r", ch);
      return;
    }
  if (IS_IMMORTAL (ch) && !str_prefix ("stat", arg))
    {
      do_notestats (ch, arg);
      return;
    }
  if (!str_cmp (arg, "edit"))
    {
      if (!is_number (argy))
	{
	  send_to_char ("Note edit which number?\n\r", ch);
	  return;
	}
      anum = atoi (argy);
      vnum = 0;
      for (pnote = note_list; pnote != NULL; pnote = pnote->next)
	{
	  if (vnum++ == anum &&
	      (!str_cmp (NAME (ch), pnote->sender) || LEVEL (ch) > 109))
	    {
	      check_ced (ch);
	      ch->ced->pnote = pnote;
	      ch->pcdata->edit_note = TRUE;
	      send_to_char ("Ok.\n\r", ch);
	      return;
	    }
	}
      send_to_char ("No such note.\n\r", ch);
      return;
    }
  if (!str_cmp (arg, "remove"))
    {
      if (!is_number (argy))
	{
	  send_to_char ("Note remove which number?\n\r", ch);
	  return;
	}
      anum = atoi (argy);
      vnum = 0;
      for (pnote = note_list; pnote != NULL; pnote = pnote->next)
	{
	  if (vnum++ == anum && ((is_note_to (ch, pnote)
				  && !is_name ("all", pnote->to_list))
				 || LEVEL (ch) > 109))
	    {
	      note_remove (ch, pnote);
	      send_to_char ("Ok.\n\r", ch);
	      return;
	    }
	}
      send_to_char ("No such note.\n\r", ch);
      return;
    }
  send_to_char ("Huh? Type 'help note' for usage.\n\r", ch);
  return;
}
