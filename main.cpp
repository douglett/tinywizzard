#include "core/core.hpp"
#include "tinybasic/tinybasicparser.hpp"
#include "tinybasic/tinybasiccompiler.hpp"
#include "tinywizzard/tinywizzardparser.hpp"
#include "tinywizzard/tinywizzardcompiler.hpp"
using namespace std;


void tinybasic() {
	printf("Running tests for TinyBasic paser...\n");
	TinybasicParser parser;
	parser.init();
	// parser.parse("tinybasic/scripts/test1.bas");
	// parser.parse("tinybasic/scripts/lander.bas");
	// parser.parse("tinybasic/scripts/hurkle.bas");
	parser.parse("tinybasic/scripts/tictactoe.bas");

	TinybasicCompiler comp;
	comp.compile(parser.ast.at(0));

	Runtime run;
	run.program = comp.program;
	run.run();
}


void tinywizzard() {
	TinyWizzardParser parser;
	parser.init();
	parser.parse("tinywizzard/scripts/test.wizz");
	// parser.parse("tinywizzard/scripts/01_basic/01_print.wizz");

	TinyWizzardCompiler comp;
	comp.compile(parser.ast.at(0));
}


int main() {
	// tinybasic();
	tinywizzard();
}