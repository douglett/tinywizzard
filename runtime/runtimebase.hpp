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
		IN_DIM, IN_DATA, IN_MAKESTR, IN_COPYSTR, IN_EQSTR, IN_NEQSTR,
		IN_END, IN_JUMP, IN_CALL, IN_RETURN, IN_RETURNI, IN_LOAD, IN_STORE, IN_PUSH,
			IN_PRINTI, IN_PRINTC, IN_PRINTS, IN_INPUT, IN_INPUTI,
		IN_ADD, IN_SUB, IN_MUL, IN_DIV, IN_EQ, IN_NEQ, IN_LT, IN_GT, IN_LTE, IN_GTE,
			IN_JUMPIF, IN_JUMPIFN
	};
	struct Instruction { INSTRUCTION_TYPE type; vector<string> args; int argi; };

	static string showinstruction(const Instruction& in) {
		string s = "\t";
		switch (in.type) {
			// pseudo-instructions
			case IN_NOOP:        s = "# " + (in.args.size() ? in.args.at(0) : "");  break;
			case IN_DSYM:        s = "# dsym: line " + to_string(in.argi);  break;
			case IN_LABEL:       s = in.args.at(0) + ":";  break;
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
			case IN_MAKESTR:     s += "makestr";  break;
			case IN_COPYSTR:     s += "copystr";  break;
			case IN_EQSTR:       s += "equalsstr";  break;
			case IN_NEQSTR:      s += "notequalsstr";  break;
			// control
			case IN_END:         s += "end";  break;
			case IN_JUMP:        s += "jump " + in.args.at(0);  break;
			case IN_CALL:        s += "call " + in.args.at(0);  break;
			case IN_RETURN:      s += "return";  break;
			case IN_RETURNI:     s += "returni " + to_string(in.argi);  break;
			case IN_LOAD:        s += "load " + in.args.at(0);  break;
			case IN_STORE:       s += "store " + in.args.at(0);  break;
			case IN_PUSH:        s += "push " + to_string(in.argi);  break;
			case IN_PRINTI:      s += "printi";  break;
			case IN_PRINTC:      s += "printc";  break;
			case IN_PRINTS:      s += "prints";  break;
			case IN_INPUT:       s += "input " + in.args.at(0);  break;
			case IN_INPUTI:      s += "inputi " + in.args.at(0);  break;
			// maths
			case IN_ADD:         s += "add";  break;
			case IN_SUB:         s += "sub";  break;
			case IN_MUL:         s += "mul";  break;
			case IN_DIV:         s += "div";  break;
			case IN_EQ:          s += "compare_eq";  break;
			case IN_NEQ:         s += "compare_neq";  break;
			case IN_LT:          s += "compare_lt";  break;
			case IN_GT:          s += "compare_gt";  break;
			case IN_LTE:         s += "compare_lte";  break;
			case IN_GTE:         s += "compare_gte";  break;
			case IN_JUMPIF:      s += "jumpif "  + in.args.at(0);  break;
			case IN_JUMPIFN:     s += "jumpifn " + in.args.at(0);  break;
		}
		return s;
	}
};
