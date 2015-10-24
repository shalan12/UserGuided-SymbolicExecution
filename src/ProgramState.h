#ifndef PROGRAM_STATE
#define PROGRAM_STATE
#include <map>
#include <vector>
#include <iostream>
#include <llvm/IR/Function.h>
#include "z3++.h"
#include "llvmExpressionTree.h"

class ProgramState
{
  private:
  std::map<llvm::Value*, ExpressionTree*> map;
  std::string pathCondition;
  static int instances;
  
  public:
  z3::context c;
  std::map<std::string, z3::expr> variables;
  std::vector<std::pair<llvm::Value*, std::string> > constraints;
  ProgramState(llvm::iterator_range<llvm::Function::arg_iterator> inputs);
  ProgramState(const ProgramState & p);

  std::string getPathCondition();
  void addCondition(std::string cond);
  
  void add(llvm::Value* value, ExpressionTree* exp);
  ExpressionTree* get(llvm::Value * s);
  std::map<llvm::Value*, ExpressionTree*> getMap();
  std::string toString();
  void printZ3Variables();
  void Z3solver();
};
#endif