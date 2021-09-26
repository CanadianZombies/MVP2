#include "mob.h"

#define DIMPLEFLAG_ONE 17
#define GOT_DIMPLE_ANSWER 18
#define SOLVED_EXISTENCE 19

ATTACH_TO_MOB (mob_101_1, 101, ENTERS, unused)
ATTACH_TO_MOB (mob_101_2, 101, SAY / TELL, unused) MOB_FUNCTION (mob_101_1)
{
  CHECK_INTEGRITY (ch, mob_101_1);
  ALLOW_INTERRUPTIONS;

  MARK_CHUNK (1)
  {
    DELAY_SCRIPT (1);
  }
  MARK_CHUNK (2)
  {
    SOCIAL (mob, "emote stands up.");
    DELAY_SCRIPT (1);
  }
  MARK_CHUNK (3)
  {
    SOCIAL (mob, "curtsey");
    DELAY_SCRIPT (1);
  }
  MARK_CHUNK (4)
  {
    SOCIAL (mob, "emote sits down.");
  }
  END_FUNCTION;
}

MOB_FUNCTION (mob_101_2)
{
  CHECK_INTEGRITY (ch, mob_101_2);
  DONT_ALLOW_INTERRUPTIONS;

  MARK_CHUNK (1)
  {
    DELAY_SCRIPT (1);
  }

  MARK_CHUNK (2)
  {
    if (CONTAINS ("hello", argy) || CONTAINS ("greetings", argy))
      {
	SAY (mob, "Uhhh... hello there.");
	END_FUNCTION;
      }
    if (CONTAINS ("who are", argy))
      {
	if (FLAG (SOLVED_EXISTENCE))
	  {
	    SAY (mob, "I am Dimples.");
	    END_FUNCTION;
	  }
	else
	  {
	    SAY (mob,
		 "I would tell you if I knew.  I cannot prove my existence.  Perhaps you could ask someone wiser than me why I exist, and relay the answer back to me...");
	    FLAG (DIMPLEFLAG_ONE) = TRUE;
	    END_FUNCTION;
	  }
      }
    if (CONTAINS ("there is no answer", argy))
      {
	if (FLAG (SOLVED_EXISTENCE))
	  {
	    END_FUNCTION;
	  }
	if (!FLAG (GOT_DIMPLE_ANSWER))
	  {
	    SAY (mob, "Bah, how do you know that... I don't believe you.");
	    END_FUNCTION;
	  }
	SAY (mob,
	     "Eh?  Well that's something to think about... I can tell you have discussed it with the great Druid.");
	DELAY_SCRIPT (ONE_SECOND);
      }
    END_FUNCTION;
  }

  MARK_CHUNK (3)
  {
    SAY (mob,
	 "I appreciate your effort, even though it does not help me in my quest.");
    AWARDXP (ch, 40000);
    AWARDTP (ch, 35);
    COINS (mob, 500);
    GIVECOINS (mob, ch, "5 gold");
    FLAG (SOLVED_EXISTENCE) = TRUE;
    FLAG (GOT_DIMPLE_ANSWER) = FALSE;
    FLAG (DIMPLEFLAG_ONE) = FALSE;
    DELAY_SCRIPT ((2 * ONE_SECOND));
  }

  MARK_CHUNK (4)
  {
    SOCIAL (mob,
	    "emote sits down and begins to draw funny symbols in the air once again.");
  }

  END_FUNCTION;
}
