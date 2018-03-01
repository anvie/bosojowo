%{
#include <string>
#include "node.h"
#include "parser.hpp"
#define SAVE_TOKEN yylval.string = new std::string(yytext, yyleng)
#define TOKEN(t) (yylval.token = t)
extern "C" int yywrap();
//int yylineno=1;
%}

%x C_COMMENT

%%

[ ]                   ;
"/*"            { BEGIN(C_COMMENT); }
<C_COMMENT>"*/" { BEGIN(INITIAL); }
<C_COMMENT>.    { }
"fungsi"                return TOKEN(TFUNC);
"mulai"                 return TOKEN(TBLOCKBEGIN);
"bar"               return TOKEN(TBLOCKEND);
"nyoh"               return TOKEN(TRETN);
"nek"                  return TOKEN(TIF);
"njuk"                  return TOKEN(TTHEN);
"nek ora"           return TOKEN(TELSE);
":"                     return TOKEN(TDDOT);
"muter"     return TOKEN(TLOOP);
"tekan"    return TOKEN(TUNTIL);
L?\"(\\.|[^\\"])*\"     SAVE_TOKEN; return TSTR;
[a-zA-Z_][a-zA-Z0-9_]*  SAVE_TOKEN; return TIDENTIFIER;
[0-9]+\.[0-9]*          SAVE_TOKEN; return TDOUBLE;
[0-9]+                  SAVE_TOKEN; return TINTEGER;
"="                     return TOKEN(TEQUAL);
"=="                    return TOKEN(TCEQ);
"!="                    return TOKEN(TCNE);
"<"                     return TOKEN(TCLT);
"<="                    return TOKEN(TCLE);
">"                     return TOKEN(TCGT);
">="                    return TOKEN(TCGE);
"("                     return TOKEN(TLPAREN);
")"                     return TOKEN(TRPAREN);
"{"                     return TOKEN(TLBRACE);
"}"                     return TOKEN(TRBRACE);
"."                     return TOKEN(TDOT);
","                     return TOKEN(TCOMMA);
"+"                     return TOKEN(TPLUS);
"-"                     return TOKEN(TMINUS);
"*"                     return TOKEN(TMUL);
"/"                     return TOKEN(TDIV);
\n                      yylineno++;
.                       printf("Unknown token!n"); yyterminate();
%%
