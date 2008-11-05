#include "tok.h"

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

