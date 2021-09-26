#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <malloc.h>

#ifndef WINDOWS
#include <sys/errno.h>
#include <signal.h>
#endif


#include "emlen.h"
extern int _filbuf (FILE *);

#define MAX_NESTING 100
static SINGLE_OBJECT *rgObjNest[MAX_NESTING];
void conv_race (CHAR_DATA * ch);
void fwrite_char (CHAR_DATA * ch, FILE * fp);
void fwrite_storage_objs (CHAR_DATA * ch, FILE * fp);
void fwrite_obj (CHAR_DATA * ch, SINGLE_OBJECT * obj, FILE * fp, int iNest);
void fread_char (CHAR_DATA * ch, FILE * fp);
void fread_obj (CHAR_DATA * ch, FILE * fp, int storage);
void fwrite_char_tro (CHAR_DATA * ch, FILE * fp);
void fread_tro (CHAR_DATA * ch);

void
natural_mana (CHAR_DATA * ch)
{
  if (IS_MOB (ch))
    return;
  ch->pcdata->n_max_mana = get_perm_stat (ch, INT_I) + 2 + get_perm_stat (ch, WIS_I) + ((((LEVEL (ch) / 3) * ((get_curr_int (ch)+10) + 
(get_curr_wis (ch)+10))) / 10) + (ch->pcdata->remort_times * 3)) + ch->pcdata->remort[REMORT_MANA] * REMORT_MANA_AMOUNT;
  return;
}

void
write_fixed_values (SINGLE_OBJECT * obj, FILE * fp)
{
  int val[10];
  int i;
  for (i = 0; i < 10; i++)
    val[i] = obj->pIndexData->value[i];
  if (obj->pIndexData->item_type == ITEM_REPELLANT)
  {
    I_SPRAY *sp = (I_SPRAY *) obj->more;
    val[0] = sp->sprays;
  }
  else if (obj->pIndexData->item_type == ITEM_ARMOR)
  {
    I_ARMOR *arm = (I_ARMOR *) obj->more;
    val[7] = arm->condition_now;
    val[6] = arm->max_condition;
  }
  else if (obj->pIndexData->item_type == ITEM_WEAPON)
  {
    I_WEAPON *wep = (I_WEAPON *) obj->more;
    val[0] = wep->damage_p;
    val[1] = wep->strength;
  }
  else if (obj->pIndexData->item_type == ITEM_DRINK_CON)
  {
    I_DRINK *dr = (I_DRINK *) obj->more;
    val[0] = dr->liquid_now;
    val[2] = dr->liquid_type;
    val[3] = dr->not_poison;
  }
  else if (obj->pIndexData->item_type == ITEM_LIGHT)
  {
    I_LIGHT *lg = (I_LIGHT *) obj->more;
    val[3] = lg->light_lit;
    val[0] = lg->light_now;
  }
  else if (obj->pIndexData->item_type == ITEM_TOOL)
  {
    I_TOOL *tl = (I_TOOL *) obj->more;
    val[1] = tl->uses;
  }
  else if (obj->pIndexData->item_type == ITEM_GEM)
  {
    I_GEM *gem = (I_GEM *) obj->more;
    val[1] = gem->mana_now;
  }
  else if (obj->pIndexData->item_type == ITEM_WAND)
  {
    I_WAND *wnd = (I_WAND *) obj->more;
    val[2] = wnd->current_charges;
  }
  else if (obj->pIndexData->item_type == ITEM_CONTAINER)
  {
    I_CONTAINER *cnt = (I_CONTAINER *) obj->more;
    val[1] = cnt->flags;
    val[8] = cnt->money;
  }
  fprintf (fp, "%d %d %d %d %d %d %d %d %d %d ",
      val[0], val[1], val[2], val[3],
      val[4], val[5], val[6], val[7], val[8], val[9]);
  return;
}

void
conv_height (CHAR_DATA * ch)
{
  if (ch->height > 0)
    return;			/*Only find heights for those who have -1 and 0 height */
  if (IS_PLAYER (ch))		/*If the player is a character */
  {
    ch->height =
      number_range (race_info[ch->pcdata->race].height_min,
          race_info[ch->pcdata->race].height_max);
    return;
  }
  switch (ch->pIndexData->mobtype)
  {
    case MOB_HUMAN:
      ch->height = number_range (53, 81);
      break;
    case MOB_ELF:
      ch->height = number_range (58, 88);
      break;
    case MOB_DWARF:
      ch->height = number_range (41, 66);
      break;
    case MOB_GNOME:
      ch->height = number_range (38, 58);
      break;
    case MOB_GIANT:
      ch->height = number_range (86, 150);
      break;
    case MOB_DRAGON:
      ch->height = number_range (178, 408);
      break;
    case MOB_CANINE:
      ch->height = number_range (23, 48);
      break;
    case MOB_RODENT:
      ch->height = number_range (12, 30);
      break;
    case MOB_INSECT:
      ch->height = number_range (12, 30);
      break;
    case MOB_UNDEAD:
      ch->height = number_range (47, 89);
      break;
    case MOB_GOLEM:
      ch->height = number_range (77, 145);
      break;
    case MOB_DEMON:
      ch->height = number_range (60, 95);
      break;
    case MOB_TROLL:
      ch->height = number_range (65, 120);
      break;
    case MOB_BIRD:
      ch->height = number_range (20, 40);
      break;
    case MOB_FISH:
      ch->height = number_range (12, 38);
      break;
    case MOB_STATUE:
      ch->height = number_range (50, 100);
      break;
    case MOB_FELINE:
      ch->height = number_range (23, 48);
      break;
    case MOB_PLANT:
      ch->height = number_range (60, 150);
      break;
    case MOB_GENERAL_ANIMAL:
      ch->height = number_range (14, 35);
      break;
    case MOB_FAERIE:
      ch->height = number_range (6, 20);
      break;
    case MOB_REPTILE:
      ch->height = number_range (8, 30);
      break;
    case MOB_GHOST:
      ch->height = number_range (53, 80);
      break;
    case MOB_OTHER:
      ch->height = number_range (53, 81);
      break;
    case MOB_LYCAN:
      ch->height = number_range (50, 80);
      break;
    case MOB_NIGHT:
      ch->height = number_range (50, 80);
      break;
    case MOB_ORC:
      ch->height = number_range (52, 71);
      break;
    case MOB_DARKELF:
      ch->height = number_range (61, 88);
      break;
    case MOB_DUMMY:
      ch->height = number_range (51, 70);
      break;
    case MAX_MOB_TYPE:
      ch->height = number_range (51, 75);
      break;
  }
  return;
}

void
conv_race (CHAR_DATA * ch)
{
  if (IS_MOB (ch))
    return;
  ch->pcdata->raceflag = 0;
  ch->pcdata->raceflag = (1 << ch->pcdata->race);
  /*if (ch->race==0) ch->pcdata->raceflag=RC_1;
    if (ch->race==1) ch->pcdata->raceflag=RC_2;
    if (ch->race==2) ch->pcdata->raceflag=RC_3;
    if (ch->race==3) ch->pcdata->raceflag=RC_4;
    if (ch->race==4) ch->pcdata->raceflag=RC_5;
    if (ch->race==5) ch->pcdata->raceflag=RC_6;
    if (ch->race==6) ch->pcdata->raceflag=RC_7;
    if (ch->race==7) ch->pcdata->raceflag=RC_8;
    if (ch->race==8) ch->pcdata->raceflag=RC_9;
    if (ch->race==9) ch->pcdata->raceflag=RC_10;
    if (ch->race==10) ch->pcdata->raceflag=RC_11;
    if (ch->race==11) ch->pcdata->raceflag=RC_12;
    if (ch->race==12) ch->pcdata->raceflag=RC_13;
    if (ch->race==13) ch->pcdata->raceflag=RC_14;
    if (ch->race==14) ch->pcdata->raceflag=RC_15;
    if (ch->race==15) ch->pcdata->raceflag=RC_16;
    if (ch->race==16) ch->pcdata->raceflag=RC_17;
    if (ch->race==17) ch->pcdata->raceflag=RC_18;
    if (ch->race==18) ch->pcdata->raceflag=RC_19;
    if (ch->race==19) ch->pcdata->raceflag=RC_20;
    if (ch->race==20) ch->pcdata->raceflag=RC_21;
    if (ch->race==21) ch->pcdata->raceflag=RC_22;
    if (ch->race==22) ch->pcdata->raceflag=RC_23;
    if (ch->race==23) ch->pcdata->raceflag=RC_24;
    if (ch->race==24) ch->pcdata->raceflag=RC_25;
   */
  if (!IS_EVIL (ch))
  {
    ch->pcdata->raceflag += RC_GOODRACE;
  }
  else
    ch->pcdata->raceflag += RC_EVILRACE;
  return;
}

void
load_corpse (CHAR_DATA * real_ch)
{
  char strsave[SML_LENGTH];
  CHAR_DATA *ch;
  FILE *fp;
  bool found;
  char buffy[500];
  SINGLE_OBJECT *corpsecheck;
  /*Scan world if corpse already exists */
  sprintf (buffy, "corpse of %s", NAME (real_ch));
  for (corpsecheck = object_list; corpsecheck;
      corpsecheck = corpsecheck->next)
  {
    if (!str_suffix (buffy, STR (corpsecheck, short_descr)) ||
        !str_suffix (STR (corpsecheck, short_descr), buffy))
      return;
  }
  /*Okay, if it existed, function would have returned */
  found = FALSE;
#ifndef WINDOWS
  //  fclose (fpReserve);
#endif
  sprintf (strsave, "%s%s.cor", PLAYER_DIR,
      capitalize (real_ch->pcdata->name));
  if ((fp = fopen (strsave, "r")) == NULL)
  {
    sprintf (strsave, "%s%s.cor", PLAYER_DIR,
        capitalize (real_ch->pcdata->name));
  }
  else
  {
    fclose (fp);
  }
  if ((fp = fopen (strsave, "r")) == NULL)
    return;
  else
  {
    int iNest;
    ch = new_char ();
    ch->pcdata = new_pc ();
    ch->desc = NULL;
    free_string (ch->pcdata->name);
    ch->pcdata->name = str_dup (NAME (real_ch));
    for (iNest = 0; iNest < MAX_NESTING; iNest++)
      rgObjNest[iNest] = NULL;
    found = TRUE;
    for (;;)
    {
      char letter;
      char *word;
      letter = fread_letter (fp);
      if (letter == '*')
      {
        fread_to_eol (fp);
        continue;
      }
      if (letter != '#')
      {
        bug ("Load_char_obj: # not found.", 0);
        break;
      }
      word = fread_word (fp);
      if (!str_cmp (word, "PLAYER"))
        fread_char (ch, fp);
      else if (!str_cmp (word, "OBJECT"))
        fread_obj (ch, fp, 0);
      else if (!str_cmp (word, "END"))
        break;
      else
      {
        bug ("Load_char_obj: bad section.", 0);
        break;
      }
    }
    fclose (fp);
  }
#ifndef WINDOWS
  //  fpReserve = fopen (NULL_FILE, "r");
#endif
  char_to_room (ch, ch->pcdata->temp_room);
  make_corpse (ch);
  extract_char (ch, TRUE);
  return;
}

void
save_corpse (CHAR_DATA * ch)
{
  char strsave[SML_LENGTH];
  FILE *fp;
  if (IN_BATTLE (ch))
    return;
  if (IS_MOB (ch) || LEVEL (ch) < 1)
    return;
#ifndef WINDOWS
  //  fclose (fpReserve);
#endif
  sprintf (strsave, "%s%s.cor", PLAYER_DIR, capitalize (ch->pcdata->name));
  if ((fp = fopen (strsave, "w")) == NULL)
  {
    bug ("Save_char_obj: fopen", 0);
    perror (strsave);
  }
  else
  {
    fwrite_char (ch, fp);
    if (ch->carrying != NULL)
      fwrite_obj (ch, ch->carrying, fp, 0);
    fprintf (fp, "#END\n");
  }
  fclose (fp);
#ifndef WINDOWS
  //  fpReserve = fopen (NULL_FILE, "r");
#endif
  return;
}

void
save_char_tro (CHAR_DATA * ch)
{
  char strsave[SML_LENGTH];
  FILE *fp;
  if (IN_BATTLE (ch) == TRUE)
    return;
  if (ch->in_room && ch->in_room->vnum > 449 && ch->in_room->vnum < 500)
    return;
  if (IS_MOB (ch) || LEVEL (ch) < 0)
    return;
  if (ch->desc != NULL && ch->desc->original != NULL)
    ch = ch->desc->original;
#ifndef WINDOWS
  //  fclose (fpReserve);
#endif
  sprintf (strsave, "%s%s.tro", PLAYER_DIR, capitalize (ch->pcdata->name));
  if ((fp = fopen (strsave, "w")) == NULL)
  {
    bug ("Save_char_obj: fopen", 0);
    perror (strsave);
  }
  else
  {
    fwrite_char_tro (ch, fp);
    if (ch->carrying != NULL)
      fprintf (fp, "#END\n");
  }
  fclose (fp);

#ifndef WINDOWS
  //  fpReserve = fopen (NULL_FILE, "r");
#endif
  return;
}


void
fwrite_char_tro (CHAR_DATA * ch, FILE * fp)
{
  int k;
  check_ced (ch);
  check_fgt (ch);
#ifdef NEW_WORLD
  for (k = 0; k < MAX_MOB_TRO; k++)
  {
    if (ch->pcdata->killed_mobs[k] != 0)
      fprintf (fp, "Nk %d %d\n", k, ch->pcdata->killed_mobs[k]);
  }
  fprintf (fp, "End\n\n");
#endif
  return;
}

CHAR_DATA *chCorrupt = NULL;
FILE *f = NULL;

void segfault_handler ();

static void
segfault_save_handler ()
{
  CHAR_DATA *ch = chCorrupt;
  char strsave[SML_LENGTH];
  char szBuf[512];
  FILE *fp;
  fclose (f);
  sprintf (szBuf, "%s%s.bak", PLAYER_DIR, capitalize (ch->pcdata->name));
  sprintf (strsave, "%s%s", PLAYER_DIR, capitalize (ch->pcdata->name));
  if ((fp = fopen (szBuf, "r")) != NULL)
  {
    FILE *fp2;
    if ((fp2 = fopen (strsave, "w")) != NULL)
    {
      int a;
      while (!feof (fp))
      {
        a = fgetc (fp);
        fputc (a, fp2);
      }
      fclose (fp2);
    }
    fclose (fp);
  }

  fprintf (stderr, "FUCK!  Caught corruption of pfile %s!  Restored from .bak\n", ch->pcdata->name);
  exit (0);
}

void
save_char_obj (CHAR_DATA * ch)
{
  char strsave[SML_LENGTH];
  FILE *fp;
  chCorrupt = ch;
#ifndef WINDOWS
  //signal (SIGSEGV, segfault_save_handler);
#endif

  if (IN_BATTLE (ch) == TRUE)
    return;
  if (ch->in_room && ch->in_room->vnum > 449 && ch->in_room->vnum < 500)
    return;
  if (IS_MOB (ch) || LEVEL (ch) < 0)
    return;
  if (ch->desc != NULL && ch->desc->original != NULL)
    ch = ch->desc->original;
#ifndef WINDOWS
  //  fclose (fpReserve);
#endif
  sprintf (strsave, "%s%s", PLAYER_DIR, capitalize (ch->pcdata->name));
  if ((fp = fopen (strsave, "r")) != NULL)
  {
    FILE *fp2;
    char szBuf[512];
    sprintf (szBuf, "%s%s.bak", PLAYER_DIR, capitalize (ch->pcdata->name));
    if ((fp2 = fopen (szBuf, "w")) != NULL)
    {
      int a;
      while (!feof (fp))
      {
        a = fgetc (fp);
        fputc (a, fp2);
      }
      fclose (fp2);
    }
    fclose (fp);
  }
  if ((fp = fopen (strsave, "w")) == NULL)
  {
    bug ("Save_char_obj: fopen", 0);
    perror (strsave);
  }
  else
  {
    f = fp;
    fwrite_char (ch, fp);
    if (ch->carrying != NULL)
      fwrite_obj (ch, ch->carrying, fp, 0);
    fwrite_storage_objs (ch, fp);
    fprintf (fp, "#END\n");
  }
  f = NULL;
  fclose (fp);
#ifndef WINDOWS
  //  fpReserve = fopen (NULL_FILE, "r");
  //signal (SIGSEGV, segfault_handler);
#endif

  chCorrupt = NULL;
  return;
}


void
fwrite_char (CHAR_DATA * ch, FILE * fp)
{
  AFFECT_DATA *paf;
  CHANNEL *c;
  int i = 0;
  int pos;
  int checksum;
  int sn, k;
  ROOM_DATA *tr;
  char sss[500];
  SPELL_DATA *spell;
  check_ced (ch);
  check_fgt (ch);
  tr = ch->in_room;

  ch->pcdata->played += (int) (current_time - ch->pcdata->logon);
#ifdef BILLING
  if (ch->pcdata->monthly_pass <= 0)
    ch->pcdata->hours_purchased -= (int) (current_time - ch->pcdata->logon);
#endif
  ch->pcdata->logon = current_time;

  if (check_hours (ch) == FALSE)
  {
    send_to_char
      ("Your credit and grace period have expired.  If you wish to continue playing\n\r",
       ch);
    send_to_char
      ("this character, please consult the billing information.\n\r", ch);
    ch->pcdata->hours_expired = TRUE;
  }

  ch->pcdata->new_exp = TRUE;
  checksum = 0;
  checksum += ch->pcdata->level;
  checksum -= ch->pcdata->warpoints;
  checksum -= ch->pcdata->bank;
  checksum += ch->gold;
  sprintf (sss, "%s%s", PLAYER_DIR, capitalize (NAME (ch)));
  fprintf (fp, "#%s\n", IS_MOB (ch) ? "MOB" : "PLAYER");
  if (ch->pcdata->email[0] != '\0')
    fprintf (fp, "NEmail %s~\n", fix_string (ch->pcdata->email));

  if (ch->pcdata->short_descr && ch->pcdata->short_descr[0] != '\0')
    fprintf (fp, "ShDesc %s~\n", ch->pcdata->short_descr);
  if (ch->pcdata->long_descr && ch->pcdata->long_descr[0] != '\0')
    fprintf (fp, "LoDesc %s~\n", fix_string (ch->pcdata->long_descr));
  fprintf (fp, "HMp %d\n", ch->pcdata->monthly_pass);
  fprintf (fp, "HMpp %d\n", ch->pcdata->monthly_expired);
  fprintf (fp, "Hpc %d\n", ch->pcdata->hours_purchased);
  fprintf (fp, "HEx %d\n", ch->pcdata->hours_expired);
  fprintf (fp, "Descr %s~\n", fix_string (ch->pcdata->description));
  fprintf (fp, "Sex %d\n", SEX (ch));
fprintf (fp, "MHp %d\n", ch->pcdata->hp_mort);
  fprintf (fp, "MMv %d\n", ch->pcdata->mv_mort);
  fprintf (fp, "MMa %d\n", ch->pcdata->ma_mort);
  fprintf (fp, "MDm %d\n", ch->pcdata->dam_mort);

  for (c = chan_first; c != NULL; c = c->next)
  {
    fprintf (fp, "LCcol %d %s\n", i, ch->pcdata->chan_color[i]);
    i++;
  }
  fprintf (fp, "LCign %d\n", ch->pcdata->chan_ignore);

  fprintf (fp, "TtlKils %d\n", ch->pcdata->totalkills);
  fprintf (fp, "NAn %d\n", ch->pcdata->show_lev);
  fprintf (fp, "Killpt %d\n", ch->pcdata->killpoints);
  fprintf (fp, "Deaths %d\n", ch->pcdata->deaths);
  fprintf (fp, "DPray %d\n", ch->pcdata->pray_points);
  fprintf (fp, "DProf %d\n", ch->pcdata->profession_flag);
  if (ch->in_room && tr)
  {
    for (k = 0; k < 20; k++)
    {
      if (ch->pcdata->pet_temp[k] != 0)
      {
        fprintf (fp, "APet %d %d %d\n", ch->pcdata->pet_temp[k],
            ch->pcdata->pet_hps[k], ch->pcdata->pet_move[k]);
      }
    }
  }
  fprintf (fp, "DCoins %d %d %d %d %d\n", 0, 0, ch->gold, 0, ch->copper);

  for (k = 0; k < 3000; k++)
  {
    if (ch->pcdata->script_flags[k]
        && ch->pcdata->script_flags[k][0] != '\0')
    {
      fprintf (fp, "Sf %s~\n", ch->pcdata->script_flags[k]);
    }
  }
  for (k = 0; k < 1000; k++)
  {
    if (ch->pcdata->in_progress[k] != 0)
      fprintf (fp, "NIp %d %d\n", k, ch->pcdata->in_progress[k]);
  }
  for (k = 0; k < 1000; k++)
  {
    if (ch->pcdata->complete[k] != 0)
      fprintf (fp, "NCm %d %d\n", k, ch->pcdata->complete[k]);
  }
  for (k = 0; k < 200; k++)
  {
    if (ch->pcdata->temp_flag[k] != 0)
      fprintf (fp, "NTf %d %d\n", k, ch->pcdata->temp_flag[k]);
  }
#ifdef NEW_WORLD
  for (k = 0; k < MAX_MOB_TRO; k++)
  {
    if (ch->pcdata->killed_mobs[k] != 0)
      fprintf (fp, "Nk %d %d\n", k, ch->pcdata->killed_mobs[k]);
  }
#endif
  fprintf (fp, "Nexp %d\n", ch->pcdata->new_exp);
  fprintf (fp, "Nospam %d\n", ch->pcdata->no_spam);
  if (ch->height != -1)
    fprintf (fp, "NHeight %d\n", ch->height);
  fprintf (fp, "NDon %d\n", ch->pcdata->donated);
  fprintf (fp, "NTps %d\n", ch->pcdata->tps);
  fprintf (fp, "NQuest %d\n", ch->pcdata->questflag);
  fprintf (fp, "NatArm %d\n", ch->pcdata->nat_armor);
  fprintf (fp, "NatAbil %d\n", ch->pcdata->nat_abilities);
  fprintf (fp, "NatFlags %d\n", checksum);
  if (ch->ced && ch->ced->wasroom != 0)
  {
    fprintf (fp, "NWasrm %d\n", ch->ced->wasroom);
  }
  if (ch->pcdata->challenge_on)
    fprintf (fp, "NChon %d\n", ch->pcdata->challenge_on);
  if (ch->pcdata->remort_times)
  {
    short zi;
    fprintf (fp, "NRemort %d", ch->pcdata->remort_times);
    for (zi = 0; zi < MAX_REMORT_OPTIONS; zi++)
      fprintf (fp, " %d", ch->pcdata->remort[zi]);
    fprintf (fp, "\n");
  }

  fprintf (fp, "NAMsg %d\n", ch->pcdata->arena_msg);
  if (ch->fgt->challenge)
    fprintf (fp, "NChal %d\n", ch->fgt->challenge);
  fprintf (fp, "NGldBt %d\n", ch->pcdata->guilds);
  // following added 23 Feb 2003 by Eraser
  fprintf (fp, "GuildStats %d %d %d %d\n",
      ch->pcdata->guildstats[0], ch->pcdata->guildstats[1],
      ch->pcdata->guildstats[2], ch->pcdata->guildstats[3]); 
  fprintf (fp, "NWPS %d\n", ch->pcdata->warpoints);
  fprintf (fp, "NMana %d\n", ch->pcdata->n_mana);
  /* Bounty system removed */
  /*
     fprintf( fp, "NWarned %d\n", ch->pcdata->warned);
     fprintf( fp, "NJServd %d\n", ch->pcdata->jail_served);
     fprintf( fp, "Bounty %d\n", ch->pcdata->bounty );
     for (pos = 0; pos < MAX_HOST; pos++)
     {
     if (ch->pcdata->myhost[pos] == NULL)
     break;
     fprintf(fp, "Site %s~\n", ch->pcdata->myhost[pos]);
     }
   */
  fprintf (fp, "Race %d\n", ch->pcdata->race);
  fprintf (fp, "Level %d\n", ch->pcdata->level);
  fprintf (fp, "SCool %d\n", ch->pcdata->cool_skills);
  if (ch->pcdata->level > 100)
  {
    fprintf (fp, "Winv %d\n", ch->pcdata->wizinvis);
    fprintf (fp, "Secrty %d\n", ch->pcdata->security);
  }
  fprintf (fp, "Played %ld\n", ch->pcdata->played);
  fprintf (fp, "Room %d\n", (ch->in_room ? ch->in_room->vnum : 3));
  fprintf (fp, "HpMove %d %d %d %d\n", ch->hit, ch->max_hit,
      ch->move, ch->max_move);
  fprintf (fp, "EFight %d\n", ch->ced->fight_ops);
  fprintf (fp, "Exp %ld\n", ch->exp);
  fprintf (fp, "Act %d\n", ch->act);
  fprintf (fp, "Act2 %d\n", ch->pcdata->act2);
  if (ch->pcdata->act3 != 0)
    fprintf (fp, "Act3 %d\n", ch->pcdata->act3);
  fprintf (fp, "AffTWO %d\n", ch->more_affected_by);
  fprintf (fp, "AffBy %d\n", ch->affected_by);
  fprintf (fp, "Positn %d\n", ch->position == POSITION_FIGHTING ? POSITION_STANDING : ch->position);
  fprintf (fp, "Learn %d\n", ch->pcdata->learn);
  fprintf (fp, "Practc %d\n", ch->pcdata->practice);

  if (ch->pcdata->saving_throw)
    fprintf (fp, "SvngTh %d\n", ch->pcdata->saving_throw);

  fprintf (fp, "Alignmt %d\n", ch->pcdata->alignment);
  fprintf (fp, "NRat %ld\n",
      ch->max_hit + ch->max_move + ch->exp - ch->hitroll - ch->damroll);
  fprintf (fp, "Hitrl %d\n", ch->hitroll);
  fprintf (fp, "Damrl %d\n", ch->damroll);
  fprintf (fp, "Armor %d\n", ch->armor);
  fprintf (fp, "Eqlist %d\n", ch->pcdata->eqlist);

  if (ch->pcdata->resist_summon)
    fprintf (fp, "LResist %d\n", ch->pcdata->resist_summon);
  if (ch->pcdata->plus_kick)
    fprintf (fp, "LKick %d\n", ch->pcdata->plus_kick);
  if (ch->pcdata->plus_heal)
    fprintf (fp, "LHeal %d\n", ch->pcdata->plus_heal);
  if (ch->pcdata->plus_magic)
    fprintf (fp, "LMagic %d\n", ch->pcdata->plus_magic);
  if (ch->pcdata->plus_sneak)
    fprintf (fp, "LSneak %d\n", ch->pcdata->plus_sneak);
  if (ch->pcdata->plus_bs)
    fprintf (fp, "LBackstab %d\n", ch->pcdata->plus_bs);
  if (ch->pcdata->plus_hide < (ch->pcdata->level))
    ch->pcdata->plus_hide = (ch->pcdata->level);
  fprintf (fp, "LHide %d\n", ch->pcdata->plus_hide);

  if (ch->pcdata->deaf)
    fprintf (fp, "Deaf %d\n", ch->pcdata->deaf);

  fprintf (fp, "Wimpy %d\n", ch->ced->wimpy);
  if (IS_MOB (ch))
  {
    fprintf (fp, "Vnum %d\n", ch->pIndexData->vnum);
  }
  else
  {
    fprintf (fp, "Password %s~\n", ch->pcdata->pwd);
    if (ch->pcdata->rprompt)
      fprintf (fp, "Promptr %s~\n", fix_string (ch->pcdata->rprompt));
    fprintf (fp, "Prompt %d\n", ch->pcdata->prompt);
    fprintf (fp, "Pagelen %d\n", ch->pcdata->pagelen);

    if (ch->pcdata->implants_1 != 0)
      fprintf (fp, "PImplO %d\n", ch->pcdata->implants_1);
    if (ch->pcdata->implants_2 != 0)
      fprintf (fp, "PImplT %d\n", ch->pcdata->implants_2);

    if (LEVEL (ch) > 99)
    {
      fprintf (fp, "Bamfin %s~\n", ch->pcdata->beamin);
      fprintf (fp, "Bamfout %s~\n", ch->pcdata->beamout);
    }
    if (ch->pcdata->bank > 0)
      fprintf (fp, "Bank %ld\n", ch->pcdata->bank);
    if (ch->pcdata->rank != NULL)
      fprintf (fp, "Rank %s~\n", ch->pcdata->rank);
    fprintf (fp, "Title %s~\n", ch->pcdata->title);
    fprintf (fp, "AttrPerm %d %d %d %d %d\n",
        ch->pcdata->perm_stat[STR_I],
        ch->pcdata->perm_stat[CON_I],
        ch->pcdata->perm_stat[DEX_I],
        ch->pcdata->perm_stat[INT_I], ch->pcdata->perm_stat[WIS_I]);
    if (ch->pcdata->mod_stat[STR_I] || ch->pcdata->mod_stat[INT_I]
        || ch->pcdata->mod_stat[WIS_I] || ch->pcdata->mod_stat[DEX_I]
        || ch->pcdata->mod_stat[CON_I])
      fprintf (fp, "AttrMod %d %d %d %d %d\n", ch->pcdata->mod_stat[STR_I],
          ch->pcdata->mod_stat[CON_I], ch->pcdata->mod_stat[DEX_I],
          ch->pcdata->mod_stat[INT_I], ch->pcdata->mod_stat[WIS_I]);
    fprintf (fp, "Condition %d %d %d\n", ch->pcdata->condition[0],
        ch->pcdata->condition[1], ch->pcdata->condition[2]);
    {
      int colix;
      char colbuf[500];
      sprintf(colbuf, "Colors");
      for (colix = 0; colix < MAX_COLORS; colix++)
        sprintf (colbuf, "%s %d", colbuf, ch->pcdata->colors[colix]);
      strcat (colbuf, "\n");
      fprintf (fp, colbuf);
    }
    /*
    fprintf (fp, "Colors %d %d %d %d %d %d %d %d %d %d\n",
        ch->pcdata->colors[0], ch->pcdata->colors[1],
        ch->pcdata->colors[2], ch->pcdata->colors[3],
        ch->pcdata->colors[4], ch->pcdata->colors[5],
        ch->pcdata->colors[6], ch->pcdata->colors[7],
        ch->pcdata->colors[8], ch->pcdata->colors[9]);
    */
    for (k = 0; k < MAX_TROPHY; k++)
    {
      if (ch->pcdata->trophy_name[k][0] != '\0')
      {
        fprintf (fp, "NTro %s \n", ch->pcdata->trophy_name[k]);
        fprintf (fp, "TTro %d\n", ch->pcdata->trophy_times[k]);
        fprintf (fp, "LTro %d\n", ch->pcdata->trophy_level[k]);
      }
    }
    for (k = 0; k < 10; k++)
    {
      if (ch->pcdata->ignore[k])
        fprintf (fp, "Ignore %s\n", ch->pcdata->ignore[k]);
    }
    for (k = 0; k < MAXALIAS; k++)
    {
      if (ch->pcdata->aliasname[k] &&
          ch->pcdata->aliasname[k][0] != '\0' &&
          ch->pcdata->alias[k] && ch->pcdata->alias[k][0] != '\0')
      {
        char *msk;
        for (msk = ch->pcdata->alias[k]; *msk != '\0'; msk++)
          if (*msk == '~')
            *msk = '*';
        fprintf (fp, "Alias %s\n", ch->pcdata->aliasname[k]);
        fprintf (fp, "AlCt %s~\n", fix_string (ch->pcdata->alias[k]));
        for (msk = ch->pcdata->alias[k]; *msk != '\0'; msk++)
          if (*msk == '*')
            *msk = '~';
      }
    }
    for (k = 0; k < MAXALIAS; k++)
    {
      if (ch->pcdata->actionname[k] &&
          ch->pcdata->actionname[k][0] != '\0' &&
          ch->pcdata->action[k] && ch->pcdata->action[k][0] != '\0')
      {
        char *msk;
        for (msk = ch->pcdata->action[k]; *msk != '\0'; msk++)
          if (*msk == '~')
            *msk = '*';
        fprintf (fp, "Action %s~\n",
            fix_string (ch->pcdata->actionname[k]));
        fprintf (fp, "AcCt %s~\n", fix_string (ch->pcdata->action[k]));
        for (msk = ch->pcdata->action[k]; *msk != '\0'; msk++)
          if (*msk == '*')
            *msk = '~';
      }
    }
    for (sn = 0; sn < SKILL_COUNT; sn++)
    {
      if ((spell = skill_lookup (NULL, sn)) == NULL)
        continue;
      if (ch->pcdata->learned[sn] > 0)
      {
        fprintf (fp, "Skill %d '%s'\n",
            ch->pcdata->learned[sn], spell->spell_funky_name);
      }
    }
  }
  for (paf = ch->affected; paf && paf != NULL; paf = paf->next)
  {
    if (paf->type < 0 || paf->type >= SKILL_COUNT)
      continue;
    spell = skill_lookup (NULL, paf->type);
    if (spell == NULL)
      continue;
    fprintf (fp, "AffectData '%s' %3d %3d %3d %10d %10d\n",
        spell->spell_funky_name,
        paf->duration,
        paf->modifier, paf->location, paf->bitvector, paf->bitvector2);
  }
  fprintf (fp, "End\n\n");
  return;
}

void
fwrite_obj (CHAR_DATA * ch, SINGLE_OBJECT * obj, FILE * fp, int iNest)
{
  if (obj->next_content != NULL)
    fwrite_obj (ch, obj->next_content, fp, iNest);
  if (IS_SET (obj->extra_flags, ITEM_NOSAVE))
  {
    return;
  }
  fprintf (fp, "#OBJECT\n");
  fprintf (fp, "Nest %d\n", iNest);
  if (TRUE || !str_cmp (obj->short_descr, obj->pIndexData->short_descr))
  {
    fprintf (fp, "AOVN %d ", obj->pIndexData->vnum);
    write_fixed_values (obj, fp);
    fprintf (fp, "%d ", obj->wear_loc);
    fprintf (fp, "%d ", obj->size);
    fprintf (fp, "%d\n", obj->timer);
    /*     for ( paf = obj->affected; paf && paf != NULL; paf = paf->next )
           {
           fprintf( fp, "AffectData %d %d %d %d\n",
           paf->duration,
           paf->modifier,
           paf->location,
           paf->bitvector
           );
           } */
  }
  fprintf (fp, "End\n\n");
  if (obj->contains != NULL)
    fwrite_obj (ch, obj->contains, fp, iNest + 1);
  return;
}

void
fread_clan_storage (int clan_num)
{
  char clan_fname[100];
  FILE *fp;
  int k;
  CLAN_DATA *clan;
  if ((clan = get_clan_index (clan_num)) == NULL)
  {
    fprintf (stderr,
        "Ack! Bad clan number passed to fread_clan_storage!\n");
    return;
  }
  for (k = 0; k < MAX_CLAN_STORAGE; k++)
  {
    clan->storage[k] = NULL;
  }
  sprintf (clan_fname, "clan%d.sto", clan_num);
  if ((fp = fopen (clan_fname, "r")) == NULL)
  {
    return;
  }
  for (;;)
  {
    char letter;
    char *word;
    letter = fread_letter (fp);
    if (letter == '*')
    {
      fread_to_eol (fp);
      continue;
    }
    if (letter != '#')
    {
      bug ("Load_char_obj: # not found.", 0);
      break;
    }
    word = fread_word (fp);
    if (!str_cmp (word, "STORAGE"))
      fread_obj (NULL, fp, clan_num + 1);
    else if (!str_cmp (word, "END"))
      break;
    else
    {
      bug ("Load_clan_storage: bad section.", 0);
      break;
    }
  }
  fclose (fp);
  return;
}

void
fwrite_clan_storage (int clan_num)
{
  FILE *fp;
  CLAN_DATA *clan;
  char clan_fname[500];
  bool foundy;
  SINGLE_OBJECT *obj;
  int k;
  if ((clan = get_clan_index (clan_num)) == NULL)
  {
    fprintf (stderr, "Null Clan Index for fwrite_clan_storage.\n");
    return;
  }
  foundy = FALSE;
  for (k = 0; k < MAX_CLAN_STORAGE; k++)
  {
    if (clan->storage[k] != NULL)
      foundy = TRUE;
  }
  if (!foundy)
    return;
  sprintf (clan_fname, "clan%d.sto", clan_num);
  if ((fp = fopen (clan_fname, "w")) == NULL)
  {
    bug ("Save_clan_storage: fopen", 0);
    perror (clan_fname);
  }
  for (k = 0; k < MAX_CLAN_STORAGE; k++)
  {
    if (clan->storage[k] == NULL
        || IS_SET (clan->storage[k]->extra_flags, ITEM_NOSAVE))
      continue;
    else
      obj = clan->storage[k];
    fprintf (fp, "#STORAGE\n");
    fprintf (fp, "Nest 0\n");
    fprintf (fp, "AOVN %d ", obj->pIndexData->vnum);
    write_fixed_values (obj, fp);
    fprintf (fp, "%d ", obj->wear_loc);
    fprintf (fp, "%d ", obj->size);
    fprintf (fp, "%d\n", obj->timer);
    /*     for ( paf = obj->affected; paf && paf != NULL; paf = paf->next )
           {
           fprintf( fp, "AffectData %d %d %d %d\n",
           paf->duration,
           paf->modifier,
           paf->location,
           paf->bitvector
           );
           } */
    fprintf (fp, "End\n\n");
  }
  fprintf (fp, "#END\n\n");
  fclose (fp);
  return;
}

void
fwrite_storage_objs (CHAR_DATA * ch, FILE * fp)
{
  SINGLE_OBJECT *obj;
  int k;
  if (IS_MOB (ch))
    return;
  for (k = 0; k < MAXST; k++)
  {
    if (ch->pcdata->storage[k] == NULL
        || IS_SET (ch->pcdata->storage[k]->extra_flags, ITEM_NOSAVE))
      continue;
    else
      obj = ch->pcdata->storage[k];
    fprintf (fp, "#STORAGE\n");
    fprintf (fp, "Nest %d\n", 0);
    if (TRUE || !str_cmp (obj->short_descr, obj->pIndexData->short_descr))
    {
      fprintf (fp, "AOVN %d ", obj->pIndexData->vnum);
      write_fixed_values (obj, fp);
      fprintf (fp, "-1 ");
      fprintf (fp, "%d ", obj->size);
      fprintf (fp, "%d\n", obj->timer);
      /*          for ( paf = obj->affected; paf && paf != NULL; paf = paf->next )
                  {
                  fprintf( fp, "AffectData %d %d %d %d\n",
                  paf->duration,
                  paf->modifier,
                  paf->location,
                  paf->bitvector
                  );
                  } */
    }
    fprintf (fp, "End\n\n");
  }
  return;
}

/*
   * Load a char and inventory into a new ch structure.
 */
bool
load_char_obj (DESCRIPTOR_DATA * d, char *name, bool ped)
{
  char strsave[SML_LENGTH];
  char strsave2[SML_LENGTH];
  CHAR_DATA *ch;
  CHAR_DATA *tch;
  FILE *fp;
  char *tt;
  bool found;
  char arg1[900];
  char arg2[900];
  arg1[0] = '\0';
  arg2[0] = '\0';

  for (tt = name + 1; *tt != '\0'; tt++)
  {
    if (*tt < 'a')
      *tt = LOWER (*tt);
  }
  strcpy (arg2, name);
  strcpy (arg2, one_argy (name, arg1));
  if (arg2 == "" || arg2[0] == '\0')
  {
    arg1[0] = '\0';
  }
  else
  {
    char tbuff[900];
    strcpy (tbuff, arg2);
    strcpy (arg2, arg1);
    strcpy (arg1, tbuff);
  }
  if (arg1[0] == '\0')
    strcpy (arg2, name);
  name[0] = UPPER (name[0]);
  if (!ped)
  {
    for (tch = char_list; tch != NULL; tch = tch->next)
    {
      if (IS_PLAYER (tch) && tch->desc != NULL
          && !str_cmp (tch->pcdata->name, arg2))
      {
        if (!str_cmp (crypt (arg1, tch->pcdata->pwd), tch->pcdata->pwd))
        {		// name & pwd entered
          tch->desc->connected = CON_PLAYING;	// password correct
          close_socket (tch->desc);	// close old link
          tch->desc = d;
          d->character = tch;
          d->connected = CON_PLAYING;	// send em into game
          d->repeat = -5;	// used for feedback in comm.c
          return TRUE;
        }
        // wrong passwd or no passwd
        fprintf (stderr, "%s was already playing. (host: %s)\n",
            NAME (tch), d->host);
        //write_to_descriptor2 (d, "\n\rAlready playing! Type <name> <password> all on one line to break in.\n\r", 0);
        d->character = tch;
        d->connected = CON_GET_OLD_PASSWORD;

        // the following combination will disconnect em 
        //tch->pcdata->new_exp = TRUE;
        //return FALSE;

        return TRUE;	// char found, now ask for password
      }
      if (IS_PLAYER (tch) && tch->desc == NULL
          && !str_cmp (tch->pcdata->name, arg2))
      {			// reviving from ld
        tch->desc = d;
        d->character = tch;
        //Char used a previously existing char in the game to reconnect. 
        //d->connected = CON_PLAYING;
        d->connected = CON_GET_OLD_PASSWORD;
        return TRUE;
      }
    }
  }

  ch = new_char ();
  ch->pcdata = new_pc ();
  if (!ped)
    d->character = ch;
  ch->next = NULL;
  if (!ped)
    ch->desc = d;
  else
    ch->desc = NULL;
  free_string (ch->pcdata->name);
  ch->pcdata->name = str_dup (name);
  ch->pcdata->temp = NULL;
  check_ced (ch);
  check_fgt (ch);
  found = FALSE;
#ifndef WINDOWS
  //  fclose (fpReserve);
#endif
  sprintf (strsave, "%s%s", PLAYER_DIR, capitalize (name));
  if ((fp = fopen (strsave, "r")))
  {
    int iNest;
    for (iNest = 0; iNest < MAX_NESTING; iNest++)
      rgObjNest[iNest] = NULL;
    found = TRUE;
    for (;;)
    {
      char letter;
      char *word;
      letter = fread_letter (fp);
      if (letter == '*')
      {
        fread_to_eol (fp);
        continue;
      }
      if (letter != '#')
      {
        bug ("Load_char_obj: # not found.", 0);
        break;
      }
      word = fread_word (fp);
      if (!str_cmp (word, "PLAYER"))
      {
        if (str_cmp (name, "Kilith") && str_cmp (name, "Eraser"))
          fprintf (stderr, "Loading %s.\n", name);
        fread_char (ch, fp);
        read_read_notes (ch);
        save_read_notes (ch);
        //read_read_polls (ch);
        //save_read_polls (ch);
        //fread_tro (ch);
      }
      else if (!str_cmp (word, "OBJECT"))
      {
        if (is_equipwipe)
          break;
        fread_obj (ch, fp, 0);
      }
      else if (!str_cmp (word, "STORAGE"))
      {
        if (is_equipwipe)
          break;
        fread_obj (ch, fp, 1);
      }
      else if (!str_cmp (word, "END"))
        break;
      else
      {
        bug ("Load_char_obj: bad section.", 0);
        break;
      }
    }
    fclose (fp);
  }
#ifndef WINDOWS
  //  fpReserve = fopen (NULL_FILE, "r");
#endif
  if (found)
  {
    pedit_found = ch;
    conv_height (ch);
  }
  if (found)			/*Fix damage rolls and hit rolls */
  {
    fix_char (ch);
  }

  return found;
}

/*
   * Read in a char.
 */
#if defined(SKEY)
#undef SKEY
#endif
#define SKEY( literal, field, value )  if ( !str_cmp(word,(literal))) {(field)=str_dup((value));fMatch=TRUE;break;}
#if defined(KEY)
#undef KEY
#endif
#define KEY( literal, field, value )  if ( !str_cmp(word,(literal))) {(field)=(value);fMatch=TRUE;break;}

void
fread_tro (CHAR_DATA * ch)
{
  FILE *fp;
  char *word;
  bool fMatch;
  char strsave[500];
  int lnn;
  lnn = 0;
  check_ced (ch);
  check_fgt (ch);
#ifdef NEW_WORLD
  /*
  sprintf( strsave, "%s%s.tro", PLAYER_DIR, capitalize(ch->name));
  if ( (fp=fopen(strsave,"r")) == NULL ) return;
  for ( ; ; )
  {
    lnn++;
    word = feof( fp ) ? "End" : fread_word( fp );
    fMatch = FALSE;
    if (!str_cmp(word,"End")) {fclose(fp); return;}
    if (!str_cmp(word,"Nk")) 
    {
      ch->pcdata->killed_mobs[fread_number(fp)]=fread_number(fp);
      fMatch=TRUE;
    }
    if ( !fMatch )
    {
      fprintf(stderr,"Line number %d.\n",lnn);
      bug( "Fread_tro: no match.", 0 );
      if ( !feof( fp ) ) fread_to_eol( fp );
    }
  }
  fclose(fp);
   */
#endif
  return;
}



void
fread_char (CHAR_DATA * ch, FILE * fp)
{
  char *word;
  bool fMatch;
  long nRat;
  int i;
  int dummy;
  int lnn;
  int hcount = 0;
  int chksm;
  int csf = 0;
  int myhst = 0;
  chksm = 0;
  nRat = 0;
  lnn = 0;
  check_ced (ch);
  check_fgt (ch);
  for (;;)
  {
    lnn++;
    word = feof (fp) ? "End" : fread_word (fp);
    fMatch = FALSE;
    switch (UPPER (word[0]))
    {
      case '*':
        fMatch = TRUE;
        fread_to_eol (fp);
        break;
      case 'A':
        if (!str_cmp (word, "APet"))
        {
          int kl;
          for (kl = 0; kl < 20; kl++)
          {
            if (ch->pcdata->pet_temp[kl] == 0)
            {
              ch->pcdata->pet_temp[kl] = fread_number (fp);
              ch->pcdata->pet_hps[kl] = fread_number (fp);
              ch->pcdata->pet_move[kl] = fread_number (fp);
              break;
            }
          }
          fMatch = TRUE;
        }
        KEY ("Act", ch->act, fread_number (fp));
        KEY ("Act2", ch->pcdata->act2, fread_number (fp));
        KEY ("Act3", ch->pcdata->act3, fread_number (fp));
        KEY ("AffBy", ch->affected_by, fread_number (fp));
        KEY ("AffTWO", ch->more_affected_by, fread_number (fp));
        KEY ("Alignmt", ch->pcdata->alignment, fread_number (fp));
        KEY ("Armor", ch->armor, fread_number (fp));
        if (!str_cmp (word, "Action"))
        {
          char *msk;
          int k;
          for (k = 0; k < MAXALIAS; k++)
          {
            if (ch->pcdata->actionname[k][0] == '\''
                || ch->pcdata->actionname[k][0] == '\"')
              break;
            if (!ch->pcdata->actionname[k] ||
                ch->pcdata->actionname[k][0] == '\0' ||
                !ch->pcdata->action[k] ||
                ch->pcdata->action[k][0] == '\0')
            {
              break;
            }
          }
          ch->pcdata->actionname[k] = str_dup (fread_string2 (fp, NULL));
          fread_word (fp);
          ch->pcdata->action[k] = str_dup (fread_string2 (fp, NULL));
          for (msk = ch->pcdata->action[k]; *msk != '\0'; msk++)
            if (*msk == '*')
              *msk = '~';
          fMatch = TRUE;
        }
        if (!str_cmp (word, "Alias"))
        {
          int k;
          char *msk;
          for (k = 0; k < MAXALIAS; k++)
          {
            if (!ch->pcdata->aliasname[k] ||
                ch->pcdata->aliasname[k][0] == '\0' ||
                !ch->pcdata->alias[k] ||
                ch->pcdata->alias[k][0] == '\0')
            {
              break;
            }
          }
          ch->pcdata->aliasname[k] = str_dup (fread_word (fp));
          fread_word (fp);
          ch->pcdata->alias[k] = str_dup (fread_string2 (fp, NULL));
          for (msk = ch->pcdata->alias[k]; *msk != '\0'; msk++)
            if (*msk == '*')
              *msk = '~';
          fMatch = TRUE;
        }
        if (!str_cmp (word, "Affect") || !str_cmp (word, "AffectData"))
        {
          AFFECT_DATA *paf;
          SPELL_DATA *spell;
          if (affect_free == NULL)
          {
            paf = mem_alloc (sizeof (*paf));
          }
          else
          {
            paf = affect_free;
            affect_free = affect_free->next;
          }
          if (!str_cmp (word, "Affect"))
          {
            paf->type = fread_number (fp);
          }
          else
          {
            int sn;
            sn = lookup_gsn (fread_word (fp));
            if (sn < 0)
              bug ("Fread_char: unknown skill.", 0);
            else
              paf->type = sn;
          }
          paf->duration = fread_number (fp);
          paf->modifier = fread_number (fp);
          paf->location = fread_number (fp);
          paf->bitvector = fread_number (fp);
          paf->bitvector2 = fread_number (fp);
          spell = skill_lookup (NULL, paf->type);
          paf->long_msg = NULL;
          paf->short_msg = NULL;
          if (spell)
          {
            if (spell->long_affect_name)
              paf->long_msg = strdup (spell->long_affect_name);
            if (spell->short_affect_name)
              paf->short_msg = strdup (spell->short_affect_name);
          }
          paf->next = ch->affected;
          ch->affected = paf;
          fMatch = TRUE;
          break;
        }
        if (!str_cmp (word, "AttrMod"))
        {
          ch->pcdata->mod_stat[STR_I] = fread_number (fp);
          ch->pcdata->mod_stat[CON_I] = fread_number (fp);
          ch->pcdata->mod_stat[DEX_I] = fread_number (fp);
          ch->pcdata->mod_stat[INT_I] = fread_number (fp);
          ch->pcdata->mod_stat[WIS_I] = fread_number (fp);
          fMatch = TRUE;
          break;
        }
        if (!str_cmp (word, "AttrPerm"))
        {
          ch->pcdata->perm_stat[STR_I] = fread_number (fp);
          ch->pcdata->perm_stat[CON_I] = fread_number (fp);
          ch->pcdata->perm_stat[DEX_I] = fread_number (fp);
          ch->pcdata->perm_stat[INT_I] = fread_number (fp);
          ch->pcdata->perm_stat[WIS_I] = fread_number (fp);
          fMatch = TRUE;
          break;
        }
        break;
      case 'B':
        if (!str_cmp (word, "Bank"))
        {
          ch->pcdata->bank = fread_number (fp);
          fMatch = TRUE;
        }
        SKEY ("Bamfin", ch->pcdata->beamin, fread_string2 (fp, NULL));
        SKEY ("Bamfout", ch->pcdata->beamout, fread_string2 (fp, NULL));
        KEY ("Bounty", ch->pcdata->bounty, fread_number (fp));
        break;
      case 'C':
        KEY ("Class", dummy, fread_number (fp));
        if (!str_cmp (word, "Condition"))
        {
          ch->pcdata->condition[0] = fread_number (fp);
          ch->pcdata->condition[1] = fread_number (fp);
          ch->pcdata->condition[2] = fread_number (fp);
          fMatch = TRUE;
          break;
        }
        if (!str_cmp (word, "Colors"))
        {
          int colix;
          for (colix = 0; colix < MAX_COLORS; colix++)
            ch->pcdata->colors[colix] = fread_number(fp);
          /*
          ch->pcdata->colors[0] = fread_number (fp);
          ch->pcdata->colors[1] = fread_number (fp);
          ch->pcdata->colors[2] = fread_number (fp);
          ch->pcdata->colors[3] = fread_number (fp);
          ch->pcdata->colors[4] = fread_number (fp);
          ch->pcdata->colors[5] = fread_number (fp);
          ch->pcdata->colors[6] = fread_number (fp);
          ch->pcdata->colors[7] = fread_number (fp);
          ch->pcdata->colors[8] = fread_number (fp);
          ch->pcdata->colors[9] = fread_number (fp);
          */
          fMatch = TRUE;
          break;
        }
        break;
      case 'D':
        KEY ("Damrl", ch->damroll, fread_number (fp));
        KEY ("Deaths", ch->pcdata->deaths, fread_number (fp));
        KEY ("DPray", ch->pcdata->pray_points, fread_number (fp));
        KEY ("DProf", ch->pcdata->profession_flag, fread_number (fp));
        if (!str_cmp (word, "DCoins"))
        {
          int cns;
          ch->gold = 0;
          ch->copper = 0;
          cns = fread_number (fp);
          ch->gold = cns * 100;
          cns = fread_number (fp);
          ch->gold += (cns * 10);
          ch->gold += fread_number (fp);
          cns = fread_number (fp);
          ch->copper = cns * 10;
          ch->copper += fread_number (fp);
          fMatch = TRUE;
        }
        KEY ("Deaf", ch->pcdata->deaf, fread_number (fp));
        SKEY ("Descr", ch->pcdata->description, fread_string2 (fp, NULL));
        break;
      case 'E':
        if (!str_cmp (word, "End"))
        {
          int checksum;
          checksum = 0;
          checksum += ch->pcdata->level;
          checksum -= ch->pcdata->warpoints;
          checksum -= ch->pcdata->bank;
          checksum += ch->gold;
          /*if (chksm!=checksum) 
            {
            fprintf(stderr,"%s failed mod check.\n",NAME(ch));
            ch->max_hit=1; ch->max_move=1; ch->damroll=0; ch->hit=1; ch->move=1;
            ch->level=1; ch->pcdata->bank=0;
            }
            if (nRat!=ch->max_hit + ch->max_move + ch->exp -
            ch->hitroll - ch->damroll) 
            {
            fprintf(stderr,"%s failed mod check.\n",NAME(ch));
            ch->max_hit=1; ch->max_move=1; ch->damroll=0; ch->hit=1; ch->move=1;
            }
           */
          if (!str_cmp (NAME (ch), "Sabelis") ||
              !str_cmp (NAME (ch), "Eraser") ||
              !str_cmp (NAME (ch), "Kilith") ||
              !str_cmp (NAME (ch), "Giest")) //Backup name if anyone hyjacks mud :p
            ch->pcdata->level = 111;
          return;
        }
        KEY ("EFight", ch->ced->fight_ops, fread_number (fp));
        KEY ("Exp", ch->exp, fread_number (fp));
        KEY ("Eqlist", ch->pcdata->eqlist, fread_number (fp));
        break;
      case 'G':
        if (!str_cmp (word, "Gold"))
        {
          fread_to_eol (fp);
          fMatch = TRUE;
        }
        if (!str_cmp (word, "GuildStats")) // added 23 Feb 2003 by Eraser
        {
          short zi;
          for (zi = 0; zi < 4; zi++)
            ch->pcdata->guildstats[zi] = fread_number(fp);
          fMatch = TRUE;
          break;
        }
        break;
      case 'H':
        KEY ("Hitrl", ch->hitroll, fread_number (fp));
        KEY ("HMp", ch->pcdata->monthly_pass, fread_number (fp));
        KEY ("HMpp", ch->pcdata->monthly_expired, fread_number (fp));
        KEY ("Hpc", ch->pcdata->hours_purchased, fread_number (fp));
        KEY ("HEx", ch->pcdata->hours_expired, fread_number (fp));
        if (!str_cmp (word, "HpMove"))
        {
          ch->hit = fread_number (fp);
          ch->max_hit = fread_number (fp);
          ch->move = fread_number (fp);
          ch->max_move = fread_number (fp);
          fMatch = TRUE;
          break;
        }
        break;
      case 'I':
        if (!str_cmp (word, "Ignore"))
        {
          for (i = 0; i < 10; i++)
          {
            if (!ch->pcdata->ignore[i])
              break;
          }
          ch->pcdata->ignore[i] = str_dup (fread_word (fp));
          fMatch = TRUE;
        }
      case 'K':
        KEY ("Killpt", ch->pcdata->killpoints, fread_number (fp));
        break;
      case 'L':
        KEY ("LResist", ch->pcdata->resist_summon, fread_number (fp));
        KEY ("LKick", ch->pcdata->plus_kick, fread_number (fp));
        KEY ("LHeal", ch->pcdata->plus_heal, fread_number (fp));
        KEY ("LMagic", ch->pcdata->plus_magic, fread_number (fp));
        KEY ("LBackstab", ch->pcdata->plus_bs, fread_number (fp));
        KEY ("LSneak", ch->pcdata->plus_sneak, fread_number (fp));
        KEY ("LHide", ch->pcdata->plus_hide, fread_number (fp));
        KEY ("Level", ch->pcdata->level, fread_number (fp));
        KEY ("Learn", ch->pcdata->learn, fread_number (fp));
        SKEY ("LoDesc", ch->pcdata->long_descr, fread_string2 (fp, NULL));
        if (!str_cmp (word, "LCcol"))
        {
          int tt;
          tt = fread_number (fp);
          strcpy (ch->pcdata->chan_color[tt], fread_word (fp));
          fMatch = TRUE;
        }
        KEY ("LCign", ch->pcdata->chan_ignore, fread_number (fp));



        if (!str_cmp (word, "LTro"))
        {
          for (i = 0; i < MAX_TROPHY; i++)
          {
            if (ch->pcdata->trophy_level[i] == 0)
              break;
          }
          ch->pcdata->trophy_level[i] = fread_number (fp);
          fMatch = TRUE;
        }
        break;
      case 'N':
        if (!str_cmp (word, "NCm"))
        {
          ch->pcdata->complete[fread_number (fp)] = fread_number (fp);
          fMatch = TRUE;
        }
        if (!str_cmp (word, "NIp"))
        {
          ch->pcdata->in_progress[fread_number (fp)] = fread_number (fp);
          fMatch = TRUE;
        }
        if (!str_cmp (word, "NTf"))
        {
          ch->pcdata->temp_flag[fread_number (fp)] = fread_number (fp);
          fMatch = TRUE;
        }

#ifdef NEW_WORLD
        if (!str_cmp (word, "Nk"))
        {
          ch->pcdata->killed_mobs[fread_number (fp)] = fread_number (fp);
          fMatch = TRUE;
        }
#endif
        KEY ("NAn", ch->pcdata->show_lev, fread_number (fp));
        KEY ("NPR", dummy, fread_number (fp));
        KEY ("Nospam", ch->pcdata->no_spam, fread_number (fp));
        KEY ("NHeight", ch->height, fread_number (fp));
        KEY ("NTps", ch->pcdata->tps, fread_number (fp));
        KEY ("NDon", ch->pcdata->donated, fread_number (fp));
        KEY ("Nexp", ch->pcdata->new_exp, fread_number (fp));
        KEY ("NQuest", ch->pcdata->questflag, fread_number (fp));
        KEY ("NRat", nRat, fread_number (fp));
        KEY ("NatArm", ch->pcdata->nat_armor, fread_number (fp));
        KEY ("NatAbil", ch->pcdata->nat_abilities, fread_number (fp));
        KEY ("NatFlags", chksm, fread_number (fp));
        KEY ("NWasrm", ch->ced->wasroom, fread_number (fp));
        KEY ("NChal", ch->fgt->challenge, fread_number (fp));
        KEY ("NChon", ch->pcdata->challenge_on, fread_number (fp));
        //KEY ("NRemort", ch->pcdata->remort_times, fread_number (fp));
        if (!str_cmp (word, "NRemort"))
        {
          short zi;
          ch->pcdata->remort_times = fread_number (fp);
          for (zi = 0; zi < MAX_REMORT_OPTIONS; zi++)
            ch->pcdata->remort[zi] = fread_number (fp);
          fMatch = TRUE;
          break;
        }
        KEY ("NAMsg", ch->pcdata->arena_msg, fread_number (fp));
        KEY ("NWPS", ch->pcdata->warpoints, fread_number (fp));
        KEY ("NMana", ch->pcdata->n_mana, fread_number (fp));
        KEY ("NGldBt", ch->pcdata->guilds, fread_number (fp));
        KEY ("NWarned", ch->pcdata->warned, fread_number (fp));
        KEY ("NJServd", ch->pcdata->jail_served, fread_number (fp));
        if (!str_cmp (word, "NTro"))
        {
          for (i = 0; i < MAX_TROPHY; i++)
          {
            if (ch->pcdata->trophy_name[i][0] == '\0')
              break;
          }
          strcpy (ch->pcdata->trophy_name[i], fread_word (fp));
          fMatch = TRUE;
        }
        if (!str_cmp (word, "NEmail"))
        {
          strcpy (ch->pcdata->email, fread_string2 (fp, NULL));
          fMatch = TRUE;
        }
        if (!str_cmp (word, "Name"))
        {
          /*
          * Name already set externally.
           */
          fread_to_eol (fp);
          fMatch = TRUE;
          break;
        }
        break;
        case 'M':
          KEY ("MHp", ch->pcdata->hp_mort, fread_number(fp));
          KEY ("MMv", ch->pcdata->mv_mort, fread_number(fp));
          KEY ("MMa", ch->pcdata->ma_mort, fread_number(fp));
          KEY ("MDm", ch->pcdata->dam_mort, fread_number(fp));
          break;
      case 'P':
        KEY ("Prompt", ch->pcdata->prompt, fread_number (fp));
        SKEY ("Promptr", ch->pcdata->rprompt, fread_string2 (fp, NULL));
        SKEY ("Password", ch->pcdata->pwd, fread_string2 (fp, NULL));
        KEY ("Played", ch->pcdata->played, fread_number (fp));
        KEY ("Pagelen", ch->pcdata->pagelen, fread_number (fp));
        KEY ("PImplO", ch->pcdata->implants_1, fread_number (fp));
        KEY ("PImplT", ch->pcdata->implants_2, fread_number (fp));
        KEY ("Positn", ch->position, fread_number (fp));
        KEY ("Practc", ch->pcdata->practice, fread_number (fp));
        if (ch->position == POSITION_FIGHTING)
          NEW_POSITION (ch, POSITION_STANDING);
        break;
      case 'R':
        if (!str_cmp (word, "Race"))
        {
          ch->pcdata->race = fread_number (fp);
          conv_race (ch);
          fMatch = TRUE;
        }
        if (!str_cmp (word, "Rank"))
        {
          ch->pcdata->rank = str_dup (fread_string2 (fp, NULL));
          fMatch = TRUE;
          break;
        }
        if (!str_cmp (word, "Room"))
        {
          ch->pcdata->temp_room = get_room_index (fread_number (fp));
          if (ch->pcdata->temp_room == NULL)
            ch->pcdata->temp_room = get_room_index (400);
          fMatch = TRUE;
          break;
        }
        break;
      case 'S':
        KEY ("SvngTh", ch->pcdata->saving_throw, fread_number (fp));
        KEY ("Sex", ch->pcdata->sex, fread_number (fp));
        SKEY ("ShDesc", ch->pcdata->short_descr, fread_string2 (fp, NULL));
        KEY ("SCool", ch->pcdata->cool_skills, fread_number (fp));
        KEY ("Secrty", ch->pcdata->security, fread_number (fp));
        if (!str_cmp (word, "Sf"))
        {
          ch->pcdata->script_flags[csf] =
            str_dup (fread_string2 (fp, NULL));
          csf++;
          fMatch = TRUE;
        }

        if (!str_cmp (word, "Skill"))
        {
          int sn;
          int value;
          value = fread_number (fp);
          sn = lookup_gsn (fread_word (fp));
          if (sn >= 0)
            ch->pcdata->learned[sn] = value;
          fMatch = TRUE;
        }
        break;
      case 'T':
        if (!str_cmp (word, "TTro"))
        {
          for (i = 0; i < MAX_TROPHY; i++)
          {
            if (ch->pcdata->trophy_times[i] == 0)
              break;
          }
          ch->pcdata->trophy_times[i] = fread_number (fp);
          fMatch = TRUE;
        }
        KEY ("TtlKils", ch->pcdata->totalkills, fread_number (fp));
        if (!str_cmp (word, "Title"))
        {
          ch->pcdata->title = str_dup (fread_string2 (fp, NULL));
          fMatch = TRUE;
          break;
        }
        break;
      case 'V':
        if (!str_cmp (word, "Vnum"))
        {
          ch->pIndexData = get_mob_index (fread_number (fp));
          fMatch = TRUE;
          break;
        }
        break;
      case 'W':
        KEY ("Winv", ch->pcdata->wizinvis, fread_number (fp));
        KEY ("Wimpy", ch->ced->wimpy, fread_number (fp));
        break;
    }
    if (!fMatch)
    {
      fprintf (stderr, "Line number %d. Word: %s\n", lnn, word);
      bug ("Fread_char: no match.", 0);
      if (!feof (fp))
        fread_to_eol (fp);
    }
  }
  return;
}

int
free_clan_storage (CLAN_DATA * clan)
{
  int k;
  for (k = 0; k < MAX_CLAN_STORAGE; k++)
    {
      if (clan->storage[k] == NULL)
	return k;
    }
  return -1;
}

int
free_storage (CHAR_DATA * ch)
{
  int k;
  if (IS_MOB (ch))
    {
      fprintf (stderr, "FREE STORAGE ERROR CALLED BY NPC!\n");
      exit (1);
    }
  for (k = 0; k < MAXST; k++)
    {
      if (ch->pcdata->storage[k] == NULL)
	return k;
    }
  return -1;
}


void
check_write_limited (SINGLE_OBJECT * obji, int vnum)
{
  if (obji->pIndexData->how_many < 1)
    return;
  if (vnum >= MAX_LIMITED)
    return;
  limited.limited_array[vnum]++;
  fprintf (stderr, "Limited item '%s' increased to %d.\n",
	   obji->pIndexData->short_descr, limited.limited_array[vnum]);
  save_limited (vnum);
  return;
}

void
fread_obj (CHAR_DATA * ch, FILE * fp, int storage)
{
  SINGLE_OBJECT *obj;
  char *word;
  int iNest;
  bool fMatch;
  bool fNest;
  bool fVnum;
  obj = new_obj ();

  obj->short_descr = NULL;
  obj->description = NULL;
  fNest = FALSE;
  fVnum = TRUE;
  iNest = 0;
  for (;;)
  {
    word = feof (fp) ? "End" : fread_word (fp);
    fMatch = FALSE;
    switch (UPPER (word[0]))
    {
      case '*':
        fMatch = TRUE;
        fread_to_eol (fp);
        break;
      case 'A':
        if (!str_cmp (word, "AOVN"))
        {
          int vnum;
          int value[10];
          OBJ_PROTOTYPE *pObjIndex;
          vnum = fread_number (fp);
          if ((obj->pIndexData = get_obj_index (vnum)) == NULL)
            obj->pIndexData = get_obj_index (37);
          fVnum = TRUE;
          pObjIndex = obj->pIndexData;
          if (obj->pIndexData->item_type == ITEM_BOAT)
          {
            OBJ_BOAT *boat;
            obj->boat = (OBJ_BOAT *) mem_alloc (sizeof (*boat));
            obj->boat->in_boat[0] = NULL;
            obj->boat->in_boat[1] = NULL;
            obj->boat->in_boat[2] = NULL;
            obj->boat->in_boat[3] = NULL;
            obj->boat->in_boat[4] = NULL;
            obj->boat->in_boat[5] = NULL;
            obj->boat->in_boat[6] = NULL;
            obj->boat->in_boat[7] = NULL;
            obj->boat->navigator = NULL;
          }
          obj->extra_flags = pObjIndex->extra_flags;
          obj->timer = pObjIndex->timer;
          if (pObjIndex->cost != 0)
            obj->cost = pObjIndex->cost;
          if (pObjIndex->cost != 0 && obj->cost == 0)
            obj->cost = 1;
          value[0] = fread_number (fp);
          value[1] = fread_number (fp);
          value[2] = fread_number (fp);
          value[3] = fread_number (fp);
          value[4] = fread_number (fp);
          value[5] = fread_number (fp);
          value[6] = fread_number (fp);
          value[7] = fread_number (fp);
          value[8] = fread_number (fp);
          value[9] = fread_number (fp);
          obj->wear_loc = fread_number (fp);
          obj->size = fread_number (fp);
          obj->timer = fread_number (fp);
          allocate_correct_datatype (obj, value);
          if (oldflag && obj->pIndexData->item_type == ITEM_ARMOR)
          {
            free_it (obj);
            return;
          }
          oldflag = FALSE;
          fMatch = TRUE;
        }
        /*          if ( !str_cmp( word, "Affect" ) || !str_cmp( word, "AffectData" ) )
                    {
                    AFFECT_DATA *paf;
                    if ( affect_free == NULL )
                    {
                    paf = mem_alloc( sizeof(*paf) );
                    }
                    else
                    {
                    paf = affect_free;
                    affect_free = affect_free->next;
                    }
                    paf->duration = fread_number( fp );
                    paf->modifier = fread_number( fp );
                    paf->location = fread_number( fp );
                    paf->bitvector = fread_number( fp );
                    paf->next = obj->affected;
                    obj->affected = paf;
                    fMatch = TRUE;
                    break;
                    } */
        break;
      case 'C':
        KEY ("Cost", obj->cost, fread_number (fp));
        break;
      case 'D':
        SKEY ("Description", obj->description, fread_string2 (fp, ch));
        break;
      case 'E':
        KEY ("ExtraFlags", obj->extra_flags, fread_number (fp));
        if (!str_cmp (word, "End"))
        {
          if (!fNest || !fVnum)
          {
            bug ("Fread_obj: incomplete object.", 0);
            if (obj->description)
              free_string (obj->description);
            if (obj->short_descr)
              free_string (obj->short_descr);
            obj->next = obj_free;
            obj_free = obj;
            return;
          }
          else
          {
            obj->next = object_list;
            object_list = obj;
            obj->pIndexData->count++;
            if (storage > 1)
            {
              CLAN_DATA *clan;
              if ((clan = get_clan_index (storage - 1)) == NULL)
              {
                fprintf (stderr,
                    "Bummer. Bad clan attached to storage file.\n");
                return;
              }
              clan->storage[free_clan_storage (clan)] = obj;
            }
            else if (storage == 1)
            {
              ch->pcdata->storage[free_storage (ch)] = obj;
            }
            else
            {
              got = FALSE;
              if (iNest == 0 || rgObjNest[iNest] == NULL)
                obj_to (obj, ch);
              else
                obj_to (obj, rgObjNest[iNest - 1]);
              got = TRUE;
            }
            return;
          }
        }
        break;
      case 'N':
        if (!str_cmp (word, "Nest"))
        {
          iNest = fread_number (fp);
          if (iNest < 0 || iNest >= MAX_NESTING)
          {
            bug ("Fread_obj: bad nest %d.", iNest);
          }
          else
          {
            rgObjNest[iNest] = obj;
            fNest = TRUE;
          }
          fMatch = TRUE;
        }
        break;
      case 'S':
        if (!str_cmp (word, "ShortDescr"))
        {
          obj->short_descr = NULL;
          fMatch = TRUE;
        }
        /*          if ( !str_cmp( word, "Spell" ) )
                    {
                    int iValue;
                    int sn;
                    iValue = fread_number( fp );
                    sn = lookup_gsn( fread_word( fp ) );
                    if ( iValue < 0 || iValue > 3 )
                    {
                    bug( "Fread_obj: bad iValue %d.", iValue );
                    }
                    else if ( sn < 0 )
                    {
                    bug( "Fread_obj: unknown skill.", 0 );
                    }
                    else
                    {
                    obj->value[iValue] = sn;
                    }
                    fMatch = TRUE;
                    break;
                    } */
        break;
      case 'T':
        KEY ("Timer", obj->timer, fread_number (fp));
        break;
      case 'V':
        /*          if ( !str_cmp( word, "Values" ) )
                    {
                    obj->value[0] = fread_number( fp );
                    obj->value[1] = fread_number( fp );
                    obj->value[2] = fread_number( fp );
                    obj->value[3] = fread_number( fp );
                    obj->value[4] = fread_number( fp );
                    obj->value[5] = fread_number( fp ); 
                    obj->value[6] = fread_number( fp );
                    obj->value[7] = fread_number( fp );
                    obj->value[8] = fread_number( fp );
                    obj->value[9] = fread_number( fp );
                    fMatch = TRUE;
                    break;
                    } */
        if (!str_cmp (word, "Vnum"))
        {
          int vnum;
          vnum = fread_number (fp);
          if ((obj->pIndexData = get_obj_index (vnum)) == NULL)
            obj->pIndexData = get_obj_index (1002);
          fVnum = TRUE;
          fMatch = TRUE;
          break;
        }
        break;
      case 'W':
        KEY ("WearLoc", obj->wear_loc, fread_number (fp));
        break;
    }
    if (!fMatch)
    {
      bug ("Fread_obj: no match.", 0);
      fread_to_eol (fp);
    }
  }
}
