
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
config_try_set_int(config_data_t & r, const string & section, const string & key, int & val)
{
	if (r[section].find(key) != r[section].end()) {
		val = atoi(r[section][key].c_str());
	}
}


