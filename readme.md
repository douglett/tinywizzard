WizzardScript parser and runtime, tiny version
==============================================

A simple parser and runtime for WizzardScript programming language.
This WIP tiny version is focused on static classes with no memory management.

- compiler stages:
	1. Lexer (Tokenizer) and Parser (to Json-AST) - these are reusable!
	2. Semantic Analysis - type checking, missing defines, missing functions (TODO?)
	3. Generator - makes WizzardASM
- WizzardASM runtime (custom)

https://www.geeksforgeeks.org/phases-of-a-compiler/


TODO:
-----
- semantics
	- duplicate function names
	- strings in expressions (add / mul)
- expressions in print
	- replace printi/v/s commands?
	- OR
	- special variable for expression results? $TEMP? other?
- local variables
- string types
- float types
