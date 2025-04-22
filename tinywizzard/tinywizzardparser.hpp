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
		ruleset.add( "$classdef",         "static class $name ;" );
		ruleset.add( "$function",         "$typeid $name ( )! $block!" );
		ruleset.add( "$dim",              "$typeid $name ;!" );
		ruleset.add( "$name",             "$identifier" );
		ruleset.add( "$typeid",           "int" );
		ruleset.add( "$block",            "{ $line* }!" );
		ruleset.add( "$line",             "$dsym $statement ;!" );
		ruleset.add( "$statement",        "$print", "or" );
		ruleset.add( "$print",            "print $integer! $printval*" );
		ruleset.add( "$printval",         ", $integer!" );

		// error messages
		// ruleset.ruleerrors["$program"] = "";
		ruleset.ruleerrors["$function"] = "error in function definition";
		ruleset.ruleerrors["$block"]    = "unknown statement in block";

		// basic formatting rules: FIRST_CHILD (replace with first-child), FIRST_VALUE (replace value with value first-child)
		FMT_CULL        = splitstr("$eof class print ; , ( ) { }");
		FMT_FIRST_CHILD = splitstr("$classmember $line $statement $printval");
		FMT_FIRST_VALUE = splitstr("$name $typeid");

		ruleset.show();
		ruleset.validate(true);
	}

	virtual void formatjson(Json& json) {
		// begin
		assert(json.type == Json::JOBJECT);
		// string& type = json.at("type").str;
	}
};