#include <pthread.h>
#include <fstream>
#include <sstream>
#include <signal.h>
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
#include "z3++.h"
#include "llvmExpressionTree.cpp"
#include "server/ServerSocket.cpp"
#include "jsoncpp/dist/jsoncpp.cpp"
#include <thread>
#include <string>
#include <utility>      


// #define DEBUG 1

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
  ProgramState(llvm::iterator_range<llvm::Function::arg_iterator> inputs)
  {     
    //s = new z3::solver(c);
        for (auto input = inputs.begin(), last = inputs.end(); input!=last; input++)
        {   
            add(input,new ExpressionTree(input));
        }
        pathCondition = "";
  }
  ProgramState(const ProgramState & p)
  {
    //s = new z3::solver(*p.s);
    for (auto& pr : p.map)
    {
      add(pr.first,new ExpressionTree(*(pr.second)));
    }
/*    for (auto&pr : p.variables)
    {
      variables.insert(std::pair<std::string, z3::expr>(pr.first,pr.second));
      //variables[pr.first] = pr.second;//z3::to_expr(c,Z3_translate(p.c, pr.second, c));
    }*/
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

  std::string toString()
  {
    std::stringstream str;
    for (auto& pr : map)
      str <<  getString(pr.first) << "\t == \t" << pr.second->toString(map) << '\n';
    return str.str();
  }
  
  void printVariables()
  {
    std::cout << toString();
  }
  void printZ3Variables()
  {
    for (auto& pr : map)
    {
      std::cout <<  getString(pr.first) << " === ";
      if (pr.second->top != NULL)
      {
        if (pr.second->top->left != NULL && pr.second->top->right != NULL)
        {
          std::string left = getString(pr.second->top->left->value);
          std::string right = getString(pr.second->top->right->value);
          variables.insert(std::pair<std::string, z3::expr>(left, c.int_const(left.c_str())));
          variables.insert(std::pair<std::string, z3::expr>(right, c.int_const(right.c_str())));
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
          variables.insert(std::pair<std::string,z3::expr>(getString(pr.second->top->value),c.int_const(getString(pr.second->top->value).c_str())));
          std::cout << variables.at(getString(pr.second->top->value)) << '\n';
        }
      }
    }
  }
  void Z3solver()
  { 
    z3::solver s(c);
    for (int i = 0; i < constraints.size(); i++)
    {
      ExpressionTree* exptree = get(constraints[i].first);
      if (exptree->top->left != NULL && exptree->top->right != NULL)
      {
        std::string left = getString(exptree->top->left->value);
        std::string right = getString(exptree->top->right->value);
        if (constraints[i].second == "true")
        {
          if(exptree->top->data == ">")
          {
            s.add(variables.at(left) > variables.at(right));
            std::cout << "Constraint is: " << left << " > " << right << std::endl;
          }  
          else if(exptree->top->data == "<")
          {
            s.add(variables.at(left) > variables.at(right));
            std::cout << "Constraint is: " << left << " < " << right << std::endl;
          }
        }
        else if (constraints[i].second == "false")
        {
          if(exptree->top->data == ">")
          {
            s.add(variables.at(left) <= variables.at(right));
            std::cout << "Constraint is: " << left << " <= " << right << std::endl;
          }
          else if(exptree->top->data == "<")
          {
            s.add(variables.at(left) >= variables.at(right)); 
            std::cout << "Constraint is: " << left << " >= " << right << std::endl;
          }
        }
      }

    }

    std::cout << s.check() << "\n";
    z3::model m = s.get_model();
    std::cout << m << std::endl;
  }
};

class SymbolicExecutor
{
  private:
    ServerSocket * socket;
    std::string filename;
    sigset_t mask;
    static int instances;
  public:
  ExpressionTree* getExpressionTree(ProgramState* state, llvm::Value* value)
  {
      if (ExpressionTree* exptree = state->get(value))
      {
          return exptree;
      }
      else return new ExpressionTree(value);
  }  /**
  Executes a nonbranching instruction and updates the program state
  */
  void executeNonBranchingInstruction(llvm::Instruction* instruction,ProgramState* state)
  {
      #ifdef DEBUG  
          std::cout << " executing :" << instruction->getOpcodeName() << " instruction \n";
      #endif
      if (instruction->getOpcode() == llvm::Instruction::Alloca)
      {
          #ifdef DEBUG  
              std::cout << "executing Store \n";
          #endif
          //state->variables.insert(std::make_pair(getString(instruction).c_str(),state->c.int_const(getString(instruction).c_str())));
          //state->variables.insert(std::pair<std::string, z3::expr>(getString(instruction).c_str(),state->c.int_const(getString(instruction).c_str())));
          //std::cout << state->variables.at(getString(instruction).c_str()) << std::endl;
          //state->variables[getString(instruction).c_str()] = state->c.int_const(getString(instruction).c_str());    
      }
      else if(instruction->getOpcode()==llvm::Instruction::Store)
      {
          #ifdef DEBUG  
              std::cout << "executing Store \n";
          #endif
          llvm::Value* memLocation = instruction->getOperand(1);
          llvm::Value* value = instruction->getOperand(0);
          state->add(memLocation,getExpressionTree(state,value));
          //state->s->add(state->variables[getString(memLocation).c_str()] == state->variables[getString(value).c_str()]);
      }
      else if(instruction->getOpcode()==llvm::Instruction::Load)
      {
          #ifdef DEBUG  
              std::cout << "executing Load \n";
          #endif
          ExpressionTree* exptree = getExpressionTree(state,instruction->getOperand(0));
          state->add(instruction,exptree);
          //state->variables.insert(std::pair<std::string, z3::expr>(getString(instruction).c_str(),state->c.int_const(getString(instruction).c_str())));
         // state->s->add(state->variables[getString(instruction).c_str()] == state->variables[getString(instruction->getOperand(0)).c_str()]);

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
              if (lhs)
                std::cout << "lhs not NULL\n";
              if (rhs)
                std::cout << "rhs not NULL\n";

              std::cout << "lhs: " << lhs->toString(state->getMap()) <<"\n";
              std::cout << "rhs: " << rhs->toString(state->getMap()) <<"\n";
          #endif
          state->add(instruction,new ExpressionTree("+",lhs,rhs));  
          //std::cout << state->variables.at(getString(instruction->getOperand(0)).c_str()) + state->variables.at(getString(instruction->getOperand(1)).c_str())  << std::endl; 
          //state->variables[getString(instruction).c_str()] = state->c.bool_const(getString(instruction).c_str());
          //state->s->add(state->variables[getString(instruction).c_str()] == state->variables[getString(instruction->getOperand(0)).c_str()] + state->variables[getString(instruction->getOperand(1)).c_str()]);   
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
            state->add(instruction,new ExpressionTree(">",lhs,rhs));
            //state->variables[getString(instruction).c_str()] = state->c.bool_const(getString(instruction).c_str());
            //state->s->add(state->variables[getString(instruction).c_str()] == state->variables[getString(instruction->getOperand(0)).c_str()] > state->variables[getString(instruction->getOperand(1)).c_str()]);
          }
      }
      #ifdef DEBUG
        std::cout << "exiting executeNonBranchingInstruction\n";
      #endif
  }

  


  /**
   Executes a branching instruction and determines which block(s) need to be explored depending on the program state
  */
  std::vector<std::pair<llvm::BasicBlock*, ProgramState*>> getNextBlocks(llvm::Instruction* inst, ProgramState* state)
  {
    std::vector<std::pair<llvm::BasicBlock*, ProgramState*> > pairs;
    llvm::Value* value = NULL;
    if(inst->getOpcode() == llvm::Instruction::Ret)
    {
      return pairs;
    }
    else
    {
      if (!llvm::isa<llvm::BasicBlock>(inst->getOperand(0)))
      {
          value = llvm::dyn_cast<llvm::Value> (inst->getOperand(0));
          //state->s->add(state->variables.at(getString(value).c_str()) == true);
          state->constraints.push_back(std::pair<llvm::Value*, std::string>(value, "true"));
      }
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
          check_expr = state->get(check);
        }
        if (basicBlock) 
        {
            ProgramState* prg = new ProgramState(*state);
            if (value)
            {
              if (j <= 1)
              {
                prg->constraints.push_back(std::pair<llvm::Value*, std::string>(value, "true"));
                //prg->s->add(prg->variables.at(getString(value).c_str()) == true);
              }
              else
              {
                prg->constraints.push_back(std::pair<llvm::Value*,std::string>(value, "false"));
                //prg->s->add(prg->variables.at(getString(value).c_str()) == false);
              }

              pairs.push_back(std::make_pair(basicBlock, prg));
            }
            else
              pairs.push_back(std::make_pair(basicBlock, state)); 
        }
      }

      /*if (check && check_expr->isConstant())
      {
        if (to_ret.size() > 1)
        {
          if (check_expr->getInteger() == 0)
          {
            to_ret.resize(1);
            #ifdef DEBUG
                std::cout << "here 1\n";
            #endif
            return to_ret;
          }
          else if (check_expr->getInteger() == 1) 
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
      */
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


      std::cout << "exiting getNextBlocks\n";

    #endif

    return pairs;
  }


  /**
   executes the basicBlock, updates programstate and returns the next Block(s) to execute if it can be determined that only the "Then" block should be executed then only the "Then" block is returned. Similarly for the else block. Otherwise both are are retuarned. NULL is returned if there's nothing left to execute
   */
  std::vector<std::pair<llvm::BasicBlock*, ProgramState*> > executeBasicBlock(llvm::BasicBlock* block, ProgramState* state)
  {
    std::vector<std::pair<llvm::BasicBlock*, ProgramState*>> to_ret;
    #ifdef DEBUG
      printf("Basic block (name= %s) has %zu instructions\n",block->getName().str().c_str(),block->size());
    #endif

    for (auto i = block->begin(), e = block->end(); i != e; ++i)
    {
      #ifdef DEBUG

          std::cout << "printing operands : " << i->getNumOperands() << "\n";
          for (int j = 0; j < i->getNumOperands(); j++)
          {
            std::cout << "operand # : " << j+1 << " : "<< getString(i->getOperand(j)) << "\n";
          }
          std::cout << "printing instruction: " << getString(i) << "\n";
          std::cout << "getOpcode: " << i->getOpcode() << "\n";
          std::cout << "move forward? \n";
          // std::cout << llvm::Instruction::Ret << "\n";
          // int x;
          // std::cin >> x;
      #endif

      if(i->getOpcode() == llvm::Instruction::Br || i->getOpcode() == llvm::Instruction::Ret) 
      {
          #ifdef DEBUG
              std::cout << "Branch Instruction Hit!\n";
              // std::cin >> x;
              // std::cout << "about to return!";
          #endif
          return getNextBlocks(i,state);
      }
      else 
      {
        #ifdef DEBUG
            int abc;
            std::cout << "non branch instruction to b executed\n";
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
          std::cout << "Instruction Executed! (either branch or non branch)\n";
          // std::cin >> x;
      #endif
    }
    #ifdef DEBUG
      std::cout << "exiting executeBasicBlock\n";
    #endif
    return to_ret;
  }

void symbolicExecute(ProgramState * s, llvm::BasicBlock * b, std::vector<ProgramState*> & vec)
  {
    std::vector<std::pair<llvm::BasicBlock*, ProgramState*>> new_blocks = executeBasicBlock(b,s);
    Json::Value msg;
    msg["id"] = Json::Value(filename.c_str());
    msg["basicblock"] = Json::Value(s->toString());
    msg["fin"] = Json::Value("0");
    Json::FastWriter fastWriter;
    std::string output = fastWriter.write(msg);
    (*socket) >> output;
    
    sigfillset(&mask);
    sigdelset(&mask, SIGRTMIN+instances);
    sigsuspend(&mask);

    if (new_blocks.size() < 1)
    {
      vec.push_back(s);
      return;
    }
    for (int i = 0; i < new_blocks.size(); i++)
    {
      if (new_blocks[i].first)
      {
        #ifdef DEBUG
          std::cout << "new block not null" << std::endl;
        #endif
        // std::cout << "************(input) printing vars!**********\n" << std::endl;
        // s->printVariables();
        // std::cout << "************(input) printing vars!**********\n" << std::endl;
        // std::cout << "************(output) printing vars!**********\n" << std::endl;
        // t->printVariables();
        // std::cout << "************(output) printing vars!**********\n" << std::endl;
        symbolicExecute(new_blocks[i].second, new_blocks[i].first, vec);
      }
      else
      {
        #ifdef DEBUG
          std::cout << "new block NULL!!" << std::endl;
        #endif
      }
    }
  }

  /**
      Converts an llvm::Value to a humanreadable string
  */

  /**
      Executes all the possible paths in the given function and returns the programState at the end of every path
  */
  std::vector<ProgramState*> executeFunction(llvm::Function* function)
  {
      std::vector<ProgramState*> vec;
      ProgramState* state = new ProgramState(function->args());
      #ifdef DEBUG
        state->printVariables();
      #endif
      llvm::BasicBlock* currBlock;
      // std::cout << "good to go!" << std::endl;
      std::vector<llvm::BasicBlock*> blocks;
      blocks.push_back(&function->getEntryBlock());
      //since we're only writing code for executing a single path we can simply do this
      symbolicExecute(state, &function->getEntryBlock(), vec);

      /***
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
      // std::cout << state->getPathCondition();
      ***/
      
      return vec;
      
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

  void proceed()
  {
    sigprocmask (SIG_UNBLOCK, &mask, NULL);
  }


  void execute()
  {
      llvm::Module* module = loadCode(filename.c_str());
      // for (auto function = module->begin(), last = module->end(); function!=last; function++)
      // {
      //     printf("%s\n",function->getName().str().c_str());
      // }
      auto function = module->getFunction("_Z7notmainii");
      std::vector<ProgramState*> final_states = executeFunction(function);
      std::cout << "final states: ("<< final_states.size() << ")\n";
      for (int i = 0; i < final_states.size(); i++)
      {
        final_states[i]->printVariables();
        std::cout << "\n\n";
      }
      for (int i = 0; i < final_states.size(); i++)
      {
        final_states[i]->printZ3Variables();
        std::cout << "\n";
        final_states[i]->Z3solver();
        std::cout << "\n\n";

      }
  }
  SymbolicExecutor(std::string f, ServerSocket * s)
  {
    socket = s;
    filename = f;
    instances++;
  }
};

int SymbolicExecutor::instances = 0;


std::map< std::string, std::pair<std::thread,SymbolicExecutor*> > threads_sym;

void runOnThread(SymbolicExecutor * sym)
{
  sym->execute();
}
void createNewSym(std::string file, ServerSocket * s)
{
  SymbolicExecutor * sym = new SymbolicExecutor(file, s);
   // std::thread th = std::thread(rnd, sym);
   // std::thread th = std::thread([sym](){
   //      sym->execute();
   //      };);
  threads_sym[file] = std::make_pair(std::thread(runOnThread,sym),sym);
}

int executeSym(std::string id)
{
  threads_sym[id].second->proceed();
}

int main ()
{
  try
  {
    ServerSocket server ( 30000 );
    std::string message;

    while(true)
    {
      try
      {
        ServerSocket new_sock;
        server.accept ( new_sock );
        new_sock >> message;
        if (message.length() > 4)
        {
          std::string type = message.substr(0,4);
          if (type == "file")
          {
            createNewSym(message.substr(4), &new_sock);
          }
          else if (type == "exec")
          {
            executeSym(message.substr(4)); 
          }
        }
      }
      catch ( SocketException& ) {}
    }
  }
  catch ( SocketException& e )
  {
    std::cout << "Exception was caught:" << e.description() << "\n";
  }
  return 0;
}

// int main()
// {
//   std::cout << "good to go!" << std::endl;
//   return 0;
// }