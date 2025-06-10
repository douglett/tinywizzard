#pragma once
#include "astparser2.hpp"
using namespace std;


struct TinyWizzardParser : ASTParser2 {
	const int INFO_TRACE = 4;

	int parse(const string& fname) {
		infolevel = INFO_TRACE;
		tokenize(fname);
		// parse program
		log(1, "syntax parsing...");
		pclass();
		// show success
		show();
		log(1, "file parsed successfully!");
		// ok
		return true;
	}

	int pclass() {
		log(INFO_TRACE, "(trace) pclass");
		// create object
		ast = { Json::JOBJECT };
		ast.obj["static"] = { Json::JBOOLEAN, true };
		ast.obj["variables"] = { Json::JARRAY };
		ast.obj["functions"] = { Json::JARRAY };
		ast._order = { "classname", "static", "variables", "functions" };
		// parse header
		require("static class $identifier ;");
		ast.obj["classname"] = { Json::JSTRING, 0, presult.at(2) };
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
		log(INFO_TRACE, "(trace) pfunction");
		// entry
		if (!accept("int $identifier ("))
			return false;
		// create json object
		auto& json = parent.push({ Json::JOBJECT });
		json.obj["name"] = { Json::JSTRING, 0, presult.at(1) };
		json.obj["arguments"] = { Json::JARRAY };
		json.obj["block"] = { Json::JARRAY };
		json._order = { "name", "arguments", "block" };
		require(") {");
		require("}");
		// function body
		return true;
	}

	int pdim(Json& parent) {
		log(INFO_TRACE, "(trace) pdim");
		// entry
		if (!accept("int $identifier"))
			return false;
		// create json object
		auto& json = parent.push({ Json::JOBJECT });
		json.obj["name"] = { Json::JSTRING, 0, presult.at(1) };
		require(";");
		return true;
	}
};