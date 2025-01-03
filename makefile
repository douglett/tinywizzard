OUT = main.out
SRC = main.cpp
HEAD = tokenhelpers.hpp tokenizer.hpp

$(OUT): $(SRC) $(HEAD)
	g++ -std=c++17 -Wall -o $(OUT) $(SRC)

run: $(OUT)
	./$(OUT)