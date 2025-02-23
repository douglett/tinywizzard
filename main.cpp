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
		ruleset.add( "$line", "$integer? $statement? $eol" );
		// statements
		ruleset.add( "$statement", "$print $input $if $let $goto $gosub", "or" );
		ruleset.add( "$print", "PRINT $stringliteral" );
		ruleset.add( "$input", "INPUT $variable $input2*" );
		ruleset.add( "$input2", ", $variable" );
		ruleset.add( "$goto", "GOTO $integer" );
		ruleset.add( "$gosub", "GOSUB $integer" );
		ruleset.add( "$if", "IF $comparison THEN $statement" );
		ruleset.add( "$let", "LET $variable = $expression" );
		// comparison expression
		ruleset.add( "$comparison", "$expression $comparison_op $expression" );
		ruleset.add( "$comparison_op", "$lte < $gte > = $noteq", "or" );
		ruleset.add( "$lte", "< =" );
		ruleset.add( "$gte", "> =" );
		ruleset.add( "$noteq", "< >" );
		// expressions
		ruleset.add( "$expression", "$add" );
		ruleset.add( "$add", "$mul $add2*" );  // $mul ((+ -)^ $mul)*
		ruleset.add( "$add2", "$add_op $mul" );
		ruleset.add( "$add_op", "$opplus -", "or" );
		ruleset.add( "$mul", "$atom $mul2*" );
		ruleset.add( "$mul2", "$mul_op $atom" );
		ruleset.add( "$mul_op", "$opmultiply /", "or" );
		ruleset.add( "$atom", "$variable $integer $brackets", "or" );
		ruleset.add( "$brackets", "( $expression )" );
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