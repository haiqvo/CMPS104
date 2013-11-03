%{

#include "lyutils.h"
#include "auxlib.h"

#define YY_USER_ACTION scanner_useraction();

%}

%option 8bit
%option debug
%option ecs
%option nodefault
%option nounput
%option noyywrap
%option perf-report
%option verbose
%option warn

LETTER          [A-Za-z_]
DIGIT           [0-9]
IDENT           ({LETTER}({LETTER}|{DIGIT})*)

%%
"[]"            {return yylval_token(TOK_NEWARRAY); }
"("             {return yylval_token('('); }
")"             {return yylval_token(')'); }
"["             {return yylval_token('['); }
"]"             {return yylval_token(']'); }
"{"             {return yylval_token('{'); }
"}"             {return yylval_token('}'); }
";"             {return yylval_token(';'); }
","             {return yylval_token(','); }
"."             {return yylval_token('.'); }
"="             {return yylval_token('='); }
"=="            {return yylval_token(TOK_EQ); }
"!="            {return yylval_token(TOK_NE); }
"<"             {return yylval_token('<'); }
"<="            {return yylval_token(TOK_LE); }
">"             {return yylval_token('>'); }
">="            {return yylval_token(TOK_GE); }
"+"             {return yylval_token('+'); }
"-"             {return yylval_token('-'); }
"*"             {return yylval_token('*'); }
"/"             {return yylval_token('/'); }
"%"             {return yylval_token('%'); }
"!"             {return yylval_token('!'); }

"void"          {return yylval_token(TOK_VOID); }
"bool"          {return yylval_token(TOK_BOOL); }
"char"          {return yylval_token(TOK_CHAR); }
"int"           {return yylval_token(TOK_INT); }
"string"        {return yylval_token(TOK_STRING); }
"struct"        {return yylval_token(TOK_STRUCT); }
"if"            {return yylval_token(TOK_IF); }
"else"          {return yylval_token(TOK_ELSE); }
"while"         {return yylval_token(TOK_WHILE); }
"return"        {return yylval_token(TOK_RETURN); }
"new"           {return yylval_token(TOK_NEW); }
"false"         {return yylval_token(TOK_FALSE); }
"true"          {return yylval_token(TOK_TRUE); }
"null"          {return yylval_token(TOK_NULL); }
"ord"           {return yylval_token(TOK_ORD); }
"chr"           {return yylval_token(TOK_CHR); }

{IDENT}         {return yylval_token(TOK_IDENT); }

{DIGIT}*        {return yylval_token(TOK_INTCON); }

('([^\\'\n]|\\[\\'"0nt])')      {return yylval_token(TOK_CHARCON); }

(\"([^\\"\n]|\\[\\'"0nt])*\")   {return yylval_token(TOK_STRINGCON); }

[ \t]+          {}

"#".*           {scanner_include(); }

\n              {scanner_newline(); }

({DIGIT}+{LETTER}({LETTER}|{DIGIT})*)   {scanner_badtoken (yytext);}

('([^\\'\n]|\\[\\'"0nt])(.*)')          {scanner_badtoken (yytext);}

('([^\\'\n]|\\[\\'"0nt]))               {scanner_badtoken (yytext);}

(\"([^\\"\n]|\\[\\'"0nt])*)             {scanner_badtoken (yytext);}

.               {scanner_badchar(*yytext); }
%%
