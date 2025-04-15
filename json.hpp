#pragma once
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