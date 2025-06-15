OUT = main.exe
SRC = main.cpp
PARSER    = parser/tokenizer.hpp parser/json.hpp parser/astparser.hpp parser/tinywizzardparser.hpp
SEMANTICS = semantics/semantics.hpp semantics/tinywizzardsemantics.hpp
GENERATOR = generator/generator.hpp generator/tinywizzardgenerator.hpp
RUNTIME   = runtime/runtimebase.hpp runtime/runtime.hpp
HEAD = $(PARSER) $(SEMANTICS) $(GENERATOR) $(RUNTIME)

$(OUT): $(SRC) $(HEAD)
	g++ -std=c++17 -O0 -gdwarf -Wall -o $(OUT) $(SRC)

run: $(OUT)
	./$(OUT)

clean:
	rm -f *.out *.exe
