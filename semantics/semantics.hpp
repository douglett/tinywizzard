#pragma once
#include "../parser/json.hpp"
using namespace std;

struct Semantics {
	int loglevel = 1, errcount = 0, dsym = -1;

	int validate(const Json& json) {
		return error("validate", "missing validate implementation");
	}

	void reset() {
		errcount = 0, dsym = -1;
	}

	// === helpers ===
	int log(int level, const string& msg) {
		if (loglevel >= level)
			printf("[Semantics] %s\n", msg.c_str());
		return true;
	}

	int errorc(const string& type, const string& msg) {
		printf("[Semantics] error in %s: %s (line %d)\n", type.c_str(), msg.c_str(), dsym);
		errcount++;
		return false;
	}

	int error(const string& type, const string& msg) {
		throw runtime_error("[Semantics] " + type + ": " + msg);
		return false;
	}
};