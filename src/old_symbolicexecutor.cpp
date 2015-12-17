#include "symbolicexecutor.h"
#include "jsoncpp/dist/json/json.h"
#include "utils.h"
#include <fstream>


#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/Metadata.h>

#include <pthread.h>


SymbolicExecutor::SymbolicExecutor(std::string f, ServerSocket * s)
{
	socket = s;
	filename = f;
	rootState = NULL;
    rootBlock = NULL;
    isBFS = false;
    dir = 0;
    steps = -1;
    currId = 0;
    prevId = -1;
}

ExpressionTree* SymbolicExecutor::getExpressionTree(ProgramState* state, llvm::Value* value)
{
	if (ExpressionTree* exptree = state->get(value))
	{
		return exptree;
	}
	else return new ExpressionTree(value);
}	
void SymbolicExecutor::executeNonBranchingInstruction(llvm::Instruction* instruction,SymbolicTreeNode* symbTreeNode, ProgramState* state)
{
	#ifdef DEBUG	
		std::cout << " executing :" << instruction->getOpcodeName() << " instruction \n";
		std::cout << "State at this point == \n------------------" << state->toString() << "\n";
	#endif
	if (llvm::MDNode *N = instruction->getMetadata("dbg")) 
	{  
		llvm::DILocation Loc(N);
		unsigned Line = Loc.getLineNumber();
		symbTreeNode->minLineNumber = std::min(symbTreeNode->minLineNumber,Line);
		symbTreeNode->maxLineNumber = std::max(symbTreeNode->maxLineNumber,Line);
		std::cout << "instruction == " << getString(instruction) << "\tLine Number == " << Line << "\n";
	}
	if (instruction->getOpcode() == llvm::Instruction::Alloca)
	{
		#ifdef DEBUG	
			std::cout << "executing Store \n";
		#endif
	}
	else if(instruction->getOpcode()==llvm::Instruction::Store)
	{
		#ifdef DEBUG	
			std::cout << "executing Store \n";
		#endif
		llvm::Value* memLocation = instruction->getOperand(1);
		llvm::Value* value = instruction->getOperand(0);
		state->add(memLocation,getExpressionTree(state,value));
	}
	else if(instruction->getOpcode()==llvm::Instruction::Load)
	{
		#ifdef DEBUG	
			std::cout << "executing Load \n";
		#endif
		ExpressionTree* exptree = getExpressionTree(state,instruction->getOperand(0));
		state->add(instruction,exptree);
	
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
	}
	else if (instruction->getOpcode() == llvm::Instruction::ICmp)
	{
		#ifdef DEBUG	
			std::cout << "executing ICmp \n";
		#endif

		llvm::ICmpInst* cmpInst = llvm::dyn_cast<llvm::ICmpInst>(instruction); 
		if(cmpInst->getSignedPredicate() == llvm::ICmpInst::ICMP_SGT)
		{	
			#ifdef DEBUG
				std::cout << "operand0 == " << getString(instruction->getOperand(0)) << "\n";
				std::cout << "operand1 == " << getString(instruction->getOperand(1)) << "\n";
			#endif
			ExpressionTree* lhs = getExpressionTree(state,instruction->getOperand(0));
			ExpressionTree* rhs = getExpressionTree(state,instruction->getOperand(1));
			#ifdef DEBUG
				if(lhs) std::cout << "lhs == " << lhs->toString(state->getMap()) << "\n";
				else std::cout << "lhs is NULL\n";
				if(rhs) std::cout << "rhs == " << rhs->toString(state->getMap()) << "\n";
				else std::cout << "rhs is NULL\n";
			#endif
			state->add(instruction,new ExpressionTree(">",lhs,rhs));
			#ifdef DEBUG
				std::cout << "state updated\n";
			#endif
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
std::vector<std::pair<llvm::BasicBlock*, ProgramState*> > 
	SymbolicExecutor::getNextBlocks(llvm::Instruction* inst, ProgramState* state)
{
	std::vector<std::pair<llvm::BasicBlock*, ProgramState*> > pairs;
	if(llvm::isa<BranchInst>(inst))
	{
		llvm::BranchInst* binst = llvm::dyn_cast<llvm::BranchInst>(inst);
		if(binst->isConditional())
		{
			ProgramState* first = new ProgramState(*state);
			llvm::Value* cond = binst->getCondition();
			first->constraints.push_back(std::make_pair(cond,"true"));
			first->addCondition(state->get(cond)->toString(state->getMap()));
			pairs.push_back(std::make_pair(binst->getSuccessor(0), first));
			int numSuccesors = binst->getNumSuccessors();
			if(numSuccesors == 2)
			{
				ProgramState* second = new ProgramState(*state);
				second->constraints.push_back(std::make_pair(cond,"false"));
				second->addCondition("not " + state->get(cond)->toString(state->getMap()));
				pairs.push_back(std::make_pair(binst->getSuccessor(1), second));
			}
		}
		else pairs.push_back(std::make_pair(binst->getSuccessor(0),state));
	}

	#ifdef DEBUG
		std::cout << "exiting getNextBlocks\n";
	#endif

	return pairs;
}


/**
 executes the basicBlock, updates programstate and returns the next Block(s) to execute if it can be determined that only the "Then" block should be executed then only the "Then" block is returned. Similarly for the else block. Otherwise both are are retuarned. NULL is returned if there's nothing left to execute
 */
std::vector<std::pair<llvm::BasicBlock*, ProgramState*> > 
	SymbolicExecutor::executeBasicBlock(SymbolicTreeNode* symbtreeNode, ProgramState* state)
{	
	llvm::BasicBlock* block = symbtreeNode->block;
	
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

			//std::cout << "move forward? \n";
			// std::cout << llvm::Instruction::Ret << "\n";
			// int x;
			// std::cin >> x;
			std::cout << "move forward? \n";			
		#endif

		if(i->getOpcode() == llvm::Instruction::Br || i->getOpcode() == llvm::Instruction::Ret) 
		{
			#ifdef DEBUG
				std::cout << "Branch Instruction Hit!\n";
				
			#endif
			return getNextBlocks(i,state);
		}
		else 
		{
			#ifdef DEBUG
				std::cout << "non branch instruction to b executed\n";
				if (i)
				{ 
					std::cout << getString(i) << "\n" << std::endl;
				}
				else
				{
					std::cout << "instruction NULL\n"<< std::endl;
				}
			#endif

			executeNonBranchingInstruction(i,symbtreeNode,state);
		}
			#ifdef DEBUG
				std::cout << "Instruction Executed! (either branch or non branch)\n";
			#endif
	}

	#ifdef DEBUG
		std::cout << "exiting executeBasicBlock\n";
	#endif

	return to_ret;
}



void SymbolicExecutor::symbolicExecute()
{
	
	while (1)
	{
		int parentId = prevId;
		#ifdef DEBUG
			std::cout << "prev id: "<< prevId << "\n";
		#endif
		std::deque<std::pair<SymbolicTreeNode*, ProgramState*> > deque;
		if (prevId == -1)
		{
			SymbolicTreeNode * tempSymTreeNode = new SymbolicTreeNode(rootBlock, NULL, NULL);
			deque.push_back(std::make_pair(tempSymTreeNode, rootState));
		}
		else
		{
			auto curr = BlockStates[prevId];

			SymbolicTreeNode * symTreeNode = curr.first;

			if (!symTreeNode)
			{
				#ifdef DEBUG
					std::cout << "tree node is NULL!!\n"; 
				#endif
			}
			else
			{
				#ifdef DEBUG
					std::cout << "tree node is not NULL!!\n"; 
				#endif
			}

			if (symTreeNode->left)
			{
				#ifdef DEBUG
					std::cout << "left child started!!\n";
				#endif

				SymbolicTreeNode * tempSymTreeNode = new SymbolicTreeNode(symTreeNode->left->block, NULL, NULL);
				if (dir) deque.push_back(std::make_pair(tempSymTreeNode, 
					new ProgramState(*curr.second)));

				if (!dir) deque.push_front(std::make_pair(tempSymTreeNode, 
					new ProgramState(*curr.second)));
				#ifdef DEBUG
					std::cout << "left child done!!\n"; 
				#endif
			}
			if (symTreeNode->right)
			{
				#ifdef DEBUG
					std::cout << "right child started!!\n"; 
				#endif
				
				SymbolicTreeNode * tempSymTreeNode = new SymbolicTreeNode(symTreeNode->right->block, NULL, NULL);
				if (dir) deque.push_back(std::make_pair(tempSymTreeNode, 
					new ProgramState(*curr.second)));

				if (!dir) deque.push_front(std::make_pair(tempSymTreeNode, 
					new ProgramState(*curr.second)));


				#ifdef DEBUG
					std::cout << "right child done!!\n"; 
				#endif
			}
		}


		for (int i = 0; i < steps || steps == -1; i++)
		{

			if (deque.empty()) break;

			auto curr = deque.front();
			ProgramState * currState = curr.second;
			SymbolicTreeNode* symbTreeNode = curr.first;
			deque.pop_front();
			

			std::vector<std::pair<llvm::BasicBlock*, ProgramState*> > new_blocks = 
				executeBasicBlock(symbTreeNode,currState);		
			
			BlockStates[currId++]=curr;
			
			Json::Value msg;
			msg["fileId"] = Json::Value(filename.c_str());
			msg["node"] = Json::Value(currId-1);
			msg["parent"] = Json::Value(parentId);
			msg["text"] = Json::Value(currState->toString());
			msg["fin"] = Json::Value("0");
			msg["constraints"] = Json::Value(currState->getPathCondition());
			msg["startLine"] = Json::Value(symbTreeNode->minLineNumber);
			msg["endLine"] = Json::Value(symbTreeNode->maxLineNumber);
			parentId = currId-1;
			prevId = parentId;
			
			Json::FastWriter fastWriter;
			std::string output = fastWriter.write(msg);
			std::cout << "sending this: " << output << std::endl;
			if (socket)
				(*socket) << output;

			if (!new_blocks.size()) continue;

			for (int j = 0; j < new_blocks.size(); j++)
			{
				int k = j;
				if (dir) k = new_blocks.size() - 1 - j;

				SymbolicTreeNode * tempSymTreeNode;	
				if (k == 0)
				{
					curr.first->left = new SymbolicTreeNode(new_blocks[k].first, NULL,
						 NULL);
					tempSymTreeNode = curr.first->left;
				}
				else
				{
					curr.first->right = new SymbolicTreeNode(new_blocks[k].first, NULL,
						 NULL);
					tempSymTreeNode = curr.first->right;
				}
				if (isBFS)
				{
					deque.push_back(std::make_pair(tempSymTreeNode, new_blocks[k].second));	
				}
				else
				{
					deque.push_front(std::make_pair(tempSymTreeNode, new_blocks[k].second));
				}
			}
			#ifdef DEBUG
				std::cout << "size of deque : " << deque.size() << "\n";
			#endif
		}
		#ifdef DEBUG
			std::cout << "proceed? \n";
			std::cout << "prevId : ";
			std::cin >>prevId;
			std::cout << "\n";
			std::cout << "isBFS : ";
			std::cin >>isBFS;
			std::cout << "\n";
			std::cout << "steps : ";
			std::cin >>steps;
			std::cout << "\n";
			std::cout << "dir : ";
			std::cin >>dir; 
			std::cout << "\n";
			continue;
		#endif

		std::cout << "going to sleep " << std::endl;

		// {
			std::unique_lock<std::mutex> lck(mtx);
			cv.wait(lck);
			lck.unlock();

		// }

		std::cout << "wakeup!! " << std::endl;
	}
}





/**
	Executes all the possible paths in the given function and returns the programState at the end of every path
*/

void SymbolicExecutor::executeFunction(llvm::Function* function)
{
	
	rootState = new ProgramState(function->args());
	rootBlock = &function->getEntryBlock();
	#ifdef DEBUG
		//std::cout <<"entry state : " << state->toString();
	#endif
	llvm::BasicBlock* currBlock;
	
	
	symbolicExecute();
	
	Json::Value msg;
	msg["fin"] = Json::Value("1");
	Json::FastWriter fastWriter;
	std::string output = fastWriter.write(msg);

	std::cout << "sending this: " << output << std::endl;
	(*socket) << output;
	std::cout << "Function executed" << std::endl;
}


llvm::Module* SymbolicExecutor::loadCode(std::string filename) 
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
		Buffer->release();
	}
	(**mainModuleOrError).materializeAllPermanently();
	return *mainModuleOrError;
}

void SymbolicExecutor::proceed(bool isbfs, int stps, int d, int prev)
{
	isBFS = isbfs;
	steps = stps;
	dir = d;
	prevId = prev; 
	
	std::cout << "WAKE UP!!!!!" << std::endl;
	std::unique_lock<std::mutex> lck(mtx);
	cv.notify_all();
	std::cout << "AWAKEN UP!!!!!" << std::endl;
}


void SymbolicExecutor::execute(bool isbfs, int stps, int d, int prev)
{
	isBFS = isbfs;
	steps = stps;
	dir = d;
	prevId = prev; 

	std::cout << filename.c_str() << " \n";
	llvm::Module* module = loadCode(filename.c_str());
	std::cout << filename.c_str() << " \n";

	auto function = module->getFunction("_Z7notmainii");
	executeFunction(function);
}


// int main()
// {
// 	SymbolicExecutor sym("src/SUT/hello.bc", NULL);
// 	sym.execute(true, 1, 0, -1);
// 	std::cout << "still working" << std::endl;
// 	return 0;
// }

/*
Old Execute Fnction code
	while(blocks.size())
	{
		currBlock = blocks[0];
		blocks.erase(blocks.begin());
		std::vector<llvm::BasicBlock*> new_blocks = executeBasicBlock(currBlock,state);
		// std::cout << "block executed" << std::endl;
		// std::cout << "new blocks received:	" << blocks.size() << std::endl;
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
*/