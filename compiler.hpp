#include "tokenizer.hpp"
#include <exception>
#include <cassert>
using namespace std;


struct Compiler : TokenHelpers {
	enum INSTRUCTION_TYPE {
		IN_NOOP, IN_DSYM, IN_LABEL,
		IN_DIM, IN_DATA, IN_END, IN_JUMP, IN_PRINTS, IN_PRINTV, IN_INPUT, IN_PUT, IN_GET, IN_PUSH,
		IN_ADD, IN_SUB, IN_MUL, IN_DIV, IN_LT, IN_GT, IN_LTE, IN_GTE,
		IN_JUMPIF, IN_JUMPIFN
	};
	struct Instruction { INSTRUCTION_TYPE type; vector<string> args; int argi; };
	vector<Instruction> inheader, inprogram;
	int ifcount = 0, litcount = 0;

	int init() {
		ifcount = litcount = 0;
		inheader = inprogram = {};
		inheader.push_back({ IN_NOOP, { "# control variables" }});
		inheader.push_back({ IN_DIM,  splitstr("CONTROL TEMP") });
		inheader.push_back({ IN_DIM,  splitstr("A B C D E F G H I J K L M N O P Q R S T U V W X Y Z") });
		inheader.push_back({ IN_NOOP, { "# literals and other data" }});

		return true;
	}

	int compile(const Json& json) {
		assert(json.type == Json::JOBJECT);
		auto& type = json.at("type").str;

		if (json.count("dsym")) {
			inprogram.push_back({ IN_DSYM, {}, int(json.at("dsym").num) });
		}
		if (json.count("linenumber")) {
			inprogram.push_back({ IN_LABEL, { "BASIC_LINE_" + to_string((int)json.at("linenumber").num) } });
		}

		if (type == "$program") {
			for (auto& line : json.at("value").arr)
				compile(line);
		}
		// basic commands
		else if (type == "$end") {
			inprogram.push_back({ IN_END });
		}
		else if (type == "$goto") {
			inprogram.push_back({ IN_JUMP, { "BASIC_LINE_" + to_string((int)json.at("value").num) } });
		}
		// I/O
		else if (type == "$print") {
			for (auto& printval : json.at("value").arr) {
				// print string literal data
				if (printval.at("type").str == "$stringliteral") {
					string strid = "STRING_LIT_" + to_string(++litcount);
					inheader.push_back({ IN_DATA, { strid, printval.at("value").str } });
					inprogram.push_back({ IN_PRINTS, { strid } });
				}
				// print variable as number
				else if (printval.at("type").str == "$variable") {
					inprogram.push_back({ IN_PRINTV, { printval.at("value").str } });
				}
				else
					error(type, "unknown print rule");
			}
		}
		else if (type == "$input") {
			assert(json.at("value").size() == 1);  // check for multiple inputs
			inprogram.push_back({ IN_INPUT, { json.at("value").at(0).at("value").str } });
		}
		// control structures
		else if (type == "$if") {
			string ifid = "BASIC_POSTIF_" + to_string(++ifcount);
			compile(json.at("value").at(0));
			inprogram.push_back({ IN_JUMPIFN, { ifid } });
			compile(json.at("value").at(1));
			inprogram.push_back({ IN_LABEL, { ifid } });
		}
		else if (type == "$comparison") {
			compile(json.at("value").at(0));
			compile(json.at("value").at(1));
			auto op = json.at("operator").str;
			if      (op == "<" )  inprogram.push_back({ IN_LT });
			else if (op == ">" )  inprogram.push_back({ IN_GT });
			else if (op == "<=")  inprogram.push_back({ IN_LTE });
			else if (op == ">=")  inprogram.push_back({ IN_GTE });
			else    error(type, "unknown operator: " + op);
		}
		// expressions
		else if (type == "$let") {
			compile(json.at("value").at(1));  // handle expression
			inprogram.push_back({ IN_PUT, { json.at("value").at(0).at("value").str } });  // put in memory
		}
		else if (type == "$variable") {
			inprogram.push_back({ IN_GET, { json.at("value").str } });  // get from memory
		}
		else if (type == "$integer") {
			inprogram.push_back({ IN_PUSH, {}, int(json.at("value").num) });
		}
		else if (type == "$add" || type == "$mul") {
			auto& value = json.at("value");
			compile(value.at(0));
			for (int i = 1; i < value.size(); i++) {
				compile(value.at(i).at("value"));
				auto op = value.at(i).at("operator").str;
				if      (op == "+")  inprogram.push_back({ IN_ADD });
				else if (op == "-")  inprogram.push_back({ IN_SUB });
				else if (op == "*")  inprogram.push_back({ IN_MUL });
				else if (op == "/")  inprogram.push_back({ IN_DIV });
				else    error(type, "unknown operator: " + op);
			}
		}
		else
			error(type, "unknown rule");

		return true;
	}

	void show() {
		fstream fs("output2.asm", ios::out);
		for (const auto& in : inheader)
			fs << showinstruction(in) << endl;
		for (const auto& in : inprogram)
			fs << showinstruction(in) << endl;
	}

	string showinstruction(const Instruction& in) {
		string s = "\t";
		switch (in.type) {
			// pseudo-instructions
			case IN_NOOP:      s = in.args.size() ? in.args.at(0) : "";  break;
			case IN_DSYM:      s = "# dsym: line " + to_string(in.argi);  break;
			case IN_LABEL:     s = in.args.at(0) + ":";  break;
			// data
			case IN_DIM:
				s += "dim ";
				for (const auto& arg : in.args)
					s += arg + " ";
				break;
			case IN_DATA:
				s += "data ";
				for (const auto& arg : in.args)
					s += arg + " ";
				break;
			// control
			case IN_END:       s += "end";  break;
			case IN_JUMP:      s += "jump " + in.args.at(0);  break;
			case IN_PRINTS:    s += "prints " + in.args.at(0);  break;
			case IN_PRINTV:    s += "printv " + in.args.at(0);  break;
			case IN_INPUT:     s += "input " + in.args.at(0);  break;
			case IN_PUT:       s += "put " + in.args.at(0);  break;
			case IN_GET:       s += "get " + in.args.at(0);  break;
			case IN_PUSH:      s += "push " + to_string(in.argi);  break;
			// maths
			case IN_ADD:       s += "add";  break;
			case IN_SUB:       s += "sub";  break;
			case IN_MUL:       s += "mul";  break;
			case IN_DIV:       s += "div";  break;
			case IN_LT:        s += "compare_lt";  break;
			case IN_GT:        s += "compare_gt";  break;
			case IN_LTE:       s += "compare_lte";  break;
			case IN_GTE:       s += "compare_gte";  break;
			case IN_JUMPIF:    s += "jump_if";  break;
			case IN_JUMPIFN:   s += "jump_ifn";  break;
		}
		return s;
	}

	int error(const string& type, const string& msg) {
		cout << "compiler error in " + type + ": " + msg << endl;
		return false;
	}
};