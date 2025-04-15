#include "tokenizer.hpp"
#include <exception>
#include <cassert>
using namespace std;


struct Compiler : TokenHelpers {
	enum INSTRUCTION_TYPE { IN_NOOP, IN_DSYM, IN_LABEL, IN_DIM, IN_DATA, IN_END, IN_JUMP, IN_PRINTS, IN_PRINTV, IN_INPUT };
	struct Instruction { INSTRUCTION_TYPE type; vector<string> args; };
	vector<Instruction> inheader, inprogram;

	stringstream header, output;
	vector<string> literals;
	int ifcount = 0, litcount = 0;

	int init() {
		// header.str(""), header.clear();
		// output.str(""), output.clear();
		// literals = {};
		// ifcount = 0;
		// header << "# control variables\n";
		// header << "	dim CONTROL TEMP\n";
		// header << "	dim A B C D E F G H I J K L M N O P Q R S T U V W X Y Z\n";

		ifcount = litcount = 0;
		inheader = inprogram = {};
		inheader.push_back({ IN_NOOP, { "# control variables" }});
		inheader.push_back({ IN_DIM,  splitstr("CONTROL TEMP") });
		inheader.push_back({ IN_DIM,  splitstr("A B C D E F G H I J K L M N O P Q R S T U V W X Y Z") });

		return true;
	}

	int compile(const Json& json) {
		assert(json.type == Json::JOBJECT);
		auto& type = json.at("type").str;

		if (json.count("dsym")) {
			// output << "# dsym: line " << json.at("dsym").num << "\n";
			inprogram.push_back({ IN_DSYM, { to_string((int)json.at("dsym").num) } });
		}
		if (json.count("linenumber")) {
			// output << "BASIC_LINE_" << json.at("linenumber").num << ":\n";
			inprogram.push_back({ IN_LABEL, { "BASIC_LINE_" + to_string((int)json.at("linenumber").num) } });
		}

		if (type == "$program") {
			for (auto& line : json.at("value").arr)
				compile(line);
		}
		// basic commands
		else if (type == "$end") {
			// output << "	end\n";
			inprogram.push_back({ IN_END });
		}
		else if (type == "$goto") {
			// output << "	goto BASIC_LINE_" << json.at("value").num << "\n";
			inprogram.push_back({ IN_JUMP, { "BASIC_LINE_" + to_string((int)json.at("value").num) } });
		}
		// I/O
		else if (type == "$print") {
			for (auto& printval : json.at("value").arr) {
				if (printval.at("type").str == "$stringliteral") {
					// literals.push_back(printval.at("value").str);
					// output << "	prints STRING_LIT_" << (literals.size() - 1) << "\n";
					string strid = "STRING_LIT_" + to_string(++litcount);
					inheader.push_back({ IN_DATA, { strid } });
					inprogram.push_back({ IN_PRINTS, { strid } });
				}
				else if (printval.at("type").str == "$variable") {
					// output << "	printi " << printval.at("value").str << "\n";
					inprogram.push_back({ IN_PRINTV, { printval.at("value").str } });
				}
				else
					error(type, "unknown print rule");
			}
		}
		else if (type == "$input") {
			assert(json.at("value").size() == 1);  // check for multiple inputs
			// output << "	input " << json.at("value").at(0).at("value").str << "\n";
			inprogram.push_back({ IN_INPUT, { json.at("value").at(0).at("value").str } });
		}
		// control structures
		else if (type == "$if") {
			int ifid = ++ifcount;
			compile(json.at("value").at(0));
			output << "	goto_ifn BASIC_POSTIF_" << ifid << "\n";
			compile(json.at("value").at(1));
			output << "BASIC_POSTIF_" << ifid << ":\n";
		}
		else if (type == "$comparison") {
			compile(json.at("value").at(0));
			compile(json.at("value").at(1));
			auto op = json.at("operator").str;
			if      (op == "<" )  output << "	compare_lt\n";
			else if (op == ">" )  output << "	compare_gt\n";
			else if (op == "<=")  output << "	compare_lte\n";
			else if (op == ">=")  output << "	compare_gte\n";
			else    error(type, "unknown operator: " + op);
		}
		// expressions
		else if (type == "$let") {
			compile(json.at("value").at(1));  // handle expression
			output << "	put " << json.at("value").at(0).at("value").str << "\n";  // put in memory
		}
		else if (type == "$integer") {
			output << "	push " << json.at("value").num << "\n";
		}
		else if (type == "$variable") {
			output << "	get " << json.at("value").str << "\n";  // get from memory
		}
		else if (type == "$add" || type == "$mul") {
			auto& value = json.at("value");
			compile(value.at(0));
			for (int i = 1; i < value.size(); i++) {
				compile(value.at(i).at("value"));
				auto op = value.at(i).at("operator").str;
				if      (op == "+")  output << "	add\n";
				else if (op == "-")  output << "	sub\n";
				else if (op == "*")  output << "	mul\n";
				else if (op == "/")  output << "	div\n";
				else    error(type, "unknown operator: " + op);
			}
		}
		else
			error(type, "unknown rule");

		return true;
	}

	// void show() {
	// 	fstream fs("output.asm", ios::out);
	// 	fs << header.str();
	// 	fs << "# define literals\n";
	// 	for (size_t i = 0; i < literals.size(); i++)
	// 		fs << "	data STRING_LIT_" << i << " " << literals[i] << "\n";
	// 	fs << output.str();
	// }

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
			case IN_NOOP:
				s = in.args.size() ? in.args.at(0) : "";
				break;
			case IN_DSYM:
				s = "# dsym: line " + in.args.at(0);
				break;
			case IN_LABEL:
				s = in.args.at(0) + ":";
				break;
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
			case IN_END:
				s += "end";
				break;
			case IN_JUMP:
				s += "jump " + in.args.at(0);
				break;
			case IN_PRINTS:
				s += "prints " + in.args.at(0);
				break;
			case IN_PRINTV:
				s += "printv " + in.args.at(0);
				break;
			case IN_INPUT:
				s += "input " + in.args.at(0);
		}
		return s;
	}

	int error(const string& type, const string& msg) {
		// throw runtime_error("compiler error in " + type + ": " + msg);
		cout << "compiler error in " + type + ": " + msg << endl;
		return false;
	}
};