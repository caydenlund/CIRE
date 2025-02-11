%{
    // Includes for the forward declarations
    /* definitions */
    #include "../lib/parser.h"
%}
num [0-9][0-9]*
id [a-zA-Z][a-zA-Z0-9_]*
comment "#".*

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

"int" { yylval.rnd_type = Node::RoundingType::INT;
        return INTTYPE; }
"fl16" { yylval.rnd_type = Node::RoundingType::FL16;
         return FPTYPE; }
"fl32" { yylval.rnd_type = Node::RoundingType::FL32;
         return FPTYPE; }
"fl64" { yylval.rnd_type = Node::RoundingType::FL64;
         return FPTYPE; }

{comment} { }
{num}  {yylval.num.type = Number::INT;
        yylval.num.ival = atoi(yytext);
        yylval.num.fval = atof(yytext);
        return INT; }
([0-9]*\.[0-9]+([eE][-+]?[0-9]+)?) { yylval.num.type = Number::FP;
                                       yylval.num.fval = atof(yytext);
                                       return FP; }

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
"cos" { return COS; }
"tan" { return TAN; }
"sinh" { return SINH; }
"cosh" { return COSH; }
"tanh" { return TANH; }
"asin" { return ASIN; }
"acos" { return ACOS; }
"atan" { return ATAN; }

"sqrt" { return SQRT; }
"exp" { return EXP; }
"log" { return LOG; }

"fma" { return FMA; }

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

{id} { yylval.str = strdup(yytext);
        return ID; }

\n+ { return EOL; }
. {}

%%

int yywrap() { return 1; }

