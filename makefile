DEPENDENCIES := lex.cpp parser.cpp parser.hpp 
OBJECTS := parser compiler parser.output

all:
	${MAKE} clean
	${MAKE} lexer
	${MAKE} parser
	${MAKE} llvm

lexer:
	flex -o lex.cpp lex.l

parser:
	bison -v -t -d parser.y -o parser.cpp

llvm: 
	g++ parser.cpp lex.cpp generator.cpp main.cpp -std=c++11 -o compiler `llvm-config-7 --cppflags --libs core` 

clean:
	rm -f $(DEPENDENCIES) $(OBJECTS)