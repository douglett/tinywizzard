#include "parser/tinywizzardparser.hpp"
#include "semantics/tinywizzardsemantics.hpp"
#include "generator/tinywizzardgenerator.hpp"
#include "runtime/runtime.hpp"
using namespace std;


void tinywizzard(const string& fname) {
	TinyWizzardParser parser;
	parser.parse(fname);

	TinyWizzardSemantics semantics;
	semantics.validate(parser.ast);
	parser.show();  // update with type info

	TinyWizzardGenerator gen;
	gen.generate(parser.ast);

	Runtime run;
	run.program = gen.program;
	run.run();
}


int main(int argc, char** argv) {
	string script = argc >= 2 ? argv[1] : "scripts/test.wizz";
	tinywizzard(script);
}