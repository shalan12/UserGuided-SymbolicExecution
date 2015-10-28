#ifndef SYMBOLIC_EXECUTOR
#define SYMBOLIC_EXECUTOR

#include "llvmExpressionTree.h"
#include "server/ServerSocket.h"
#include "ProgramState.h"


#include <iostream>
#include <map>
#include <vector>
#include <deque>

#include <llvm/IR/Module.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>



#include <thread>
#include <condition_variable>
#include <utility>      
#include <mutex>              // std::mutex, std::unique_lock


struct SymbolicTreeNode {
  llvm::BasicBlock * block;
  SymbolicTreeNode * left;
  SymbolicTreeNode * right;
  SymbolicTreeNode(llvm::BasicBlock * b, SymbolicTreeNode * l, SymbolicTreeNode * r)
  {
    block = b;
    left = l;
    right = r;
  }
};

class SymbolicExecutor
{
  private:
    std::mutex mtx;
    std::condition_variable cv;
    ServerSocket * socket;
    std::string filename;
    std::map<int, std::pair<SymbolicTreeNode*, ProgramState*> > BlockStates;
    ProgramState * rootState;
    llvm::BasicBlock * rootBlock;
    bool isBFS;
    int dir;
    int steps;
    int currId;
    int prevId;

    llvm::Module* loadCode(std::string filename);
    ExpressionTree* getExpressionTree(ProgramState* state, llvm::Value* value);
    /**
     Executes a nonbranching instruction and updates the program state
    */
    void executeNonBranchingInstruction(llvm::Instruction* instruction,ProgramState* state);
     /**
      Executes a branching instruction and determines which block(s) need to be explored depending on the program state
    */
    std::vector<std::pair<llvm::BasicBlock*, ProgramState*>> 
      getNextBlocks(llvm::Instruction* inst, ProgramState* state);


  public:
  SymbolicExecutor(std::string f, ServerSocket * s);
  
  /**
   executes the basicBlock, updates programstate and returns the next Block(s) to execute if it can be determined that only the "Then" block should be executed then only the "Then" block is returned. Similarly for the else block. Otherwise both are are retuarned. NULL is returned if there's nothing left to execute
   */
  std::vector<std::pair<llvm::BasicBlock*, ProgramState*> > 
      executeBasicBlock(llvm::BasicBlock* block, ProgramState* state);

  void symbolicExecute();

  /**
      Executes all the possible paths in the given function and returns the programState at the 
      end of every path
  */
  void executeFunction(llvm::Function* function);
  void proceed();

  void execute(bool isbfs, int stps, int d, int prev);

};
#endif