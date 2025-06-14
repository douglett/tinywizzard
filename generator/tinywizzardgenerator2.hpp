#pragma once
#include "generator.hpp"
// #include <vector>
// #include <cassert>
using namespace std;


struct TinyWizzardGenerator : Generator {
	int generate(const Json& json) {
		log(1, "generating single-class program...");
		reset();
		// compile
		// compileclass(json);
		// error
		if (errcount)
			error("Generator", "compile failed with " + to_string(errcount) + " errors.");
		// success
		// outputfunctions();
		show();
		log(1, "compiled successfully!");
		return true;
	}

	void reset() {
		Generator::reset();
	}
};