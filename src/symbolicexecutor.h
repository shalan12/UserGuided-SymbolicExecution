#ifndef SYMBOLIC_EXECUTOR
#define SYMBOLIC_EXECUTOR

#include "llvmExpressionTree.h"
#include "server/ServerSocket.h"
#include "ProgramState.h"
#include "utils.h"

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
    int prevId;

  public:
    llvm::BasicBlock * block;
    ProgramState * state;
    SymbolicTreeNode * left;
    SymbolicTreeNode * right;
    bool isExecuted, isModel;
    int id;
    unsigned int minLineNumber,maxLineNumber;
    SymbolicTreeNode* returnNode;
    std::vector<std::pair<ExpressionTree*, std::string> > modelVals;
    
    SymbolicTreeNode(llvm::BasicBlock * b,
                     ProgramState * ps, int id, int pid)
    {
      block = b;
      this->id = id;
      state = ps;
      prevId = pid;
      
      if (block)
        this->instructionPtr = new InstructionPtr(block->begin());
      else
        this->instructionPtr = NULL;
      this->returnNode = NULL;
      left = NULL;
      right = NULL;
      hasNext = true;
      hasPrev = false;
      maxLineNumber = 0;
      minLineNumber = std::numeric_limits<unsigned int>::max();
      isExecuted = false;
      isModel = false;
    }
    SymbolicTreeNode(llvm::BasicBlock * b,ProgramState * ps, int id, int pid,
                     InstructionPtr* itr,SymbolicTreeNode * returnNode) 
                    : SymbolicTreeNode(b,ps,id,pid)
    {
      if(itr != NULL) this->instructionPtr = itr;
      this->returnNode = returnNode;
    }
    int getPrevId()
    {
        return prevId;
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
        hasNext = true;

        if(*instructionPtr == block->begin())
        {
            hasPrev = false;
            return *instructionPtr;
        }
        else
        {
            return --(*instructionPtr);
        }
    }

    InstructionPtr getNextInstruction()
    {
        InstructionPtr toRet = *instructionPtr;
        if (hasNext)
        {
            hasNext = (++(*instructionPtr) != block->end());
        }
        std::cout << "hasNext == " << hasNext << "\nInstructionPtr now at ";
        if(*instructionPtr == block->end())
        {
            std::cout << "END of block\n";
        }
        else std::cout << getString(*instructionPtr) << "\n";
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
    int numNodes;


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
  std::vector<SymbolicTreeNode*> executeModel(SymbolicTreeNode*);

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