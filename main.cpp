#include "parser/tinywizzardparser.hpp"
#include "semantics/tinywizzardsemantics.hpp"
#include "generator/tinywizzardgenerator.hpp"
#include "runtime/runtime.hpp"
using namespace std;


void tinywizzard() {
	TinyWizzardParser parser;
	parser.parse("scripts/test.wizz");

	TinyWizzardSemantics semantics;
	semantics.validate(parser.ast);
	parser.show();

	TinyWizzardGenerator gen;
	gen.generate(parser.ast);

	Runtime run;
	run.program = gen.program;
	run.run();

	int ptr = run.var("s");
	string& s = run.stringheap.at(ptr);
	printf("runtime test output: %s\n", s.c_str());
}


int main() {
	tinywizzard();
}