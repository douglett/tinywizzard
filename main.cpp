#include "core/core.hpp"
#include "tinywizzard/tinywizzardparser.hpp"
#include "tinywizzard/tinywizzardcompiler.hpp"
using namespace std;


void tinywizzard() {
	TinyWizzardParser parser;
	parser.init();
	parser.parse("tinywizzard/scripts/test.wizz");
	// parser.parse("tinywizzard/scripts/01_basic/01_print.wizz");

	TinyWizzardCompiler comp;
	comp.compile(parser.ast.at(0));

	Runtime run;
	run.program = comp.program;
	run.run();
}


int main() {
	// tinybasic();
	tinywizzard();
}