#pragma once
#include "runtimebase.hpp"
#include <string>
#include <vector>
#include <sstream>
using namespace std;


/**
 * Common Runtime
 */
struct Runtime : RuntimeBase {
	vector<Instruction> program;
	map<string, string> data;
	map<string, int> variables;
	vector<int> stack, callstack;
	stringstream ss;
	string s;
	size_t PC = 0;

	int run() {
		printf("-----\n");
		printf("running program...\n");
		return call("STATIC_INIT") && call("main");
	}

	int call(const string& funcname) {
		pushst();
		jump(funcname);
		return run_instructions(true);
	}

	int run_instructions(bool external=false) {
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
				case IN_CALL:      pushst();  jump(instr.args.at(0));  break;
				case IN_RETURN:    popst();  if (external) return true;  break;
				case IN_GETLINE:   getline(cin, s);  ss.str(s); ss.clear();  break;
				case IN_INPUT:     a = 0; ss >> a; variables.at(instr.args.at(0)) = a;  break;
				case IN_PUT:       variables.at(instr.args.at(0)) = pop();  break;
				case IN_GET:       push( variables.at(instr.args.at(0)) );  break;
				case IN_PUSH:      push(instr.argi);  break;
				case IN_PRINTI:    cout << instr.argi;  break;
				case IN_PRINTC:    cout << (char)instr.argi;  break;
				case IN_PRINTV:    cout << variables.at(instr.args.at(0));  break;
				case IN_PRINTS:    cout << data.at(instr.args.at(0));  break;
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

	int push(int t) {
		stack.push_back(t);
		return t;
	}
	int pop() {
		if (stack.size() == 0)
			error("pop", "stack empty");
		int t = stack.back();
		stack.pop_back();
		return t;
	}
	void pushst() {
		callstack.push_back(PC);
	}
	void popst() {
		if (callstack.size() == 0)
			error("popst", "callstack empty");
		PC = callstack.back();
		callstack.pop_back();
	}
	int jump(const string& label) {
		for (size_t i = 0; i < program.size(); i++)
			if (program[i].type == IN_LABEL && program[i].args.at(0) == label)
				return PC = i;
		return error("jump", "missing label: " + label);
	}

	int error(const string& type, const string& msg) {
		throw runtime_error("[runtime]: " + type + ": " + msg);
		return false;
	}
};
