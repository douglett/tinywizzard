#include "tokenizer.hpp"
#include <exception>
#include <cassert>
using namespace std;


struct Compiler : TokenHelpers {
	stringstream header, output;
	vector<string> literals;

	int init() {
		header.str(""), header.clear();
		output.str(""), output.clear();
		literals = {};
		header << "	dim CONTROL TEMP\n";
		header << "	dim A B C D E F G H I J K L M N O P Q R S T U V W X Y Z\n";
		return true;
	}

	int compile(const Json& json) {
		assert(json.type == Json::JOBJECT);
		auto& type = json.at("type").str;

		if (json.count("dsym"))
			output << "# dsym: line " << json.at("dsym").num << "\n";
		if (json.count("linenumber"))
			output << "BASIC_LINE_" << json.at("linenumber").num << ":\n";

		if (type == "$program") {
			for (auto& line : json.at("value").arr)
				compile(line);
		}
		else if (type == "$integer") {
			output << "	push " << json.at("value").num << "\n";
		}
		else if (type == "$end") {
			output << "	end\n";
		}
		else if (type == "$let") {
			compile(json.at("value").at(1));  // handle expression
			output << "	put " << json.at("value").at(0).at("value").str << "\n";  // put in memory
		}
		else if (type == "$print") {
			for (auto& printval : json.at("value").arr) {
				// compile(printval);
				if (printval.at("type").str == "$stringliteral") {
					literals.push_back(printval.at("value").str);
					output << "	prints STRING_LIT_" << (literals.size() - 1) << "\n";
				}
				else if (printval.at("type").str == "$variable")
					output << "	printi " << printval.at("value").str << "\n";
				else
					error(type, "unknown print rule");
			}
		}
		else
			error(type, "unknown rule");

		return true;
	}

	void show() {
		fstream fs("output.asm", ios::out);
		fs << header.str();
		for (size_t i = 0; i < literals.size(); i++)
			fs << "	dim STRING_LIT_" << i << " " << literals[i] << "\n";
		fs << output.str();
	}

	int error(const string& type, const string& msg) {
		// throw runtime_error("compiler error in " + type + ": " + msg);
		cout << "compiler error in " + type + ": " + msg << endl;
		return false;
	}
};