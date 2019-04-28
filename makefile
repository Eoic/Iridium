all:
	${MAKE} lexer
	${MAKE} parser
	g++ parser.tab.c lex.yy.c -o compiler
lexer:
	flex lex.l

parser:
	bison parser.y