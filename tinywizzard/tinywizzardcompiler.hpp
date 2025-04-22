#pragma once
#include "../core/compiler.hpp"
#include <vector>
#include <cassert>
using namespace std;


struct TinyWizzardCompiler : Compiler {
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

	}

	void compileast(const Json& json) {

	}
};