#pragma once
#include "../core/compiler.hpp"
#include <vector>
#include <cassert>
using namespace std;


struct TinyWizzardCompiler : Compiler {
	vector<Instruction> header;
	string classname, funcname;
	int litcount = 0;

	int compile(const Json& json) {
		// begin
		printf("-----\n");
		printf("compiling program...\n");
		assert(json.at("type").str == "$program");  // make sure first item is a $program
		// compile
		initheader();      // initialise program header
		compileast(json);  // compile program
		if (errcount)  error("compiler", "compile failed with " + to_string(errcount) + " errors.");
		// success
		program.insert(program.begin(), header.begin(), header.end());
		show();
		printf("compiled successfully!\n");
		return true;
	}

	void initheader() {
		printf("initialising compiler...\n");
		errcount = litcount = 0;
		program = header = {};
		header.push_back({ IN_NOOP, { "# literals and other data" } });
		header.push_back({ IN_DATA, { "STRING_LIT_NEWLINE", "\n" } });
		header.push_back({ IN_DATA, { "STRING_LIT_SPACE", " " } });
	}

	void compileast(const Json& json) {
		assert(json.type == Json::JOBJECT);
		auto& type = json.at("type").str;

		// output debug symbols
		if (json.count("dsym")) {
			dsym = json.at("dsym").num;
			program.push_back({ IN_DSYM, {}, dsym });
		}
		
		// compile class definitions
		if (type == "$program") {
			for (auto& line : json.at("value").arr)
				compileast(line);
		}
		else if (type == "$classdef") {
			classname = json.at("value").at(1).at("value").str;
			printf("compiling class: %s\n", classname.c_str());
		}
		else if (type == "$function") {
			funcname = json.at("value").at(1).at("value").str;
			printf("compiling function: %s\n", funcname.c_str());
			compileast(json.at("value").at(2));
			funcname = "";
		}
		else if (type == "$dim") {
			// TODO: only works for global DIM
			assert(funcname == "");
			header.push_back({ IN_DIM, { json.at("value").at(1).at("value").str } });
		}
		else if (type == "$block") {
			for (auto& stmt : json.at("value").arr)
				compileast(stmt);
		}
		else if (type == "$print") {
			for (auto& printval : json.at("value").arr) {
				// print integer
				if (printval.at("type").str == "$integer")
					program.push_back({ IN_PRINTI, {}, int(printval.at("value").num) });
				// print variable value
				else if (printval.at("type").str == "$variable")
					program.push_back({ IN_PRINTV, { printval.at("value").str } });
				// print string literal
				else if (printval.at("type").str == "$stringliteral") {
					auto name = "STRING_LIT_" + to_string(++litcount);
					header.push_back({ IN_DATA, { name, stripliteral(printval.at("value").str) } });
					program.push_back({ IN_PRINTS, { name } });
				}
				else
					error(type, "unknown type: " + printval.at("type").str);
				// space-seperate values
				program.push_back({ IN_PRINTS, { "STRING_LIT_SPACE" } });
			}
			program.push_back({ IN_PRINTS, { "STRING_LIT_NEWLINE" } });
		}
		else if (type == "$assign") {
			compileast(json.at("value").at(1));
			auto& varname = json.at("value").at(0).at("value").str;
			program.push_back({ IN_PUT, { varname } });
		}

		// expressions
		else if (type == "$integer") {
			program.push_back({ IN_PUSH, {}, int(json.at("value").num) });
		}

		// unknown 
		else
			errorc(type, "unknown rule");
	}
};