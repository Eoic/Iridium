all:
	${MAKE} lexer
	${MAKE} parser
	${MAKE} llvm
lexer:
	flex -o lex.cpp lex.l

parser:
	bison -d -o parser.cpp parser.y

llvm: 
	g++ -o parser parser.cpp lex.cpp main.cpp -std=c++11 `llvm-config-7 --cppflags`