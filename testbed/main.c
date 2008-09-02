#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <pthread.h>

#include <event.h>
#include <evhttp.h>

#include "markov.h"

typedef struct Buffer Buffer;

struct Buffer {
	char * buf;
	unsigned int size;
	unsigned int pos;
};

void buf_append(Buffer * buf, const char * w)
{
	while (*w && (buf->pos < buf->size)) {
		buf->buf[buf->pos ++] = *w++;
	}

	if (buf->pos == buf->size) {
		buf->size *= 2;
		buf->buf   = realloc(buf->buf, buf->size);
		buf_append(buf, w);
	}

	buf->buf[buf->pos] = 0;
}

void buf_append_num(Buffer * buf, int num)
{
	char buf1[256];
	sprintf(buf1, "%d", num);
	buf_append(buf, buf1);
}

/* generate: produce output, one word per line */
//void generate(int nwords, IdealState * state, int links_per_page,
void generate(int nwords, 
		TextState * state, 
//		IdealState * state,
		int links_per_page,
		int links_total, 
		unsigned int * seed,
		//Buffer * buf
		struct evbuffer * buf
		)
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
			if (rand_r(seed) % ++nmatch == 0) /* prob = 1/nmatch */
				w = suf->word;

		if (strcmp(w, NONWORD) == 0)
			break;

		link = (rand_r(seed) < (RAND_MAX / (links_per_page + 1)));

		if (rand_r(seed) < RAND_MAX / 50) {
			if (p_open) {
				evbuffer_add_printf(buf, "</p>\n");
				//buf_append(buf, "</p>\n");
			}
			evbuffer_add_printf(buf, "<p>\n");
			//buf_append(buf, "<p>\n");
			p_open = 1;
		}

		if (link) {
			evbuffer_add_printf(buf, "<a href=\"/%d.html\">%s</a> ",
								(int)(rand_r(seed) % links_total), w);

			/*buf_append(buf, "<a href=\"/");
			buf_append_num(buf, (int)(rand_r(seed) % links_total));
			buf_append(buf, ".html\">");
			buf_append(buf, w); 
			buf_append(buf, "</a> ");*/
		} else {
			//buf_append(buf, w); buf_append(buf, " ");
			evbuffer_add_printf(buf, "%s ", w);
		}

		if (rand_r(seed) < RAND_MAX / 3) {
			//buf_append(buf, "\n");
			evbuffer_add_printf(buf, "\n");
		}
		memmove(prefix, prefix + 1, (NPREF - 1) * sizeof(prefix[0]));
		prefix[NPREF - 1] = w;
	}

	if (p_open) {
		//buf_append(buf, "</p>\n");
		evbuffer_add_printf(buf, "</p>\n");
	}
}

void gencb(struct evhttp_request * req, void * data)
{
	struct evbuffer *answer = evbuffer_new();
	const char * uri = evhttp_request_uri(req);
	unsigned int seed = 0;
	int nwords;

//	Buffer buf;

	if (sscanf(uri, "/%u.html", &seed) != 1) {
		seed = time(0);
	}

	nwords   = rand_r(&seed) % 1000;

//	buf.size = nwords * 10;
//	buf.pos  = 0;
//	buf.buf  = malloc(buf.size);

/*	buf_append(&buf, "<html><head></head><body>\n");
	buf_append(&buf, "<title>");
	buf_append_num(&buf, num);
	buf_append(&buf, "</title>\n");*/

	evbuffer_expand(answer, nwords * 10);
	evbuffer_add_printf(answer, "<html><head></head><body>\n"
			"<title>%d</title>\n", seed);
	generate(nwords,
			 &text_state[rand_r(&seed) % num_states] /* base text */,
			// &ideal_state[rand_r(&seed) % num_states] /* base text */,
			1 + rand_r(&seed) %  50    /* links per page */, 
			1 + rand_r(&seed) % 100000 /* links total    */,
			&seed,
			//&buf
			answer
			);
//	buf_append(&buf, "</body></html>\n");
	evbuffer_add_printf(answer, "</body></html>\n");

//	evbuffer_expand(answer, buf.size);
//	evbuffer_add_printf(answer, buf.buf);

	evhttp_add_header(req->output_headers, "Content-Type", 
			"text/html; charset=windows-1251");
	evhttp_send_reply(req, HTTP_OK, "OK", answer);

//	free(buf.buf);

//	printf("%s\n", req->uri);
//	printf("%s\n", evhttp_find_header(req->input_headers, "Host"));
}

#define THREADS 4

struct Thread {
	struct event_base * base;
	pthread_t id;
	int snd;
	int rcv;
};

void thr_notify(int fd, short ev, void * arg)
{
	char buf[256];
	read(fd, buf, 1);
}

void * run_thr(void * arg)
{
	struct Thread * thread = arg;
	struct event notify;
	int fds[2];
	pipe(fds);
	thread->rcv = fds[0];
	thread->snd = fds[1];

	event_set(&notify, thread->rcv, 
			EV_READ | EV_PERSIST, thr_notify, thread);
	event_base_set(thread->base, &notify);
	event_add(&notify, 0);
	event_base_loop(thread->base, 0);
	return 0;
}

struct HttpState {
	int cur;
	struct Thread * threads;
};

struct event_base * set_base_cb(void * arg)
{
	struct HttpState * state = arg;
	struct event_base * base;
	static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

	pthread_mutex_lock(&mutex);
	base = state->threads[state->cur].base;

	state->cur ++;
	if (state->cur == THREADS) {
		state->cur = 0;
	}
	pthread_mutex_unlock(&mutex);

//	printf("set base %p\n", base);

	return base;
}

int main(int argc, char ** argv)
{
	int i;
	struct Thread threads[THREADS];
	struct event_base *main_base;
	struct evhttp * http;
	struct HttpState state;

	main_base = event_base_new();

	http = evhttp_new(main_base);
	evhttp_set_gencb(http, gencb, 0);

	init_markov("./texts/");

	evhttp_set_base_cb(http, set_base_cb, &state);
	evhttp_bind_socket(http, "0.0.0.0", 8083);

	state.cur = 0;
	state.threads = threads;

	for (i = 0; i < THREADS; ++i) {
		threads[i].base = event_base_new();
		pthread_create(&threads[i].id, 0, run_thr, &threads[i]);
	}

	event_base_loop(main_base, 0);

	for (i = 0; i < THREADS; ++i) {
		pthread_join(threads[i].id, 0);
	}
	
	return 0;
}

