#include "ProgramState.h"
#include "utils.h"
#include <sstream>
#include <vector>
#include <llvm/ADT/iterator_range.h>
ProgramState::ProgramState(llvm::iterator_range<llvm::Function::arg_iterator> inputs, 
	std::vector<ExpressionTree*> arguments)
{		 
	//s = new z3::solver(c);
	int i = 0;
	for (auto input = inputs.begin(), last = inputs.end(); input!=last; input++)
	{	
		if(i >= arguments.size())
			add(input,new ExpressionTree(input));
		else 
			add(input,arguments[i++]);
		std::cout << "program state constructor, function parameters : " << getString(input) << "\n";
	}
	pathCondition = "";
}
void ProgramState::Copy(const ProgramState& from, ProgramState* to, bool copyMap = true)
{
	if(copyMap)
	{
		for (auto& pr : from.map)
		{
			to->add(pr.first, new ExpressionTree(*pr.second));
		}
		for (auto& pr : from.userVarMap)
		{
			to->userVarMap[pr.first] = pr.second;
		}
	}
	for (auto constraint : from.z3Constraints)
	{
		to->z3Constraints.push_back(constraint);
	}
	to->pathCondition = from.pathCondition;
}

ProgramState::ProgramState(const ProgramState & p)
{
	ProgramState::Copy(p,this);
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

void ProgramState::addUserVar(std::string varname, llvm::Value* val)
{
	// std::cout << "adding this \n";
	// int x;
	// std::cin >> x;
	// std::cout << "adding this expression tree to user vars:  \n " << varname << "  :  " << exp->toString();
	userVarMap[varname] = val;
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
	/*for (auto& pr : map)
	{
		str <<	getString(pr.first) << "\t == \t" << pr.second->toString() << '\n';
	}*/
	/*str << "\n user variables: \n";*/

	for (auto& pr : userVarMap)
	{
		if (get(pr.second))
			str << pr.first <<	"\t = \t" << map[pr.second]->toString() << '\n';
		else
			str << pr.first <<	"\t = \t" << getString(pr.second) << '\n';
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

std::map<std::string, llvm::Value*> ProgramState::getUserVarMap()
{
	return userVarMap;
}

bool ProgramState::Z3solver()
{ 
	z3::solver s(c);
	#ifdef DEBUG
		std::cout << "size of constraints = " << z3Constraints.size() << "\n";
	#endif
	for (int i = 0; i < z3Constraints.size(); i++)
	{
		std::cout << "i = " << i << "\n";
		if(z3Constraints[i].second == "true")
		{
			s.add(*z3Constraints[i].first);
		}
		else if(z3Constraints[i].second == "false")
		{
			s.add(!(*z3Constraints[i].first));
		} 
	}
	bool toRet = (s.check() == z3::sat);
	#ifdef DEBUG
		std::cout << this->getPathCondition() << "\n";
		std::cout << toRet << "\n";
		z3::model m = s.get_model();
		std::cout << m << std::endl;
	#endif
	return toRet;
}