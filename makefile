OUT = main.exe
SRC = main.cpp
# CORE = core/core.hpp core/tokenizer.hpp core/json.hpp core/ruleset.hpp core/astparser.hpp core/compiler.hpp core/runtimebase.hpp core/runtime.hpp
# TINYWIZZARD = tinywizzard/tinywizzardparser.hpp tinywizzard/tinywizzardcompiler.hpp
PARSER = parser/tokenizer.hpp parser/json.hpp parser/ruleset.hpp parser/astparser.hpp parser/tinywizzardparser.hpp
HEAD = $(PARSER)

$(OUT): $(SRC) $(HEAD)
	g++ -std=c++17 -O0 -gdwarf -Wall -o $(OUT) $(SRC)

run: $(OUT)
	./$(OUT)

clean:
	rm -f *.out *.exe
