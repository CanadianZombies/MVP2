/*
 * Spell editor 
 */
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"
#define KEY( literal, field, value ) if ( !str_cmp( buf, literal ) ) {field=value; fMatch=TRUE;}
static bool prereq_count[SKILL_COUNT];
/*
 * This function places all spells in the correct hashed spell list by
 * name spots... called whenever a spell name is changed :) 
 */
void
update_prereq (void)
{
  SPELL_DATA *spell;
  SPELL_DATA *prereq;
  int i, j, counter;

  /* The idea here is to set up something so that its alot faster
     doing the bigass prereq trees. */

  for (i = 0; i < SPELL_HASH; i++)
    {
      for (spell = spell_list[i]; spell != NULL; spell = spell->next)
        {

          if ((prereq = skill_lookup(spell->prereq_1, -1)) != NULL)
            {
              spell->pre1 = prereq;
            }
          if ((prereq = skill_lookup(spell->prereq_2, -1)) != NULL)
            {
              spell->pre2 = prereq;
            }

        }
    }

  /* This is so that we dont have to recalc the number of prereqs for
     any given skill each time it asks for it. */

  for (i = 0; i < SPELL_HASH; i++)
    {
      for (spell = spell_list[i]; spell != NULL; spell = spell->next)
        {
          for (j = 0; j < SKILL_COUNT; j++)
            {
              prereq_count[j] = FALSE;
            }
          check_prereq(spell);
          counter = 0;
          for (j = 0; j < SKILL_COUNT; j++)
            {
              if (prereq_count[j] == TRUE)
                {
                  counter++;
                }
            }
          spell->num_prereqs = counter;

        /* Now we set up the check for whether or not we make a vert line
           to the second prereq in the spell tree. */
          if(spell->pre1 && spell->pre2 && spell->pre1 != spell->pre2)
             {
                spell->go_to_second = check_prereq_depth(spell,spell->pre2);
             }
           else
            spell->go_to_second = FALSE;
        }
    }
  return;
}

void check_prereq(SPELL_DATA *spell)
{
  prereq_count[spell->gsn] = TRUE;
  if (spell->pre1 != NULL && !prereq_count[spell->pre1->gsn])
    check_prereq(spell->pre1);
  if (spell->pre2 != NULL && !prereq_count[spell->pre2->gsn])
    check_prereq(spell->pre2);
  return;
}


void
update_name_hash (void)
{
  SPELL_DATA *spell;
  int i;
  bzero (spell_list_2, sizeof (spell_list_2));
  for (i = 0; i < SPELL_HASH; i++)
    {
      for (spell = spell_list[i]; spell != NULL; spell = spell->next)
	{
	  spell->next_a =
	    spell_list_2[UPPER (spell->spell_funky_name[0]) % SPELL_HASH];
	  spell_list_2[UPPER (spell->spell_funky_name[0]) % SPELL_HASH] =
	    spell;
	}
    }
  return;
}

/*
 * This function is called when gsn's are changed... for instance, right
 * at bootup when it assigns a high gsn to a skill... 
 */
void
update_gsn_hash (void)
{
  SPELL_DATA *spell;
  int i;
  bzero (spell_list, sizeof (spell_list));
  for (i = 0; i < SPELL_HASH; i++)
    {
      for (spell = spell_list_2[i]; spell != NULL; spell = spell->next_a)
	{
	  spell->next = spell_list[spell->gsn % SPELL_HASH];
	  spell_list[spell->gsn % SPELL_HASH] = spell;
	}
    }
  return;
}

bool
is_digit (char test)
{
  if (test >= '0' && test <= '9')
    return TRUE;
  else
    return FALSE;
}

/*
 * SPELLS/SKILLS are now hashed by name and spell gsn... fast lookups 
 */
SPELL_DATA *
skill_lookup (char *txt, int sn)
{
  SPELL_DATA *spell;
  if (sn < 0 && !txt)
    return NULL;
  if (sn < 0)
    {
      for (spell = spell_list_2[UPPER (txt[0]) % SPELL_HASH];
	   spell != NULL; spell = spell->next_a)
	{
	  if (!str_prefix (txt, spell->spell_name)
	      || !str_prefix (txt, spell->spell_funky_name))
	    return spell;
	}
      return NULL;
    }
  else
    for (spell = spell_list[sn % SPELL_HASH]; spell != NULL;
	 spell = spell->next)
      {
	if (spell->gsn == sn)
	  return spell;
      }
  return NULL;
}

void
fix_skillst (void)
{
  if ((gsn_bite = lookup_gsn ("bite")) == -1)
    {
      if ((gsn_bite = lookup_gsn ("bite")) == -1)
	{
	  SPELL_DATA *spell;
	  spell = new_spell ();
	  spell->slot = 0;
	  spell->gsn = 585;
	  gsn_bite = spell->gsn;
	  free_string (spell->spell_name);
	  spell->spell_name = str_dup ("bite");
	  free_string (spell->spell_funky_name);
	  spell->spell_funky_name = str_dup ("bite");
	}
    }

  if ((gsn_berserk = lookup_gsn ("Berserk")) == -1)
    {
      if ((gsn_berserk = lookup_gsn ("Berserk")) == -1)
	{
	  SPELL_DATA *spell;
	  spell = new_spell ();
	  spell->slot = 0;
	  spell->gsn = 580;
	  gsn_berserk = spell->gsn;
	  free_string (spell->spell_name);
	  spell->spell_name = str_dup ("Berserk");
	  free_string (spell->spell_funky_name);
	  spell->spell_funky_name = str_dup ("Berserk");
	}
    }
  if ((gsn_invest = lookup_gsn ("Investigation")) == -1)
    {
      if ((gsn_invest = lookup_gsn ("Investigate")) == -1)
	{
	  SPELL_DATA *spell;
	  spell = new_spell ();
	  spell->slot = 0;
	  spell->gsn = 578;
	  gsn_invest = spell->gsn;
	  free_string (spell->spell_name);
	  spell->spell_name = str_dup ("Investigation");
	  free_string (spell->spell_funky_name);
	  spell->spell_funky_name = str_dup ("Investigation");
	}
    }
  gsn_lance = lookup_gsn ("Lance");
  gsn_grunt = lookup_gsn ("Warcry");
  gsn_stab = lookup_gsn ("Stab");
  gsn_vbite = lookup_gsn ("Vampiric Bite");
  gsn_thrust = lookup_gsn ("Dagger Thrust");
  //gsn_charge = lookup_gsn ("Charge");
  //gsn_rush = lookup_gsn ("Shield Rush");
  //gsn_calm = lookup_gsn ("Calm");
  gsn_circle = lookup_gsn ("Circle");
  gsn_penetration = lookup_gsn ("Armor Penetration");
  gsn_swim = lookup_gsn ("Swim");
  gsn_butcher = lookup_gsn ("Butcher");
  gsn_backstab = lookup_gsn ("Backstab");
  gsn_dual_backstab = lookup_gsn ("Dual Backstab");
  gsn_dodge = lookup_gsn ("Dodge");
  gsn_hide = lookup_gsn ("Hide");
  gsn_peek = lookup_gsn ("Peek");
  gsn_pick_lock = lookup_gsn ("Lock");
  gsn_sneak = lookup_gsn ("Sneak");
  gsn_steal = lookup_gsn ("Steal");
  gsn_tackle = lookup_gsn ("Tackle");
  gsn_bash = lookup_gsn ("Bash");
  gsn_track = lookup_gsn ("Track");
  gsn_disarm = lookup_gsn ("Disarm");
  gsn_enhanced_damage = lookup_gsn ("Enhanced Dam");
  gsn_flash = lookup_gsn ("Flash");
  gsn_kick = lookup_gsn ("Kick");
  gsn_second_kick = lookup_gsn ("Second Kick");
  gsn_parry = lookup_gsn ("Parry");
  gsn_shield_block = lookup_gsn ("Shield Block");
  gsn_rescue = lookup_gsn ("Rescue");
  gsn_second_attack = lookup_gsn ("Second Attack");
  gsn_third_attack = lookup_gsn ("Third Attack");
  gsn_dual_wield = lookup_gsn ("Dual Wield");
  gsn_dual_daggers = lookup_gsn ("Dual Daggers");
  gsn_pierce = lookup_gsn ("Pierce");
  gsn_slashing = lookup_gsn ("Slash");
  gsn_concussion = lookup_gsn ("Concus");
  gsn_whip = lookup_gsn ("Whip");
  gsn_search = lookup_gsn ("Search");
  update_name_hash ();
  update_gsn_hash ();
  return;
}

void
write_spells (void)
{
  SPELL_DATA *spell;
  FILE *sfp;
  int i;
  if ((spell = skill_lookup ("Slashing", -1)) == NULL
      || spell->spell_level == -1)
    setup_skills ();
  update_name_hash ();
  update_gsn_hash ();
  if ((sfp = fopen ("spells.dat", "w")) == NULL)
    {
      fprintf (stderr, "Shit, failed on spell write!!\n");
      exit (99);
    }
  for (i = 0; i < SPELL_HASH; i++)
    {
      for (spell = spell_list[i]; spell != NULL; spell = spell->next)
	{
	  fprintf (sfp, "#SPELL\n");
	  fprintf (sfp, "Slot %d\n", spell->slot);
	  fprintf (sfp, "Gsn %d\n", spell->gsn);
	  fprintf (sfp, "SType %d\n", spell->spell_type);
	  fprintf (sfp, "SBits %d\n", spell->spell_bits);
	  fprintf (sfp, "Sname %s~\n", spell->spell_name);
	  fprintf (sfp, "SFname %s~\n", spell->spell_funky_name);
	  fprintf (sfp, "SLev %d\n", spell->spell_level);
	  if (spell->prereq_1 != NULL)
	    fprintf (sfp, "Pr1 %s~\n", spell->prereq_1);
	  if (spell->prereq_2 != NULL)
	    fprintf (sfp, "Pr2 %s~\n", spell->prereq_2);
	  fprintf (sfp, "Noun %s~\n", spell->noun_damage);
	  if (spell->act_to_ch_1 != NULL)
	    fprintf (sfp, "A2C1 %s~\n", spell->act_to_ch_1);
	  if (spell->act_to_ch_2 != NULL)
	    fprintf (sfp, "A2C2 %s~\n", spell->act_to_ch_2);
	  if (spell->act_to_vict_1 != NULL)
	    fprintf (sfp, "A2V1 %s~\n", spell->act_to_vict_1);
	  if (spell->act_to_vict_2 != NULL)
	    fprintf (sfp, "A2V2 %s~\n", spell->act_to_vict_2);
	  if (spell->act_to_notvict_1 != NULL)
	    fprintf (sfp, "A2NV1 %s~\n", spell->act_to_notvict_1);
	  if (spell->act_to_notvict_2 != NULL)
	    fprintf (sfp, "A2NV2 %s~\n", spell->act_to_notvict_2);
	  fprintf (sfp, "Locat %d\n", spell->location);


	  if (spell->linked_to != NULL)
	    {
	      fprintf (sfp, "Linkd %s~\n", spell->linked_to);
	    }
	  if (spell->duration != NULL)
	    fprintf (sfp, "Durat %s~\n", spell->duration);
	  if (spell->duration != NULL)
	    {
	      if (spell->modifier)
		fprintf (sfp, "Modif %s~\n", spell->modifier);
	      else
		fprintf (sfp, "Modif 0~\n");
	    }
	  fprintf (sfp, "BitVe %d\n", spell->bitvector);
	  fprintf (sfp, "BitT %d\n", spell->bitvector2);
	  fprintf (sfp, "BitA1 %d\n", spell->addbitvector);
	  fprintf (sfp, "BitA2 %d\n", spell->addbitvector2);
	  fprintf (sfp, "BitR1 %d\n", spell->rembitvector);
	  fprintf (sfp, "BitR2 %d\n", spell->rembitvector2);
	  if (spell->damage != NULL)
	    fprintf (sfp, "Dam %s~\n", spell->damage);
	  if (spell->long_affect_name)
	    fprintf (sfp, "Affl %s~\n", spell->long_affect_name);
	  if (spell->short_affect_name)
	    fprintf (sfp, "Affs %s~\n", spell->short_affect_name);
	  fprintf (sfp, "Mana %d\n", spell->mana);
	  fprintf (sfp, "ManaT %d\n", spell->mana_type);
	  fprintf (sfp, "CastT %d\n", spell->casting_time);
	  fprintf (sfp, "Pos %d\n", spell->position);
	  fprintf (sfp, "WearO %s~\n", spell->wear_off_msg);
          if (spell->creates_obj != 0)
	  fprintf (sfp, "CrObj %d\n", spell->creates_obj);
	  fprintf (sfp, "CrV1 %d\n", spell->values[0]);
	  fprintf (sfp, "CrV2 %d\n", spell->values[1]);
	  fprintf (sfp, "CrV3 %d\n", spell->values[2]);
	  fprintf (sfp, "CrV4 %d\n", spell->values[3]);
	  fprintf (sfp, "CrV5 %d\n", spell->values[4]);
	  fprintf (sfp, "CrV6 %d\n", spell->values[5]);
	  fprintf (sfp, "CrV7 %d\n", spell->values[6]);
	  fprintf (sfp, "CrV8 %d\n", spell->values[7]);
	  fprintf (sfp, "CrV9 %d\n", spell->values[8]);
	  fprintf (sfp, "CrV10 %d\n", spell->values[9]);
	  fprintf (sfp, "Minint %d\n", spell->min_int);
	  fprintf (sfp, "Minwis %d\n", spell->min_wis);
    // lame fix again, Eraser 12 Feb 2003
    if (spell->guildflag1 && str_cmp(spell->guildflag1, "(null)"))
          fprintf (sfp, "Guild1 %s~\n", spell->guildflag1);
    if (spell->guildflag2 && str_cmp(spell->guildflag2, "(null)"))
	  fprintf (sfp, "Guild2 %s~\n", spell->guildflag2);
    if (spell->guildflag3 && str_cmp(spell->guildflag3, "(null)"))
          fprintf (sfp, "Guild3 %s~\n", spell->guildflag3);
    if (spell->guildflag3 && str_cmp(spell->guildflag4, "(null)"))
          fprintf (sfp, "Guild4 %s~\n", spell->guildflag4);
	}
    }
  fprintf (sfp, "#END");
  fclose (sfp);
  return;
}

void
load_spells (void)
{
  SPELL_DATA *spell = NULL;
  FILE *sfp;
  char buf[500];
  bool fMatch;
  if ((sfp = fopen ("spells.dat", "r")) == NULL)
    {
      write_spells ();
      return;
    }
  for (;;)
    {
      strcpy (buf, fread_word (sfp));
      fMatch = FALSE;
      if (!str_cmp ("END", buf) || !str_cmp ("#END", buf))
	break;
      if (!str_cmp (buf, "SPELL") || !str_cmp ("#SPELL", buf))
	{
	  spell = new_spell ();
	  free_string (spell->spell_name);
	  free_string (spell->spell_funky_name);
	  free_string (spell->noun_damage);
	  free_string (spell->wear_off_msg);
	  continue;
	}
      if (spell == NULL)
	{
	  fprintf (stderr, "BAD FIRST HEADER IN SPELLS.DAT - %s!\n", buf);
	  exit (99);
	}
      KEY ("Slot", spell->slot, fread_number (sfp));
      KEY ("SType", spell->spell_type, fread_number (sfp));
      KEY ("SBits", spell->spell_bits, fread_number (sfp));
      KEY ("Gsn", spell->gsn, fread_number (sfp));
      KEY ("Sname", spell->spell_name, fread_string (sfp, NULL));
      KEY ("SFname", spell->spell_funky_name, fread_string (sfp, NULL));
      KEY ("SLev", spell->spell_level, fread_number (sfp));
      KEY ("Pr1", spell->prereq_1, fread_string (sfp, NULL));
      KEY ("Pr2", spell->prereq_2, fread_string (sfp, NULL));
      KEY ("Noun", spell->noun_damage, fread_string (sfp, NULL));
      KEY ("A2C1", spell->act_to_ch_1, fread_string (sfp, NULL));
      KEY ("A2C2", spell->act_to_ch_2, fread_string (sfp, NULL));
      KEY ("A2V1", spell->act_to_vict_1, fread_string (sfp, NULL));
      KEY ("A2V2", spell->act_to_vict_2, fread_string (sfp, NULL));
      KEY ("A2NV1", spell->act_to_notvict_1, fread_string (sfp, NULL));
      KEY ("A2NV2", spell->act_to_notvict_2, fread_string (sfp, NULL));
      KEY ("Locat", spell->location, fread_number (sfp));
      KEY ("Linkd", spell->linked_to, fread_string (sfp, NULL));
      KEY ("Durat", spell->duration, fread_string (sfp, NULL));
      KEY ("Modif", spell->modifier, fread_string (sfp, NULL));
      KEY ("BitVe", spell->bitvector, fread_number (sfp));
      KEY ("BitT", spell->bitvector2, fread_number (sfp));
      KEY ("BitA1", spell->addbitvector, fread_number (sfp));
      KEY ("BitA2", spell->addbitvector2, fread_number (sfp));
      KEY ("BitR1", spell->rembitvector, fread_number (sfp));
      KEY ("BitR2", spell->rembitvector2, fread_number (sfp));
      KEY ("Dam", spell->damage, fread_string (sfp, NULL));
      KEY ("Affl", spell->long_affect_name, fread_string (sfp, NULL));
      KEY ("Affs", spell->short_affect_name, fread_string (sfp, NULL));
      KEY ("Mana", spell->mana, fread_number (sfp));
      KEY ("ManaT", spell->mana_type, fread_number (sfp));
      KEY ("CastT", spell->casting_time, fread_number (sfp));
      KEY ("Pos", spell->position, fread_number (sfp));
      KEY ("WearO", spell->wear_off_msg, fread_string (sfp, NULL));
      KEY ("CrObj", spell->creates_obj, fread_number (sfp));
      KEY ("CrV1", spell->values[0], fread_number (sfp));
      KEY ("CrV2", spell->values[1], fread_number (sfp));
      KEY ("CrV3", spell->values[2], fread_number (sfp));
      KEY ("CrV4", spell->values[3], fread_number (sfp));
      KEY ("CrV5", spell->values[4], fread_number (sfp));
      KEY ("CrV6", spell->values[5], fread_number (sfp));
      KEY ("CrV7", spell->values[6], fread_number (sfp));
      KEY ("CrV8", spell->values[7], fread_number (sfp));
      KEY ("CrV9", spell->values[8], fread_number (sfp));
      KEY ("CrV10", spell->values[9], fread_number (sfp));
      KEY ("Minint", spell->min_int, fread_number (sfp));
      KEY ("Minwis", spell->min_wis, fread_number (sfp));
      KEY ("Guild1", spell->guildflag1, fread_string (sfp, NULL));
      KEY ("Guild2", spell->guildflag2, fread_string (sfp, NULL));
      KEY ("Guild3", spell->guildflag3, fread_string (sfp, NULL));
      KEY ("Guild4", spell->guildflag4, fread_string (sfp, NULL));
      if (!fMatch)
	{
	  fprintf (stderr, "%s\n", buf);
	  bug ("Fread_spell: no match.", 0);
	  if (!feof (sfp))
	    fread_to_eol (sfp);
	}
      if (spell->values[0] == -9)
	spell->values[0] = 0;
    }
  fclose (sfp);
  update_name_hash ();
  update_gsn_hash ();
  fix_skillst ();
  update_name_hash ();
  update_gsn_hash ();
  return;
}

int
lookup_gsn (char *nm)
{
  SPELL_DATA *spell = NULL;
  spell = skill_lookup (nm, -1);
  if (spell)
    return spell->gsn;
  else
    return -1;
}

int
tally_skills (CHAR_DATA * ch)
{
  SPELL_DATA *spell;
  int tally = 0;
  int i;
  if (IS_MOB (ch))
    return 0;
  for (i = 0; i < SKILL_COUNT; i++)
    {
      if (ch->pcdata->learned[i] > -1)
	{
	  spell = skill_lookup (NULL, i);
	  if (spell->slot == 0)
	    tally++;
	}
    }
  return tally;
}

int
tally_spells (CHAR_DATA * ch)
{
  SPELL_DATA *spell;
  int tally = 0;
  int i;
  if (IS_MOB (ch))
    return 0;
  for (i = 0; i < SKILL_COUNT; i++)
    {
      if (ch->pcdata->learned[i] > -1)
	{
	  spell = skill_lookup (NULL, i);
	  if (spell->slot != 0)
	    tally++;
	}
    }
  return tally;
}

void
setup_skills (void)
{
  SPELL_DATA *spell;

  spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 550;
  gsn_circle = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Circle");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Circle");
  free_string (spell->noun_damage);
  spell->noun_damage = str_dup ("circle");

  spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 549;
  gsn_tackle = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Tackle");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Tackle");

spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 586;
  gsn_lance = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Lance Weapons");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Lance Weapons");

spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 560;
  gsn_flash = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Flash");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Flash");

/*spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 587;
  gsn_rush = spell->gsn;
  free_string(spell->spell_name);
  spell->spell_name = str_dup ("Shield Rush");
  free_string(spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Shield Rush");
  free_string (spell->noun_damage);
  spell->noun_damage = str_dup ("Shield Rush");
*/
spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 588;
  gsn_thrust = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Dagger Thrust");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Dagger Thrust");

spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 581;
  gsn_vbite = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Vampiric Bite");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Vampiric Bite");
  free_string (spell->noun_damage);
  spell->noun_damage = str_dup ("Vampiric Bite");

/*spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 582;
  gsn_charge = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Charge");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Charge");
  free_string (spell->noun_damage);
  spell->noun_damage = str_dup ("Charge");
*/
spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 583;
  gsn_stab = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Stab");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Stab");
  free_string (spell->noun_damage);
  spell->noun_damage = str_dup ("Stab");

spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 579;
  gsn_grunt = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Warcry");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Warcry");

/*spell = new_spell();
  spell->slot = 0;
  spell->gsn = 588;
  gsn_calm = spell->gsn;
  free_string(spell->spell_name);
  spell->spell_name = str_dup ("Calm");
  free_string(spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Calm");
*/
spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 551;
  gsn_penetration = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Armor Penetration");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Armor Penetration");
  
spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 552;
  gsn_swim = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Swim");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Swim");
  
spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 553;
  gsn_butcher = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Butcher");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Butcher");
  
spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 554;
  gsn_backstab = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Backstab");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Backstab");
  free_string (spell->noun_damage);
  spell->noun_damage = str_dup ("backstab");
  
spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 555;
  gsn_dual_backstab = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Dual Backstab");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Dual Backstab");
  free_string (spell->noun_damage);
  spell->noun_damage = str_dup ("backstab");
  
spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 578;
  gsn_dodge = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Dodge");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Dodge");
  
spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 556;
  gsn_hide = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Hide");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Hide");
  
spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 557;
  gsn_peek = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Peek");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Peek");

  spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 558;
  gsn_pick_lock = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Locksmithy");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Locksmithy");

  spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 559;
  gsn_sneak = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Sneaking");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Sneaking");

  spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 561;
  gsn_bash = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Bash");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Bash");
  free_string (spell->noun_damage);
  spell->noun_damage = str_dup ("bash");

  spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 562;
  gsn_track = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Tracking");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Tracking");

  spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 563;
  gsn_disarm = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Disarm");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Disarm");

  spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 564;
  gsn_enhanced_damage = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Enhanced Damage");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Enhanced Damage");

  spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 565;
  gsn_second_kick = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Second Kick");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Second Kick");
  free_string (spell->noun_damage);
  spell->noun_damage = str_dup ("kick");
  
  spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 577;		// Watch out with the GSN.
  gsn_kick = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Kick");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Kick");
  free_string (spell->noun_damage);
  spell->noun_damage = str_dup ("kick");

  spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 566;
  gsn_parry = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Parry");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Parry");

  spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 567;
  gsn_shield_block = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Shield Block");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Shield Block");

  spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 568;
  gsn_rescue = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Rescue");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Rescue");

  spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 569;
  gsn_second_attack = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Second Attack");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Second Attack");

  spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 570;
  gsn_third_attack = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Third Attack");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Third Attack");

  spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 571;
  gsn_dual_wield = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Dual Wield");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Dual Wield");

  spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 572;
  gsn_pierce = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Piercing Weapons");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Piercing Weapons");

  spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 573;
  spell->spell_level = -1;
  gsn_slashing = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Slashing Weapons");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Slashing Weapons");

  spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 574;
  gsn_concussion = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Concussion Weapons");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Concussion Weapons");

  spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 575;
  gsn_whip = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Whipping Weapons");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Whipping Weapons");

  spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 576;
  gsn_search = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Searching");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Searching");
  update_name_hash ();
  update_gsn_hash ();
  return;

  spell = new_spell ();
  spell->slot = 0;
  spell->gsn = 584;
  gsn_dual_daggers = spell->gsn;
  free_string (spell->spell_name);
  spell->spell_name = str_dup ("Dual Daggers");
  free_string (spell->spell_funky_name);
  spell->spell_funky_name = str_dup ("Dual Daggers");

}

char *
stpos (int pos)
{
  static char retb[25];
  retb[0] = '\0';
  if (pos == POSITION_STANDING)
    {
      strcpy (retb, "STANDING");
    }
  if (pos == POSITION_RESTING)
    {
      strcpy (retb, "RESTING");
    }
  if (pos == POSITION_FIGHTING)
    {
      strcpy (retb, "FIGHTING");
    }
  return retb;
}

int
stpos_rev (char *pos)
{
  if (!str_cmp (pos, "standing"))
    return POSITION_STANDING;
  if (!str_cmp (pos, "resting"))
    return POSITION_RESTING;
  if (!str_cmp (pos, "fighting"))
    return POSITION_FIGHTING;
  return -99;
}

char *
mtype (int zmana)
{
  static char retb[50];
  retb[0] = '\0';
  if (zmana == MANA_ANY)
    {
      strcat (retb, "ANY ");
      return retb;
    }
  if (zmana & MANA_NONE)
    {
      strcat (retb, "NONE ");
    }
  if (zmana & MANA_EARTH)
    {
      strcat (retb, "EARTH ");
    }
  if (zmana & MANA_WATER)
    {
      strcat (retb, "WATER ");
    }
  if (zmana & MANA_AIR)
    {
      strcat (retb, "AIR ");
    }
  if (zmana & MANA_FIRE)
    {
      strcat (retb, "FIRE ");
    }
  if (zmana & MANA_DEATH)
    {
      strcat (retb, "DEATH ");
    }
  if (retb[0] == '\0')
    {
      strcat (retb, "ERROR");
    }
  return retb;
}

int
mtype_rev (char *smana)
{
  if (!str_cmp (smana, "any"))
    return MANA_ANY;
  if (!str_cmp (smana, "none"))
    return MANA_NONE;
  if (!str_cmp (smana, "earth"))
    return MANA_EARTH;
  if (!str_cmp (smana, "water"))
    return MANA_WATER;
  if (!str_cmp (smana, "air"))
    return MANA_AIR;
  if (!str_cmp (smana, "fire"))
    return MANA_FIRE;
  if (!str_cmp (smana, "death"))
    return MANA_DEATH;
  return -99;
}

int
spell_extra_bits_rev (char *wrd)
{
  if (!str_cmp (wrd, "summon"))
    return SPELL_SUMMON;
  if (!str_cmp (wrd, "teleport"))
    return SPELL_TELEPORT;
  if (!str_cmp (wrd, "all_in_room"))
    return SPELL_ALL_IN_ROOM;
  if (!str_cmp (wrd, "removes_bit"))
    return SPELL_REMOVES_BIT;
  if (!str_cmp (wrd, "heals_dam"))
    return SPELL_HEALS_DAM;
  if (!str_cmp (wrd, "message"))
    return SPELL_MESSAGE;
  if (!str_cmp (wrd, "dispel_magic"))
    return SPELL_DISPEL_MAGIC;
  if (!str_cmp (wrd, "hurt_evil"))
    return SPELL_HURT_EVIL;
  if (!str_cmp (wrd, "hurt_good"))
    return SPELL_HURT_GOOD;
  if (!str_cmp (wrd, "hurt_undead"))
    return SPELL_HURT_UNDEAD;
  if (!str_cmp (wrd, "modify_object"))
    return SPELL_MODIFY_OBJECT;
  if (!str_cmp (wrd, "identify"))
    return SPELL_IDENTIFY;
  if (!str_cmp (wrd, "locate_object"))
    return SPELL_LOCATE_OBJECT;
  if (!str_cmp (wrd, "scry"))
    return SPELL_SCRY;
  if (!str_cmp (wrd, "locate_person"))
    return SPELL_LOCATE_PERSON;
  if (!str_cmp (wrd, "recall"))
    return SPELL_RECALL;
  if (!str_cmp (wrd, "fail1/2"))
    return SPELL_FAIL_1_2;
  if (!str_cmp (wrd, "fail1/3"))
    return SPELL_FAIL_1_3;
  if (!str_cmp (wrd, "fail1/4"))
    return SPELL_FAIL_1_4;
  if (!str_cmp (wrd, "acid"))
    return SPELL_ACID;
  if (!str_cmp (wrd, "fireflag"))
    return SPELL_FIRE;
  if (!str_cmp (wrd, "two_saves"))
    return SPELL_TWO_SAVES;
  if (!str_cmp (wrd, "armor1/2"))
    return SPELL_ARMOR_1_2;
  if (!str_cmp (wrd, "armor1/4"))
    return SPELL_ARMOR_1_4;
  if (!str_cmp (wrd, "only_multi_mobs"))
    return SPELL_ONLY_MULTI_MOBS;
  if (!str_cmp (wrd, "dam1/2"))
    return SPELL_HALVED_SAVE;
  if (!str_cmp (wrd, "not_self"))
    return SPELL_NOT_SELF;
  if (!str_cmp (wrd, "raise_undead"))
    return SPELL_RAISE_UNDEAD;
  if (!str_cmp (wrd, "not_mob"))
    return SPELL_NOT_MOB;
  if (!str_cmp (wrd, "obj_to_room"))
    return SPELL_OBJ_TO_ROOM;
  if (!str_cmp (wrd, "add_move"))
    return SPELL_ADD_MOVE;
  if (!str_cmp (wrd, "only_group"))
    return SPELL_ONLY_GROUP;
  if (!str_cmp (wrd, "only_not_group"))
    return SPELL_ONLY_NOT_GROUP;
  return -99;
}

char *
spell_extra_bits (int bits)
{
  static char retb[200];
  retb[0] = '\0';
  if (bits & SPELL_SUMMON)
    {
      strcat (retb, "Summon ");
    }
  if (bits & SPELL_TELEPORT)
    {
      strcat (retb, "Teleport ");
    }
  if (bits & SPELL_ALL_IN_ROOM)
    {
      strcat (retb, "All_in_room ");
    }
  if (bits & SPELL_REMOVES_BIT)
    {
      strcat (retb, "Removes_bit ");
    }
  if (bits & SPELL_HEALS_DAM)
    {
      strcat (retb, "Heals_dam ");
    }
  if (bits & SPELL_MESSAGE)
    {
      strcat (retb, "Message ");
    }
  if (bits & SPELL_DISPEL_MAGIC)
    {
      strcat (retb, "Dispel_magic ");
    }
  if (bits & SPELL_HURT_EVIL)
    {
      strcat (retb, "Hurt_evil ");
    }
  if (bits & SPELL_HURT_GOOD)
    {
      strcat (retb, "Hurt_good ");
    }
  if (bits & SPELL_HURT_UNDEAD)
    {
      strcat (retb, "Hurt_undead ");
    }
  if (bits & SPELL_MODIFY_OBJECT)
    {
      strcat (retb, "Modify_object ");
    }
  if (bits & SPELL_IDENTIFY)
    {
      strcat (retb, "identify ");
    }
  if (bits & SPELL_LOCATE_OBJECT)
    {
      strcat (retb, "Locate_object ");
    }
  if (bits & SPELL_SCRY)
    {
      strcat (retb, "Scry ");
    }
  if (bits & SPELL_LOCATE_PERSON)
    {
      strcat (retb, "Locate_person ");
    }
  if (bits & SPELL_RECALL)
    {
      strcat (retb, "Recall ");
    }
  if (bits & SPELL_FAIL_1_2)
    {
      strcat (retb, "Fail1/2 ");
    }
  if (bits & SPELL_FAIL_1_3)
    {
      strcat (retb, "Fail1/3 ");
    }
  if (bits & SPELL_FAIL_1_4)
    {
      strcat (retb, "Fail1/4 ");
    }
  if (bits & SPELL_TWO_SAVES)
    {
      strcat (retb, "Two_saves ");
    }
  if (bits & SPELL_ARMOR_1_2)
    {
      strcat (retb, "Armor1/2 ");
    }
  if (bits & SPELL_ARMOR_1_4)
    {
      strcat (retb, "Armor1/4 ");
    }
  if (bits & SPELL_ONLY_MULTI_MOBS)
    {
      strcat (retb, "only_multi_mobs ");
    }
  if (bits & SPELL_HALVED_SAVE)
    {
      strcat (retb, "Dam1/2 ");
    }
  if (bits & SPELL_NOT_SELF)
    {
      strcat (retb, "Not_self ");
    }
   if (bits & SPELL_RAISE_UNDEAD)
   {
   strcat (retb, "Raise_Undead ");
   }
  if (bits & SPELL_NOT_MOB)
    {
      strcat (retb, "Not_mob ");
    }
  if (bits & SPELL_OBJ_TO_ROOM)
    {
      strcat (retb, "Obj_to_room ");
    }
  if (bits & SPELL_ADD_MOVE)
    {
      strcat (retb, "Add_move ");
    }
  if (bits & SPELL_ACID)
    {
      strcat (retb, "Acid ");
    }
  if (bits & SPELL_FIRE)
    {
      strcat (retb, "Fireflag ");
    }
  if (bits & SPELL_ONLY_GROUP)
    {
      strcat (retb, "only_group ");
    }
  if (bits & SPELL_ONLY_NOT_GROUP)
    {
      strcat (retb, "only_not_group ");
    }
  return retb;
}

char *
styp (int stype)
{
  static char retb[20];
  retb[0] = '\0';
  if (stype == TAR_IGNORE)
    {
      strcpy (retb, "EMPTY");
    }
  if (stype == TAR_CHAR_OFFENSIVE)
    {
      strcpy (retb, "OFFNS");
    }
  if (stype == TAR_CHAR_DEFENSIVE)
    {
      strcpy (retb, "DEFNS");
    }
  if (stype == TAR_CHAR_SELF)
    {
      strcpy (retb, "SELF");
    }
  if (stype == TAR_OBJ_INV)
    {
      strcpy (retb, "OBJINV");
    }
  if (stype == TAR_OBJ_ROOM) 
    {
    strcpy (retb, "OBJROOM");
    }
 return retb;
}

int
styp_rev (char *stype)
{
  if (!str_cmp (stype, "Empty"))
    return TAR_IGNORE;
  if (!str_cmp (stype, "offensive"))
    return TAR_CHAR_OFFENSIVE;
  if (!str_cmp (stype, "defensive"))
    return TAR_CHAR_DEFENSIVE;
  if (!str_cmp (stype, "self"))
    return TAR_CHAR_SELF;
  if (!str_cmp (stype, "objinv"))
    return TAR_OBJ_INV;
  if (!str_cmp (stype, "objroom"))
    return TAR_OBJ_ROOM;
  return -99;
}

void
print_profs (CHAR_DATA * ch, int flags)
{
#ifdef NEW_WORLD
  char buf[500];
  strcpy (buf, "Profs:");
  if (IS_SET (flags, PROF_HEALER))
    strcat (buf, " Healer");
  if (IS_SET (flags, PROF_WIZARD))
    strcat (buf, " Wizard");
  if (IS_SET (flags, PROF_KNIGHT))
    strcat (buf, " Knight");
  if (IS_SET (flags, PROF_BARBARIAN))
    strcat (buf, " Barbarian");
  if (IS_SET (flags, PROF_RANGER))
    strcat (buf, " Ranger");
  if (IS_SET (flags, PROF_PALADIN))
    strcat (buf, " Paladin");
  if (IS_SET (flags, PROF_DRUID))
    strcat (buf, " Druid");
  if (IS_SET (flags, PROF_ARCHMAGE))
    strcat (buf, " Archmage");
  if (IS_SET (flags, PROF_ASSASSIN))
    strcat (buf, " Assassin");
  if (IS_SET (flags, PROF_MONK))
    strcat (buf, " Monk");
  if (IS_SET (flags, PROF_BARD))
    strcat (buf, " Bard");
  if (IS_SET (flags, PROF_ATURION_HUNTER))
    strcat (buf, " Hunter");
  if (IS_SET (flags, PROF_ATURION_ENGINEER))
    strcat (buf, " Engineer");
  if (IS_SET (flags, PROF_ATURION_PHILOSOPHER))
    strcat (buf, " Philosopher");
  act (buf, ch, NULL, ch, TO_CHAR);
#endif
  return;
}

void
show_bash_skill (CHAR_DATA * ch, SPELL_DATA * spell)
{
  char buf[500];
  send_to_char (buf, ch);
  send_to_char ("\x1B[1;1f\x1B[K", ch);
  sprintf (buf, "Skill Bash (#%d).  Delay: %d rounds bashed.  Basher delay is double this.\n\r", spell->gsn, spell->casting_time);
  send_to_char (buf, ch);
  sprintf (buf, "Level: %d  MStr: %d  MCon: %d  (Min str/con to practice this skill fully).\n\r", spell->spell_level, spell->min_int, spell->min_wis);
  send_to_char (buf, ch);
  print_profs (ch, spell->values[0]);
  send_to_char ("\x1B[K----------------------------------------------------------------------------", ch);
  sprintf (buf, "\x1B[%d;1f", ch->pcdata->pagelen);
  send_to_char (buf, ch);
  return;
}

void
show_gen_skill (CHAR_DATA * ch, SPELL_DATA * spell)
{
  char buf[500];
  sprintf (buf, "\x1B[1;1f\x1B[K\x1B[2;1f\x1B[K\x1B[3;1f\x1B[K\x1B[4;1f\x1B[K\x1B[5;1f\x1B[K\x1B[6;1f\x1B[K\x1B[7;1f\x1B[K\x1B[8;1f\x1B[K\x1B[9;1f\x1B[K\x1B[10;1f\x1B[K\x1B[11;1f\x1B[K\x1B[12;1f\x1B[K\x1B[14;%dr",
	   ch->pcdata->pagelen);
  send_to_char (buf, ch);

  send_to_char ("\x1B[1;1f\x1B[K", ch);

  if (spell->gsn == gsn_backstab || spell->gsn == gsn_circle || spell->gsn == gsn_stab /*|| spell->gsn == gsn_charge*/ || spell->gsn == gsn_vbite
     || spell->gsn == gsn_disarm || spell->gsn == gsn_kick || spell->gsn == gsn_berserk || spell->gsn == gsn_penetration 
     || spell->gsn == gsn_grunt || spell->gsn == gsn_dual_backstab /*|| spell->gsn == gsn_rush || spell->gsn == gsn_calm*/ || spell->gsn == gsn_disarm
     || spell->gsn == gsn_enhanced_damage || spell->gsn == gsn_second_kick || spell->gsn == gsn_shield_block  || spell->gsn == gsn_third_attack 
     || spell->gsn == gsn_second_attack || spell->gsn == gsn_dual_wield || spell->gsn == gsn_flash || spell->gsn == gsn_dual_daggers
     || spell->gsn == gsn_sneak || spell->gsn == gsn_parry || spell->gsn == gsn_dodge || spell->gsn == gsn_thrust || spell->gsn == gsn_pick_lock)
    {
sprintf (buf, "\x1B[0;37mSkill \x1B[1;31m%s \x1B[0;37m# \x1B[1;31m%d \x1B[0;37mLvl: \x1B[1;31m%d \x1B[0;37mDelay (Post): \x1B[1;31m%d  \x1B[0;37mMoves=(Pre-%s delay): \x1B[1;31m%d\n\r",
	     spell->spell_name, spell->gsn, spell->spell_level, spell->casting_time, spell->spell_name, spell->mana);
send_to_char (buf, ch);

sprintf (buf, "\x1B[0;37mGuild1 \x1B[1;31m%s \x1B[0;37mGuild2 \x1B[1;31m%s \x1B[0;37mGuild3 \x1B[1;31m%s \x1B[0;37mGuild4 \x1B[1;31m%s",
               spell->guildflag1 ? spell->guildflag1 : "",
               spell->guildflag2 ? spell->guildflag2 : "",
               spell->guildflag3 ? spell->guildflag3 : "",
               spell->guildflag4 ? spell->guildflag4 : "");
   }
  else
    sprintf (buf, "Skill %s (#%d).  Delay: %d.  Moves: %d.\n\r",
	     spell->spell_name, spell->gsn, spell->casting_time, spell->mana);

  send_to_char (buf, ch);
/*    {
      sprintf (buf, "Damg: %s.\n\r",
	       (spell->damage == NULL ? "N/A" : spell->damage));
      send_to_char (buf, ch);
    }*/
  print_profs (ch, spell->values[0]);
  send_to_char ("\n\r\x1B[K----------------------------------------------------------------------------", ch);
  sprintf (buf, "\x1B[%d;1f", ch->pcdata->pagelen);
  send_to_char (buf, ch);
  return;
}


/*void
show_grunt_skill (CHAR_DATA * ch, SPELL_DATA * spell)
{
  char buf[500];
  sprintf (buf,
	   "\x1B[1;1f\x1B[K\x1B[2;1f\x1B[K\x1B[3;1f\x1B[K\x1B[4;1f\x1B[K\x1B[5;1f\x1B[K\x1B[6;1f\x1B[K\x1B[7;1f\x1B[K\x1B[8;1f\x1B[K\x1B[9;1f\x1B[K\x1B[10;1f\x1B[K\x1B[11;1f\x1B[K\x1B[12;1f\x1B[K\x1B[14;%dr",
	   ch->pcdata->pagelen);
  send_to_char (buf, ch);
  send_to_char ("\x1B[1;1f\x1B[K", ch);
  sprintf (buf, "Skill Grunt (#%d)  Moves required: %d  Level: %d\n\r",
	   spell->gsn, spell->mana, spell->spell_level);
  send_to_char (buf, ch);
  print_profs (ch, spell->values[0]);
  send_to_char
    ("\x1B[K----------------------------------------------------------------------------",
     ch);
  sprintf (buf, "\x1B[%d;1f", ch->pcdata->pagelen);
  send_to_char (buf, ch);
  return;
}*/

void
show_weap_skill (CHAR_DATA * ch, SPELL_DATA * spell)
{
  char buf[500];
  sprintf (buf,
	   "\x1B[1;1f\x1B[K\x1B[2;1f\x1B[K\x1B[3;1f\x1B[K\x1B[4;1f\x1B[K\x1B[5;1f\x1B[K\x1B[6;1f\x1B[K\x1B[7;1f\x1B[K\x1B[8;1f\x1B[K\x1B[9;1f\x1B[K\x1B[10;1f\x1B[K\x1B[11;1f\x1B[K\x1B[12;1f\x1B[K\x1B[14;%dr",
	   ch->pcdata->pagelen);
  send_to_char (buf, ch);
  send_to_char ("\x1B[1;1f\x1B[K", ch);
  sprintf (buf, "Skill %s (#%d)  Level: %d\n\r",
	   spell->spell_name, spell->gsn, spell->spell_level);
  send_to_char (buf, ch);
  print_profs (ch, spell->values[0]);
  send_to_char
    ("\x1B[K----------------------------------------------------------------------------",
     ch);
  sprintf (buf, "\x1B[%d;1f", ch->pcdata->pagelen);
  send_to_char (buf, ch);
  return;
}


void
show_spell (CHAR_DATA * ch, SPELL_DATA * spell, int row)
{
  char buf[500];
  if (spell->gsn == gsn_bash)
    {
      show_bash_skill (ch, spell);
      return;
    }

/*  if (spell->gsn == gsn_grunt)
    {
      show_grunt_skill (ch, spell);
      return;
    }*/

  /*if (spell->gsn == gsn_headbutt || spell->gsn == gsn_bodyslam ||
      spell->gsn == gsn_stab || spell->gsn == gsn_backstab ||
      spell->gsn == gsn_circle)*/
if (spell->gsn == gsn_backstab || spell->gsn == gsn_circle || spell->gsn == gsn_stab /*|| spell->gsn == gsn_charge*/ || spell->gsn == gsn_vbite
     || spell->gsn == gsn_disarm || spell->gsn == gsn_kick || spell->gsn == gsn_berserk || spell->gsn == gsn_penetration
     || spell->gsn == gsn_grunt || spell->gsn == gsn_dual_backstab /*|| spell->gsn == gsn_rush || spell->gsn == gsn_calm*/ || spell->gsn == gsn_disarm
     || spell->gsn == gsn_enhanced_damage || spell->gsn == gsn_second_kick || spell->gsn == gsn_shield_block  || spell->gsn == gsn_third_attack
     || spell->gsn == gsn_second_attack || spell->gsn == gsn_dual_wield || spell->gsn == gsn_flash || spell->gsn == gsn_sneak 
     || spell->gsn == gsn_parry || spell->gsn == gsn_dodge || spell->gsn == gsn_pick_lock || spell->gsn == gsn_thrust || spell->gsn == gsn_tackle) 
     /*|| spell->gsn == gsn_)*/
    {
      show_gen_skill (ch, spell);
      return;
    }

  if (spell->gsn >= 549)
    {
      show_weap_skill (ch, spell);
      return;
    }


  /*Name: Animate Revenant Lvl: 110 Mana: 1 ManaType: FIRE  Pos: RESTING Time: 10 Spel: 199/208
    Type: OBJROOM ndam: Spell  Name2: Animate Revenant Mint: 0 Mwis: 0
    Pre1: Animate Skeleton Pre2: Animate Zombie Creates: 803 Val: 0 -9 -9 -9 -9 -9 -9 -9 -9 -9
    Lctn:  None Duration: N/A Modifier: None Bit:   Bit2:  
    Damg: N/A
    Slot: 1 Wear_off: The spell has worn off.
    TO_CHAR1     Your spell has risen an revenant from the $R$4de$B$0a$R$4d$R$7!
    TO_CHAR2      
    TO_VICT1      
    TO_VICT2      
    TO_NOTVICT1   $n has just risen an ethereal being from the dead!
    TO_NOTVICT2   
    Extra_bits..: Raise_Undead   Linked:  
    Guild1 necrog  Guild2   Guild3 Guild4 
    Addbitvector: None    Rembitvector: None  
    Affect long: None
    Affect short: None
   * ----------------------------------------------------------------------------- 
   */
  if (row == -1)
    {
      sprintf (buf,
	       
"\x1B[1;1f\x1B[K\x1B[2;1f\x1B[K\x1B[3;1f\x1B[K\x1B[4;1f\x1B[K\x1B[5;1f\x1B[K\x1B[6;1f\x1B[K\x1B[7;1f\x1B[K\x1B[8;1f\x1B[K\x1B[9;1f\x1B[K\x1B[10;1f\x1B[K\x1B[11;1f\x1B[K\x1B[12;1f\x1B[K\x1B[13;1f\x1B[K\x1B[14;1f\x1B[K\x1B[15;1f\x1B[K\x1B[16;1f\x1B[K\x1B[17;1f\x1B[K\x1B[18;%dr",
	       ch->pcdata->pagelen);
      send_to_char (buf, ch);
    }
  if (row == -1 || row == 1)
    {
      send_to_char ("\x1B[1;1f\x1B[K", ch);
      sprintf (buf,
"\x1B[0;37mName: \x1B[1;31m%s \x1B[0;37mLvl: \x1B[1;31m%d \x1B[0;37mMana: \x1B[1;31m%d \x1B[0;37mManaType: \x1B[1;34m%s \x1B[0;37mPos: \x1B[1;31m%s \x1B[0;37mTime: \x1B[1;31m%d \x1B[0;37mSpel: \x1B[1;31m%d\x1B[0;37m/\x1B[1;31m%d",
	       spell->spell_name, spell->spell_level,spell->mana,
	       mtype (spell->mana_type), stpos (spell->position),
	       spell->casting_time, spell->gsn, top_spell);
      send_to_char (buf, ch);
    }
  if (row == -1 || row == 2)
    {
      send_to_char ("\x1B[2;1f\x1B[K", ch);
      sprintf (buf, 
"\x1B[0;37mType: \x1B[1;31m%s \x1B[0;37mndam: \x1B[1;31m%s  \x1B[0;37mName2: \x1B[1;31m%s \x1B[0;37mMint: \x1B[1;31m%d \x1B[0;37mMwis: \x1B[1;31m%d",
	       styp (spell->spell_type),
	       (spell->noun_damage == NULL ? "N/S" : spell->noun_damage),
	       spell->spell_funky_name, spell->min_int, spell->min_wis);
      send_to_char (buf, ch);
    }
  if (row == -1 || row == 3)
    {
      send_to_char ("\x1B[3;1f\x1B[K", ch);
      sprintf (buf, 
"\x1B[0;37mPre1: \x1B[1;31m%s \x1B[0;37mPre2: \x1B[1;31m%s \x1B[0;37mCreates: \x1B[1;31m%d \x1B[0;37mVal: \x1B[1;31m%d %d %d %d %d %d %d %d %d %d",
	       (spell->prereq_1 == NULL ? "None" : spell->prereq_1),
	       (spell->prereq_2 == NULL ? "None" : spell->prereq_2),
	       spell->creates_obj, spell->values[0], spell->values[1],
	       spell->values[2], spell->values[3], spell->values[4],
	       spell->values[5], spell->values[6], spell->values[7],
	       spell->values[8], spell->values[9]);
      send_to_char (buf, ch);
    }
  if (row == -1 || row == 4)
    {
      send_to_char ("\x1B[4;1f\x1B[K", ch);
      sprintf (buf, 
"\x1B[0;37mLctn:  \x1B[1;31m%s \x1B[0;37mDuration: \x1B[1;31m%s \x1B[0;37mModifier: \x1B[1;31m%s \x1B[0;37mBit: \x1B[1;31m%s \x1B[0;37mBit2: \x1B[1;31m%s",
	       capitalize (affect_loc_name (spell->location)),
	       (spell->duration == NULL ? "N/A" : spell->duration),
	       (spell->modifier == NULL ? "None" : spell->modifier),
	       affect_bit_name (spell->bitvector),
	       affect_bit_name_two (spell->bitvector2));
      send_to_char (buf, ch);
    }
  if (row == -1 || row == 5)
    {
      send_to_char ("\x1B[5;1f\x1B[K", ch);
      sprintf (buf, "\x1B[0;37mDamg: \x1B[1;31m%s",
	       (spell->damage == NULL ? "N/A" : spell->damage));
      send_to_char (buf, ch);
    }
  if (row == -1 || row == 6)
    {
      send_to_char ("\x1B[6;1f\x1B[K", ch);
      sprintf (buf, "\x1B[0;37mSlot: \x1B[1;31m%d \x1B[0;37mWear_off: \x1B[37;0m%s",
      spell->slot,
               (spell->wear_off_msg == NULL ? "None" : spell->wear_off_msg));
      send_to_char (buf, ch);
    }
  if (row == -1 || row == 7)
    {
      send_to_char ("\x1B[7;1f\x1B[K", ch);
      sprintf (buf, "\x1B[0;37mTO_CHAR1     \x1B[0;37m%s",
	       (spell->act_to_ch_1 == NULL ? "" : spell->act_to_ch_1));
      send_to_char (buf, ch);
    }
  if (row == -1 || row == 8)
    {
      send_to_char ("\x1B[8;1f\x1B[K", ch);
      sprintf (buf, "\x1B[0;37mTO_CHAR2      \x1B[0;37m%s",
	       (spell->act_to_ch_2 == NULL ? "" : spell->act_to_ch_2));
      send_to_char (buf, ch);
    }
  if (row == -1 || row == 9)
    {
      send_to_char ("\x1B[9;1f\x1B[K", ch);
      sprintf (buf, "\x1B[0;37mTO_VICT1      \x1B[0;37m%s",
	       (spell->act_to_vict_1 == NULL ? "" : spell->act_to_vict_1));
      send_to_char (buf, ch);
    }
  if (row == -1 || row == 10)
    {
      send_to_char ("\x1B[10;1f\x1B[K", ch);
      sprintf (buf, "\x1B[0;37mTO_VICT2      \x1B[0;37m%s",
	       (spell->act_to_vict_2 == NULL ? "" : spell->act_to_vict_2));
      send_to_char (buf, ch);
    }
  if (row == -1 || row == 11)
    {
      send_to_char ("\x1B[11;1f\x1B[K", ch);
      sprintf (buf, "\x1B[0;37mTO_NOTVICT1   \x1B[0;37m%s",
	       (spell->act_to_notvict_1 ==
		NULL ? "" : spell->act_to_notvict_1));
      send_to_char (buf, ch);
    }
  if (row == -1 || row == 12)
    {
      send_to_char ("\x1B[12;1f\x1B[K", ch);
      sprintf (buf, "\x1B[0;37mTO_NOTVICT2   \x1B[0;37m%s",
	       (spell->act_to_notvict_2 ==
		NULL ? "" : spell->act_to_notvict_2));
      send_to_char (buf, ch);
    }
  if (row == -1 || row == 13)
    {
      send_to_char ("\x1B[13;1f\x1B[K", ch);
      sprintf (buf,
	       "\x1B[0;37mExtra_bits..: \x1B[1;31m%s  \x1B[0;37mLinked: \x1B[1;31m%s",
	       spell_extra_bits (spell->spell_bits),
	       (spell->linked_to == NULL ? " " : spell->linked_to));
         
      send_to_char (buf, ch);
    }
  if (row == -1 || row == 14)
    {
      send_to_char ("\x1B[14;1f\x1B[K", ch);
      sprintf (buf, "\x1B[0;37mGuild1 \x1B[1;31m%s \x1B[0;37mGuild2 \x1B[1;31m%s \x1B[0;37mGuild3 \x1B[1;31m%s \x1B[0;37mGuild4 \x1B[1;31m%s",
               spell->guildflag1 ? spell->guildflag1 : "",
               spell->guildflag2 ? spell->guildflag2 : "",
               spell->guildflag3 ? spell->guildflag3 : "",
               spell->guildflag4 ? spell->guildflag4 : "");
      
      send_to_char (buf, ch);
    }
  if (row == -1 || row == 15)
    {
      send_to_char ("\x1B[15;1f\x1B[K", ch);
      sprintf (buf, "\x1B[0;37mAddbitvector: \x1B[1;31m%s %s",
	       (!spell->addbitvector && !spell->addbitvector2) ? "None" :
	       affect_bit_name (spell->addbitvector),
	       affect_bit_name_two (spell->addbitvector2));
      send_to_char (buf, ch);
      sprintf (buf, "  \x1B[0;37mRembitvector: \x1B[1;31m%s %s\n\r",
	       (!spell->rembitvector && !spell->rembitvector2) ? "None" :
	       affect_bit_name (spell->rembitvector),
	       affect_bit_name_two (spell->rembitvector2));
      send_to_char (buf, ch);
    }
  if (row == -1 || row == 16)
    {
      send_to_char ("\x1B[16;1f\x1B[K", ch);
      sprintf (buf, "\x1B[0;37mAffect long: %s\n\r",
	       ((!spell->long_affect_name || !str_cmp (spell->long_affect_name, "(null)")) ? "None" : spell->long_affect_name));
      send_to_char (buf, ch);
    }
  if (row == -1 || row == 17)
    {
      send_to_char ("\x1B[17;1f\x1B[K", ch);
      sprintf (buf, "\x1B[0;37mAffect short: %s\n\r",
	       ((!spell->short_affect_name || !str_cmp (spell->short_affect_name, "(null)")) ? "None" : spell->short_affect_name));
      send_to_char (buf, ch);
    }
  if (row == -1)
    {
      send_to_char("\x1B[0;37m----------------------------------------------------------------------------\x1B[37;0m\n\r",ch);
    }
  sprintf (buf, "\x1B[%d;1f", ch->pcdata->pagelen);
  send_to_char (buf, ch);
  return;
}

void
do_sedit (CHAR_DATA * ch, char *argy)
{
  char arg1[500];
  int value;
  SPELL_DATA *spell;
  DEFINE_COMMAND ("sedit", do_sedit, POSITION_DEAD, 111, LOG_NORMAL, "The spell editor... use with caution!") if (IS_MOB (ch))
    return;
  argy = one_argy (argy, arg1);
  if (is_number (arg1))
    {
      value = atoi (arg1);
      if ((spell = skill_lookup (NULL, value)) == NULL)
	{
	  send_to_char ("That spell number was not found.\n\r", ch);
	  return;
	}
      /*
       * if (spell->slot==0) { send_to_char("That is a reserved skill,
       * sorry.\n\r",ch); return; } 
       */
      ch->desc->pEdit = (void *) spell;
      ch->desc->connected = CON_SEDITOR;
      show_spell (ch, spell, -1);
      return;
    }

  if (!str_cmp (arg1, "create"))
    {
      spell = new_spell ();
      changed_spells = TRUE;
      ch->desc->pEdit = (void *) spell;
      ch->desc->connected = CON_SEDITOR;
      update_name_hash ();
      update_gsn_hash ();
      show_spell (ch, spell, -1);
      return;
    }
  if (arg1[0] != '\0' && (spell = skill_lookup (arg1, -1)) != NULL)
    {
      /*
       * if (spell->slot==0) { send_to_char("That is a reserved skill,
       * sorry.\n\r",ch); return; } 
       */
      ch->desc->pEdit = (void *) spell;
      ch->desc->connected = CON_SEDITOR;
      show_spell (ch, spell, -1);
      return;
    }
  send_to_char ("Syntax: SEDIT #, SEDIT 'Spell_name', or SEDIT CREATE.\n\r", ch);
  return;
}

void
sedit (CHAR_DATA * ch, char *argy)
{
  char *orarg = argy;
  char arg1[500];
  char arg2[500];
  bool fnn;
  int bit = 0;
  SPELL_DATA *spell;
  spell = (SPELL_DATA *) ch->desc->pEdit;
  if (!spell)
    {
      interpret (ch, argy);
      return;
    }
  arg1[0] = '\0';
  arg2[0] = '\0';
  argy = one_argy (argy, arg1);
  if (arg1[0] == '\0')
    {
      show_spell (ch, spell, -1);
      return;
    }
  if (!str_cmp (arg1, "done"))
    {
      ch->desc->pEdit = NULL;
      ch->desc->connected = CON_PLAYING;
      sprintf (arg1, "\x1B[1;%dr\x1B[2J\x1B[%d;1f", ch->pcdata->pagelen, ch->pcdata->pagelen);
      send_to_char (arg1, ch);
      return;
    }
  S_HEALER (arg1, bit)
    S_WIZARD (arg1, bit)
    S_KNIGHT (arg1, bit)
    S_BARBARIAN (arg1, bit)
    S_RANGER (arg1, bit)
    S_PALADIN (arg1, bit)
    S_DRUID (arg1, bit)
    S_ARCHMAGE (arg1, bit)
    S_ASSASSIN (arg1, bit)
    S_MONK (arg1, bit)
    S_BARD (arg1, bit)
    S_HUNTER (arg1, bit)
    S_ENGINEER (arg1, bit) S_PHILOSOPHER (arg1, bit) if (bit != 0)
    {
      spell->values[0] ^= bit;
      changed_spells = TRUE;
      show_spell (ch, spell, 3);
      show_spell (ch, spell, 13);
      return;
    }
  if (!str_cmp (arg1, "value0") && is_number (argy))
    {
      int i;
      if ((i = atoi (argy)) < 0)
	return;
      spell->values[0] = i;
      changed_spells = TRUE;
      show_spell (ch, spell, 3);
      return;
    }
  if (!str_cmp (arg1, "affshort"))
    {
      if (argy == NULL || argy[0] == '\0')
	{
	  free (spell->short_affect_name);
	  spell->short_affect_name = NULL;
	  changed_spells = TRUE;
	  show_spell (ch, spell, -1);
	  return;
	}
      free (spell->short_affect_name);
      spell->short_affect_name = strdup (argy);
      changed_spells = TRUE;
      show_spell (ch, spell, -1);
      return;
    }
  if (!str_cmp (arg1, "afflong"))
    {
      if (argy == NULL || argy[0] == '\0')
	{
	  free (spell->long_affect_name);
	  spell->long_affect_name = NULL;
	  changed_spells = TRUE;
	  show_spell (ch, spell, -1);
	  return;
	}
      free (spell->long_affect_name);
      spell->long_affect_name = strdup (argy);
      changed_spells = TRUE;
      show_spell (ch, spell, -1);
      return;
    }
  if (!str_cmp (arg1, "name"))
    {
      if (argy == NULL || argy[0] == '\0' || argy == "")
	{
	  send_to_char ("Syntax: Name <spell_name>.\n\r", ch);
	  return;
	}
      free_string (spell->spell_name);
      spell->spell_name = str_dup (argy);
      changed_spells = TRUE;
      update_name_hash ();
      show_spell (ch, spell, 2);
      return;
    }
  if (!str_cmp (arg1, "name2"))
    {
      if (argy == NULL || argy[0] == '\0' || argy == "")
	{
	  send_to_char ("Syntax: Name2 <spell_name_2>.\n\r", ch);
	  return;
	}
      free_string (spell->spell_funky_name);
      spell->spell_funky_name = str_dup (argy);
      changed_spells = TRUE;
      show_spell (ch, spell, 2);
      return;
    }
  if (!str_cmp (arg1, "ndam"))
    {
      if (argy == NULL || argy[0] == '\0' || argy == "")
	{
	  send_to_char ("Syntax: Ndam <word>.\n\r", ch);
	  return;
	}
      if (spell->noun_damage != NULL)
	free_string (spell->noun_damage);
      spell->noun_damage = str_dup (argy);
      changed_spells = TRUE;
      show_spell (ch, spell, 2);
      return;
    }
  if (!str_prefix ("link", arg1))
    {
      if (argy == NULL || argy[0] == '\0' || argy == "")
	{
	  if (spell->linked_to != NULL)
	    free_string (spell->linked_to);
	  spell->linked_to = NULL;
	  send_to_char ("Linked_to cleared.\n\r", ch);
	  return;
	}
      if (spell->linked_to != NULL)
	free_string (spell->linked_to);
      spell->linked_to = str_dup (argy);
      changed_spells = TRUE;
      show_spell (ch, spell, 12);
      return;
    }
  if (!str_cmp (arg1, "damg"))
    {
      if (argy == NULL || argy[0] == '\0' || argy == "")
	{
	  send_to_char ("Syntax: Damg <string>.\n\r", ch);
	  return;
	}
      if (spell->damage != NULL)
	free_string (spell->damage);
      spell->damage = str_dup (argy);
      changed_spells = TRUE;
      show_spell (ch, spell, 5);
      return;
    }
  if (!str_cmp (arg1, "wear_off"))
    {
      if (argy == NULL || argy[0] == '\0' || argy == "")
	{
	  send_to_char ("Syntax: wear_off <string>.\n\r", ch);
	  return;
	}
      if (spell->wear_off_msg != NULL)
	free_string (spell->wear_off_msg);
      spell->wear_off_msg = str_dup (argy);
      changed_spells = TRUE;
      show_spell (ch, spell, 5);
      return;
    }
  if (!str_cmp (arg1, "duration") || !str_cmp (arg1, "dur"))
    {
      if (argy == NULL || argy[0] == '\0' || argy == "")
	{
	  send_to_char ("Syntax: duration <duration val>.\n\r", ch);
	  return;
	}
      if (spell->duration != NULL)
	free_string (spell->duration);
      spell->duration = str_dup (argy);
      changed_spells = TRUE;
      show_spell (ch, spell, 4);
      return;
    }
  if (!str_cmp (arg1, "to_char1") || !str_cmp (arg1, "char1"))
    {
      if (argy == NULL || argy[0] == '\0' || argy == "")
	{
	  if (spell->act_to_ch_1 != NULL)
	    free_string (spell->act_to_ch_1);
	  spell->act_to_ch_1 = NULL;
	  return;
	}
      if (spell->act_to_ch_1 != NULL)
	free_string (spell->act_to_ch_1);
      spell->act_to_ch_1 = str_dup (argy);
      changed_spells = TRUE;
      show_spell (ch, spell, 6);
      return;
    }
  if (!str_cmp (arg1, "to_char2") || !str_cmp (arg1, "char2"))
    {
      if (argy == NULL || argy[0] == '\0' || argy == "")
	{
	  if (spell->act_to_ch_2 != NULL)
	    free_string (spell->act_to_ch_2);
	  spell->act_to_ch_2 = NULL;
	  return;
	}
      if (spell->act_to_ch_2 != NULL)
	free_string (spell->act_to_ch_2);
      changed_spells = TRUE;
      spell->act_to_ch_2 = str_dup (argy);
      show_spell (ch, spell, 7);
      return;
    }
  if (!str_cmp (arg1, "to_vict1") || !str_cmp (arg1, "vict1"))
    {
      if (argy == NULL || argy[0] == '\0' || argy == "")
	{
	  if (spell->act_to_vict_1 != NULL)
	    free_string (spell->act_to_vict_1);
	  spell->act_to_vict_1 = NULL;
	  return;
	}
      if (spell->act_to_vict_1 != NULL)
	free_string (spell->act_to_vict_1);
      spell->act_to_vict_1 = str_dup (argy);
      changed_spells = TRUE;
      show_spell (ch, spell, 8);
      return;
    }
  if (!str_cmp (arg1, "to_vict2") || !str_cmp (arg1, "vict2"))
    {
      if (argy == NULL || argy[0] == '\0' || argy == "")
	{
	  if (spell->act_to_vict_2 != NULL)
	    free_string (spell->act_to_vict_2);
	  spell->act_to_vict_2 = NULL;
	  return;
	}
      if (spell->act_to_vict_2 != NULL)
	free_string (spell->act_to_vict_2);
      spell->act_to_vict_2 = str_dup (argy);
      changed_spells = TRUE;
      show_spell (ch, spell, 9);
      return;
    }
  if (!str_cmp (arg1, "to_notvict1") || !str_cmp (arg1, "notvict1"))
    {
      if (argy == NULL || argy[0] == '\0' || argy == "")
	{
	  if (spell->act_to_notvict_1 != NULL)
	    free_string (spell->act_to_notvict_1);
	  spell->act_to_notvict_1 = NULL;
	  return;
	}
      if (spell->act_to_notvict_1 != NULL)
	free_string (spell->act_to_notvict_1);
      spell->act_to_notvict_1 = str_dup (argy);
      changed_spells = TRUE;
      show_spell (ch, spell, 10);
      return;
    }
  if (!str_cmp (arg1, "to_notvict2") || !str_cmp (arg1, "notvict2"))
    {
      if (argy == NULL || argy[0] == '\0' || argy == "")
	{
	  if (spell->act_to_notvict_2 != NULL)
	    free_string (spell->act_to_notvict_2);
	  spell->act_to_notvict_2 = NULL;
	  return;
	}
      if (spell->act_to_notvict_2 != NULL)
	free_string (spell->act_to_notvict_2);
      spell->act_to_notvict_2 = str_dup (argy);
      changed_spells = TRUE;
      show_spell (ch, spell, 11);
      return;
    }
  if (!str_cmp (arg1, "guild4"))
  {
    if (argy == NULL || argy[0] == '\0')
    {
      if (spell->guildflag4 != NULL)
        free_string (spell->guildflag4);
      spell->guildflag4 = NULL;
      return;
    }
    if (spell->guildflag4 != NULL)
      free_string (spell->guildflag4);
    spell->guildflag4 = str_dup (argy);
    changed_spells = TRUE;
    show_spell (ch, spell, 3);
    return;
  }
  if (!str_cmp (arg1, "guild3"))
  {
    if (argy == NULL || argy[0] == '\0')
    {
      if (spell->guildflag3 != NULL)
        free_string (spell->guildflag3);
      spell->guildflag3 = NULL;
      return;
    }
    if (spell->guildflag3 != NULL)
      free_string (spell->guildflag3);
    spell->guildflag3 = str_dup (argy);
    changed_spells = TRUE;
    show_spell (ch, spell, 3);
    return;
  }
  if (!str_cmp (arg1, "guild2"))
  {  
    if (argy == NULL || argy[0] == '\0')
    {
      if (spell->guildflag2 != NULL)    
        free_string (spell->guildflag2);  
      spell->guildflag2 = NULL;  
      return;
    }
    if (spell->guildflag2 != NULL)
      free_string (spell->guildflag2); 
    spell->guildflag2 = str_dup (argy);
    changed_spells = TRUE;    
    show_spell (ch, spell, 3);  
    return;
  }
  if (!str_cmp (arg1, "guild1"))
  {
    if (argy == NULL || argy[0] == '\0')
    {
      if (spell->guildflag1 != NULL)
        free_string (spell->guildflag1);
      spell->guildflag1 = NULL;
      return;
    }
    if (spell->guildflag1 != NULL)
      free_string (spell->guildflag1);
    spell->guildflag1 = str_dup (argy);
    changed_spells = TRUE;
    show_spell (ch, spell, 3);
    return;
  }

  if (!str_cmp (arg1, "pre1"))
    {
      if (argy == NULL || argy[0] == '\0' || argy == "")
	{
	  if (spell->prereq_1 != NULL)
	    free_string (spell->prereq_1);
	  spell->prereq_1 = NULL;
	  return;
	}
      if (spell->prereq_1 != NULL)
	free_string (spell->prereq_1);
      spell->prereq_1 = str_dup (argy);
      changed_spells = TRUE;
      show_spell (ch, spell, 3);
      return;
    }
  if (!str_cmp (arg1, "pre2"))
    {
      if (argy == NULL || argy[0] == '\0' || argy == "")
	{
	  if (spell->prereq_2 != NULL)
	    free_string (spell->prereq_2);
	  spell->prereq_2 = NULL;
	  return;
	}
      if (spell->prereq_2 != NULL)
	free_string (spell->prereq_2);
      spell->prereq_2 = str_dup (argy);
      changed_spells = TRUE;
      show_spell (ch, spell, 3);
      return;
    }
  if (!str_cmp (arg1, "modifier") || !str_cmp (arg1, "mod"))
    {
      if (argy == NULL || argy[0] == '\0' || argy == "")
	{
	  send_to_char ("Syntax: modifier <modifier val>.\n\r", ch);
	  return;
	}
      if (spell->modifier != NULL)
	free_string (spell->modifier);
      spell->modifier = str_dup (argy);
      changed_spells = TRUE;
      show_spell (ch, spell, 4);
      return;
    }
  if ((!str_cmp (arg1, "mwis") || !str_cmp (arg1, "mcon")) && is_number (argy))
    {
      int ii;
      ii = atoi (argy);
      spell->min_wis = ii;
      changed_spells = TRUE;
      show_spell (ch, spell, 2);
      return;
    }
  if ((!str_cmp (arg1, "mint") || !str_cmp (arg1, "mstr")) && is_number (argy))
    {
      int ii;
      ii = atoi (argy);
      spell->min_int = ii;
      changed_spells = TRUE;
      show_spell (ch, spell, 1);
      return;
    }
  if ((!str_cmp (arg1, "mana") || !str_cmp (arg1, "moves")) && is_number (argy))
    {
      int ii;
      ii = atoi (argy);
      spell->mana = ii;
      changed_spells = TRUE;
      show_spell (ch, spell, 1);
      return;
    }
  if (!str_cmp (arg1, "lvl") && is_number (argy))
    {
      int ii;
      ii = atoi (argy);
      spell->spell_level = ii;
      changed_spells = TRUE;
      show_spell (ch, spell, 1);
      return;
    }
  if ((!str_cmp (arg1, "time") || !str_cmp (arg1, "delay"))
      && is_number (argy))
    {
      int ii;
      ii = atoi (argy);
      spell->casting_time = ii;
      changed_spells = TRUE;
      show_spell (ch, spell, 1);
      return;
    }
  if ((!str_cmp (arg1, "creates_obj") || !str_cmp (arg1, "creates")) && is_number (argy))
    {
      int ii;
      ii = atoi (argy);
      spell->creates_obj = ii;
      changed_spells = TRUE;
      return;
    }
  if (!str_cmp (arg1, "slot") && is_number (argy))
    {
      int ii;
      ii = atoi (argy);
      if (ii == 0)
	{
	  send_to_char ("You can not set slot number to 0.\n\r", ch);
	  return;
	}
      spell->slot = ii;
      changed_spells = TRUE;
      show_spell (ch, spell, 5);
      return;
    }
  fnn = FALSE;
  if (stpos_rev (arg1) != -99)
    {
      spell->position = stpos_rev (arg1);
      changed_spells = TRUE;
      show_spell (ch, spell, 1);
      fnn = TRUE;
    }
  if (mtype_rev (arg1) != -99)
    {
      spell->mana_type ^= mtype_rev (arg1);
      changed_spells = TRUE;
      show_spell (ch, spell, 1);
      fnn = TRUE;
    }
  if (styp_rev (arg1) != -99)
    {
      spell->spell_type = styp_rev (arg1);
      changed_spells = TRUE;
      show_spell (ch, spell, 2);
      fnn = TRUE;
    }
  if (spell_extra_bits_rev (arg1) != -99)
    {
      spell->spell_bits ^= spell_extra_bits_rev (arg1);
      changed_spells = TRUE;
      show_spell (ch, spell, 12);
      fnn = TRUE;
    }
  if (affect_name_bit (arg1) != 0)
    {
      spell->bitvector ^= affect_name_bit (arg1);
      changed_spells = TRUE;
      show_spell (ch, spell, 4);
      fnn = TRUE;
    }
  if (affect_name_bit_two (arg1) != 0)
    {
      spell->bitvector2 ^= affect_name_bit_two (arg1);
      changed_spells = TRUE;
      show_spell (ch, spell, 4);
      fnn = TRUE;
    }
  if (!str_cmp (arg1, "addbit") && affect_name_bit (argy))
    {
      spell->addbitvector ^= affect_name_bit (argy);
      changed_spells = TRUE;
      show_spell (ch, spell, 14);
      fnn = TRUE;
    }
  if (!str_cmp (arg1, "addbit") && affect_name_bit_two (argy))
    {
      spell->addbitvector2 ^= affect_name_bit_two (argy);
      changed_spells = TRUE;
      show_spell (ch, spell, 14);
      fnn = TRUE;
    }
  if (!str_cmp (arg1, "rembit") && affect_name_bit (argy))
    {
      spell->rembitvector ^= affect_name_bit (argy);
      changed_spells = TRUE;
      show_spell (ch, spell, 14);
      fnn = TRUE;
    }
  if (!str_cmp (arg1, "rembit") && affect_name_bit_two (argy))
    {
      spell->rembitvector2 ^= affect_name_bit_two (argy);
      changed_spells = TRUE;
      show_spell (ch, spell, 14);
      fnn = TRUE;
    }
  if (affect_name_loc (arg1) != -99)
    {
      spell->location = affect_name_loc (arg1);
      changed_spells = TRUE;
      show_spell (ch, spell, 4);
      fnn = TRUE;
    }
  if (!fnn)
    interpret (ch, orarg);
  return;
}

SPELL_DATA *
new_spell (void)
{
  SPELL_DATA *spell;
  spell = mem_alloc (sizeof (*spell));
  bzero (spell, sizeof (*spell));
  top_spell++;
  spell->slot = 1;
  spell->linked_to = NULL;
  spell->noun_damage = str_dup ("Spell");
  spell->gsn = top_spell;
  spell->min_int = 0;
  spell->min_wis = 0;
  spell->spell_type = 0;
  spell->spell_bits = 0;
  spell->spell_name = str_dup ("Newspell");
  spell->spell_funky_name = str_dup ("NewSpell");
  spell->long_affect_name = NULL;
  spell->short_affect_name = NULL;
  spell->spell_level = 0;
  spell->prereq_1 = NULL;
  spell->prereq_2 = NULL;
  spell->act_to_ch_1 = NULL;
  spell->act_to_ch_2 = NULL;
  spell->act_to_vict_1 = NULL;
  spell->act_to_vict_2 = NULL;
  spell->act_to_notvict_1 = NULL;
  spell->act_to_notvict_2 = NULL;
  spell->location = 0;
  spell->duration = NULL;
  spell->modifier = NULL;
  spell->bitvector = 0;
  spell->addbitvector = 0;
  spell->addbitvector2 = 0;
  spell->rembitvector = 0;
  spell->rembitvector2 = 0;
  spell->damage = NULL;
  spell->mana = 1;
  spell->mana_type = 0;
  spell->casting_time = 10;
  spell->position = POSITION_RESTING;
  spell->wear_off_msg = str_dup ("The spell has worn off.");
  spell->creates_obj = 0;
  spell->values[0] = 0;
  spell->values[1] = -9;
  spell->values[2] = -9;
  spell->values[3] = -9;
  spell->values[4] = -9;
  spell->values[5] = -9;
  spell->values[6] = -9;
  spell->values[7] = -9;
  spell->values[8] = -9;
  spell->values[9] = -9;
  spell->guildflag1 = NULL;
  spell->guildflag2 = NULL;
  spell->guildflag3 = NULL;
  spell->guildflag4 = NULL;
  spell->next = spell_list[spell->gsn % SPELL_HASH];
  spell_list[spell->gsn % SPELL_HASH] = spell;
  spell->next_a = spell_list_2[UPPER (spell->spell_funky_name[0]) % SPELL_HASH];
  spell_list_2[UPPER (spell->spell_funky_name[0]) % SPELL_HASH] = spell;
  return spell;
}

/*
 * Below is my translator. Variables accepted: l = level i = char's
 * intelligence w = char's wisdom a = random from 1-10 b = random from
 * 10-20 c = random from 20-30 d = random from 1-level m = Syntax m42
 * would be max of 42 damage (numeric only) z = Syntax z3 would be minimum 
 * 3 damage (numeric only) k = kick damage g = Wizard's Guild then 1, else 
 * 0 h = Healer's Guild then 1, else 0 o = Warrior's Guild then 1, else 0
 * t = max hit points... used for mob exp calc. please note: all
 * operations are performed from left to right. This makes for simple
 * coding, and a damn quick pass thru a complicated expression. sample:
 * l+w/3 would be written in real terms as: l/3 + w/3 or (l+w)/3
 * i+w+l*2/3+l would be written in real terms as: (((i+w)+l)*2)/3)+l You
 * can now use parens :) 
 */
char *pos;
long
translate (char *line, int level, CHAR_DATA * ch)
{
  long dam = 0;
  long cur_num = 0;
  char *tp;
  char cur_op = '+';
  long ctt;
  if (!line)
    return 0;
  pos = line;
  for (pos = line; *pos != '\0'; pos++)
    {
      if (is_digit (*pos))
	{
	  cur_num *= 10;
	  cur_num += (*pos - '0');
	  continue;
	}
      if (*pos == ')')
	{
	  if (cur_op == '*')
	    dam *= cur_num;
	  if (cur_op == '/')
	    dam /= cur_num;
	  if (cur_op == '+')
	    dam += cur_num;
	  if (cur_op == '-')
	    dam -= cur_num;
	  if (cur_op == '\0')
	    dam = cur_num;
	  return dam;
	}
      if (*pos == '(')
	{
	  cur_num = translate (pos + 1, level, ch);
	  continue;
	}
      if (*pos == 't')
	{
	  cur_num = ch->max_hit;
	}
      if (*pos == 'g')
	{
	  cur_num = is_member (ch, GUILD_WIZARD) ? 1 : 0;
	}
      if (*pos == 'h')
	{
	  cur_num = is_member (ch, GUILD_HEALER) ? 1 : 0;
	}
      if (*pos == 'o')
	{
	  cur_num = is_member (ch, GUILD_WARRIOR) ? 1 : 0;
	}
      if (*pos == 'u')
        {
          cur_num = is_member (ch, GUILD_MARAUDER) ? 1 : 0;
        }
      if (*pos == 'f')
        {
          cur_num = is_member (ch, GUILD_THIEFG) ? 1 : 0;
        }
      if (*pos == 'e')
        {
          cur_num = is_member (ch, GUILD_ELEMENTAL) ? 1 : 0;
        }
      if (*pos == 'p')
        {
          cur_num = is_member (ch, GUILD_PALADIN) ? 1 : 0;
        }
      if (*pos == 'n')
        {
          cur_num = is_member (ch, GUILD_NECROMANCER) ? 1 : 0;
        }
      if (*pos == 'j')
        {
          cur_num = is_member (ch, GUILD_BATTLEMAGE) ? 1 : 0;
        }
      if (*pos == 'z')
	{
	  int tnn = 0;
	  if (cur_op == '*')
	    dam *= cur_num;
	  if (cur_op == '/')
	    dam /= cur_num;
	  if (cur_op == '+')
	    dam += cur_num;
	  if (cur_op == '-')
	    dam -= cur_num;
	  if (cur_op == '\0')
	    dam = cur_num;
	  cur_op = '[';
	  cur_num = 0;
	  ctt = 0;
	  for (tp = pos + 1; *tp != '\0' && is_digit ((*tp)); tp++)
	    {
	      ctt++;
	      tnn *= 10;
	      tnn += (*tp - '0');
	    }
	  if (dam < tnn)
	    {
	      dam = tnn;
	      cur_num = tnn;
	    }
	  pos += ctt;
	  continue;
	}
      if (*pos == 'm')
	{
	  int tnn = 0;
	  if (cur_op == '*')
	    dam *= cur_num;
	  if (cur_op == '/')
	    dam /= cur_num;
	  if (cur_op == '+')
	    dam += cur_num;
	  if (cur_op == '-')
	    dam -= cur_num;
	  if (cur_op == '\0')
	    dam = cur_num;
	  cur_op = '[';
	  cur_num = 0;
	  ctt = 0;
	  for (tp = pos + 1; *tp != '\0' && is_digit ((*tp)); tp++)
	    {
	      ctt++;
	      tnn *= 10;
	      tnn += (*tp - '0');
	    }
	  if (dam > tnn)
	    {
	      dam = tnn;
	      cur_num = tnn;
	    }
	  pos += ctt;
	  continue;
	}
      if (*pos == '+')
	{
	  if (cur_op == '*')
	    dam *= cur_num;
	  if (cur_op == '/')
	    dam /= cur_num;
	  if (cur_op == '+')
	    dam += cur_num;
	  if (cur_op == '-')
	    dam -= cur_num;
	  if (cur_op == '\0')
	    dam = cur_num;
	  cur_op = '+';
	  cur_num = 0;
	}
      if (*pos == '-')
	{
	  if (cur_op == '*')
	    dam *= cur_num;
	  if (cur_op == '/')
	    dam /= cur_num;
	  if (cur_op == '+')
	    dam += cur_num;
	  if (cur_op == '-')
	    dam -= cur_num;
	  if (cur_op == '\0')
	    dam = cur_num;
	  cur_op = '-';
	  cur_num = 0;
	}
      if (*pos == '/')
	{
	  if (cur_op == '*')
	    dam *= cur_num;
	  if (cur_op == '/')
	    dam /= cur_num;
	  if (cur_op == '+')
	    dam += cur_num;
	  if (cur_op == '-')
	    dam -= cur_num;
	  if (cur_op == '\0')
	    dam = cur_num;
	  cur_op = '/';
	  cur_num = 0;
	}
      if (*pos == '*')
	{
	  if (cur_op == '*')
	    dam *= cur_num;
	  if (cur_op == '/')
	    dam /= cur_num;
	  if (cur_op == '+')
	    dam += cur_num;
	  if (cur_op == '-')
	    dam -= cur_num;
	  if (cur_op == '\0')
	    dam = cur_num;
	  cur_op = '*';
	  cur_num = 0;
	}
      if (*pos == 'v')
        cur_num = ch->pcdata->plus_heal;
      if (*pos == 'r')
        cur_num = ch->pcdata->plus_magic;
      if (*pos == 'd')
	cur_num = number_range (12, 48);
      if (*pos == 'k')
	cur_num = ch->pcdata->plus_kick;
      if (*pos == 'l')
	cur_num = level;
      if (*pos == 'w' && ch)
	cur_num = get_curr_wis (ch);
      if (*pos == 'i' && ch)
	cur_num = get_curr_int (ch);
      if (*pos == 's' && ch)
	cur_num = get_curr_str (ch);
      if (*pos == 'x' && ch)
	cur_num = get_curr_con (ch);
      if (*pos == 'y' && ch)
	cur_num = get_curr_dex (ch);
      if (*pos == 'a')
	cur_num = number_range (1, 10);
      if (*pos == 'b')
	cur_num = number_range (10, 20);
      if (*pos == 'c')
	cur_num = number_range (20, 30);
    }
  if (cur_op == '*')
    dam *= cur_num;
  if (cur_op == '/')
    dam /= cur_num;
  if (cur_op == '+')
    dam += cur_num;
  if (cur_op == '-')
    dam -= cur_num;
  if (cur_op == '\0')
    dam = cur_num;
  return dam;
}
