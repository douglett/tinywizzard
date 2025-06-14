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
	int infolevel = 1, errcount = 0, dsym = -1;

	int generate(const Json& json) {
		return error("Generator", "missing generate implementation");
	}

	void reset() {
		errcount = 0, dsym = -1;
	}

	//  === helpers ===
	
	int log(int level, const string& msg) {
		if (level >= infolevel)
			printf("[Generator] %s\n", msg.c_str());
		return true;
	}

	int errorc(const string& type, const string& msg) {
		printf("[Generator] error in %s: %s (line %d)\n", type.c_str(), msg.c_str(), dsym);
		errcount++;
		return false;
	}

	int error(const string& type, const string& msg) {
		throw runtime_error("[Generator] " + type + ": " + msg);
		return false;
	}

	void show() {
		log(1, "outputting compiled ASM to output.asm...");
		fstream fs("output.asm", ios::out);
		for (const auto& in : program)
			fs << showinstruction(in) << endl;
	}
};
