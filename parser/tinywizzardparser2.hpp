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
		ast.obj["classname"] = { Json::JSTRING, 0, "ass" };
		// ast.obj["static"] = { Json::JBOOL };
		ast.obj["static"] = { Json::JNUMBER, 1 };
		return true;
	}
};