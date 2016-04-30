#include "ProgramState.h"
#include "utils.h"
#include <sstream>
#include <vector>
#include <llvm/ADT/iterator_range.h>


ProgramState::ProgramState(llvm::iterator_range<llvm::Function::arg_iterator> inputs, 
	std::vector<ExpressionTree*> arguments)
{		 
	int i = 0;
	for (auto input = inputs.begin(), last = inputs.end(); input!=last; input++)
	{	
		if(i >= arguments.size())
			add(input,new ExpressionTree(input, getUserVarMap(), getLLVMVarMap()));
		else 
			add(input,arguments[i++]);
		
		#ifdef DEBUG
			std::cout << "program state constructor, function parameters : " << getString(input) << "\n";
		#endif
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

	for (auto& pr : from.llvmVarMap)
	{
		to->llvmVarMap[pr.first] = pr.second;
	}
	for (auto& pr : from.stores)
	{
		to->stores[pr.first] = pr.second;
	}

	for (auto constraint : from.z3Constraints)
	{
		z3::expr * copy_constraint = new z3::expr(to->context);
		*copy_constraint = to_expr(to->context, Z3_translate(from.context, *constraint.first, to->context));
		to->z3Constraints.push_back(std::make_pair(copy_constraint, constraint.second));
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

/**
* Appends a condition to the constraint of the current path
*/
void ProgramState::addCondition(std::string cond)
{
	if(pathCondition == "") pathCondition = cond;
	else pathCondition += " &&\n" + cond;
}

/**
* Adds a llvm IR variable and its expression tree to the program state
*/
void ProgramState::add(llvm::Value* value, ExpressionTree* exp)
{
	map[value] = exp;
}

/**
* Adds a user variable and its expression tree to the program state
*/
void ProgramState::addUserVar(std::string varname, llvm::Value* val)
{
	userVarMap[varname] = val;
	llvmVarMap[val] = varname;
}

/**
* Adds a llvm IR variable and its expression tree to the program state
*/
void ProgramState::addLLVMVar(std::string varname, llvm::Value* val)
{
	llvmVarMap[val] = varname;
}

/**
* Adds a llvm IR store instruction variable to the stores map in program state
*/
void ProgramState::addStore(llvm::Value* val2, llvm::Value* val1)
{
	stores[val2] = val1;
}


std::map<llvm::Value*, llvm::Value*> ProgramState::getStoreMap()
{
	return stores;
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
	
	for (auto& pr : userVarMap)
	{
		if (get(pr.second))
			str << pr.first <<	"\t = \t" << map[pr.second]->toStringHumanReadable(this->getLLVMVarMap(), this->getStoreMap()) << '\n';
		else
			str << pr.first <<	"\t = \t" << getString(pr.second) << '\n';
	}
	return str.str();
}

void ProgramState::printZ3Variables()
{
	for (auto& pr : map)
	{
		#ifdef DEBUG
			std::cout << getString(pr.first) << " === ";
		#endif
		if (pr.second->top != NULL)
		{
			if (pr.second->top->left != NULL && pr.second->top->right != NULL)
			{
				std::string left = getString(pr.second->top->left->value);
				std::string right = getString(pr.second->top->right->value);
				variables.insert(std::make_pair(left, context.int_const(left.c_str())));
				variables.insert(std::make_pair(right, context.int_const(right.c_str())));

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
								context.int_const(getString(pr.second->top->value).c_str())));

				std::cout << variables.at(getString(pr.second->top->value)) << '\n';
			}
		}
	}
}


std::map<std::string, llvm::Value*> ProgramState::getUserVarMap()
{
	return userVarMap;
}

std::map<llvm::Value*, std::string> ProgramState::getLLVMVarMap()
{
	return llvmVarMap;
}

/**
* Uses Z3 to check constraint satisfiability
*/
bool ProgramState::Z3solver()
{ 
	int xyz;
	z3::solver s(context);
	#ifdef DEBUG
		std::cout << "size of constraints = " << z3Constraints.size() << "\n";
	#endif
	for (int i = 0; i < z3Constraints.size(); i++)
	{
		#ifdef DEBUG
			if (z3Constraints[i].first)
			{
				std::cout << "z3 expression not NULL!" << *(z3Constraints[i].first) << "\n";
				std::cin >> xyz; 
			}
			else
			{
				std::cout << "z3 expression is NULL!\n";
				std::cin >> xyz;
			}
			std::cout << "i = " << i << "\n";
		#endif
		if(z3Constraints[i].second == "true")
		{
			#ifdef DEBUG
				std::cout << "condition true\n";
				std::cin >> xyz;
			#endif
			s.add(*(z3Constraints[i].first));
		}
		else if(z3Constraints[i].second == "false")
		{
			#ifdef DEBUG
				std::cout << "condition false\n";
				std::cin >> xyz;
			#endif
			s.add(!(*(z3Constraints[i].first)));
		} 
	}
	std::cout << "some info from Z3\n";
	int abcdef;
	std::cin >> abcdef;
	std::cout << "s" << s << "\n";
	std::cout << "s.check()" << s.check() << "\n";
	std::cout << "*****************\n";
	bool toRet = (s.check() == z3::sat);
	std::string toRet_str = "";  
	if (toRet)
	{
		z3::model * model;
		z3::model m = s.get_model();
		std::stringstream ss;
		ss << m;
		toRet_str = ss.str();
		std::cout << m << std::endl;
		model = &m;
		// return model;
	}
	// return NULL;
	std::cout << "*****************\n";
	return toRet_str;
	// #ifdef DEBUG
	// 	std::cout << this->getPathCondition() << "\n";
	// 	std::cout << toRet << "\n";
	// 	if (toRet)
	// 	{
	// 		z3::model m = s.get_model();
	// 		std::cout << m << std::endl;
	// 	}
	// #endif
	// return toRet;
}