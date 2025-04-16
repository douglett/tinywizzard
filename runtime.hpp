#pragma once
#include <string>
#include <vector>
using namespace std;


struct RuntimeBase {
		enum INSTRUCTION_TYPE {
		IN_NOOP, IN_DSYM, IN_LABEL,
		IN_DIM, IN_DATA, IN_END, IN_JUMP, IN_PRINTS, IN_PRINTV, IN_INPUT, IN_PUT, IN_GET, IN_PUSH,
		IN_ADD, IN_SUB, IN_MUL, IN_DIV, IN_LT, IN_GT, IN_LTE, IN_GTE,
		IN_JUMPIF, IN_JUMPIFN
	};
	struct Instruction { INSTRUCTION_TYPE type; vector<string> args; int argi; };
};


struct Runtime : RuntimeBase {
	vector<Instruction> program;

	int run() {
		printf("running program...\n");
		return 0;
	}
};