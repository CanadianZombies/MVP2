#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emlen.h"

/* sucksville searching and sorting demo it seems.  maybe I shoulda taken a coding
class first */

void check_topten (CHAR_DATA * ch)
{
	int k;
	int slotnum;
	int tk;
	bool in_topten_already;
	if (IS_MOB (ch) || LEVEL (ch) > 99)
		return;
	slotnum = -1;
	for (k = 0; k < MAX_TOPTEN; k++)
	{
		if (ch->pcdata->warpoints > topten.topten_warpoints[k]
				&& str_cmp (capitalize (NAME (ch)), topten.topten_name[k]))
		{
			slotnum = k;
			break;
		}
		if (!str_cmp (capitalize (NAME (ch)), topten.topten_name[k]))
			topten.topten_warpoints[k] = ch->pcdata->warpoints;
	}
	if (slotnum == -1)
		return;
	in_topten_already = FALSE;
	/*Scan if already somewhere ELSE in topten */
	for (tk = 0; tk < MAX_TOPTEN; tk++)
	{
		if (!str_cmp (capitalize (NAME (ch)), topten.topten_name[tk]))
		{
			/*move everyone after that slot down on the list */
			for (k = tk; k < MAX_TOPTEN - 1; k++)
			{
				if (k > MAX_TOPTEN - 2)
					break;
				strcpy (topten.topten_name[k], topten.topten_name[k + 1]);
				topten.topten_warpoints[k] = topten.topten_warpoints[k + 1];
				topten.topten_good[k] = topten.topten_good[k + 1];
			}
			break;
		}
	}
	/*Move everyone after that slot down on the list */
	for (k = MAX_TOPTEN - 1; k > slotnum; k--)
	{
		strcpy (topten.topten_name[k], topten.topten_name[k - 1]);
		topten.topten_warpoints[k] = topten.topten_warpoints[k - 1];
		topten.topten_good[k] = topten.topten_good[k - 1];
	}
	topten.topten_warpoints[slotnum] = ch->pcdata->warpoints;
	strcpy (topten.topten_name[slotnum], capitalize (NAME (ch)));
	if (IS_EVIL (ch))
		topten.topten_good[slotnum] = FALSE;
	else
		topten.topten_good[slotnum] = TRUE;
	save_topten ();
	return;
}

void check_rating (CHAR_DATA * ch)
{
	int k;
	int slotnum;
	int tk;
	bool in_topten_already;
	int ratng;
	if (IS_MOB (ch) || LEVEL (ch) > 99)
		return;
	ratng = rating (ch);
	slotnum = -1;
	for (k = 0; k < MAX_RATING; k++)
	{
		if (ratng > ratingd.rating_rating[k]
				&& str_cmp (capitalize (NAME (ch)), ratingd.rating_name[k]))
		{
			slotnum = k;
			break;
		}
		if (!str_cmp (capitalize (NAME (ch)), ratingd.rating_name[k]))
			ratingd.rating_rating[k] = ratng;
	}
	if (slotnum == -1)
		return;
	in_topten_already = FALSE;
	/*Scan if already somewhere ELSE in topten */
	for (tk = 0; tk < MAX_RATING; tk++)
	{
		if (!str_cmp (capitalize (NAME (ch)), ratingd.rating_name[tk]))
		{
			/*move everyone after that slot down on the list */
			for (k = tk; k < MAX_RATING - 1; k++)
			{
				if (k > MAX_RATING - 2)
					break;
				strcpy (ratingd.rating_name[k], ratingd.rating_name[k + 1]);
				ratingd.rating_rating[k] = ratingd.rating_rating[k + 1];
				ratingd.rating_good[k] = ratingd.rating_good[k + 1];
			}
			break;
		}
	}
	/*Move everyone after that slot down on the list */
	for (k = MAX_RATING - 1; k > slotnum; k--)
	{
		strcpy (ratingd.rating_name[k], ratingd.rating_name[k - 1]);
		ratingd.rating_rating[k] = ratingd.rating_rating[k - 1];
		ratingd.rating_good[k] = ratingd.rating_good[k - 1];
	}
	ratingd.rating_rating[slotnum] = ratng;
	strcpy (ratingd.rating_name[slotnum], capitalize (NAME (ch)));
	if (IS_EVIL (ch))
		ratingd.rating_good[slotnum] = FALSE;
	else
		ratingd.rating_good[slotnum] = TRUE;
	if (number_range (1, 10) == 3)
		save_rating ();
	return;
}

void do_trophy (CHAR_DATA * ch, char *argy)
{
	bool foundy;
	char buffy[500];
	int used[MAX_TROPHY];
	int itt;
	int i;
	int curlevel;
	int ittt;
	int tempslot;
	DEFINE_COMMAND ("trophy", do_trophy, POSITION_SLEEPING, 0, LOG_NORMAL, "Shows your mob or pkill trophy.")
	if (IS_MOB (ch))
		return;
#ifdef NEW_WORLD
	if (argy[0] != 'm' && argy[0] != 'M' && argy[0] != 'p' && argy[0] != 'P')
	{
		send_to_char ("Please specify trophy [M]obs or trophy [P]layers.\n\r", ch);
		return;
	}
	if (argy[0] == 'm' || argy[0] == 'M')
	{
		MOB_PROTOTYPE *mid;
		hugebuf_o[0] = '\0';
		for (iHash = 0; iHash < HASH_MAX; iHash++)
		{
			for (mid = mob_index_hash[iHash]; mid != NULL; mid = mid->next)
			{
				if (mid->vnum >= MAX_MOB_TRO)
					continue;
				if (ch->pcdata->killed_mobs[mid->vnum] == 0)
					continue;
				i = mid->vnum;
				if (ch->pcdata->killed_mobs[i] > 999)
					ch->pcdata->killed_mobs[i] = 999;
				sprintf (hugebuf_o + strlen (hugebuf_o), "[%3d] %s\n\r",
					ch->pcdata->killed_mobs[i], mid->short_descr);
				if (strlen (hugebuf_o) > 29800)
				{
					hugebuf_o[29800] = '\0';
					goto eld;
				}
			}
		}
		eld:
		page_to_char (hugebuf_o, ch);
		return;
	}
#endif
	for (i = 0; i < MAX_TROPHY; i++)
		used[i] = 0;
	foundy = FALSE;
	send_to_char (" \x1B[34;1m[\x1B[30mTrophy of Kills\x1B[34m]\x1B[37;1m\n\r\n\r", ch);
		sprintf (buffy, "%5s %6s %5s\n\r", "\x1B[30;1m[\x1B[32mLvl\x1B[30m]", "\x1B[31;1m[\x1B[37mName\x1B[31m]", "\x1B[36;1m              [\x1B[35mTimes\x1B[36m]");
			send_to_char (buffy, ch);
	/*Sort Code */
			curlevel = 1000;
			for (ittt = 0; ittt < MAX_TROPHY; ittt++)
			{
				tempslot = -1;
				for (i = 0; i < MAX_TROPHY; i++)
				{
					bool gotone;
					gotone = FALSE;
			/*Find highest level trophy entry */
					if (ch->pcdata->trophy_level[i] < 1
						|| ch->pcdata->trophy_name[i][0] == '\0')
						continue;
					if (ch->pcdata->trophy_level[i] <= curlevel)
						if (used[i] == 0)
						{
							bool flagr;
							flagr = FALSE;
							for (itt = 0; itt < MAX_TROPHY; itt++)
							{
								if (used[itt] == 1)
									continue;
								if (ch->pcdata->trophy_level[itt] >
									ch->pcdata->trophy_level[i])
									flagr = TRUE;
							}
							if (!flagr)
							{
								curlevel = ch->pcdata->trophy_level[i];
								tempslot = i;
								used[i] = 1;
								gotone = TRUE;
							}
						}
						if (gotone)
							break;
					}
					if (tempslot != -1)
					{
						foundy = TRUE;
						sprintf (buffy, "\x1B[37;0m[\x1B[34;1m%3d\x1B[37;0m] \x1B[1;31m%-23s \x1B[37;1m*%-4d\n\r",
							ch->pcdata->trophy_level[tempslot],
							ch->pcdata->trophy_name[tempslot],
							ch->pcdata->trophy_times[tempslot]);
						send_to_char (buffy, ch);
						tempslot = -1;
					}
				}
				if (!foundy)
				{
					send_to_char ("\n\rNone.\n\r", ch);
				}
				send_to_char ("\x1B[37;0m", ch);
					return;
}

void do_topten (CHAR_DATA * ch, char *argy)
{
	bool foundy;
	char buffy[500];
	int used[MAX_TOPTEN];
	int itt;
	int position;
	int i;
	int curlevel;
	int ittt;
	int tempslot;
	char ttt[200];
	DEFINE_COMMAND ("topten", do_topten, POSITION_SLEEPING, 0, LOG_NORMAL, "Shows the top PKillers.")
	if (IS_MOB (ch))
		return;
	position = ch->position;
	ch->position = POSITION_STANDING;
	if (!str_cmp(argy, "reset") && LEVEL(ch) == 111)
		{
			send_to_char("Resetting topten.\n\r", ch);
			for (i = 0; i < MAX_RATING; i++)
				{
					 strcpy(topten.topten_name[i], "Free-Slot");
					 topten.topten_warpoints[i] = 0;
					 topten.topten_good[i] = 0;
				 }
			return;
		}

	for (i = 0; i < MAX_TOPTEN; i++)
		used[i] = 0;
	foundy = FALSE;
	sprintf (ttt, "$B$0[$4Top List of PKillers (%s/%s and %s/%s)$0]", good, evil, evil, good);
	act (ttt, ch, NULL, ch, TO_CHAR);
	send_to_char ("\n\r", ch);
	/*Sort Code */
	curlevel = 999000;
	for (ittt = 0; ittt < MAX_TOPTEN; ittt++)
	{
		tempslot = -1;
		for (i = 0; i < MAX_TOPTEN; i++)
		{
			bool gotone;
			gotone = FALSE;
			/*Find highest level trophy entry */
			if (topten.topten_warpoints[i] <= curlevel)
				if (used[i] == 0)
				{
					bool flagr;
					flagr = FALSE;
					for (itt = 0; itt < MAX_TOPTEN; itt++)
					{
						if (used[itt] == 1)
							continue;
						if (topten.topten_warpoints[itt] >
								topten.topten_warpoints[i])
							flagr = TRUE;
					}
					if (!flagr)
					{
						curlevel = topten.topten_warpoints[i];
						tempslot = i;
						used[i] = 1;
						gotone = TRUE;
					}
				}
			if (gotone)
				break;
		}
		if (tempslot != -1)
		{
			foundy = TRUE;
			sprintf (buffy, "%s $B$7#%-2d $1%-35s $3-->$2 %-6d$3 warpoints",
					(topten.topten_good[tempslot] ? "\x1B[34;1m*" : "\x1B[31;1m*"),
					ittt + 1, topten.topten_name[tempslot],
					topten.topten_warpoints[tempslot]);
			act (buffy, ch, NULL, ch, TO_CHAR);
			tempslot = -1;
		}
	}
	if (!foundy)
	{
		send_to_char ("\n\rNone.\n\r", ch);
	}
	send_to_char ("\x1B[37;0m", ch);
	ch->position = position;
	return;
}

void do_rating (CHAR_DATA * ch, char *argy)
{
	bool foundy;
	char buffy[500];
	int used[MAX_RATING];
	int itt;
	int position;
	int i;
	int curlevel;
	int ittt;
	int tempslot;
	DEFINE_COMMAND ("rating", do_rating, POSITION_SLEEPING, 0, LOG_NORMAL, "Shows you your current rating.")
	if (IS_MOB (ch))
		return;
	if (!pow.old_creation_method && LEVEL (ch) < 10)
	{
		send_to_char ("You can't see your rating until level 8.\n\r", ch);
		return;
	}
	if (LEVEL (ch) < 100)
		check_rating (ch);
	if (!str_cmp(argy, "reset") && LEVEL(ch) == 111)
		{
			send_to_char("Resetting rating.\n\r", ch);
			for (i = 0; i < MAX_RATING; i++)
				{
					 strcpy(ratingd.rating_name[i], "Free-Slot");
					 ratingd.rating_rating[i] = 0;
					 ratingd.rating_good[i] = 0;
				 }
			return;
		}
	position = ch->position;
	ch->position = POSITION_STANDING;
	for (i = 0; i < MAX_RATING; i++)
		used[i] = 0;
	foundy = FALSE;
	act ("$B$0[$4Top Rated Players$0]", ch, NULL, ch, TO_CHAR);
	send_to_char ("\n\r", ch);
	/*Sort Code */
	curlevel = 999000;
	for (ittt = 0; ittt < MAX_RATING; ittt++)
	{
		tempslot = -1;
		for (i = 0; i < MAX_RATING; i++)
		{
			bool gotone;
			gotone = FALSE;
			/*Find highest level trophy entry */
			if (ratingd.rating_rating[i] <= curlevel)
				if (used[i] == 0)
				{
					bool flagr;
					flagr = FALSE;
					for (itt = 0; itt < MAX_RATING; itt++)
					{
						if (used[itt] == 1)
							continue;
						if (ratingd.rating_rating[itt] > ratingd.rating_rating[i])
							flagr = TRUE;
					}
					if (!flagr)
					{
						curlevel = ratingd.rating_rating[i];
						tempslot = i;
						used[i] = 1;
						gotone = TRUE;
					}
				}
			if (gotone)
				break;
		}
		if (tempslot != -1)
		{
			foundy = TRUE;
			sprintf (buffy, "%s $B$7#%-2d $1%s%-35s $3-->$2 %-6d$3 **Rating**",
					(ratingd. rating_good[tempslot] ? "\x1B[34;1m*" : "\x1B[31;1m*"),
					ittt + 1,
					(!str_cmp(NAME(ch), ratingd.rating_name[tempslot]) ? "$B" : ""),
					ratingd.rating_name[tempslot],
					ratingd.rating_rating[tempslot]);
			act (buffy, ch, NULL, ch, TO_CHAR);
			tempslot = -1;
		}
	}
	if (!foundy)
	{
		send_to_char ("\n\rNone.\n\r", ch);
	}
	sprintf (buffy, "\n\r\x1B[37;0mYour current rating: \x1B[37;1m%d\x1B[37;0m\n\r", rating (ch));
	send_to_char (buffy, ch);
	ch->position = position;
	return;
}

int free_trophy_slot (CHAR_DATA * ch, CHAR_DATA * victim)
{
	int i;
	int templev;
	int tempnum;
	if (IS_MOB (ch))
		return -1;
	for (i = 0; i < MAX_TROPHY; i++)
	{
		if (ch->pcdata->trophy_name[i][0] == '\0')
			return i;
	}
	/*Okay, no free slots, replace lowest level */
	templev = 100;
	tempnum = -1;
	for (i = 0; i < MAX_TROPHY; i++)
	{
		if (ch->pcdata->trophy_level[i] < templev)
		{
			tempnum = i;
			templev = ch->pcdata->trophy_level[i];
		}
	}
	if (ch->pcdata->trophy_level[tempnum] <= LEVEL (victim))
		return tempnum;
	else
		return -1;
}

void check_add_trophy (CHAR_DATA * ch, CHAR_DATA * victim, int same_align, int diff_align)
{
	int i;
	int nummy;
	int same = same_align;
	char buf[STD_LENGTH];
	if (IS_MOB (ch) || IS_MOB (victim))
		return;
	/*Handle warpoints, improved by Kenor 1-Mar-2003 */
 //Updated in 2005
	if (rchars_in_group (ch) > 1)
	{
		int wpgain = 0;
		if (LEVEL (victim) >= 1)                             
						wpgain += 8;				// 8/4 = 2 8/2 = 4				
		if (LEVEL (victim) >= 14)
			wpgain += 8;				// 16/4 = 4 16/2 = 8
		if (LEVEL (victim) >= 20)
			wpgain += 16;				// 32/4 = 8 32/2 = 16
		if (LEVEL (victim) >= 50)
			wpgain += 32;				// 64/4 = 16 64/2 = 32
		if (LEVEL (victim) >= 75)
			wpgain += 64;				// 128/4 = 32 128/2 = 64
		if (victim->pcdata->remort_times == 0)
						wpgain += 0;
		if (victim->pcdata->remort_times == 1)
						wpgain += 5;				
		if (victim->pcdata->remort_times == 2)
						wpgain += 20;
		if (victim->pcdata->remort_times == 3)
						wpgain += 50;
		if (victim->pcdata->remort_times == 4)
						wpgain += 100;
		if (victim->pcdata->remort_times == 5)
						wpgain += 125;
		if (victim->pcdata->remort_times == 6)
						wpgain += 150;
		if (victim->pcdata->remort_times == 7)
						wpgain += 175;
		if (victim->pcdata->remort_times == 8)
						wpgain += 200;
		if (victim->pcdata->remort_times == 9)
						wpgain += 250;
		wpgain = wpgain / rchars_in_group(ch);
		ch->pcdata->warpoints += wpgain;
		sprintf(buf, "You just gained \x1B[31;1m%d\x1B[0m warpoints!\n\r", wpgain);
		send_to_char(buf, ch);
		sprintf(buf, "\x1B[1;31m%s \x1B[0;37mgained \x1B[1;31m%d \x1B[0;37mwarpoints on the pkill of \x1B[1;31m%s\x1B[0;37m\n\r", NAME(ch), wpgain, NAME(victim));
		log_string(buf);

	 }
	else { // Solo kill  
		int wpgain = 0;
		if (LEVEL (victim) >= 1)
						wpgain += 1;			// 1
		if (LEVEL (victim) >= 25)
			wpgain += 15;			// 16
		if (LEVEL (victim) >= 45)
			wpgain += 16;			// 32
		if (LEVEL (victim) >= 60)
			wpgain += 32;		        // 64
		if (LEVEL (victim) >= 75)
			wpgain += 64;		        // 128
		if (victim->pcdata->remort_times == 0)
						wpgain += 0;
		if (victim->pcdata->remort_times == 1)
						wpgain += 10;
		if (victim->pcdata->remort_times == 2)
						wpgain += 30;
		if (victim->pcdata->remort_times == 3)
						wpgain += 40;
		if (victim->pcdata->remort_times == 4)
						wpgain += 50;
		if (victim->pcdata->remort_times == 5)
						wpgain += 80;
		if (victim->pcdata->remort_times == 6)
						wpgain += 100;
		if (victim->pcdata->remort_times == 7)
						wpgain += 120;
		if (victim->pcdata->remort_times == 8)
						wpgain += 150;
		if (victim->pcdata->remort_times == 9)
						wpgain += 200;
		ch->pcdata->warpoints += wpgain;
		sprintf(buf, "You just gained \x1B[31;1m%d\x1B[0m warpoints!\n\r", wpgain);
		send_to_char(buf, ch);
		sprintf(buf, "\x1B[1;31m%s \x1B[0;37mgained \x1B[1;31m%d \x1B[0;37mwarpoints on the pkill of \x1B[1;31m%s\x1B[0;37m\n\r", NAME(ch), wpgain, NAME(victim));
		log_string(buf);
	}
 
		check_topten (ch);
	/*End Handle warpoints */
	for (i = 0; i < MAX_TROPHY; i++)
	{
		if (ch->pcdata->trophy_name[i][0] == '\0')
			continue;
		if (!str_cmp (RNAME (victim), ch->pcdata->trophy_name[i]))
		{
			ch->pcdata->trophy_times[i]++;
			ch->pcdata->trophy_level[i] = LEVEL (victim);
			return;
		}
	}
	nummy = free_trophy_slot (ch, victim);
	if (nummy == -1)
		return;
	strcpy (ch->pcdata->trophy_name[nummy], RNAME (victim));
	ch->pcdata->trophy_times[nummy] = 1;
	ch->pcdata->trophy_level[nummy] = LEVEL (victim);
	return;
}
