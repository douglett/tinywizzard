#pragma once
#include "../parser/tokenizer.hpp"
#include "../runtime/runtimebase.hpp"
#include <vector>
#include <exception>
using namespace std;

/**
 * Language Generator
 * Generates ASM output from AST
 */
struct Generator : TokenHelpers, RuntimeBase {
	vector<Instruction> program;
	int infolevel = 1, errcount = 0, dsym = 0;

	// stub
	int generate(const Json& json) {
		return error("compiler", "missing generate implementation");
	}


	//  === helpers ===
	void show() {
		if (infolevel >= 1)
			printf("outputting compiled ASM to output.asm...\n");
		fstream fs("output.asm", ios::out);
		for (const auto& in : program)
			fs << showinstruction(in) << endl;
	}

	// error and continue
	int errorc(const string& type, const string& msg) {
		cout << "[compiler] error in " << type << ": " + msg 
			<< " (line " << dsym << ")"
			<< endl;
		errcount++;
		return false;
	}

	int error(const string& type, const string& msg) {
		throw runtime_error("[compiler] " + type + ": " + msg);
		return false;
	}
};
