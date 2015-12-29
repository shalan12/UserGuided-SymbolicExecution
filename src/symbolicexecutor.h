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

#include "jsoncpp/dist/json/json.h"
#include "jsonreader.h"
#include <thread>
#include <condition_variable>
#include <utility>      
#include <mutex>              // std::mutex, std::unique_lock
#include <limits>
typedef llvm::BasicBlock::iterator InstructionPtr;
class SymbolicTreeNode 
{
  private:
    bool hasNext, hasPrev;
    InstructionPtr* instructionPtr;

  public:
    llvm::BasicBlock * block;
    ProgramState * state;
    SymbolicTreeNode * left;
    SymbolicTreeNode * right;
    SymbolicTreeNode * parent;
    bool isExecuted;
    int prevId;
    int id;
    unsigned int minLineNumber,maxLineNumber;
    static int instances;

    SymbolicTreeNode(llvm::BasicBlock * b,
                     ProgramState * ps, int pid,
                     InstructionPtr* itr = NULL,
                     SymbolicTreeNode * l = NULL, 
                     SymbolicTreeNode * r = NULL, 
                     SymbolicTreeNode * parent = NULL)
    {
      block = b;
      if(itr == NULL) itr = new InstructionPtr(block->begin());
      this->instructionPtr = itr;
      left = l;
      right = r;
      state = ps;
      hasNext = true;
      hasPrev = false;
      this->parent = parent;
      maxLineNumber = 0;
      minLineNumber = std::numeric_limits<unsigned int>::max();
      prevId = pid;
      id = instances++;
      isExecuted = false;
    }
    bool hasNextInstruction()
    {
      return hasNext;
    }
    bool hasPrevInstruction()
    {
      return hasPrev;
    }
    InstructionPtr getPreviousInstruction()
    {
      if(*instructionPtr == block->begin())
      {
        hasPrev = false;
        return *instructionPtr;
      }
      else
      {
        hasNext = true;
        return --(*instructionPtr);
      }
    }
    InstructionPtr getNextInstruction()
    {
      InstructionPtr toRet = *instructionPtr;
      if (hasNext)
      {
        hasNext = (++*instructionPtr != block->end());
      }
      hasPrev = true;
      return toRet;
    }
};

class SymbolicExecutor
{
  private:
    unsigned int minLineNumber;
    unsigned int maxLineNumber;
    std::map<int, llvm::BasicBlock*> lineToBlock; 
    std::string filename;
    std::map<int, SymbolicTreeNode* > BlockStates;
    std::map<llvm::BasicBlock*, bool > excludedNodes;
    SymbolicTreeNode * rootNode;
    JsonReader * reader;


    llvm::Module* loadCode(std::string filename);
    ExpressionTree* getExpressionTree(ProgramState* state, llvm::Value* value);
    /**
     Executes a nonbranching instruction and updates the program state
    */
    void executeNonBranchingInstruction(llvm::Instruction* instruction,SymbolicTreeNode* symTreeNode);
     /**
      Executes a branching instruction and determines which block(s) need to be explored depending on the program state
    */
    std::vector<SymbolicTreeNode*> getNextBlocks(llvm::Instruction* inst, SymbolicTreeNode* node);


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
  void exclude(int id, int b);
  void proceed(Json::Value val);
  void execute(Json::Value val);

};
#endif