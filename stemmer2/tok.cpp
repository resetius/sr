#include "tok.h"
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

extern "C" {
#include "tokenizer.h"
}

extern char * state;

using namespace std;

class Tokenizer::PImpl {
public:
	YY_BUFFER_STATE buf_state;

	PImpl() : buf_state(0) {}

	~PImpl() {
		if (buf_state) tok_delete_buffer(buf_state);
	}
};

Tokenizer::Tokenizer(const string & string2parse)
{
	impl_ = new PImpl;

	impl_->buf_state = tok_scan_string(string2parse.c_str());
}

Tokenizer::Tokenizer(FILE * f)
{
	impl_ = new PImpl;

	if (f) tokrestart(f);
}

Tokenizer::~Tokenizer()
{
	delete impl_;
}

bool Tokenizer::next(Token & tok)
{
	int result = toklex();
	if (!result) {
		if (impl_->buf_state) { tok_delete_buffer(impl_->buf_state); impl_->buf_state = 0; }
		return false;
	}

	tok.type = result;
	tok.word = state;
	return true;
}

