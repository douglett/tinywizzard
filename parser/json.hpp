#pragma once
#include "tokenizer.hpp"
#include <vector>
#include <map>
#include <cassert>
using namespace std;


struct Json {
	enum JTYPE { JNULL, JNUMBER, JBOOLEAN, JSTRING, JARRAY, JOBJECT };
	JTYPE type; double num; string str; vector<Json> arr; map<string, Json> obj;

	Json& at(const string& key) { assert(type == JOBJECT);  return obj.at(key); }
	Json& at(size_t key)        { assert(type == JARRAY);   return arr.at(key); }
	const Json& at(const string& key) const { assert(type == JOBJECT);  return obj.at(key); }
	const Json& at(size_t key)        const { assert(type == JARRAY);   return arr.at(key); }
	Json& push(const Json& obj) { assert(type == JARRAY);  arr.push_back(obj);  return arr.back(); }
	int count(const string& key) const { return type != JOBJECT ? 0 : obj.count(key); }
	int size() const { return type == JOBJECT ? obj.size() : type == JARRAY ? arr.size() : 0; }
};


// Serialise JSON to ostream
ostream& operator<<(ostream& os, const Json& json) {
	static const char INDENTCHAR = ' ';
	static int indent = 0;
	// output type
	switch (json.type) {
		case Json::JNULL:
			os << "NULL";
			break;
		case Json::JSTRING:
			if (TokenHelpers::isliteral(json.str))
				os << json.str;
			else
				os << '"' << json.str << '"';
			break;
		case Json::JNUMBER:
			os << json.num;
			break;
		case Json::JBOOLEAN:
			os << (json.num ? "true" : "false");
			break;
		case Json::JARRAY:
			if (json.arr.size() == 0)
				os << "[]";
			else {
				os << "[\n";
				indent++;
				string indentstr(indent*2, INDENTCHAR);
				size_t i = 0;
				for (auto& js : json.arr) {
					os << indentstr << js;
					os << (++i < json.arr.size() ? "," : "") << "\n";
				}
				indent--;
				os << string(indent*2, INDENTCHAR) << "]";
			}
			break;
		case Json::JOBJECT:
			if (json.obj.size() == 0)
				os << "{}";
			else {
				os << "{\n";
				indent++;
				string indentstr(indent*2, INDENTCHAR);
				size_t i = 0;
				for (auto& pair : json.obj) {
					os << indentstr << '"' << pair.first << "\": " << pair.second;
					os << (++i < json.obj.size() ? "," : "") << "\n";
				}
				indent--;
				os << string(indent*2, INDENTCHAR) << "}";
			}
			break;
	}
	return os;
}