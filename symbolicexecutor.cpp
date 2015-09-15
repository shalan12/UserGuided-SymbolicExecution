#include <fstream>
#include <iostream>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/raw_ostream.h>
#include <iostream>
#include <map>
#include <vector>
#include "llvmExpressionTree.cpp"

class ProgramState
{
  private:
  std::map<llvm::Value*, ExpressionTree*> map;
  std::string pathCondition;
  
  public:
  
  ProgramState(llvm::iterator_range<llvm::Function::arg_iterator> inputs)
  {
        for (auto input = inputs.begin(), last = inputs.end(); input!=last; input++)
        {
            add(input,new ExpressionTree(input));
        }
        pathCondition = "";
  }
  std::string getPathCondition()
  {
  	return pathCondition;
  }

  void add(llvm::Value* value, ExpressionTree* exp)
  {
    map[value] = exp;
  }
  

  ExpressionTree* get(llvm::Value * s)
  {
    if ( map.find(s) == map.end() ) return NULL;
    else return map[s];
  }

  std::map<llvm::Value*, ExpressionTree*> getMap()
  {
    return map;
  }
  
  void printVariables()
  {
    for (auto& pr : map)
      std::cout <<  getString(pr.first) << " === " << pr.second->toString(map) << '\n';
  }
};
ExpressionTree* getExpressionTree(ProgramState& state, llvm::Value* value);
/**
Executes a nonbranching instruction and updates the program state
*/
void executeNonBranchingInstruction(llvm::Instruction* instruction,ProgramState& state)
{
    #ifdef DEBUG  
        std::cout << instruction->getOpcodeName() << " executing something \n";
    #endif
    if(instruction->getOpcode()==llvm::Instruction::Store)
    {
        #ifdef DEBUG  
            std::cout << "executing Store \n";
        #endif
        llvm::Value* memLocation = instruction->getOperand(1);
        llvm::Value* value = instruction->getOperand(0);
        state.add(memLocation,getExpressionTree(state,value));
    }
    else if(instruction->getOpcode()==llvm::Instruction::Load)
    {
        #ifdef DEBUG  
            std::cout << "executing Load \n";
        #endif
        ExpressionTree* exptree = getExpressionTree(state,instruction->getOperand(0));
        state.add(instruction,exptree);

        #ifdef DEBUG  
        if(!exptree)
            std::cout << "expression tree not found \n";
        #endif 
    }
    else if(instruction->getOpcode()==llvm::Instruction::Add)
    {
        #ifdef DEBUG  
            std::cout << "executing Add \n";
        #endif
        ExpressionTree* lhs = getExpressionTree(state,instruction->getOperand(0));
        ExpressionTree* rhs = getExpressionTree(state,instruction->getOperand(1));
        #ifdef DEBUG  
            std::cout << "lhs: " << lhs->toString(state.getMap()) <<"\n";
            std::cout << "rhs: " << rhs->toString(state.getMap()) <<"\n";
        #endif
        state.add(instruction,new ExpressionTree("+",lhs,rhs));     
    }
    else if (instruction->getOpcode() == llvm::Instruction::ICmp)
    {
        #ifdef DEBUG  
            std::cout << "executing ICmp \n";
        #endif
        llvm::ICmpInst* cmpInst = llvm::dyn_cast<llvm::ICmpInst>(instruction); 
        // if (llvm::ConstantInt* cl = llvm::dyn_cast<llvm::ConstantInt>(value))
        if(cmpInst->getSignedPredicate() == llvm::ICmpInst::ICMP_SGT)
        {
          ExpressionTree* lhs = getExpressionTree(state,instruction->getOperand(0));
          ExpressionTree* rhs = getExpressionTree(state,instruction->getOperand(1));
          state.add(instruction,new ExpressionTree(">",lhs,rhs));
        }
    }
}

ExpressionTree* getExpressionTree(ProgramState& state, llvm::Value* value)
{
    if (ExpressionTree* exptree = state.get(value))
    {
        return exptree;
    }
    else return new ExpressionTree(value);
}


/**
 Executes a branching instruction and determines which block(s) need to be explored depending on the program state
*/
std::vector<llvm::BasicBlock*> getNextBlocks(llvm::Instruction* inst, ProgramState& state)
{
  std::vector<llvm::BasicBlock*> to_ret;
  llvm::Value * check = NULL;
  ExpressionTree * check_expr;
  for (int j = 0; j < inst->getNumOperands(); j++)
  {
    // std::cout <<"operand no : " << j+1 << "\n" << getString(inst->getOperand(j)) << "\n";
    llvm::BasicBlock* basicBlock = NULL;
    if (llvm::isa<llvm::BasicBlock>(inst->getOperand(j)))
    {
        basicBlock = llvm::dyn_cast<llvm::BasicBlock> (inst->getOperand(j));
    }
    llvm::Value* v = dynamic_cast<llvm::Value*> (inst->getOperand(j));
    if (!basicBlock && j == 0)
    {
      // std::cout << "yy!!\n";
      check = v;
      check_expr = state.get(check);
    }
    if (basicBlock) to_ret.push_back(basicBlock);
  }
  if (check && check_expr->isConstant())
  {
    if (to_ret.size() > 1)
    {
      // std::cout <<inst->getNumOperands() << " going to get integer!" << std::endl;
      if (check_expr->getInteger(check_expr->getTop()->value) == 0)
      {
        to_ret.resize(1);
        #ifdef DEBUG
            std::cout << "here 1\n";
        #endif
        return to_ret;
      }
      else if (check_expr->getInteger(check_expr->getTop()->value) == 1) 
      {
        to_ret[0] = to_ret[1];
        to_ret.resize(1);
        #ifdef DEBUG
            std::cout << "here 2\n";
        #endif
        return to_ret;
      }
    }
  }
  #ifdef DEBUG
  // std::cout << "about to return possible branches as follows:" << std::endl;
  // for (int j = 0; j < to_ret.size(); j++)
  // {
    // auto block = to_ret[j];
    // if (block)
      // std::cout << "new block not null" << std::endl;
    // for (auto i = block->begin(), e = block->end(); i != e; ++i)
    // {
      // printf("Basic block (name= %s) has %zu instructions\n",block->getName().str().c_str(),block->size());

      // std::cout << "printing operands ";
      // for (int j = 0; j < i->getNumOperands(); j++)
      // {
      //   std::cout << getString(i->getOperand(j)) << "\n";
      // }
      // std::cout << getString(i) << "\n";
      // std::cout << (*i).print();
      // The next statement works since operator<<(ostream&,...)
      // is overloaded for Instruction&
      // std::cout << *(i).str().c_str() << "\n";
      // std::cout << "move forward? ";
      // int x;
      // std::cin >> x;  
    // }
  // }


    std::cout << "about to return!";
  #endif

  return to_ret;
}


/**
 executes the basicBlock, updates programstate and returns the next Block(s) to execute if it can be determined that only the "Then" block should be executed then only the "Then" block is returned. Similarly for the else block. Otherwise both are are retuarned. NULL is returned if there's nothing left to execute
 */
std::vector<llvm::BasicBlock*> executeBasicBlock(llvm::BasicBlock* block, ProgramState& state)
{

  std::vector<llvm::BasicBlock*> to_ret;
 
  for (auto i = block->begin(), e = block->end(); i != e; ++i)
  {
    #ifdef DEBUG
            
        // printf("Basic block (name= %s) has %zu instructions\n",block->getName().str().c_str(),block->size());

        // std::cout << "printing operands ";
        // for (int j = 0; j < i->getNumOperands(); j++)
        // {
          // std::cout << getString(i->getOperand(j)) << "\n";
        // }
        // std::cout << getString(i) << "\n";
        // std::cout << (*i).print();
        // The next statement works since operator<<(ostream&,...)
        // is overloaded for Instruction&
        // std::cout << *(i).str().c_str() << "\n";
        // std::cout << "move forward? ";
        // int x;
        // std::cin >> x;
    #endif

    if(i->getOpcode() == llvm::Instruction::Br || i->getOpcode() == llvm::Instruction::Ret) 
    {
        #ifdef DEBUG
            std::cout << "Its a branch!";
            // std::cin >> x;
            // std::cout << "about to return!";
        #endif
        return getNextBlocks(i,state);
    }
    else 
    {
      #ifdef DEBUG
          int abc;
          std::cout << "instruction to b executed\n"<< std::endl;
          // std::cin >> abc;
          if (i)
          { 
            std::cout << getString(i) << "\n" << std::endl;
          }
          else
          {
            std::cout << "instruction NULL\n"<< std::endl;
          }
          // std::cin >> abc;
      #endif
      executeNonBranchingInstruction(i,state);
    }
    #ifdef DEBUG
        std::cout << "Executed!";
        // std::cin >> x;
    #endif
  }
  #ifdef DEBUG
    std::cout << "about to return! basic block";
  #endif
  return to_ret;
}
/**
    Converts an llvm::Value to a humanreadable string
*/

/**
    Executes all the possible paths in the given function and returns the programState at the end of every path
*/
ProgramState executeFunction(llvm::Function* function)
{
    ProgramState state = ProgramState(function->args());
    llvm::BasicBlock* currBlock;
    // std::cout << "good to go!" << std::endl;
    std::vector<llvm::BasicBlock*> blocks;
    blocks.push_back(&function->getEntryBlock());
    //since we're only writing code for executing a single path we can simply do this
    while(blocks.size())
    {
      currBlock = blocks[0];
      blocks.erase(blocks.begin());
      std::vector<llvm::BasicBlock*> new_blocks = executeBasicBlock(currBlock,state);
      // std::cout << "block executed" << std::endl;
      // std::cout << "new blocks received:  " << blocks.size() << std::endl;
      for (int i = 0; i < new_blocks.size(); i++)
      {
        if (blocks[i])
        {
          blocks.push_back(new_blocks[i]);
          #ifdef DEBUG
            std::cout << "new block not null" << std::endl;
          #endif
        }
        else
        {
          #ifdef DEBUG
            std::cout << "new block NULL!!" << std::endl;
          #endif
        }
      }
      #ifdef DEBUG
        std::cout << "new blocks added!!" << blocks.size() << std::endl;
      #endif

      // int y;
      // std::cin >> y;
      // if(blocks.size() > 0) currBlock = blocks[0];
    }
    // std::cout << state.getPathCondition();
    return state;
    
}

/**

*/
llvm::Module* loadCode(std::string filename) 
{
    auto Buffer = llvm::MemoryBuffer::getFileOrSTDIN(filename.c_str());
    if(!Buffer)
    {
        printf("not Buffer\n");
    }
    auto mainModuleOrError = getLazyBitcodeModule(Buffer->get(), llvm::getGlobalContext());
    if(!mainModuleOrError)
    {
        printf("not mainModuleOrError\n");
    }
    else 
    {
        // The module has taken ownership of the MemoryBuffer so release it
        // from the std::unique_ptr
        Buffer->release();
    }
    (**mainModuleOrError).materializeAllPermanently();
    return *mainModuleOrError;
}


int main()
{
    llvm::Module* module = loadCode("/media/ACER/Users/Shalan/Dropbox/SHALAN/LUMS/sproj/llvm-stuff/SUT/hello.bc");
    auto function = module->getFunction("main");
    ProgramState finalState = executeFunction(function);
    finalState.printVariables();
    return 0;
}