#pragma once
#include <string>
#include <vector>
#include <sstream>
using namespace std;


struct RuntimeBase {
		enum INSTRUCTION_TYPE {
		IN_NOOP, IN_DSYM, IN_LABEL,
		IN_DIM, IN_DATA, IN_END, IN_JUMP, IN_CALL, IN_RETURN, IN_PRINTS, IN_PRINTV, IN_GETLINE, IN_INPUT, IN_PUT, IN_GET, IN_PUSH,
		IN_ADD, IN_SUB, IN_MUL, IN_DIV, IN_EQ, IN_NEQ, IN_LT, IN_GT, IN_LTE, IN_GTE,
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
			case IN_CALL:      s += "call " + in.args.at(0);  break;
			case IN_RETURN:    s += "return";  break;
			case IN_PRINTS:    s += "prints " + in.args.at(0);  break;
			case IN_PRINTV:    s += "printv " + in.args.at(0);  break;
			case IN_GETLINE:   s += "getline";  break;
			case IN_INPUT:     s += "input " + in.args.at(0);  break;
			case IN_PUT:       s += "put " + in.args.at(0);  break;
			case IN_GET:       s += "get " + in.args.at(0);  break;
			case IN_PUSH:      s += "push " + to_string(in.argi);  break;
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


struct Runtime : RuntimeBase {
	vector<Instruction> program;
	map<string, string> data;
	map<string, int> variables;
	vector<int> stack, callstack;
	stringstream ss;
	string s;
	size_t PC = 0;

	int run() {
		printf("running program...\n");
		int a = 0, b = 0;

		while (PC < program.size()) {
			const auto& instr = program.at(PC);
			switch (instr.type) {
				// pseudo-instructions
				case IN_NOOP:   break;
				case IN_DSYM:   break;
				case IN_LABEL:  break;
				// data
				case IN_DIM:
					for (auto& vname : instr.args)
						variables[vname] = 0;
					break;
				case IN_DATA:
					data[instr.args.at(0)] = instr.args.at(1);
					break;
				// control
				case IN_END:       return true;
				case IN_JUMP:      jump(instr.args.at(0));  break;
				case IN_CALL:      callstack.push_back(PC);  jump(instr.args.at(0));  break;
				case IN_RETURN:    PC = callstack.back();  callstack.pop_back();  break;
				case IN_PRINTS:    cout << data.at(instr.args.at(0));  break;
				case IN_PRINTV:    cout << variables.at(instr.args.at(0));  break;
				case IN_GETLINE:   getline(cin, s);  ss.str(s); ss.clear();  break;
				case IN_INPUT:     a = 0; ss >> a; variables.at(instr.args.at(0)) = a;  break;
				case IN_PUT:       variables.at(instr.args.at(0)) = pop();  break;
				case IN_GET:       push( variables.at(instr.args.at(0)) );  break;
				case IN_PUSH:      push(instr.argi);  break;
				// maths
				case IN_ADD:       b = pop(), a = pop(), push(a +  b);  break;
				case IN_SUB:       b = pop(), a = pop(), push(a -  b);  break;
				case IN_MUL:       b = pop(), a = pop(), push(a *  b);  break;
				case IN_DIV:       b = pop(), a = pop(), push(a /  b);  break;
				case IN_EQ:        b = pop(), a = pop(), push(a == b);  break;
				case IN_NEQ:       b = pop(), a = pop(), push(a != b);  break;
				case IN_LT:        b = pop(), a = pop(), push(a <  b);  break;
				case IN_GT:        b = pop(), a = pop(), push(a >  b);  break;
				case IN_LTE:       b = pop(), a = pop(), push(a <= b);  break;
				case IN_GTE:       b = pop(), a = pop(), push(a >= b);  break;
				case IN_JUMPIF:    if (pop())  jump(instr.args.at(0));  break;
				case IN_JUMPIFN:   if (!pop()) jump(instr.args.at(0));  break;
				default:           error("unhandled-instruction", showinstruction(instr));
			}
			// next instruction
			PC++;
		}

		// OK
		printf("end of program: program terminated\n");
		return true;
	}

	int pop() {
		int t = stack.back();
		stack.pop_back();
		return t;
	}
	int push(int t) {
		stack.push_back(t);
		return t;
	}
	int jump(const string& label) {
		for (size_t i = 0; i < program.size(); i++)
			if (program[i].type == IN_LABEL && program[i].args.at(0) == label)
				return PC = i;
		return error("jump", "missing label: " + label);
	}

	int error(const string& type, const string& msg) {
		// cout << type + ": " + msg << endl;
		throw runtime_error(type + ": " + msg);
		return false;
	}
};