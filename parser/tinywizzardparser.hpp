#pragma once
#include "astparser.hpp"
using namespace std;


struct TinyWizzardParser : ASTParser {
	int parse(const string& fname) {
		// loglevel = 4;  // 4 = trace
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
		ast.obj["dims"] = { Json::JARRAY };
		ast.obj["functions"] = { Json::JARRAY };
		ast._order = { "classname", "static", "dims", "functions" };
		// parse header
		require("static class $identifier ;");
		ast.obj["classname"] = { Json::JSTRING, 0, presult.at(2) };
		// class members
		while (!accept("$eof"))
			if      (pfunction(ast.at("functions"))) ;
			else if (pdim(ast.at("dims"))) ;
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
		json.obj["dsym"]      = { Json::JNUMBER, (double)presultline };
		json.obj["arguments"] = { Json::JARRAY };
		json.obj["block"]     = { Json::JARRAY };
		json._order = { "name", "dsym", "arguments", "block" };
		require(")");
		pblock(json.at("block"));
		// function body
		return true;
	}

	int pdim(Json& parent) {
		log(4, "(trace) pdim");
		if (!accept("$identifier $identifier"))
			return false;
		// create json object
		auto& json = parent.push({ Json::JOBJECT });
		json.obj["statement"] = { Json::JSTRING, 0, "dim" };
		json.obj["dsym"]      = { Json::JNUMBER, (double)presultline };
		json.obj["name"]      = { Json::JSTRING, 0, presult.at(1) };
		json.obj["type"]      = { Json::JSTRING, 0, presult.at(0) };
		json._order = { "statement", "dsym", "name", "type", "local", "expression" };
		// assignment
		if (accept("=")) {
			json.obj["expression"] = { Json::JOBJECT };
			pexpression(json.at("expression"));
			// if (!pexpression(json.at("expression")))
			// 	error("syntax-error", "expected expression after '='");
		}
		require(";");
		return true;
	}

	int pblock(Json& block) {
		log(4, "(trace) pblock");
		assert(block.type == Json::JARRAY);
		require("{");
		while (!tok.eof())
			if      (peek("}"))  break;
			else if (passign(block)) ;
			else if (pprint(block)) ;
			else if (pinput(block)) ;
			else if (pif(block)) ;
			else if (pwhile(block)) ;
			else if (pbreak(block)) ;
			else if (preturn(block)) ;
			else if (pdim(block)) ;
			else if (pexpressionline(block)) ;
			else    { error("pblock", "unknown statement");  break; }
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
		json.obj["name"]       = { Json::JSTRING, 0, presult.at(0) };
		json.obj["expression"] = { Json::JOBJECT };
		json._order = { "statement", "dsym", "name", "type", "expression" };
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
		if (pexpression(expr, false)) {
			json.at("printvals").push(expr);
			while (accept(",")) {
				// if (!pexpression(expr))
				// 	error("syntax-error", "expected argument after ','");
				pexpression(expr);
				json.at("printvals").push(expr);
			}
		}
		require(";");
		return true;
	}

	int pinput(Json& parent) {
		log(4, "(trace) pinput");
		if (!accept("input"))
			return false;
		// create json object
		auto& json = parent.push({ Json::JOBJECT });
		json.obj["statement"] = { Json::JSTRING, 0, "input" };
		json.obj["dsym"]      = { Json::JNUMBER, (double)presultline };
		json.obj["variable"]  = { Json::JOBJECT };
		json._order = { "statement", "dsym", "prompt", "variable" };
		if (accept("$strlit")) {
			json.obj["prompt"] = { Json::JSTRING, 0, stripliteral( presult.at(0) ) };
			require(",");
		}
		// get variable
		require("$identifier");
		json.at("variable").obj["expr"]  = { Json::JSTRING, 0, "variable" };
		json.at("variable").obj["value"] = { Json::JSTRING, 0, presult.at(0) };
		require(";");
		return true;
	}

	int pif(Json& parent) {
		log(4, "(trace) pif");
		if (!peek("if"))
			return false;
		// create json object
		auto& json = parent.push({ Json::JOBJECT });
		json.sets("statement")    = "if";
		json.setn("dsym")         = presultline;
		json.set ("conditionals") = { Json::JARRAY };
		json._order = { "statement", "dsym", "conditionals" };
		// parse: if / else-if / else
		pifconditional(json.at("conditionals"), "if");
		while (peek("else if")) 
			pifconditional(json.at("conditionals"), "else if");
		if (peek("else"))
			pifconditional(json.at("conditionals"), "else");
		return true;
	}

	int pifconditional(Json& parent, const string& cond) {
		log(4, "(trace) pifconditional " + cond);
		require(cond);
		// create json object
		auto& json = parent.push({ Json::JOBJECT });
		json.sets("conditional")  = cond;
		json.setn("dsym")         = presultline;
		json.set ("block")        = { Json::JARRAY };
		json._order = { "conditional", "dsym", "expression", "block" };
		// conditional expression
		if (cond != "else") {
			require("(");
			pexpression(json.set("expression"));
			require(")");
		}
		// block
		pblock(json.at("block"));
		return true;
	}

	int pwhile(Json& parent) {
		log(4, "(trace) pwhile");
		if (!accept("while"))
			return false;
		// create json object
		auto& json = parent.push({ Json::JOBJECT });
		json.sets("statement")  = "while";
		json.setn("dsym")       = presultline;
		json.set ("block")      = { Json::JARRAY };
		json._order = { "statement", "dsym", "expression", "block" };
		// conditional expression
		require("(");
		pexpression(json.set("expression"));
		require(")");
		// block
		pblock(json.at("block"));
		return true;
	}

	int pbreak(Json& parent) {
		log(4, "(trace) pbreak");
		if (!accept("break"))
			return false;
		// create json object
		auto& json = parent.push({ Json::JOBJECT });
		json.sets("statement")  = "break";
		json.setn("dsym")       = presultline;
		json._order = { "statement", "dsym", "level" };
		// break level
		if (accept("$number"))
			json.setn("level") = stoi(presult.at(0));
		// OK
		require(";");
		return true;
	}

	int preturn(Json& parent) {
		log(4, "(trace) preturn");
		if (!accept("return"))
			return false;
		// create json object
		auto& json = parent.push({ Json::JOBJECT });
		json.sets("statement")  = "return";
		json.setn("dsym")       = presultline;
		json._order = { "statement", "dsym", "expression" };
		// return expression
		if (!peek(";"))
			pexpression(json.set("expression"));
		// OK
		require(";");
		return true;
	}

	int pexpressionline(Json& parent) {
		log(4, "(trace) pexpressionline");
		Json expr;
		int dsym = tok.pos;
		if (!pexpression(expr, false))
			return false;
		// create json object
		auto& json = parent.push({ Json::JOBJECT });
		json.sets("statement")  = "expression";
		json.setn("dsym")       = dsym;
		json.set ("expression") = expr;
		json._order = { "statement", "dsym", "expression" };
		// OK
		require(";");
		return true;
	}

	// === expressions ===

	int pexpression(Json& json, bool require=true) {
		log(4, "(trace) prexpression");
		if      (pequals(json))  return true;
		else if (require)        return error("syntax-error", "expected expression");
		else                     return false;
	}

	int pequals(Json& json) {
		log(4, "(trace) pequals");
		if (!padd(json))
			return false;
		if (accept("= =") || accept("! =") || accept("< =") || accept("> =") || accept("<") || accept(">")) {
			auto temp = json;
			json = { Json::JOBJECT };
			json.sets("expr")     = "equals";
			json.sets("operator") = joinstr(presult, "");
			json.set ("lhs")      = temp;
			json._order = { "expr", "operator", "lhs", "rhs" };
			padd(json.set("rhs"));
		}
		return true;
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
			json.obj["expr"]  = { Json::JSTRING, 0, "integer" };
			json.obj["value"] = { Json::JNUMBER, (double)stoi(presult.at(0)) };
			return true;
		}
		else if (accept("$identifier (")) {
			json.sets("expr")  = "call";
			json.sets("value") = presult.at(0);
			require(")");
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