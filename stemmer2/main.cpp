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

