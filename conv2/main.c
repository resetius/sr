/*$Id$*/

/* Copyright (c) 2008 Alexey Ozeritsky (Алексей Озерицкий)
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Alexey Ozeritsky.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
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

/**
 * Конвертер испорченных файлов.
 * Файл был в utf-8, отобразили в cp1251 и сохранили.
 * Исправляем эту ситуацию.
 * Потерянные данные пытаемся восстановить с помощью Hunspell
 */

#include <glib.h>
#include <string.h>
#include <hunspell/hunspell.h>

#include "tokenizer.h"

#define ALNUM 256
#define ALPHA 257
#define DIGIT 258
#define PUNCT 259
#define BLANK 260
#define RUS 261 

extern char * state;

int word2int(gunichar * ret, char * mask, char * word, char * from)
{
	int i = 0;
	const char * p;

	if (!g_utf8_validate(word, -1, 0)) {
		return 0;
	}

	for (p = word; *p; p = g_utf8_next_char(p)) {
		gunichar c = g_utf8_get_char(p);
		ret[i] = c;

		if (mask && *p == '?') {
			mask[i] = 1;
		}
		++i;
	}

	return i;
}

char * check(char * word, Hunhandle * h)
{
	int n, i, j;
	int n1, n2;
	int l;

	gunichar * iword;
	gunichar * isug;

	char * mask;
	char ** sug;
	char * ret = 0;

	if (!strcmp(word, "?")) {
		free(word);
		return strdup("и");
	}

	if (Hunspell_spell(h, word)) {
		return word;
	}

	if (!strstr(word, "?")) {
		return word;
	}

	n = Hunspell_suggest(h, &sug, word);

	if (n == 0) {
		return word;
	}

	l = strlen(word);
	iword = alloca(l * sizeof(gunichar));
	isug  = alloca(l * sizeof(gunichar));
	mask  = alloca(l); memset(mask, 0, l);

	n1 = word2int(iword, mask, word, 0);
	if (n1 == 0) {
		return word;
	}

	for (i = 0; i < n; ++i) {
		int ok = 1;
		n2 = word2int(isug, 0, sug[i], 0);
		if (n2 == 0 || n1 != n2) continue;


		for (j = 0; j < n1; ++j) {
			if (mask[j] == 0 && iword[j] != isug[j]) {
				ok = 0;
				break;
			} else if (mask[j] && !(isug[j] == g_utf8_get_char("и") ||
									isug[j] == g_utf8_get_char("И") ||
									isug[j] == g_utf8_get_char("ш") ||
									isug[j] == g_utf8_get_char("Ш")))
			{
				ok = 0;
				break;
			}
		}

		if (ok) {
			/* bingo ! */
			fprintf(stderr, "bingo %s->%s\n", word, sug[i]);
			ret = strdup(sug[i]);
			break;
		}
	}

	Hunspell_free_list(h, &sug, n);

	if (ret == 0) return word; 

	free(word);
	return ret;
}

char * try_convert(char * word, Hunhandle * h)
{
	GError * e = 0;
	char * ans1, * ans2;
	int rd, wd;
	int l = strlen(word);
	char * sep = "?";
	char * w;
	char * buf = malloc(l * 10);

	int flag = (word[l - 1] == '?');

	buf[0] = 0;

	if (Hunspell_spell(h, word)) {
		/* OK */
		return strdup(word);
	}

	for (w = strtok(word, sep); w; w = strtok(0, sep)) {
		ans1 = g_convert_with_fallback(w, strlen(w), "cp1251", "utf-8", "?",
									   &rd, &wd, &e);
		if (e) { return strdup(word); }
		e = 0;

		ans2 = g_convert_with_fallback(ans1, strlen(ans1), "cp1251", "utf-8",
									   "?", &rd, &wd, &e);
		free(ans1);

		if (e) { return strdup(word); }
		e = 0;
		ans1 = g_convert_with_fallback(ans2, strlen(ans2), "utf-8",
									   "cp1251", "?",
									   &rd, &wd, &e);
		free(ans2);

		if (e) { return strdup(word); }

		strcat(buf, ans1); strcat(buf, "?");
		free(ans1);
	}

	if (!flag) {
		buf[strlen(buf) - 1] = 0;
	}

	return check(buf, h);

fail:
	return strdup(word);
}

void do_all(const char * dpath, int mode)
{
	int result;
	char * nw;
	Hunhandle * h = 0;
	char * ru_aff = malloc(strlen(dpath) + 15);
	char * ru_dic = malloc(strlen(dpath) + 15);

	strcpy(ru_aff, dpath); strcat(ru_aff, "ru_RU.aff");
	strcpy(ru_dic, dpath); strcat(ru_dic, "ru_RU.dic");

	h = Hunspell_create(ru_aff, ru_dic);

	if (!h) {
		fprintf(stderr, "cannot initialize Hunspell\n");
		fprintf(stderr, "%s-%s\n", ru_aff, ru_dic);
		goto err;
	}

	while ((result = toklex()) != 0) {
		switch (result) {
		case ALNUM:
			printf("%s", state);
			break;
		case ALPHA:
			printf("%s", state);
			break;
		case DIGIT:
			printf("%s", state);
			break;
		case BLANK:
			printf("%s", state);
			break;
		case RUS:
			nw = try_convert(state, h);
			printf("%s", nw);
			free(nw);
			break;
		default:
			printf("%s", state);
			break;
		}
	}

err:
	if (h) Hunspell_destroy(h);
	free(ru_aff); free(ru_dic);
}

static void
usage(const char * name)
{
	fprintf(stderr, "usage: %s [--dpath path_to_dicts] [--mode mode]\n", name);
	exit(1);
}

int main(int argc, char * argv[])
{
	int mode = 0, i;
	const char * dict_path = "./";
	for (i = 0; i < argc; ++i) {
		if (!strcmp(argv[i], "--dpath")) {
			if (i < argc - 1) {
				dict_path = argv[i + 1];
			} else {
				usage(argv[0]);
			}
		} else if (!strcmp(argv[i], "--mode")) {
			if (i < argc - 1) {
				mode = atoi(argv[i + 1]);
			} else {
				usage(argv[0]);
			}
		}
	}

	do_all(dict_path, mode);
	return 0;
}
