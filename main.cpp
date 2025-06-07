#include "parser/tinywizzardparser.hpp"
#include "generator/tinywizzardgenerator.hpp"
#include "generator/tinywizzardgenerator2.hpp"
#include "runtime/runtime.hpp"
using namespace std;


void tinywizzard() {
	TinyWizzardParser parser;
	parser.init();
	parser.parse("scripts/test.wizz");
	// parser.parse("tinywizzard/scripts/01_basic/01_print.wizz");

	// TinyWizzardGenerator gen;
	// gen.generate(parser.ast.at(0));
	TinyWizzardGenerator2 gen2;
	gen2.generate(parser.ast.at(0));

	// Runtime run;
	// run.program = gen.program;
	// run.run();
}


int main() {
	tinywizzard();
}