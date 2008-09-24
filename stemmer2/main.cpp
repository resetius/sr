#include <stdio.h>
#include <string.h>
#include <hunspell/hunspell.hxx>
#include "libstemmer.h"

extern "C" {
#include "tokenizer.h"
}

#define ALNUM 256
#define ALPHA 257
#define DIGIT 258
#define PUNCT 259
#define BLANK 260
#define RUS 261 

extern char * state;

void do_stem(Hunspell * h, struct sb_stemmer * sb, const char * word)
{
	char ** stem;
	int i, n;

	if (h) {
		n = h->stem(&stem, word);

		if (n) {
			printf("[%s: ", word);
			for (i = 0; i < n; ++i) {
				printf("%s, ", stem[i]);
			}
			printf("]");
		} else {
			/* snowball */
			printf("[%s: ", word);
			printf("%s", (const char *)sb_stemmer_stem(sb, (const sb_symbol*)word, strlen(word)));
			printf("]");
		}

		h->free_list(&stem, n);
	}
}

void do_all(Hunspell * h_ru, struct sb_stemmer * sb_ru, Hunspell * h, struct sb_stemmer * sb)
{
	int result;

	while ((result = toklex()) != 0) {
		switch (result) {
		case ALNUM:
			printf("%s", state);
			break;
		case ALPHA:
			do_stem(h, sb, state);
			break;
		case DIGIT:
			printf("%s", state);
			break;
		case BLANK:
			printf("%s", state);
			break;
		case RUS:
			do_stem(h_ru, sb_ru, state);
			break;
		default:
			printf("%s", state);
			break;
		}
	}
}

int main(int agrc, char * argv[])
{
	Hunspell * h_ru = new Hunspell("ru_RU.aff", "ru_RU.dic");
	Hunspell * h_en = new Hunspell("en_US.aff", "en_US.dic");
	struct sb_stemmer * sb_ru = sb_stemmer_new("ru", "UTF_8");
	struct sb_stemmer * sb_en = sb_stemmer_new("en", "UTF_8");

	do_all(h_ru, sb_ru, h_en, sb_en);

	delete h_ru;
	delete h_en;

	if (sb_ru) sb_stemmer_delete(sb_ru);
	if (sb_en) sb_stemmer_delete(sb_en);

	return 0;
}

