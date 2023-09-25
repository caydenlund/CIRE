%{
    /* definitions */
    #include "parser.h"

%}
num [0-9][0-9]*
id [a-zA-Z][a-zA-Z0-9]*

/* rules */
%%
"INPUTS" { return INPUTS; }
"OUTPUTS" { return OUTPUTS; }
"CONSTRAINTS" { return CONSTRAINTS; }
"EXPRS" { return EXPRS; }
"if" { return IF; }
"then" { return THEN; }
"else" { return ELSE; }
"endif" { return ENDIF; }

"fl16" { return FPTYPE; }
"fl32" { return FPTYPE; }
"fl64" { return FPTYPE; }
"rnd16" { return FPTYPE; }
"rnd32" { return FPTYPE; }
"rnd64" { return FPTYPE; }

{num}  {yylval.num.type = Number::INT;
        yylval.num.ival = atoi(yytext);
        return INT; }
-?([0-9]*\.[0-9]+([eE][-+]?[0-9]+)?) { yylval.num.type = Number::FP;
                                       yylval.num.fval = atof(yytext);
                                       return FP;}

"+" { return ADD; }
"-" { return SUB; }
"*" { return MUL; }
"/" { return DIV; }

"==" { return EQ; }
"!=" { return NEQ; }
"<=" { return LEQ; }
"<" { return LT; }
">=" { return GEQ; }
">" { return GT; }

"&&" { return AND; }
"||" { return OR; }
"!" { return NOT; }

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

{id} { return ID; }

\n+ { return EOL; }
. {}

%%

int yywrap() { return 1; }

