#include "llvmExpressionTree.h"
#include "utils.h"
#include <sstream>
#include <stdexcept>
#include <llvm/Support/Casting.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/LLVMContext.h>


ExpressionTreeNode::ExpressionTreeNode(std::string data, llvm::Value* value)
{
    this->data = data;
    this->value = value;
    this->left = NULL;
    this->right = NULL;
}
/*ExpressionTreeNode::ExpressionTreeNode(const ExpressionTreeNode & n)
{
    this->data = n.data;
    this->value = n.value;
    this->left = n.left;
    this->right = n.right;
}*/
bool ExpressionTree::isConstant(llvm::Value* value)
{
    return value && llvm::isa<llvm::Constant>(value); 
}


ExpressionTree::ExpressionTree(llvm::Value* value)
{
    this->top = new ExpressionTreeNode(ExpressionTreeNode("", value));
}

/*ExpressionTree::ExpressionTree(const ExpressionTree & e)
{
    top = new ExpressionTreeNode(ExpressionTreeNode(*(e.top)));
}*/

bool ExpressionTree::isConstant()
{
    return isConstant(this->top->value);
}

ExpressionTree::ExpressionTree(std::string op, ExpressionTree* lhs, ExpressionTree* rhs)
{

    if (lhs->isConstant() && rhs->isConstant()) 
    {
        this->top = new ExpressionTreeNode(ExpressionTreeNode("",evaluate(lhs->top->value, rhs->top->value, op)));
    }
    else if (op == "+" && rhs->isConstant() && rhs->getInteger() == 0)
    {
        this->top = new ExpressionTreeNode(ExpressionTreeNode("",lhs->top->value));
        
    }
    else if (op == "+" && lhs->isConstant() && lhs->getInteger() == 0)
    {
    	this->top = new ExpressionTreeNode(ExpressionTreeNode("",rhs->top->value));
    }
    else
    {
        this->top = new ExpressionTreeNode(ExpressionTreeNode(op, NULL));
        this->top->left = lhs->top;
        this->top->right = rhs->top;
    }

}

ExpressionTree::ExpressionTree(std::string op, llvm::Value* lhs, llvm::Value* rhs)
{
    if (isConstant(lhs) && isConstant(rhs)) 
    {
        this->top = new ExpressionTreeNode(ExpressionTreeNode("",evaluate(lhs, rhs, op)));
    }
    else if (op == "+" && isConstant(rhs))
    {
        if (::getInteger(rhs) == 0) this->top = new ExpressionTreeNode(ExpressionTreeNode("",lhs));
    }
    else
    {   
        this->top = new ExpressionTreeNode(  ExpressionTreeNode(op, NULL));
        this->top->left = new ExpressionTreeNode(  ExpressionTreeNode("",lhs));
        this->top->right = new ExpressionTreeNode(  ExpressionTreeNode("",rhs));
    }

}

int ExpressionTree::getInteger()
{
    if(this->isConstant()) return ::getInteger(this->top->value);
    else throw std::invalid_argument("not a constant");

}



llvm::Value* ExpressionTree::evaluate(llvm::Value* lhs, llvm::Value* rhs, std::string op)
{
        int result = 0;
        int lhsInt = ::getInteger(lhs);
        int rhsInt = ::getInteger(rhs);
        if (op == "+")
            result = lhsInt + rhsInt;
        else if (op == ">")
            result = lhsInt > rhsInt;
        llvm::Value* ans = llvm::ConstantInt::get( llvm::getGlobalContext() , llvm::APInt(64, result, false));
        return ans;
}


std::string ExpressionTree::toString(std::map<llvm::Value*, ExpressionTree*> table)
{
    std::stringstream toReturn;
    getExpressionString(this->top, table, toReturn);
    return toReturn.str();
}

void ExpressionTree::getExpressionString(ExpressionTreeNode* node,
                                         std::map<llvm::Value*, ExpressionTree*> table, 
                                         std::stringstream& toReturn)
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

int ExpressionTree::compare(int value)
{
    if(isConstant(this->top->value))
    {
        int myval = this->getInteger();
        if(value > myval) return 1;
        else if(value < myval) return -1;
        else return 0;
    }
    else return -2;
}

int ExpressionTree::compare(ExpressionTree* exp1)
{
    if (isConstant(exp1->top->value))
    {
        return compare(exp1->getInteger());
    }
    else return -2;

}
