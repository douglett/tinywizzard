#pragma once
#include "tokenizer.hpp"
#include "runtime.hpp"
#include <exception>
#include <cassert>
using namespace std;


struct Compiler : TokenHelpers, RuntimeBase {
	vector<Instruction> program, inheader, inprogram;
	int ifcount = 0, litcount = 0;

	int compile(const Json& json) {
		printf("compiling program...\n");
		initheader();      // initialise program header
		assert(json.at("type").str == "$program");  // make sure first item is a $program
		compileast(json);  // compile program
		// assemble program
		program.insert(program.end(), inheader.begin(), inheader.end());
		program.insert(program.end(), inprogram.begin(), inprogram.end());
		printf("compiled successfully!\n");
		return true;
	}

	int initheader() {
		printf("initialising compiler...\n");
		ifcount = litcount = 0;
		program = inheader = inprogram = {};
		inheader.push_back({ IN_NOOP, { "# control variables" }});
		inheader.push_back({ IN_DIM,  splitstr("CONTROL TEMP") });
		inheader.push_back({ IN_DIM,  splitstr("A B C D E F G H I J K L M N O P Q R S T U V W X Y Z") });
		inheader.push_back({ IN_NOOP, { "# literals and other data" }});
		inheader.push_back({ IN_DATA, { "STRING_LIT_NEWLINE", "\n" } });
		return true;
	}

	int compileast(const Json& json) {
		assert(json.type == Json::JOBJECT);
		auto& type = json.at("type").str;

		if (json.count("dsym")) {
			inprogram.push_back({ IN_DSYM, {}, int(json.at("dsym").num) });
		}
		if (json.count("linenumber")) {
			inprogram.push_back({ IN_LABEL, { "BASIC_LINE_" + to_string((int)json.at("linenumber").num) } });
		}

		// program entry
		if (type == "$program") {
			for (auto& line : json.at("value").arr)
				compileast(line);
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
					inheader.push_back({ IN_DATA, { strid, stripliteral(printval.at("value").str) } });
					inprogram.push_back({ IN_PRINTS, { strid } });
				}
				// print variable as number
				else if (printval.at("type").str == "$variable") {
					inprogram.push_back({ IN_PRINTV, { printval.at("value").str } });
				}
				else
					error(type, "unknown print rule");
			}
			inprogram.push_back({ IN_PRINTS, { "STRING_LIT_NEWLINE" } });  // add newline character
		}
		else if (type == "$input") {
			assert(json.at("value").size() == 1);  // check for multiple inputs
			inprogram.push_back({ IN_INPUT, { json.at("value").at(0).at("value").str } });
		}
		// control structures
		else if (type == "$if") {
			string ifid = "BASIC_POSTIF_" + to_string(++ifcount);
			compileast(json.at("value").at(0));
			inprogram.push_back({ IN_JUMPIFN, { ifid } });
			compileast(json.at("value").at(1));
			inprogram.push_back({ IN_LABEL, { ifid } });
		}
		else if (type == "$comparison") {
			compileast(json.at("value").at(0));
			compileast(json.at("value").at(1));
			auto op = json.at("operator").str;
			if      (op == "<" )  inprogram.push_back({ IN_LT });
			else if (op == ">" )  inprogram.push_back({ IN_GT });
			else if (op == "<=")  inprogram.push_back({ IN_LTE });
			else if (op == ">=")  inprogram.push_back({ IN_GTE });
			else    error(type, "unknown operator: " + op);
		}
		// expressions
		else if (type == "$let") {
			compileast(json.at("value").at(1));  // handle expression
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
			compileast(value.at(0));
			for (int i = 1; i < value.size(); i++) {
				compileast(value.at(i).at("value"));
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

	//  === helpers ===
	void show() {
		fstream fs("output2.asm", ios::out);
		for (const auto& in : inheader)
			fs << showinstruction(in) << endl;
		for (const auto& in : inprogram)
			fs << showinstruction(in) << endl;
	}

	int error(const string& type, const string& msg) {
		cout << "compiler error in " + type + ": " + msg << endl;
		return false;
	}
};