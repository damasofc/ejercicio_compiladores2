%code requires{
    #include "expr_ast.h"
    typedef ASTNode* YYSTYPE;
    #define YYSTYPE_IS_DECLARED 1
}

%{
    #include <iostream>
    #include <unordered_map>
    extern int yylex();
    extern int yylineno;
    extern FILE* yyin;
    int errors;
    std::unordered_map<std::string, int> symbols;
    void yyerror(const char *msg){
        std::cout<<"Error en linea: "<<yylineno<<" -> "<<msg<<std::endl;
        errors++;
    }

    #define YYERROR_VERBOSE 1


%}

%token Num "num"
%token opAdd "+"
%token opSub "-"
%token opMul "*"
%token opDiv "/"
%token opEqual "=="
%token opAssign "="
%token Kw_if "if"
%token Kw_else "else"
%token OpenParen "("
%token CloseParen ")"
%token OpenCorchetes "{"
%token CloseCorchetes "}"
%token Kw_Print "print"
%token ID "id"
%token ERROR "error"
%token SemiColon ";"


%%

prog: stmts {((Expr*)$1)->eval(symbols);}
    ;

stmts: stmt {$$ = new BlockST(); ((BlockST*)$$)->stmts.push_back((Statement*)$1);}
    | stmts stmt {$$ = $1; ((BlockST*)$$)->stmts.push_back(((Statement*)$2)); }
    ;

stmt: assign {$$ = $1;}
    | print {$$ = $1;}
    | if_else {$$ = $1;}
    ;

assign: "id" "=" expr ";" {$$ = new AssignStatement((Expr*)$1,(Expr*)$3);}
    ;

print: "print" expr ";" {$$ = new PrintStatement((Expr*)$2); }
    ;

if_else: "if" "(" expr ")" "{" stmts "}" "else" "{" stmts "}" {$$ = new IfStatement((Expr*)$3,(BlockST*)$6,(BlockST*)$10);}
    ;

expr: expr "+" term {$$ = new AddExpr((Expr*)$1,(Expr*)$3);}
    | expr "-" term {$$ = new SubExpr((Expr*)$1,(Expr*)$3);}
    | term {$$ = $1;}
    ;

term: term "*" factor {$$ = new MulExpr((Expr*)$1,(Expr*)$3);}
    | term "/" factor {$$ = new DivExpr((Expr*)$1,(Expr*)$3);}
    | factor {$$ = $1;}
    ;

factor: "id" {$$ = $1;}
    | "num" {$$ = $1;}
    | "(" expr ")" {$$ = $2;}
    ;

%%

//AGREGARLE CONDICONALES AND OR Y NOP

int main(int argc, char const *argv[])
{
    yyin = fopen(argv[1],"rb");
    if (!yyin)
    {
        std::cout<<"Cannot open file!!\n";
        return 1;
    }
    
    try
    {
        yyparse();
        if(errors > 0)
        {
            std::cout<<"Se encontraron errores en el parse\n";
        }
        else{
            std::cout<<"Parse succes\n";

        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }


    return 0;
}