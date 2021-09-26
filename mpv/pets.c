#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"

void
place_pets (CHAR_DATA * ch)
{
  int k;
  MOB_PROTOTYPE *mid;
  CHAR_DATA *mob;
  if (IS_MOB (ch))
    return;
  for (k = 0; k < 20; k++)
    {
      if (ch->pcdata->pet_temp[k] != 0)
	{
	  char buf[1000];
	  if ((mid = get_mob_index (ch->pcdata->pet_temp[k])) == NULL)
	    {
	      ch->pcdata->pet_temp[k] = 0;
	      continue;
	    }
	  mob = create_mobile (mid);
	  char_to_room (mob, ch->in_room);
	  ch->pcdata->number_pets++;
	  mob->hit = ch->pcdata->pet_hps[k];
	  mob->move = ch->pcdata->pet_move[k];
	  SET_BIT (mob->affected_by, AFF_CHARM);
	  SET_BIT (mob->act, ACT_PET);
	  add_follower (mob, ch);
	}
    }
  return;
}
void do_order (CHAR_DATA* ch, char* argy)
{
        char victim[SML_LENGTH];
        char command[SML_LENGTH];
        char target[SML_LENGTH];

        CHAR_DATA *mobile;

        DEFINE_COMMAND ("order", do_order, POSITION_DEAD, 0, LOG_NORMAL, "Orders a servant mob to do something.")

        argy = one_argy (argy, victim);
        one_argy (argy, command);
        one_argy (argy, target);



        if (victim[0] == '\0' || argy[0] == '\0') {
          send_to_char ("\x1B[0;37mOrder who to do what?\n\n\r", ch);
          send_to_char ("\x1B[0;37mSyntax: order <undead mob> <option> <target>\n\r", ch);
          send_to_char ("\x1B[0;37mkill\n\r", ch);
          send_to_char ("\x1B[0;37mk - short for kill\n\r", ch);
          send_to_char ("\x1B[0;37mkick\n\r", ch);
          send_to_char ("\x1B[0;37mrescue\n\r", ch);
          send_to_char ("\x1B[0;37massist\n\r", ch);
          send_to_char ("\x1B[0;37mgive\n\r", ch);
          send_to_char ("\x1B[0;37mwear\n\r", ch);
          send_to_char ("\x1B[0;37mremove\n\r", ch);
          send_to_char ("\x1B[0;37mresize\n\r", ch);
          send_to_char ("\x1B[0;37mrepair\n\r", ch);
          send_to_char ("\x1B[0;37mflee\n\r", ch);
          send_to_char ("\x1B[0;37mdrop\n\r", ch);
          send_to_char ("\x1B[0;37mbash\n\r", ch);
          return;
        }
        if ((mobile = get_char_world_2 (ch, victim)) == NULL) {
          send_to_char ("They aren't here.\n\r", ch);
          return;
        }
        if (!(IS_AFFECTED(mobile, AFF_CHARM) && mobile->fgt->master == ch) ) {
          send_to_char ("Yeah right!\n\r", ch);
          return;
        }
        if(!str_prefix (command, "kill") ||
           !str_prefix (command, "k") ||
           !str_prefix (command, "kick") ||
           !str_prefix (command, "rescue") ||
           !str_prefix (command, "assist") ||
           !str_prefix (command, "give") ||
           !str_prefix (command, "get") ||
           !str_prefix (command, "wear") ||
           !str_prefix (command, "remove") ||
           !str_prefix (command, "resize") ||
           !str_prefix (command, "repair") ||
           !str_prefix (command, "flee") ||
           !str_prefix (command, "drop") ||
           !str_prefix (command, "bash")) {
          send_to_char ("Ok.\n\r", ch);
          interpret (mobile, argy);
        } else send_to_char("Your minion doesn't seem to understand.\n\r", ch);

  return;
}
