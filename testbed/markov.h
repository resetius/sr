#ifndef MARKOV_H
#define MARKOV_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAXFILES 1000
#define MAXPATH 32768

	enum {
		NPREF   = 2,    /* number of prefix words */
		NHASH   = 4093, /* size of state hash table array */
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
	extern TextState text_state[MAXFILES];
	extern IdealState ideal_state[MAXFILES];
	extern int num_states;

	State* lookup(const char *prefix[NPREF], State   **statetab, int create);
	State * lookup_ideal(const char * prefix[NPREF], Ideal ** ideal);
	void init_markov(const char * text_folder);

#ifdef __cplusplus
}
#endif

#endif /* MARKOV_H */
