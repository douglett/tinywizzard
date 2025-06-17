#pragma once
#include "../parser/tokenizer.hpp"
#include <string>
#include <vector>
#include <sstream>
using namespace std;


/**
 * Runtime instruction information and display.
 * Useful for runtime and compile stages.
 */
struct RuntimeBase {
	enum INSTRUCTION_TYPE {
		IN_NOOP, IN_DSYM, IN_LABEL,
		IN_DIM, IN_DATA, IN_MAKESTR, IN_COPYSTRL,
		IN_END, IN_JUMP, IN_CALL, IN_RETURN, IN_GETLINE, IN_INPUT, IN_PUT, IN_GET, IN_PUSH,
			IN_PRINTI, IN_PRINTC, IN_PRINTV, IN_PRINTS, IN_PRINTVS,
		IN_ADD, IN_SUB, IN_MUL, IN_DIV, IN_EQ, IN_NEQ, IN_LT, IN_GT, IN_LTE, IN_GTE,
			IN_JUMPIF, IN_JUMPIFN
	};
	struct Instruction { INSTRUCTION_TYPE type; vector<string> args; int argi; };

	static string showinstruction(const Instruction& in) {
		string s = "\t";
		switch (in.type) {
			// pseudo-instructions
			case IN_NOOP:      s = "# " + (in.args.size() ? in.args.at(0) : "");  break;
			case IN_DSYM:      s = "# dsym: line " + to_string(in.argi);  break;
			case IN_LABEL:     s = in.args.at(0) + ":";  break;
			// data
			case IN_DIM:
				s += "dim ";
				for (const auto& arg : in.args)
					s += arg + " ";
				break;
			case IN_DATA:
				s += "data "
					+ in.args.at(0) 
					+ " \"" + TokenHelpers::escapeliteral(in.args.at(1)) + "\"";
				break;
			case IN_MAKESTR:   s += "makestr";  break;
			case IN_COPYSTRL:  s += "copystrl " + in.args.at(0) + " " + in.args.at(1);  break;
			// control
			case IN_END:       s += "end";  break;
			case IN_JUMP:      s += "jump " + in.args.at(0);  break;
			case IN_CALL:      s += "call " + in.args.at(0);  break;
			case IN_RETURN:    s += "return";  break;
			case IN_GETLINE:   s += "getline";  break;
			case IN_INPUT:     s += "input " + in.args.at(0);  break;
			case IN_PUT:       s += "put " + in.args.at(0);  break;
			case IN_GET:       s += "get " + in.args.at(0);  break;
			case IN_PUSH:      s += "push " + to_string(in.argi);  break;
			case IN_PRINTI:    s += "printi " + to_string(in.argi);  break;
			case IN_PRINTC:    s += "printc " + to_string(in.argi);  break;
			case IN_PRINTV:    s += "printv " + in.args.at(0);  break;
			case IN_PRINTS:    s += "prints " + in.args.at(0);  break;
			case IN_PRINTVS:   s += "printvs " + in.args.at(0);  break;
			// maths
			case IN_ADD:       s += "add";  break;
			case IN_SUB:       s += "sub";  break;
			case IN_MUL:       s += "mul";  break;
			case IN_DIV:       s += "div";  break;
			case IN_EQ:        s += "compare_eq";  break;
			case IN_NEQ:       s += "compare_neq";  break;
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
