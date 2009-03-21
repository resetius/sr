#ifndef MARKOV_H
#define MARKOV_H
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

/*
 * Markov chain random text generator :
 * Copyright (C) 1999 Lucent Technologies
 * Excerpted from 'The Practice of Programming'
 * by Brian W. Kernighan and Rob Pike
 */

#ifdef __cplusplus
extern "C" {
#endif

#define MARKOV_MAXFILES 100
#define MARKOV_MAXPATH 3276

	enum {
		NPREF   = 2,    /* number of prefix words */
		NHASH   = 40930, /* size of state hash table array */
		MAXGEN  = 1000  /* maximum words generated */
	};

	typedef struct State State;
	typedef struct Suffix Suffix;
	typedef struct TextState TextState;
	typedef struct IdealState IdealState;

	struct Suffix { /* list of suffixes */
		const char    *word;                  /* suffix */
		Suffix  *next;                  /* next in list of suffixes */
	};

	struct State {  /* prefix + suffix list */
		const char    *pref[NPREF];   /* prefix words */
		Suffix  *suf;                   /* list of suffixes */
		State   *next;                  /* next in hash table */
	};

	struct TextState {
		State   *statetab[NHASH];       /* hash table of states */
	};

	typedef struct Ideal Ideal;
	
	struct Ideal {
		State ** sub;
		int size;
		int hash_num;
	};

	struct IdealState {
		Ideal * statetab[NHASH];
	};

	extern const char * NONWORD;
	extern TextState  text_state[MARKOV_MAXFILES];
	extern IdealState ideal_state[MARKOV_MAXFILES];
	extern int num_states;

	State* lookup(const char *prefix[NPREF], State   **statetab, int create);
	State * lookup_ideal(const char * prefix[NPREF], Ideal ** ideal);
	void init_markov(const char * text_folder);

#ifdef __cplusplus
}
#endif

#endif /* MARKOV_H */
