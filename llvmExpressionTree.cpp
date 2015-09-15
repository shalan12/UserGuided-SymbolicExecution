#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <memory>
#include <stdexcept>
#include "utils.cpp"
#include <llvm/Support/Casting.h>
#include <llvm/IR/Constants.h>
 
class ExpressionTreeNode
{
    public:
        std::string data;
        // the memory addresses pointed to by value come from llvm's representation of the SUT
        // => they are here to stay as long as the program runs.
        // => we're not responsible for deleting it and we're guranteed not to have a dangling pointer
        // => we can just keep a raw pointer
        llvm::Value* value;
        // a node owns it's children uniquely. No other ptr will ever point to them
        std::shared_ptr<ExpressionTreeNode> left;
        std::shared_ptr<ExpressionTreeNode> right;
        ExpressionTreeNode(std::string data, llvm::Value* value)
        {
            this->data = data;
            this->value = value;
        }
};
class ExpressionTree
{
private:
    bool isConstant(llvm::Value* value)
    {
        return llvm::isa<llvm::Constant>(value); 
    }   
public:
 
    std::shared_ptr<ExpressionTreeNode> top;
    ExpressionTree(){}
    ExpressionTree(llvm::Value* value)
    {
        this->top = std::make_shared<ExpressionTreeNode>(ExpressionTreeNode("", value));
    }
    
    bool isConstant()
    {
        return isConstant(this->top->value);
    }

    ExpressionTree(std::string op, ExpressionTree* lhs, ExpressionTree* rhs)
    {
        if (isConstant(lhs->top->value) && isConstant(rhs->top->value)) 
        {
            this->top = std::make_shared<ExpressionTreeNode>(ExpressionTreeNode("",evaluate(lhs->top->value, rhs->top->value, op)));
        }
        else if (op == "+" && isConstant(rhs->top->value) && getInteger(rhs->top->value) == 0)
        {
            this->top = std::make_shared<ExpressionTreeNode>(ExpressionTreeNode("",lhs->top->value));
            
        }
        else if (op == "+" && isConstant(lhs->top->value) && getInteger(lhs->top->value) == 0)
        {
        	this->top = std::make_shared<ExpressionTreeNode>(ExpressionTreeNode("",rhs->top->value));
        }
        else
        {
            this->top = std::make_shared<ExpressionTreeNode>(ExpressionTreeNode(op, NULL));
            this->top->left = lhs->top;
            this->top->right = rhs->top;
        }
 
    }
    ExpressionTree(std::string op, llvm::Value* lhs, llvm::Value* rhs)
    {
        if (isConstant(lhs) && isConstant(rhs)) 
        {
            this->top = std::make_shared<ExpressionTreeNode>(ExpressionTreeNode("",evaluate(lhs, rhs, op)));
        }
        else if (op == "+" && isConstant(rhs))
        {
            if (getInteger(rhs) == 0) this->top = std::make_shared<ExpressionTreeNode>(ExpressionTreeNode("",lhs));
        }
        else
        {   
            this->top = std::make_shared<ExpressionTreeNode>(  ExpressionTreeNode(op, NULL));
            this->top->left = std::make_shared<ExpressionTreeNode>(  ExpressionTreeNode("",lhs));
            this->top->right = std::make_shared<ExpressionTreeNode>(  ExpressionTreeNode("",rhs));
        }
 
    }

    int getInteger()
    {
        if(this->isConstant()) return getInteger(this->top->value);
        else throw std::invalid_argument("not a constant");

    }
    // should be private and static
    int getInteger(llvm::Value* value)
    {
        if (llvm::ConstantInt* cl = llvm::dyn_cast<llvm::ConstantInt>(value))
        {
        	return cl->getSExtValue();
        }
         
        // to through execption, please do something to catch this exception 
        // while calling this function otherwise it will cause to crash the prog.
         
        else throw std::invalid_argument("not a constant");
         
        // else return -1;
    }
 
 
    llvm::Value* evaluate(llvm::Value* lhs, llvm::Value* rhs, std::string op)
    {
            int result = 0;
            int lhsInt = getInteger(lhs);
            int rhsInt = getInteger(rhs);
            if (op == "+")
                result = lhsInt + rhsInt;
            else if (op == ">")
                result = lhsInt > rhsInt;
            llvm::Value* ans = llvm::ConstantInt::get( llvm::getGlobalContext() , llvm::APInt(64, result, false));
            return ans;
    }
 
 
    std::string toString(std::map<llvm::Value*, ExpressionTree*> table)
    {
        std::stringstream toReturn;
        getExpressionString(this->top, table, toReturn);
        return toReturn.str();
    }
 
    void getExpressionString(std::shared_ptr<ExpressionTreeNode> node, std::map<llvm::Value*, ExpressionTree*> table, std::stringstream& toReturn)
    {
        if (node != NULL)
        {
            if (node->left != NULL)
            {
 
                toReturn << "(";
                if (isConstant(node->left->value))
                    toReturn << getString(node->left->value);
                else
                {
                    ExpressionTree* left = table[node->left->value];
                    getExpressionString(left->top, table, toReturn);
                }   
            }
            if (node->left == NULL && node->right == NULL)
                toReturn << getString(node->value);
            else toReturn << node->data;
            if (node->right != NULL)
            {
                if (isConstant(node->right->value))
                    toReturn << getString(node->right->value);
                else
                {
                    ExpressionTree* right = table[node->right->value];
                    getExpressionString(right->top, table, toReturn);
                    toReturn << ")";
                }   
            }
             
        }
    }
    
    int compare(int value)
    {
        if(isConstant(this->top->value))
        {
            int myval = getInteger(this->top->value);
            if(value > myval) return 1;
            else if(value < myval) return -1;
            else return 0;
        }
        else return -2;
    }
 
    int compare(ExpressionTree* exp1)
    {
        if (isConstant(exp1->top->value))
        {
            return (compare(getInteger(exp1->top->value)));
        }
        else return -2;
 
    }
 
};