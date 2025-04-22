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
		ruleset.add( "$program", "$line* $eof!" );

		ruleset.validate();
	}
};