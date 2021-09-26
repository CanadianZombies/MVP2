#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"

// prototyping
bool is_poll_to (CHAR_DATA * ch, POLL_DATA * poll);
void poll_attach (CHAR_DATA * ch);
void poll_remove (CHAR_DATA * ch, POLL_DATA * poll);
void poll_delete (POLL_DATA * poll);
void add_read_poll_stamp (CHAR_DATA * ch, time_t stamp, short choice);
void save_read_polls (CHAR_DATA * ch);
POLL_STAMP_LIST *has_read_poll (CHAR_DATA * ch, POLL_DATA * poll);
int count_chars (char *text);
int count_lines (char *text);
int count_options (POLL_DATA * poll);
void notify_new_poll (CHAR_DATA * ch, POLL_DATA * poll);
int command_level (char *);

void
save_polls (void)		// save polls to disk
{
  FILE *fp;
  POLL_DATA *poll;
  int opt;
  if ((fp = fopen (POLL_FILE, "w")) == NULL)
    perror (POLL_FILE);
  else
  {
    for (poll = poll_list; poll != NULL; poll = poll->next)
    {
      fprintf (fp, "Sender %s~\n", fix_string (poll->sender));
      fprintf (fp, "Date %s~\n", poll->date);
      fprintf (fp, "Stamp %ld\n", poll->date_stamp);
      fprintf (fp, "To %s~\n", fix_string (poll->to_list));
      fprintf (fp, "Level %d\n", poll->level);
      fprintf (fp, "Subject %s~\n", fix_string (poll->subject));
      fprintf (fp, "Text\n%s~\n", fix_string (poll->text));
      for (opt = 0; opt < MAX_OPTION; opt++)
        if (poll->options[opt])
          fprintf (fp, "Option %d %s~\n", opt,
              fix_string (poll->options[opt]));
      for (opt = 0; opt < MAX_OPTION; opt++)
        if (poll->options[opt])
          fprintf (fp, "Votes %d %d\n", opt, poll->votes[opt]);

      fprintf (fp, "End\n");
    }
    fclose (fp);
  }
}

bool
is_poll_to (CHAR_DATA * ch, POLL_DATA * poll)
{				// used to determine whether ch can read the poll info
  if ((LEVEL (ch) < poll->level) && !IS_IMMORTAL (ch))
    return FALSE;
  if (is_name ("all", poll->to_list))
    return TRUE;
  if (is_name ("immortal", poll->to_list) && IS_IMMORTAL (ch))
    return TRUE;
  if (is_name ("admin", poll->to_list) && IS_IMMORTAL (ch))
    return TRUE;
  if (is_name (NAME (ch), poll->to_list))
    return TRUE;
  if (LEVEL (ch) >= 110)
    return TRUE;
  return FALSE;
}

bool
can_vote (CHAR_DATA * ch, POLL_DATA * poll)
{				// is the player allowed to vote on this poll?
  if (LEVEL (ch) < poll->level)
    return FALSE;
  if (is_name ("all", poll->to_list))
    return TRUE;
  if (is_name ("immortal", poll->to_list) && IS_IMMORTAL (ch))
    return TRUE;
  if (is_name ("admin", poll->to_list) && IS_IMMORTAL (ch))
    return TRUE;
  if (is_name (NAME (ch), poll->to_list))
    return TRUE;
  return FALSE;
}

void
poll_attach (CHAR_DATA * ch)
{				// setup poll data on character
  POLL_DATA *poll;
  int opt;
  check_ced (ch);
  if (ch->ced->poll != NULL)
    return;
  if (poll_free == NULL)
    poll = mem_alloc (sizeof (POLL_DATA));
  else
  {
    poll = poll_free;
    poll_free = poll_free->next;
  }
  poll->next = NULL;
  poll->sender = str_dup (NAME (ch));
  poll->date = str_dup (" ");
  poll->to_list = NULL;
  poll->subject = NULL;
  poll->text = str_dup ("\r");
  for (opt = 0; opt < MAX_OPTION; opt++)
  {
    poll->options[opt] = NULL;
    poll->votes[opt] = 0;
  }
  poll->open = TRUE;
  poll->publicstats = FALSE;
  poll->level = 0;
  ch->ced->poll = poll;
  send_to_char ("Creating new poll.\n\r", ch);
  return;
}

void
poll_remove (CHAR_DATA * ch, POLL_DATA * poll)
{				// remove a poll, called by do_poll
  FILE *fp;
  POLL_DATA *prev;
  int opt;
  if (poll == poll_list)
    poll_list = poll->next;
  else
  {
    for (prev = poll_list; prev != NULL; prev = prev->next)
    {
      if (prev->next == poll)
        break;
    }
    if (prev == NULL)
    {
      bug ("Poll_remove: poll not found.", 0);
      return;
    }
    prev->next = poll->next;
  }
  free_string (poll->text);
  free_string (poll->subject);
  free_string (poll->to_list);
  free_string (poll->date);
  free_string (poll->sender);
  for (opt = 0; opt < MAX_OPTION; opt++)
    free_string (poll->options[opt]);
  poll->next = poll_free;
  poll_free == poll;
  save_polls ();
  return;
}

int
command_level (char *comm)
{
  COMMAND *c;
  bool fnd = FALSE;
  if (!comm || comm[0] == '\0')
    return 0;
  for (c = command_hash[toupper(comm[0])]; c && !fnd; c = c->next)
  {
    if (!strcmp (c->name, comm))
    {
      fnd = TRUE;
      break;
    }
  }
  if (c && fnd)
    return c->level;
  else
    return MAX_LEVEL + 1;
}

void
poll_details (CHAR_DATA * ch, time_t stamp)
{
  PLAYERBASE_DATA *playerbase;
  FILE *fp;
  char *word;
  char *choice;
  char bufx[500];
  for (playerbase = playerbase_zero; playerbase != NULL;
      playerbase = playerbase->next)
  {
    sprintf (bufx, "%s%s.poll", PLAYER_DIR_2,
        capitalize (playerbase->player_name));
    if ((fp = fopen (bufx, "r")) == NULL)
      continue;
    for (;;)
    {
      if (feof (fp))
        break;
      word = fread_word (fp);
      if (!str_cmp (word, "End"))
        break;
      if (!is_number (word))
        break;
      if (feof (fp))
        break;
      choice = fread_word (fp);
      if (!str_cmp (word, "End"))
        break;
      if (!is_number (choice))
        break;
      if (stamp == atoi (word))
      {
        sprintf (bufx, "%s voted ^[[1m%c^[[0m.\n\r",
            capitalize (playerbase->player_name),
            (char) (atoi (choice) + 'A'));
        send_to_char (bufx, ch);
        break;
      }
    }
    fclose (fp);
  }
  return;
}

void
do_poll (CHAR_DATA * ch, char *argy)
{
  char arg[STD_LENGTH];
  DEFINE_COMMAND ("poll", do_poll, POSITION_DEAD, 111, LOG_NORMAL,
      "See poll information or statistics.")
  if (IS_MOB (ch))
    return;
  argy = one_argy (argy, arg);
  smash_tilde (argy);
  if (arg[0] == '\0')
  {				// if no args, read the first poll you didn't vote for
    do_poll (ch, "read");
    return;
  }
  if (!str_cmp (arg, "list"))
  {				// list all polls
    char buf[STD_LENGTH * 20];
    char sbuf[STD_LENGTH];
    char number = 0;
    POLL_DATA *poll;
    sprintf (buf,
        "\x1B[34;1m------------------------------------------------------------------------\x1B[37;0m\n\r");
    sprintf (buf + strlen (buf),
        "\x1B[36;1m New  ###  Open  From                Subject\x1B[37;0m\n\r");
    sprintf (buf + strlen (buf),
        "\x1B[34;1m------------------------------------------------------------------------\x1B[37;0m\n\r");
    for (poll = poll_list; poll; poll = poll->next)
    {
      if (is_poll_to (ch, poll))
      {
        if (poll->subject && strlen (poll->subject) > 35)
          poll->subject[35] = '\0';
        sprintf (sbuf,
            "%s \x1B[37;1m%3d\x1B[37;0m %s From \x1B[35;1m%-14s\x1B[37;0m '\x1B[37;1m%s\x1B[37;0m'\n\r",
            !has_read_poll (ch,
              poll) ? "\x1B[34;1m(New)\x1B[37;0m" :
            "     ", number,
            poll->open ? "\x1B[32;1m(Open)\x1B[37;0m" : "      ",
            poll->sender, poll->subject ? poll->subject : "");
        strcat (buf, sbuf);
      }
      number++;
      if (strlen (buf) > (STD_LENGTH * 19))
      {
        strcat (buf,
            "\n\rToo many polls... wait until a god removes some!\n\r");
        break;
      }
    }
    page_to_char (buf, ch);
    return;
  }				// end list
  if (!str_cmp (arg, "vote"))
  {				// bring out your vote on a certain poll
    int number = 0;
    POLL_DATA *poll;
    POLL_STAMP_LIST *list;
    char arg1[STD_LENGTH];
    char arg2[STD_LENGTH];
    int pollno = -1;
    short choice = -1;
    bool fnd = FALSE;
    argy = one_argy (argy, arg1);
    argy = one_argy (argy, arg2);
    pollno = atoi (arg1);
    choice = (short) (toupper (arg2[0]) - 'A');
    for (poll = poll_list; poll && !fnd; poll = poll->next)
    {
      if ((number++ == pollno) && is_poll_to (ch, poll))
      {
        fnd = TRUE;
        break;
      }
    }
    if (!poll || !fnd)
    {
      send_to_char ("Poll not found.\n\r", ch);
      return;
    }
    if (!can_vote (ch, poll))
    {
      send_to_char
        ("You are not allowed to vote for this poll, sorry.\n\r", ch);
      return;
    }
    if ((list = has_read_poll (ch, poll)) != NULL)
    {
      sprintf (arg1,
          "You already voted for that poll (you voted \x1B[36;1m%c\x1B[37;0m).\n\r",
          (char) (list->choice + 'A'));
      send_to_char (arg1, ch);
      return;
    }
    if (!poll->open)
    {
      send_to_char ("That poll is closed, sorry.\n\r", ch);
      return;
    }
    if ((choice < 0) || (choice > MAX_OPTION - 1) || !poll->options[choice])
    {
      send_to_char ("Invalid choice.\n\r", ch);
      return;
    }
    poll->votes[choice]++;
    add_read_poll_stamp (ch, poll->date_stamp, choice);
    save_read_polls (ch);
    save_polls ();
    sprintf (arg1,
        "You voted [\x1B[36;1m%c\x1B[37;0m], thanks for voting!\n\r",
        (char) (choice + 'A'));
    send_to_char (arg1, ch);
    return;
  }				// end vote
  if (!str_cmp (arg, "write"))
  {				// write the text for the poll
    if (!IS_IMMORTAL (ch))
    {
      send_to_char ("You are not allowed to set up a poll.\n\r", ch);
      return;
    }
    check_ced (ch);
    poll_attach (ch);
    string_append (ch, &ch->ced->poll->text);
    return;
  }				// end write
  if (!str_cmp (arg, "option"))
  {				// set an option
    int opt;
    char arg1[STD_LENGTH];
    if (!IS_IMMORTAL (ch))
    {
      send_to_char ("You are not allowed to set up a poll.\n\r", ch);
      return;
    }
    check_ced (ch);
    poll_attach (ch);
    argy = one_argy (argy, arg1);
    opt = toupper (arg1[0]) - 'A';
    if ((opt < 0) || (opt > MAX_OPTION - 1))
    {
      send_to_char ("Invalid option.\n\r", ch);
      return;
    }
    if (!str_cmp (argy, "clear"))
    {
      int cnt = count_options (ch->ced->poll) - 1;
      free_string (ch->ced->poll->options[opt]);
      ch->ced->poll->options[opt] = ch->ced->poll->options[cnt];
      ch->ced->poll->options[cnt] = NULL;
      send_to_char ("Option cleared.\n\r", ch);
      return;
    }
    if (opt > count_options (ch->ced->poll))
    {
      send_to_char ("Please setup earlier options first.\n\r", ch);
      return;
    }
    free_string (ch->ced->poll->options[opt]);
    ch->ced->poll->options[opt] = str_dup (argy);
    send_to_char ("Option set.\n\r", ch);
    return;
  }
  if (!str_cmp (arg, "+"))
  {				// add a line of text to the text of the poll
    char buf[STD_LENGTH * 20];
    if (!IS_IMMORTAL (ch))
    {
      send_to_char ("You are not allowed to set up a poll.\n\r", ch);
      return;
    }
    check_ced (ch);
    poll_attach (ch);
    strcpy (buf, ch->ced->poll->text);
    if (strlen (buf) + strlen (argy) >= STD_LENGTH - 200)
    {
      send_to_char ("Text too long.\n\r", ch);
      return;
    }
    strcat (buf, argy);
    strcat (buf, "\n\r");
    free_string (ch->ced->poll->text);
    ch->ced->poll->text = str_dup (buf);
    send_to_char ("String appended.\n\r", ch);
    return;
  }				// end +
  if (!str_cmp (arg, "-"))
  {				// remove the last line from the poll text
    int num, cnt;
    char new_buf[STD_LENGTH];
    const char *str;
    if (!IS_IMMORTAL (ch))
    {
      send_to_char ("You are not allowed to set up a poll.\n\r", ch);
      return;
    }
    check_ced (ch);
    if (ch->ced->poll == NULL)
    {
      send_to_char ("You have to start a poll first.\n\r", ch);
      return;
    }
    if (count_lines (ch->ced->poll->text) < 1)
    {
      send_to_char ("Nothing to delete.\n\r", ch);
      return;
    }
    if (count_lines (ch->ced->poll->text) == 1)
    {
      free_string (ch->ced->poll->text);
      ch->ced->poll->text = strdup ("");
      send_to_char ("Last line deleted.\n\r", ch);
      return;
    }
    new_buf[0] = '\0';
    str = ch->ced->poll->text;
    cnt = 1;
    num = 1;
    while (*str != '\0' && num <= count_lines (ch->ced->poll->text))
    {
      if (*str == '\r')
        num++;
      cnt++;
      str++;
    }
    strncpy (new_buf, ch->ced->poll->text, cnt);
    new_buf[cnt] = '\0';
    free_string (ch->ced->poll->text);
    ch->ced->poll->text = str_dup (new_buf);
    send_to_char ("Last line deleted.\n\r", ch);
    return;
  }				// end -
  if (!str_cmp (arg, "subject"))
  {				// setup the poll subject
    char *c;
    if (!IS_IMMORTAL (ch))
    {
      send_to_char ("You are not allowed to set up a poll.\n\r", ch);
      return;
    }
    check_ced (ch);
    poll_attach (ch);
    for (c = argy; *c != '\0'; c++)
    {
      if (*c == '{' || *c == '}')
      {
        send_to_char ("No curly braces allowed.\n\r", ch);
        return;
      }
    }
    free_string (ch->ced->poll->subject);
    ch->ced->poll->subject = str_dup (argy);
    send_to_char ("Subject set.\n\r", ch);
    return;
  }				// end subject
  if (!str_cmp (arg, "to"))
  {				// setup the recipient list of the poll
    char *c;
    if (!IS_IMMORTAL (ch))
    {
      send_to_char ("You are not allowed to set up a poll.\n\r", ch);
      return;
    }
    check_ced (ch);
    poll_attach (ch);
    for (c = argy; *c != '\0'; c++)
    {
      if (*c == '{' || *c == '}')
      {
        send_to_char ("No curly braces allowed.\n\r", ch);
        return;
      }
    }
    free_string (ch->ced->poll->to_list);
    ch->ced->poll->to_list = str_dup (argy);
    send_to_char ("Recipients set.\n\r", ch);
    return;
  }				// end to
  if (!str_cmp (arg, "clear"))
  {				// clear the poll
    int opt;
    check_ced (ch);
    if (ch->ced->poll == NULL)
    {
      send_to_char ("You have no poll in progress.\n\r", ch);
      return;
    }
    free_string (ch->ced->poll->sender);
    free_string (ch->ced->poll->date);
    free_string (ch->ced->poll->to_list);
    free_string (ch->ced->poll->subject);
    free_string (ch->ced->poll->text);
    for (opt = 0; opt < MAX_OPTION; opt++)
      free_string (ch->ced->poll->options[opt]);
    ch->ced->poll->next = poll_free;
    poll_free == ch->ced->poll;
    ch->ced->poll = NULL;
    send_to_char ("Poll cleared.\n\r", ch);
    return;
  }				// end clear
  if (!str_cmp (arg, "show"))
  {				// show current poll info
    char buf[STD_LENGTH * 20];
    char sbuf[STD_LENGTH];
    int opt;
    check_ced (ch);
    if (ch->ced->poll == NULL)
    {
      send_to_char ("You have no poll in progress.\n\r", ch);
      return;
    }
    sprintf (buf, "From: %s  To: %s\n\rLevel: %d  Subject: %s\n\r",
        ch->ced->poll->sender,
        ch->ced->poll->to_list ? ch->ced->poll->to_list : "???",
        ch->ced->poll->level,
        ch->ced->poll->subject ? ch->ced->poll->subject : "???");
    strcat (buf, ch->ced->poll->text);
    strcat (buf, "\n\rOptions:\n\r");
    for (opt = 0; opt < MAX_OPTION; opt++)
      if (ch->ced->poll->options[opt])
      {
        sprintf (sbuf, "[\x1B[36;1m%c\x1B[37;0m] ", (char) (opt + 'A'));
        strcat (buf, sbuf);
        strcat (buf, ch->ced->poll->options[opt]);
        strcat (buf, "\n\r");
      }
    page_to_char (buf, ch);
    return;
  }				// end show
  if (!str_cmp (arg, "post") || !str_prefix (arg, "send"))
  {				// setup the poll
    FILE *fp;
    char strtime[200];
    POLL_DATA *poll;
    if (!IS_IMMORTAL (ch))
    {
      send_to_char ("You are not allowed to set up a poll.\n\r", ch);
      return;
    }
    check_ced (ch);
    poll = ch->ced->poll;
    if (poll == NULL)
    {
      send_to_char ("You have no poll in progress.\n\r", ch);
      return;
    }
    if (poll->to_list == NULL ||
        !str_cmp (poll->to_list, "") || poll->to_list[0] == '\0')
    {
      send_to_char
        ("You need to provide one or more recipients (names, all, or immortal).\n\r",
         ch);
      return;
    }
    if (poll->subject == NULL ||
        !str_cmp (poll->subject, "") || poll->subject[0] == '\0')
    {
      send_to_char ("You need to provide a subject.\n\r", ch);
      return;
    }
    if (poll->text == NULL ||
        !str_cmp (poll->text, "") ||
        poll->text[0] == '\0' || count_chars (poll->text) < 1)
    {
      send_to_char ("You should not post an empty poll.\n\r", ch);
      return;
    }
    if (count_options (poll) < 2)
    {
      send_to_char ("You should setup at least 2 options.\n\r", ch);
      return;
    }
    poll->next = NULL;
    strcpy (strtime, ctime (&current_time));
    strtime[strlen (strtime) - 1] = '\0';
    poll->date = str_dup (strtime);
    poll->date_stamp = current_time;
    if (poll_list == NULL)
    {
      poll_list = poll;
    }
    else
    {
      for (poll = poll_list; poll->next != NULL; poll = poll->next);
      poll->next = ch->ced->poll;
    }
    poll = ch->ced->poll;
    ch->ced->poll = NULL;
    if ((fp = fopen (POLL_FILE, "a")) == NULL)
      perror (POLL_FILE);
    else
    {
      int opt;
      fprintf (fp, "Sender %s~\n", fix_string (poll->sender));
      fprintf (fp, "Date %s~\n", poll->date);
      fprintf (fp, "Stamp %ld\n", poll->date_stamp);
      fprintf (fp, "To %s~\n", fix_string (poll->to_list));
      fprintf (fp, "Level %d\n", poll->level);
      fprintf (fp, "Subject %s~\n", fix_string (poll->subject));
      fprintf (fp, "Text\n%s~\n", poll->text);
      fprintf (fp, "Open %d\n", poll->open);
      fprintf (fp, "Public %d\n", poll->publicstats);
      for (opt = 0; opt < MAX_OPTION; opt++)
        if (poll->options[opt])
          fprintf (fp, "Option %d %s~\n", opt, poll->options[opt]);
      for (opt = 0; opt < MAX_OPTION; opt++)
        if (poll->options[opt])
          fprintf (fp, "Votes %d %d~\n", opt, poll->votes[opt]);
      fprintf (fp, "End\n\n");
      fclose (fp);
    }
    send_to_char ("Poll setup complete.\n\r", ch);
    notify_new_poll (ch, poll);
    return;
  }				// end post / send
  if (!str_cmp (arg, "remove"))
  {				// delete a poll
    POLL_DATA *poll;
    int number;
    int anum;
    if (!IS_IMMORTAL (ch))
    {
      send_to_char ("You are not allowed to remove polls.\n\r", ch);
      return;
    }
    if (!is_number (argy))
    {
      send_to_char ("Poll remove which number?\n\r", ch);
      return;
    }
    anum = atoi (argy);
    number = 0;
    for (poll = poll_list; poll != NULL; poll = poll->next)
    {
      if (number++ == anum)
      {
        poll_remove (ch, poll);
        send_to_char ("Poll removed.\n\r", ch);
        return;
      }
    }
    send_to_char ("No such poll.\n\r", ch);
    return;
  }				// end remove
  if (!str_cmp (arg, "open"))
  {				// open a (closed) poll
    POLL_DATA *poll;
    int pollno;
    int number = 0;
    bool fnd = FALSE;
    if (!IS_IMMORTAL (ch))
    {
      send_to_char ("You are not allowed to edit polls.\n\r", ch);
      return;
    }
    if (!argy || !argy[0])
    {
      send_to_char ("Open which poll?\n\r", ch);
      return;
    }
    pollno = atoi (argy);
    if (pollno < 0)
    {
      send_to_char ("Invalid poll number.\n\r", ch);
      return;
    }
    for (poll = poll_list; poll && !fnd; poll = poll->next)
    {
      if (number++ == pollno)
      {
        fnd = TRUE;
        break;
      }
    }
    if (!poll || !fnd)
    {
      send_to_char ("No such poll.\n\r", ch);
      return;
    }
    if (poll->open)
    {
      send_to_char ("Poll is already open.\n\r", ch);
      return;
    }
    poll->open = TRUE;
    send_to_char ("Poll opened.\n\r", ch);
    save_polls ();
    return;
  }
  if (!str_cmp (arg, "close"))
  {				// close an (open) poll
    POLL_DATA *poll;
    int pollno;
    int number = 0;
    bool fnd = FALSE;
    if (!IS_IMMORTAL (ch))
    {
      send_to_char ("You are not allowed to edit polls.\n\r", ch);
      return;
    }
    if (!argy || !argy[0])
    {
      send_to_char ("Open which poll?\n\r", ch);
      return;
    }
    pollno = atoi (argy);
    if (pollno < 0)
    {
      send_to_char ("Invalid poll number.\n\r", ch);
      return;
    }
    for (poll = poll_list; poll && !fnd; poll = poll->next)
    {
      if (number++ == pollno)
      {
        fnd = TRUE;
        break;
      }
    }
    if (!poll || !fnd)
    {
      send_to_char ("No such poll.\n\r", ch);
      return;
    }
    if (!poll->open)
    {
      send_to_char ("Poll is already closed.\n\r", ch);
      return;
    }
    poll->open = FALSE;
    send_to_char ("Poll closed.\n\r", ch);
    save_polls ();
    return;
  }
  if (!str_cmp (arg, "public"))
  {				// toggle whether the results are public or not
    POLL_DATA *poll;
    if (!IS_IMMORTAL (ch))
    {
      send_to_char ("You are not allowed to edit polls.\n\r", ch);
      return;
    }
    check_ced (ch);
    if (ch->ced->poll)
      poll = ch->ced->poll;
    else
    {
      int number = 0;
      int pollno = atoi (argy);
      bool fnd = FALSE;
      for (poll = poll_list; poll && !fnd; poll = poll->next)
      {
        if (number++ == pollno)
        {
          fnd = TRUE;
          break;
        }
      }
      if (!poll || !fnd)
      {
        send_to_char ("No such poll.\n\r", ch);
        return;
      }
    }
    if (poll->publicstats)
      send_to_char ("Poll results are no longer public now.\n\r", ch);
    else
      send_to_char ("Poll results are now public.\n\r", ch);
    poll->publicstats = !poll->publicstats;
    if (!ch->ced->poll)
      save_polls ();
    return;
  }
  if (!str_cmp (arg, "level"))
  {				// setup the poll level
    int level;
    if (!IS_IMMORTAL (ch))
    {
      send_to_char ("You are not allowed to set up polls.\n\r", ch);
      return;
    }
    check_ced (ch);
    poll_attach (ch);
    if (!argy || !argy[0] || !is_number (argy))
    {
      send_to_char ("Invalid level.\n\r", ch);
      return;
    }
    level = atoi (argy);
    if ((level < 0) || (level > MAX_LEVEL))
    {
      send_to_char ("Invalid level.\n\r", ch);
      return;
    }
    ch->ced->poll->level = level;
    send_to_char ("Level set.\n\r", ch);
    return;
  }
  if (!str_cmp (arg, "read"))
  {				// read poll info
    char buf[STD_LENGTH * 20];
    char sbuf[STD_LENGTH];
    int opt;
    POLL_DATA *poll;
    int pollno;
    int number = 0;
    bool fnd = FALSE;
    if (argy && argy[0])
      pollno = atoi (argy);
    else
      pollno = -1;
    for (poll = poll_list; poll && !fnd; poll = poll->next)
    {
      if ((number++ == pollno) && is_poll_to (ch, poll))
      {
        fnd = TRUE;
        break;
      }
      if (pollno == -1 && is_poll_to (ch, poll)
          && !has_read_poll (ch, poll))
      {
        fnd = TRUE;
        break;
      }
    }
    if (!poll || !fnd)
    {
      if (pollno == -1)
        send_to_char ("No unread polls.\n\r", ch);
      else
        send_to_char ("No such poll.\n\r", ch);
      return;
    }
    sprintf (buf,
        "\x1B[33;1m--------------------[ \x1B[37;1mPoll #%-3d  Status: %s \x1B[33;1m]--------------------\x1B[37;0m\n\r",
        number - 1,
        poll->open ? "\x1B[32;1mOpen  " : "\x1B[31;1mClosed");
    sprintf (sbuf, "From: %s  To: %s\n\rSubject: %s\n\r", poll->sender,
        poll->to_list ? poll->to_list : "",
        poll->subject ? poll->subject : "");
    strcat (buf, sbuf);
    strcat (buf, poll->text);
    strcat (buf, "\n\rOptions:\n\r");
    for (opt = 0; opt < MAX_OPTION; opt++)
      if (poll->options[opt])
      {
        sprintf (sbuf, "[\x1B[36;1m%c\x1B[37;0m] ", (char) (opt + 'A'));
        strcat (buf, sbuf);
        strcat (buf, poll->options[opt]);
        strcat (buf, "\n\r");
      }
    strcat (buf,
        "\x1B[33;1m---------------------------------------------------------------------\x1B[37;0m\n\r");
    if (IS_IMMORTAL (ch))
    {
      sprintf (sbuf, "Poll level: %d, ", poll->level);
      strcat (buf, sbuf);
      if (poll->publicstats)
        strcat (buf, "results are public.\n\r");
      else
        strcat (buf, "results are NOT public.\n\r");
    }
    page_to_char (buf, ch);
    return;
  }				// end read
  if (!str_cmp (arg, "results"))
  {				// see poll results
    char buf[STD_LENGTH];
    int totalvotes;
    POLL_DATA *poll;
    int pollno;
    int number = 0;
    int opt;
    bool fnd = FALSE;
    if (!argy || !argy[0])
    {
      send_to_char ("Show the results of what poll number?\n\r", ch);
      return;
    }
    pollno = atoi (argy);
    for (poll = poll_list; poll && !fnd; poll = poll->next)
    {
      if (number++ == pollno)
      {
        fnd = TRUE;
        break;
      }
    }
    if (!poll || !fnd)
    {
      send_to_char ("No such poll.\n\r", ch);
      return;
    }
    if (!is_poll_to (ch, poll))
    {
      send_to_char ("No such poll.\n\r", ch);
      return;
    }
    if (!IS_IMMORTAL (ch) && !has_read_poll (ch, poll))
    {
      send_to_char
        ("You can't see the results until you have voted for this poll.\n\r",
         ch);
      return;
    }
    sprintf (buf, "read %s", argy);
    do_poll (ch, buf);
    if (!IS_IMMORTAL (ch) && !poll->publicstats)
    {
      send_to_char
        ("Sorry, the results of that poll are not public yet.\n\r", ch);
      return;
    }
    totalvotes = 0;
    for (opt = 0; opt < MAX_OPTION; opt++)
      totalvotes += poll->votes[opt];
    if (totalvotes == 0)
      send_to_char ("Nobody voted yet for this poll.\n\r", ch);
    else
    {
      for (opt = 0; opt < MAX_OPTION; opt++)
      {
        if (poll->options[opt])
        {
          sprintf (buf,
              "[\x1B[36;1m%c\x1B[37;0m] %3d of %3d total vote%s (%2.1f%%)\n\r",
              (char) (opt + 'A'), poll->votes[opt], totalvotes,
              totalvotes == 1 ? "" : "s",
              (float) ((poll->votes[opt] * 1.0f) * 100.0f /
                       (totalvotes * 1.0f)));
          send_to_char (buf, ch);
        }
      }
      if (IS_IMMORTAL (ch))
        send_to_char
          ("Admin Info: Use 'poll details <nr>' for player names.\n\r",
           ch);
    }
    if (poll->open)
      send_to_char ("Note: This poll is still open for votes.\n\r", ch);
    send_to_char
      ("\x1B[33;1m---------------------------------------------------------------------\x1B[37;0m\n\r",
       ch);
    return;
  }				// end results
  if (!str_cmp (arg, "details"))
  {
    POLL_DATA *poll;
    int pollno;
    int number = 0;
    bool fnd = FALSE;
    if (!IS_IMMORTAL(ch))
    {
      send_to_char("Huh?\n\r", ch);
      return;
    }
    if (!argy || !argy[0])
    {
      send_to_char ("Show the results of what poll number?\n\r", ch);
      return;
    }
    pollno = atoi (argy);
    for (poll = poll_list; poll && !fnd; poll = poll->next)
    {
      if (number++ == pollno)
      {
        fnd = TRUE;
        break;
      }
    }
    if (!poll || !fnd)
    {
      send_to_char ("No such poll.\n\r", ch);
      return;
    }
    send_to_char ("Poll details:\n\r", ch);
    poll_details (ch, poll->date_stamp);
    return;
  }
  if (is_number (arg))
  {
    char sbuf[STD_LENGTH];
    sprintf (sbuf, "read %s", arg);
    do_poll (ch, sbuf);
    return;
  }
  send_to_char ("Huh? Type 'help poll' for usage.\n\r", ch);
  return;
}

bool
poll_stamp_exists (time_t stamp)
{
  POLL_DATA *poll;
  for (poll = poll_list; poll; poll = poll->next)
  {
    if (poll->date_stamp == stamp)
      return TRUE;
  }
  return FALSE;
}

void
save_read_polls (CHAR_DATA * ch)
{
  POLL_STAMP_LIST *list;
  FILE *fp;
  char pollfile[100];
  if (!ch || IS_MOB (ch))
    return;
  if (LEVEL (ch) < command_level ("poll"))
    return;
  sprintf (pollfile, "%s%s.poll", PLAYER_DIR, cap (NAME (ch)));
  if ((fp = fopen (pollfile, "w")) == NULL)
    perror (pollfile);
  else
  {
    for (list = ch->pcdata->read_polls; list; list = list->next)
    {
      fprintf (fp, "%ld %d\n", list->stamp, list->choice);
    }
    fprintf (fp, "End\n");
    fclose (fp);
  }
  return;
}

void
read_read_polls (CHAR_DATA * ch)
{
  FILE *fp;
  char pollfile[100];
  char *word;
  char *choice;
  if (!ch || IS_MOB (ch))
    return;
  if (LEVEL (ch) < command_level ("poll"))
    return;
  sprintf (pollfile, "%s%s.poll", PLAYER_DIR, cap (NAME (ch)));
  if ((fp = fopen (pollfile, "r")) == NULL);
  //perror (pollfile);
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
      if (feof (fp))
        break;
      choice = fread_word (fp);
      if (!str_cmp (word, "End"))
        break;
      if (!is_number (choice))
        break;
      if (poll_stamp_exists (atoi (word)))
        add_read_poll_stamp (ch, atoi (word), atoi (choice));
    }
    fclose (fp);
  }
  return;
}

void
add_read_poll_stamp (CHAR_DATA * ch, time_t stamp, short choice)
{
  POLL_STAMP_LIST *list;
  if (!ch || IS_MOB (ch))
    return;
  list = (POLL_STAMP_LIST *) malloc (sizeof (POLL_STAMP_LIST));
  list->stamp = stamp;
  list->choice = choice;
  list->next = ch->pcdata->read_polls;
  ch->pcdata->read_polls = list;
  return;
}

void
free_read_polls (CHAR_DATA * ch)
{
  POLL_STAMP_LIST *list;
  POLL_STAMP_LIST *list_next;
  if (!ch || IS_MOB (ch))
    return;
  list = ch->pcdata->read_polls;
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
  ch->pcdata->read_polls = NULL;
  return;
}

POLL_STAMP_LIST *
has_read_poll (CHAR_DATA * ch, POLL_DATA * poll)
{
  POLL_STAMP_LIST *list;
  for (list = ch->pcdata->read_polls; list; list = list->next)
  {
    if (list->stamp == poll->date_stamp)
      return list;
  }
  return NULL;
}

int
count_options (POLL_DATA * poll)
{
  int res = 0;
  int opt;
  for (opt = 0; opt < MAX_OPTION; opt++)
    if (poll->options[opt] && *(poll->options[opt]))
      res++;
  return res;
}

void
notify_new_poll (CHAR_DATA * ch, POLL_DATA * poll)
{
  CHAR_DATA *toch;
  char buf[STD_LENGTH];
  for (toch = char_list; toch != NULL; toch = toch->next)
  {
    if (!IS_PLAYER (toch))
      continue;
    if (toch != ch && is_poll_to (toch, poll) && !IS_IMMORTAL (toch))
    {
      sprintf (buf, "There is a new poll!\n\r");
      send_to_char (buf, toch);
    }
    else if (toch != ch && is_poll_to (toch, poll) && IS_IMMORTAL (toch))
    {
      sprintf (buf, "%s just set up a new poll!\n\r", NAME (ch));
      send_to_char (buf, toch);
    }
  }
  return;
}
