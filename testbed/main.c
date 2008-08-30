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
			if (rand() % ++nmatch == 0) /* prob = 1/nmatch */
				w = suf->word;

		if (strcmp(w, NONWORD) == 0)
			break;

		link = (rand() < (RAND_MAX / (links_per_page + 1)));

		if (rand() < RAND_MAX / 50) {
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
								(int)(rand() % links_total), w);

			/*buf_append(buf, "<a href=\"/");
			buf_append_num(buf, (int)(rand() % links_total));
			buf_append(buf, ".html\">");
			buf_append(buf, w); 
			buf_append(buf, "</a> ");*/
		} else {
			//buf_append(buf, w); buf_append(buf, " ");
			evbuffer_add_printf(buf, "%s ", w);
		}

		if (rand() < RAND_MAX / 3) {
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
	int num = 0;
	int nwords;
//	Buffer buf;

	if (sscanf(uri, "/%d.html", &num) != 1) {
		num = time(0);
	}
	srand(num);
	nwords   = rand() % 1000;
//	buf.size = nwords * 10;
//	buf.pos  = 0;
//	buf.buf  = malloc(buf.size);

/*	buf_append(&buf, "<html><head></head><body>\n");
	buf_append(&buf, "<title>");
	buf_append_num(&buf, num);
	buf_append(&buf, "</title>\n");*/

	evbuffer_expand(answer, nwords * 10);
	evbuffer_add_printf(answer, "<html><head></head><body>\n"
			"<title>%d</title>\n", num);
	generate(nwords,
			 &text_state[rand() % num_states] /* base text */,
			// &ideal_state[rand() % num_states] /* base text */,
			1 + rand() %  50    /* links per page */, 
			1 + rand() % 100000 /* links total    */,
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

