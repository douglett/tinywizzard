#pragma once
#include "astparser2.hpp"
using namespace std;


struct TinyWizzardParser : ASTParser2 {
	int parse(const string& fname) {
		infolevel = 4;  // 4 = trace
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
		log(4, "(trace) pclass");
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
		log(4, "(trace) pfunction");
		if (!accept("int $identifier ("))
			return false;
		// create json object
		auto& json = parent.push({ Json::JOBJECT });
		json.obj["name"] = { Json::JSTRING, 0, presult.at(1) };
		json.obj["arguments"] = { Json::JARRAY };
		json.obj["block"] = { Json::JARRAY };
		json._order = { "name", "arguments", "block" };
		require(")");
		pblock(json.at("block"));
		// function body
		return true;
	}

	int pdim(Json& parent) {
		log(4, "(trace) pdim");
		if (!accept("int $identifier"))
			return false;
		// create json object
		auto& json = parent.push({ Json::JOBJECT });
		json.obj["name"] = { Json::JSTRING, 0, presult.at(1) };
		require(";");
		return true;
	}

	int pblock(Json& block) {
		log(4, "(trace) pblock");
		assert(block.type == Json::JARRAY);
		require("{");
		while (!tok.eof())
			if    (passign(block)) ;
			else  break;
		require("}");
		return true;
	}

	int passign(Json& parent) {
		log(4, "(trace) passign");
		if (!accept("$identifier ="))
			return false;
		// create json object
		auto& json = parent.push({ Json::JOBJECT });
		json.obj["statement"] = { Json::JSTRING, 0, "assign" };
		json.obj["varpath"] = { Json::JSTRING, 0, presult.at(0) };
		json.obj["expression"] = { Json::JOBJECT };
		json._order = { "statement", "varpath", "expression" };
		// parse expression
		pexpression(json.at("expression"));
		require(";");
		return false;
	}

	int pexpression(Json& json) {
		require("$number");
		json.obj["expr"] = { Json::JSTRING, 0, "number" };
		json.obj["value"] = { Json::JNUMBER, (double)stoi(presult.at(0)) };
		return true;
	}
};