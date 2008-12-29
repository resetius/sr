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

#include <stdlib.h>
#include "my_config.h"
#include "gen_config.h"

static void
load_defaults(struct GenConfig * conf)
{
	conf->daemon_port    = 8083;
	conf->words_per_page = 1000;
	conf->links_per_page = 50;
	conf->links_total    = 100000;
	conf->worker_threads = 1;
}

static void
print_config(struct GenConfig * conf)
{
	fprintf(stderr, "daemon_port %d\n",     conf->daemon_port);
	fprintf(stderr, "words_per_page %d\n",  conf->words_per_page);
	fprintf(stderr, "links_per_page %d\n",  conf->links_per_page);
	fprintf(stderr, "links_per_page %lf\n", conf->links_per_page_v);
	fprintf(stderr, "links_total %d\n",     conf->links_total);
	fprintf(stderr, "worker_threads %d\n",  conf->worker_threads);
}

void load_config(struct GenConfig * conf, const char * config_name)
{
	load_defaults(conf);
	config_data_t c = config_load(config_name);
	config_try_set_int(c, "generator", "daemon_port",       conf->daemon_port);
	config_try_set_int(c, "generator", "words_per_page",    conf->words_per_page);
	config_try_set_double(c, "generator", "links_per_page", conf->links_per_page_v);
	config_try_set_int(c, "generator", "links_total",       conf->links_total);
	config_try_set_int(c, "generator", "worker_threads",    conf->worker_threads);

	if (conf->links_per_page_v <= 0 || conf->links_per_page_v >= 1.0) {
		conf->links_per_page_v = 0.1;
	}

	conf->links_per_page = (int)((double)RAND_MAX * conf->links_per_page_v);
	print_config(conf);
}

