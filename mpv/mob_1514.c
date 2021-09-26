#include "mob.h"

#define HAS_TALKED_TO_WIZARD      14
#define GAVE_RING                 15
#define GNOME_WIZARD_RING_QUEST   16

ATTACH_TO_MOB (mob_1514_1, 1514, ENTERS_ROOM, unused)
ATTACH_TO_MOB (mob_1514_2, 1514, GIVEN_OBJECT, vnum) MOB_FUNCTION (mob_1514_1)
{
  INTEGER (tempint_1);
  CHECK_INTEGRITY (ch, mob_1514_1);
  ALLOW_INTERRUPTIONS;

  MARK_CHUNK (1)
  {
    DELAY_SCRIPT (1);
  }
  MARK_CHUNK (2)
  {
    if (FLAG (GNOME_WIZARD_RING_QUEST))
      {
	char temp[200];
	sprintf (temp, "Hello again, %s.", NAME (ch));
	SAY (mob, temp);
	END_FUNCTION;
      }
    if (FLAG (GAVE_RING))
      {
	SAY (mob, "Fool!  Where is my ring?");
	END_FUNCTION;
      }
    if (FLAG (HAS_TALKED_TO_WIZARD))
      {
	SAY (mob, "Fool!  Go get me my ring... NOW!!");
	tempint_1 = 1;
	DELAY_SCRIPT ((5 * ONE_SECOND));
      }
    SAY (mob,
	 "I need you to do a favor for me... There is a ring which possesses great power!  I MUST have it!!");
    DELAY_SCRIPT (ONE_SECOND);
  }

  MARK_CHUNK (3)
  {
    if (tempint_1 == 1)
      {
	tempint_1 = 0;
	START_FIGHTING (mob, ch);
	END_FUNCTION;
      }
    SAY (mob,
	 "You must fetch it from the wretched human slave who stole it!  You may have to slap him around a bit...  You must not wear the ring; it possesses powers beyond your comprehension!");
    DELAY_SCRIPT (ONE_SECOND);
  }

  MARK_CHUNK (4)
  {
    char temp[200];
    sprintf (temp, "I want YOU to bring it to me, %s!", NAME (ch));
    SAY (mob, temp);
    FLAG (HAS_TALKED_TO_WIZARD) = TRUE;
    DELAY_SCRIPT ((2 * ONE_SECOND));
  }

  MARK_CHUNK (5)
  {
    SAY (mob, "Now, begone!!");
  }

  END_FUNCTION;
}

MOB_FUNCTION (mob_1514_2)
{
  INTEGER (i);
  CHECK_INTEGRITY (ch, mob_1514_2);
  DONT_ALLOW_INTERRUPTIONS;

  i = NUM (argy);

  if (i != 41)
    {
      END_FUNCTION;
    }

  MARK_CHUNK (1)
  {
    if (!FLAG (GAVE_RING))
      {
	SAY (mob, "Incompetant fool!  Stealing my ring like that.....!");
	START_FIGHTING (mob, ch);
	END_FUNCTION;
      }
    DESTROY (mob, i);
    DELAY_SCRIPT (ONE_SECOND);
  }

  MARK_CHUNK (2)
  {
    SAY (mob, "You were wise to do my bidding, and I shall thus reward you!");
    FLAG (HAS_TALKED_TO_WIZARD) = FALSE;
    FLAG (GAVE_RING) = FALSE;
    FLAG (GNOME_WIZARD_RING_QUEST) = TRUE;
    AWARDXP (ch, 70000);
    COINS (mob, 500);
    GIVECOINS (mob, ch, "5 gold");
    AWARDSKILL (ch, "Grapitr", 27);
    DELAY_SCRIPT (ONE_SECOND);
  }

  MARK_CHUNK (3)
  {
    SAY (mob, "Now, begone, and let me get some work done!!");
    SHOW_TO_ROOM ("The ring vanishes before your very eyes!", mob);
  }

  END_FUNCTION;
}
