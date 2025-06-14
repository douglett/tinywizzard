#pragma once
#include "semantics.hpp"
using namespace std;

struct TinyWizzardSemantics : Semantics {
	map<string, bool> dims;

	int validate(const Json& json) {
		log(1, "validating file...");
		errcount = 0;
		for (auto& var : json.at("variables").arr)
			pdim(var);
		if (errcount)
			return error("validate", "failed with " + to_string(errcount) + " errors.");
		log(1, "validate successful!");
		return true;
	}

	void pdim(const Json& json) {
		dsym = json.at("dsym").num;
		auto& name = json.at("name").str;
		if (dims.count(name))
			errorc("pdim", "re-definition of '" + name + "'");
		dims[name] = true;
	}
};