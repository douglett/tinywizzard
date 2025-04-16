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

	static string showinstruction(const Instruction& in) {
		string s = "\t";
		switch (in.type) {
			// pseudo-instructions
			case IN_NOOP:      s = in.args.size() ? in.args.at(0) : "";  break;
			case IN_DSYM:      s = "# dsym: line " + to_string(in.argi);  break;
			case IN_LABEL:     s = in.args.at(0) + ":";  break;
			// data
			case IN_DIM:
				s += "dim ";
				for (const auto& arg : in.args)
					s += arg + " ";
				break;
			case IN_DATA:
				s += "data ";
				for (const auto& arg : in.args)
					s += arg + " ";
				break;
			// control
			case IN_END:       s += "end";  break;
			case IN_JUMP:      s += "jump " + in.args.at(0);  break;
			case IN_PRINTS:    s += "prints " + in.args.at(0);  break;
			case IN_PRINTV:    s += "printv " + in.args.at(0);  break;
			case IN_INPUT:     s += "input " + in.args.at(0);  break;
			case IN_PUT:       s += "put " + in.args.at(0);  break;
			case IN_GET:       s += "get " + in.args.at(0);  break;
			case IN_PUSH:      s += "push " + to_string(in.argi);  break;
			// maths
			case IN_ADD:       s += "add";  break;
			case IN_SUB:       s += "sub";  break;
			case IN_MUL:       s += "mul";  break;
			case IN_DIV:       s += "div";  break;
			case IN_LT:        s += "compare_lt";  break;
			case IN_GT:        s += "compare_gt";  break;
			case IN_LTE:       s += "compare_lte";  break;
			case IN_GTE:       s += "compare_gte";  break;
			case IN_JUMPIF:    s += "jump_if";  break;
			case IN_JUMPIFN:   s += "jump_ifn";  break;
		}
		return s;
	}
};


struct Runtime : RuntimeBase {
	vector<Instruction> program;
	map<string, string> data;
	map<string, int> variables;
	size_t PC = 0;

	int run() {
		printf("running program...\n");

		while (true) {
			const auto& instr = program.at(PC);
			switch (instr.type) {
				case IN_NOOP:   break;
				case IN_DSYM:   break;
				case IN_LABEL:  break;
				case IN_DIM:
					for (auto& vname : instr.args)
						variables[vname] = 0;
					break;
				case IN_DATA:
					data[instr.args.at(0)] = instr.args.at(1);
					break;
				default:
					error("unhandled-instruction", showinstruction(instr));
			}
			// next instruction
			PC++;
		}

		// OK
		return true;
	}

	int error(const string& type, const string& msg) {
		// cout << type + ": " + msg << endl;
		throw runtime_error(type + ": " + msg);
		return false;
	}
};