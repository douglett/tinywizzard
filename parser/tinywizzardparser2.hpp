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
		// create object
		ast = { Json::JOBJECT };
		ast.obj["static"] = { Json::JBOOLEAN, true };
		ast.obj["variables"] = { Json::JARRAY };
		ast.obj["functions"] = { Json::JARRAY };
		ast._order = { "classname", "static", "variables", "functions" };
		// parse header
		require("static"), require("class"), require("$identifier");
		ast.obj["classname"] = { Json::JSTRING, 0, lasttok };
		require(";");
		// class members
		while (!accept("$eof"))
			if      (pfunction(ast.at("functions"))) ;
			else if (pdim(ast.at("variables"))) ;
			else    break;
		// class end
		require("$eof");
		return true;
	}

	int pfunction(Json& parent) {
		return false;
	}

	int pdim(Json& parent) {
		if (infolevel >= INFO_TRACE)
			printf("[trace] pdim\n");
		int pos = tok.pos;
		if (!accept("int") || !accept("$identifier"))
			return tok.pos = pos, false;
		auto& dim = parent.push({ Json::JOBJECT });
		dim.obj["name"] = { Json::JSTRING, 0, lasttok };
		require(";");
		return true;
	}
};