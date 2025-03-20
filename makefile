OUT = main.exe
SRC = main.cpp
HEAD = tokenizer.hpp ruleparser.hpp compiler.hpp

$(OUT): $(SRC) $(HEAD)
	g++ -std=c++17 -O0 -gdwarf -Wall -o $(OUT) $(SRC)

run: $(OUT)
	./$(OUT)

clean:
	rm -f *.out *.exe
