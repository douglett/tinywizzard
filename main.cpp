#include "parser/tinywizzardparser.hpp"
#include "generator/tinywizzardgenerator.hpp"
#include "runtime/runtime.hpp"
using namespace std;


void tinywizzard() {
	TinyWizzardParser parser;
	parser.init();
	parser.parse("scripts/test.wizz");
	// parser.parse("tinywizzard/scripts/01_basic/01_print.wizz");

	TinyWizzardGenerator gen;
	gen.generate(parser.ast.at(0));

	Runtime run;
	run.program = gen.program;
	run.run();
}


int main() {
	tinywizzard();
}