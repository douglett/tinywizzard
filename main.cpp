#include "tokenizer.hpp"
#include <exception>
#include <map>
using namespace std;


struct Sexpr {
	enum SEXPR_TYPE {
		T_NIL,
		T_LIST,
		T_TOKEN,
		T_LITERAL
	};
	SEXPR_TYPE type;
	string val;
	vector<Sexpr> list;
};


struct AST : public TokenHelpers {
	Tokenizer tok;
	map<string, Sexpr> rules;

	int parse() {
		tok.flag_eol = 1;
		tok.tokenize("simplebasic.ast");
		tok.show();
		// parse lines
		while (!tok.eof())
			if      ( tok.peek() == "$EOL" )  tok.get();
			else if ( !prule() )  return false;
		show();
		return true;
	}

	int prule() {
		string rulename;
		prulename(rulename);
		rules[rulename] = { Sexpr::T_LIST };  // define rule
		psexpr( rules[rulename] );
		if ( tok.peek() != "$EOL" )  return error("rule-definition", "expected $EOL after rule definition s-expression");
		return true;
	}

	int prulename(string& rulename) {
		if ( !isidentifier(tok.peek()) )  goto err;
		rulename = tok.get();
		if ( tok.peek() != ":" )  goto err;
		tok.get();
		if ( tok.peek() != "$EOL" )  goto err;
		tok.get();
		return true;
		// error
		err:
		return error("rule-name", "expected 'rulename: $EOL'");
	}

	int psexpr(Sexpr& sexpr) {
		string t;
		if ( tok.peek() != "(" )  goto err;
		tok.get();
		while (true)
			if      ( tok.eof() )  goto err;
			else if ( tok.peek() == "$EOL" )  tok.get();
			else if ( tok.peek() == "(" )     sexpr.list.push_back({ Sexpr::T_LIST }),  psexpr( sexpr.list.back() );
			else if ( tok.peek() == ")" )     { tok.get();  break; }
			else if ( tok.peek() == "$" )     pspecialtoken(t),  sexpr.list.push_back({ Sexpr::T_TOKEN, t });
			else                              sexpr.list.push_back({ Sexpr::T_TOKEN, tok.get() });
		// validate
		if ( sexpr.list.size() == 0 || sexpr.list[0].type != Sexpr::T_TOKEN )
			return error("s-expression", "expected rule type in s-expression");
		return true;
		// error
		err:
		return error("s-expression", "error parsing bracketed s-expression");
	}

	int pspecialtoken(string& t) {
		if ( tok.peek() != "$" )  goto err;
		t = tok.get();
		if ( tok.eof() || tok.peek() == "$EOL" )  goto err;
		t += tok.get();
		return true;
		// error
		err:
		return error("$-token", "error parsing $-token");
	}

	int error(const string& rule, const string& msg) {
		throw runtime_error(
			rule + " error: " 
			+ msg
			+ " (line " + to_string(tok.linepos()) + ")" );
		return false;
	}

	void show() {
		for (const auto& rule : rules) {
			cout << rule.first << ":";
			showsexpr( rule.second, 1 );
		}
	}

	void showsexpr(const Sexpr& sexpr, int indent=0) {
		switch (sexpr.type) {
			case Sexpr::T_NIL:      cout << "NIL ";  break;
			case Sexpr::T_TOKEN:    cout << sexpr.val << " ";  break;
			case Sexpr::T_LITERAL:  cout << sexpr.val << " ";  break;
			case Sexpr::T_LIST:
				cout << endl << string(indent*3, ' ') << "( ";
				for (size_t i = 0; i < sexpr.list.size(); i++) {
					showsexpr( sexpr.list[i], indent+1 );
					if ( sexpr.list[i].type == Sexpr::T_LIST && i < sexpr.list.size()-1 && sexpr.list[i+1].type != Sexpr::T_LIST )
						cout << endl << string((indent+1)*3, ' ');
				}
				cout << ") ";
				break;
		}
	}
};


int main() {
	AST ast;
	ast.parse();
}