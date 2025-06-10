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
		log(1, "loading file to token stream: " + fname);
		if ( !tok.tokenize(fname) )
			error("parse", tok.errormsg);
		log(2, tok.showstr());
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
		log(1, "outputting program AST to output.json...");
		fstream fs("output.json", ios::out);
		fs << ast;
	}

	int log(int level, const string& msg) {
		if (infolevel >= level)
			printf("[Parser] %s\n", msg.c_str());
		return true;
	}

	int error(const string& type, const string& msg) {
		throw runtime_error("[Parser] " + type + ": " + msg 
			+ "\n\t\tline-" + to_string(tok.linepos()) 
			+ " @ '" + tok.peek() + "'" );
		return false;
	}
};