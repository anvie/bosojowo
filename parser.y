%{
    #include "node.h"

    NBlock *programBlock; /* the top level root node of our final AST */

    extern int yyget_lineno();
    extern int yylex();
    void yyerror(const char *s) { fprintf(stderr,"%s At line %d\n", s, yyget_lineno()); }
%}

/* Represents the many different ways we can access our data */
%union {
    Node *node;
    NBlock *block;
    NExpression *expr;
    NStatement *stmt;
    NIdentifier *ident;
    NVariableDeclaration *var_decl;
    std::vector<NVariableDeclaration*> *varvec;
    std::vector<NExpression*> *exprvec;
    std::string *string;
    int token;
}

/* Define our terminal symbols (tokens). This should
   match our tokens.l lex file. We also define the node type
   they represent.
 */
%token <string> TIDENTIFIER TINTEGER TDOUBLE TSTR
%token <token> TCEQ TCNE TCLT TCLE TCGT TCGE TEQUAL
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE TCOMMA TDOT TDDOT TRETN TFUNC TBLOCKBEGIN TBLOCKEND TIF TTHEN TELSE
%token <token> TPLUS TMINUS TMUL TDIV
%token <token> TLOOP TUNTIL

/* Define the type of node our nonterminal symbols represent.
   The types refer to the %union declaration above. Ex: when
   we call an ident (defined by union type ident) we are really
   calling an (NIdentifier*). It makes the compiler happy.
 */
%type <ident> ident
%type <expr> numeric conditional expr
%type <varvec> func_decl_args
%type <exprvec> call_args
%type <block> program stmts block
%type <stmt> stmt var_decl func_decl
%type <token> comparison

/* Operator precedence for mathematical operators */
%left TPLUS TMINUS
%left TMUL TDIV

%start program

%%

program : stmts { programBlock = $1; }
        ;

stmts : stmt { $$ = new NBlock(); $$->statements.push_back($<stmt>1); }
      | stmts stmt { $1->statements.push_back($<stmt>2); }
      ;

stmt : var_decl | func_decl
     | TRETN expr { $$ = new NReturn($2); }
     | TRETN { $$ = new NReturn(); }
     | expr { $$ = new NExpressionStatement(*$1); }
     ;

block : TLBRACE stmts TRBRACE { $$ = $2; }
      | TLBRACE TRBRACE { $$ = new NBlock(); }
      | TBLOCKBEGIN stmts TBLOCKEND { $$ = $2; }
      | TBLOCKBEGIN TBLOCKEND { $$ = new NBlock(); }
      ;

var_decl : ident ident { $$ = new NVariableDeclaration(*$1, *$2); }
         | ident ident TEQUAL expr { $$ = new NVariableDeclaration(*$1, *$2, $4); }
         ;

func_decl : TFUNC ident TLPAREN func_decl_args TRPAREN TDDOT ident block
            { $$ = new NFunctionDeclaration($7, *$2, *$4, *$8); delete $4; }
          | TFUNC ident TLPAREN func_decl_args TRPAREN block { $$ = new NFunctionDeclaration(nullptr, *$2, *$4, *$6); delete $4; }
          | TFUNC ident TLPAREN TRPAREN block { $$ = new NFunctionDeclaration(nullptr, *$2, *$5); }
          ;

func_decl_args : { $$ = new VariableList(); }
          | var_decl { $$ = new VariableList(); $$->push_back($<var_decl>1); }
          | func_decl_args TCOMMA var_decl { $1->push_back($<var_decl>3); }
          ;

ident : TIDENTIFIER { $$ = new NIdentifier(*$1); delete $1; }
      ;

numeric : TINTEGER { $$ = new NInteger(atol($1->c_str())); delete $1; }
        | TDOUBLE { $$ = new NDouble(atof($1->c_str())); delete $1; }
        ;

conditional : TIF expr TTHEN stmts TELSE stmts { $$ = new NConditionalBlock(*$2, $4, $6); }
            | TIF expr TTHEN block { $$ = $4; }
            ;

expr : ident TEQUAL expr { $$ = new NAssignment(*$<ident>1, *$3); }
     | ident TLPAREN call_args TRPAREN { $$ = new NMethodCall(*$1, *$3); delete $3; }
     | conditional
     | ident { $<ident>$ = $1; }
     | numeric
     | TSTR { $$ = new NStr(*$1); delete $1; }
     | expr comparison expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
     | TLPAREN expr TRPAREN { $$ = $2; }
     | TLOOP expr TUNTIL expr block { $$ = new NLoop(*$2, *$4, $5); }
     ;

call_args : { $$ = new ExpressionList(); }
          | expr { $$ = new ExpressionList(); $$->push_back($1); }
          | call_args TCOMMA expr  { $1->push_back($3); }
          ;

comparison : TCEQ | TCNE | TCLT | TCLE | TCGT | TCGE
           | TPLUS | TMINUS | TMUL | TDIV
           ;

%%
