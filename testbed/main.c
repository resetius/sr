#include <event.h>
#include <evhttp.h>

void gencb(struct evhttp_request * req, void * data)
{
}

int main(int argc, char ** argv)
{
	struct event_base * base = event_init(); 
	struct evhttp * http = evhttp_new(base);
	evhttp_set_gencb(http, gencb, 0);

	evhttp_bind_socket(http, "0.0.0.0", 8083);
	event_base_loop(base, 0);
	return 0;
}

