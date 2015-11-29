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
#include <limits>

struct SymbolicTreeNode {
  llvm::BasicBlock * block;
  ProgramState * state;
  SymbolicTreeNode * left;
  SymbolicTreeNode * right;
  int prevId;
  int id;
  unsigned int minLineNumber,maxLineNumber;
  static int instances;

  SymbolicTreeNode(llvm::BasicBlock * b, ProgramState * ps, int pid, SymbolicTreeNode * l = NULL, SymbolicTreeNode * r = NULL)
  {
    block = b;
    left = l;
    right = r;
    state = ps;
    maxLineNumber = 0;
    minLineNumber = std::numeric_limits<unsigned int>::max();
    prevId = pid;
    id = instances++;
  }
};

class SymbolicExecutor
{
  private:
    std::mutex mtx;
    std::condition_variable cv;
    ServerSocket * socket;
    std::string filename;
    std::map<int, SymbolicTreeNode* > BlockStates;
    std::map<SymbolicTreeNode*, bool > excludedNodes;
    SymbolicTreeNode * rootNode;
    bool isBFS;
    int dir,steps,prevId;

    llvm::Module* loadCode(std::string filename);
    ExpressionTree* getExpressionTree(ProgramState* state, llvm::Value* value);
    /**
     Executes a nonbranching instruction and updates the program state
    */
    void executeNonBranchingInstruction(llvm::Instruction* instruction,SymbolicTreeNode* symTreeNode);
     /**
      Executes a branching instruction and determines which block(s) need to be explored depending on the program state
    */
    std::vector<SymbolicTreeNode*> getNextBlocks(llvm::Instruction* inst, ProgramState* state, 
      int prevId);


  public:
  SymbolicExecutor(std::string f, ServerSocket * s);
  
  /**
   executes the basicBlock, updates programstate and returns the next Block(s) to execute if it can be determined that only the "Then" block should be executed then only the "Then" block is returned. Similarly for the else block. Otherwise both are are retuarned. NULL is returned if there's nothing left to execute
   */
  std::vector<SymbolicTreeNode*> executeBasicBlock(SymbolicTreeNode*);

  void symbolicExecute();

  /**
      Executes all the possible paths in the given function and returns the programState at the 
      end of every path
  */
  void executeFunction(llvm::Function* function);
  void proceed(bool isbfs, int stps, int d, int prev);
  void exclude(std:string id);


  void execute(bool isbfs, int stps, int d, int prev);

};
#endif