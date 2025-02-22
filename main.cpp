#include "Ruleparser.hpp"
using namespace std;


struct TestlangParser : Parser {
	int init() {
		// tokenizer settings
		tok.lcomment = "REM";
		tok.flag_eol = true;

		// initialise ruleset
		ruleset.name = "testlang";
		ruleset.add( "$program", "$line* $eof" );
		ruleset.add( "$line", "$emptyline $print", "or" );
		ruleset.add( "$emptyline", "$eol" );
		ruleset.add( "$print", "print $identifier $eol" );
		// ruleset.add( "$statement", "$print" );
		ruleset.show();
		ruleset.validate();
		return true;
	}
};


int main() {
	printf("hello world\n");

	TestlangParser parser;
	parser.init();
	parser.parse("test/test1.script");
}