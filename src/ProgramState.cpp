#include "ProgramState.h"
#include "utils.h"
#include <sstream>
ProgramState::ProgramState(llvm::iterator_range<llvm::Function::arg_iterator> inputs)
{		 
	//s = new z3::solver(c);
	for (auto input = inputs.begin(), last = inputs.end(); input!=last; input++)
	{	 
		add(input,new ExpressionTree(input));
	}
	pathCondition = "";
}
ProgramState::ProgramState(const ProgramState & p)
{
	for (auto& pr : p.map)
	{
		add(pr.first,new ExpressionTree(*(pr.second)));
	}
	for (auto constraint : p.constraints)
	{
		this->constraints.push_back(constraint);
	}
	this->pathCondition = p.pathCondition;
}

std::string ProgramState::getPathCondition()
{
	return pathCondition;
}
void ProgramState::addCondition(std::string cond)
{
	if(pathCondition == "") pathCondition = cond;
	else pathCondition += " &&\n" + cond;
}

void ProgramState::add(llvm::Value* value, ExpressionTree* exp)
{
	map[value] = exp;
}

ExpressionTree* ProgramState::get(llvm::Value * s)
{
	if ( map.find(s) == map.end() ) return NULL;
	else return map[s];
}

std::map<llvm::Value*, ExpressionTree*> ProgramState::getMap()
{
	return map;
}

std::string ProgramState::toString()
{
	std::stringstream str;
	for (auto& pr : map)
	{
		str <<	getString(pr.first) << "\t == \t" << pr.second->toString(map) << '\n';
	}
	return str.str();
}

void ProgramState::printZ3Variables()
{
	for (auto& pr : map)
	{
		std::cout << getString(pr.first) << " === ";
		if (pr.second->top != NULL)
		{
			if (pr.second->top->left != NULL && pr.second->top->right != NULL)
			{
				std::string left = getString(pr.second->top->left->value);
				std::string right = getString(pr.second->top->right->value);
				variables.insert(std::make_pair(left, c.int_const(left.c_str())));
				variables.insert(std::make_pair(right, c.int_const(right.c_str())));

				if (pr.second->top->data == "+")
				{
					std::cout << variables.at(left) + variables.at(right) << '\n';
				}
				else if (pr.second->top->data == "*")
				{
					std::cout << variables.at(left) * variables.at(right) << '\n';
				}
				else if (pr.second->top->data == ">")
				{
					std::cout << "(>" << variables.at(left) << variables.at(right) << ")" << '\n';
				}
				else if (pr.second->top->data == "<")
				{
					std::cout << "(<" << variables.at(left) << variables.at(right) << ")" << '\n';
				}
				else 
				{
					std::cout << variables.at(left) << '\n';
				}
			}
			else if (pr.second->top->left == NULL && pr.second->top->right == NULL)
			{
				variables.insert(std::make_pair(getString(pr.second->top->value),
								c.int_const(getString(pr.second->top->value).c_str())));

				std::cout << variables.at(getString(pr.second->top->value)) << '\n';
			}
		}
	}
}

void ProgramState::Z3solver()
{ 
	z3::solver s(c);
	#ifdef DEBUG
		std::cout << "size of constraints = " << constraints.size() << "\n";
	#endif
	for (int i = 0; i < constraints.size(); i++)
	{
		std::cout << "i = " << i << "\n";
		ExpressionTree* exptree = get(constraints[i].first);
		if (exptree->top->left != NULL && exptree->top->right != NULL)
		{
			std::string left = getString(exptree->top->left->value);
			std::string right = getString(exptree->top->right->value);

			if (constraints[i].second == "true")
			{
				if(exptree->top->data == ">")
				{
					s.add(variables.at(left) > variables.at(right));
				}	
				else if(exptree->top->data == "<")
				{
					s.add(variables.at(left) < variables.at(right));
				}
			}
			else if (constraints[i].second == "false")
			{
				if(exptree->top->data == ">")
				{
					s.add(variables.at(left) <= variables.at(right));
				}
				else if(exptree->top->data == "<")
				{
					s.add(variables.at(left) >= variables.at(right)); 
				}
			}
		}
	}
	std::cout << this->getPathCondition();
	std::cout << s.check() << "\n";
	z3::model m = s.get_model();
	std::cout << m << std::endl;
}