OUT = main.exe
SRC = main.cpp
PARSER = parser/tokenizer.hpp parser/json.hpp parser/ruleset.hpp parser/astparser.hpp parser/tinywizzardparser.hpp \
	parser/astparser2.hpp parser/tinywizzardparser2.hpp
GENERATOR = generator/generator.hpp generator/tinywizzardgenerator.hpp
RUNTIME = runtime/runtimebase.hpp runtime/runtime.hpp
HEAD = $(PARSER) $(GENERATOR) $(RUNTIME)

$(OUT): $(SRC) $(HEAD)
	g++ -std=c++17 -O0 -gdwarf -Wall -o $(OUT) $(SRC)

run: $(OUT)
	./$(OUT)

clean:
	rm -f *.out *.exe
