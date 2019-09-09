%option noyywrap

%{
    #include "Tokens.h"
    #include <stdio.h>
    #include "expr_ast.h"
%}

digit [0-9]
%x BLOQCOMMENT

%%

[ \t\r] {;}
[0-9]+ { yylval = new Number(std::stoi(yytext));return Num; }
"+" {return opAdd; }
"-" {return opSub; }
"*" {return opMul; }
"/*" {BEGIN(BLOQCOMMENT);}
"/" {return opDiv; }
"=" {return opAssign; }
"==" {return opEqual; }
">" {return opGreater; }
">=" {return opGreaterEqual; }
"<" {return opLess; }
"<=" {return opLessEqual; }
"!=" {return opNEQ; }
"(" {return OpenParen; }
")" {return CloseParen; }
"{" {return OpenCorchetes; }
"}" {return CloseCorchetes; }
";" {return SemiColon; }
"if" {return Kw_if; }
"else" {return Kw_else; }
"print" { return Kw_Print; }
[A-Za-z]+ { yylval = new Ident(yytext);return ID; }
\n { yylineno++;}
. { return ERROR; }

<BLOQCOMMENT>"*/" {BEGIN(INITIAL);}
<BLOQCOMMENT>\n {yylineno++;}
<BLOQCOMMENT>. {;}
<BLOQCOMMENT><<EOF>> {throw "Error bloque de comentario no cerrado";}

%%