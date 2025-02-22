OUT = main.out
SRC = main.cpp
HEAD = tokenizer.hpp ruleparser.hpp

$(OUT): $(SRC) $(HEAD)
	g++ -std=c++17 -Wall -o $(OUT) $(SRC)

run: $(OUT)
	./$(OUT)