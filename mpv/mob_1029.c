#include "mob.h"

#define DIMPLEFLAG_ONE 17
#define GOT_DIMPLE_ANSWER 18
#define SOLVED_EXISTENCE 19

ATTACH_TO_MOB (mob_1029_1, 101, ENTERS, unused)
ATTACH_TO_MOB (mob_1029_2, 101, SAY / TELL, unused) MOB_FUNCTION (mob_1029_1)
{
  CHECK_INTEGRITY (ch, mob_1029_1);
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
    SAY (mob, "Hello there...");
    DELAY_SCRIPT (1);
  }
  MARK_CHUNK (4)
  {
    SOCIAL (mob, "emote goes back into his deep, meditative trance.");
  }
  END_FUNCTION;
}

MOB_FUNCTION (mob_1029_2)
{
  CHECK_INTEGRITY (ch, mob_1029_2);
  DONT_ALLOW_INTERRUPTIONS;

  MARK_CHUNK (1)
  {
    DELAY_SCRIPT (1);
  }

  MARK_CHUNK (2)
  {
    if (CONTAINS ("why do I exist", argy) || CONTAINS ("dimples exist", argy)
	|| CONTAINS ("existence", argy) || CONTAINS ("existance", argy)
	|| CONTAINS ("why do we exist", argy))
      {
	if (FLAG (DIMPLEFLAG_ONE))
	  {
	    SOCIAL (mob,
		    "emote stops meditating and looks you over thoroughly.");
	    DELAY_SCRIPT (ONE_SECOND);
	  }
      }
    END_FUNCTION;
  }

  MARK_CHUNK (3)
  {
    SAY (mob,
	 "There is no answer.  Finding the answer to that question would in itself prove our nonexistence.");
    DELAY_SCRIPT (1);
  }

  MARK_CHUNK (4)
  {
    SAY (mob,
	 "The idea of existence was created by our own mind, thus it cannot be proved, only contemplated.  There is no answer.");
    FLAG (GOT_DIMPLE_ANSWER) = TRUE;
    DELAY_SCRIPT (2);
  }

  MARK_CHUNK (5)
  {
    SOCIAL (mob, "emote goes back into a deep, meditative trance.");
  }

  END_FUNCTION;
}
