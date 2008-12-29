/*
 * Copyright 2008 Alexey Ozeritsky <aozeritsky@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Markov chain random text generator :
 * Copyright (C) 1999 Lucent Technologies
 * Excerpted from 'The Practice of Programming'
 * by Brian W. Kernighan and Rob Pike
 */

/*
 * Ideal hashing algorithm is excerpted from:
 * Introduction to Algorithms, second edition  Introduction to Algorithms, 2/e
 * Thomas H. Cormen, Dartmouth College
 * Charles E. Leiserson, Massachusetts Institute of Technology
 * Ronald L. Rivest, Massachusetts Institute of Technology
 * Clifford Stein, Columbia University
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <assert.h>

#include <pthread.h>

#include <event.h>
#include <evhttp.h>

#include "markov.h"
#include "gen_config.h"

static struct GenConfig config;

/* generate: produce html-output */
void generate(int nwords, 
#ifdef IDEAL_HASHING		
		IdealState * state,
#else
		TextState * state,
#endif
		int intern_links,
		int extern_links,
		int links_total, 
		char * ext_prefix,
		char * ext_suffix,
		int ext_servers,
		unsigned int * seed,
		struct evbuffer * buf
		)
{
	State *sp;
	Suffix *suf;
	const char *prefix[NPREF], *w = 0;
	int i, nmatch;
	int link;
	int ext_link, int_link;
	int p_open = 0;

	for (i = 0; i < NPREF; i++)     /* reset initial prefix */
		prefix[i] = NONWORD;

	for (i = 0; i < nwords; i++) {
#ifdef IDEAL_HASHING
		sp = lookup_ideal(prefix, state->statetab); 
#else
		sp = lookup(prefix, state->statetab, 0);
#endif
		nmatch = 0;
		for (suf = sp->suf; suf != NULL; suf = suf->next)
			if (rand_r(seed) % ++nmatch == 0) /* prob = 1/nmatch */
				w = suf->word;

		if (strcmp(w, NONWORD) == 0)
			break;

		int_link = (rand_r(seed) < (intern_links));
		ext_link = (rand_r(seed) < (extern_links));

		if (rand_r(seed) < RAND_MAX / 50) {
			if (p_open) {
				evbuffer_add_printf(buf, "</p>\n");
			}
			evbuffer_add_printf(buf, "<p>\n");
			p_open = 1;
		}

		if (int_link) {
			evbuffer_add_printf(buf, "<a href=\"/%d.html\">%s</a> ",
					(int)(rand_r(seed) % links_total), w);
		} else if (ext_link) {
			evbuffer_add_printf(buf, "<a href=\"http://%s%d%s/%d.html\">%s</a> ", 
					ext_prefix, 
					rand_r(seed) % ext_servers,
					ext_suffix,
					(int)(rand_r(seed) % links_total), w);
		} else {
			evbuffer_add_printf(buf, "%s ", w);
		}

		if (rand_r(seed) < RAND_MAX / 3) {
			evbuffer_add_printf(buf, "\n");
		}
		memmove(prefix, prefix + 1, (NPREF - 1) * sizeof(prefix[0]));
		prefix[NPREF - 1] = w;
	}

	if (p_open) {
		evbuffer_add_printf(buf, "</p>\n");
	}
}

void gencb(struct evhttp_request * req, void * data)
{
	struct evbuffer *answer = evbuffer_new();
	const char * uri = evhttp_request_uri(req);
	unsigned int seed = 0;
	int nwords;

	if (sscanf(uri, "/%u.html", &seed) != 1) {
		seed = time(0);
	}

	nwords   = rand_r(&seed) % config.words_per_page;

	evbuffer_expand(answer, nwords * 10);
	evbuffer_add_printf(answer, "<html><head></head><body>\n"
			"<title>%u</title>\n", seed);
	generate(nwords,
#ifdef IDEAL_HASHING
			 &ideal_state[rand_r(&seed) % num_states] /* base text */,
#else
			 &text_state[rand_r(&seed) % num_states]  /* base text */,
#endif
			config.intern_links, 
			config.extern_links,
			config.links_total,
			config.extern_links_prefix,
			config.extern_links_suffix,
			config.extern_links_servers,
			&seed,
			answer
			);
	evbuffer_add_printf(answer, "</body></html>\n");

	evhttp_add_header(req->output_headers, "Content-Type", 
			"text/html; charset=windows-1251");
	evhttp_send_reply(req, HTTP_OK, "OK", answer);
	evbuffer_free(answer);
}

void * run_thr(void * arg)
{
	struct event_base * base = arg;
	int ret;

	printf("base %p started\n", base);

	ret = event_base_loop(base, 0);

	fprintf(stderr, "sipez %d, %d, %s\n", 
			ret, errno, strerror(errno));
	exit(1);
	return 0;
}

int main(int argc, char ** argv)
{
	int i;
	int nthreads = 1;
	pthread_t * threads;
	struct event_base *main_base;
	struct evhttp * http;

	load_config(&config, "gen.ini");

	nthreads = config.worker_threads;
	if (nthreads <= 0) nthreads = 1;
	threads = malloc(nthreads * sizeof(pthread_t));

	main_base = event_base_new();

	http = evhttp_new(main_base);

	init_markov("./texts/");

	for (i = 0; i < nthreads; ++i) {
		pthread_create(&threads[i], 0, run_thr, evhttp_add_worker(http));
	}

	evhttp_set_gencb(http, gencb, 0);

	evhttp_bind_socket(http, "0.0.0.0", config.daemon_port);

	event_base_loop(main_base, 0);

	for (i = 0; i < nthreads; ++i) {
		pthread_join(threads[i], 0);
	}

	free(threads);

	return 0;
}

