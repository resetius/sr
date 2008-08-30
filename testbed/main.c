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


#define MAXFILES 1000
#define MAXPATH 32768

enum {
	NPREF   = 2,    /* number of prefix words */
//	NHASH   = 4093, /* size of state hash table array */
	NHASH   = 500000,
	MAXGEN  = 1000  /* maximum words generated */
};

typedef struct State State;
typedef struct Suffix Suffix;

struct State {  /* prefix + suffix list */
	char    *pref[NPREF];   /* prefix words */
	Suffix  *suf;                   /* list of suffixes */
	State   *next;                  /* next in hash table */
};

struct Suffix { /* list of suffixes */
	char    *word;                  /* suffix */
	Suffix  *next;                  /* next in list of suffixes */
};

char NONWORD[] = "\n";  /* cannot appear as real word */


typedef struct TextState TextState;
struct TextState {
	State   *statetab[NHASH];       /* hash table of states */
};

int num_states = 0;
TextState text_state[MAXFILES];

const int MULTIPLIER = 31;  /* for hash() */

/* hash: compute hash value for array of NPREF strings */
unsigned int hash(char *s[NPREF])
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
State* lookup(char *prefix[NPREF], State   **statetab, int create)
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
void addsuffix(State *sp, char *suffix)
{
	Suffix *suf;

	suf = (Suffix *) malloc(sizeof(Suffix));
	suf->word = suffix;
	suf->next = sp->suf;
	sp->suf = suf;
}

/* add: add word to suffix list, update prefix */
void add(char *prefix[NPREF], TextState * state, char *suffix)
{
	State *sp;

	sp = lookup(prefix, state->statetab, 1);  /* create if not found */
	addsuffix(sp, suffix);
	/* move the words down the prefix */
	memmove(prefix, prefix+1, (NPREF-1)*sizeof(prefix[0]));
	prefix[NPREF-1] = suffix;
}

/* build: read input, build prefix table */
void build_markov(char *prefix[NPREF], TextState * state, FILE *f)
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
	char *prefix[NPREF];            /* current input prefix */
	for (i = 0; i < NPREF; i++)     /* set up initial prefix */
		prefix[i] = NONWORD;

	f = fopen(buf, "r");
	if (!f) {
		fprintf(stderr, "cannot read %s\n", buf);
		exit(1);
	}

	build_markov(prefix, &text_state[num], f);
	add(prefix, &text_state[num], NONWORD);

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

/* generate: produce output, one word per line */
void generate(int nwords, TextState * state, int links_per_page, int links_total, struct evbuffer *answer)
{
	State *sp;
	Suffix *suf;
	char *prefix[NPREF], *w = 0;
	int i, nmatch;
	int link;
	int p_open = 0;

	for (i = 0; i < NPREF; i++)     /* reset initial prefix */
		prefix[i] = NONWORD;

	for (i = 0; i < nwords; i++) {
		sp = lookup(prefix, state->statetab, 0);
		nmatch = 0;
		for (suf = sp->suf; suf != NULL; suf = suf->next)
			if (rand() % ++nmatch == 0) /* prob = 1/nmatch */
				w = suf->word;

		if (strcmp(w, NONWORD) == 0)
			break;

		link = (rand() < (RAND_MAX / (links_per_page + 1)));

		if (rand() < RAND_MAX / 50) {
			if (p_open) {
				evbuffer_add_printf(answer, "</p>\n");
			}
			evbuffer_add_printf(answer, "<p>\n");
			p_open = 1;
		}

		if (link) {
			evbuffer_add_printf(answer, "<a href=\"/%d.html\">", (int)(rand() % links_total));
			evbuffer_add_printf(answer, "%s ", w);
			evbuffer_add_printf(answer, "</a>");
		} else {
			evbuffer_add_printf(answer, "%s ", w);
		}

		if (rand() < RAND_MAX / 3) evbuffer_add_printf(answer, "\n");
		memmove(prefix, prefix + 1, (NPREF - 1) * sizeof(prefix[0]));
		prefix[NPREF - 1] = w;
	}

	if (p_open) {
		evbuffer_add_printf(answer, "</p>\n");
	}
}

void gencb(struct evhttp_request * req, void * data)
{
	struct evbuffer *buf = evbuffer_new();
	const char * uri = evhttp_request_uri(req);
	int num = 0;

//	evbuffer_add_printf(buf, "Requested: %s\n", uri);

	if (sscanf(uri, "/%d.html", &num) != 1) {
		num = time(0);
	}
	srand(num);

	evbuffer_add_printf(buf, "<html><head></head><body>\n");
	evbuffer_add_printf(buf, "<title>%d</title>\n", num);

	generate(rand() % 1000 /*words*/ , 
			&text_state[rand() % num_states] /* base text */, 
			1 + rand() %  50    /* links per page */, 
			1 + rand() % 100000 /* links total    */,
			buf);
	evbuffer_add_printf(buf, "</body></html>\n");

//	evbuffer_add_printf(buf, "test");

	evhttp_add_header(req->output_headers, "Content-Type", "text/html; charset=windows-1251");
	evhttp_send_reply(req, HTTP_OK, "OK", buf);

//	printf("%s\n", req->uri);
//	printf("%s\n", evhttp_find_header(req->input_headers, "Host"));
}

void * run_thr(void * arg)
{
	event_base_loop((struct event_base *)arg, 0);
	return 0;
}

int main(int argc, char ** argv)
{
	struct event_base * base1 = event_base_new(); 
	struct event_base * base2 = event_base_new();
	struct event_base *bases[3];
	bases[0] = base1;
	bases[1] = base2;
	bases[2] = 0;

	struct evhttp * http1 = evhttp_new(base1);
//	struct evhttp * http2 = evhttp_new(base2);

	pthread_t th1, th2;

	evhttp_set_gencb(http1, gencb, 0);
//	evhttp_set_gencb(http2, gencb, 0);

	init_markov("./texts/");

	evhttp_bind_socket(http1, "0.0.0.0", 8083);
//	evhttp_bind_socket(http2, "0.0.0.0", 8083);

	//event_base_loop(base, 0);

	pthread_create(&th1, 0, run_thr, base1);
//	pthread_create(&th2, 0, run_thr, base2);

	pthread_join(th1, 0);
//	pthread_join(th2, 0);

	return 0;
}

