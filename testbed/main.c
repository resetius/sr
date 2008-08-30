#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <pthread.h>

#include <event.h>
#include <evhttp.h>

#include "markov.h"

/* generate: produce output, one word per line */
//void generate(int nwords, IdealState * state, int links_per_page,
void generate(int nwords, TextState * state, int links_per_page,
			  int links_total, struct evbuffer *answer)
{
	State *sp;
	Suffix *suf;
	const char *prefix[NPREF], *w = 0;
	int i, nmatch;
	int link;
	int p_open = 0;

	for (i = 0; i < NPREF; i++)     /* reset initial prefix */
		prefix[i] = NONWORD;

	for (i = 0; i < nwords; i++) {
		sp = lookup(prefix, state->statetab, 0);
//		sp = lookup_ideal(prefix, state->statetab); 
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
			 //&ideal_state[rand() % num_states] /* base text */,
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

