#define MAX_OPTION 4
#define POLL_FILE  "polls.dat"

typedef struct poll_data POLL_DATA;
struct poll_data
{
  POLL_DATA *next;
  char *sender;			/* Who set it up */
  char *date;			/* Date it was set up */
  char *to_list;		/* List of people it is to */
  char *subject;		/* Subject of the poll */
  char *text;			/* The actual text */
  time_t date_stamp;		/* Date stamp */
  bool open;			/* Open or closed */
  bool publicstats;		/* Are stats public */
  char *options[MAX_OPTION];	/* Options */
  int votes[MAX_OPTION];	/* Votes for each option */
  int level;			/* Level of poll */
};

typedef struct poll_stamp_list POLL_STAMP_LIST;
struct poll_stamp_list
{
  POLL_STAMP_LIST *next;
  time_t stamp;			/* Defines the poll */
  short choice;			/* What did the player choose */
};
