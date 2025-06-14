#pragma once
#include "../parser/json.hpp"
using namespace std;

struct Semantics {
	int infolevel = 1, errcount = 0;

	int validate(const Json& json) {
		return error("validate", "missing validate implementation");
	}

	// === helpers ===
	int log(int level, const string& msg) {
		if (infolevel >= level)
			printf("[Semantics] %s\n", msg.c_str());
		return true;
	}

	int errorc(const string& type, const string& msg) {
		// printf("[Semantics] error in %s: %s (line %d)\n", type.c_str(), msg.c_str(), -1);
		printf("[Semantics] error in %s: %s\n", type.c_str(), msg.c_str());
		errcount++;
		return false;
	}

	int error(const string& type, const string& msg) {
		throw runtime_error("[Semantics] " + type + ": " + msg);
		return false;
	}
};