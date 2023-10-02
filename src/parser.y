%{
    /* definitions */
    #include<iostream>

    int yylex();
    extern FILE *yyin;


    int yyerror(char *s);
    #define YYDEBUG 1
%}

%code requires {
    #include "../include/Graph.h"
    #include <ibex.h>
    #include <ibex_Expr.h>

    extern Graph *graph;
    struct Number {
        enum { INT, FP } type;
        union {
            int ival;
            double fval;
        };
    };
    typedef struct Number Number;
}

%union {
    Number num;
    char *str;
    Node *node;
    ibex::Interval *interval;
    ibex::IntervalVector *interval_vector;
}

%token EOL
%token<num> INT
%token<num> FP

%type<node> number

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
%token<str> ID

%type<interval> interval;
%type<interval_vector> interval_list inputs;
%type<node> arith_exp arith_term arith_fact;
%type<num> intv_expr intv_term intv_factor;

/* rules */
%%

program: inputs EOL outputs EOL constraints EOL exprs {

        }
        | inputs EOL outputs EOL exprs {

        }
        ;

intv_factor:    FP { $$ = $1; }
                ;

intv_term:  intv_factor { $$ = $1; }
        | intv_term MUL intv_factor {
            $$ = $1;
            $$.fval = $1.fval * $3.fval; }
        | intv_term DIV intv_factor {
            $$ = $1;
            $$.fval = $1.fval / $3.fval; }
        ;

intv_expr:  intv_term { $$ = $1; }
        | intv_expr ADD intv_term {
            $$ = $1;
            $$.fval = $1.fval + $3.fval;
        }
        | intv_expr SUB intv_term {
            $$ = $1;
            $$.fval = $1.fval - $3.fval;
        }
        ;

interval:   ID FPTYPE COLON LPAREN intv_expr COMMA intv_expr RPAREN SEMICOLON {
                /* $$ = new ibex::Interval($5.fval, $7.fval);
                std::cout << *$$ << std::endl; */

                graph->inputs[new VariableNode(ibex::ExprSymbol::new_($1))] =
                                        new ibex::Interval($5.fval, $7.fval);
                std::cout << *graph << std::endl;
            }
            | EOL {  }
            ;

interval_list: interval_list interval {
            /*if ($1 != NULL) {
                $1->resize($1->size() + 1);
                $1[$1->size() - 1] = *$2;
            }

            $$ = $1;*/
        }
        | interval {
            /*if ($1 != NULL) {
                $$ = new ibex::IntervalVector(*$1);
                std::cout << *$$ << std::endl;
            }*/
        }
        ;

inputs: INPUTS LBRACE interval_list RBRACE {
            if ($3 != NULL) {
                $$ = $3;
            }
        }
        | EOL {}
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

number: INT {
            $$ = new Integer(ibex::ExprConstant::new_scalar($1.ival));
            std::cout << *$$ << std::endl;
        }
        | FP {
            $$ = new Double(ibex::ExprConstant::new_scalar($1.fval));
            std::cout << *$$ << std::endl;
        }
        ;


arith_fact: number { $$ = $1; }
            | ID {
                if(graph->inputs.find(new VariableNode(ibex::ExprSymbol::new_($1))) == graph->inputs.end()) {
                    $$ = new VariableNode(ibex::ExprSymbol::new_($1));
                    graph->inputs[(VariableNode*)$$] = new ibex::Interval(0.0, 0.0);
                } else {

                }
                std::cout << *$$ << std::endl;
            }
            ;

arith_term: arith_fact {
                $$ = $1;
            }
            | arith_term MUL arith_fact {
                ibex::ExprNode *a = $1->getExprNode();
                ibex::ExprNode *b = $3->getExprNode();

                const ibex::ExprBinaryOp *c = (ibex::ExprBinaryOp*)&ibex::ExprMul::new_(*a, *b);

                $$ = new BinaryOp($1, $3, BinaryOp::MUL, *c);
                std::cout << *$$ << std::endl;
            }
            | arith_term DIV arith_fact {
                ibex::ExprNode *a = $1->getExprNode();
                ibex::ExprNode *b = $3->getExprNode();

                const ibex::ExprBinaryOp *c = (ibex::ExprBinaryOp*)&ibex::ExprDiv::new_(*a, *b);

                $$ = new BinaryOp($1, $3, BinaryOp::DIV, *c);
                std::cout << *$$ << std::endl;
            }
            ;

arith_exp:  arith_term {
                $$ = $1;
            }
            | arith_exp ADD arith_term {
                ibex::ExprNode *a = $1->getExprNode();
                ibex::ExprNode *b = $3->getExprNode();

                const ibex::ExprBinaryOp *c = (ibex::ExprBinaryOp*)&ibex::ExprAdd::new_(*a, *b);

                $$ = new BinaryOp($1, $3, BinaryOp::ADD, *c);
                std::cout << *$$ << std::endl;
            }
            | arith_exp SUB arith_term {
                ibex::ExprNode *a = $1->getExprNode();
                ibex::ExprNode *b = $3->getExprNode();

                const ibex::ExprBinaryOp *c = (ibex::ExprBinaryOp*)&ibex::ExprSub::new_(*a, *b);

                $$ = new BinaryOp($1, $3, BinaryOp::SUB, *c);
                std::cout << *$$ << std::endl;
            }
            ;

assign_exp: ID ASSIGN arith_exp SEMICOLON {

        }
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
    graph = new Graph();

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

    free(graph);
    return 0;
}

int yyerror(char *s) {
    std::cout << "ERROR: " << s << std::endl;

    return 0;
}