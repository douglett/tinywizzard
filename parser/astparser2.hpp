#pragma once
#include "tokenizer.hpp"
#include "json.hpp"
using namespace std;


struct ASTParser2 : TokenHelpers {
	Tokenizer tok;
	Json ast = { Json::JNULL };
	vector<string> presult;
	int infolevel = 1;

	int parse(const string& fname) {
		return error("parse", "missing parse implementation");
	}

	int tokenize(const string& fname) {
		if (infolevel >= 1)
			printf("-----\n"),
			printf("loading file to token stream: %s\n", fname.c_str());
		if ( !tok.tokenize(fname) )
			error("parse", tok.errormsg);
		if (infolevel >= 2)
			tok.show();
		return true;
	}

	// === parsing primitives ===
	int accept(const string& rulestr) {
		auto rulelist = splitstr(rulestr);
		assert(rulelist.size() > 0);
		presult = {};
		int pos = tok.pos;
		for (const auto& rule : rulelist)
			if (rule == "$eof" && tok.eof())
				presult.push_back(tok.get());
			else if (rule == "$identifier" && isidentifier(tok.peek()))
				presult.push_back(tok.get());
			else if (tok.peek() == rule)
				presult.push_back(tok.get());
			else
				return tok.pos = pos, false;
		return true;
	}

	int require(const string& rule) {
		if (accept(rule))
			return true;
		return error("syntax-error", "expected '" + rule + "'");
	}

	// === helpers ===
	void show() {
		if (infolevel >= 1)
			printf("outputting program AST to output.json...\n");
		fstream fs("output.json", ios::out);
		fs << ast;
	}

	int error(const string& type, const string& msg) {
		throw runtime_error("[ASTParser] " + type + ": " + msg 
			+ "\n\t\tline-" + to_string(tok.linepos()) 
			+ " @ '" + tok.peek() + "'" );
		return false;
	}
};