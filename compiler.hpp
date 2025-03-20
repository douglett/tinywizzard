#include "tokenizer.hpp"
#include <exception>
#include <cassert>
using namespace std;


struct Compiler : TokenHelpers {
	stringstream output;

	int compile(const Json& json) {
		assert(json.type == Json::JOBJECT);
		auto& type = json.obj.at("type").str;

		if (json.obj.count("dsym"))
			output << "# dsym: line " << json.obj.at("dsym").num << "\n";

		if (type == "$program") {
			for (auto& line : json.obj.at("value").arr)
				compile(line);
		}
		else if (type == "$end") {
			output << "end\n";
		}
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