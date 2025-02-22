#include "tokenizer.hpp"
#include <map>
#include <exception>
#include <algorithm>
using namespace std;


struct Ruleset : TokenHelpers {
	struct Rule     { string type; vector<string>list; };
	struct RuleExpr { string name; char expr; bool require; };
	static inline const vector<string> 
		RULE_TYPES       = { "and", "or" },
		RULE_EXPRESSIONS = { "*" },
		RULE_PREDEF      = { "$eof", "$eol", "$identifier" };
	string name;
	map<string, Rule> rules;


	// === build ruleset ===
	int add(const string& name, const string& list, const string& type = "and") {
		if ( rules.count(name) )
			return error( "add", "duplicate rule name: " + name );
		rules[ name ] = { type, splitstr(list) };
		return true;
	}

	int validate() {
		for (const auto& [name, rule] : rules) {
			// check name
			if ( !isvalidname(name) )
				return error( "validate", name + ": bad rule name" );
			// check rule type
			if ( !isruletype(rule.type) )
				return error( "validate", name + ": unknown rule type: " + rule.type );
			// make sure the $program rule is defined
			if ( !rules.count("$program") )
				return error( "validate", "missing entry rule $program" );
			// check individual rules exist
			for (auto& rexstr : rule.list) {
				auto rex = splitruleexpr(rexstr);
				if      ( ispredef( rex.name ) ) ;
				else if ( isuserdef( rex.name ) ) ;
				else if ( !isrulename( rex.name ) ) ;
				else    return error( "validate", name + ": unknown or invalid rule: " + rex.name );
			}
		}
		printf("ruleset validated!\n");
		return true;
	}

	int isrulename(const string& name) {
		return name.length() && name[0] == '$';
	}
	int isvalidname(const string& name) {
		if (name.length() < 2)  return false;
		if (name[0] != '$')  return false;
		for (size_t i = 1; i < name.length(); i++)
			if ( !isalphanum(name[i]) )  return false;
		return true;
	}
	int isruletype(const string& type) {
		return find( RULE_TYPES.begin(), RULE_TYPES.end(), type ) != RULE_TYPES.end();
	}
	int isruleexpr(const string& ex) {
		return find( RULE_EXPRESSIONS.begin(), RULE_EXPRESSIONS.end(), ex ) != RULE_EXPRESSIONS.end();
	}
	int ispredef(const string& name) {
		return find( RULE_PREDEF.begin(), RULE_PREDEF.end(), name ) != RULE_PREDEF.end();
	}
	int isuserdef(const string& name) {
		return rules.count( name );
	}

	RuleExpr splitruleexpr(string rexstr) {
		RuleExpr rex = { "", 0, 0 };
		if (rexstr.length() && rexstr.back() == '!')
			rex.require = true, rexstr.pop_back();
		if (rexstr.length() && isruleexpr( string()+rexstr.back() ) )
			rex.expr = rexstr.back(), rexstr.pop_back();
		rex.name = rexstr;
		return rex;
	}


	//  === helpers ===
	void show() {
		printf(":: %s ::\n", name.c_str());
		for (const auto& [name, rule] : rules) {
			printf("%s: (%s)\n    ", name.c_str(), rule.type.c_str());
			for (auto& r : rule.list)
				printf("%s ", r.c_str());
			printf("\n");
		} 
	}

	int error(const string& rule, const string& msg) {
		throw runtime_error( rule + " error: " + msg );
		return false;
	}
};


struct Parser : TokenHelpers {
	Tokenizer tok;
	Ruleset ruleset;

	int parse(const string& fname) {
		// tokenize
		printf("-----\n");
		printf("loading file: %s\n", fname.c_str());
		if ( !tok.tokenize(fname) )
			error("parse", tok.errormsg);
		tok.show();
		// parse program
		if (!pruleexpr("$program"))
			return error("$program", "unknown error parsing $program");
		printf("file parsed successfully!");
		// ok
		return true;
	}

	int pruleexpr(const string& ruleexpr) {
		printf("parsing RuleExpr: '%s' @ Line %d '%s'\n", ruleexpr.c_str(), tok.linepos(), tok.peek().c_str());
		auto rex = ruleset.splitruleexpr(ruleexpr);
		// run the rule expression
		switch (rex.expr) {
			// match
			case 0:
				return prule(rex.name);
			// 0-to-many
			case '*':
				while (prule(rex.name)) ;
				return true;
			// 1-to-many
			// case '+':
			// 	if (!prule(rex.name))
			// 		return false;
			// 	while (prule(rex.name)) ;
			// 	return true;
		}
		// unknown error
		return error( "pruleexpr", "unexpected error" );
	}

	int prule(const string& name) {
		// built in rules
		if (name == "$eof") {
			return tok.eof();
		}
		else if (name == "$eol") {
			if (tok.peek() == "$EOL")
				return tok.get(), true;
			return false;
		}
		else if (name == "$identifier") {
			if (isidentifier(tok.peek()))  
				return tok.get(), true;
			return false;
		}

		// user defined rules
		else if (ruleset.isuserdef(name)) {
			const auto& rule = ruleset.rules[name];
			int pos = tok.pos;
			// and
			if (rule.type == "and") {
				for (auto& subrule : rule.list)
					if (!pruleexpr(subrule))
						return tok.pos = pos, false;
				return true;
			}
			// or
			else if (rule.type == "or") {
				for (auto& subrule : rule.list)
					if (pruleexpr(subrule))
						return true;
					else
						tok.pos = pos;
				return false;
			}
		}

		// string match
		else if (!ruleset.isrulename(name)) {
			if (tok.peek() == name)
				return tok.get(), true;
			return false;
		}

		// unknown rule error
		return error("prule", "unexpected error");
	}

	int error(const string& rule, const string& msg) {
		throw runtime_error(rule + " error: " + msg);
		return false;
	}
};
