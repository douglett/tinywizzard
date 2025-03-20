#include "tokenizer.hpp"
#include <exception>
#include <cassert>
using namespace std;


struct Compiler : TokenHelpers {
	stringstream output;
	stringstream data;

	int init() {
		output.str(""), output.clear();
		data.str(""), data.clear();
		output << "	dim CONTROL TEMP\n";
		output << "	dim A B C D E F G H I J K L M N O P Q R S T U V W X Y Z\n";
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
		// else if (type == "$print") {
		// }
		else
			error(type, "unknown rule");

		return true;
	}

	void show() {
		fstream fs("output.asm", ios::out);
		fs << output.str();
	}

	int error(const string& type, const string& msg) {
		// throw runtime_error("compiler error in " + type + ": " + msg);
		cout << "compiler error in " + type + ": " + msg << endl;
		return false;
	}
};