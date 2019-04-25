all:
	${MAKE} lexer
	${MAKE} parser
	g++ -Wall -Werror parser.tab.c lex.yy.c

lexer:
	flex lex.l

parser:
	bison parser.y