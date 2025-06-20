#pragma once
#include <vector>
#include <map>
#include <algorithm>
#include <cassert>
using namespace std;


struct Json {
	enum JTYPE { JNULL, JNUMBER, JBOOLEAN, JSTRING, JARRAY, JOBJECT };
	JTYPE type = JNULL; double num = 0; string str; vector<Json> arr; map<string, Json> obj; vector<string> _order;

	// object info
	int size() const { assert(type == JOBJECT || type == JARRAY);  return type == JOBJECT ? obj.size() : arr.size(); }
	int count(const string& key) const { assert(type == JOBJECT);  return obj.count(key); }
	// get members
	Json& at(const string& key) { return (Json&)(((const Json*)this)->at(key)); }
	Json& at(size_t key)        { return (Json&)(((const Json*)this)->at(key)); }
	const Json& at(const string& key) const {
		assert(type == JOBJECT);
		if (!obj.count(key))  throw out_of_range("Json(Object).at: '" + key + "'");
		return obj.at(key);
	}
	const Json& at(size_t key) const {
		assert(type == JARRAY);
		if (arr.size() < 0 || arr.size() >= key)  throw out_of_range("Json(Array).at: '" + to_string(key) + "'");
		return arr.at(key);
	}
	// mofifiers
	Json&   push(const Json& val)   { assert(type == JARRAY);  arr.push_back(val);  return arr.back(); }
	Json&   set (const string& key) { assert(type == JOBJECT);  return obj[key]; }
	string& sets(const string& key) { assert(type == JOBJECT);  if (obj[key].type != JSTRING) obj[key] = { JSTRING };  return obj[key].str; }
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
				// calculate object print order
				auto order = json._order;
				for (auto& pair : json.obj)
					if (find(order.begin(), order.end(), pair.first) == order.end())
						order.push_back(pair.first);
				// show object
				os << "{\n";
				indent++;
				string indentstr(indent*2, INDENTCHAR);
				size_t i = 0;
				for (auto& key : order) {
					if (!json.obj.count(key))  continue;
					os << indentstr << '"' << key << "\": " << json.obj.at(key);
					os << (++i < json.obj.size() ? "," : "") << "\n";
				}
				indent--;
				os << string(indent*2, INDENTCHAR) << "}";
			}
			break;
	}
	return os;
}