%{
    /* definitions */
    #include<iostream>
    #include<string>
    int yylex();
    extern FILE *yyin;
    int yyerror(char *s);
%}

%code requires {
    struct Number {
        enum { INTEGER, FPTYPE } type;
        union {
            int ival;
            double fval;
        };
    };
    typedef struct Number Number;
}

%union value {
    Number num;
    char op;
}

%token EOL
%token<num> INTEGER
%token<num> FPTYPE
%type<num> number

%token<op> ADD SUB MUL DIV
%token INPUTS OUTPUTS EXPRS
%token<op> MINUS
%token SIN COS TAN
%token ASIN
%token COT
%token SINH COSH
%token SQRT EXP LOG
%token LBRACE RBRACE LPAREN RPAREN LBRACKET RBRACKET
%token COMMA COLON SEMICOLON ASSIGN
%token ID

%type<num> arith_exp arith_term arith_fact;

/* rules */
%%

program: exprs
        ;

exprs:  EXPRS LBRACE stmts RBRACE
        ;

stmts:  assign_exp stmts
        |
        ;

assign_exp: ID ASSIGN arith_exp SEMICOLON { printf("%lf\n", $3.fval); }
        | EOL
        ;

number: INTEGER { $$ = $1; }
        | FPTYPE { $$ = $1; }
        ;


arith_fact: number { $$ = $1; }
            ;

arith_term: arith_fact { $$ = $1; }
            | arith_term MUL arith_fact { }
            | arith_term DIV arith_fact { }
            ;

arith_exp:  arith_term { $$ = $1; }
            | arith_exp ADD arith_term { }
            | arith_exp SUB arith_term { }
            ;


%%

int main(int argc, char *argv[]) {
    yyin = fopen(argv[1], "r");
    if(!yyin) {
       std::cout << "Bad Input.Non-existant file" << std::endl;
       return -1;
    }
    do {
       yyparse();
    } while (!feof(yyin));

    return 0;
}

int yyerror(char *s) {
    printf("ERROR: %s\n", s);

    return 0;
}