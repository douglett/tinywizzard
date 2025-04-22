#pragma once
#include "../core/core.hpp"
using namespace std;


/**
 * Parses TinyBasic language file to json AST
 */
struct TinybasicParser : ASTParser {
	int init() {
		// tokenizer settings
		tok.lcomment = "REM";
		tok.flag_eol = true;

		// initialise ruleset
		ruleset.name = "testlang";
		ruleset.add( "$program", "$line* $eof!" );
		ruleset.add( "$line", "$dsym $integer? $statement? $eol" );
		// statements
		ruleset.add( "$statement", "$print $input $if $let $goto $gosub $return $end", "or" );
		ruleset.add( "$print", "PRINT $print_val $print2*" );
		ruleset.add( "$print2", ", $print_val" );
		ruleset.add( "$print_val", "$stringliteral $variable", "or" );
		ruleset.add( "$input", "INPUT $variable $input2*" );
		ruleset.add( "$input2", ", $variable" );
		ruleset.add( "$goto", "GOTO $integer" );
		ruleset.add( "$gosub", "GOSUB $integer" );
		ruleset.add( "$if", "IF $comparison THEN $statement" );
		ruleset.add( "$let", "LET $variable = $expression" );
		ruleset.add( "$return", "RETURN" );
		ruleset.add( "$end", "END" );
		// comparison expression
		ruleset.add( "$comparison", "$expression $comparison_op $expression" );
		ruleset.add( "$comparison_op", "$lte $gte $neq < > =", "or" );
		ruleset.add( "$lte", "< =" );
		ruleset.add( "$gte", "> =" );
		ruleset.add( "$neq", "< >" );
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

		// error messages
		ruleset.ruleerrors["$program"] = "unknown keyword or command";

		// basic formatting rules
		FMT_CULL        = splitstr("$eof $eol PRINT INPUT IF THEN LET GOSUB GOTO END RETURN , ( )");
		FMT_FIRST_CHILD = splitstr("$statement $expression $brackets $atom $add $mul $print_val $print2 $input2");
		FMT_FIRST_VALUE = splitstr("$variable $add_op $mul_op $comparison_op $goto $gosub");

		ruleset.show();
		ruleset.validate();
		return true;
	}

	virtual void formatjson(Json& json) {
		// begin
		assert(json.type == Json::JOBJECT);
		string& type = json.obj["type"].str;

		// format line statements
		if (type == "$line") {
			auto& value = json.obj["value"].arr;
			auto dsym = json.obj["dsym"];
			if (value.size() == 2) {
				auto linenumber = value.at(0).obj.at("value");
				auto var = value.at(1);
				json = var;
				json.obj["dsym"] = dsym;
				json.obj["linenumber"] = linenumber;
			}
			else if (value.size() == 1) {
				auto var = value.at(0);
				json = var;
				json.obj["dsym"] = dsym;
			}
			else if (value.size() == 0)
				json = { Json::JNULL };
		}

		// format let - remove '=' operator
		else if (type == "$let") {
			auto& arr = json.at("value").arr;
			arr.erase(arr.begin() + 1);
		}
		// these keywords don't take arguments
		else if (type == "$end" || type == "$return") {
			assert(json.at("value").size() == 0);
			json.obj.erase("value");
		}

		// expressions
		else if (type == "$integer") {
			json.obj["value"] = { Json::JNUMBER, stod(json.obj["value"].str) };
		}

		// pick out operators
		else if (type == "$add2" || type == "$mul2") {
			auto& value = json.at("value");
			json.obj["operator"] = value.at(0).at("value");
			auto var = value.at(1);
			value = var;
		}
		else if (type == "$lte") {
			json.at("value") = { Json::JSTRING, 0, "<=" };
		}
		else if (type == "$gte") {
			json.at("value") = { Json::JSTRING, 0, ">=" };
		}
		else if (type == "$neq") {
			json.at("value") = { Json::JSTRING, 0, "<>" };
		}
		else if (type == "$comparison") {
			auto& value = json.at("value");
			json.obj["operator"] = value.at(1).at("value");  // pick out comparison operator
			value.arr.erase(value.arr.begin() + 1);
		}
	}
};
