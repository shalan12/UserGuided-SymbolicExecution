#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <stdexcept>
#include "utils.cpp"
#include <llvm/Support/Casting.h>
#include <llvm/IR/Constants.h>
 
class ExpressionTreeNode
{
    public:
        std::string data;
        llvm::Value* value;
        ExpressionTreeNode* left;
        ExpressionTreeNode* right;
        ExpressionTreeNode(std::string data, llvm::Value* value)
        {
            this->data = data;
            this->value = value;
            this->left = NULL;
            this->right = NULL;
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
 
    ExpressionTreeNode* top = NULL;
    ExpressionTree(){}
    ExpressionTree(llvm::Value* value)
    {
        this->top = new ExpressionTreeNode("", value);
    }
    ExpressionTreeNode* getTop()
    {
        return this->top;
    }
    ExpressionTree(std::string op, ExpressionTree* lhs, ExpressionTree* rhs)
    {
        if (isConstant(lhs->getTop()->value) && isConstant(rhs->getTop()->value)) 
            this->top = new ExpressionTreeNode("",evaluate(lhs->getTop()->value, rhs->getTop()->value, op));
        else if (op == "+" && isConstant(rhs->getTop()->value) && getInteger(rhs->getTop()->value) == 0)
        {
            this->top = new ExpressionTreeNode("",lhs->getTop()->value);
            
        }
        else if (op == "+" && isConstant(lhs->getTop()->value) && getInteger(lhs->getTop()->value) == 0)
        {
        	this->top = new ExpressionTreeNode("",rhs->getTop()->value);
        }
        else
        {
            this->top = new ExpressionTreeNode(op, NULL);
            this->top->left = lhs->getTop();
            this->top->right = rhs->getTop();
        }
 
    }
    ExpressionTree(std::string op, llvm::Value* lhs, llvm::Value* rhs)
    {
        //temporary
        if (isConstant(lhs) && isConstant(rhs)) 
            this->top = new ExpressionTreeNode("",evaluate(lhs, rhs, op));
        else if (op == "+" && isConstant(rhs))
        {
            if (getInteger(rhs) == 0)
                this->top = new ExpressionTreeNode("",lhs);
        }
        else
        {
            this->top = new ExpressionTreeNode(op, NULL);
            this->top->left = new ExpressionTreeNode("",lhs);
            this->top->right = new ExpressionTreeNode("",rhs);
        }
 
    }
 	
 	bool isConstant()
    {
        return isConstant(getTop()->value);
    }

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
 
    void getExpressionString(ExpressionTreeNode* node, std::map<llvm::Value*, ExpressionTree*> table, std::stringstream& toReturn)
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