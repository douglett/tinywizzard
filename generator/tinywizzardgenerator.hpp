#pragma once
#include "generator.hpp"
#include <algorithm>
using namespace std;


struct TinyWizzardGenerator : Generator {
	struct Func { string name; vector<Instruction> ilist; };
	map<string, Func> functions;
	string funcname;
	vector<string> literals;
	int conditions = 0;

	int generate(const Json& json) {
		// loglevel = 4;  // 4 = trace
		log(1, "generating single-class program...");
		reset();
		// compile
		pclass(json);
		// success or fail
		if (errcount)
			error("Generator", "compile failed with " + to_string(errcount) + " errors.");
		outputstrlit();
		outputfunctions();
		show();
		log(1, "compiled successfully!");
		return true;
	}

	void reset() {
		Generator::reset();
		functions = {};
		functions["$STATIC_INIT"] = { "$STATIC_INIT" };
		funcname = "";
		literals = {};
		conditions = 0;
	}

	// === generate helpers ===

	vector<Instruction>& getilist() {
		return funcname.length() ? functions.at(funcname).ilist : functions.at("$STATIC_INIT").ilist;
	}
	void output(INSTRUCTION_TYPE type, int argi) {
		getilist().push_back({ type, {}, argi });
	}
	void output(INSTRUCTION_TYPE type, const vector<string>& args={}, int argi=0) {
		getilist().push_back({ type, args, argi });
	}

	string addstrlit(const string& str) {
		literals.push_back(str);
		return "$STRLIT_" + to_string(literals.size() - 1);
	}

	string nextlabel() {
		return "$COND_" + to_string(conditions++);
	}

	void outputstrlit() {
		auto& fn = functions.at("$STATIC_INIT").ilist;
		fn.insert(fn.begin(), { IN_NOOP, { "string-literals" } });
		for (size_t i = 0; i < literals.size(); i++)
			fn.insert(fn.begin()+i+1, { IN_DATA, { "$STRLIT_" + to_string(i), literals.at(i) } });
	}

	void outputfunctions() {
		log(1, "ouputting functions to main program...");
		// calculate order
		vector<string> order = { "$STATIC_INIT", "main" };
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
		for (auto& dim : json.at("dims").arr)
			pdim(dim);
		for (auto& func : json.at("functions").arr)
			pfunction(func);
	}

	void pdim(const Json& json) {
		log(4, "(trace) pdim");
		auto& name  = json.at("name").str;
		auto& type  = json.at("type").str;
		dsym        = json.at("dsym").num;
		// generate dim
		output( IN_DSYM, dsym );
		output( IN_DIM, { name } );
		if (type == "int") {
			if (json.count("expression")) {
				pexpression(json.at("expression"));
				output( IN_PUT, { name } );
			}
		}
		else if (type == "string") {
			output( IN_MAKESTR );
			output( IN_PUT, { name } );
			if (json.count("expression")) {
				pexpressionstr(json.at("expression"), name);
			}
		}
		else
			errorc("pdim", "unknown type '" + type + "'");
	}

	void pfunction(const Json& json) {
		log(4, "(trace) pfunction");
		funcname            = json.at("name").str;
		functions[funcname] = { funcname };
		dsym                = json.at("dsym").num;
		for (auto& stmt : json.at("block").arr)
			pstatement(stmt);
		funcname = "";
	}

	// === statements ===

	void pstatement(const Json& json) {
		log(4, "(trace) pstatement");
		auto& stmt  = json.at("statement").str;
		dsym        = json.at("dsym").num;
		output( IN_DSYM, dsym );
		// assign
		if (stmt == "assign") {
			auto& type = json.at("type").str;
			if (type == "int") {
				pexpression(json.at("expression"));
				output( IN_PUT, { json.at("name").str } );
			}
			else if (type == "string") {
				pexpressionstr(json.at("expression"), json.at("name").str);
			}
			else
				errorc("pstatement-assign", "unknown type '" + type + "'");
		}
		// print
		else if (stmt == "print") {
			for (auto& printval : json.at("printvals").arr) {
				auto& type = printval.at("expr").str;
				if (type == "strlit") {
					auto litname = addstrlit(printval.at("value").str);
					output( IN_PRINTS, { litname } );
				}
				else if (type == "variable" && printval.at("type").str == "string") {
					output( IN_PRINTVS, { printval.at("value").str } );
				}
				else {
					pexpression(printval);
					output( IN_PRINTV, { "$POP" } );
				}
				output( IN_PRINTC, ' ' );  // space-seperate values
			}
			output( IN_PRINTC, '\n' );  // end-of-line
		}
		// input
		else if (stmt == "input") {
			string promptid;
			if (json.count("prompt"))
				promptid = addstrlit(json.at("prompt").str);
			else
				promptid = addstrlit("> ");
			output( IN_PRINTS, { promptid } );
			auto& varname = json.at("variable").at("value").str;
			auto& type    = json.at("variable").at("type").str;
			if (type == "int")
				output( IN_INPUTI, { varname } );
			else if (type == "string")
				output( IN_INPUT, { varname } );
			else
				errorc("pstatement-input", "unknown type '" + type + "'");
		}
		// if-else
		else if (stmt == "if") {
			string nextcond, endcond = nextlabel();
			for (auto& cond : json.at("conditionals").arr) {
				// check block entry condition
				nextcond = nextlabel();
				if (cond.at("conditional").str != "else") {
					pexpression(cond.at("expression"));
					output( IN_JUMPIFN, { nextcond } );
				}
				// condition success point
				for (auto& stmt : cond.at("block").arr)
					pstatement(stmt);
				output( IN_JUMP, { endcond } );
				// condition fail point
				output( IN_LABEL, { nextcond } );
			}
			// if-block end
			output( IN_LABEL, { endcond } );
		}
		// unknown
		else
			errorc("pstatement", "unknown statement '" + stmt + "'");
	}

	// === expressions ===

	void pexpression(const Json& json) {
		log(4, "(trace) pexpression");
		auto& expr = json.at("expr").str;
		// generate expression
		if (expr == "integer")
			output( IN_PUSH, json.at("value").num );
		else if (expr == "variable")
			output( IN_GET, { json.at("value").str } );
		else if (expr == "add" || expr == "mul" || expr == "equals") {
			pexpression(json.at("lhs"));
			pexpression(json.at("rhs"));
			auto& op = json.at("operator").str;
			if      (op == "+" )  output( IN_ADD );
			else if (op == "-" )  output( IN_SUB );
			else if (op == "*" )  output( IN_MUL );
			else if (op == "/" )  output( IN_DIV );
			else if (op == "==")  output( IN_EQ  );
			else if (op == "!=")  output( IN_NEQ );
			else if (op == "<=")  output( IN_LTE );
			else if (op == ">=")  output( IN_GTE );
			else if (op == "<" )  output( IN_LT  );
			else if (op == ">" )  output( IN_GT  );
			else    errorc("pexpression-add-mul-equals", "unknown operator '" + op + "'");
		}
		else
			errorc("pexpression", "unknown expression '" + expr + "'");
	}

	void pexpressionstr(const Json& json, const string& varname) {
		log(4, "(trace) pexpressionstr");
		auto& expr = json.at("expr").str;
		// handle string expression
		if (expr == "strlit") {
			auto litname = addstrlit(json.at("value").str);
			output( IN_COPYSTRL, { varname, litname } );
		}
		else if (expr == "variable") {
			output( IN_COPYSTRV, { varname, json.at("value").str } );
		}
		else
			errorc("pexpression", "unknown string expression '" + expr + "'");
	}
};