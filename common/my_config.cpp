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

#include <stdio.h>
#include <string.h>

#include <map>
#include <string>
#include <list>

#include "my_config.h"

using namespace std;

config_data_t
config_load(const string & file)
{
	config_data_t r;
#define BUF_SZ 4096
	char buf[BUF_SZ];
	char section[BUF_SZ];
	const char * sep = " =\t";

	config_section_t * cur = 0;
	FILE * f = fopen(file.c_str(), "rb");

	if (!f) {
		fprintf(stderr, "cannot open %s\n", file.c_str());
		return r;
	}

	fprintf(stderr, "loading values from %s\n", file.c_str());

	while (fgets(buf, BUF_SZ, f)) {
		if (sscanf(buf, "[%s]", section) == 1) {
			section[strlen(section) - 1] = 0;
			//fprintf(stderr, "section -> %s\n", section);
			cur = &r[section];
		} else if (*buf == '#') {
			continue;
		} else if (cur) {
			char * k, * v;
			k = strtok(buf, sep);
			v = strtok(0, sep);

			//fprintf(stderr, "%s:%s\n", k, v);
			if (k && v) {
				(*cur)[k] = v;
			}
		}
	}

	fclose(f);

	return r;
#undef BUF_SZ
}

void
config_try_set_int(config_data_t & r, const string & section,
				   const string & key, int & val)
{
	if (r[section].find(key) != r[section].end()) {
		val = atoi(r[section][key].c_str());
	}
}
