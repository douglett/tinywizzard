#include "parser/tinywizzardparser2.hpp"
// #include "generator/tinywizzardgenerator.hpp"
// #include "runtime/runtime.hpp"
using namespace std;


void tinywizzard() {
	TinyWizzardParser parser;
	// parser.init();
	parser.parse("scripts/test2.wizz");

	// TinyWizzardGenerator gen;
	// gen.generate(parser.ast.at(0));

	// Runtime run;
	// run.program = gen.program;
	// run.run();
}


int main() {
	tinywizzard();
}