#pragma once
#include "tokenizer.hpp"
#include <vector>
#include <map>
#include <cassert>
using namespace std;


struct Json {
	enum JTYPE { JNULL, JNUMBER, JSTRING, JARRAY, JOBJECT };
	JTYPE type; double num; string str; vector<Json> arr; map<string, Json> obj;

	Json& at(const string& key) { assert(type == JOBJECT);  return obj.at(key); }
	Json& at(size_t key)        { assert(type == JARRAY);   return arr.at(key); }
	const Json& at(const string& key) const { assert(type == JOBJECT);  return obj.at(key); }
	const Json& at(size_t key)        const { assert(type == JARRAY);   return arr.at(key); }
	int count(const string& key) const { return type != JOBJECT ? 0 : obj.count(key); }
	int size() const { return type == JOBJECT ? obj.size() : type == JARRAY ? arr.size() : 0; }
};


// Serialise JSON to ostream	
ostream& operator<<(ostream& os, const Json& json) {
	static const char INDENTCHAR = ' ';
	static string indent;
	// increase indent 
	indent += INDENTCHAR;
	// output type
	switch (json.type) {
		case Json::JNULL:
			os << "NULL" << ",\n";
			break;
		case Json::JSTRING:
			if (TokenHelpers::isliteral(json.str))
				os << json.str << "\n";
			else
				os << '"' << json.str << "\",\n";
			break;
		case Json::JNUMBER:
			os << json.num << ",\n";
			break;
		case Json::JARRAY:
			if (json.arr.size() == 0)
				os << "[],\n";
			else {
				os << "[\n";
				for (auto& js : json.arr) {
					os << indent << INDENTCHAR;
					os << js;
				}
				os << indent << "],\n";
			}
			break;
		case Json::JOBJECT:
			os << "{\n";
			for (auto& pair : json.obj) {
				os << indent << INDENTCHAR << '"' << pair.first << "\": ";
				os << pair.second;
			}
			os << indent << "},\n";
			break;
	}
	// decrease indent and return
	indent.pop_back();
	return os;
}