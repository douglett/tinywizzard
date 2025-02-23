#include "Ruleparser.hpp"
using namespace std;


struct TestlangParser : Parser {
	int init() {
		// tokenizer settings
		// tok.lcomment = "REM";
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


struct TinybasicParser : Parser {
	int init() {
		// tokenizer settings
		tok.lcomment = "REM";
		tok.flag_eol = true;

		// initialise ruleset
		ruleset.name = "testlang";
		ruleset.add( "$program", "$line* $eof" );
		ruleset.add( "$line", "$statement? $eol" );
		// statements
		ruleset.add( "$statement", "$print $input $if $let $gosub", "or" );
		ruleset.add( "$print", "PRINT $stringliteral" );
		ruleset.add( "$input", "INPUT $variable" );
		ruleset.add( "$gosub", "GOSUB $integer" );
		ruleset.add( "$if", "IF $comparison THEN $let" );
		ruleset.add( "$let", "LET $variable = $integer" );
		// expressions
		ruleset.add( "$comparison", "$expression $comparison_op $expression" );
		ruleset.add( "$expression", "$variable $integer", "or" );
		ruleset.add( "$comparison_op", "= < $lte > $gte $noteq", "or" );
		ruleset.add( "$lte", "< =" );
		ruleset.add( "$gte", "> =" );
		ruleset.add( "$noteq", "< >" );
		ruleset.add( "$variable", "A B C D E F G H I J K L M N O P Q R S T U V W X Y Z", "or" );

		ruleset.show();
		ruleset.validate();
		return true;
	}
};


int main() {
	printf("hello world\n");

	// TestlangParser parser;
	// parser.init();
	// parser.parse("test/test1.script");

	TinybasicParser parser;
	parser.init();
	parser.parse("basic/hurkle.bas");
}