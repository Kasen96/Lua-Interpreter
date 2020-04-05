%top{
    #include "main.h"
    #include "node.h"
}

%x QUOTE

%option noyywrap nounput batch noinput 

%%

do      { return yy::parser::make_DO(yytext); }
end     { return yy::parser::make_END(yytext); }
while   { return yy::parser::make_WHILE(yytext); }
repeat  { return yy::parser::make_REPEAT(yytext); }
until   { return yy::parser::make_UNTIL(yytext); }
for     { return yy::parser::make_FOR(yytext); }
in      { return yy::parser::make_IN(yytext); }
break   { return yy::parser::make_BREAK(yytext); }

if      { return yy::parser::make_IF(yytext); }
then    { return yy::parser::make_THEN(yytext); }
elseif  { return yy::parser::make_ELSEIF(yytext); }
else    { return yy::parser::make_ELSE(yytext); }

local    { return yy::parser::make_LOCAL(yytext); }
function { return yy::parser::make_FUNCTION(yytext); }
return   { return yy::parser::make_RETURN(yytext); }

and     { return yy::parser::make_AND(yytext); }
or      { return yy::parser::make_OR(yytext); }
not     { return yy::parser::make_NOT(yytext); }
#       { return yy::parser::make_LEN(yytext); }

"+"     { return yy::parser::make_ADD(yytext); }
"-"     { return yy::parser::make_SUB(yytext); }
"*"     { return yy::parser::make_MUL(yytext); }
"/"     { return yy::parser::make_DIV(yytext); }
"^"     { return yy::parser::make_POW(yytext); }
"%"     { return yy::parser::make_MOD(yytext); }
"=="    { return yy::parser::make_EQUALS(yytext); }
"~="    { return yy::parser::make_NOTEQUALS(yytext); }
"<"     { return yy::parser::make_LESSTHAN(yytext); }
"<="    { return yy::parser::make_LESSEQUALS(yytext); }
">"     { return yy::parser::make_GREATTHAN(yytext); }
">="    { return yy::parser::make_GREATEQUALS(yytext); }
".."    { return yy::parser::make_CONCAT(yytext); }

nil                       { return yy::parser::make_NIL(yytext); }
false                     { return yy::parser::make_FALSE(yytext); }
true                      { return yy::parser::make_TRUE(yytext); }
"..."                     { return yy::parser::make_TRIPLEDOTS(yytext); }
([0-9]*\.[0-9]+)|([0-9]+) { return yy::parser::make_NUMBER(yytext); }
[_a-zA-Z][_a-zA-Z0-9]*    { return yy::parser::make_NAME(yytext); }

\"                        { BEGIN(QUOTE); }
<QUOTE>[^\"]*             { return yy::parser::make_STRING(yytext); }
<QUOTE>\"                 { BEGIN(0); }

"("     { return yy::parser::make_LPARENTHESE(yytext); }
")"     { return yy::parser::make_RPARENTHESE(yytext); }
"["     { return yy::parser::make_LBRACKET(yytext); }
"]"     { return yy::parser::make_RBRACKET(yytext); }
"{"     { return yy::parser::make_LBRACE(yytext); }
"}"     { return yy::parser::make_RBRACE(yytext); }

[ \t]   { /* space */ }
\n      { /* new line */ }
";"     { return yy::parser::make_SEMI(yytext); }
"="     { return yy::parser::make_ASSIGN(yytext); }
","     { return yy::parser::make_COMMA(yytext); }
"."     { return yy::parser::make_DOT(yytext); }
":"     { return yy::parser::make_COLON(yytext); }
<<EOF>> { return yy::parser::make_QUIT(); }
%%