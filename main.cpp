#include "tokenizer.hpp"
#include <exception>
using namespace std;


struct AST : public TokenHelpers {
	Tokenizer tok;

	int parse() {
		tok.flag_eol = 1;
		tok.tokenize("simplebasic.ast");
		tok.show();
		// parse lines
		while (!tok.eof())
			if (tok.peek() == "$EOL")  tok.get();
			else if (!prule())  return false;
		return true;
	}

	int error(const string& msg) {
		// printf("error: parsing rule: %s (line %d)\n", msg.c_str(), tok.linepos());
		throw runtime_error("error: " + msg
			+ " (line " + to_string(tok.linepos()) + ")" );
		return false;
	}

	int prule() {
		string rulename;
		if (!prulename(rulename))  return false;
		if (!psexpr())  return false;
		if ( tok.peek() != "$EOL" )  return error("expected $EOL after rule definition s-expression");
		return true;
	}

	int prulename(string& rulename) {
		// int pos = tok.pos;
		if ( !isidentifier(tok.peek()) )  goto err;
		rulename = tok.get();
		if ( tok.peek() != ":" )  goto err;
		tok.get();
		if ( tok.peek() != "$EOL" )  goto err;
		tok.get();
		printf("rulename: [%s]\n", rulename.c_str());
		return true;
		// error
		err:
		// tok.pos = pos;
		return error("expected 'rulename: $EOL'");
	}

	int psexpr() {
		// err:
		return error("error parsing bracketed s-expression");
	}
};


int main() {
	AST ast;
	ast.parse();
}