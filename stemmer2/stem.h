#ifndef STEM_H
#define STEM_H

class StemmerOut {
public:
	void * h_;

	StemmerOut(): h_(0), size(0), type(0), stem(0) {}
	~StemmerOut();

	enum {
		HUNSPELL = 1,
		SNOWBALL = 2,
	};

	int size;
	int type;

	/**
	 * for (i = 0; i < size; ++i) {
	 *    string = stem[i];
	 *    ...
	 * }
	 */
	char ** stem;
};

class Token;

class Stemmer {
	class PImpl;
	PImpl * impl_;

public:
	Stemmer();
	~Stemmer();

	StemmerOut stem(const Token & tok);
};

#endif /* STEM_H */

