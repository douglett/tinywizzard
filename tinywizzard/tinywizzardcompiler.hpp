#pragma once
#include "../core/compiler.hpp"
#include <vector>
#include <cassert>
using namespace std;


struct TinyWizzardCompiler : Compiler {
	string classname;

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
		show();
		printf("compiled successfully!\n");
		return true;
	}

	void initheader() {
		printf("initialising compiler...\n");
		errcount = 0;
		program = {};
		program.push_back({ IN_NOOP, { "# literals and other data" }});
		program.push_back({ IN_DATA, { "STRING_LIT_NEWLINE", "\n" } });
		program.push_back({ IN_DATA, { "STRING_LIT_SPACE", " " } });
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
			string funcname = json.at("value").at(1).at("value").str;
			printf("compiling function: %s\n", funcname.c_str());
			compileast(json.at("value").at(2));
		}
		else if (type == "$block") {
			for (auto& stmt : json.at("value").arr)
				compileast(stmt);
		}
		else if (type == "$print") {
			for (auto& printval : json.at("value").arr) {
				// print value
				if (printval.at("type").str == "$integer")
					program.push_back({ IN_PRINTI, {}, int(printval.at("value").num) });
				else
					error(type, "unknown type: " + printval.at("type").str);
				// space-seperate values
				program.push_back({ IN_PRINTS, { "STRING_LIT_SPACE" } });
			}
			program.push_back({ IN_PRINTS, { "STRING_LIT_NEWLINE" } });
		}
		else
			errorc(type, "unknown rule");
	}
};