#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>


using namespace std;
class ExpressionTreeNode
{
	public:
		string data;
		ExpressionTreeNode* left;
		ExpressionTreeNode* right;
		ExpressionTreeNode(string data)
		{
			this->data = data;
			this->left = NULL;
			this->right = NULL;
		}
};
class ExpressionTree
{
public:
	enum class Instruction
	{
		ADD, MUL, SUB
	};
	Instruction type;
	ExpressionTreeNode* top = NULL;
	ExpressionTree(Instruction instr, string e)
	{
		this->type = instr;
		char op ;
		if (instr == ExpressionTree::Instruction::ADD) op = '+';
		else if (instr == ExpressionTree::Instruction::SUB) op = '-';
		else if (instr == ExpressionTree::Instruction::MUL) op = '*';	
		string lhs = e.substr(0, e.find(','));
		string rhs = e.substr(e.find(',')+1, e.length());
		if (isConstant(lhs) && isConstant(rhs)) 
			this->top = new ExpressionTreeNode(evaluate(lhs, rhs, op));
		else if (op == '*' && isConstant(rhs))
		{
			if (stoi(rhs) == 1)
				this->top = new ExpressionTreeNode(lhs);
		}	
		else if (op == '+' && isConstant(rhs))
		{
			if (stoi(rhs) == 1)
				this->top = new ExpressionTreeNode(lhs);
		}
		else 
		{
			string s;
			stringstream ss;
			ss << op;
			ss >> s;
			this->top = new ExpressionTreeNode(s);
			this->top->left = new ExpressionTreeNode(lhs);
			this->top->right = new ExpressionTreeNode(rhs);
		}

	}


	string toString(map<string, ExpressionTree*> table)
	{
		stringstream toReturn;
		getExpressionString(this->top, table, toReturn);
		return toReturn.str();
	}
	void getExpressionString(ExpressionTreeNode* node, map<string, ExpressionTree*> table, stringstream& toReturn)
	{
		if (node != NULL)
		{
			if (node->left != NULL)
			{

				toReturn << "(";
				if (isConstant(node->left->data))
					toReturn << node->left->data;
				else
				{
					ExpressionTree* left = table[node->left->data];
					getExpressionString(left->top, table, toReturn);
				}	
			}
			toReturn << node->data;
			if (node->right != NULL)
			{
				if (isConstant(node->right->data))
					toReturn << node->right->data;
				else
				{
					ExpressionTree* right = table[node->right->data];
					getExpressionString(right->top, table, toReturn);
					toReturn << ")";
				}	
			}
			
		}
	}

	bool isConstant(string str)
	{
		return str.find_first_not_of("0123456789") == std::string::npos;
	}

	string evaluate(string lhs, string rhs, char op)
	{
			int result = 0;
			switch(op)
			{
				case '+':
					result = stoi(lhs) + stoi(rhs);
					break;	
				case '-':
					result = stoi(lhs) - stoi(rhs);
					break;
				case '*':
					result = stoi(lhs) * stoi(rhs);
					break;
			}
			return to_string(result);
	}

	int compare(ExpressionTree* exp1)
	{
		if (isConstant(exp1->top->data) && isConstant(this->top->data))
		{
			if (stoi(this->top->data) > stoi(exp1->top->data))
				return 1;
			else if (stoi(this->top->data) < stoi(exp1->top->data))
				return -1;
			else return 0;
		}
		else return -2;

	}

};
int main()
{
	map<string, ExpressionTree*> table; 
	ExpressionTree* x = new ExpressionTree(ExpressionTree::Instruction::ADD, "4,0");
	ExpressionTree* y = new ExpressionTree(ExpressionTree::Instruction::SUB, "30,20");
	table["x"] = x;
	table["y"] = y;
	ExpressionTree* a = new ExpressionTree(ExpressionTree::Instruction::ADD, "x,y");
	ExpressionTree* b = new ExpressionTree(ExpressionTree::Instruction::MUL, "x,a");
	ExpressionTree* c = new ExpressionTree(ExpressionTree::Instruction::MUL, "a,b");
	table["a"] = a;
	table["b"] = b;
	cout << c->toString(table) << endl;
	cout << x->compare(y) << endl;

		
}