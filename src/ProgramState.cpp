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
	//s = new z3::solver(*p.s);
	for (auto& pr : p.map)
	{
		add(pr.first,new ExpressionTree(*(pr.second)));
	}
	this->pathCondition = p.pathCondition;
/*		for (auto&pr : p.variables)
	{
	variables.insert(std::pair<std::string, z3::expr>(pr.first,pr.second));
	//variables[pr.first] = pr.second;//z3::to_expr(c,Z3_translate(p.c, pr.second, c));
	}*/
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
		std::cout <<	getString(pr.first) << " === ";
		if (pr.second->top != NULL)
		{
			if (pr.second->top->left != NULL && pr.second->top->right != NULL)
			{
				std::string left = getString(pr.second->top->left->value);
				std::string right = getString(pr.second->top->right->value);
				variables.insert(std::pair<std::string, z3::expr>
								(left, c.int_const(left.c_str())));

				variables.insert(std::pair<std::string, z3::expr>
								(right, c.int_const(right.c_str())));

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
				variables.insert(std::pair<std::string,z3::expr>
					(getString(pr.second->top->value),
						c.int_const(getString(pr.second->top->value).c_str())));

				std::cout << variables.at(getString(pr.second->top->value)) << '\n';
			}
		}
	}
}

void ProgramState::Z3solver()
{ 
	z3::solver s(c);
	for (int i = 0; i < constraints.size(); i++)
	{
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