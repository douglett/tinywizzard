#include "core/core.hpp"
#include "tinybasic/tinybasicparser.hpp"
#include "tinybasic/tinybasiccompiler.hpp"
using namespace std;


int main() {
	printf("hello world\n");

	// TestlangParser parser;
	// parser.init();
	// parser.parse("test/test1.script");

	TinybasicParser parser;
	parser.init();
	// parser.parse("tinybasic/scripts/test1.bas");
	// parser.parse("tinybasic/scripts/lander.bas");
	parser.parse("tinybasic/scripts/hurkle.bas");

	TinybasicCompiler comp;
	comp.compile(parser.ast.at(0));

	Runtime run;
	run.program = comp.program;
	run.run();
}