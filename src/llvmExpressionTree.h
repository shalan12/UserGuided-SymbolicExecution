#ifndef LLVM_EXPRESSION_TREE
#define LLVM_EXPRESSION_TREE
#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <llvm/IR/Value.h>


class ExpressionTreeNode
{
    public:
        std::string data;
        // the memory addresses pointed to by value come from llvm's representation of the SUT
        // => they are here to stay as long as the program runs.
        // => we're not responsible for deleting it and we're guranteed not to have a dangling pointer
        // => we can just keep a raw pointer
        llvm::Value* value;
        ExpressionTreeNode* left;
        ExpressionTreeNode* right;
        ExpressionTreeNode(std::string data, llvm::Value* value);
        //ExpressionTreeNode(const ExpressionTreeNode & n);
};

class ExpressionTree
{
private:
    bool isConstant(llvm::Value* value);
    void getExpressionString(ExpressionTreeNode* node, std::map<llvm::Value*, ExpressionTree*> table, std::stringstream& toReturn);

public:
 
    ExpressionTreeNode* top;
    ExpressionTree(){}
    ExpressionTree(llvm::Value* value);
    //ExpressionTree(const ExpressionTree & e);
    ExpressionTree(std::string op, ExpressionTree* lhs, ExpressionTree* rhs);
    ExpressionTree(std::string op, llvm::Value* lhs, llvm::Value* rhs);
    bool isConstant();
    int getInteger();
    llvm::Value* evaluate(llvm::Value* lhs, llvm::Value* rhs, std::string op);
    std::string toString(std::map<llvm::Value*, ExpressionTree*> table);
    int compare(int value);
    int compare(ExpressionTree* exp1);
 
};
#endif