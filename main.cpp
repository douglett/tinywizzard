#include "tokenizer.hpp"
#include <exception>
#include <map>
#include <cassert>
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


struct GrammarParser : TokenHelpers {
	Tokenizer tok;
	map<string, Sexpr> rules;

	int parse(const string& fname) {
		tok.flag_eol = 1;
		if ( !tok.tokenize(fname) )
			error("parse", tok.errormsg);
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
		if ( tok.peek() != "$" )  goto err;
		rulename = tok.get();
		if ( !isidentifier(tok.peek()) )  goto err;
		rulename += tok.get();
		if ( tok.peek() != ":" )  goto err;
		tok.get();
		if ( tok.peek() != "$EOL" )  goto err;
		tok.get();
		return true;
		// error
		err:
		return error("rule-name", "expected '$rulename: $EOL'");
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
		cout << endl;
	}

	static void showsexpr(const Sexpr& sexpr, int indent=0) {
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


struct LanguageParser : TokenHelpers {
	Tokenizer tok;
	map<string, Sexpr> rules;

	int parse(const string& fname) {
		tok.lcomment = "REM";
		tok.flag_eol = true;
		if ( !tok.tokenize(fname) )
			error("parse", tok.errormsg);
		tok.show();
		// parse program
		if (!prulename("$program"))
			return error("$program", "error parsing $program");
		// make sure we are at the end of file
		// while (tok.peek() == "$eol")
		// 	tok.get();
		// if (!tok.eof())
		// 	return error("program", "error: parsing $program did not reach end-of-file");
		return true;
	}

	int error(const string& rule, const string& msg) {
		throw runtime_error(
			rule + " error: " 
			+ msg
			+ " (line " + to_string(tok.linepos()) + ")" );
		return false;
	}

	int prulename(const string& rulename) {
		assert( rulename.length() > 0 );  // sanity check
		// sanity check
		// if (rulename.length() == 0)
		// 	return error("prulename", "expected rule name, got nothing");
		// built in rules

		// user defined rule
		if (rulename[0] == '$') {
			if (!rules.count(rulename))
				return error("prulename", "missing rule: " + rulename);
			return prule( rules.at(rulename) );
		}
		// string matching
		else {
			if (tok.peek() == rulename)
				return tok.get(), true;
			return false;
		}
	}

	int prule(const Sexpr& rule) {
		// GrammarParser::showsexpr( rule );  cout << endl;
		assert( rule.type == Sexpr::T_LIST );  // sanity checks
		assert( rule.list.size() > 0 && rule.list[0].type == Sexpr::T_TOKEN );
		string ruletype = rule.list[0].val;
		if (ruletype == "accept") {
			for (size_t i = 1; i < rule.list.size(); i++) {
				auto& subrule = rule.list[i];
				if      ( i == 0 )  continue;
				else if ( subrule.type == Sexpr::T_TOKEN )  return prulename( subrule.val );
				else if ( subrule.type == Sexpr::T_LIST )  return prule( subrule );
				else    return error("prule", "unexpected Sexpr type: " + to_string(subrule.type));
			}
		}
		else
			return error("prule", "unknown rule type: " + ruletype);
	}
};


int main() {
	GrammarParser grammar;
	grammar.parse("test/test1.ast");
	LanguageParser lang;
	lang.rules = grammar.rules;
	lang.parse("test/test1.script");
}