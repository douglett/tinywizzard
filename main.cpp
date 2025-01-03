#include "tokenizer.hpp"
using namespace std;


int main() {
	Tokenizer tok;
	tok.tokenize("simplebasic.ast");
	tok.show();
}