#pragma once
#include "semantics.hpp"
using namespace std;


struct TinyWizzardSemantics : Semantics {
	map<string, string> dims, locals, functions;
	int loopblocklevel = 0;

	int validate(const Json& json) {
		loglevel = 2;
		// loglevel = 4;  // trace
		log(1, "validating file...");
		reset();
		// validate class members
		for (auto& var : json.at("dims").arr)
			pdim(var);
		pallfunctions(json.at("functions"));
		// success or fail
		if (errcount)
			return error("validate", "failed with " + to_string(errcount) + " errors.");
		log(1, "validate successful!");
		return true;
	}

	void reset() {
		Semantics::reset();
		dims = locals = {};
		functions = { { "$STATIC_INIT", "int" } };
		loopblocklevel = 0;
	}

	void pallfunctions(const Json& funclist) {
		log(4, "(trace) pallfunctions");
		assert(funclist.type == Json::JARRAY);
		// hoist function definitions
		for (auto& func : funclist.arr) {
			dsym = func.at("dsym").num;
			auto& name = func.at("name").str;
			if (functions.count(name))
				errorc("pfunction", "re-definition of '" + name + "'");
			functions[name] = true;
		}
		// check function blocks
		for (auto& func : funclist.arr) {
			for (auto& stmt : func.at("block").arr)
				pstatement(stmt);
			locals = {};
		}
		// check for main function
		if (!functions.count("main"))
			errorc("validate", "missing function 'main'");
	}

	void pdim(const Json& json) {
		log(4, "(trace) pdim");
		dsym = json.at("dsym").num;
		auto& name = json.at("name").str;
		auto& type = json.at("type").str;
		if (dims.count(name))
			return errorc("pdim", "re-definition of '" + name + "'"), void();
		if (type != "int" && type != "string")
			errorc("pdim", "unknown type '" + type + "'");
		if (json.count("expression")) {
			auto extype = pexpression(json.at("expression"));
			if (type != extype)
				errorc("pdim", "initializing '" + type + "' with '" + extype + "'");
		}
		dims[name] = type;
		((Json&)json).setb("local") = false;
	}

	void pdimlocal(const Json& json) {
		log(4, "(trace) pdimlocal");
		dsym = json.at("dsym").num;
		auto& name = json.at("name").str;
		auto& type = json.at("type").str;
		if (locals.count(name))
			return errorc("pdim", "re-definition of '" + name + "'"), void();
		if (type != "int" && type != "string")
			errorc("pdim", "unknown type '" + type + "'");
		if (type != "int")
			errorc("pdim", "locals must be int, got '" + type + "'");
		if (json.count("expression")) {
			auto extype = pexpression(json.at("expression"));
			if (type != extype)
				errorc("pdim", "initializing '" + type + "' with '" + extype + "'");
		}
		locals[name] = type;
		((Json&)json).setb("local") = true;
	}

	void pstatement(const Json& json) {
		log(4, "(trace) pstatement");
		dsym = json.at("dsym").num;
		auto& stmt = json.at("statement").str;
		// assign
		if (stmt == "assign") {
			auto&  name  = json.at("name").str;
			string type  = "void";
			bool   local = false;
			// get type information
			if      (dims.count(name))    type = dims.at(name);
			else if (locals.count(name))  type = locals.at(name), local = true;
			else    errorc("pstatement", "assign to undefined variable '" + name + "'");
			// add type information to json
			log(2, "assign   '" + name + "' => '" + type + "'");
			((Json&)json).sets("type")  = type;
			((Json&)json).setb("local") = local;
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
			if (loopblocklevel < 1)
				errorc("pstatement-break", "break outside of loop-block");
			else if (json.count("level")) {
				int level = json.at("level").num;
				if (level < 1)
					errorc("pstatement-break", "illegal break level '" + to_string(level) + "'");
				else if (level > loopblocklevel)
					errorc("pstatement-break", "break level too high '" + to_string(level) + "'");
			}
		}
		// dim
		else if (stmt == "dim") {
			pdimlocal(json);
		}
		// return
		else if (stmt == "return") {
			if (json.count("expression"))
				pexpression(json.at("expression"));
		}
		// expression statement
		else if (stmt == "expression") {
			pexpression(json.at("expression"));
		}
		// unknown
		else
			errorc("pstatement", "unknown statement '" + stmt + "'");
	}

	string pexpression(const Json& json) {
		log(4, "(trace) pexpression");
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
		else if (expr == "call") {
			auto fname = json.at("value").str;
			if (!functions.count(fname))
				errorc("pexpression", "calling undefined function '" + fname + "'");
			return "int";
		}
		else
			return errorc("pexpression", "unknown in expression '" + expr + "'"), "void";
	}

	string pexprvar(const Json& json) {
		log(4, "(trace) pexprvar");
		auto&  name  = json.at("value").str;
		string type  = "void";
		bool   local = false;
		// check if defined
		if      (dims.count(name))    type = dims.at(name);
		else if (locals.count(name))  type = locals.at(name), local = true;
		else    return errorc("pexpression", "undefined variable '" + name + "'"), "void";
		// add type information to json
		log(2, "variable '" + name + "' => '" + type + "'");
		((Json&)json).sets("type")  = type;
		((Json&)json).setb("local") = local;
		return type;
	}
};