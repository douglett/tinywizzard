#pragma once
#include "semantics.hpp"
using namespace std;


struct TinyWizzardSemantics : Semantics {
	map<string, string> dims, functions;
	int loopblocklevel = 0;

	int validate(const Json& json) {
		loglevel = 2;
		log(1, "validating file...");
		reset();
		// validate class members
		for (auto& var : json.at("dims").arr)
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
		loopblocklevel = 0;
	}

	void pdim(const Json& json) {
		dsym = json.at("dsym").num;
		auto& name = json.at("name").str;
		auto& type = json.at("type").str;
		if (dims.count(name))
			errorc("pdim", "re-definition of '" + name + "'");
		if (type != "int" && type != "string")
			errorc("pdim", "unknown type '" + type + "'");
		if (json.count("expression")) {
			auto extype = pexpression(json.at("expression"));
			if (type != extype)
				errorc("pdim", "initializing '" + type + "' with '" + extype + "'");
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
		auto& stmt = json.at("statement").str;
		// assign
		if (stmt == "assign") {
			auto& name = json.at("name").str;
			if (!dims.count(name))
				errorc("pstatement", "assign to undefined variable '" + name + "'");
			// check type information and add to json
			auto& type = dims.at(name);
			log(2, "assign   '" + name + "' => '" + type + "'");
			((Json&)json).obj["type"] = { Json::JSTRING, 0, type };
			// check types match
			auto extype = pexpression(json.at("expression"));
			if (type != extype)
				errorc("pstatement", "assign to variable '" + type + "' with '" + extype + "'");
		}
		// print
		else if (stmt == "print") {
			for (auto& pval : json.at("printvals").arr)
				pexpression(pval);
		}
		// input
		else if (stmt == "input") {
			pexprvar(json.at("variable"));
		}
		// if-else
		else if (stmt == "if") {
			for (auto& cond : json.at("conditionals").arr) {
				if (cond.at("conditional").str != "else")
					pexpression(cond.at("expression"));
				for (auto& stmt : cond.at("block").arr)
					pstatement(stmt);
			}
		}
		// while
		else if (stmt == "while") {
			pexpression(json.at("expression"));
			loopblocklevel++;
			for (auto& stmt : json.at("block").arr)
				pstatement(stmt);
			loopblocklevel--;
		}
		// break
		else if (stmt == "break") {
			int level = json.at("level").num;
			if (loopblocklevel < 1)
				errorc("pstatement-break", "break outside of loop-block");
			else if (level < 1)
				errorc("pstatement-break", "illegal break level '" + to_string(level) + "'");
			else if (level > loopblocklevel)
				errorc("pstatement-break", "break level too high '" + to_string(level) + "'");
		}
		// return
		else if (stmt == "return") {
			if (json.count("expression"))
				pexpression(json.at("expression"));
		}
		// unknown
		else
			errorc("pstatement", "unknown statement '" + stmt + "'");
	}

	string pexpression(const Json& json) {
		auto& expr = json.at("expr").str;
		if      (expr == "integer")   return "int";
		else if (expr == "strlit")    return "string";
		else if (expr == "variable")  return pexprvar(json);
		else if (expr == "equals") {
			auto ltype = pexpression(json.at("lhs"));
			auto rtype = pexpression(json.at("rhs"));
			if (ltype == "int" && rtype == "int")
				return "int";
			else if (ltype == "string" && rtype == "string") {
				auto& op = json.at("operator").str;
				if (op != "==" && op != "!=")
					errorc("pexpression", "invalid string comparison '" + op + "'");
				((Json&)json).sets("expr") = "equals-string";
				return "string";
			}
			errorc("pexpression", "trying to check equality between '" + ltype + "' and '" + rtype + "'");
			return "void";
		}
		else if (expr == "add" || expr == "mul") {
			auto ltype = pexpression(json.at("lhs"));
			auto rtype = pexpression(json.at("rhs"));
			if (ltype == "int" && rtype == "int")
				return "int";
			errorc("pexpression", "trying to add/multiply between '" + ltype + "' and '" + rtype + "'");
			return "void";
		}
		else
			return errorc("pexpression", "unknown in expression '" + expr + "'"), "void";
	}

	string pexprvar(const Json& json) {
		auto& name = json.at("value").str;
		if (!dims.count(name))
			errorc("pexpression", "undefined variable '" + name + "'");
		// add type information to json
		auto& type = dims.at(name);
		log(2, "variable '" + name + "' => '" + type + "'");
		((Json&)json).obj["type"] = { Json::JSTRING, 0, type };
		return type;
	}
};