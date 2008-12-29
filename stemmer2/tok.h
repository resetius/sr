#ifndef TOK_H
#define TOK_H

#include <stdio.h>
#include <string>

struct Token {
	enum {
		ALNUM = 256, // example: abcd124fff
		ALPHA = 257, // example: qwertyadff
		DIGIT = 258, // example: 234134
		PUNCT = 259, // example: .,;: ...
		BLANK = 260, // example: ' \t\n\r'
		RUS   = 261, // example: 'тест тест йцукен'
	};

	int type;
	const char * word;  // указатель на память внутри
};

/* TODO: нельзя создать два токенайзера */
class Tokenizer {
	class PImpl;
	PImpl * impl_;

public:
	Tokenizer(const std::string & str); // scan string
	Tokenizer(FILE * f);           // scan file

	~Tokenizer();

	bool next(Token & tok);
};

#endif /* TOK_H */

