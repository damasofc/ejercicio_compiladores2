#ifndef AST_H
#define AST_H

#include <iostream>
#include <list>
#include <memory>
#include <unordered_map>
#include <vector>

class ASTNode
{
private:
    /* data */
public:
    ASTNode(/* args */){};
    ~ASTNode(){};
    virtual int eval(std::unordered_map<std::string, int>& symbols) = 0;
    virtual std::string toString() = 0;
};

using PAstNode = *ASTNode;
using AstNodeList = std::list<PAstNode>;

#define Define_Bin_Expr(name, oper, prec,opS)\
    class name##Expr: public BinaryExpr \
    {\
    public:\
        name##Expr(PAstNode expr1,PAstNode expr2):\
        BinaryExpr(std::move(expr1),std::move(expr2)){};\
        \
        std::string toString() override{\
            return expr1->toString() + opS + expr2->toString() ;\
        }\
        int eval(std::unordered_map<std::string, int>& symbols) override{\
            return expr1->eval(symbols) oper expr2->eval(symbols);\
        }\
\
    };


class Statement: public ASTNode
{
private:
    /* data */
public:
    Statement(/* args */){}
};

class AssignStatement : public Statement
{
private:
    /* data */
public:
    AssignStatement(std::string lhs, const PAstNode rhs):
    lhs(lhs), rhs(std::move(rhs)){}

    std::string toString() override {
        return lhs + " = " + rhs->toString();
    }
    int eval(std::unordered_map<std::string, int>& symbols) 
    {
        symbols[lhs] = rhs->eval(symbols);
        return symbols[lhs];
    }

    std::string lhs;
    PAstNode rhs;
};

class BlockST : public Statement
{
public:
  std::vector<ASTNode*> stmts;
  BlockST() {}
  int eval(std::unordered_map<std::string, int>& symbols){
      for(int i = 0; i<stmts.size();i++){
        stmts[i]->eval(symbols);
    }
  }
  void addStmt(ASTNode* st){ stmts.push_back(st);}
};
class IfStatement : public Statement
{
private:
    /* data */
public:
    IfStatement(PAstNode cond, BlockST stmtsTrue, BlockST stmtsFalse):
    cond(cond), stmtsFalse(std::move(stmtsFalse)), stmtsTrue(stmtsTrue{}

    std::string toString() override {
        return lhs + " = " + rhs->toString();
    }
    int eval(std::unordered_map<std::string, int>& symbols) 
    {
        if(cond->eval(symbols))
        {
            for (size_t i = 0; i < stmtsTrue.size(); i++)
            {
                stmtsTrue[i].eval(symbols);
            }
            
        }
        else
        {
            for (size_t i = 0; i < stmtsTrue.size(); i++)
            {
                stmtsFalse[i].eval(symbols);
            }
        }
    }

    PAstNode cond;
    BlockST stmtsTrue;
    BlockST stmtsFalse;
};

class PrintStatement : public Statement
{
private:
    /* data */
public:
    PrintStatement(PAstNode expr):
    expr(std::move(expr)){}

    std::string toString() override {
        return "print (" + expr->toString() + ")";
    }

    int eval(std::unordered_map<std::string, int>& symbols) override {
        print(symbols);
        return expr->eval(symbols);
    }

    void print(std::unordered_map<std::string, int>& symbols)
    {
        std::cout<<expr->eval(symbols)<<std::endl;
    }


    PAstNode expr;
};

class Expr : public ASTNode
{
private:
    /* data */
public:
    Expr(){};
    ~Expr(){};
    // virtual int getPrec() = 0;
};


class BinaryExpr : public Expr
{
private:
    /* data */
public:
    BinaryExpr(PAstNode expr1,PAstNode expr2):
    expr1(std::move(expr1)), expr2(std::move(expr2)){}
    ~BinaryExpr(){}
    PAstNode expr1;
    PAstNode expr2;

};

Define_Bin_Expr(Add,+,0,"+");
Define_Bin_Expr(Sub,-,0,"-");
Define_Bin_Expr(Mul,*,1,"*");
Define_Bin_Expr(Div,/,1,"/");


class Ident: public Expr
{
private:
    /* data */
public:
    Ident(std::string name):name(name){};

    std::string toString() override {return name;};
    int eval(std::unordered_map<std::string, int>& symbols) override {
        if(symbols.find(name) != symbols.end()){
            return symbols[name];
        }
        else
        {
            std::cout<<"\033[1;31mERROR:\033[0m Ident: "<<name<< " not declared\n";
            throw "error";
        }
        
    }
    std::string name;
};

class Number: public Expr
{
private:
    /* data */
public:
    Number(int value): value(value){};

    std::string toString() override {return std::to_string(value);}
    int eval(std::unordered_map<std::string, int>& symbols)override{
        return value;
    }
    int value;
};

#endif