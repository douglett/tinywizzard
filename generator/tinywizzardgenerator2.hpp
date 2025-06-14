#pragma once
#include "generator.hpp"
// #include <vector>
// #include <cassert>
using namespace std;


struct TinyWizzardGenerator : Generator {
	struct Func { string name; vector<Instruction> ilist; };
	map<string, Func> functions;

	int generate(const Json& json) {
		loglevel = 4;  // 4 = trace
		log(1, "generating single-class program...");
		reset();
		// compile
		pclass(json);
		// success or fail
		if (errcount)
			error("Generator", "compile failed with " + to_string(errcount) + " errors.");
		// outputfunctions();
		show();
		log(1, "compiled successfully!");
		return true;
	}

	void reset() {
		Generator::reset();
		functions["STATIC_INIT"] = { "STATIC_INIT" };
	}

	// === Generate class ===

	void pclass(const Json& json) {
		auto& classname = json.at("classname").str;
		log(1, "compiling class: " + classname);
		for (auto& dim : json.at("variables").arr)
			pdim(dim);
	}

	void pdim(const Json& json) {
		log(4, "(trace) pdim");
		auto& name = json.at("name").str;
		dsym = json.at("dsym").num;
		auto& ilist = functions.at("STATIC_INIT").ilist;
		ilist.push_back({ IN_DSYM, {}, dsym });
		ilist.push_back({ IN_DIM, { name } });
		if (json.count("expression"))
			pexpression(json.at("expression"));
	}

	// === Generate expressions ===

	void pexpression(const Json& json) {
		log(4, "(trace) pexpression");

	}
};