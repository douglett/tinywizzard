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
		if (json.count("expression")) {
			auto extype = pexpression(json.at("expression"));
			if (type != extype)
				errorc("pdim", "initializing '" + type + "' with '" + extype);
		}
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
			const auto& type = dims.at(name);
			auto extype = pexpression(json.at("expression"));
			if (type != extype)
				errorc("pstatement", "assign to variable '" + type + "' with '" + extype + "'");
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

	string pexpression(const Json& json) {
		auto& type = json.at("expr").str;
		if      (type == "integer")  return "int";
		else if (type == "strlit")   return "string";
		else if (type == "add" || type == "mul") {
			auto ltype = pexpression(json.at("lhs"));
			auto rtype = pexpression(json.at("rhs"));
			if (ltype == "int" && rtype == "int")
				return "int";
			errorc("pexpression", "trying to add/multiply between '" + ltype + "' and '" + rtype + "'");
			return "void";
		}
		else if (type == "variable") {
			auto& name = json.at("value").str;
			if (!dims.count(name))
				errorc("pexpression", "undefined variable '" + name + "'");
			return "int";
		}
		else {
			errorc("pexpression", "unknown in expression '" + type + "'");
			return "void";
		}
	}
};