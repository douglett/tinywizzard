OUT = main.exe
SRC = main.cpp
CORE = core/core.hpp core/tokenizer.hpp core/json.hpp core/ruleparser.hpp core/runtimebase.hpp core/runtime.hpp
TINYBASIC = tinybasic/tinybasicparser.hpp tinybasic/tinybasiccompiler.hpp
HEAD = $(CORE) $(TINYBASIC)

$(OUT): $(SRC) $(HEAD)
	g++ -std=c++17 -O0 -gdwarf -Wall -o $(OUT) $(SRC)

run: $(OUT)
	./$(OUT)

clean:
	rm -f *.out *.exe
