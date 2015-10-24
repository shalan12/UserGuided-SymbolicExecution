#ifndef SYMBOLIC_EXECUTOR
#define SYMBOLIC_EXECUTOR

#include "llvmExpressionTree.h"
#include "server/ServerSocket.h"
#include "ProgramState.h"


#include <iostream>
#include <map>
#include <vector>

#include <llvm/IR/Module.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>



#include <thread>
#include <condition_variable>
#include <utility>      
#include <mutex>              // std::mutex, std::unique_lock


class SymbolicExecutor
{
  private:
    std::mutex mtx;
    std::condition_variable cv;
    ServerSocket * socket;
    std::string filename;
    int currId;
    std::map<llvm::BasicBlock*, int> BlockIds;
    // sigset_t mask;
    sigset_t mask, oldmask;
    static int instances;

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

  void symbolicExecute(ProgramState * s, llvm::BasicBlock * prev, 
                    llvm::BasicBlock * b, std::vector<ProgramState*> & vec);

  /**
      Executes all the possible paths in the given function and returns the programState at the 
      end of every path
  */
  std::vector<ProgramState*> executeFunction(llvm::Function* function);
  void proceed();

  void execute();

};
#endif