#pragma once
#include "tokenizer.hpp"
#include <map>
#include <algorithm>
#include <exception>
using namespace std;


/**
 * Language Ruleset
 * Contains a formatted set of rules only for our language.
 */
struct Ruleset : TokenHelpers {
	struct Rule     { string type; vector<string> list; };
	struct RuleExpr { string name; char expr; bool require; };
	static inline const vector<string> 
		RULE_TYPES       = { "and", "or" },
		RULE_EXPRESSIONS = { "*", "?", "+" },
		RULE_PREDEF      = { "$eof", "$eol", "$opplus", "$opmultiply", "$opdollar", "$identifier", "$stringliteral", "$integer" };
	string name;
	map<string, Rule>   rules;
	map<string, string> ruleerrors;


	// === build ruleset ===
	int add(const string& name, const string& list, const string& type = "and") {
		if ( rules.count(name) )
			return error( "add", "duplicate rule name: " + name );
		rules[ name ] = { type, splitstr(list) };
		return true;
	}

	int validate(bool report=false) {
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
				if      ( rex.name == "" )  error( "validate", name + ": empty rule" );
				else if ( rex.require && (rex.expr == '*' || rex.expr == '?') )  error( "validate", "bad rule-expression arguments: " + rexstr );
				else if ( ispredef( rex.name ) ) ;
				else if ( isuserdef( rex.name ) ) ;
				else if ( !isrulename( rex.name ) ) ;
				else if ( rex.name == "$dsym" && rex.expr == 0 && !rex.require ) ;
				else    error( "validate", name + ": unknown or invalid rule: " + rex.name );
			}
		}
		if (report)
			printf("%s ruleset validated!\n", name.c_str());
		return true;
	}

	int isrulename(const string& name) {
		return name.length() >= 2 && name[0] == '$';
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
		throw runtime_error( "[ruleset] " + rule + " error: " + msg );
		return false;
	}
};
