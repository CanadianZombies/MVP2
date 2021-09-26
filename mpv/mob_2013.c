#include "mob.h"

#define HAS_TALKED_TO_WIZARD      14
#define GAVE_RING                 15
#define GNOME_WIZARD_RING_QUEST   16

ATTACH_TO_MOB (mob_2013_1, 2013, SAYS, ANYTHING)
ATTACH_TO_MOB (mob_2013_2, 2013, GIVEN_OBJECT, vnum) MOB_FUNCTION (mob_2013_1)
{
  CHECK_INTEGRITY (ch, mob_2013_1);
  DONT_ALLOW_INTERRUPTIONS;

  MARK_CHUNK (1)
  {
    DELAY_SCRIPT (1);
  }

  MARK_CHUNK (2)
  {
    SAY (mob, "I didn't do it!  It wasn't me!  You got the wrong person!!");
  }

  END_FUNCTION;
}

MOB_FUNCTION (mob_2013_2)
{
  INTEGER (given_ring_this_reboot);
  CHECK_INTEGRITY (ch, mob_2013_2);
  DONT_ALLOW_INTERRUPTIONS;

  if (given_ring_this_reboot)
    {
      END_FUNCTION;
    }

  if (!(CONTAINS ("slap", argy) || CONTAINS ("punch", argy)))
    {
      END_FUNCTION;
    }

  MARK_CHUNK (1)
  {
    DELAY_SCRIPT (2);
  }

  MARK_CHUNK (2)
  {
    if (FLAG (GAVE_RING))
      {
	SAY (mob, "Stop that!!  I want my ring back!!");
	START_FIGHTING (mob, ch);
	END_FUNCTION;
      }
    if (FLAG (HAS_TALKED_TO_WIZARD))
      {
	SAY (mob,
	     "Okay, Okay, stop it!  I will give you the ring, but you don't realize the power it contains!");
	DELAY_SCRIPT (1);
      }
    END_FUNCTION;
  }

  MARK_CHUNK (3)
  {
    SAY (mob, "If you wear it, you will become a GOD!!  I know it!...");
    SOCIAL (mob, "insane");
    DELAY_SCRIPT (2);
  }

  MARK_CHUNK (4)
  {
    SAY (mob, "Wait!  You can't look at my secret hiding place...");
    DELAY_SCRIPT (2);
  }

  MARK_CHUNK (5)
  {
    SOCIAL (mob,
	    "emote reaches to his backside and extracts a dirty, brown hoop of metal from his rather disgusting bodily oriface.");
    DELAY_SCRIPT (2);
  }

  MARK_CHUNK (6)
  {
    CREATEOBJ (mob, 41);
    GIVEITEM (mob, ch, "ring");
    SOCIAL (mob, "grin");
    FLAG (GAVE_RING) = TRUE;
    given_ring_this_reboot = TRUE;
  }

  END_FUNCTION;
}
