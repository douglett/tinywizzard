#pragma once
#include "tokenizer.hpp"
#include "runtimebase.hpp"
#include <vector>
#include <exception>
using namespace std;

/**
 * Language Compiler
 * Compiles from AST to ASM output
 */
struct Compiler : TokenHelpers, RuntimeBase {
	vector<Instruction> program;
	int errcount = 0, dsym = 0;

	// stub
	int compile(const Json& json) {
		return error("compiler", "missing compile implementation");
	}


	//  === helpers ===
	void show() {
		printf("outputting compiled ASM to output.asm...\n");
		fstream fs("output.asm", ios::out);
		for (const auto& in : program)
			fs << showinstruction(in) << endl;
	}

	// error and continue
	int errorc(const string& type, const string& msg) {
		cout << "compiler error in " << type << ": " + msg 
			<< " (line " << dsym << ")"
			<< endl;
		errcount++;
		return false;
	}

	int error(const string& type, const string& msg) {
		throw runtime_error(type + ": " + msg);
		return false;
	}
};
