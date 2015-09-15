#ifndef EXPRESSIONTREE_H
#define EXPRESSIONTREE_H

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
		ExpressionTree(Instruction instr, string e);
		string toString(map<string, ExpressionTree*> table);
		string evaluate(string lhs, string rhs, char oper);
		bool isConstant(string str);
		void getExpressionString(ExpressionTreeNode* node, map<string, ExpressionTree*> table, stringstream& toReturn);
		int compare(ExpressionTree* exp1);
		Instruction type;
		ExpressionTreeNode* top = NULL;
};

#endif