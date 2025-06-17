#pragma once
#include "semantics.hpp"
using namespace std;


struct TinyWizzardSemantics : Semantics {
	map<string, string> dims, functions;

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
		functions = { { "STATIC_INIT", "int" } };
	}

	void pdim(const Json& json) {
		dsym = json.at("dsym").num;
		auto& name = json.at("name").str;
		auto& type = json.at("type").str;
		if (dims.count(name))
			errorc("pdim", "re-definition of '" + name + "'");
		if (type != "int")
			errorc("pdim", "unknown type '" + type + "'");
		if (json.count("expression"))
			pexpression(json.at("expression"));
		dims[name] = type;
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
				if    (pval.at("expr").str == "strlit") ;
				else  pexpression(pval);
		}
		else
			errorc("pstatement", "unknown statement '" + type + "'");
	}

	void pexpression(const Json& json) {
		auto& type = json.at("expr").str;
		if      (type == "integer") ;
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
		else
			errorc("pexpression", "unknown in expression '" + type + "'");
	}
};