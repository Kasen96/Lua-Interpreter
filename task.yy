%skeleton "lalr1.cc"
%language "c++"
%defines
%define api.value.type variant
%define api.token.constructor

%code requires{
    #include "node.h"
}

%code provides {
    extern FILE *yyin;
}

%code{
    #include "main.h"
    Node root;
}

%token <std::string> DO END WHILE REPEAT UNTIL FOR IN BREAK
%token <std::string> IF THEN ELSEIF ELSE
%token <std::string> LOCAL FUNCTION RETURN
%token <std::string> AND OR NOT LEN
%token <std::string> ADD SUB MUL DIV POW MOD EQUALS NOTEQUALS LESSTHAN LESSEQUALS GREATTHAN GREATEQUALS CONCAT
%token <std::string> NIL FALSE TRUE TRIPLEDOTS NUMBER STRING NAME
%token <std::string> LPARENTHESE RPARENTHESE LBRACKET RBRACKET LBRACE RBRACE
%token <std::string> SEMI ASSIGN COMMA DOT COLON

%token QUIT 0 "end of file"

%type <Node> chunk stat laststat block varlist explist functioncall namelist funcname funcbody var 
             prefixexp function tableconstructor unop args parlist fieldlist field
%type <Node> ifstat thenstat
%type <Node> exp exp_or exp_and exp_lge exp_concat exp_as exp_md exp_unop exp_pow exp_rest
%type <std::string> optsemi fieldsep

/* Follow the EBNF grammar of Lua. http://www.lua.org/manual/5.1/manual.html#8 */
%%

chunk : stat optsemi           { $$ = Node("chunk", ""); $$.children.push_back($1); root = $$; }
      | chunk stat optsemi     { $$ = $1; $$.children.push_back($2); root = $$; }
      | laststat optsemi       { $$ = Node("chunk", ""); $$.children.push_back($1); root = $$; }
      | chunk laststat optsemi { $$ = $1; $$.children.push_back($2); root = $$; }
      ;

optsemi :      { /* empty */ }
        | SEMI { /* empty */ }
        ;

block : chunk { $$ = Node("block", ""); $$.children.push_back($1); }
      ;

stat : varlist ASSIGN explist { $$ = Node("stat", ""); $$.children.push_back($1); $$.children.push_back(Node("ASSIGN", $2)); $$.children.push_back($3); }
     | functioncall           { $$ = $1; }
     | DO block END           { $$ = Node("stat", "DO"); $$.children.push_back($2); }
     | WHILE exp DO block END { $$ = Node("stat", "WHILE"); $$.children.push_back($2); $$.children.push_back($4); }
     | REPEAT block UNTIL exp { $$ = Node("stat", "REPEAT"); $$.children.push_back($2); $$.children.push_back($4); }
     | ifstat                 { $$ = $1; }
     
     /* for loop */
     | FOR NAME ASSIGN exp COMMA exp DO block END { 
         $$ = Node("stat", "FOR");
         $$.children.push_back(Node("NAME", $2)); 
         $$.children.push_back(Node("ASSIGN", $3));
         $$.children.push_back($4); 
         $$.children.push_back($6); 
         $$.children.push_back($8); 
        }
     | FOR NAME ASSIGN exp COMMA exp COMMA exp DO block END {
         $$ = Node("stat", "FOR");
         $$.children.push_back(Node("NAME", $2)); 
         $$.children.push_back(Node("ASSIGN", $3));
         $$.children.push_back($4); 
         $$.children.push_back($6); 
         $$.children.push_back($8); 
         $$.children.push_back($10); 
        }
     | FOR namelist IN explist DO block END { 
         $$ = Node("stat", "FOR");
         $$.children.push_back($2); 
         $$.children.push_back($4); 
         $$.children.push_back($6); 
        }

     | FUNCTION funcname funcbody    { $$ = Node("stat", "FUNCTION"); $$.children.push_back($2); $$.children.push_back($3); }
     | LOCAL FUNCTION NAME funcbody  { $$ = Node("stat", "FUNCTION"); $$.children.push_back(Node("NAME", $3)); $$.children.push_back($4); }
     | LOCAL namelist                { $$ = Node("stat", ""); $$.children.push_back($2); }
     | LOCAL namelist ASSIGN explist { $$ = Node("stat", ""); $$.children.push_back($2); $$.children.push_back(Node("ASSIGN", $3)); $$.children.push_back($4); }
     ; 

/* if loop */
ifstat : thenstat END              { $$ = Node("stat", "IF"); $$.children.push_back($1); }
       | thenstat ELSE block END   { $$ = Node("stat", "IF"); $$.children.push_back($1); $$.children.push_back($3); }
       ;

thenstat : IF exp THEN block              { $$ = Node("stat(then)", ""); $$.children.push_back($2); $$.children.push_back($4); }
         | thenstat ELSEIF exp THEN block { $$ = $1; $$.children.push_back($3); $$.children.push_back($5); }
         ;

laststat : RETURN explist { $$ = Node("laststat", "RETURN"); $$.children.push_back($2); }
/*       | RETURN {  } */
/*       | BREAK  {  } */
         ;
        
funcname : NAME                { $$ = Node("funcname", ""); $$.children.push_back(Node("NAME", $1)); }
         | funcname DOT NAME   { $$ = $1; $$.children.push_back(Node("NAME", $3)); }
         | funcname COLON NAME { $$ = $1; $$.children.push_back(Node("NAME", $3)); }
         ;

varlist : var               { $$ = Node("varlist", ""); $$.children.push_back($1); }
        | varlist COMMA var { $$ = $1; $$.children.push_back($3); }
        ;

var : NAME                            { $$ = Node("var", ""); $$.children.push_back(Node("NAME", $1)); }
    | prefixexp LBRACKET exp RBRACKET { $$ = Node("var", ""); $$.children.push_back($1); $$.children.push_back($3); }
    | prefixexp DOT NAME              { $$ = Node("var", ""); $$.children.push_back($1); $$.children.push_back(Node("NAME", $3)); }
    ;

namelist : NAME                { $$ = Node("namelist", ""); $$.children.push_back(Node("NAME", $1)); }
         | namelist COMMA NAME { $$ = $1; $$.children.push_back(Node("NAME", $3)); }
         ;

explist : exp               { $$ = Node("explist", ""); $$.children.push_back($1); }
        | explist COMMA exp { $$ = $1; $$.children.push_back($3); }
        ;

/*
Operator Priority

^                hign
not -(unary)
* / %
+ -
..
< > <= >= ~= ==
and
or               low
*/

exp : exp_or { $$ = $1; }
    ; 

exp_or : exp_and           { $$ = $1; }
       | exp_or OR exp_and { $$ = Node("exp", ""); $$.children.push_back($1); $$.children.push_back(Node("binop", $2)); $$.children.push_back($3); }
       ;

exp_and : exp_lge             { $$ = $1; }
        | exp_and AND exp_lge { $$ = Node("exp", ""); $$.children.push_back($1); $$.children.push_back(Node("binop", $2)); $$.children.push_back($3); }
        ;

exp_lge : exp_concat                     { $$ = $1; }
        | exp_lge LESSTHAN exp_concat    { $$ = Node("exp", ""); $$.children.push_back($1); $$.children.push_back(Node("binop", $2)); $$.children.push_back($3); }
        | exp_lge GREATTHAN exp_concat   { $$ = Node("exp", ""); $$.children.push_back($1); $$.children.push_back(Node("binop", $2)); $$.children.push_back($3); }
        | exp_lge LESSEQUALS exp_concat  { $$ = Node("exp", ""); $$.children.push_back($1); $$.children.push_back(Node("binop", $2)); $$.children.push_back($3); }
        | exp_lge GREATEQUALS exp_concat { $$ = Node("exp", ""); $$.children.push_back($1); $$.children.push_back(Node("binop", $2)); $$.children.push_back($3); }
        | exp_lge NOTEQUALS exp_concat   { $$ = Node("exp", ""); $$.children.push_back($1); $$.children.push_back(Node("binop", $2)); $$.children.push_back($3); }
        | exp_lge EQUALS exp_concat      { $$ = Node("exp", ""); $$.children.push_back($1); $$.children.push_back(Node("binop", $2)); $$.children.push_back($3); }
        ;

exp_concat : exp_as                   { $$ = $1; }
           | exp_concat CONCAT exp_as { $$ = Node("exp", ""); $$.children.push_back($1); $$.children.push_back(Node("binop", $2)); $$.children.push_back($3); }
           ;

exp_as : exp_md            { $$ = $1; }
       | exp_as ADD exp_md { $$ = Node("exp", ""); $$.children.push_back($1); $$.children.push_back(Node("binop", $2)); $$.children.push_back($3); }
       | exp_as SUB exp_md { $$ = Node("exp", ""); $$.children.push_back($1); $$.children.push_back(Node("binop", $2)); $$.children.push_back($3); }
       ;

exp_md : exp_unop            { $$ = $1; }
       | exp_md MUL exp_unop { $$ = Node("exp", ""); $$.children.push_back($1); $$.children.push_back(Node("binop", $2)); $$.children.push_back($3); }
       | exp_md DIV exp_unop { $$ = Node("exp", ""); $$.children.push_back($1); $$.children.push_back(Node("binop", $2)); $$.children.push_back($3); }
       | exp_md MOD exp_unop { $$ = Node("exp", ""); $$.children.push_back($1); $$.children.push_back(Node("binop", $2)); $$.children.push_back($3); }
       ;

exp_unop : exp_pow      { $$ = $1; }
         | unop exp_pow { $$ = Node("exp", ""); $$.children.push_back($1); $$.children.push_back($2); }
         ;

exp_pow : exp_rest             { $$ = $1; }
        | exp_pow POW exp_rest { $$ = Node("exp", ""); $$.children.push_back($1); $$.children.push_back(Node("binop", $2)); $$.children.push_back($3); }
        ;

exp_rest : NIL              { $$ = Node("exp", $1); }
         | FALSE            { $$ = Node("exp", $1); }
         | TRUE             { $$ = Node("exp", $1); }
         | NUMBER           { $$ = Node("num_exp", $1); }
         | STRING           { $$ = Node("exp", ""); $$.children.push_back(Node("STRING", $1)); }
         | TRIPLEDOTS       { $$ = Node("exp", ""); $$.children.push_back(Node("TRIPLEDOTS", $1)); }
         | function         { $$ = $1; }
         | prefixexp        { $$ = $1; }
         | tableconstructor { $$ = $1; }
         ;

prefixexp : var                         { $$ = $1; }
          | functioncall                { $$ = $1; }
          | LPARENTHESE exp RPARENTHESE { $$ = Node("prefixexp", ""); $$.children.push_back($2); }
          ;

functioncall : prefixexp args            { $$ = Node("functioncall", ""); $$.children.push_back($1); $$.children.push_back($2); }
             | prefixexp COLON NAME args { $$ = Node("functioncall", ""); $$.children.push_back($1); $$.children.push_back(Node("NAME", $3)); $$.children.push_back($4); }
             ;

args : LPARENTHESE RPARENTHESE         { /* empty */ }
     | LPARENTHESE explist RPARENTHESE { $$ = Node("args", ""); $$.children.push_back($2); }
     | tableconstructor                { $$ = $1; }
     | STRING                          { $$ = Node("args", ""); $$.children.push_back(Node("STRING", $1)); }
     ;

function : FUNCTION funcbody { $$ = Node("function", ""); $$.children.push_back($2); }
         ;

funcbody : LPARENTHESE RPARENTHESE block END         { $$ = Node("funcbody", ""); $$.children.push_back($3); }
         | LPARENTHESE parlist RPARENTHESE block END { $$ = Node("funcbody", ""); $$.children.push_back($2); $$.children.push_back($4); }
         ;

parlist : namelist                  { $$ = $1; }
        | namelist COMMA TRIPLEDOTS { $$ = Node("parlist", ""); $$.children.push_back($1); $$.children.push_back(Node("TRIPLEDOTS", $3)); }
        | TRIPLEDOTS                { $$ = Node("parlist", ""); $$.children.push_back(Node("TRIPLEDOTS", $1)); }
        ;

tableconstructor : LBRACE RBRACE           { /* empty */ }
                 | LBRACE fieldlist RBRACE { $$ = Node("tableconstructor", ""); $$.children.push_back($2); }
                 ;

fieldlist : field                    { $$ = Node("fieldlist", ""); $$.children.push_back($1); }
          | fieldlist fieldsep field { $$ = $1; $$.children.push_back($3); }
          | fieldlist fieldsep       { $$ = $1; }
          ;

field : LBRACKET exp RBRACKET ASSIGN exp { $$ = Node("field", ""); $$.children.push_back($2); $$.children.push_back(Node("ASSIGN", $4)); $$.children.push_back($5); }
      | NAME ASSIGN exp                  { $$ = Node("field", ""); $$.children.push_back(Node("NAME", $1)); $$.children.push_back(Node("ASSIGN", $2)); $$.children.push_back($3); }
      | exp                              { $$ = $1; }
      ;

fieldsep : COMMA { /* empty */ }
         | SEMI  { /* empty */ }
         ;

/* put binop into exp for precedences */

unop : SUB { $$ = Node("unop", $1); }
     | NOT { $$ = Node("unop", $1); }
     | LEN { $$ = Node("unop", $1); }
     ; 

%%