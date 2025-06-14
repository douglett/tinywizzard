#include "parser/tinywizzardparser2.hpp"
#include "semantics/tinywizzardsemantics.hpp"
#include "generator/tinywizzardgenerator2.hpp"
#include "runtime/runtime.hpp"
using namespace std;


void tinywizzard() {
	TinyWizzardParser parser;
	parser.parse("scripts/test.wizz");

	TinyWizzardSemantics semantics;
	semantics.validate(parser.ast);

	TinyWizzardGenerator gen;
	gen.generate(parser.ast);

	Runtime run;
	run.program = gen.program;
	run.run();
}


int main() {
	tinywizzard();
}