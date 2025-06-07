#pragma once
#include "generator.hpp"
#include <vector>
#include <cassert>
using namespace std;


struct TinyWizzardGenerator2 : Generator {
	struct Func { string name; vector<string> inst; };
	map<string, Func> functions;
	string classname, funcname;

	int generate(const Json& json) {
		// begin
		if (infolevel >= 1)
			printf("-----\n"),
			printf("generating single-class program...\n");
		assert(json.at("type").str == "$program");  // make sure first item is a $program
		compileclass(json);
		return true;
	}

	void compileclass(const Json& jsonclass) {
		for (auto& json : jsonclass.at("value").arr) {
			auto& type = json.at("type").str;
			auto& value = json.at("value");

			if (type == "$classdef") {
				classname = value.at(1).at("value").str;
				printf("compiling class: %s\n", classname.c_str());
			}

			// unknown 
			else
				errorc(type, "unknown rule");
		}
	}
};