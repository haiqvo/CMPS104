%{
// Dummy parser for scanner project.

#include "lyutils.h"
#include "astree.h"

%}

%debug
%defines
%error-verbose
%token-table
%verbose

%token NONTERM_CONSTANT NONTERM_VAR NONTERM_E NONTERM_CALL  
%token NONTERM_UNOP NONTERM_BINOP NONTERM_EXPR NONTERM_RET
%token NONTERM_IFELSE NONTERM_WHILE NONTERM_VARDECL NONTERM_ALLOCATOR
%token NONTERM_STATEMENT NONTERM_D NONTERM_BLOCK NONTERM_DECL
%token NONTERM_C NONTERM_FUNCTION NONTERM_BASETYPE NONTERM_TYPE 
%token NONTERM_B NONTERM_STRUCTDEF NONTERM_A NONTERM_PROGRAM 

%token TOK_VOID TOK_BOOL TOK_CHAR TOK_INT TOK_STRING
%token TOK_WHILE TOK_RETURN TOK_STRUCT
%token TOK_FALSE TOK_TRUE TOK_NULL 
%token TOK_IDENT TOK_INTCON TOK_CHARCON TOK_STRINGCON

%token TOK_NEWARRAY 

%nonassoc ')'    
%right TOK_IF TOK_ELSE
%right '='    
%left  TOK_EQ TOK_NE '<' TOK_LE '>' TOK_GE
%left  '+' '-'
%left  '*' '/' '%'
%right TOK_POS TOK_NEG '!' TOK_ORD TOK_CHR
%right  '[' '.'
%left  TOK_CALL
%right  TOK_ARRAY TOK_FIELD
%nonassoc TOK_NEW
%nonassoc TOK_PAREN


%start program

%%

program     : a { $$=new_parseroot(); adopt1($$, $1); 
                   adoptgchild($$,$1); }
            |   { $$=new_parseroot(); }
            ;

a           : structdef a
              { $$=new_astree(NONTERM_A, $1->filenr, 
                    $1->linenr, $1->offset, "NONTERM_A");
                adopt2($$, $1, $2); adoptgchild($$, $2); }
            | function a
              { $$=new_astree(NONTERM_A, $1->filenr, 
                    $1->linenr, $1->offset, "NONTERM_A");
                adopt2($$, $1, $2); adoptgchild($$, $2); }
            | statement a
              { $$=new_astree(NONTERM_A, $1->filenr, 
                    $1->linenr, $1->offset, "NONTERM_A");
                adopt2($$, $1, $2);
                adoptgchild($$, $1); adoptgchild($$, $2); }
            | structdef
              { $$=new_astree(NONTERM_A, $1->filenr, 
                    $1->linenr, $1->offset, "NONTERM_A");
                adopt1($$, $1); }
            | function
              { $$=new_astree(NONTERM_A, $1->filenr, 
                    $1->linenr, $1->offset, "NONTERM_A");
                adopt1($$, $1); }
            | statement
              { $$=new_astree(NONTERM_A, $1->filenr, 
                    $1->linenr, $1->offset, "NONTERM_A");
                adopt1($$, $1); adoptgchild($$, $1); }
            ;
structdef   : TOK_STRUCT TOK_IDENT '{' b '}'
              { $$=new_astree(NONTERM_STRUCTDEF, 
                    $1->filenr, $1->linenr, $1->offset, "structdef"); 
                adopt1($$, $2); adopt1($$,$4); adoptgchild($$, $4); }
            | TOK_STRUCT TOK_IDENT '{' '}'
              { $$=new_astree(NONTERM_STRUCTDEF, $1->filenr, 
                    $1->linenr, $1->offset, "structdef");
                adopt1($$, $2); }
            ;
b           : decl ';' b
              { $$=new_astree(NONTERM_B, $1->filenr, 
                    $1->linenr, $1->offset, "B_NONTERM");
                adopt2($$, $1, $3); adoptgchild($$, $3); }
            | decl ';'
              { $$=new_astree(NONTERM_B, $1->filenr, 
                    $1->linenr, $1->offset, "B_NONTERM");
                adopt1($$, $1); }
            ;
decl        : type TOK_IDENT
              { $$=new_astree(NONTERM_DECL, $1->filenr, 
                    $1->linenr, $1->offset, "decl");
                adopt2($$, $1, $2); }
            ;
type        : basetype
              { $$=new_astree(NONTERM_TYPE, $1->filenr, 
                    $1->linenr, $1->offset, "type");
                adopt1($$, $1); }
            | basetype TOK_NEWARRAY
              { $$=new_astree(NONTERM_TYPE, $1->filenr, 
                    $1->linenr, $1->offset, "type");
                adopt2($$, $1, $2); }
            ;
basetype    : TOK_VOID
              { $$=new_astree(NONTERM_BASETYPE, $1->filenr, 
                    $1->linenr, $1->offset, "basetype");
                adopt1($$, $1); }
            | TOK_BOOL
              { $$=new_astree(NONTERM_BASETYPE, $1->filenr, 
                    $1->linenr, $1->offset, "basetype");
                adopt1($$, $1); }
            | TOK_CHAR
              { $$=new_astree(NONTERM_BASETYPE, $1->filenr, 
                    $1->linenr, $1->offset, "basetype");
                adopt1($$, $1); }
            | TOK_INT
              { $$=new_astree(NONTERM_BASETYPE, $1->filenr, 
                    $1->linenr, $1->offset, "basetype");
                adopt1($$, $1); }
            | TOK_STRING
              { $$=new_astree(NONTERM_BASETYPE, $1->filenr, 
                    $1->linenr, $1->offset, "basetype");
                adopt1($$, $1); }
            | TOK_IDENT
              { $$=new_astree(NONTERM_BASETYPE, $1->filenr, 
                    $1->linenr, $1->offset, "basetype");
                adopt1($$, $1); }
            ;
function    : type TOK_IDENT '(' c ')' block
              { $$=new_astree(NONTERM_FUNCTION, $1->filenr, 
                    $1->linenr, $1->offset, "function");
                adopt2($$, $1, $2); adopt1($$, $4); 
                adoptgchild($$, $4); adopt1($$,$6); }
            | type TOK_IDENT '(' ')' block
              { $$=new_astree(NONTERM_FUNCTION, $1->filenr, 
                    $1->linenr, $1->offset, "function"); 
                adopt2($$, $1, $2); adopt1($$, $5); }
            ;
c           : decl
              { $$=new_astree(NONTERM_C, $1->filenr, 
                    $1->linenr, $1->offset, "C_NONTERM");
                adopt1($$, $1); }
            | c ',' decl
              { $$=new_astree(NONTERM_C, $1->filenr, 
                    $1->linenr, $1->offset, "C_NONTERM");
                adopt2($$, $1, $3); adoptgchild($$, $1); }
            ;
block       : '{' d '}'
              { $$=new_astree(NONTERM_BLOCK, $1->filenr, 
                    $1->linenr, $1->offset, "block");
                adopt1($$, $2); adoptgchild($$, $2);}
            | '{' '}'
              { $$=new_astree(NONTERM_BLOCK, $1->filenr, 
                    $1->linenr, $1->offset, "block");
                adopt2($$, $1, $2); }
            | ';'
              { $$=new_astree(NONTERM_BLOCK, $1->filenr, 
                    $1->linenr, $1->offset, "block");
                adopt1($$, $1); }
            ;
d           : d statement
              { $$=new_astree(NONTERM_D, $1->filenr, 
                    $1->linenr, $1->offset, "D_NONTERM"); 
                adopt2($$, $1, $2); 
                adoptgchild($$, $1); adoptgchild($$, $2); }
            | statement
              { $$=new_astree(NONTERM_D, $1->filenr, 
                    $1->linenr, $1->offset, "D_NONTERM");
                adopt1($$, $1); adoptgchild($$, $1); }
            ;
statement   : block
              { $$=new_astree(NONTERM_STATEMENT, $1->filenr, 
                    $1->linenr, $1->offset, "statement");
                adopt1($$, $1); }
            | vardecl
              { $$=new_astree(NONTERM_STATEMENT, $1->filenr, 
                    $1->linenr, $1->offset, "statement");
                adopt1($$, $1); }
            | while
              { $$=new_astree(NONTERM_STATEMENT, $1->filenr, 
                    $1->linenr, $1->offset, "statement");
                adopt1($$, $1); }
            | ifelse
              { $$=new_astree(NONTERM_STATEMENT, $1->filenr, 
                    $1->linenr, $1->offset, "statement");
                adopt1($$, $1); }
            | return 
              { $$=new_astree(NONTERM_STATEMENT, $1->filenr, 
                    $1->linenr, $1->offset, "statement");
                adopt1($$, $1); }
            | expr ';'
              { $$=new_astree(NONTERM_STATEMENT, $1->filenr, 
                    $1->linenr, $1->offset, "statement");
                adopt1($$, $1); adoptgchild($$, $1); }
            ;
vardecl     : type TOK_IDENT '=' expr ';'
              { $$=new_astree(NONTERM_VARDECL, $1->filenr, 
                    $1->linenr, $1->offset, "vardecl");
                adopt2($$, $1, $2); adopt1($$, $4); 
                adoptgchild($$, $4); }
            ;
while       : TOK_WHILE '(' expr ')' statement
              { $$=new_astree(NONTERM_WHILE, $1->filenr, 
                    $1->linenr, $1->offset, "while");
                adopt1($$, $3); adopt1($$, $5);
                adoptgchild($$, $3); adoptgchild($$, $5); }
            ;
ifelse      : TOK_IF '(' expr ')' statement TOK_ELSE statement
              { $$=new_astree(NONTERM_IFELSE, $1->filenr, 
                    $1->linenr, $1->offset, "ifelse"); 
                adopt1($$, $3); adoptgchild($$, $3);
                adoptgchild($$, $5); adoptgchild($$, $7); }
            | TOK_IF '(' expr ')' statement
              { $$=new_astree(NONTERM_IFELSE, $1->filenr, 
                    $1->linenr, $1->offset, "ifelse");
                adopt1($$, $3); adoptgchild($$, $3); 
                adoptgchild($$, $5); }
            ;
return      : TOK_RETURN expr ';'
              { $$=new_astree(NONTERM_RET, $1->filenr, 
                    $1->linenr, $1->offset, "return");
                adopt1($$, $2); adoptgchild($$, $2); }
            | TOK_RETURN ';'
              { $$=new_astree(NONTERM_RET, $1->filenr, 
                    $1->linenr, $1->offset, "return");
                adopt1($$, $2);}
            ;
expr        : binop
              { $$=new_astree(NONTERM_EXPR, $1->filenr, 
                    $1->linenr, $1->offset, "expr");
                adopt1($$, $1); }
            | unop %prec TOK_POS
              { $$=new_astree(NONTERM_EXPR, $1->filenr,
                    $1->linenr, $1->offset, "expr");
                adopt1($$, $1); }
            | allocator %prec TOK_NEW
              { $$=new_astree(NONTERM_EXPR, $1->filenr, 
                    $1->linenr, $1->offset, "expr");
                adopt1($$, $1); }
            | call %prec TOK_CALL
              { $$=new_astree(NONTERM_EXPR, $1->filenr, 
                    $1->linenr, $1->offset, "expr");
                adopt1($$, $1); }
            | '(' expr ')' %prec TOK_PAREN
              { $$=new_astree(NONTERM_EXPR, $1->filenr, 
                    $1->linenr, $1->offset, "expr");
                adopt1($$, $2); adoptgchild($$, $2); }
            | variable %prec TOK_ARRAY
              { $$=new_astree(NONTERM_EXPR, $1->filenr, 
                    $1->linenr, $1->offset, "expr");
                adopt1($$, $1); }
            | constant
              { $$=new_astree(NONTERM_EXPR, $1->filenr, 
                    $1->linenr, $1->offset, "expr");
                adopt1($$, $1); }
            ;          
binop       : expr '=' expr 
              { $$=new_astree(NONTERM_BINOP, $1->filenr, 
                    $1->linenr, $1->offset, "binop");
                adopt1($$, $1); adoptgchild($$, $1);
                adopt1($$, $2); adopt1($$, $3); adoptgchild($$, $3); }
            | expr TOK_EQ expr
              { $$=new_astree(NONTERM_BINOP, $1->filenr, 
                  $1->linenr, $1->offset, "binop");
                adopt1($$, $1); adoptgchild($$, $1); adopt1($$, $2);
                adopt1($$, $3); adoptgchild($$, $3); }
            | expr TOK_NE expr
              { $$=new_astree(NONTERM_BINOP, $1->filenr, 
                    $1->linenr, $1->offset, "binop");
                adopt1($$, $1); adoptgchild($$, $1); adopt1($$, $2);
                adopt1($$, $3); adoptgchild($$, $3); }
            | expr '<' expr
              { $$=new_astree(NONTERM_BINOP, $1->filenr, 
                    $1->linenr, $1->offset, "binop");
                adopt1($$, $1); adoptgchild($$, $1); adopt1($$, $2);
                adopt1($$, $3); adoptgchild($$, $3); }
            | expr TOK_LE expr
              { $$=new_astree(NONTERM_BINOP, $1->filenr, 
                    $1->linenr, $1->offset, "binop");
                adopt1($$, $1); adoptgchild($$, $1); adopt1($$, $2);
                adopt1($$, $3); adoptgchild($$, $3); }
            | expr '>' expr
              { $$=new_astree(NONTERM_BINOP, $1->filenr, 
                    $1->linenr, $1->offset, "binop");
                adopt1($$, $1); adoptgchild($$, $1); adopt1($$, $2);
                adopt1($$, $3); adoptgchild($$, $3); }
            | expr TOK_GE expr 
              { $$=new_astree(NONTERM_BINOP, $1->filenr, 
                    $1->linenr, $1->offset, "binop");
                adopt1($$, $1); adoptgchild($$, $1); adopt1($$, $2);
                adopt1($$, $3); adoptgchild($$, $3); }
            | expr '+' expr
              { $$=new_astree(NONTERM_BINOP, $1->filenr, 
                    $1->linenr, $1->offset, "binop");
                adopt1($$, $1); adoptgchild($$, $1); adopt1($$, $2);
                adopt1($$, $3); adoptgchild($$, $3); }
            | expr '-' expr
              { $$=new_astree(NONTERM_BINOP, $1->filenr, 
                    $1->linenr, $1->offset, "binop");
                adopt1($$, $1); adoptgchild($$, $1); adopt1($$, $2);
                adopt1($$, $3); adoptgchild($$, $3); }
            | expr '*' expr
              { $$=new_astree(NONTERM_BINOP, $1->filenr, 
                    $1->linenr, $1->offset, "binop");
                adopt1($$, $1); adoptgchild($$, $1); adopt1($$, $2);
                adopt1($$, $3); adoptgchild($$, $3); }
            | expr '/' expr
              { $$=new_astree(NONTERM_BINOP, $1->filenr, 
                    $1->linenr, $1->offset, "binop");
                adopt1($$, $1); adoptgchild($$, $1); adopt1($$, $2);
                adopt1($$, $3); adoptgchild($$, $3); }
            | expr '%' expr
              { $$=new_astree(NONTERM_BINOP, $1->filenr, 
                    $1->linenr, $1->offset, "binop");
                adopt1($$, $1); adoptgchild($$, $1); adopt1($$, $2);
                adopt1($$, $3); adoptgchild($$, $3); }
            ;
unop        : '+' expr %prec TOK_POS
              { $$=new_astree(NONTERM_UNOP, $1->filenr, 
                    $1->linenr, $1->offset,"unop");
                adopt1($$, $1); adopt1($$, $2); adoptgchild($$, $2); }
            | '-' expr %prec TOK_NEG
              { $$=new_astree(NONTERM_UNOP, $1->filenr, 
                    $1->linenr, $1->offset,"unop");
                adopt1($$, $1); adopt1($$, $2); adoptgchild($$, $2); }
            | '!' expr
              { $$=new_astree(NONTERM_UNOP, $1->filenr, 
                    $1->linenr, $1->offset,"unop");
                adopt1($$, $1); adopt1($$, $2); adoptgchild($$, $2); }
            | TOK_ORD expr
              { $$=new_astree(NONTERM_UNOP, $1->filenr, 
                    $1->linenr, $1->offset,"unop");
                adopt1($$, $1); adopt1($$, $2); adoptgchild($$, $2); }
            | TOK_CHR expr
              { $$=new_astree(NONTERM_UNOP, $1->filenr, 
                    $1->linenr, $1->offset,"unop");
                adopt1($$, $1); adopt1($$, $2); adoptgchild($$, $2); }
            ;
allocator   : TOK_NEW basetype '(' expr ')'
              { $$=new_astree(NONTERM_ALLOCATOR, $1->filenr, 
                    $1->linenr, $1->offset,"allocator");
                adopt2($$, $2, $4); adoptgchild($$, $4); }
            | TOK_NEW basetype '(' ')'
              { $$=new_astree(NONTERM_ALLOCATOR, $1->filenr, 
                    $1->linenr, $1->offset,"allocator");
                adopt1($$, $2); }
            | TOK_NEW basetype '[' expr ']'
              { $$=new_astree(NONTERM_ALLOCATOR, $1->filenr, 
                    $1->linenr, $1->offset,"allocator");
                adopt2($$, $2, $4); adoptgchild($$, $4); }
            ;
call        : TOK_IDENT '(' E ')'
              { $$=new_astree(NONTERM_CALL, $1->filenr, 
                    $1->linenr, $1->offset,"call");
                adopt1($$,$1); adoptgchild($$, $3); }
            | TOK_IDENT '(' ')'
              { $$=new_astree(NONTERM_CALL, $1->filenr, 
                    $1->linenr, $1->offset,"call");
                adopt1($$,$1); }
            ;
E           : expr 
              { $$=new_astree(NONTERM_E, $1->filenr, 
                    $1->linenr, $1->offset,"E_NONTERM");
                adopt1($$,$1); adoptgchild($$, $1); }
            | expr ',' E
              { $$=new_astree(NONTERM_E, $1->filenr, 
                    $1->linenr, $1->offset,"E_NONTERM");
                adopt2($$,$1,$3); 
                adoptgchild($$, $3); adoptgchild($$, $1); }
            ;
variable    : TOK_IDENT
              { $$=new_astree(NONTERM_VAR, $1->filenr, 
                    $1->linenr, $1->offset,"variable");
                adopt1($$, $1); }
            | expr '[' expr ']' %prec TOK_ARRAY
              { $$=new_astree(TOK_ARRAY, $1->filenr, 
                    $1->linenr, $1->offset,"variable");
                adopt2($$, $1, $3);
                adoptgchild($$, $1); adoptgchild($$, $3); }
            | expr '.' TOK_IDENT %prec TOK_FIELD
              { $$=new_astree(TOK_FIELD, $1->filenr, 
                    $1->linenr, $1->offset,"variable");
                adopt2($$,$1,$3); adoptgchild($$, $1); }
            ;
constant    : TOK_INTCON
              { $$=new_astree(NONTERM_CONSTANT, $1->filenr, 
                    $1->linenr, $1->offset,"constant"); 
                adopt1($$, $1); }
            | TOK_CHARCON
              { $$=new_astree(NONTERM_CONSTANT, $1->filenr, 
                    $1->linenr, $1->offset,"constant"); 
                adopt1($$, $1); }
            | TOK_STRINGCON
              { $$=new_astree(NONTERM_CONSTANT, $1->filenr, 
                    $1->linenr, $1->offset,"constant"); 
                adopt1($$, $1); }
            | TOK_FALSE
              { $$=new_astree(NONTERM_CONSTANT, $1->filenr, 
                    $1->linenr, $1->offset,"constant"); 
                adopt1($$, $1); }
            | TOK_TRUE
              { $$=new_astree(NONTERM_CONSTANT, $1->filenr, 
                    $1->linenr, $1->offset,"constant"); 
                adopt1($$, $1); }
            | TOK_NULL
              { $$=new_astree(NONTERM_CONSTANT, $1->filenr, 
                    $1->linenr, $1->offset,"constant"); 
                adopt1($$, $1); }
            ;

%%

const char *get_yytname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}


bool is_defined_token (int symbol) {
   return YYTRANSLATE (symbol) > YYUNDEFTOK;
}

/*static void* yycalloc (size_t size) {
   void* result = calloc (1, size);
   assert (result != NULL);
   return result;
}*/
