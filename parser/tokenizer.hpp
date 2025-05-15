#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;


/**
 * Tokenizer base class.
 * Contains generally useful token parsing and string functions.
 */ 
struct TokenHelpers {
	// parsing
	static int isalphanum(char c) {
		return isalnum(c) || c == '_';
	}
	static int isnumber(const string& s) {
		for (auto c : s)
			if (!isdigit(c))  return 0;
		return 1;
	}
	static int isidentifier(const string& s) {
		if (s.length() == 0)  return 0;
		if (!isalpha(s[0]) && s[0] != '_')  return 0;
		for (size_t i = 1; i < s.length(); i++)
			if ( !isalphanum(s[i]) )  return 0;
		return 1;
	}
	static int isliteral(const string& s) {
		return s.length() >= 2 && s.front() == '"' && s.back() == '"';
	}
	static int isarray(const string& s) {
		return s.substr(s.length()-2, 2) == "[]" && isidentifier(s.substr(0, s.length()-2));
	}
	static string basetype(const string& type) {
		return isarray(type) ? type.substr(0, type.length()-2) : type;
	}

	// strings
	static string stripliteral(const string& str) {
		return isliteral(str) ? str.substr(1, str.length()-2) : str;
	}
	static string escapeliteral(const string& str) {
		string s;
		for (auto c : str)
			if      (c == '\n')  s += "\\n";
			else if (c == '"')  s += "\\\"";
			else    s += c;
		return s;
	}
	static vector<string> splitstr(const string str) {
		vector<string> vs;
		stringstream ss(str);
		string s;
		while (ss >> s)
			vs.push_back(s);
		return vs;
	}
	static string joinstr(const vector<string>& vs, const string& glue = " ") {
		string str;
		for (auto& s : vs)
			str += (str.length() ? glue : "") + s;
		return str;
	}
};


/**
 * Tokenizer class
 * Takes a file and processes it into a token stream
 */
struct Tokenizer : public TokenHelpers {
	struct Tok { string str; int lpos; int hpos; };
	const Tok TOK_EOF = { "$EOF", -1, -1 };
	int flag_eol = 0;
	string lcomment = "//";
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
			else if (lcomment.length() && lcomment[0] == c && line.substr(i, lcomment.length()) == lcomment)  break;
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