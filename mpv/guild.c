#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "emlen.h"

/*TINKER GUILD ALLOWS MEND SKILL, +1 WIS */
/*WARRIOR GUILD gives +1 STR +10 max HPS and FLURRY COMMAND */
/*HEALER GUILD gives +1 WIS and +hps on heal spells */
/*WIZARD GUILD gives +damage on some offensive spells and +1 INT */
/*RANGER GUILD gives +1 CON and makes you track at 100% */
/*ASSASSIN GUILD gives you +1 DEX, increases BS damage */
/*THIEF GUILD gives you +1 DEX, increase dodge */

bool
is_member (CHAR_DATA * ch, int guildflag)
{
  if (IS_MOB (ch))
    return TRUE;		/* Mobs are guild members */
  if (IS_SET (ch->pcdata->guilds, guildflag))
    return TRUE;
  else
    return FALSE;
}

void flagGuild (CHAR_DATA * mob, CHAR_DATA * target, int guild, bool flag)
{
	char buf[STD_LENGTH];

	sprintf (buf, "%s scriptflag ", NAME(target));
	switch (guild)
	{
	case ACT3_TINKER:
	    strcat(buf, "tinkerg"); 
		break;
	case ACT3_WARRIOR:
	    strcat(buf, "warriorg"); 
		break;
	case ACT3_RANGER:
	    strcat(buf, "rangerg"); 
		break;
	case ACT3_ASSASSIN:
	    strcat(buf, "assassing"); 
		break;
	case ACT3_THIEFG:
	    strcat(buf, "thiefg"); 
		break;
	case ACT3_PALADIN:
	    strcat(buf, "palading"); 
		break;
	case ACT3_ELEMENTAL:
	    strcat(buf, "elementg"); 
		break;
	case ACT3_BATTLEMAGE:
	    strcat(buf, "battleg"); 
		break;
	case ACT3_MARAUDER:
	    strcat(buf, "marauderg"); 
		break;
        case ACT3_WIZARD:
            strcat(buf, "wizardg"); 
		break;
        case ACT3_HEALER:
            strcat(buf, "healerg"); 
		break;
        case ACT3_NECROMANCER:
            strcat(buf, "necrog"); 
		break;
        case ACT3_TATICS:
            strcat(buf, "tacticg");
                break;
        case ACT3_SHAMAN:
            strcat(buf, "shamang");
                break;
        case ACT3_MYSTICS:
            strcat(buf, "mysticg");
                break;
	}
	if (flag)
	  strcat(buf, " ON");
	else
	  strcat(buf, " OFF");
	
	do_pset(mob, buf);
}
void
do_guild (CHAR_DATA * ch, char *argy)
{
  CHAR_DATA *mob;
  char buf[STD_LENGTH];
  char arg1[SML_LENGTH];
  char arg2[SML_LENGTH];
  DEFINE_COMMAND ("guild", do_guild, POSITION_STANDING, 0, LOG_NORMAL, "This command allows you to perform guild options at a guild house.")
    if (IS_MOB (ch))
    return;

  argy = one_argy (argy, arg1);	// Only join uses the second argument
  argy = one_argy (argy, arg2);	// so technically we could put this there

  for (mob = ch->in_room->more->people; mob != NULL; mob = mob->next_in_room)
    {
      if (IS_MOB (mob) &&
	     (IS_SET (mob->pIndexData->act3, ACT3_TINKER)
	   || IS_SET (mob->pIndexData->act3, ACT3_WARRIOR)
	   || IS_SET (mob->pIndexData->act3, ACT3_HEALER)
	   || IS_SET (mob->pIndexData->act3, ACT3_WIZARD)
	   || IS_SET (mob->pIndexData->act3, ACT3_THIEFG)
	   || IS_SET (mob->pIndexData->act3, ACT3_RANGER)
	   || IS_SET (mob->pIndexData->act3, ACT3_ASSASSIN)
	   || IS_SET (mob->pIndexData->act3, ACT3_PALADIN)
	   || IS_SET (mob->pIndexData->act3, ACT3_ELEMENTAL)
	   || IS_SET (mob->pIndexData->act3, ACT3_BATTLEMAGE)
	   || IS_SET (mob->pIndexData->act3, ACT3_MARAUDER)
           || IS_SET (mob->pIndexData->act3, ACT3_TATICS)
           || IS_SET (mob->pIndexData->act3, ACT3_SHAMAN)
           || IS_SET (mob->pIndexData->act3, ACT3_MYSTICS)
           || IS_SET (mob->pIndexData->act3, ACT3_NECROMANCER)))
	break;
    }
  if (mob == NULL)
    {
      send_to_char ("There is no guildmaster present here!\n\r", ch);
      return;
    }
  if (!str_cmp (arg1, "info"))
    {
      if (IS_SET (mob->pIndexData->act3, ACT3_TINKER))
	{
	  sprintf (buf, "The Tinker's guild costs %d coins to join. You must be level %d.", COST_TINKER, LEVEL_TINKER);
          do_say (mob, buf);
	  do_help (ch, "TINKER");
	  return;
	}
      if (IS_SET (mob->pIndexData->act3, ACT3_TATICS))
        {
          sprintf (buf, "The Tactic's guild costs %d coins to join. You must be level %d.", COST_TATICS, LEVEL_TATICS);
          do_say (mob, buf);
          do_help (ch, "TACTICS");
          return;
        }
      if (IS_SET (mob->pIndexData->act3, ACT3_SHAMAN))
        {
          sprintf (buf, "The Shaman's guild costs %d coins to join. You must be level %d.", COST_SHAMAN, LEVEL_SHAMAN);
          do_say (mob, buf);
          do_help (ch, "SHAMAN");
          return;
        }
      if (IS_SET (mob->pIndexData->act3, ACT3_WARRIOR))
	{
	  sprintf (buf, "The Warrior's guild costs %d coins to join.  You must be level %d.", COST_WARRIOR, LEVEL_WARRIOR);
          do_say (mob, buf);
          do_help (ch, "WARRIOR");
	  return;
	}
      if (IS_SET (mob->pIndexData->act3, ACT3_HEALER))
	{
	  sprintf (buf, "The Healer's guild costs %d coins to join.  You must be level %d.", COST_HEALER, LEVEL_HEALER);
          do_say (mob, buf);
          do_help (ch, "HEALER");
	  return;
	}
      if (IS_SET (mob->pIndexData->act3, ACT3_WIZARD))
	{
	  sprintf (buf, "The Wizard's guild costs %d coins to join.  You must be level %d.", COST_WIZARD, LEVEL_WIZARD);
          do_say (mob, buf);
          do_help (ch, "WIZARD");
	  return;
	}
      if (IS_SET (mob->pIndexData->act3, ACT3_THIEFG))
	{
	  sprintf (buf, "The Thief's guild costs %d coins to join.  You must be level %d.", COST_THIEFG, LEVEL_THIEFG);
          do_say (mob, buf);
          do_help (ch, "THIEF");
	  return;
	}
      if (IS_SET (mob->pIndexData->act3, ACT3_RANGER))
	{
	  sprintf (buf, "The Ranger's guild costs %d coins to join.  You must be level %d.", COST_RANGER, LEVEL_RANGER);
          do_say (mob, buf);
          do_help (ch, "RANGER");
	  return;
	}
      if (IS_SET (mob->pIndexData->act3, ACT3_ASSASSIN))
	{
	  sprintf (buf, "The Assassin's guild costs %d coins to join.  You must be level %d.", COST_ASSASSIN, LEVEL_ASSASSIN);
          do_say (mob, buf);
          do_help (ch, "ASSASSIN");
	  return;
	}

      if (IS_SET (mob->pIndexData->act3, ACT3_PALADIN))
	{
	  sprintf (buf, "The Paladin's guild costs %d coins to join. You must be level %d.", COST_PALADIN, LEVEL_PALADIN);
          do_say (mob, buf);
          do_help (ch, "PALADIN");
	  return;
	}

      if (IS_SET (mob->pIndexData->act3, ACT3_ELEMENTAL))
	{
	  sprintf (buf, "The Elemental's guild costs %d coins to join. You must be level %d.", COST_ELEMENTAL, LEVEL_ELEMENTAL);
          do_say (mob, buf);
          do_help (ch, "ELEMENTAL");
	  return;
	}

      if (IS_SET (mob->pIndexData->act3, ACT3_BATTLEMAGE))
	{
	  sprintf (buf, "The Battlemage's guild costs %d coins to join. You must be level %d.", COST_BATTLEMAGE, LEVEL_BATTLEMAGE);
          do_say (mob, buf);
          do_help (ch, "BATTLEMAGE");
	  return;
	}

      if (IS_SET (mob->pIndexData->act3, ACT3_MARAUDER))
	{
	  sprintf (buf, "The Marauder's guild costs %d coins to join. You must be level %d.", COST_MARAUDER, LEVEL_MARAUDER);
          do_say (mob, buf);
          do_help (ch, "MARAUDER");
	  return;
	}
      if (IS_SET (mob->pIndexData->act3, ACT3_NECROMANCER))
        {
          sprintf (buf, "The Necromancer's guild costs %d coins to join. You must be level %d.", COST_NECROMANCER, LEVEL_NECROMANCER);
          do_say (mob, buf);
          do_help (ch, "NECROMANCER");
          return;
        }
      if (IS_SET (mob->pIndexData->act3, ACT3_MYSTICS))
        {
          sprintf (buf, "The Mystics's guild costs %d coins to join. You must be level %d.", COST_MYSTICS, LEVEL_MYSTICS);
          do_say (mob, buf);
          do_help (ch, "MYSTICS");
          return;
    }
}				/*end of GUILD INFO */
  if (!str_cmp (arg1, "status"))
    {
      if (!IS_SET (ch->pcdata->guilds, (mob->pIndexData->act3)))
	{
	  sprintf (buf, "You are not a member of our guild!\n\r");
          do_say (mob, buf);
	  return;
	}
   if (ch->pcdata->remort_times < 1)
        {
      sprintf  (buf, "You are a beginer of the guild %s.\n\r", NAME(ch));
      do_say (mob, buf);
       }
   else if (ch->pcdata->remort_times == 1)
        {
      sprintf  (buf, "You are a apprentice of the guild %s.\n\r", NAME(ch));
      do_say (mob, buf);
       }
   else if (ch->pcdata->remort_times >= 2 && ch->pcdata->remort_times <= 5)
        {
      sprintf  (buf, "You are a novice of the guild %s.\n\r", NAME(ch));
      do_say (mob, buf);
       }
   else if (ch->pcdata->remort_times > 5 && ch->pcdata->remort_times <= 8)
        {
      sprintf  (buf, "You are an experienced member of the guild %s.\n\r", NAME(ch));
      do_say (mob, buf);
       }
   else if (ch->pcdata->remort_times > 8)
        {
      sprintf  (buf, "You are a veteran of the guild %s.\n\r", NAME(ch));
      do_say (mob, buf);
       }
     return;
   }				
     /*end of GUILD STATUS */

  if (!str_cmp (arg1,"leave"))
    {

      if (!is_member (ch, (mob->pIndexData->act3)))
	{
	  send_to_char ("You aren't even a member!!\n\r", ch);
	  return;
	}
       /*sprintf (buf, "%s ", NAME(ch));
       do_preset (mob, buf);*/

      if (IS_SET (mob->pIndexData->act3, ACT3_TINKER))
        {
          ch->pcdata->perm_stat[CON_I] -= 1;
          ch->pcdata->guilds -= ACT3_TINKER;
          flagGuild(mob, ch, ACT3_TINKER, FALSE);
          sprintf (buf, "%s all 0 ", NAME(ch));
          do_sset (mob, buf);
          sprintf (buf, "%s practice 100 ", NAME(ch));
          do_pset (mob, buf);
          sprintf (buf, "%s learn 100 ", NAME(ch));
          do_pset (mob, buf);
        }
      else if (IS_SET (mob->pIndexData->act3, ACT3_WARRIOR))
        {
          ch->pcdata->perm_stat[STR_I] -= 1;
          ch->pcdata->guilds -= ACT3_WARRIOR;
          flagGuild(mob, ch, ACT3_WARRIOR, FALSE);
          sprintf (buf, "%s all 0 ", NAME(ch));
          do_sset (mob, buf);
          sprintf (buf, "%s practice 100 ", NAME(ch));
          do_pset (mob, buf);
          sprintf (buf, "%s learn 100 ", NAME(ch));
          do_pset (mob, buf);
        }
      else if (IS_SET (mob->pIndexData->act3, ACT3_HEALER))
        {
          ch->pcdata->perm_stat[WIS_I] -= 1;
          ch->pcdata->guilds -= ACT3_HEALER;
          flagGuild(mob, ch, ACT3_HEALER, FALSE);
          sprintf (buf, "%s all 0 ", NAME(ch));
          do_sset (mob, buf);
          sprintf (buf, "%s practice 100 ", NAME(ch));
          do_pset (mob, buf);
          sprintf (buf, "%s learn 100 ", NAME(ch));
          do_pset (mob, buf);
        }
      else if (IS_SET (mob->pIndexData->act3, ACT3_WIZARD))
        {
          ch->pcdata->perm_stat[INT_I] -= 1;
          ch->pcdata->guilds -= ACT3_WIZARD;
          flagGuild(mob, ch, ACT3_WIZARD, FALSE);
          sprintf (buf, "%s all 0 ", NAME(ch));
          do_sset (mob, buf);
          sprintf (buf, "%s practice 100 ", NAME(ch));
          do_pset (mob, buf);
          sprintf (buf, "%s learn 100 ", NAME(ch));
          do_pset (mob, buf);
        }
      else if (IS_SET (mob->pIndexData->act3, ACT3_RANGER))
        {
          ch->pcdata->perm_stat[CON_I] -= 1;
          ch->pcdata->guilds -= ACT3_RANGER;
          flagGuild(mob, ch, ACT3_RANGER, FALSE);
          sprintf (buf, "%s no_view_mobs", NAME(ch));
          do_pset(mob, buf);
          sprintf (buf, "%s all 0 ", NAME(ch));
          do_sset (mob, buf);
          sprintf (buf, "%s practice 100 ", NAME(ch));
          do_pset (mob, buf);
          sprintf (buf, "%s learn 100 ", NAME(ch));
          do_pset (mob, buf);
        }
      else if (IS_SET (mob->pIndexData->act3, ACT3_ASSASSIN))
        {
          ch->pcdata->perm_stat[DEX_I] -= 1;
          ch->pcdata->guilds -= ACT3_ASSASSIN;
          flagGuild(mob, ch, ACT3_ASSASSIN, FALSE);
          sprintf (buf, "%s all 0 ", NAME(ch));
          do_sset (mob, buf);
          sprintf (buf, "%s practice 100 ", NAME(ch));
          do_pset (mob, buf);
          sprintf (buf, "%s learn 100 ", NAME(ch));
          do_pset (mob, buf);
        }
      else if (IS_SET (mob->pIndexData->act3, ACT3_THIEFG))
        {
          ch->pcdata->perm_stat[DEX_I] -= 1;
          ch->pcdata->guilds -= ACT3_THIEFG;
          flagGuild(mob, ch, ACT3_THIEFG, FALSE);
          sprintf (buf, "%s all 0 ", NAME(ch));
          do_sset (mob, buf);
          sprintf (buf, "%s practice 100 ", NAME(ch));
          do_pset (mob, buf);
          sprintf (buf, "%s learn 100 ", NAME(ch));
          do_pset (mob, buf);
        }
      else if (IS_SET (mob->pIndexData->act3, ACT3_PALADIN))
        {
          ch->pcdata->perm_stat[STR_I] -= 1;
          ch->pcdata->guilds -= ACT3_PALADIN;
          flagGuild(mob, ch, ACT3_PALADIN, FALSE);
          sprintf (buf, "%s all 0 ", NAME(ch));
          do_sset (mob, buf);
          sprintf (buf, "%s practice 100 ", NAME(ch));
          do_pset (mob, buf);
          sprintf (buf, "%s learn 100 ", NAME(ch));
          do_pset (mob, buf);
        }
      else if (IS_SET (mob->pIndexData->act3, ACT3_ELEMENTAL))
        {
          ch->pcdata->perm_stat[INT_I] -= 1;
          ch->pcdata->guilds -= ACT3_ELEMENTAL;
          flagGuild(mob, ch, ACT3_ELEMENTAL, FALSE);
          sprintf (buf, "%s all 0 ", NAME(ch));
          do_sset (mob, buf);
          sprintf (buf, "%s practice 100 ", NAME(ch));
          do_pset (mob, buf);
          sprintf (buf, "%s learn 100 ", NAME(ch));
          do_pset (mob, buf);
        }
      else if (IS_SET (mob->pIndexData->act3, ACT3_BATTLEMAGE))
        {
          ch->pcdata->perm_stat[STR_I] -= 1;
          ch->pcdata->guilds -= ACT3_BATTLEMAGE;
          flagGuild(mob, ch, ACT3_BATTLEMAGE, FALSE);
          sprintf (buf, "%s all 0 ", NAME(ch));
          do_sset (mob, buf);
          sprintf (buf, "%s practice 100 ", NAME(ch));
          do_pset (mob, buf);
          sprintf (buf, "%s learn 100 ", NAME(ch));
          do_pset (mob, buf);
        }
      else if (IS_SET (mob->pIndexData->act3, ACT3_MARAUDER))
        {
          ch->pcdata->perm_stat[STR_I] -= 1;
          ch->pcdata->guilds -= ACT3_MARAUDER;
          flagGuild(mob, ch, ACT3_MARAUDER, FALSE);
          sprintf (buf, "%s all 0 ", NAME(ch));
          do_sset (mob, buf);
          sprintf (buf, "%s practice 100 ", NAME(ch));
          do_pset (mob, buf);
          sprintf (buf, "%s learn 100 ", NAME(ch));
          do_pset (mob, buf);
        }
      else if (IS_SET (mob->pIndexData->act3, ACT3_NECROMANCER))
        {
          ch->pcdata->perm_stat[WIS_I] -= 1;
          ch->pcdata->guilds -= ACT3_NECROMANCER;
          flagGuild(mob, ch, ACT3_NECROMANCER, FALSE);
          sprintf (buf, "%s all 0 ", NAME(ch));
          do_sset (mob, buf);
          sprintf (buf, "%s practice 100 ", NAME(ch));
          do_pset (mob, buf);
          sprintf (buf, "%s learn 100 ", NAME(ch));
          do_pset (mob, buf);
        }
      else if (IS_SET (mob->pIndexData->act3, ACT3_MYSTICS))
        {
          ch->pcdata->perm_stat[INT_I] -= 1;
          ch->pcdata->guilds -= ACT3_MYSTICS;
          flagGuild(mob, ch, ACT3_MYSTICS, FALSE);
          sprintf (buf, "%s all 0 ", NAME(ch));
          do_sset (mob, buf);
          sprintf (buf, "%s practice 100 ", NAME(ch));
          do_pset (mob, buf);
          sprintf (buf, "%s learn 100 ", NAME(ch));
          do_pset (mob, buf);
        }
      else if (IS_SET (mob->pIndexData->act3, ACT3_TATICS))
        {
          ch->pcdata->perm_stat[CON_I] -= 1;
          ch->pcdata->guilds -= ACT3_TATICS;
          flagGuild(mob, ch, ACT3_TATICS, FALSE);
          sprintf (buf, "%s all 0 ", NAME(ch));
          do_sset (mob, buf);
          sprintf (buf, "%s practice 100 ", NAME(ch));
          do_pset (mob, buf);
          sprintf (buf, "%s learn 100 ", NAME(ch));
          do_pset (mob, buf);
        }
      else if (IS_SET (mob->pIndexData->act3, ACT3_SHAMAN))
        {
          ch->pcdata->perm_stat[WIS_I] -= 1;
          ch->pcdata->guilds -= ACT3_SHAMAN;
          flagGuild(mob, ch, ACT3_SHAMAN, FALSE);
          sprintf (buf, "%s all 0 ", NAME(ch));
          do_sset (mob, buf);
          sprintf (buf, "%s practice 100 ", NAME(ch));
          do_pset (mob, buf);
          sprintf (buf, "%s learn 100 ", NAME(ch));
          do_pset (mob, buf);
        }
      sprintf (buf, "You are no longer a member of the guild %s!!", NAME (ch));
      do_say (mob, buf);
      return;
    }				/*end of GUILD LEAVE */

  if (!str_cmp (arg1, "join"))
    {
      int cst;
      int lvl;
      lvl = 0;
      cst = 0;      
      
      if (IS_SET (ch->pcdata->guilds, mob->pIndexData->act3))
	{
	  sprintf (buf, "You are already a member of our guild %s!", NAME (ch));
          do_say (mob, buf);
	  return;
	}
//KILITH
//I WILL REDO ALL GUILD HATES AND ALSO OTHER THINGS WHEN TIME COMES.

//SHAMAN GUILD HATES GO WITH ONLY PALADIN, HEALER, OR NECROMANCERS IF THEY WANT THEM
  if (IS_SET (mob->pIndexData->act3, ACT3_SHAMAN))
        {
          if (IS_SET (ch->pcdata->guilds, GUILD_THIEFG))
            {
              sprintf (buf, "We don't get along with members of the Thief's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_ASSASSIN))
            {
              sprintf (buf, "We don't get along with members of the Assassin's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_MARAUDER))
            {
              sprintf (buf, "We don't get along with members of the Marauder's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_WARRIOR))
            {
              sprintf (buf, "We don't get along with members of the Battle Warrior's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_BATTLEMAGE))
            {
              sprintf (buf, "We don't get along with members of the Battle Mage's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_WIZARD))
            {
              sprintf (buf, "We don't get along with members of the Wizard's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_ELEMENTAL))
            {
              sprintf (buf, "We don't get along with members of the Elemental's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_TATICS))
            {
              sprintf (buf, "We don't get along with members of the Tactic's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          /*if (IS_SET (ch->pcdata->guilds, GUILD_MYSTICS))
            {
              sprintf (buf, "We don't get along with members of the Mystic's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }*/
	}

//TACTICS GUILD ONLY WARRIOR, BATTLEMAGE, PALADIN, MARAUDER
  if (IS_SET (mob->pIndexData->act3, ACT3_TATICS))
        {
          if (IS_SET (ch->pcdata->guilds, GUILD_THIEFG))
            {
              sprintf (buf, "We don't get along with members of the Thief's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_ASSASSIN))
            {
              sprintf (buf, "We don't get along with members of the Assassin's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_WIZARD))
            {
              sprintf (buf, "We don't get along with members of the Wizard's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_NECROMANCER))
            {
              sprintf (buf, "We don't get along with members of the Necromancer's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_HEALER))
            {
              sprintf (buf, "We don't get along with members of the Healer's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_SHAMAN))
            {
              sprintf (buf, "We don't get along with members of the Shaman's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_ELEMENTAL))
            {
              sprintf (buf, "We don't get along with members of the Elemental's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_MYSTICS))
            {
              sprintf (buf, "We don't get along with members of the Mystic's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
	}

//MYSTICS GUILDS HATES
  if (IS_SET (mob->pIndexData->act3, ACT3_MYSTICS))
        {
          if (IS_SET (ch->pcdata->guilds, GUILD_WARRIOR))
            {
              sprintf (buf, "We don't get along with members of the Battle Warrior's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_THIEFG))
            {
              sprintf (buf, "We don't get along with members of the Thief's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_HEALER))
            {
              sprintf (buf, "We don't get along with members of the Healer's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_MARAUDER))
            {
              sprintf (buf, "We don't get along with members of the Marauder's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_PALADIN))
            {
              sprintf (buf, "We don't get along with members of the Paladin's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          /*if (IS_SET (ch->pcdata->guilds, GUILD_SHAMAN) || !IS_SET (ch->pcdata->guilds, NECROMANCER))
            {
              sprintf (buf, "We don't get along with members of the Shaman's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }*/
          if (IS_SET (ch->pcdata->guilds, GUILD_TATICS))
            {
              sprintf (buf, "We don't get along with members of the Tactic's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_ASSASSIN))
            {
              sprintf (buf, "We don't get along with members of the Assassin's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
        }

//WARRIOR GUILD HATES
  if (IS_SET (mob->pIndexData->act3, ACT3_WARRIOR))
	{
          if (IS_SET (ch->pcdata->guilds, GUILD_MYSTICS))
            {
              sprintf (buf, "We don't get along with members of the Healer's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_SHAMAN))
            {
              sprintf (buf, "We don't get along with members of the Shaman's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
	  if (IS_SET (ch->pcdata->guilds, GUILD_HEALER))
	    {
	      sprintf (buf, "We don't get along with members of the Healer's Guild %s.", NAME (ch));
		do_say (mob, buf);
	        return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_WIZARD))
	    {
              sprintf (buf, "We don't get along with members of the Wizard's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_THIEFG))
	    {
              sprintf (buf, "We don't get along with members of the Thief's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_BATTLEMAGE))
	    {
              sprintf (buf, "We don't get along with members of the Battle Mages's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_MARAUDER))
	    {
              sprintf (buf, "We don't get along with members of the Marauder's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_PALADIN))
	    {
              sprintf (buf, "We don't get along with members of the Paladin's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_ELEMENTAL))
	    {
              sprintf (buf, "We don't get along with members of the Elemental's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_ASSASSIN))
	    {
              sprintf (buf, "We don't get along with members of the Assassin's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
          if (IS_SET (ch->pcdata->guilds, GUILD_NECROMANCER))
            {
              sprintf (buf, "We don't get along with members of the Necromancer's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
	}

//HEALER GUILD HATES
    if (IS_SET (mob->pIndexData->act3, ACT3_HEALER))
	{
          if (IS_SET (ch->pcdata->guilds, GUILD_MYSTICS))
            {
              sprintf (buf, "We don't get along with members of the Healer's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_NECROMANCER))
            {
              sprintf (buf, "We don't get along with members of the Necromancer's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
	  if (IS_SET (ch->pcdata->guilds, GUILD_ASSASSIN))
	    {
              sprintf (buf, "We don't get along with members of the Assassin's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_WIZARD))
	    {
              sprintf (buf, "We don't get along with members of the Wizard's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_THIEFG))
	    {
              sprintf (buf, "We don't get along with members of the Thief's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_WARRIOR))
	    {
              sprintf (buf, "We don't get along with members of the Warrior's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_MARAUDER))
	    {
             sprintf (buf, "We don't get along with members of the Marauder's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_ELEMENTAL))
	    {
              sprintf (buf, "We don't get along with members of the Elemental's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_PALADIN))
	    {
              sprintf (buf, "We don't get along with members of the Paladin's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_BATTLEMAGE))
	    {
              sprintf (buf, "We don't get along with members of the Battle Mage's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
          if (IS_SET (ch->pcdata->guilds, GUILD_TATICS))
            {
              sprintf (buf, "We don't get along with members of the Tactic's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
	}

//WIZARD GUILD HATES
      if (IS_SET (mob->pIndexData->act3, ACT3_WIZARD))
	{
          if (IS_SET (ch->pcdata->guilds, GUILD_SHAMAN))
            {
              sprintf (buf, "We don't get along with members of the Shaman's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_TATICS))
            {
              sprintf (buf, "We don't get along with members of the Tactic's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_NECROMANCER))
            {
              sprintf (buf, "We don't get along with members of the Necromancer's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
	  if (IS_SET (ch->pcdata->guilds, GUILD_ASSASSIN))
	    {
              sprintf (buf, "We don't get along with members of the Assassin's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_HEALER))
	    {
              sprintf (buf, "We don't get along with members of the Healer's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_THIEFG))
	    {
              sprintf (buf, "We don't get along with members of the Thief's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_WARRIOR))
	    {
              sprintf (buf, "We don't get along with members of the Warrior's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_MARAUDER))
	    {
              sprintf (buf, "We don't get along with members of the Marauder's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_ELEMENTAL))
	    {
              sprintf (buf, "We don't get along with members of the Elemental's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_PALADIN))
	    {
              sprintf (buf, "We don't get along with members of the Paladin's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_BATTLEMAGE))
	    {
              sprintf (buf, "We don't get along with members of the Battle Mage's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	}

//ASSASSIN GUILD HATES
      if (IS_SET (mob->pIndexData->act3, ACT3_ASSASSIN))
	{
          if (IS_SET (ch->pcdata->guilds, GUILD_SHAMAN))
            {
              sprintf (buf, "We don't get along with members of the Shaman's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_TATICS))
            {
              sprintf (buf, "We don't get along with members of the Tactic's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_MYSTICS))
            {
              sprintf (buf, "We don't get along with members of the Healer's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_NECROMANCER))
            {
              sprintf (buf, "We don't get along with members of the Necromancer's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
	  if (IS_SET (ch->pcdata->guilds, GUILD_WIZARD))
	    {
              sprintf (buf, "We don't get along with members of the Wizard's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_HEALER))
	    {
              sprintf (buf, "We don't get along with members of the Healers's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_WARRIOR))
	    {
              sprintf (buf, "We don't get along with members of the Warrior's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_PALADIN))
	    {
              sprintf (buf, "We don't get along with members of the Paladin's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_ELEMENTAL))
	    {
              sprintf (buf, "We don't get along with members of the Elemental's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_BATTLEMAGE))
	    {
              sprintf (buf, "We don't get along with members of the Battle Mage's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	}

//THIEF GUILD HATES
      if (IS_SET (mob->pIndexData->act3, ACT3_THIEFG))
	{
          if (IS_SET (ch->pcdata->guilds, GUILD_SHAMAN))
            {
              sprintf (buf, "We don't get along with members of the Shaman's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_TATICS))
            {
              sprintf (buf, "We don't get along with members of the Tactic's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_MYSTICS))
            {
              sprintf (buf, "We don't get along with members of the Healer's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_NECROMANCER))
            {
              sprintf (buf, "We don't get along with members of the Necromancer's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
	  if (IS_SET (ch->pcdata->guilds, GUILD_WARRIOR))
	    {
              sprintf (buf, "We don't get along with members of the Warrior's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_WIZARD))
	    {
              sprintf (buf, "We don't get along with members of the Wizard's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_HEALER))
	    {
              sprintf (buf, "We don't get along with members of the Healer's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_MARAUDER))
	    {
              sprintf (buf, "We don't get along with members of the Marauder's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_PALADIN))
	    {
              sprintf (buf, "We don't get along with members of the Paladin's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_BATTLEMAGE))
	    {
              sprintf (buf, "We don't get along with members of the Battle Mage's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_ELEMENTAL))
	    {
              sprintf (buf, "We don't get along with members of the Elemental's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
	    }
	}

//MARAUDER GUILD HATES
      if (IS_SET (mob->pIndexData->act3, ACT3_MARAUDER))
	{
          if (IS_SET (ch->pcdata->guilds, GUILD_MYSTICS))
            {
              sprintf (buf, "We don't get along with members of the Healer's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_NECROMANCER))
            {
              sprintf (buf, "We don't get along with members of the Necromancer's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_SHAMAN))
            {
              sprintf (buf, "We don't get along with members of the Shaman's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
	  if (IS_SET (ch->pcdata->guilds, GUILD_WARRIOR))
	    {
              sprintf (buf, "We don't get along with members of the Warrior's Guild %s.", NAME (ch));
                do_say (mob, buf);
	      return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_WIZARD))
	    {
              sprintf (buf, "We don't get along with members of the Wizard's Guild %s.", NAME (ch));
                do_say (mob, buf);
	      return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_HEALER))
	    {
              sprintf (buf, "We don't get along with members of the Healer's Guild %s.", NAME (ch));
                do_say (mob, buf);
	      return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_PALADIN))
	    {
              sprintf (buf, "We don't get along with members of the Paladin's Guild %s.", NAME (ch));
                do_say (mob, buf);
	      return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_ELEMENTAL))
	    {
              sprintf (buf, "We don't get along with members of the Elemental's Guild %s.", NAME (ch));
                do_say (mob, buf);
	      return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_THIEFG))
	    {
              sprintf (buf, "We don't get along with members of the Thief's Guild %s.", NAME (ch));
                do_say (mob, buf);
	      return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_BATTLEMAGE))
	    {
              sprintf (buf, "We don't get along with members of the Battle Mage's Guild %s.", NAME (ch));
                do_say (mob, buf);
	      return;
	    }
	}

//PALADIN GUILD HATES
      if (IS_SET (mob->pIndexData->act3, ACT3_PALADIN))
	{
          if (IS_SET (ch->pcdata->guilds, GUILD_MYSTICS))
            {
              sprintf (buf, "We don't get along with members of the Healer's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_NECROMANCER))
            {
              sprintf (buf, "We don't get along with members of the Necromancer's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
	  if (IS_SET (ch->pcdata->guilds, GUILD_BATTLEMAGE))
	    {
              sprintf (buf, "We don't get along with members of the Battle Mage's Guild %s.", NAME (ch));
                do_say (mob, buf);
	      return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_WARRIOR))
	    {
              sprintf (buf, "We don't get along with members of the Warrior's Guild %s.", NAME (ch));
                do_say (mob, buf);
	      return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_THIEFG))
	    {
              sprintf (buf, "We don't get along with members of the Thief's Guild %s.", NAME (ch));
                do_say (mob, buf);
	      return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_ASSASSIN))
	    {
              sprintf (buf, "We don't get along with members of the Assassin's Guild %s.", NAME (ch));
                do_say (mob, buf);
	      return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_WIZARD))
	    {
              sprintf (buf, "We don't get along with members of the Wizard's Guild %s.", NAME (ch));
                do_say (mob, buf);
	      return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_HEALER))
	    {
              sprintf (buf, "We don't get along with members of the Healer's Guild %s.", NAME (ch));
                do_say (mob, buf);
	      return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_ELEMENTAL))
	    {
              sprintf (buf, "We don't get along with members of the Elemental's Guild %s.", NAME (ch));
                do_say (mob, buf);
	      return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_MARAUDER))
	    {
              sprintf (buf, "We don't get along with members of the Marauder's Guild %s.", NAME (ch));
                do_say (mob, buf);
	      return;
	    }
	}

//ELEMENTAL GUILD HATES
      if (IS_SET (mob->pIndexData->act3, ACT3_ELEMENTAL))
	{
          if (IS_SET (ch->pcdata->guilds, GUILD_NECROMANCER))
            {
              sprintf (buf, "We don't get along with members of the Necromancer's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
	  if (IS_SET (ch->pcdata->guilds, GUILD_WARRIOR))
	    {
              sprintf (buf, "We don't get along with members of the Warrior's Guild %s.", NAME (ch));
                do_say (mob, buf);
	      return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_THIEFG))
	    {
              sprintf (buf, "We don't get along with members of the Thief's Guild %s.", NAME (ch));
                do_say (mob, buf);
	      return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_ASSASSIN))
	    {
              sprintf (buf, "We don't get along with members of the Assassin's Guild %s.", NAME (ch));
                do_say (mob, buf);
	      return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_WIZARD))
	    {
              sprintf (buf, "We don't get along with members of the Wizard's Guild %s.", NAME (ch));
                do_say (mob, buf);
	      return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_HEALER))
	    {
              sprintf (buf, "We don't get along with members of the Healer's Guild %s.", NAME (ch));
                do_say (mob, buf);
	      return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_BATTLEMAGE))
	    {
              sprintf (buf, "We don't get along with members of the Battle Mage's Guild %s.", NAME (ch));
                do_say (mob, buf);
	      return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_MARAUDER))
	    {
              sprintf (buf, "We don't get along with members of the Marauder's Guild %s.", NAME (ch));
                do_say (mob, buf);
	      return;
	    }
	  if (IS_SET (ch->pcdata->guilds, GUILD_PALADIN))
	    {
              sprintf (buf, "We don't get along with members of the Paladin's Guild %s.", NAME (ch));
                do_say (mob, buf);
	      return;
	    }
          if (IS_SET (ch->pcdata->guilds, GUILD_SHAMAN))
            {
              sprintf (buf, "We don't get along with members of the Shaman's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_TATICS))
            {
              sprintf (buf, "We don't get along with members of the Tactic's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
	}

//BATTLEMAGE GUILD HATES
      if (IS_SET (mob->pIndexData->act3, ACT3_BATTLEMAGE))
	{
          if (IS_SET (ch->pcdata->guilds, GUILD_NECROMANCER))
            {
              sprintf (buf, "We don't get along with members of the Necromancer's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_WARRIOR))
            {
              sprintf (buf, "We don't get along with members of the Warrior's Guild %s.", NAME (ch));
                do_say (mob, buf);
              return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_THIEFG))
            {
              sprintf (buf, "We don't get along with members of the Thief's Guild %s.", NAME (ch));
                do_say (mob, buf);
              return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_ASSASSIN))
            {
              sprintf (buf, "We don't get along with members of the Assassin's Guild %s.", NAME (ch));
                do_say (mob, buf);
              return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_WIZARD))
            {
              sprintf (buf, "We don't get along with members of the Wizard's Guild %s.", NAME (ch));
                do_say (mob, buf);
              return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_HEALER))
            {
              sprintf (buf, "We don't get along with members of the Healer's Guild %s.", NAME (ch));
                do_say (mob, buf);
              return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_MARAUDER))
            {
              sprintf (buf, "We don't get along with members of the Marauder's Guild %s.", NAME (ch));
                do_say (mob, buf);
              return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_PALADIN))
            {
              sprintf (buf, "We don't get along with members of the Paladin's Guild %s.", NAME (ch));
                do_say (mob, buf);
              return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_ELEMENTAL))
            {
              sprintf (buf, "We don't get along with members of the Elemental's Guild %s.", NAME (ch));
                do_say (mob, buf);
              return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_SHAMAN))
            {
              sprintf (buf, "We don't get along with members of the Shaman's Guild %s.", NAME (ch));
                do_say (mob, buf);
              return;
            }
	}
//NECROMANCER GUILD HATES
      if (IS_SET (mob->pIndexData->act3, ACT3_NECROMANCER))
        {
          if (IS_SET (ch->pcdata->guilds, GUILD_BATTLEMAGE))
            {
              sprintf (buf, "We don't get along with members of the Battle Mage's Guild %s.", NAME (ch));
                do_say (mob, buf);
                return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_WARRIOR))
            {
              sprintf (buf, "We don't get along with members of the Warrior's Guild %s.", NAME (ch));
                do_say (mob, buf);
              return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_THIEFG))
            {
              sprintf (buf, "We don't get along with members of the Thief's Guild %s.", NAME (ch));
                do_say (mob, buf);
              return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_ASSASSIN))
            {
              sprintf (buf, "We don't get along with members of the Assassin's Guild %s.", NAME (ch));
                do_say (mob, buf);
              return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_WIZARD))
            {
              sprintf (buf, "We don't get along with members of the Wizard's Guild %s.", NAME (ch));
                do_say (mob, buf);
              return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_HEALER))
            {
              sprintf (buf, "We don't get along with members of the Healer's Guild %s.", NAME (ch));
                do_say (mob, buf);
              return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_BATTLEMAGE))
            {
              sprintf (buf, "We don't get along with members of the Battle Mage's Guild %s.", NAME (ch));
                do_say (mob, buf);
              return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_MARAUDER))
            {
              sprintf (buf, "We don't get along with members of the Marauder's Guild %s.", NAME (ch));
                do_say (mob, buf);
              return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_PALADIN))
            {
              sprintf (buf, "We don't get along with members of the Paladin's Guild %s.", NAME (ch));
                do_say (mob, buf);
              return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_ELEMENTAL))
            {
              sprintf (buf, "We don't get along with members of the Elemental's Guild %s.", NAME (ch));
                do_say (mob, buf);
              return;
            }
          if (IS_SET (ch->pcdata->guilds, GUILD_TATICS))
            {
              sprintf (buf, "We don't get along with members of the Tactic's Guild %s.", NAME (ch));
                do_say (mob, buf);
              return;
            }
	}
/* Kilith added in the hates of Paladin, Marauder, Elemental, and Battlemage Feb 2003*/
/*Added in Necromancers and Mystics 2006. My god 3 years how sad.*/
/*Added in the new guilds of Shaman and Tatics so that we can have all guilds have 4 guilds. 2006*/

//GUILD COSTS AND LEVELS
      if (IS_SET (mob->pIndexData->act3, ACT3_TINKER))
	{
	  cst = COST_TINKER;
	  lvl = LEVEL_TINKER;
	}
      else if (IS_SET (mob->pIndexData->act3, ACT3_TATICS))
        {
          cst = COST_TATICS;
          lvl = LEVEL_TATICS;
        }
      else if (IS_SET (mob->pIndexData->act3, ACT3_SHAMAN))
        {
          cst = COST_SHAMAN;
          lvl = LEVEL_SHAMAN;
        }
      else if (IS_SET (mob->pIndexData->act3, ACT3_WARRIOR))
	{
	  cst = COST_WARRIOR;
	  lvl = LEVEL_WARRIOR;
	}
      else if (IS_SET (mob->pIndexData->act3, ACT3_HEALER))
	{
	  cst = COST_HEALER;
	  lvl = LEVEL_HEALER;
	}
      else if (IS_SET (mob->pIndexData->act3, ACT3_WIZARD))
	{
	  cst = COST_WIZARD;
	  lvl = LEVEL_WIZARD;
	}
      else if (IS_SET (mob->pIndexData->act3, ACT3_RANGER))
	{
	  cst = COST_RANGER;
	  lvl = LEVEL_RANGER;
	}
      else if (IS_SET (mob->pIndexData->act3, ACT3_ASSASSIN))
	{
	  cst = COST_ASSASSIN;
	  lvl = LEVEL_ASSASSIN;
	}
      else if (IS_SET (mob->pIndexData->act3, ACT3_THIEFG))
	{
	  cst = COST_THIEFG;
	  lvl = LEVEL_THIEFG;
	}
      else if (IS_SET (mob->pIndexData->act3, ACT3_PALADIN))
	{
	  cst = COST_PALADIN;
	  lvl = LEVEL_PALADIN;
	}
      else if (IS_SET (mob->pIndexData->act3, ACT3_ELEMENTAL))
	{
	  cst = COST_ELEMENTAL;
	  lvl = LEVEL_ELEMENTAL;
	}
      else if (IS_SET (mob->pIndexData->act3, ACT3_BATTLEMAGE))
	{
	  cst = COST_BATTLEMAGE;
	  lvl = LEVEL_BATTLEMAGE;
	}
      else if (IS_SET (mob->pIndexData->act3, ACT3_MARAUDER))
	{
	  cst = COST_MARAUDER;
	  lvl = LEVEL_MARAUDER;
	}
      else if (IS_SET (mob->pIndexData->act3, ACT3_MYSTICS))
        {
          cst = COST_MYSTICS;
          lvl = LEVEL_MYSTICS;
        }
      else if (IS_SET (mob->pIndexData->act3, ACT3_NECROMANCER))
        {
          cst = COST_NECROMANCER;
          lvl = LEVEL_NECROMANCER;
        }
     if (ch->pcdata->remort_times < 1 && LEVEL (ch) < lvl)
	{
	  sprintf (buf, "You need to be level %d to join the guild or have at least 1 remort!\n\r", lvl);
	  send_to_char (buf, ch);
	  return;
	}
      if (tally_coins (ch) < cst)
	{
	  sprintf (buf, "We need a payment of %d coins before we can let you join.\n\r", cst);
	  send_to_char (buf, ch);
	  return;
	}

	
	sub_coins (cst, ch);
        
        if (IS_SET (mob->pIndexData->act3, ACT3_TINKER))
          {
            ch->pcdata->perm_stat[CON_I] += 1;
            ch->pcdata->guilds += ACT3_TINKER;
              sprintf (buf, "Congratulations %s!  You are now a member of the Tinker's Guild!", NAME (ch));
                do_say (mob, buf);
              sprintf (buf, "\n\r\x1B[0;31mThere is a new member to the \x1B[1;37mTinker's Guild\x1B[1;31m!!\x1B[0;37m");
                do_echo (mob, buf);
             flagGuild(mob, ch, ACT3_TINKER, TRUE);
          }
        else if (IS_SET (mob->pIndexData->act3, ACT3_MYSTICS))
          {
            ch->pcdata->perm_stat[INT_I] += 1;
            ch->pcdata->guilds += ACT3_MYSTICS;
              sprintf (buf, "Congratulations %s!  You are now a member of the Mystics's Guild!", NAME (ch));
                do_say (mob, buf);
              sprintf (buf, "\n\r\x1B[0;31mThere is a new member to the \x1B[1;37mMystic's Guild\x1B[1;31m!!\x1B[0;37m");
                do_echo (mob, buf);
             flagGuild(mob, ch, ACT3_MYSTICS, TRUE);
          }
	else if (IS_SET (mob->pIndexData->act3, ACT3_TATICS))
	  {
	    ch->pcdata->perm_stat[CON_I] += 1;
	    ch->pcdata->guilds += ACT3_TATICS;
              sprintf (buf, "Congratulations %s!  You are now a member of the Tactic's Guild!", NAME (ch));
                do_say (mob, buf);
              sprintf (buf, "\n\r\x1B[0;31mThere is a new member to the \x1B[1;37mTactic's Guild\x1B[1;31m!!\x1B[0;37m");
                do_echo (mob, buf);
             flagGuild(mob, ch, ACT3_TATICS, TRUE);
	  }
	else if (IS_SET (mob->pIndexData->act3, ACT3_WARRIOR))
	  {
	    ch->pcdata->perm_stat[STR_I] += 1;
	    ch->pcdata->guilds += ACT3_WARRIOR;
              sprintf (buf, "Congratulations %s!  You are now a member of the Warrior's Guild!", NAME (ch));
                do_say (mob, buf);
              sprintf (buf, "\n\r\x1B[0;31mThere is a new member to the \x1B[1;37mWarrior's Guild\x1B[1;31m!!\x1B[0;37m");
                do_echo (mob, buf);
	    flagGuild(mob, ch, ACT3_WARRIOR, TRUE);
	  }
	else if (IS_SET (mob->pIndexData->act3, ACT3_HEALER))
	  {
	    ch->pcdata->perm_stat[WIS_I] += 1;
	    ch->pcdata->guilds += ACT3_HEALER;
              sprintf (buf, "Congratulations %s!  You are now a member of the Healer's Guild!", NAME (ch));
                do_say (mob, buf);
              sprintf (buf, "\n\r\x1B[0;31mThere is a new member to the \x1B[1;37mHealer's Guild\x1B[1;31m!!\x1B[0;37m");
                do_echo (mob, buf);
	    flagGuild(mob, ch, ACT3_HEALER, TRUE);
	  }
	else if (IS_SET (mob->pIndexData->act3, ACT3_WIZARD))
	  {
	    ch->pcdata->perm_stat[INT_I] += 1;
	    ch->pcdata->guilds += ACT3_WIZARD;
              sprintf (buf, "Congratulations %s!  You are now a member of the Wizard's Guild!", NAME (ch));
                do_say (mob, buf);
              sprintf (buf, "\n\r\x1B[0;31mThere is a new member to the \x1B[1;37mWizard's Guild\x1B[1;31m!!\x1B[0;37m");
                do_echo (mob, buf);
	    flagGuild(mob, ch, ACT3_WIZARD, TRUE);
	  }
	else if (IS_SET (mob->pIndexData->act3, ACT3_RANGER))
	  {
	    ch->pcdata->perm_stat[CON_I] += 1;
	    ch->pcdata->guilds += ACT3_RANGER;
              sprintf (buf, "Congratulations %s!  You are now a member of the Ranger's Guild!", NAME (ch));
                do_say (mob, buf);
              sprintf (buf, "\n\r\x1B[0;31mThere is a new member to the \x1B[1;37mRanger's Guild\x1B[1;31m!!\x1B[0;37m");
                do_echo (mob, buf);
          flagGuild(mob, ch, ACT3_RANGER, TRUE);
          sprintf (buf, "%s view_mobs", NAME(ch));
          do_pset(mob, buf);

	  }
	else if (IS_SET (mob->pIndexData->act3, ACT3_ASSASSIN))
	  {
	    ch->pcdata->perm_stat[DEX_I] += 1;
	    ch->pcdata->guilds += ACT3_ASSASSIN;
              sprintf (buf, "Congratulations %s!  You are now a member of the Assassin's Guild!", NAME (ch));
                do_say (mob, buf);
              sprintf (buf, "\n\r\x1B[0;31mThere is a new member to the \x1B[1;37mAssassin's Guild\x1B[1;31m!!\x1B[0;37m");
                do_echo (mob, buf);
	    flagGuild(mob, ch, ACT3_ASSASSIN, TRUE);
	  }
	else if (IS_SET (mob->pIndexData->act3, ACT3_THIEFG))
	  {
	    ch->pcdata->perm_stat[DEX_I] += 1;
	    ch->pcdata->guilds += ACT3_THIEFG;
              sprintf (buf, "Congratulations %s!  You are now a member of the Thief's Guild!", NAME (ch));
                do_say (mob, buf);
              sprintf (buf, "\n\r\x1B[0;31mThere is a new member to the \x1B[1;37mThief's Guild\x1B[1;31m!!\x1B[0;37m");
                do_echo (mob, buf);
	    flagGuild(mob, ch, ACT3_THIEFG, TRUE);
	  }
	else if (IS_SET (mob->pIndexData->act3, ACT3_PALADIN))
	  {
            ch->pcdata->perm_stat[WIS_I] += 1;
	    ch->pcdata->guilds += ACT3_PALADIN;
              sprintf (buf, "Congratulations %s!  You are now a member of the Paladin's Guild!", NAME (ch));
                do_say (mob, buf);
              sprintf (buf, "\n\r\x1B[0;31mThere is a new member to the \x1B[1;37mPaladin's Guild\x1B[1;31m!!\x1B[0;37m");
                do_echo (mob, buf);
	    flagGuild(mob, ch, ACT3_PALADIN, TRUE);
	  }
	else if (IS_SET (mob->pIndexData->act3, ACT3_ELEMENTAL))
	  {
            ch->pcdata->perm_stat[INT_I] += 1;
	    ch->pcdata->guilds += ACT3_ELEMENTAL;
              sprintf (buf, "Congratulations %s!  You are now a member of the Elemental's Guild!", NAME (ch));
                do_say (mob, buf);
              sprintf (buf, "\n\r\x1B[0;31mThere is a new member to the \x1B[1;37mElemental's Guild\x1B[1;31m!!\x1B[0;37m");
                do_echo (mob, buf);
	    flagGuild(mob, ch, ACT3_ELEMENTAL, TRUE);
	  }
	else if (IS_SET (mob->pIndexData->act3, ACT3_BATTLEMAGE))
	  {
            ch->pcdata->perm_stat[STR_I] += 1;
	    ch->pcdata->guilds += ACT3_BATTLEMAGE;
              sprintf (buf, "Congratulations %s!  You are now a member of the Battle Mage's Guild!", NAME (ch));
                do_say (mob, buf);
              sprintf (buf, "\n\r\x1B[0;31mThere is a new member to the \x1B[1;37mBattle Mage's Guild\x1B[1;31m!!\x1B[0;37m");
                do_echo (mob, buf);
	    flagGuild(mob, ch, ACT3_BATTLEMAGE, TRUE);
	  }
	else if (IS_SET (mob->pIndexData->act3, ACT3_MARAUDER))
	  {
            ch->pcdata->perm_stat[STR_I] += 1;
	    ch->pcdata->guilds += ACT3_MARAUDER;
              sprintf (buf, "Congratulations %s!  You are now a member of the Marauder's Guild!", NAME (ch));
                do_say (mob, buf);
              sprintf (buf, "\n\r\x1B[0;31mThere is a new member to the \x1B[1;37mMarauder's Guild\x1B[1;31m!!\x1B[0;37m");
                do_echo (mob, buf);
	    flagGuild(mob, ch, ACT3_MARAUDER, TRUE);
	  }
        else if (IS_SET (mob->pIndexData->act3, ACT3_SHAMAN))
          {
            ch->pcdata->perm_stat[WIS_I] += 1;
            ch->pcdata->guilds += ACT3_SHAMAN;
              sprintf (buf, "Congratulations %s!  You are now a member of the Shaman's Guild!", NAME (ch));
                do_say (mob, buf);
              sprintf (buf, "\n\r\x1B[0;31mThere is a new member to the \x1B[1;37mShaman's Guild\x1B[1;31m!!\x1B[0;37m");
                do_echo (mob, buf);
             flagGuild(mob, ch, ACT3_SHAMAN, TRUE);
          }
        else if (IS_SET (mob->pIndexData->act3, ACT3_NECROMANCER))
          {
            ch->pcdata->perm_stat[WIS_I] += 1;
            ch->pcdata->guilds += ACT3_NECROMANCER;
              sprintf (buf, "Congratulations %s!  You are now a member of the Necromancer's Guild!", NAME (ch));
                do_say (mob, buf);
              sprintf (buf, "\n\r\x1B[0;31mThere is a new member to the \x1B[1;37mNecromancer's Guild\x1B[1;31m!!\x1B[0;37m");
                do_echo (mob, buf);
             flagGuild(mob, ch, ACT3_NECROMANCER, TRUE);
          }
return;
       }
    				/*end of GUILD JOIN */

	send_to_char("\n\rOptions:\n\r---> Guild leave\n\r---> Guild info\n\r---> Guild join\n\r---> Guild status\n\r---> Help Guild\n\r", ch);
  return;
}
