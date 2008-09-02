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
IdealState ideal_state[MAXFILES];

const int MULTIPLIER = 31;  /* for hash() */

/* hash: compute hash value for array of NPREF strings */
static unsigned long hash_(const char *s[NPREF], int nhash, int mult)
{
	unsigned long h;
	unsigned char *p;
	int i;

	h = 0;
	for (i = 0; i < NPREF; i++) {
		for (p = (unsigned char *) s[i]; *p != '\0'; p++) {
			h = mult * h + *p;
		}
		h = mult * h + 1;
	}

	return h % nhash;
}

static unsigned long hash(const char *s[NPREF])
{
	unsigned long h;
	unsigned char *p;
	int i;

	h = 5381;
	for (i = 0; i < NPREF; i++) {
		for (p = (unsigned char *) s[i]; *p != '\0'; p++) {
			h = (h << 5) + h + *p;
		}
		h = (h << 5) + h + 1;
	}
	return h % NHASH;
}

/* lookup: search for prefix; create if requested. */
/*  returns pointer if present or created; NULL if not. */
/*  creation doesn't strdup so strings mustn't change later. */
State* lookup(const char *prefix[NPREF], State   **statetab, int create)
{
	int i;
	unsigned long h;
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

State * lookup_ideal(const char * prefix[NPREF], Ideal ** ideal)
{
	unsigned long h1;
	unsigned long h2;
	Ideal * i;

	h1 = hash(prefix);

	i  = ideal[h1];
	h2 = hash_(prefix, i->size, i->hash_num);

	if (i->sub[h2] == 0) {
		fprintf(stderr, "%s %s\n", prefix[0], prefix[1]);
	}
	return i->sub[h2];
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
static void add(const char *prefix[NPREF], TextState * state, const char *suffix)
{
	State *sp;

	sp = lookup(prefix, state->statetab, 1);  /* create if not found */
	addsuffix(sp, suffix);
	/* move the words down the prefix */
	memmove(prefix, prefix+1, (NPREF-1)*sizeof(prefix[0]));
	prefix[NPREF-1] = suffix;
}

/* build: read input, build prefix table */
static void build_markov(const char *prefix[NPREF], TextState * state, FILE *f)
{
	char buf[100], fmt[10];
	/* create a format string; %s could overflow buf */
	sprintf(fmt, "%%%lds", sizeof(buf)-1);
	while (fscanf(f, fmt, buf) != EOF)
		add(prefix, state, strdup(buf));
}

static Ideal * ideal_hashing_(State * state)
{
	State * p;
	Ideal * r = malloc(sizeof(Ideal));
	int size1 = 0;
	int size  = 0;
	int mult  = 1;
	int col   = 0;

	for (p = state; p != 0; p = p->next)
	{
		size1 += 1;
	}

	size = size1 * 10;
	
	r->sub  = malloc(size * sizeof(State *));
	r->size = size;
	
	memset(r->sub, 0, size * sizeof(State *));

//	fprintf(stderr, " size: %d\n", size);
	
	//check 100 hash functions
	for (; mult < 10000; ++mult) {
		r->hash_num = mult;

		col = 0;
		for (p = state; p != 0; p = p->next) {
			unsigned long h = hash_(p->pref, size, mult);
			if (r->sub[h]) {
				//collision
				memset(r->sub, 0, size * sizeof(State *));
				col = 1;
			//	fprintf(stderr, "h = %u\n", h);
				break;
			}

			r->sub[h] = p;
		}

		if (col == 0) {
			//found !
			break;
		}
	}

	if (col == 1) {
		fprintf(stderr, "not found size1, size, hash: %d, %d, %d\n", 
				size1, size, mult);
		for (p = state; p != 0; p = p->next) {
			fprintf(stderr, "'%s %s'\n", p->pref[0], p->pref[1]);
		}
	}
//	exit(1);
	
//	fprintf(stderr, "found size, hash: %d, %d\n", size, mult);
	return r;
}

static void ideal_hashing(State *statetab[NHASH], int num)
{
	int i;
	for (i = 0; i < NHASH; ++i)
	{
		if (!statetab[i]) {
			ideal_state[num].statetab[i] = 0;
			continue;
		}

		ideal_state[num].statetab[i] = ideal_hashing_(statetab[i]);
	}
}

static void init_file(const char * buf, int num)
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

//	fprintf(stderr, "ideal hashing\n");
	ideal_hashing(text_state[num].statetab, num);
	fprintf(stderr, "ideal hashing done\n");
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

	fprintf(stderr, "server started\n");
}
