#include "llvmExpressionTree.h"
#include "utils.h"
#include <sstream>
#include <stdexcept>
#include <llvm/Support/Casting.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/LLVMContext.h>


ExpressionTreeNode::ExpressionTreeNode(std::string data = "", llvm::Value* value = NULL)
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

ExpressionTree::ExpressionTree(llvm::Value* value, std::map<std::string, llvm::Value*> userVarMap, std::map<llvm::Value*, std::string> llvmVarMap)
{
    this->top = new ExpressionTreeNode(ExpressionTreeNode("", value));
    this->userVarMap = userVarMap;
    this->llvmVarMap = llvmVarMap;
}

bool ExpressionTree::isConstant()
{
    return ::isConstant(this->top->value);
}

ExpressionTree::ExpressionTree(std::string op, ExpressionTree* lhs, ExpressionTree* rhs)
{

    if (lhs->isConstant() && rhs->isConstant() && (op == "+" || op == "-")) 
    {
        this->top = new ExpressionTreeNode(ExpressionTreeNode("",evaluate(lhs->top->value, rhs->top->value, op)));
    }
    else if ((op == "+" || op == "-" ) && rhs->isConstant() && rhs->getInteger() == 0)
    {
        this->top = new ExpressionTreeNode(ExpressionTreeNode("",lhs->top->value));
        
    }
    else if ((op == "+") && lhs->isConstant() && lhs->getInteger() == 0)
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
    if (::isConstant(lhs) && ::isConstant(rhs) && (op == "+" || op == "-")) 
    {
        this->top = new ExpressionTreeNode(ExpressionTreeNode("",evaluate(lhs, rhs, op)));
    }
    else if ((op == "+" || op == "-") && ::isConstant(rhs))
    {
        if (::getInteger(rhs) == 0) this->top = new ExpressionTreeNode(ExpressionTreeNode("",lhs));
    }
    else if (op == "+" && ::isConstant(lhs) && ::getInteger(lhs) == 0)
    {
        this->top = new ExpressionTreeNode(ExpressionTreeNode("",rhs));
    }
    else
    {   
        this->top = new ExpressionTreeNode(  ExpressionTreeNode(op, NULL));
        this->top->left = new ExpressionTreeNode(  ExpressionTreeNode("",lhs));
        this->top->right = new ExpressionTreeNode(  ExpressionTreeNode("",rhs));
    }

}

void ExpressionTree::constructTree(std::stringstream & iss, ExpressionTreeNode* node)
{
    if (!iss)
        return;
    std::string curr;
    iss >> curr;
    if (curr == "+" || curr == "-" || curr == "/" || curr == "*")
    {
        node->data = curr;
        node->left = new ExpressionTreeNode();
        node->right = new ExpressionTreeNode();
        // set left and recurse
        constructTree(iss, node->left);
        constructTree(iss, node->right);
    }
    else
    {
        //set value using map in state.
        ExpressionTreeNode * temp_top = map[userVarMap[curr]]->top;
        node->value = temp_top->value;
        node->left = temp_top->left;
        node->right = temp_top->right;
        node->data = temp_top->data;
    }
}

ExpressionTree::ExpressionTree(std::string str, std::map<std::string, llvm::Value*> userVarMap, std::map<llvm::Value*, std::string> llvmVarMap,
    std::map<llvm::Value*, ExpressionTree*> map)
{
    std::stringstream iss(str);
    this->userVarMap = userVarMap;
    this->llvmVarMap = llvmVarMap;
    this->map = map;
    top = new ExpressionTreeNode("", NULL);
    constructTree(iss, top);   
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
        else if (op == "-")
            result = lhsInt - rhsInt; 
        llvm::Value* ans = llvm::ConstantInt::get( llvm::getGlobalContext() , llvm::APInt(32, result, false));
        return ans;
}


std::string ExpressionTree::toString()
{
    std::stringstream toReturn;
    getExpressionString(this->top, toReturn);
    return toReturn.str();
}

std::string ExpressionTree::toStringHumanReadable(std::map<llvm::Value*, std::string> varMap
    , std::map<llvm::Value*, llvm::Value*> store)
{
    std::stringstream toReturn;
    getExpressionStringHumanReadable(this->top, toReturn, varMap, store);
    return toReturn.str();
}

void ExpressionTree::getExpressionStringHumanReadable(ExpressionTreeNode* node, std::stringstream& toReturn, 
    std::map<llvm::Value*, std::string> varMap, std::map<llvm::Value*, llvm::Value*> store)
{
    if (node != NULL)
    {
        if (node->left == NULL && node->right == NULL)
        {

            if (llvm::isa<llvm::Constant>(node->value))
            {
                toReturn << ::getInteger(node->value);   
            }
            else toReturn << varMap[store[node->value]];
        }
        else
        {
            if(node->left != NULL)
            {
                toReturn << "(";
                getExpressionStringHumanReadable(node->left, toReturn, varMap, store);
                
            }
            toReturn << node->data;
            if(node->right != NULL)
            {
                getExpressionStringHumanReadable(node->right, toReturn, varMap, store);
                toReturn << ")";
            }
        }
    }
}

void ExpressionTree::getExpressionString(ExpressionTreeNode* node, std::stringstream& toReturn)
{
    if (node != NULL)
    {
        if (node->left == NULL && node->right == NULL)
        { 
            toReturn << getString(node->value);
        }
        else
        {
            if(node->left != NULL)
            {
                toReturn << "(";
                getExpressionString(node->left, toReturn);
                
            }
            toReturn << node->data;
            if(node->right != NULL)
            {
                getExpressionString(node->right, toReturn);
                toReturn << ")";
            }
        }
    }
}

z3::expr* ExpressionTree::toZ3Expression(std::map<llvm::Value*, z3::expr*>& z3Map, z3::context& c)
{
    return getZ3Expression(this->top, z3Map, c);
    
}

void ExpressionTree::addZ3ExpressionToMap(llvm::Value* value, std::map<llvm::Value*, 
                z3::expr*>& z3Map, z3::context& context)
{
    int xyz;
    std::string str = getString(value);
    if (::isConstant(value))
    {
        int val = ::getInteger(value);
        str = std::to_string(val);
        z3::expr * z3Val = new z3::expr(context);
        #ifdef DEBUG
            std::cout << "real value : " << str << "\n";
            std::cin >> xyz;
        #endif

        *z3Val = context.int_val(val);
        #ifdef DEBUG
            std::cout << "adding expression to MAP : " << *z3Val << "\n";
            std::cin >> xyz;
        #endif
        z3Map.insert(std::make_pair(value, z3Val)); 
    }
    else
    {
        z3::expr * z3Val = new z3::expr(context);
        *z3Val = context.int_const(str.c_str());
        #ifdef DEBUG
            std::cout << "adding expression to MAP : " << *z3Val << "\n";
            std::cin >> xyz;
        #endif
        z3Map.insert(std::make_pair(value, z3Val)); 
    }
}


z3::expr* ExpressionTree::getZ3Expression(ExpressionTreeNode* node, 
            std::map<llvm::Value*, z3::expr*>& z3Map, z3::context& context)
{
    if (node != NULL)
    {
        if (node->left == NULL && node->right == NULL)
        {
            std::string str = getString(node->value);
            if (z3Map.find(node->value) == z3Map.end())
            {
                addZ3ExpressionToMap(node->value, z3Map, context);
            }
            return z3Map[node->value];
         
        }
        else if (node->left != NULL && node->right != NULL)
        {
            int xyz;
            z3::expr * constraint = new z3::expr(context);
            z3::expr * left = getZ3Expression(node->left, z3Map, context);
            z3::expr * right = getZ3Expression(node->right, z3Map, context);
            
            #ifdef DEBUG
                if (!left)
                {
                    std::cout << "left NULL!\n";
                    std::cin >> xyz;
                }
                else
                {
                    std::cout << "left not NULL!" << *left << "\n";
                    std::cin >> xyz;
                }

                if (!right)
                {
                    std::cout << "right NULL!\n";
                    std::cin >> xyz; 
                }
                else
                {
                    std::cout << "right not NULL!" << *right << "\n";
                    std::cin >> xyz;
                }
            #endif

            std::string op = node->data;
            if (op == "+")
                *constraint = *left + *right;
            else if (op == "-")
                *constraint = *left - *right;
            else if (op == "*")
                *constraint = *left * *right;
            else if (op == "/")
                *constraint = *left / *right;
            else if (op == ">")
                *constraint = *left > *right;
            else if (op == "<")
                *constraint = *left < *right;
            else if (op == ">=")
                *constraint = *left >= *right;
            else if (op == "<=")
                *constraint = *left <= *right;
            else if (op == "==")
                *constraint = *left == *right;
            else if (op == "!=")
                *constraint = *left != *right;
            else if (op == "%")
                *constraint = *left - (*left / *right * *right);
            return constraint;
        }
    }
}

int ExpressionTree::compare(int value)
{
    if(::isConstant(this->top->value))
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
    if (::isConstant(exp1->top->value))
    {
        return compare(exp1->getInteger());
    }
    else return -2;

}
