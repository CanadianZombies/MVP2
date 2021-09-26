#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"

void
draw_menu (CHAR_DATA * ch)
{
  send_to_char ("\x1B[2J\x1B[1;8r\x1B[1;1f", ch);
  send_to_char ("1. Start world save            2. Clear all tps\n\r", ch);
  send_to_char
    ("3. Reload Helpfiles            4. Reload Score/Info Data\n\r", ch);
  send_to_char
    ("5. Start 5 tick reboot         6. Save all players online\n\r", ch);
  send_to_char
    ("7. Delete inactive characters  8. Set all last login to now\n\r", ch);
  send_to_char ("9. Write players.txt to disk\n\r", ch);
  return;
}

void
draw_mp (CHAR_DATA * ch)
{
  send_to_char ("\x1B[20;1f(Q = quit menu) Option: ", ch);
  return;
}

void
enter_menu (CHAR_DATA * ch, char *argy)
{
  DEFINE_COMMAND ("menu", enter_menu, POSITION_DEAD, 110, LOG_NORMAL,
		  "Menu of a few god-options.") if (ch->desc)
    ch->desc->connected = CON_GODMENU;
  draw_menu (ch);
  draw_mp (ch);
  return;
}

void
menu_interp (CHAR_DATA * ch, char *inpt)
{
  if (*inpt == '1')
    {
      interpret (ch, "asave world");
      draw_menu (ch);
      send_to_char ("\x1B[19;1f[World save started]", ch);
      draw_mp (ch);
    }
  else if (*inpt == '2')
    {
      do_catps (ch, "");
      draw_menu (ch);
      send_to_char ("\x1B[19;1f[All travel points cleared]", ch);
      draw_mp (ch);
    }
  else if (*inpt == '3')
    {
      do_reload_help (ch, "");
      draw_menu (ch);
      send_to_char ("\x1B[19;1f[Help files reloaded]", ch);
      draw_mp (ch);
    }
  else if (*inpt == '4')
    {
      read_score (ch, "");
      draw_menu (ch);
      send_to_char ("\x1B[19;1f[Score/Info data reloaded]", ch);
      draw_mp (ch);
    }
  else if (*inpt == '5')
    {
      interpret (ch, "reboot 6");
      draw_menu (ch);
      send_to_char ("\x1B[19;1f[Reboot set to 5 ticks]", ch);
      draw_mp (ch);
    }
  else if (*inpt == '6')
    {
      do_allsave (ch, "");
      draw_menu (ch);
      send_to_char
	("\x1B[19;1f[Everyone Online's Pfile has been saved to disk]", ch);
      draw_mp (ch);
    }
  else if (*inpt == '7')
    {
      write_to_descriptor2 (ch->desc,
			    "\n\rPlease wait... deleting old chars...", 0);
      clean_oldchars (ch, "");
      draw_menu (ch);
      send_to_char ("\x1B[19;1f[Deleted old characters]", ch);
      draw_mp (ch);
    }
  else if (*inpt == '8')
    {
      p_refresh (ch, "");
      draw_menu (ch);
      send_to_char
	("\x1B[19;1f[All players in playerbase; set last login to now]", ch);
      draw_mp (ch);
    }
  else if (UPPER ((*inpt)) == 'Q')
    {
      do_clear (ch, "");
      if (ch->desc)
	ch->desc->connected = CON_PLAYING;
      return;
    }
  else
    {
      draw_menu (ch);
      draw_mp (ch);
    }
  return;
}
