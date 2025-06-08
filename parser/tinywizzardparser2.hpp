#pragma once
#include "astparser2.hpp"
using namespace std;


struct TinyWizzardParser : ASTParser2 {
	const int INFO_TRACE = 4;

	int parse(const string& fname) {
		infolevel = INFO_TRACE;
		tokenize(fname);
		// parse program
		if (infolevel >= 1)
			printf("syntax parsing...\n");
		pclass();
		// show success
		show();
		if (infolevel >= 1)
			printf("file parsed successfully!\n");
		// ok
		return true;
	}

	int pclass() {
		if (infolevel >= INFO_TRACE)
			printf("[trace] pclass\n");
		ast = { Json::JOBJECT };
		ast.obj["static"] = { Json::JBOOLEAN, true };

		// require("static"), require("class"), require("$identifier"), require(";");
		require("static"), require("class"), require("$identifier");
		ast.obj["classname"] = { Json::JSTRING, 0, lasttok };
		require(";");
		require("$eof");

		return true;
	}
};