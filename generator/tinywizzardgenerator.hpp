#pragma once
#include "generator.hpp"
#include <vector>
#include <cassert>
using namespace std;


struct TinyWizzardGenerator : Generator {
	struct Func { string name; vector<Instruction> ilist; };
	map<string, Func> functions;
	string classname, funcname;
	int litcount = 0;

	int generate(const Json& json) {
		// begin
		if (infolevel >= 1)
			printf("-----\n"),
			printf("generating single-class program...\n");
		// compile
		reset();
		compileclass(json);
		// error
		if (errcount)
			error("compiler", "compile failed with " + to_string(errcount) + " errors.");
		// success
		program.insert(program.end(), functions["STATIC_INIT"].ilist.begin(), functions["STATIC_INIT"].ilist.end());
		for (const auto& [name, fn] : functions)
			if (name != "STATIC_INIT")
				program.insert(program.end(), fn.ilist.begin(), fn.ilist.end());
		show();
		if (infolevel >= 1)
			printf("compiled successfully!\n");
		return true;
	}

	void reset() {
		Generator::reset();
		classname = funcname = "";
		functions = {};
		litcount = 0;
		functions["STATIC_INIT"] = { "STATIC_INIT", {{ IN_LABEL, {"STATIC_INIT"} }} };
	}

	void compileclass(const Json& jclass) {
		assert(jclass.at("type").str == "$program");  // make sure first item is a $program
		for (auto& json : jclass.at("value").arr) {
			auto& type = json.at("type").str;
			auto& value = json.at("value");
			auto& ilist = functions.at("STATIC_INIT").ilist;

			if (json.count("dsym"))
				dsym = json.at("dsym").num;

			if (type == "$classdef") {
				classname = value.at(1).at("value").str;
				printf("compiling class: %s\n", classname.c_str());
			}
			else if (type == "$dim") {
				assert(funcname == "");  // TODO: only works for global DIM
				ilist.push_back({ IN_DSYM, {}, dsym });
				auto varname = value.at(1).at("value").str;
				ilist.push_back({ IN_DIM, { varname } });
				// dim and assign
				if (value.size() == 3) {
					compileexpr(value.at(2));
					ilist.push_back({ IN_PUT, { varname } });
				}
			}
			else if (type == "$function") {
				funcname = value.at(1).at("value").str;
				printf("compiling function: %s\n", funcname.c_str());
				functions[funcname] = { funcname, {{ IN_LABEL, {funcname} }} };
				compilestmt(value.at(2));
				funcname = "";
			}

			// unknown
			else
				errorc(type, "unknown rule in class");
		}
	}

	void compilestmt(const Json& json) {
		auto& type = json.at("type").str;
		auto& value = json.at("value");
		auto& ilist = functions.at(funcname).ilist;

		if (json.count("dsym"))
			dsym = json.at("dsym").num;

		if (type == "$block") {
			for (auto& stmt : value.arr)
				compilestmt(stmt);
		}
		else if (type == "$assign") {
			ilist.push_back({ IN_DSYM, {}, dsym });
			compileexpr(value.at(1));
			auto& varname = value.at(0).at("value").str;
			ilist.push_back({ IN_PUT, { varname } });
		}
		else if (type == "$print") {
			ilist.push_back({ IN_DSYM, {}, dsym });
			for (auto& printval : value.arr) {
				// print integer
				if (printval.at("type").str == "$integer")
					ilist.push_back({ IN_PRINTI, {}, int(printval.at("value").num) });
				// print variable value
				else if (printval.at("type").str == "$variable")
					ilist.push_back({ IN_PRINTV, { printval.at("value").str } });
				// print string literal
				else if (printval.at("type").str == "$stringliteral") {
					auto name = "STRING_LIT_" + to_string(++litcount);
					functions.at("STATIC_INIT").ilist.push_back({ IN_DATA, { name, stripliteral(printval.at("value").str) } });
					ilist.push_back({ IN_PRINTS, { name } });
				}
				else
					error(type, "unknown type: " + printval.at("type").str);
				// space-seperate values
				ilist.push_back({ IN_PRINTC, {}, ' ' });
			}
			ilist.push_back({ IN_PRINTC, {}, '\n' });
		}

		// unknown
		else
			errorc(type, "unknown rule in statement");
	}

	void compileexpr(const Json& json) {
		auto& type = json.at("type").str;
		auto& value = json.at("value");
		auto& ilist = funcname == "" ? functions.at("STATIC_INIT").ilist : functions.at(funcname).ilist;

		if (type == "$integer") {
			ilist.push_back({ IN_PUSH, {}, int(value.num) });
		}
		else if (type == "$variable") {
			ilist.push_back({ IN_GET, { value.str } });
		}
		else if (type == "$add" || type == "$mul") {
			compileexpr(value.at(0));
			compileexpr(value.at(1));
			auto& op = json.at("operator").str;
			if      (op == "+")  ilist.push_back({ IN_ADD });
			else if (op == "-")  ilist.push_back({ IN_SUB });
			else if (op == "*")  ilist.push_back({ IN_MUL });
			else if (op == "/")  ilist.push_back({ IN_DIV });
			else    errorc(type, "unknown operator: " + op);
		}

		// unknown
		else
			errorc(type, "unknown rule in expr");
	}
};