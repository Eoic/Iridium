 // Headers and declarations
%{
    #include <stdio.h>
    #include "parser.tab.h"
%}

 // Regular expressions
name            \w([0-9]\w)+
whitespace      [\r\t\s]
linefeed        \n

 // Token specification
%%
{name}          { return NAME; }
"{"             { return LEFT_BRACE; }
"}"             { return RIGHT_BRACE; }
{whitespace}
{linefeed}      ++yylineno
%%
