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


/**
* A struct to hold information about loops while symbolically executing them
*/
struct loopInfoStruct$
{
  llvm::BasicBlock * loopStartPoint;
  int numExecutions;
  int maxExecutions;
}typedef LoopInfo;

/**
* A struct to hold infromation about satisfiability of a certain path in Symbolic Execution
*/
struct satInfoStruct$
{
  bool isSatisfiable;

}typedef SATInfo;

/**
* A class to represnet node in the Symbolic Execution Tree to 
* store intermediate and final program states of different paths
*/
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
    std::string input;
    bool isExecuted, isModel;
    int id;
    unsigned int minLineNumber,maxLineNumber;
    SymbolicTreeNode* returnNode;
    std::vector<std::pair<ExpressionTree*, std::string> > modelVals;
    LoopInfo loopInfo;
    SATInfo satInfo;

    void setLoopInfo(llvm::BasicBlock * startPoint, int numExecutions)
    {
      this->loopInfo.loopStartPoint = startPoint;
      this->loopInfo.numExecutions = numExecutions; 
    }

    void setSATInfo(bool isSatisfiable)
    {
      this->satInfo.isSatisfiable = isSatisfiable;
    }

    SymbolicTreeNode(llvm::BasicBlock * b, ProgramState * ps, int id, int pid)
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
      this->loopInfo.loopStartPoint = NULL;
      this->loopInfo.numExecutions = 0;
      this->loopInfo.maxExecutions = 1;
      this->satInfo.isSatisfiable = true;
    }
    SymbolicTreeNode(llvm::BasicBlock * b,ProgramState * ps, int id, int pid,
                     InstructionPtr* itr,SymbolicTreeNode * returnNode) 
                    : SymbolicTreeNode(b,ps,id,pid)
    {
      if(itr != NULL) this->instructionPtr = itr;
      this->returnNode = returnNode;
    }
    /**
    * Returns the ID of the Parent node of the node
    */
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
    /**
    * Moves program counter one step back
    */
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

    /**
    * Moves program counter one step ahead
    */
    InstructionPtr getNextInstruction()
    {
        InstructionPtr toRet = *instructionPtr;
        if (hasNext)
        {
            hasNext = (++(*instructionPtr) != block->end());
        }
        #ifdef DEBUG
          std::cout << "hasNext == " << hasNext << "\nInstructionPtr now at ";
        #endif
        if(*instructionPtr == block->end())
        {
          #ifdef DEBUG
            std::cout << "END of block\n";
          #endif
        }
        
        #ifdef DEBUG
          else std::cout << getString(*instructionPtr) << "\n";
        #endif

        hasPrev = true;
        
        return toRet;
    }
};

/**
* A symbolic executor to execute an input program symbolically.
*/
class SymbolicExecutor
{
  private:
    std::map<int, std::vector< llvm::BasicBlock* > > lineToBlock; 
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
  void printAllFunctions(llvm::Function* function, int index, Json::Value & functions);
  void executeFunction(llvm::Function* function);
  void proceed(bool isbfs, int stps, int d, int prev);
  void exclude(int id, int b);
  void proceed(Json::Value val);
  void execute(Json::Value val);

};
#endif