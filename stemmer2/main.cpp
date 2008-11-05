#include <stdio.h>

#include "stem.h"
#include "tok.h"


void do_all()
{
	Stemmer s;
	Tokenizer t(0);
	Token tok;

	while (t.next(tok)) {
		switch (tok.type) {
		case Token::ALNUM:
			printf("%s", tok.word);
			break;
		case Token::DIGIT:
			printf("%s", tok.word);
			break;
		case Token::BLANK:
			printf("%s", tok.word);
			break;
		case Token::ALPHA:
		case Token::RUS:
		{
			StemmerOut out = s.stem(tok);
			printf("[%s: ", tok.word);
			for (int i = 0; i < out.size; ++i) {
				printf("%s, ", out.stem[i]);
			}
			printf(":%d", out.type);
			printf("]");
			break;
		}
		default:
			printf("%s", tok.word);
			break;
		}
	}
}

int main(int agrc, char * argv[])
{
	do_all();

	return 0;
}

