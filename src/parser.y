%{
    /* definitions */
    #define YYDEBUG 1
%}

%code requires {
    #include "../include/Graph.h"
    #include "../include/CIRE.h"
    #include<iostream>
    #include <ibex.h>
    #include <ibex_Expr.h>

    extern FILE *yyin;
    extern int yydebug;

    int yylex(Graph *graph);
    int yyerror(Graph *graph, char *s);


    struct Number {
        enum { INT, FP } type;
        union {
            int ival;
            double fval;
        };
    };
    typedef struct Number Number;
}

%param { Graph *graph }

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

%type<interval_vector> interval_list inputs;
%type<node> arith_exp arith_term arith_fact interval assign_exp;
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
                if(Node *FreeVarNode = graph->findFreeVarNode($1)) {
                    $$ = FreeVarNode;
                    assert(FreeVarNode->type == NodeType::FREE_VARIABLE);
                } else {
                    $$ = graph->inputs[$1] = new FreeVariable(*new ibex::Interval($5.fval, $7.fval));
                    graph->nodes.insert($$);
                }
                // std::cout << *graph << std::endl;
            }
            | EOL {  }
            ;

interval_list: interval_list interval { }
        | interval { }
        ;

inputs: INPUTS LBRACE interval_list RBRACE { }
        | EOL { }
        ;

output: ID SEMICOLON {
            graph->outputs.push_back($1);
        }
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
            graph->nodes.insert($$);
            std::cout << *$$ << std::endl;
        }
        | FP {
            $$ = new Double(ibex::ExprConstant::new_scalar($1.fval));
            graph->nodes.insert($$);
            std::cout << *$$ << std::endl;
        }
        ;


arith_fact: number { $$ = $1; }
            | ID {
                if(Node *VarNode = graph->findVarNode($1)) {
                    $$ = VarNode;
                } else {
                    $$ = graph->variables[$1] = new VariableNode(ibex::ExprSymbol::new_($1));
                    graph->nodes.insert($$);
                }
                std::cout << *$$ << std::endl;
            }
            ;

arith_term: arith_fact {
                $$ = $1;
            }
            | arith_term MUL arith_fact {
                $$ = *$1*$3;
                graph->nodes.insert($$);
                std::cout << *$$ << std::endl;
            }
            | arith_term DIV arith_fact {
                $$ = *$1/$3;
                graph->nodes.insert($$);
                std::cout << *$$ << std::endl;
            }
            ;

arith_exp:  arith_term {
                $$ = $1;
            }
            | arith_exp ADD arith_term {
                $$ = *$1+$3;
                graph->nodes.insert($$);
                std::cout << *$$ << std::endl;
            }
            | arith_exp SUB arith_term {
                $$ = *$1-$3;
                graph->nodes.insert($$);
                std::cout << *$$ << std::endl;
            }
            ;

assign_exp: ID ASSIGN arith_exp SEMICOLON {
                $$ = graph->variables[$1] = $3;
                std::cout << *$$ << std::endl;
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


int yyerror(Graph *graph, char *s) {
    std::cout << "ERROR: " << s << std::endl;

    return 0;
}