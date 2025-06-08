#pragma once
#include "generator.hpp"
#include <vector>
#include <cassert>
using namespace std;


struct TinyWizzardGenerator2 : Generator {
	struct Func { string name; vector<Instruction> ilist; };
	map<string, Func> functions;
	string classname, funcname;

	int generate(const Json& json) {
		// begin
		if (infolevel >= 1)
			printf("-----\n"),
			printf("generating single-class program...\n");
		// compile
		reset();
		compileclass(json);
		// error
		// if (errcount)
		// 	error("compiler", "compile failed with " + to_string(errcount) + " errors.");
		// success
		printf("len: %d\n", (int)functions["STATIC_INIT"].ilist.size());
		program.insert(program.begin(), functions["STATIC_INIT"].ilist.begin(), functions["STATIC_INIT"].ilist.end());
		show();
		// if (infolevel >= 1)
		// 	printf("compiled successfully!\n");
		return true;
	}

	void reset() {
		Generator::reset();
		classname = funcname = "";
		functions = {};
		functions["STATIC_INIT"] = { "STATIC_INIT", {{ IN_LABEL, {"STATIC_INIT"} }} };
	}

	void compileclass(const Json& jclass) {
		assert(jclass.at("type").str == "$program");  // make sure first item is a $program
		for (auto& json : jclass.at("value").arr) {
			auto& type = json.at("type").str;
			auto& value = json.at("value");
			auto& ilist = functions["STATIC_INIT"].ilist;

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
				classname = value.at(1).at("value").str;
				printf("compiling function: %s\n", classname.c_str());
				compilestmt(value.at(2));
			}

			// unknown
			else
				errorc(type, "unknown rule in class");
		}
	}

	void compilestmt(const Json& json) {
		auto& type = json.at("type").str;
		auto& value = json.at("value");
		auto& ilist = functions["STATIC_INIT"].ilist;

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

		// unknown
		else
			errorc(type, "unknown rule in statement");
	}

	void compileexpr(const Json& json) {
		auto& type = json.at("type").str;
		auto& value = json.at("value");
		auto& ilist = functions["STATIC_INIT"].ilist;

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