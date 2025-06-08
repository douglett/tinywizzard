#pragma once
#include "tokenizer.hpp"
#include "json.hpp"
using namespace std;


struct ASTParser2 : TokenHelpers {
	Tokenizer tok;
	Json ast = { Json::JNULL };
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

	//  === helpers ===
	void show() {
		if (infolevel >= 1)
			printf("outputting program AST to output.json...\n");
		fstream fs("output.json", ios::out);
		// if (ast.arr.size())
		// 	fs << ast.at(0);
		// else
		// 	fs << "(json empty)\n";
		fs << ast;
	}

	int error(const string& rule, const string& msg) {
		throw runtime_error("[ASTParser] " + rule + ": " + msg 
			+ "\n\t\tline-" + to_string(tok.linepos()) 
			+ " @ '" + tok.peek() + "'" );
		return false;
	}
};