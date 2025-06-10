#pragma once
#include "tokenizer.hpp"
#include "json.hpp"
using namespace std;


struct ASTParser2 : TokenHelpers {
	Tokenizer tok;
	Json ast = { Json::JNULL };
	string lasttok;
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
	int accept(const string& rule) {
		assert(rule != "");
		if (rule == "$eof" && tok.eof())
			return lasttok = tok.get(), true;
		else if (rule == "$identifier" && isidentifier(tok.peek()))
			return lasttok = tok.get(), true;
		else if (tok.peek() == rule)
			return lasttok = tok.get(), true;
		return false;
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