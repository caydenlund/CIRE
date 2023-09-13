%{
    /* definitions */
    #include<iostream>
    #include<string>
    int yylex();
    extern FILE *yyin;
    int yyerror(char *s);
%}

%union {
  int num;
  char sym;
}

%token EOL
%token<num> NUMBER

%token ADD SUB MUL DIV
%token INPUTS OUTPUTS EXPRS
%token MINUS
%token SIN COS TAN
%token ASIN
%token COT
%token SINH COSH
%token SQRT EXP LOG
%token LBRACE RBRACE LPAREN RPAREN LBRACKET RBRACKET
%token COMMA COLON SEMICOLON ASSIGN
%token ID

%type<num> arith_exp;
%type<num> arith_term;
%type<num> arith_fact;

/* rules */
%%

input:
        | stmt input
        ;

stmt:   ID ASSIGN arith_exp SEMICOLON { printf("%d\n", $3); }
        | EOL;

arith_fact: NUMBER { $$ = $1; }
            | LOG arith_fact
            | ID

arith_term: arith_fact { $$ = $1; }
            | arith_term MUL arith_fact { $$ = $1 * $3; }
            | arith_term DIV arith_fact { $$ = $1 / $3; }

arith_exp:  arith_term { $$ = $1; }
            | arith_exp ADD arith_term { $$ = $1 + $3; }
            | arith_exp SUB arith_term { $$ = $1 - $3; }
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