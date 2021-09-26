#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"

static bool used_spell[SKILL_COUNT];
static bool vert_line[100];

void do_new_prereq (CHAR_DATA *ch, char *argy)
{
  SPELL_DATA *spell;
  int i;

  DEFINE_COMMAND ("zprerequisite", do_new_prereq, POSITION_DEAD, 0, LOG_NORMAL, "Allows you to see prerequisites to the skill-big tree format.")

    if (IS_MOB (ch))
      return;
  for (i = 0; i < 100; i ++)
    vert_line[i] = FALSE;
   for (i = 0; i < SKILL_COUNT; i++)
    used_spell[i] = FALSE;


  if (argy[0] == '\0' || (spell = skill_lookup (argy, -1)) == NULL)
    {
      send_to_char ("Syntax: Prereq <skill/spell/prof name>\n\r", ch);
      return;
    }
  xprereq(ch, spell);


  /* Now clean up our static bool */

  return;
}
void xprereq (CHAR_DATA *ch, SPELL_DATA *spell)
{
  static int deppy = 0;
  int jj;
  int diff = 0;
  int len = 3*deppy+3;
  char buf[STD_LENGTH];
  char buff[100];
  if (used_spell[spell->gsn] == TRUE)
    return;
  sprintf(buf, " ");
  if(deppy > 0)
    {
      for (jj = 0; jj < deppy; jj++)
        {
          if (!vert_line[jj])
            strcat(buf, "...");
          else
            strcat(buf, "|..");
        }
    }
  strcat(buf, "\\__ ");
  strcat(buf, spell->spell_name);
  len += strlen(spell->spell_name);
  diff = 70-len;
  if (diff > 0)
    {
      for (jj = 0; jj < diff; jj++)
        {
          strcat(buf, ".");
        }
    }
  sprintf(buff, "[%d]\n\r", spell->num_prereqs);
  strcat(buf, buff);
  send_to_char(buf, ch);
  deppy++;
  used_spell[spell->gsn] = TRUE;
  vert_line[deppy] = FALSE;
  if (spell->pre1 != NULL && spell->pre2 != NULL && !used_spell[spell->pre1->gsn] && !used_spell[spell->pre2->gsn])
    {
      vert_line[deppy] = spell->go_to_second;
    }

  if (spell->pre1 != NULL )
    xprereq(ch, spell->pre1);
  vert_line[deppy] = FALSE;
  if (spell->pre2 != NULL)
    xprereq(ch, spell->pre2);
  deppy--;
  return;
}

bool
check_prereq_depth(SPELL_DATA *spell, SPELL_DATA *prereq)
{
  static int depthy = 0;
  if (depthy > 13) return TRUE;
  if (prereq == spell->pre1 || (prereq == spell->pre2 && depthy > 0))
    return FALSE;
  depthy++;
  if (spell->pre1 != NULL && !check_prereq_depth(spell->pre1, prereq))
    {
      depthy--;
      return FALSE;
    }
  if (spell->pre2 != NULL && !check_prereq_depth(spell->pre2, prereq))
    {
      depthy--;
      return FALSE;
    }
  depthy--;

  return TRUE;
}

char *
how_good (int percent)
{
  static char bf[256];
  int i;
  bf[0] = '\0';
  for (i = 0; i < 20; i++)
    {
      if (pow.learned[i] != NULL && percent >= pow.low_range[i]
	  && percent <= pow.high_range[i])
	{
	  strcpy (bf, pow.learned[i]);
	  return (bf);
	}
    }
  return (bf);
}

void
do_weapon (CHAR_DATA * ch, char *argy)
{
  char buf[500];
  DEFINE_COMMAND ("weapon", do_weapon, POSITION_STANDING, 0, LOG_NORMAL, "This command shows you how good you are at what types of weapons.")
    send_to_char ("Will fix it later, Palio :)\n\r", ch);
  if (IS_MOB (ch))
    return;
  send_to_char ("\x1B[34;1m------------------------------ [Weapon Skills] ------------------------------\n\r", ch);
  sprintf (buf, "\x1B[37;1m  Slashing Weapons.... \x1B[32;1m%-12s\x1B[37;1m  Piercing Weapons.. \x1B[32;1m%-12s\n\r",
	   how_good (ch->pcdata->learned[gsn_slashing]),
	   how_good (ch->pcdata->learned[gsn_pierce]));
  send_to_char (buf, ch);
  sprintf (buf, "\x1B[37;1m  Concussion Weapons.. \x1B[32;1m%-12s\x1B[37;1m  Whipping Weapons.. \x1B[32;1m%-12s\x1B[37;0m\n\r",
	   how_good (ch->pcdata->learned[gsn_concussion]),
	   how_good (ch->pcdata->learned[gsn_whip]));
  send_to_char (buf, ch);
  if (ch->pcdata->learned[gsn_lance] > 0)
    {
      sprintf (buf, "\x1B[37;1m  Lance Weapons.... \x1B[32;1m%-12s\x1B[37;0m\n\r",
	       how_good (ch->pcdata->learned[gsn_lance]));
      send_to_char (buf, ch);
    }
  send_to_char ("\x1B[34;1m-----------------------------------------------------------------------------\x1B[37;0m\n\r", ch);
  return;
}

bool
has_scriptflag (CHAR_DATA * ch,char * flag)
{
  int i;
  if (!ch || !ch->pcdata || !flag || !flag[0])
    return TRUE; 
  if (!str_cmp(flag, "(null)")) // lame fix because of bad coding,
    return TRUE;                // Eraser 12 Feb 2003
  for (i = 0; i < 3000; i++)
  {
    if (!str_cmp (ch->pcdata->script_flags[i], flag))
      return TRUE;
  }
  return FALSE;
}
								
void
do_learn (CHAR_DATA * ch, char *argy)
{
  char buf[STD_LENGTH];
  int sn;
  int skl, spl;
  SPELL_DATA *spell;
  int cntt;
  CHAR_DATA *mob;
  DEFINE_COMMAND ("learn", do_learn, POSITION_STANDING, 0, LOG_NORMAL, "This command allows you to learn a skill or spell.  You must be at a practitioner.")
  cntt = 0;
  if (IS_MOB (ch))
    return;
  if (pow.professions && ch->pcdata->profession_flag == 0)
  {
    send_to_char
      ("You should look into choosing a profession.  Type 'help profession' for info.\n\r",
       ch);
  }
  if (!IS_AWAKE (ch))
  {
    send_to_char ("In your dreams, or what?\n\r", ch);
    return;
  }

  for (mob = ch->in_room->more->people; mob != NULL; mob = mob->next_in_room)
  {
    if (IS_MOB (mob) && IS_SET (mob->act, ACT_PRACTICE) && mob->pIndexData->opt)
      break;
  }

  if (mob == NULL || !IS_SET (mob->act, ACT_PRACTICE) || IS_PLAYER (mob))
  {
    send_to_char ("You can't learn anything here!\n\r", ch);
    return;
  }

  skl = tally_skills (ch);
  spl = tally_spells (ch);
  if (mob != NULL && (IS_MOB (mob)) && argy[0] == '\0')
  {
    int col;
    send_to_char
      ("\x1B[37;1mYou may learn these skills (that you do not already know):\x1B[37;0m\n\r",
       ch);
    send_to_char
      ("\x1B[1;34m--------------------------------------------------------------------\x1B[37;0m\n\r",
       ch);
    col = 0;
    for (sn = 0; sn < 28; sn++)
    {
      if (mob->pIndexData->opt->skltaught[sn] != 0 &&
          ch->pcdata->learned[mob->pIndexData->opt->skltaught[sn]] == -100
          && (spell = skill_lookup (NULL, mob->pIndexData->opt->skltaught[sn])) != NULL
          && LEVEL (ch) >= spell->spell_level)
      {
        if (!(CAN_LEARN (ch, spell)))
          continue;
	{
		bool needFlag = FALSE;
		bool hasFlag = FALSE;
		
	if (spell->guildflag1)
		needFlag = TRUE;
        if (spell->guildflag2)
                needFlag = TRUE;
        if (spell->guildflag3)
                needFlag = TRUE;
        if (spell->guildflag4)
                needFlag = TRUE;
	if (has_scriptflag(ch, spell->guildflag1))
		hasFlag = TRUE;
        if (has_scriptflag(ch, spell->guildflag2))
                hasFlag = TRUE;
        if (has_scriptflag(ch, spell->guildflag3))
                hasFlag = TRUE;
        if (has_scriptflag(ch, spell->guildflag4))
                hasFlag = TRUE;
//copy of how this used to be below in do_learn and other code
        if (needFlag && !hasFlag)
		continue;	
        }
      	sprintf (buf, "[Level \x1B[37;1m%-2d\x1B[37;0m] \x1B[32;1m%18s\x1B[37;0m     ", spell->spell_level, spell->spell_funky_name);
        send_to_char (buf, ch);
        if (++col % 2 == 0)
          send_to_char ("\n\r", ch);
      }
    }
    sprintf (buf,
        "\n\r\n\rYou have \x1B[37;1m%d\x1B[37;0m learning sessions left.\n\rYou may learn \x1B[33;1m%d\x1B[37;0m and more skills and \x1B[33;1m%d\x1B[37;0m more spells in your lifetime.\n\r",
        ch->pcdata->learn,
        race_info[ch->pcdata->race].max_skills +
        ch->pcdata->remort_times - skl,
        race_info[ch->pcdata->race].max_spells - spl +
        (ch->pcdata->remort_times * 2));
    send_to_char (buf, ch);
    return;
  }

  if (ch->pcdata->learn <= 0)
  {
    send_to_char ("You have no learning sessions left.\n\r", ch);
    return;
  }

  if ((spell = skill_lookup (argy, -1)) == NULL || IS_MOB (ch))
  {
    send_to_char ("I know nothing about that.\n\r", ch);
    return;
  }


  /*if ( spell->slot==0 && get_curr_dex(ch) < spell->min_wis) {
    send_to_char("You aren't dexterous enough to execute this skill, so why learn it?\n\r",ch);
    return;
    }
    if ( spell->slot==0 && get_curr_str(ch) < spell->min_int) {
    send_to_char("You aren't physically strong enough to make use of this skill.\n\r",ch);
    return;
    }
    if ( spell->slot!=0 && get_curr_wis(ch) < spell->min_wis) {
    send_to_char("This spell requires more wisdom than you've got.\n\r",ch);
    return;
    }
    if ( spell->slot!=0 && get_curr_int(ch) < spell->min_int) {
    send_to_char("This spell requires more intelligence than you've got.\n\r",ch);
    return;
    }
  */
  
  {
	bool needFlag = FALSE;
	bool hasFlag = FALSE;

        if (spell->guildflag1)
                needFlag = TRUE;
        if (spell->guildflag2)
                needFlag = TRUE;
        if (spell->guildflag3)
                needFlag = TRUE;
        if (spell->guildflag4)
                needFlag = TRUE;
        if (has_scriptflag(ch, spell->guildflag1))
                hasFlag = TRUE;
        if (has_scriptflag(ch, spell->guildflag2))
                hasFlag = TRUE;
        if (has_scriptflag(ch, spell->guildflag3))
                hasFlag = TRUE;
        if (has_scriptflag(ch, spell->guildflag4))
                hasFlag = TRUE;
//commented this out to try something new This is the old type of code below that didnt work right
	/*if (spell->guildflag1 || spell->guildflag2 || spell->guildflag3) 
            needFlag = TRUE;
	if (has_scriptflag(ch, spell->guildflag1)) || has_scriptflag(ch, spell->guildflag2) || has_scriptflag(ch, spell->guildflag3))
	    hasFlag = TRUE;
  else  if (has_scriptflag(ch, spell->guildflag2))
            hasFlag = TRUE;
  else  if (has_scriptflag(ch, spell->guildflag3))
            hasFlag = TRUE;*/
//nods lol
	if (needFlag && !hasFlag) 
            {
    send_to_char ("You can not learn that with your quest or guild status.\n\r", ch);
    return; 
    }
  }
  if (!(CAN_LEARN (ch, spell)))
  {
    send_to_char ("You can not learn that with your current profession(s).\n\r", ch);
    return;
  }
  if (spell->slot == 0 && skl >= (race_info[ch->pcdata->race].max_skills + ch->pcdata->remort_times))
  {
    send_to_char ("You have already learned the maximum skills for your race.\n\r", ch);
    return;
  }

  if (spell->slot != 0 && spl >= (race_info[ch->pcdata->race].max_spells + (ch->pcdata->remort_times * 2)))
  {
    send_to_char ("You have already learned the maximum spells for your race.\n\r", ch);
    return;
  }

  if (mob == NULL || IS_PLAYER (mob))
    return;
  while (cntt <= 28)
  {
    if (spell->gsn == mob->pIndexData->opt->skltaught[cntt])
    {
      cntt = 50;
      break;
    }
    cntt++;
  }

  if (cntt != 50)
  {
    send_to_char ("I have no knowledge of that to teach you.\n\r", ch);
    return;
  }

  if (LEVEL (ch) < spell->spell_level)
  {
    send_to_char ("You find it too difficult to understand.\n\r", ch);
    return;
  }

  if (IS_PLAYER (ch))
  {
    SPELL_DATA *spl1;
    SPELL_DATA *spl2;
    if (spell->prereq_1 != NULL && spell->prereq_2 == NULL)
    {
      if ((spl1 = skill_lookup (spell->prereq_1, -1)) == NULL)
        goto skippy;
      if (ch->pcdata->learned[spl1->gsn] < pow.prereq)
      {
        sprintf (buf,
            "You need to have sufficient knowledge in %s first.\n\r",
            spl1->spell_funky_name);
        send_to_char (buf, ch);
        return;
      }
    }
    if (spell->prereq_1 != NULL && spell->prereq_2 != NULL)
    {
      if ((spl1 = skill_lookup (spell->prereq_1, -1)) == NULL)
        goto skippy;
      if ((spl2 = skill_lookup (spell->prereq_2, -1)) == NULL)
        goto skippy;
      if (ch->pcdata->learned[spl1->gsn] < pow.prereq)
      {
        sprintf (buf, "You need to have sufficient knowledge in %s first.\n\r", spl1->spell_funky_name);
        send_to_char (buf, ch);
        return;
      }
      if (ch->pcdata->learned[spl2->gsn] < pow.prereq)
      {
        sprintf (buf, "You need to have sufficient knowledge in %s first.\n\r", spl2->spell_funky_name);
        send_to_char (buf, ch);
        return;
      }
    }
skippy:
    if (ch)
    {
    };
  }

  if (ch->pcdata->learned[spell->gsn] != -100)
  {
    sprintf (buf, "You already know %s.\n\r", spell->spell_funky_name);
    send_to_char (buf, ch);
  }

  else
  {
    ch->pcdata->learn--;
    ch->pcdata->learned[spell->gsn] = 1;
    act ("You learn $T.", ch, NULL, spell->spell_funky_name, TO_CHAR);
    act( "$n learns $T.", ch, NULL,spell->spell_funky_name,TO_ROOM);
  }

  return;
}

void
do_skill (CHAR_DATA * ch, char *argy)
{
  char buf[STD_LENGTH];
  char hg[300];
  SPELL_DATA *spell;
  int sn;
  int col;
  int oldtrack;
  DEFINE_COMMAND ("skills", do_skill, POSITION_DEAD, 0, LOG_NORMAL, "This command shows you what skills you know, if any.")
    if (IS_MOB (ch))
    return;
  hugebuf_o[0] = '\0';
  send_to_char ("\x1B[37;1mYou have knowledge in the following skills:\x1B[37;0m\n\r", ch);
  send_to_char ("\x1B[1;34m-----------------------------------------------------\x1B[37;0m\n\r", ch);
  send_to_char ("\x1B[1;31m\n\rSEE: HELP SKILL LEVEL\x1B[37;0m\n\r", ch);
  col = 0;
  oldtrack = ch->pcdata->learned[gsn_track];
  if (is_member (ch, GUILD_RANGER))
  ch->pcdata->learned[gsn_track] = 100;
  if (is_member (ch, GUILD_THIEFG))
  ch->pcdata->learned[gsn_dodge] = 100;
  ch->pcdata->learned[gsn_sneak] += ch->pcdata->plus_sneak;
  ch->pcdata->learned[gsn_hide] += ch->pcdata->plus_hide;
  for (sn = 0; sn < SKILL_COUNT; sn++)
    {
      spell = skill_lookup (NULL, sn);
      if (spell == NULL)
	continue;
      if ((LEVEL (ch) < spell->spell_level) || (ch->pcdata->learned[sn] == -100) || spell->slot != 0)
	continue;
      strcpy (hg, how_good (ch->pcdata->learned[sn]));
      if (LEVEL (ch) >= 100)
	sprintf (buf, "%22s %3d%% ", spell->spell_funky_name, ch->pcdata->learned[sn]);
      else
	sprintf (buf, " %22s %10s", spell->spell_funky_name, hg);
      strcat (hugebuf_o, buf);
      if (++col % 2 == 0)
	strcat (hugebuf_o, "\n\r");
    }

  ch->pcdata->learned[gsn_track] = oldtrack;
  ch->pcdata->learned[gsn_sneak] -= ch->pcdata->plus_sneak;
  ch->pcdata->learned[gsn_hide] -= ch->pcdata->plus_hide;
  if (col % 2 != 0)
    strcat (hugebuf_o, "\n\r");
  sprintf (buf, "\n\rYou have %d practice sessions left.\n\r", ch->pcdata->practice);
  strcat (hugebuf_o, buf);
  page_to_char (hugebuf_o, ch);
  return;
}


void
do_practice (CHAR_DATA * ch, char *argy)
{
  char buf[STD_LENGTH];
  char hg[500];
  bool found;
  int sn;
  int cnttt;
  SPELL_DATA *spell;
  CHAR_DATA *mob;
  int adept;
  DEFINE_COMMAND ("practice", do_practice, POSITION_STANDING, 0, LOG_NORMAL, "This command can be used to list all skills/spells you know, or, at a practitioner, can be used to practice a spell or skill.")
    hugebuf_o[0] = '\0';
  cnttt = 0;
  if (IS_MOB (ch))
    return;
  if (pow.professions && ch->pcdata->profession_flag == 0)
  {
    send_to_char ("You should look into choosing a profession.  Type 'help profession' for info.\n\r", ch);
  }
  for (mob = ch->in_room->more->people; mob != NULL; mob = mob->next_in_room)
  {
    if (IS_MOB (mob) && IS_SET (mob->act, ACT_PRACTICE)
        && mob->pIndexData->opt)
      break;
  }

  if (mob != NULL && (!IS_SET (mob->act, ACT_PRACTICE) || IS_PLAYER (mob)))
  {
    if (argy[0] != '\0')
    {
      send_to_char ("You cannot practice here!\n\r", ch);
      return;
    }
  }

  if (mob == NULL && argy[0] == '\0')
  {
    int col;
    int oldtrack;
    sprintf (hugebuf_o, "\x1B[37;1mYou have knowledge in the following:\x1B[37;0m\n\r\n\r");
    sprintf (hugebuf_o, "\x1B[37;1mYou have knowledge in the following:\x1B[37;0m\n\r");
    sprintf (hugebuf_o + strlen (hugebuf_o), "\x1B[1;34m----------------------------------------------\x1B[37;0m\n\r");

    col = 0;
    oldtrack = ch->pcdata->learned[gsn_track];
    if (is_member (ch, GUILD_RANGER))
      ch->pcdata->learned[gsn_track] = 100;
    if (is_member (ch, GUILD_THIEFG))
      ch->pcdata->learned[gsn_dodge] = 100;
    ch->pcdata->learned[gsn_sneak] += ch->pcdata->plus_sneak;
    ch->pcdata->learned[gsn_hide] += ch->pcdata->plus_hide;
    for (sn = 0; sn < SKILL_COUNT; sn++)
    {
      if ((spell = skill_lookup (NULL, sn)) == NULL)
        continue;
      if ((LEVEL (ch) < spell->spell_level) || (ch->pcdata->learned[sn] < -1))
        continue;
      strcpy (hg, how_good ((spell->slot == 0 ? ch->pcdata->learned[sn] : (ch->pcdata->learned[sn] + (is_member (ch, GUILD_WIZARD) ?  15 : (is_member (ch, GUILD_HEALER) ? 10 : 0))))));
      if (LEVEL (ch) >= 100)
        sprintf (buf, "%22s %3d%% ", spell->spell_funky_name, ch->pcdata->learned[sn]);
      else
        sprintf (buf, " %22s %10s", spell->spell_funky_name, hg);
      strcat (hugebuf_o, buf);
      if (++col % 2 == 0)
        strcat (hugebuf_o, "\n\r");
    }
    ch->pcdata->learned[gsn_track] = oldtrack;
    ch->pcdata->learned[gsn_sneak] -= ch->pcdata->plus_sneak;
    ch->pcdata->learned[gsn_hide] -= ch->pcdata->plus_hide;
    if (col % 2 != 0)
      strcat (hugebuf_o, "\n\r");
    sprintf (buf, "\n\rYou have %d practices and %d learns left.\x1B[0m\n\r", ch->pcdata->practice, ch->pcdata->learn);
    strcat (hugebuf_o, buf);
    page_to_char (hugebuf_o, ch);
    return;
  }

  if (!IS_AWAKE (ch))
  {
    send_to_char ("In your dreams?  Good luck!\n\r", ch);
    return;
  }

  if (argy[0] == '\0')
  {
    int col;
    int oldtrack;
    hugebuf_o[0] = '\0';
    send_to_char ("\x1B[37;1mYou have knowledge in the following:\x1B[37;0m\n\r", ch);
    send_to_char ("\x1B[1;34m-------------------------------------------\x1B[37;0m\n\r", ch);
    send_to_char ("\x1B[1;31m\n\rSEE: HELP SKILL LEVEL\x1B[37;0m\n\r", ch);
    found = FALSE;
    col = 0;
    if (mob == NULL || IS_PLAYER (mob))
      return;
    oldtrack = ch->pcdata->learned[gsn_track];
    if (is_member (ch, GUILD_RANGER))
      ch->pcdata->learned[gsn_track] = 100;
    if (is_member (ch, GUILD_THIEFG))
      ch->pcdata->learned[gsn_dodge] = 100;
      ch->pcdata->learned[gsn_sneak] += ch->pcdata->plus_sneak;
      ch->pcdata->learned[gsn_hide] += ch->pcdata->plus_hide;
    for (sn = 0; sn <= 28; sn++)
    {
      if (mob->pIndexData->opt->skltaught[sn] > 0 &&
          (spell =
           skill_lookup (NULL,
             mob->pIndexData->opt->skltaught[sn])) != NULL)
      {
        if (!CAN_LEARN (ch, spell))
          continue;
	{
		bool needFlag = FALSE;
		bool hasFlag = FALSE;

        if (spell->guildflag1)
                needFlag = TRUE;
        if (spell->guildflag2)
                needFlag = TRUE;
        if (spell->guildflag3)
                needFlag = TRUE;
        if (spell->guildflag4)
                needFlag = TRUE;
        if (has_scriptflag(ch, spell->guildflag1))
                hasFlag = TRUE;
        if (has_scriptflag(ch, spell->guildflag2))
                hasFlag = TRUE;
        if (has_scriptflag(ch, spell->guildflag3))
                hasFlag = TRUE;
        if (has_scriptflag(ch, spell->guildflag4))
                hasFlag = TRUE;
//trying something new
	/*if (spell->guildflag1 || spell->guildflag2 || spell->guildflag3)
		needFlag = TRUE;
	if (has_scriptflag(ch, spell->guildflag1) || has_scriptflag(ch, spell->guildflag2) || has_scriptflag(ch, spell->guildflag3))
		hasFlag = TRUE;*/
//So this was commented out
	if (needFlag && !hasFlag)
          continue;
	}
	if (LEVEL (ch) >= spell->spell_level)
        {
          char hgx[300];
          found = TRUE;
          strcpy (hgx, how_good (ch->pcdata->learned[spell->gsn]));
          sprintf (buf, " %22s %10s", spell->spell_funky_name, hgx);
          strcat (hugebuf_o, buf);
          if (++col % 2 == 0)
            strcat (hugebuf_o, "\n\r");
        }
      }
    }
    ch->pcdata->learned[gsn_track] = oldtrack;
    ch->pcdata->learned[gsn_sneak] -= ch->pcdata->plus_sneak;
    ch->pcdata->learned[gsn_hide] -= ch->pcdata->plus_hide;
    if (!found)
      strcat (hugebuf_o, "Nothing.\n\r");
    if (col % 2 != 0)
      strcat (hugebuf_o, "\n\r");
    sprintf (buf, "\n\rYou have %d practice sessions left.\n\r", ch->pcdata->practice);
    strcat (hugebuf_o, buf);
    page_to_char (hugebuf_o, ch);
    return;
  }

  spell = skill_lookup (argy, -1);
  if (spell == NULL)
  {
    send_to_char ("What's that?\n\r", ch);
    return;
  }

  if (spell == NULL || (IS_PLAYER (ch) && (LEVEL (ch) < spell->spell_level || ch->pcdata->learned[spell->gsn] == -100)))
  {
    if (spell->gsn > 0 && spell->gsn < SKILL_COUNT && ch->pcdata->learned[spell->gsn] == -100)
    {
      do_learn (ch, argy);
      if (ch->pcdata->learned[spell->gsn] == -100)
      {
        send_to_char ("For some reason or another, you could not LEARN this spell/skill.\n\r", ch);
        send_to_char ("Perhaps you are out of learns?  Type 'learn' for more information.\n\r", ch);
        return;
      }
      goto lerned_it;
    }

    send_to_char ("I have no knowledge in that area.\n\r", ch);
    return;
  }
lerned_it:
  if (mob == NULL)
    return;
  while (cnttt <= 28)
  {
    if (IS_MOB (mob)
        && mob->pIndexData->opt->skltaught[cnttt] == spell->gsn)
    {
      cnttt = 50;
      break;
    }
    cnttt++;
  }

  if (cnttt != 50)
  {
    send_to_char ("I have too little knowledge in that area to help you practice.\n\r", ch);
    return;
  }

  if (!(CAN_LEARN (ch, spell)))
  {
    send_to_char ("You can not learn that with your current profession(s).\n\r", ch);
    return;
  }

  {
	  bool needFlag = FALSE;
	  bool hasFlag = FALSE;
	
        if (spell->guildflag1)
                needFlag = TRUE;
        if (spell->guildflag2)
                needFlag = TRUE;
        if (spell->guildflag3)
                needFlag = TRUE;
        if (has_scriptflag(ch, spell->guildflag3))
                hasFlag = TRUE;
        if (has_scriptflag(ch, spell->guildflag1))
                hasFlag = TRUE;
        if (has_scriptflag(ch, spell->guildflag2))
                hasFlag = TRUE;
        if (has_scriptflag(ch, spell->guildflag3))
                hasFlag = TRUE;
        if (has_scriptflag(ch, spell->guildflag4))
                hasFlag = TRUE;
//trying something new
	  /*if (spell->guildflag1 || spell->guildflag2 || spell->guildflag3)
		  needFlag = TRUE;
	  if (has_scriptflag(ch, spell->guildflag1) || has_scriptflag(ch, spell->guildflag2) || has_scriptflag(ch, spell->guildflag3))
		  hasFlag = TRUE;*/
//so I commented this out think bad coding by kenor
  	if (needFlag && !hasFlag)
	  {
    send_to_char ("You do not have the proper quest or guild status.\n\r", ch);
    return; }
  }

  if (ch->pcdata->practice < 1)
  {
    send_to_char ("You have no practice sessions left!\n\r", ch);
    return;
  }

  if (spell->slot != 0)
    adept = pow.max_prac_spells;
  else
    adept = pow.max_prac_skills;

  if (spell->slot == 0 && get_curr_dex (ch) < spell->min_wis)
  {
    adept -= (spell->min_wis - get_curr_dex (ch)) * 8;
    if (adept < 20)
      adept = 20;
  }
  if (spell->slot == 0 && get_curr_str (ch) < spell->min_int)
  {
    adept -= (spell->min_int - get_curr_str (ch)) * 8;
    if (adept < 20)
      adept = 20;
  }
  if (spell->slot != 0 && get_curr_wis (ch) < spell->min_wis)
  {
    adept -= (spell->min_wis - get_curr_wis (ch)) * 8;
    if (adept < 20)
      adept = 20;
  }
  if (spell->slot != 0 && get_curr_int (ch) < spell->min_int)
  {
    adept -= (spell->min_int - get_curr_int (ch)) * 8;
    if (adept < 20)
      adept = 20;
  }

  if (IS_PLAYER (ch))
  {
    SPELL_DATA *spl1;
    SPELL_DATA *spl2;
    if (spell->prereq_1 != NULL && spell->prereq_2 == NULL)
    {
      if ((spl1 = skill_lookup (spell->prereq_1, -1)) == NULL)
        goto skippy;
      if (ch->pcdata->learned[spl1->gsn] < pow.prereq)
      {
        sprintf (buf, "You need to be skilled in %s first.\n\r", spl1->spell_funky_name);
        send_to_char (buf, ch);
        return;
      }
    }
    if (spell->prereq_1 != NULL && spell->prereq_2 != NULL)
    {
      if ((spl1 = skill_lookup (spell->prereq_1, -1)) == NULL)
        goto skippy;
      if ((spl2 = skill_lookup (spell->prereq_2, -1)) == NULL)
        goto skippy;
      if (ch->pcdata->learned[spl1->gsn] < pow.prereq)
      {
        sprintf (buf, "You need to be skilled in %s first.\n\r", spl1->spell_funky_name);
        send_to_char (buf, ch);
        return;
      }
      if (ch->pcdata->learned[spl2->gsn] < pow.prereq)
      {
        sprintf (buf, "You need to be skilled in %s first.\n\r", spl2->spell_funky_name);
        send_to_char (buf, ch);
        return;
      }
    }
skippy:
    if (ch)
    {
    };
  }

  if (ch->pcdata->learned[spell->gsn] >= adept)
  {
    sprintf (buf, "You are already quite good at %s.\n\r", spell->spell_funky_name);
    send_to_char (buf, ch);
  }

  else
  {
    ch->pcdata->practice--;
    if (spell->slot != 0)
      ch->pcdata->learned[spell->gsn] +=
        int_app[get_curr_int (ch) - 1].learn;
    if (spell->slot == 0)
      ch->pcdata->learned[spell->gsn] +=
        int_app[get_curr_int (ch) - 1].learn - 1;
    if (ch->pcdata->learned[spell->gsn] < adept)
    {
      act ("You practice $T.", ch, NULL, spell->spell_funky_name, TO_CHAR);
    }
    else
    {
      ch->pcdata->learned[spell->gsn] = adept;
      act ("You now have a good understanding of $T.", ch, NULL, spell->spell_funky_name, TO_CHAR);
      act ("$n now has a good understanding of $T.", ch, NULL,spell->spell_funky_name,TO_ROOM);
    }
  }

  return;
}

void
do_prereqs (CHAR_DATA * ch, char *argy)
{
  char buf[STD_LENGTH];
  SPELL_DATA *spell;
  DEFINE_COMMAND ("prereqs", do_prereqs, POSITION_DEAD, 0, LOG_NORMAL, "Allows you to see prerequisites to the specified skill or spell.")
    if (IS_MOB (ch))
    return;
  if (argy[0] == '\0' || (spell = skill_lookup (argy, -1)) == NULL)
    {
      send_to_char ("Syntax: Prereq <skill/spell name>\n\r", ch);
      return;
    }

  if (spell->prereq_1 != NULL && spell->prereq_2 != NULL)
    {
      sprintf (buf, "\x1B[0;36mTwo prerequisites are required to learn this and they are: \n\r\x1B[0;35m%s\x1B[0;36m and \x1B[0;35m%s.\x1B[37;0m\n\r", spell->prereq_1, spell->prereq_2);
      send_to_char (buf, ch);
      return;
    }

  if (spell->prereq_1 != NULL)
    {
      sprintf (buf, "\x1B[0;36mOne prerequisite is required to learn this and it is: \n\r\x1B[0;35m%s.\x1B[0m\n\r", spell->prereq_1);
      send_to_char (buf, ch);
      return;
    }

  sprintf (buf, "\x1B[0;36mThere are no prerequisites whatsoever required to learn \x1B[1;37m%s\x1B[0;36m.\n\r", spell->spell_funky_name);
  send_to_char (buf, ch);
  return;
}


void
do_spells (CHAR_DATA * ch, char *argy)
{
  char buf[STD_LENGTH];
  char bbb[50];
  char hg[300];
  SPELL_DATA *spell;
  bool fnd = FALSE;
  int sn;
  int col;
  DEFINE_COMMAND ("spells", do_spells, POSITION_DEAD, 0, LOG_NORMAL, "This command shows you all the spells you currently know.")
    hugebuf_o[0] = '\0';
  if (IS_MOB (ch))
    {
      send_to_char ("You do not know how to cast spells!\n\r", ch);
      return;
    }

  hugebuf_o[0] = '\0';
  send_to_char ("\x1B[37;1mYou have knowledge in the following spells:\x1B[37;0m\n\r", ch);
  send_to_char ("\x1B[1;34m-----------------------------------------------------\x1B[37;0m\n\r", ch);
  send_to_char ("\x1B[1;31m\n\rSEE: HELP SKILL LEVEL\x1B[37;0m\n\r", ch);
  col = 0;
  for (sn = 0; sn < SKILL_COUNT; sn++)
    {
      if ((spell = skill_lookup (NULL, sn)) == NULL)
	continue;
      if ((LEVEL (ch) < spell->spell_level) || (spell->spell_level > 90))
	continue;
      if (spell->slot == 0)
	continue;
      if (ch->pcdata->learned[sn] == -100)
	continue;
      fnd = TRUE;
      if (spell->mana_type == -1)
	strcpy (bbb, " ");
      if (spell->mana_type == MANA_ANY)
	strcpy (bbb, "\x1B[0;37m*");
      if (spell->mana_type == MANA_WATER)
	strcpy (bbb, "\x1B[0;34mW");
      if (spell->mana_type == MANA_AIR)
	strcpy (bbb, "\x1B[0;36mA");
      if (spell->mana_type == MANA_EARTH)
	strcpy (bbb, "\x1B[0;33mE");
      if (spell->mana_type == MANA_FIRE)
	strcpy (bbb, "\x1B[0;31mF");
      if (spell->mana_type == MANA_DEATH)
        strcpy (bbb, "\x1B[1;30mD");
      strcpy (hg, how_good (ch->pcdata->learned[sn] + (is_member (ch, GUILD_WIZARD) ? 15 : (is_member (ch, GUILD_HEALER) ? 10 : 0))));
      if (LEVEL (ch) >= 100)
	sprintf (buf, "\x1B[0;36m[\x1B[1;31m%2d\x1B[0;36m] %s \x1B[1;31m%3d\x1B[0;37mM \x1B[1;30m(\x1B[0;37m%3d\x1B[1;30m) \x1B[0;37m%-25s \x1B[37;0m", 
         spell->spell_level,
         bbb, 
         ((2 + LEVEL (ch) - spell->spell_level == 0) ? spell->mana : (UMAX (spell->mana, 100 / (2 + LEVEL (ch) - spell->spell_level)))), 
         ch->pcdata->learned[sn], 
         spell->spell_funky_name);
      else
	sprintf (buf, "\x1B[0;36m[\x1B[1;31m%2d\x1B[0;36m] %s \x1B[1;31m%3d\x1B[0;37mM \x1B[1;30m(\x1B[1;37m%8s\x1B[1;30m) \x1B[0;37m%-20s \x1B[37;0m", 
        spell->spell_level,
        bbb, 
       ((2 + LEVEL (ch) - spell->spell_level == 0) ? spell->mana : (UMAX (spell->mana, 100 / (2 + LEVEL (ch) - spell->spell_level)))), 
        hg, 
        spell->spell_funky_name);
      strcat (hugebuf_o, buf);
      if (++col % 2 == 0)
	strcat (hugebuf_o, "\n\r");
    }

  if (col % 2 != 0)
    strcat (hugebuf_o, "\n\r");
  if (!fnd)
    strcpy (hugebuf_o, "You know no spells at the moment.\n\r");
  page_to_char (hugebuf_o, ch);
  return;
}

void
do_info_sp_sk (CHAR_DATA * ch, SPELL_DATA * spell)
{
  char buffy[500];
  char hg[300];
  if (IS_MOB (ch))
    {
      send_to_char ("NPC's cannot do this!\n\r", ch);
      return;
    }

  if (spell == NULL)
    {
      send_to_char ("Does not exist.\n\r", ch);
      return;
    }
sprintf(buffy,"\n\r\x1b[1;37m%s\n\r", spell->spell_funky_name);//KILITH 05
send_to_char(buffy, ch);
sprintf(buffy,"\x1b[1;30m[\x1b[1;31m+\x1b[1;30m]\x1b[1;34m-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\x1b[1;30m[\x1b[1;31m+\x1b[1;30m]\x1b[0;37m\n\r");
send_to_char(buffy, ch);
  if (spell->slot == 0)
    sprintf (buffy, "\x1B[0;36mThis is a level \x1B[1;31m%d\x1B[0;36m skill.\x1B[37;0m\n\r\n\r", spell->spell_level);
  else
    sprintf (buffy, "\x1B[0;36mThis is a level \x1B[1;31m%d\x1B[0;36m spell.\x1B[37;0m\n\r\n\r", spell->spell_level);
  send_to_char (buffy, ch);
  do_prereqs (ch, spell->spell_name);
  if (spell->slot == 0)
    {
      strcpy (hg, how_good (ch->pcdata->learned[spell->gsn]));
      sprintf (buffy, "\x1B[0;36mYour proficiency in this skill is \x1B[1;37m%s\x1B[0m.\n\r\n\r", hg);
    }
  else
    {
      strcpy (hg, how_good (ch->pcdata->learned[spell->gsn]));
      sprintf (buffy, "\x1B[0;36mYour proficiency in this spell is \x1B[1;37m%s\x1B[0m.\n\r\n\r", hg);
    }

  send_to_char (buffy, ch);
  if (spell->slot != 0)
    {
      sprintf (buffy, "\x1B[0;36mThis spell is the mana type of %s and takes \x1B[1;34m%d \x1B[0;36mminimum mana.\n\r\n\r", 
        (spell->mana_type == MANA_AIR ?   "'\x1B[0;36mAir\x1B[0;36m'" : 
        (spell->mana_type == MANA_EARTH ? "'\x1B[0;33mEarth\x1B[0;36m'" : 
        (spell->mana_type == MANA_WATER ? "'\x1B[0;34mWater\x1B[0;36m'" : 
        (spell->mana_type == MANA_FIRE ?  "'\x1B[0;31mFire\x1B[0;36m'" :
        (spell->mana_type == MANA_DEATH ? "'\x1B[1;30mDeath\x1B[0;36m'" 
         : "'\x1B[1;37mAny Mana\x1B[0;36m'"))))), spell->mana);
      send_to_char (buffy, ch);
    }
sprintf(buffy,"\x1b[1;30m[\x1b[1;31m+\x1b[1;30m]\x1b[1;34m-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\x1b[1;30m[\x1b[1;31m+\x1b[1;30m]\x1b[0;37m\n\r");
send_to_char(buffy, ch);
  return;
}

void
do_showskills (CHAR_DATA * ch, char *argy)
{
  SPELL_DATA *spellstuff[500];
  char buf[STD_LENGTH];
  SPELL_DATA *spell;
  int sn;
  int bit = 0;
  int col;
  int mnum = 0;
  int level;
  char bbb[40];
  bool pSpell;
  bool spsk;
  bool allflag = FALSE;
  int spec_skill;
  DEFINE_COMMAND ("showskills", do_showskills, POSITION_DEAD, 1, LOG_NORMAL, "This command lists skills you can learn given your current profession(s).")
    for (sn = 0; sn < 500; sn++)
    spellstuff[sn] = NULL;
  spsk = TRUE;
  spec_skill = 0;
  if (pow.professions)
    {
      bit = ch->pcdata->profession_flag;
    }

  spsk = FALSE;

  hugebuf_o[0] = '\0';
  strcat (hugebuf_o, "Skills available:\n\r");
  strcat (hugebuf_o, "Lv Skills\n\r-------------\n\r");
  for (sn = 0; sn < SKILL_COUNT; sn++)
    {
      if ((spell = skill_lookup (NULL, sn)) == NULL)
	continue;
      if (allflag && spell->slot == 0)
	spsk = FALSE;
      if (allflag && spell->slot != 0)
	spsk = TRUE;
      if (spsk && spell->slot == 0)
	continue;
      if (!spsk && spell->slot != 0)
	continue;
      if (spsk && spell->spell_level > 110)
	continue;
      if (pow.professions && !(spell->values[0] & bit))
	continue;
      spellstuff[mnum] = spell;
      mnum++;
    }

  for (level = 0; level <= 110; level++)
    {
      col = 0;
      pSpell = TRUE;
      for (sn = 0; sn < mnum; sn++)
	{
	  spell = spellstuff[sn];
	  if (spell->spell_level != level)
	    continue;
	  if (pSpell)
	    {
	      sprintf (buf, "%2d:", level);
	      strcat (hugebuf_o, buf);
	      pSpell = FALSE;
	    }
	  else
	    strcat (hugebuf_o, "   ");
	  if (spsk && spell->mana_type == -1)
	    strcpy (bbb, " ");
	  if (spsk && spell->mana_type == MANA_ANY)
	    strcpy (bbb, "\x1B[1;37m*\x1B[0m");
	  if (spsk && spell->mana_type == MANA_WATER)
	    strcpy (bbb, "\x1B[0;34mW\x1B[37;0m");
	  if (spsk && spell->mana_type == MANA_AIR)
	    strcpy (bbb, "\x1B[0;36mA\x1B[37;0m");
	  if (spsk && spell->mana_type == MANA_EARTH)
	    strcpy (bbb, "\x1B[0;33mE\x1B[37m");
	  if (spsk && spell->mana_type == MANA_FIRE)
	    strcpy (bbb, "\x1B[0;31mF\x1B[37;0m");
          if (spsk && spell->mana_type == MANA_DEATH)
            strcpy (bbb, "\x1B[1;30mD\x1B[37;0m");
	  if (spsk && LEVEL (ch) > 109)
	    sprintf (buf, "%22s (%10s) %s (%2d)  ", spell->spell_name, spell->spell_funky_name, bbb, spell->mana);
	  else if (spsk)
	    sprintf (buf, "%22s %s (%2d)  ", spell->spell_funky_name, bbb, spell->mana);
	  else
	    sprintf (buf, "%22s", spell->spell_funky_name);
	  strcat (hugebuf_o, buf);
	  if ((++col) % 2 == 0)
	    {
	      strcat (hugebuf_o, "\n\r");
	    }
	}
      if (col % 2 != 0)
	strcat (hugebuf_o, "\n\r");
    }
  strcat (hugebuf_o, "\n\r");

  page_to_char (hugebuf_o, ch);
  return;
}

void
do_showspells (CHAR_DATA * ch, char *argy)
{
  SPELL_DATA *spellstuff[500];
  char buf[STD_LENGTH];
  SPELL_DATA *spell;
  int sn;
  int bit = 0;
  int col;
  int mnum = 0;
  int level;
  char bbb[40];
  bool pSpell;
  bool spsk;
  bool allflag = FALSE;
  int spec_skill;
  DEFINE_COMMAND ("showspells", do_showspells, POSITION_DEAD, 1, LOG_NORMAL, "This command lists spells you can learn given your current profession(s).")
    for (sn = 0; sn < 500; sn++)
    spellstuff[sn] = NULL;
  spsk = TRUE;
  spec_skill = 0;
  if (pow.professions)
    {
      bit = ch->pcdata->profession_flag;
    }

  spsk = TRUE;

  hugebuf_o[0] = '\0';
  strcat (hugebuf_o, "Spells available:\n\r");
  strcat (hugebuf_o, "-----------------\n\r");
  for (sn = 0; sn < SKILL_COUNT; sn++)
    {
      if ((spell = skill_lookup (NULL, sn)) == NULL)
	continue;
      if (allflag && spell->slot == 0)
	spsk = FALSE;
      if (allflag && spell->slot != 0)
	spsk = TRUE;
      if (spsk && spell->slot == 0)
	continue;
      if (!spsk && spell->slot != 0)
	continue;
      if (spsk && spell->spell_level > 110)
	continue;
      if (pow.professions && !(spell->values[0] & bit))
	continue;
      spellstuff[mnum] = spell;
      mnum++;
    }

  for (level = 0; level <= 110; level++)
    {
      col = 0;
      pSpell = TRUE;
      for (sn = 0; sn < mnum; sn++)
	{
	  spell = spellstuff[sn];
	  if (spell->spell_level != level)
	    continue;
	  if (pSpell)
	    {
	      sprintf (buf, "%2d:", level);
	      strcat (hugebuf_o, buf);
	      pSpell = FALSE;
	    }
	  else
	    strcat (hugebuf_o, "   ");
	  if (spsk && spell->mana_type == -1)
	    strcpy (bbb, " ");
	  if (spsk && spell->mana_type == MANA_ANY)
	    strcpy (bbb, "\x1B[1;37m*\x1B[0m");
	  if (spsk && spell->mana_type == MANA_WATER)
	    strcpy (bbb, "\x1B[0;34mW\x1B[37;0m");
	  if (spsk && spell->mana_type == MANA_AIR)
	    strcpy (bbb, "\x1B[0;36mA\x1B[37;0m");
	  if (spsk && spell->mana_type == MANA_EARTH)
	    strcpy (bbb, "\x1B[0;33mE\x1B[37m");
	  if (spsk && spell->mana_type == MANA_FIRE)
	    strcpy (bbb, "\x1B[0;31mF\x1B[37;0m");
          if (spsk && spell->mana_type == MANA_DEATH)
            strcpy (bbb, "\x1B[1;30mD\x1B[37;0m");
	  if (FALSE && spsk && LEVEL (ch) > 109)

	    sprintf (buf,
		     "%22s (%10s) %s (%3d) \x1B[1;36m%22s\x1B[37;0m \x1B[1;36m%22s\x1B[37;0m\n\r",
		     spell->spell_name, spell->spell_funky_name, bbb,
		     spell->mana,
		     (spell->prereq_1 == NULL ? "" : spell->prereq_1),
		     (spell->prereq_2 == NULL ? "" : spell->prereq_2));
	  else if (spsk)
	    sprintf (buf,
		     "%22s %s (%3d) \x1B[1;36m%22s\x1B[37;0m \x1B[1;36m%22s\x1B[37;0m\n\r",
		     spell->spell_funky_name, bbb, spell->mana,
		     (spell->prereq_1 == NULL ? "" : spell->prereq_1),
		     (spell->prereq_2 == NULL ? "" : spell->prereq_2));
	  else
	    sprintf (buf, "%18s\n\r", spell->spell_funky_name);
	  strcat (hugebuf_o, buf);
	}
    }
  strcat (hugebuf_o, "\n\r");

  page_to_char (hugebuf_o, ch);
  return;
}


void
do_slist (CHAR_DATA * ch, char *argy)
{
  SPELL_DATA *spellstuff[500];
  char buf[STD_LENGTH];
  SPELL_DATA *spell;
  int sn;
  int bit = 0;
  int mnum = 0;
  int col;
  int level;
  char bbb[40];
  bool pSpell;
  bool spsk;
  bool allflag = FALSE;
  int spec_skill;
  DEFINE_COMMAND ("slist", do_slist, POSITION_DEAD, 1, LOG_NORMAL, "This command lists spells/skills.") 
   if (argy[0] == '\0')
    {
      send_to_char ("'slist spells': list all spells in the game.\n\r'slist skills': list all skills in the game.\n\r", ch);
      send_to_char ("'slist <profession>': list what spells and skills this profession gets.\n\r", ch);
      return;
    }

  spsk = TRUE;
  spec_skill = 0;
  if (pow.professions)
    {
      S_ALL (argy, bit);

      if (bit != 0)
	allflag = TRUE;
      if (bit == 0)
	bit = 0xffffffff;
      if (bit == PROF_DRUID)
	bit += PROF_HEALER;
      if (bit == PROF_ARCHMAGE)
	bit += PROF_WIZARD;
    }
  if (!allflag)
    {
      if (!str_cmp (argy, "spells"))
	spsk = TRUE;
      else if (!str_cmp (argy, "skills"))
	spsk = FALSE;
      else
	{
	  if ((spell = skill_lookup (argy, -1)) == NULL)
	    {
	      send_to_char ("Valid argys are 'spells' or skills' ...or slist <specific spell/skill>.\n\r", ch);
	      return;
	    }
	  do_info_sp_sk (ch, spell);
	  return;
	}
    }

  hugebuf_o[0] = '\0';
  if (!allflag)
    {
      if (spsk)
	strcat (hugebuf_o, "\n\r\x1B[0;37mSpells available:\n\n\r");
      else
	strcat (hugebuf_o, "\n\r\x1B[0;37mSkills available:\n\n\r");
      if (spsk)
strcat (hugebuf_o, "\x1B[0;37m  Lv       \x1B[0;36mSpells                     \x1B[0;37mMT    \x1B[0;31mMN     \x1B[0;35mPre-1         \x1B[0;35mPre-2\n\r\x1B[1;32m+\x1B[1;34m------------------------------------------------------------------------------\x1B[1;32m+\x1B[0;37m\n\r");
      else
	strcat (hugebuf_o, "Lv Skills\n\r---------\n\r");
    }

  for (sn = 0; sn < SKILL_COUNT; sn++)
    {
      if ((spell = skill_lookup (NULL, sn)) == NULL)
	continue;
      if (allflag && spell->slot == 0)
	spsk = FALSE;
      if (allflag && spell->slot != 0)
	spsk = TRUE;
      if (spsk && spell->slot == 0)
	continue;
      if (!spsk && spell->slot != 0)
	continue;
      if (spsk && spell->spell_level > 110)
	continue;
      if (pow.professions && !(spell->values[0] & bit))
	continue;
      spellstuff[mnum] = spell;
      mnum++;
    }

  for (level = 0; level <= 110; level++)
    {
      col = 0;
      pSpell = TRUE;
      for (sn = 0; sn < mnum; sn++)
	{
	  spell = spellstuff[sn];
	  if (spell->spell_level != level)
	    continue;
	  if (pSpell)
	    {
	      sprintf (buf, "\x1B[0;36m[\x1B[1;31m%2d\x1B[0;36m]", level);
	      strcat (hugebuf_o, buf);
              if(IS_SET(spell->spell_bits, SPELL_ALL_IN_ROOM))
              sprintf(buf, "\x1b[0;35m Area ");
          else
             sprintf(buf, "      ");
          strcat (hugebuf_o, buf);
	    }
	  else
	    strcat (hugebuf_o, "");
	  if (spsk && spell->mana_type == MANA_ANY)
	    strcpy (bbb, "\x1B[1;37m*\x1B[0m");
	  else if (spsk && spell->mana_type == MANA_WATER)
	    strcpy (bbb, "\x1B[0;34mW\x1B[37;0m");
	  else if (spsk && spell->mana_type == MANA_AIR)
	    strcpy (bbb, "\x1B[0;36mA\x1B[37;0m");
	  else if (spsk && spell->mana_type == MANA_EARTH)
	    strcpy (bbb, "\x1B[0;33mE\x1B[37m");
	  else if (spsk && spell->mana_type == MANA_FIRE)
	    strcpy (bbb, "\x1B[0;31mF\x1B[37;0m");
          else if (spsk && spell->mana_type == MANA_DEATH)
            strcpy (bbb, "\x1B[1;30mD\x1B[37;0m");
	  else
	    strcpy (bbb, " ");
	  if (spsk)
//go_here	    
sprintf (buf, "\x1B[0;36m |%-24s \x1B[0;37m [%s\x1B[0;37m] \x1B[0;36m(\x1B[1;31m%3d\x1B[0;36m) \x1B[0;35m%-20s \x1B[0;35m%-20s\x1B[0;37m\n\r",
		     spell->spell_funky_name, bbb, spell->mana,
		     (spell->prereq_1 == NULL ? "" : spell->prereq_1),
		     (spell->prereq_2 == NULL ? "" : spell->prereq_2));
	  else
	    sprintf (buf, "%22s\n\r", spell->spell_funky_name);
	  strcat (hugebuf_o, buf);
	}
    }
  strcat (hugebuf_o, "\n\r");

  page_to_char (hugebuf_o, ch);
  return;
}
