#pragma once
#include "semantics.hpp"
using namespace std;


struct TinyWizzardSemantics : Semantics {
	map<string, bool> dims, functions;

	int validate(const Json& json) {
		log(1, "validating file...");
		reset();
		// validate class members
		for (auto& var : json.at("variables").arr)
			pdim(var);
		for (auto& func : json.at("functions").arr)
			pfunction(func);
		// success or fail
		if (errcount)
			return error("validate", "failed with " + to_string(errcount) + " errors.");
		log(1, "validate successful!");
		return true;
	}

	void reset() {
		Semantics::reset();
		dims = {};
		functions = { { "STATIC_INIT", true } };
	}

	void pdim(const Json& json) {
		dsym = json.at("dsym").num;
		auto& name = json.at("name").str;
		if (dims.count(name))
			errorc("pdim", "re-definition of '" + name + "'");
		if (json.count("expression"))
			pexpression(json.at("expression"));
		dims[name] = true;
	}

	void pfunction(const Json& json) {
		dsym = json.at("dsym").num;
		auto& name = json.at("name").str;
		if (functions.count(name))
			errorc("pfunction", "re-definition of '" + name + "'");
		functions[name] = true;
		for (auto& stmt : json.at("block").arr)
			pstatement(stmt);
	}

	void pstatement(const Json& json) {
		dsym = json.at("dsym").num;
		auto& type = json.at("statement").str;
		// assign
		if (type == "assign") {
			auto& name = json.at("variable").str;
			if (!dims.count(name))
				errorc("pstatement", "assign to undefined variable '" + name + "'");
			pexpression(json.at("expression"));
		}
		// print
		else if (type == "print") {
			for (auto& pval : json.at("printvals").arr)
				pexpression(pval);
		}
		// warning
		else {
			log(1, "warning: unchecked statement '" + type + "'");
		}
	}

	void pexpression(const Json& json) {
		auto& type = json.at("expr").str;
		if      (type == "integer") ;
		else if (type == "strlit") ;
		else if (type == "add" || type == "mul") {
			if (json.at("lhs").at("expr").str == "strlit" || json.at("rhs").at("expr").str == "strlit")
				errorc("pexpression", "trying to add or multiply a string");
			pexpression(json.at("lhs"));
			pexpression(json.at("rhs"));
		}
		else if (type == "variable") {
			auto& name = json.at("value").str;
			if (!dims.count(name))
				errorc("pexpression", "undefined variable '" + name + "'");
		}
		else {
			log(1, "warning: unchecked expression '" + type + "'");
		}
	}
};