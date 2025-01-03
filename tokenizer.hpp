#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include "tokenhelpers.hpp"
using namespace std;

struct Tokenizer : public TokenHelpers {
	struct Tok { string str; int lpos; int hpos; };
	const Tok TOK_EOF = { "$EOF", -1, -1 };
	int flag_eol = 0;
	vector<Tok> tok;
	string errormsg;
	int pos = 0, plinepos = 0;

	int tokenize(const string& fname) {
		fstream fs(fname, ios::in);
		if (!fs.is_open())
			return error("error: opening file: " + fname);
		// setup
		reset();
		string line;
		// parse line-by-line
		while (getline(fs, line))
			if (!tokenizeline(line))
				return false;
		// ok
		return true;
	}

	int tokenizeline(const string& line) {
		plinepos++;
		int hpos = 0;
		string t;
		#define addtok() ( t.length() ? tok.push_back({ t, plinepos, hpos }), t = "", 1 : 0 )
		// parse line
		for (size_t i = 0; i < line.length(); i++) {
			char c = line[i];
			hpos++;
			// spaces
			if (isspace(c))  addtok();
			// line comments (exit and ignore)
			else if (c == '/' && line.substr(i, 2) == "//")  break;
			// string
			else if (c == '"') {
				addtok(), t = c;
				for (i++; i < line.length() && line[i] != '"'; i++)
					t += line[i];
				t += '"';
				if (i >= line.length())
					return error("error: unterminated string, line " + to_string(plinepos));
			}
			// special characters
			else if (!isalphanum(c))  addtok(), t += c, addtok();
			// normal characters
			else  t += c;  
		}
		// final token (if necessary)
		addtok();
		// add EOF token, if required
		if (flag_eol)  t = "$EOL", addtok();
		return true;
	}

	int reset() {
		tok = {}, errormsg = "", pos = plinepos = 0;
		return 0;
	}

	// helpers
	int error(const string& msg) {
		// fprintf(stderr, "%s\n", msg.c_str());
		errormsg = msg;
		return false;
	}
	void show() {
		cout << "tokens: ";
		for (const auto& t : tok)
			cout << t.str << " ";
		cout << endl;
	}

	// parsing
	int eof() {
		return pos < 0 || pos >= (int)tok.size();
	}
	int linepos() {
		return eof() ? TOK_EOF.lpos : tok[pos].lpos;
	}
	const string& peek() {
		return eof() ? TOK_EOF.str : tok[pos].str;
	}
	const string& get() {
		return eof() ? TOK_EOF.str : tok[pos++].str;
	}
};