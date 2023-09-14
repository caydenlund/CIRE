%{
    /* definitions */
    #include "parser.h"

%}
num [1-9][0-9]*
id [a-zA-Z][a-zA-Z0-9]*

/* rules */
%%
"INPUTS" { return INPUTS; }
"OUTPUTS" { return OUTPUTS; }
"EXPRS" { return EXPRS; }

{num}  { yylval.num.type = Number::INTEGER;
        yylval.num.ival = atoi(yytext);
        return INTEGER; }
-?([0-9]*\.[0-9]+([eE][-+]?[0-9]+)?) { yylval.num.type = Number::FPTYPE;
                                        yylval.num.fval = atof(yytext);
                                        return FPTYPE;}
{id} { return ID; }


"+" { return ADD; }
"-" { return SUB; }
"*" { return MUL; }
"/" { return DIV; }

"-" { return MINUS; }

"sin" { return SIN; }
"asin" { return ASIN; }
"cos" { return COS; }
"tan" { return TAN; }
"cot" { return COT; }
"sinh" { return SINH; }
"cosh" { return COSH; }

"sqrt" { return SQRT; }
"exp" { return EXP; }
"log" { return LOG; }

"{" {return LBRACE; }
"}" {return RBRACE; }
"(" {return LPAREN; }
")" {return RPAREN; }
"[" {return LBRACKET; }
"]" {return RBRACKET; }
"," {return COMMA; }
";" {return SEMICOLON; }
":" {return COLON; }
"=" {return ASSIGN; }

\n+ { return EOL; }
. {}

%%

int yywrap() { return 0; }

