#pragma once
#include "ruleset.hpp"
#include "tokenizer.hpp"
#include "json.hpp"
#include <exception>
#include <cassert>
using namespace std;


/**
 * Language Parser
 * Uses a defined Rulset to parse a file to json output AST.
 */
struct RuleParser : TokenHelpers {
	Tokenizer tok;
	Ruleset ruleset;
	Json ast;
	bool trace = false;
	// json formatting rules
	vector<string> FMT_CULL, FMT_FIRST_CHILD, FMT_FIRST_VALUE;

	int parse(const string& fname) {
		// tokenize
		printf("-----\n");
		printf("loading file: %s\n", fname.c_str());
		if ( !tok.tokenize(fname) )
			error("parse", tok.errormsg);
		tok.show();
		// parse program
		ast = { Json::JARRAY };
		printf("parsing file...\n");
		if (!pruleexpr("$program", ast))
			return error("$program", "unknown error parsing $program");
		show();
		printf("file parsed successfully!\n");
		// ok
		return true;
	}

	int pruleexpr(const string& ruleexpr, Json& parent) {
		assert(parent.type == Json::JARRAY);
		// printf("parsing RuleExpr: '%s' @ Line %d '%s'\n", ruleexpr.c_str(), tok.linepos(), tok.peek().c_str());
		bool found = false;
		auto rex = ruleset.splitruleexpr(ruleexpr);
		// run the rule expression
		switch (rex.expr) {
			// match
			case 0:
				found = pruleformat(rex.name, parent);
				break;
			// 0-to-many
			case '*':
				while (pruleformat(rex.name, parent)) ;
				found = true;
				break;
			// 0-to-1
			case '?':
				pruleformat(rex.name, parent);
				found = true;
				break;
			// 1-to-many
			case '+':
				while (pruleformat(rex.name, parent))
					found = true;
				break;
			// unknown error
			default:
				return error( "pruleexpr", "unexpected error" );
		}
		// found action
		if (found && trace)
			printf("found RuleExpr: '%s' @ Line %d\n", ruleexpr.c_str(), tok.linepos());
		return found;
	}

	int pruleformat(const string& name, Json& parent) {
		assert(parent.type == Json::JARRAY);
		if (!prule(name, parent))
			return false;
		
		// apply some basic formatting
		assert(parent.arr.size() > 0);
		auto& json = parent.arr.back();
		// take values from the json object
		assert(json.type == Json::JOBJECT);
		string& type = json.obj["type"].str;
		auto& value = json.obj["value"].arr;

		// cull these rules totally
		if ( find(FMT_CULL.begin(), FMT_CULL.end(), type) != FMT_CULL.end() )
			json = { Json::JNULL };
		// replace whole json object with the first child
		else if ( find(FMT_FIRST_CHILD.begin(), FMT_FIRST_CHILD.end(), type) != FMT_FIRST_CHILD.end() ) {
			if (value.size() == 1) {
				auto var = value.at(0);
				json = var;
			}
		}
		// take first child value as the value
		else if ( find(FMT_FIRST_VALUE.begin(), FMT_FIRST_VALUE.end(), type) != FMT_FIRST_VALUE.end() ) {
			if (value.size() == 1) {
				auto var = value.at(0).obj.at("value");
				json.obj["value"] = var;
			}
		}
		// otherwise, apply user override custom formatting
		else
			formatjson(json);
		
		// cull anything formatted to null
		if (json.type == Json::JNULL)
			parent.arr.pop_back();
		return true;
	}

	virtual void formatjson(Json& json) {}

	int prule(const string& name, Json& parent) {
		assert(parent.type == Json::JARRAY);
		// built in rules
		if (name == "$eof")
			return tok.eof() ? paccepttok(parent, name) : false;
		else if (name == "$eol")
			return tok.peek() == "$EOL" ? paccepttok(parent, name) : false;
		else if (name == "$opplus")
			return tok.peek() == "+" ? paccepttok(parent, name) : false;
		else if (name == "$opmultiply")
			return tok.peek() == "*" ? paccepttok(parent, name) : false;
		else if (name == "$opdollar")
			return tok.peek() == "$" ? paccepttok(parent, name) : false;
		else if (name == "$identifier")
			return isidentifier(tok.peek()) ? paccepttok(parent, name) : false;
		else if (name == "$stringliteral")
			return isliteral(tok.peek()) ? paccepttok(parent, name) : false;
		else if (name == "$integer")
			return isnumber(tok.peek()) ? paccepttok(parent, name) : false;

		// string match
		else if (!ruleset.isrulename(name))
			return tok.peek() == name ? paccepttok(parent, name) : false;

		// user defined rules
		else if (ruleset.isuserdef(name)) {
			const auto& rule = ruleset.rules[name];
			int pos = tok.pos;
			parent.arr.push_back({ Json::JOBJECT });
			auto& obj = parent.arr.back();
			obj.obj["type"] = { Json::JSTRING, 0, name };
			obj.obj["value"] = { Json::JARRAY };
			auto& js = obj.obj["value"]; 
			// and
			if (rule.type == "and") {
				for (auto& subrule : rule.list)
					if (subrule == "$dsym")
						obj.obj["dsym"] = { Json::JNUMBER, double(tok.linepos()) };
					else if (!pruleexpr(subrule, js))
						return tok.pos = pos, parent.arr.pop_back(), false;
				return true;
			}
			// or
			else if (rule.type == "or") {
				for (auto& subrule : rule.list)
					if (subrule == "$dsym")
						obj.obj["dsym"] = { Json::JNUMBER, double(tok.linepos()) };
					else if (pruleexpr(subrule, js))
						return true;
					else
						tok.pos = pos;
				return parent.arr.pop_back(), false;
			}
		}

		// unknown rule error
		return error("prule", "unexpected error");
	}

	int paccepttok(Json& parent, const string& type) {
		assert(parent.type == Json::JARRAY);
		int lpos = tok.linepos();
		auto token = tok.get();
		parent.arr.push_back({ Json::JOBJECT });
		auto& obj = parent.arr.back();
		obj.obj["type"] = { Json::JSTRING, 0, type };
		obj.obj["value"] = { Json::JSTRING, 0, token };
		if (trace)
			printf("  accept-tok: %s  (line %d)\n", token.c_str(), lpos);
		return true;
	}

	//  === helpers ===
	void show() {
		printf("outputting program AST to output.json...\n");
		fstream fs("output.json", ios::out);
		if (ast.arr.size())
			fs << ast.at(0);
		else
			fs << "(json empty)\n";
	}

	int error(const string& rule, const string& msg) {
		throw runtime_error(rule + " error: " + msg);
		return false;
	}
};