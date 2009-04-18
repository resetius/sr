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
#include <string.h>
#include <hunspell/hunspell.hxx>
#include <assert.h>

#include "libstemmer.h"

#include "stem.h"
#include "tok.h"

StemmerOut::~StemmerOut()
{
	if (h_) {
		Hunspell * hs = reinterpret_cast < Hunspell * > (h_);
		hs->free_list(&stem, size);
	}
}

class Stemmer::PImpl
{
public:
	Hunspell * h_ru;
	Hunspell * h_en;

	struct sb_stemmer * sb_ru;
	struct sb_stemmer * sb_en;
	
	PImpl() {
		h_ru = new Hunspell("ru_RU.aff", "ru_RU.dic");
		h_en = new Hunspell("en_US.aff", "en_US.dic");

		sb_ru = sb_stemmer_new("ru", "UTF_8");
		sb_en = sb_stemmer_new("en", "UTF_8");
	}

	~PImpl() {
		delete h_ru;
		delete h_en;

		if (sb_ru) sb_stemmer_delete(sb_ru);
		if (sb_en) sb_stemmer_delete(sb_en);
	}

	void do_stem(StemmerOut & ret, Hunspell * h, struct sb_stemmer * sb, const char * word) {
		char ** stem = 0;
		int i, n = 0;

		if (h) {
			n = h->stem(&stem, word);

			if (n) {
				ret.type = StemmerOut::HUNSPELL;
				ret.size = n;
				ret.stem = stem;
				ret.h_   = h;
			} else {
				ret.type = StemmerOut::SNOWBALL;
				ret.size = 1;
				ret.stem = (char**)malloc(sizeof(char *));
				ret.stem[0] = (char *)sb_stemmer_stem(sb, (const sb_symbol*)word, strlen(word));
			}
		}
	}
};

Stemmer::Stemmer()
{
	impl_ = new PImpl();
}

Stemmer::~Stemmer()
{
	delete impl_;
}

StemmerOut Stemmer::stem(const Token & tok)
{
	StemmerOut ret;

	switch (tok.type) {
	case Token::ALPHA:
		impl_->do_stem(ret, impl_->h_en, impl_->sb_en, tok.word);
		break;
	case Token::RUS:
		impl_->do_stem(ret, impl_->h_ru, impl_->sb_ru, tok.word);
		break;
	default:
		assert(0);
		break;
	}

	return ret;
}
