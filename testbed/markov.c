#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include <event.h>
#include <evhttp.h>

#include "markov.h"

const char * NONWORD = "\n";  /* cannot appear as real word */
int num_states = 0;
TextState text_state[MAXFILES];

const int MULTIPLIER = 31;  /* for hash() */

/* hash: compute hash value for array of NPREF strings */
unsigned int hash(const char *s[NPREF])
{
	unsigned int h;
	unsigned char *p;
	int i;

	//h = 5381;
	h = 0;
	for (i = 0; i < NPREF; i++)
		for (p = (unsigned char *) s[i]; *p != '\0'; p++)
			h = MULTIPLIER * h + *p;
			//h = (h << 5) + h + *p;
	return h % NHASH;
}

/* lookup: search for prefix; create if requested. */
/*  returns pointer if present or created; NULL if not. */
/*  creation doesn't strdup so strings mustn't change later. */
State* lookup(const char *prefix[NPREF], State   **statetab, int create)
{
	int i, h;
	State *sp;

	h = hash(prefix);
	for (sp = statetab[h]; sp != NULL; sp = sp->next) {
		for (i = 0; i < NPREF; i++)
			if (strcmp(prefix[i], sp->pref[i]) != 0)
				break;
		if (i == NPREF)         /* found it */
			return sp;

//		printf("collision\n");
	}
	
	if (create) {
		sp = (State *) malloc(sizeof(State));
		for (i = 0; i < NPREF; i++)
			sp->pref[i] = prefix[i];
		sp->suf = NULL;
		sp->next = statetab[h];
		statetab[h] = sp;
	}
	return sp;
}

/* addsuffix: add to state. suffix must not change later */
void addsuffix(State *sp, const char *suffix)
{
	Suffix *suf;

	suf = (Suffix *) malloc(sizeof(Suffix));
	suf->word = suffix;
	suf->next = sp->suf;
	sp->suf = suf;
}

/* add: add word to suffix list, update prefix */
void add(const char *prefix[NPREF], TextState * state, const char *suffix)
{
	State *sp;

	sp = lookup(prefix, state->statetab, 1);  /* create if not found */
	addsuffix(sp, suffix);
	/* move the words down the prefix */
	memmove(prefix, prefix+1, (NPREF-1)*sizeof(prefix[0]));
	prefix[NPREF-1] = suffix;
}

/* build: read input, build prefix table */
void build_markov(const char *prefix[NPREF], TextState * state, FILE *f)
{
	char buf[100], fmt[10];
	/* create a format string; %s could overflow buf */
	sprintf(fmt, "%%%lds", sizeof(buf)-1);
	while (fscanf(f, fmt, buf) != EOF)
		add(prefix, state, strdup(buf));
}

void init_file(const char * buf, int num)
{
	int i;
	FILE * f;
	const char *prefix[NPREF];            /* current input prefix */
	for (i = 0; i < NPREF; i++)     /* set up initial prefix */
		prefix[i] = (char*)NONWORD;

	f = fopen(buf, "r");
	if (!f) {
		fprintf(stderr, "cannot read %s\n", buf);
		exit(1);
	}

	build_markov(prefix, &text_state[num], f);
	add(prefix, &text_state[num], (char*)NONWORD);

	fclose(f);
}

void init_markov(const char * text_folder)
{
	DIR *dp;
	struct dirent *dir_entry;
	struct stat stat_info;
	char buf[MAXPATH];
	int num = 0;

	if ((dp = opendir(text_folder)) == NULL) {
		fprintf(stderr, "cannot read folder %s\n", text_folder);
		exit(-1);
	}

	while ((dir_entry = readdir(dp)) != NULL) {
		int err; 
		strcpy(buf, text_folder);
		strcat(buf, dir_entry->d_name);

		if ((err = lstat(buf, &stat_info)) != 0) {
			fprintf(stderr, "cannot stat %s\n", buf);
			continue;
		}

		if (S_ISREG(stat_info.st_mode)) {
			fprintf(stderr, "loading %s\n", buf);
			init_file(buf, num ++);
		}
	}

	num_states = num;
}
