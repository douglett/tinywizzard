#pragma once
#include "generator.hpp"
#include <algorithm>
using namespace std;


struct TinyWizzardGenerator : Generator {
	struct Func { string name; vector<Instruction> ilist; };
	map<string, Func> functions;
	vector<string> literals;
	string funcname;

	int generate(const Json& json) {
		loglevel = 4;  // 4 = trace
		log(1, "generating single-class program...");
		reset();
		// compile
		pclass(json);
		// success or fail
		if (errcount)
			error("Generator", "compile failed with " + to_string(errcount) + " errors.");
		outputliterals();
		outputfunctions();
		show();
		log(1, "compiled successfully!");
		return true;
	}

	void reset() {
		Generator::reset();
		functions = {};
		functions["STATIC_INIT"] = { "STATIC_INIT" };
		funcname = "";
	}

	// === generate helpers ===

	vector<Instruction>& getilist() {
		return funcname.length() ? functions.at(funcname).ilist : functions.at("STATIC_INIT").ilist;
	}
	void output(INSTRUCTION_TYPE type, int argi) {
		getilist().push_back({ type, {}, argi });
	}
	void output(INSTRUCTION_TYPE type, const vector<string>& args={}, int argi=0) {
		getilist().push_back({ type, args, argi });
	}

	void outputliterals() {
		auto& fn = functions.at("STATIC_INIT").ilist;
		fn.insert(fn.begin(), { IN_NOOP, { "string-literals" } });
		for (size_t i = 0; i < literals.size(); i++)
			fn.insert(fn.begin()+i+1, { IN_DATA, { "STRING_LIT_"+to_string(i), literals.at(i) } });
	}

	void outputfunctions() {
		log(1, "ouputting functions to main program...");
		// calculate order
		vector<string> order = { "STATIC_INIT", "main" };
		for (const auto& fn : functions)
			if (find(order.begin(), order.end(), fn.first) == order.end())
				order.push_back(fn.first);
		// show functions
		for (const auto& fname : order) {
			log(4, "(trace) function-output: " + fname);
			if (!functions.count(fname))  continue;
			const auto& fn = functions.at(fname);
			program.push_back({ IN_LABEL, {fn.name} });
			program.insert(program.end(), fn.ilist.begin(), fn.ilist.end());
			program.push_back({ IN_RETURN });
		}
	}

	// === class definition ===

	void pclass(const Json& json) {
		auto& classname = json.at("classname").str;
		log(1, "compiling class: " + classname);
		for (auto& dim : json.at("variables").arr)
			pdim(dim);
		for (auto& func : json.at("functions").arr)
			pfunction(func);
	}

	void pdim(const Json& json) {
		log(4, "(trace) pdim");
		auto& ilist = functions.at("STATIC_INIT").ilist;
		auto& name  = json.at("name").str;
		dsym        = json.at("dsym").num;
		// generate dim
		output( IN_DSYM, dsym );
		output( IN_DIM, { name } );
		if (json.count("expression")) {
			pexpression(json.at("expression"));
			ilist.push_back({ IN_PUT, { name } });
		}
	}

	void pfunction(const Json& json) {
		log(4, "(trace) pfunction");
		funcname            = json.at("name").str;
		functions[funcname] = { funcname };
		for (auto& stmt : json.at("block").arr)
			pstatement(stmt);
		funcname = "";
	}

	// === statements ===

	void pstatement(const Json& json) {
		log(4, "(trace) pstatement");
		auto& type  = json.at("statement").str;
		dsym        = json.at("dsym").num;
		// generate statement
		output( IN_DSYM, dsym );
		if (type == "assign") {
			pexpression(json.at("expression"));
			output( IN_PUT, { json.at("variable").str } );
		}
		else if (type == "print") {
			for (auto& printval : json.at("printvals").arr) {
				auto& type = printval.at("expr").str;
				if (type == "number")
					output( IN_PRINTI, printval.at("value").num );
				else if (type == "variable")
					output( IN_PRINTV, { printval.at("value").str } );
				else if (type == "strlit") {
					literals.push_back(printval.at("value").str);
					output( IN_PRINTS, { "STRING_LIT_"+to_string(literals.size()-1) } );
				}
				else
					errorc("pstatement-print", "unknown printval type: " + type);
				output( IN_PRINTC, ' ' );  // space-seperate values
			}
			output( IN_PRINTC, '\n' );  // enf-of-line
		}
		else
			errorc("pstatement", "unknown statement '" + type + "'");
	}

	// === expressions ===

	void pexpression(const Json& json) {
		log(4, "(trace) pexpression");
		auto& type  = json.at("expr").str;
		// generate expression
		if (type == "number")
			output( IN_PUSH, json.at("value").num );
		else if (type == "variable")
			output( IN_GET, { json.at("value").str } );
		else if (type == "add" || type == "mul") {
			pexpression(json.at("lhs"));
			pexpression(json.at("rhs"));
			auto& op = json.at("operator").str;
			if      (op == "+")  output( IN_ADD );
			else if (op == "-")  output( IN_SUB );
			else if (op == "*")  output( IN_MUL );
			else if (op == "/")  output( IN_DIV );
			else    errorc("pexpression-add-mul", "unknown operator: " + op);
		}
		else
			errorc("pexpression", "unknown expression '" + type + "'");
	}
};