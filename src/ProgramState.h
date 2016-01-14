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
  std::map<std::string, llvm::Value*> userVarMap;

  std::string pathCondition;
  
  public:
  z3::context c;
  std::map<std::string, z3::expr> variables;
  std::map<llvm::Value*, z3::expr*> z3Variables;
  std::vector<std::pair<z3::expr*, std::string> > z3Constraints;

  //std::vector<std::pair<llvm::Value*, std::string> > constraints;
  
  static void Copy(const ProgramState& other, ProgramState* to, bool copyMap);
  ProgramState(){};
  ProgramState(llvm::iterator_range<llvm::Function::arg_iterator> inputs,
              std::vector<ExpressionTree*> arguments = std::vector<ExpressionTree*>(0));
  ProgramState(const ProgramState & p);

  std::string getPathCondition();
  void addCondition(std::string cond);
  
  void add(llvm::Value* value, ExpressionTree* exp);
  void addUserVar(std::string, llvm::Value* val);
  ExpressionTree* get(llvm::Value * s);
  std::map<llvm::Value*, ExpressionTree*> getMap();
  std::string toString();
  void printZ3Variables();
  bool Z3solver();
  std::map<std::string, llvm::Value*> getUserVarMap();

};
#endif