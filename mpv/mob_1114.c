#include "mob.h"

ATTACH_TO_MOB (mob_1114_1, 1114, ENTERS_ROOM, unused)
ATTACH_TO_MOB (mob_1114_2, 1114, GIVEN_OBJECT, vnum) MOB_FUNCTION (mob_1114_1)
{
  CHECK_INTEGRITY (ch, mob_1114_1);
  ALLOW_INTERRUPTIONS;

  MARK_CHUNK (1)
  {
    DELAY_SCRIPT (1);
  }
  MARK_CHUNK (2)
  {
    if (RAND (1, 2) == 1)
      {
	END_FUNCTION;
      }
    TELL (mob, ch,
	  "Hey buddy!  Go get me a seashell; I need more for my collection.");
    DELAY_SCRIPT (1);
  }
  MARK_CHUNK (3)
  {
    SOCIAL_TO (mob, ch, "wink");
  }

  END_FUNCTION;
}

MOB_FUNCTION (mob_1114_2)
{
  INTEGER (i);
  CHECK_INTEGRITY (ch, mob_1114_2);
  DONT_ALLOW_INTERRUPTIONS;

  i = NUM (argy);

  MARK_CHUNK (1)
  {
    DELAY_SCRIPT (1);
  }

  MARK_CHUNK (2)
  {
    if (i != 257)
      {
	SAY (mob, "Well, thank you!");
	END_FUNCTION;
      }
  }

  MARK_CHUNK (3)
  {
    char temps[200];
    sprintf (temps, "Hey!  Thank you, %s!", NAME (ch));
    SAY (mob, temps);
    DESTROY (mob, i);
    CREATEOBJ (mob, 97);
    DELAY_SCRIPT (2);
  }

  MARK_CHUNK (4)
  {
    SAY (mob, "Here is one of my finest vessels, as a reward!");
    GIVEITEM (mob, ch, "boat");
    DELAY_SCRIPT (1);
  }

  MARK_CHUNK (5)
  {
    SOCIAL_TO (mob, ch, "bow");
  }

  END_FUNCTION;
}
