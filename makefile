all:
	${MAKE} semantics
	${MAKE} parser
	g++ parser.tab.c semantics.yy.c

semantics:
	flex semantics.l

parser:
	bison parser.y