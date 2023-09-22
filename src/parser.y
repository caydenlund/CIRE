%{
    /* definitions */
    #include<iostream>
    int yylex();
    extern FILE *yyin;
    int yyerror(char *s);
    #define YYDEBUG 1
%}

%code requires {
    #include <string>
    using namespace std;
    struct Number {
        enum { INTEGER, FP } type;
        union {
            int ival;
            double fval;
        };
    };
    typedef struct Number Number;
}

%union value {
    Number num;

}

%token EOL
%token<num> INTEGER
%token<num> FP
%type<num> number

%token FPTYPE
%token ADD SUB MUL DIV
%token EQ NEQ LEQ LT GEQ GT
%token AND OR NOT
%token INPUTS OUTPUTS CONSTRAINTS EXPRS
%token IF THEN ELSE ENDIF
%token MINUS
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

program: inputs EOL outputs EOL constraints EOL exprs
        | inputs EOL outputs EOL exprs
        ;

intv_factor:    INTEGER
        | FP
        | ID
        ;

intv_term:  intv_factor
        | intv_term MUL intv_factor
        | intv_term DIV intv_factor
        ;

intv_expr:  intv_term
        | intv_expr ADD intv_term
        | intv_expr SUB intv_term
        ;

interval:   ID FPTYPE COLON LPAREN intv_expr COMMA intv_expr RPAREN SEMICOLON
        | EOL
        ;

interval_list: interval_list interval
        | interval
        ;

inputs: INPUTS LBRACE interval_list RBRACE
        | EOL
        ;

output: ID SEMICOLON
        | EOL
        ;

output_list:    output_list output
        | output
        ;

outputs: OUTPUTS LBRACE output_list RBRACE
        | EOL
        ;

cond_term:  LPAREN cond_expr RPAREN
        | arith_exp EQ arith_exp
        | arith_exp NEQ arith_exp
        | arith_exp LEQ arith_exp
        | arith_exp LT arith_exp
        | arith_exp GEQ arith_exp
        | arith_exp GT arith_exp
        ;

cond_expr:  cond_term
        | cond_expr AND cond_term
        | cond_expr OR cond_term
        ;

assign_constraint_expr: ID COLON cond_expr SEMICOLON
                        | EOL
                        ;

constraint_list:    constraint_list assign_constraint_expr
                | assign_constraint_expr
                ;

constraints:    CONSTRAINTS LBRACE constraint_list RBRACE
            | EOL
            ;

exprs:  EXPRS LBRACE stmts RBRACE
        ;

number: INTEGER { $$ = $1; }
        | FP { $$ = $1; }
        ;


arith_fact: number { $$ = $1; }
            | ID
            ;

arith_term: arith_fact { $$ = $1; }
            | arith_term MUL arith_fact { }
            | arith_term DIV arith_fact { }
            ;

arith_exp:  arith_term { $$ = $1; }
            | arith_exp ADD arith_term { }
            | arith_exp SUB arith_term { }
            ;

assign_exp: ID ASSIGN arith_exp SEMICOLON { }
        | EOL
        ;

if_block: IF cond_expr THEN stmts ELSE stmts ENDIF
    | IF cond_expr THEN stmts ENDIF
    ;

stmts:  stmts assign_exp
        | stmts if_block
        | assign_exp
        | if_block
        ;


%%

int main(int argc, char *argv[]) {
    yydebug = 1;
    yyin = fopen(argv[1], "r");
    if(!yyin) {
       std::cout << "Bad Input.Non-existant file" << std::endl;
       return -1;
    }
    do {
       std::cout << "Parsing..." << std::endl;
       yyparse();
    } while (!feof(yyin));

    return 0;
}

int yyerror(char *s) {
    printf("ERROR: %s\n", s);

    return 0;
}