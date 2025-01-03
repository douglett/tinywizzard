#include "tokenizer.hpp"
using namespace std;


int main() {
	Tokenizer tok;
	tok.flag_eof = 1;
	tok.tokenize("simplebasic.ast");
	tok.show();
}