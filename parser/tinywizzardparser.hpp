#pragma once
#include "../core/core.hpp"
using namespace std;


/**
 * Parses TinyWizzard language file to json AST
 * Very small version if WizzardScript:
 *   - single file only
 *   - integers only
 */
struct TinyWizzardParser : ASTParser {
	void init() {
		// tokenizer settings
		tok.lcomment = "//";
		tok.flag_eol = false;

		// initialise ruleset
		ruleset.name = "TinyWizzard";
		ruleset.add( "$program",          "$classdef! $classmember* $eof!" );
		ruleset.add( "$classmember",      "$function $dim", "or" );
		ruleset.add( "$classdef",         "$dsym static class $name ;" );
		ruleset.add( "$function",         "$dsym $typeid $name ( )! $block!" );
		ruleset.add( "$dim",              "$dsym $typeid $name $dim2? ;!" );
		ruleset.add( "$dim2",             "= $expr!" );
		ruleset.add( "$name",             "$identifier" );
		ruleset.add( "$typeid",           "int" );
		ruleset.add( "$block",            "{ $line* }!" );
		ruleset.add( "$line",             "$dsym $statement ;!" );
		// statements
		ruleset.add( "$statement",        "$print $assign", "or" );
		ruleset.add( "$print",            "print $value! $print2*" );
		ruleset.add( "$print2",           ", $value!" );
		ruleset.add( "$assign",           "$variable =! $expr!" );
		// expressions
		ruleset.add( "$value",            "$integer $variable $stringliteral", "or" );
		ruleset.add( "$variable",         "$identifier" );
		ruleset.add( "$expr",             "$add" );
		ruleset.add( "$add",              "$mul $add2?" );
		ruleset.add( "$add2",             "$opplus $mul!" );
		ruleset.add( "$mul",              "$value $mul2?" );
		ruleset.add( "$mul2",             "$opmultiply $value!" );

		// error messages
		// ruleset.ruleerrors["$program"] = "";
		ruleset.ruleerrors["$function"] = "error in function definition";
		ruleset.ruleerrors["$block"]    = "unknown statement in block";

		// basic formatting rules: FIRST_CHILD (replace with first-child), FIRST_VALUE (replace value with value first-child)
		FMT_CULL        = splitstr("$eof class print ; , ( ) { }");
		FMT_FIRST_CHILD = splitstr("$classmember $line $statement $print2 $value $expr");
		FMT_FIRST_VALUE = splitstr("$name $typeid $variable");

		ruleset.show();
		ruleset.validate(true);
	}

	virtual void formatjson(Json& json) {
		// begin
		assert(json.type == Json::JOBJECT);
		string& type = json.at("type").str;

		if (type == "$assign") {
			auto& arr = json.at("value").arr;
			arr.erase(arr.begin() + 1);  // erase '=' punctuation
		}
		else if (type == "$dim2") {
			auto val = json.at("value").at(1);
			json = val;
		}
		else if (type == "$add" || type == "$mul") {
			// single value - just replace with value
			if (json.at("value").size() == 1) {
				auto val = json.at("value").at(0);
				json = val;
			}
			// 2 values - multiply values in order
			else {
				auto& add2 = json.at("value").at(1);
				auto op    = add2.at("value").at(0).at("value");
				auto val   = add2.at("value").at(1);
				add2 = val;
				json.obj["operator"] = op;
			}
		}
	}
};