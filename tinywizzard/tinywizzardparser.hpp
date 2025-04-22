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
		ruleset.add( "$program", "$eof!" );

		// error messages
		// ruleset.ruleerrors["$program"] = "";

		// basic formatting rules
		// FMT_CULL        = splitstr("");
		// FMT_FIRST_CHILD = splitstr("");
		// FMT_FIRST_VALUE = splitstr("");

		ruleset.show();
		ruleset.validate(true);
	}

	virtual void formatjson(Json& json) {
		// begin
		assert(json.type == Json::JOBJECT);
		// string& type = json.at("type").str;
	}
};