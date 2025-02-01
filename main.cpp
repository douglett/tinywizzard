#include "tokenizer.hpp"
#include <map>
#include <exception>
#include <algorithm>
// #include <cassert>
using namespace std;


struct Ruleset : TokenHelpers {
	struct Rule { string type; vector<string>list; };
	const vector<string> 
		RULE_TYPES      = { "accept", "require" },
		RULE_PREDEF     = { "$eol" };
	map<string, Rule> rules;

	// int add(const string& name, const string& list) {
	// 	return add( name, "accept", list );
	// }
	int add(const string& name, const string& list, const string& type = "accept") {
		if ( rules.count(name) )
			return error( "add", "duplicate rule name: " + name );
		rules[ name ] = { type, splitstr(list) };
		return true;
	}

	void show() {
		for (const auto& [name, rule] : rules) {
			printf("%s: (%s)\n    ", name.c_str(), rule.type.c_str());
			for (auto& r : rule.list)
				printf("%s ", r.c_str());
			printf("\n");

		} 
	}

	int validate() {
		for (const auto& [name, rule] : rules) {
			// check name
			if ( !validatename(name) )
				return error( "validate", name + ": bad rule name" );
			// check rule type
			if ( find( RULE_TYPES.begin(), RULE_TYPES.end(), rule.type ) == RULE_TYPES.end() )
				return error( "validate", name + ": unknown rule type: " + rule.type );
			// check individual rules exist
			for (auto& subrule : rule.list)
				if ( find( RULE_PREDEF.begin(), RULE_PREDEF.end(), subrule ) != RULE_PREDEF.end() )
					continue;
				else if ( rules.count( subrule ) )
					continue;
				else
					return error( "validate", name + ": unknown subrule: " + subrule );
		}
		printf("ruleset validated!\n");
		return true;
	}

	int validatename(const string& name) {
		if (name.length() < 2)  return false;
		if (name[0] != '$')  return false;
		for (size_t i = 1; i < name.length(); i++)
			if ( !isalphanum(name[i]) )  return false;
		return true;
	}

	int error(const string& rule, const string& msg) {
		throw runtime_error(
			rule + " error: " 
			+ msg );
		return false;
	}
};


Ruleset testlang;


int main() {
	printf("hello world\n");

	testlang.add( "$program", "$eol" );
	testlang.show();
	testlang.validate();
}