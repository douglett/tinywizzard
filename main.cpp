#include "tokenizer.hpp"
#include <map>
#include <exception>
#include <algorithm>
// #include <cassert>
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
				else    return error( "validate", name + ": unknown or invalid rule: " + rex.name );
			}
		}
		printf("ruleset validated!\n");
		return true;
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
		if ( !pruleexpr("$program") )
			return error( "$program", "unknown error parsing $program" );
		printf("file parsed successfully!");
		// ok
		return true;
	}

	int pruleexpr(const string& ruleexpr) {
		printf("parsing RuleExpr: '%s' @ '%s'\n", ruleexpr.c_str(), tok.peek().c_str());
		auto rex = ruleset.splitruleexpr(ruleexpr);

		return false;
	}

	// int prule(const string& rulename) {
	// 	printf("parsing rule: %s %s\n", rulename.c_str(), tok.peek().c_str());
	// 	// built in rules
	// 	if ( rulename == "$eof" ) {
	// 		if (!tok.eof())  return false;
	// 	}
	// 	else if ( rulename == "$eol" ) {
	// 		if (tok.peek() != "$EOL")  return false;
	// 		tok.get();
	// 	}
	// 	else if ( rulename == "$identifier" ) {
	// 		if (!isidentifier( tok.peek() ))  return false;
	// 		tok.get();
	// 	}
	// 	// user defined rules
	// 	else if ( rulename.size() && rulename[0] == '$' ) {
	// 		if ( !ruleset.rules.count(rulename) )
	// 			return error( "prule", "missing rule: " + rulename );
	// 		for (const auto& subrule : ruleset.rules[rulename].list)
	// 			if (!prule( subrule ))  return false;
	// 	}
	// 	// match text
	// 	else {
	// 		if ( tok.peek() != rulename )  return false;
	// 		tok.get();
	// 	}
	// 	return true;
	// }

	int error(const string& rule, const string& msg) {
		throw runtime_error( rule + " error: " + msg );
		return false;
	}
};


struct TestlangParser : Parser {
	int init() {
		// tokenizer settings
		tok.lcomment = "REM";
		tok.flag_eol = true;

		// initialise ruleset
		ruleset.name = "testlang";
		ruleset.add( "$program", "$identifier* $eof" );
		// ruleset.add( "$line", "$identifier* $eol" );
		ruleset.show();
		ruleset.validate();
		return true;
	}
};


int main() {
	printf("hello world\n");

	TestlangParser parser;
	parser.init();
	parser.parse("test/test1.script");
}