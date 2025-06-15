#pragma once
#include "astparser.hpp"
using namespace std;


struct TinyWizzardParser : ASTParser {
	int parse(const string& fname) {
		loglevel = 4;  // 4 = trace
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
		ast.obj["static"]    = { Json::JBOOLEAN, true };
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
		json.obj["name"]      = { Json::JSTRING, 0, presult.at(1) };
		json.obj["arguments"] = { Json::JARRAY };
		json.obj["block"]     = { Json::JARRAY };
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
		json.obj["dsym"] = { Json::JNUMBER, (double)presultline };
		json._order = { "name", "dsym", "expression" };
		// assignment
		if (accept("=")) {
			json.obj["expression"] = { Json::JOBJECT };
			if (!pexpression(json.at("expression")))
				error("syntax-error", "expected expression after '='");
		}
		require(";");
		return true;
	}

	int pblock(Json& block) {
		log(4, "(trace) pblock");
		assert(block.type == Json::JARRAY);
		require("{");
		while (!tok.eof())
			if      (passign(block)) ;
			else if (pprint(block)) ;
			else    break;
		require("}");
		return true;
	}

	// === statements ===

	int passign(Json& parent) {
		log(4, "(trace) passign");
		if (!accept("$identifier ="))
			return false;
		// create json object
		auto& json = parent.push({ Json::JOBJECT });
		json.obj["statement"]  = { Json::JSTRING, 0, "assign" };
		json.obj["dsym"]       = { Json::JNUMBER, (double)presultline };
		json.obj["variable"]   = { Json::JSTRING, 0, presult.at(0) };
		json.obj["expression"] = { Json::JOBJECT };
		json._order = { "statement", "dsym", "variable", "expression" };
		// parse expression
		pexpression(json.at("expression"));
		require(";");
		return true;
	}

	int pprint(Json& parent) {
		log(4, "(trace) pprint");
		if (!accept("print"))
			return false;
		// create json object
		auto& json = parent.push({ Json::JOBJECT });
		json.obj["statement"] = { Json::JSTRING, 0, "print" };
		json.obj["dsym"]      = { Json::JNUMBER, (double)presultline };
		json.obj["printvals"] = { Json::JARRAY };
		json._order = { "statement", "dsym", "printvals" };
		// parse expressions
		Json expr = { Json::JOBJECT };
		if (pexpression(expr))
			json.at("printvals").push(expr);
		while (accept(",")) {
			if (!pexpression(expr))
				error("syntax-error", "expected argument after ','");
			json.at("printvals").push(expr);
		}
		require(";");
		return true;
	}

	// === expressions ===

	int pexpression(Json& json, bool require=false) {
		log(4, "(trace) prexpression");
		if      (padd(json))  return true;
		else if (require)     return error("syntax-error", "expected expression");
		else                  return false;
	}

	int padd(Json& json) {
		log(4, "(trace) padd");
		if (!pmul(json))
			return false;
		while (accept("+") || accept("-")) {
			auto temp = json;
			json = { Json::JOBJECT };
			json.obj["expr"]     = { Json::JSTRING, 0, "add" };
			json.obj["operator"] = { Json::JSTRING, 0, presult.at(0) };
			// json.obj["expr"]     = { Json::JSTRING, 0, presult.at(0) == "+" ? "add" : "sub" };
			json.obj["lhs"]      = temp;
			json._order = { "expr", "operator", "lhs", "rhs" };
			pmul(json.obj["rhs"]);
		}
		return true;
	}

	int pmul(Json& json) {
		log(4, "(trace) pmul");
		if (!patom(json))
			return false;
		while (accept("*") || accept("/")) {
			auto temp = json;
			json = { Json::JOBJECT };
			json.obj["expr"]     = { Json::JSTRING, 0, "mul" };
			json.obj["operator"] = { Json::JSTRING, 0, presult.at(0) };
			json.obj["lhs"]      = temp;
			json._order = { "expr", "operator", "lhs", "rhs" };
			patom(json.obj["rhs"]);
		}
		return true;
	}

	int patom(Json& json) {
		log(4, "(trace) patom");
		// log(4, tok.peek());
		json = { Json::JOBJECT };
		json._order = { "expr" };
		if (accept("$number")) {
			json.obj["expr"]  = { Json::JSTRING, 0, "number" };
			json.obj["value"] = { Json::JNUMBER, (double)stoi(presult.at(0)) };
			// json = { Json::JNUMBER, (double)stoi(presult.at(0)) };
			return true;
		}
		else if (accept("$identifier")) {
			json.obj["expr"]  = { Json::JSTRING, 0, "variable" };
			json.obj["value"] = { Json::JSTRING, 0, presult.at(0) };
			return true;
		}
		else if (accept("$strlit")) {
			json.obj["expr"]  = { Json::JSTRING, 0, "strlit" };
			json.obj["value"] = { Json::JSTRING, 0, stripliteral( presult.at(0) ) };
			return true;
		}
		return false;
	}
};