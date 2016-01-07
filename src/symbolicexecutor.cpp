#include "symbolicexecutor.h"
#include "jsoncpp/dist/json/json.h"
#include "utils.h"
#include <fstream>
#include "messagetypes.h"


#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/Metadata.h>
#include <llvm/IR/IntrinsicInst.h>
#include <pthread.h>

std::pair<ProgramState*,InstructionPtr*> getReturnState(SymbolicTreeNode* node, ExpressionTree* returnValue);

void printBlock(llvm::BasicBlock * b)
{
	for (auto instruction = b->begin(), e = b->end(); instruction != e; ++instruction)
	{
		std::cout << "printing operands : " << instruction->getNumOperands() << "\n";
			for (int j = 0; j < instruction->getNumOperands(); j++)
			{
				std::cout << "operand # : " << j+1 << " : " << getString(instruction->getOperand(j)) << "\n";
			}
			std::cout << "printing instruction: " << getString(instruction) << "\n";
			std::cout << "getOpcode: " << instruction->getOpcode() << "\n";
	}
}


SymbolicExecutor::SymbolicExecutor(std::string f, ServerSocket * s)
{
	reader = new JsonReader(s);
	filename = f;
	rootNode = NULL;
	numNodes = 0;
}

ExpressionTree* SymbolicExecutor::getExpressionTree(ProgramState* state, llvm::Value* value)
{
	if (ExpressionTree* exptree = state->get(value))
	{
		return exptree;
	}
	else return new ExpressionTree(value);
}	
void SymbolicExecutor::executeNonBranchingInstruction(llvm::Instruction* instruction,SymbolicTreeNode* symTreeNode)
{
	ProgramState* state = symTreeNode->state;
	#ifdef DEBUG	
		std::cout << " executing :" << instruction->getOpcodeName() << " instruction \n";
		std::cout << "State at this point == \n------------------" << state->toString() << "\n";
	#endif
	if (instruction->getOpcode() == 49)
	{
		if (llvm::DbgDeclareInst* DbgDeclare = llvm::dyn_cast<llvm::DbgDeclareInst>(instruction)) {
      		llvm::MDNode* Var = DbgDeclare->getVariable();
      		if (llvm::Value * val = llvm::dyn_cast<llvm::Value>(DbgDeclare->getAddress()))
      		{
	      		llvm::StringRef strRef = llvm::DIVariable(Var).getName();
	      		std::cout << " variable name : -- " << strRef.str() << std::endl << "\n";
	      		state->addUserVar(strRef.str(), val);
      		}
      	}
	}
	else if (instruction->getOpcode() == llvm::Instruction::Alloca)
	{
		#ifdef DEBUG	
			std::cout << "executing Allocate \n";
		#endif
	}
	else if(instruction->getOpcode()==llvm::Instruction::Store)
	{
		llvm::Value* memLocation = instruction->getOperand(1);
		llvm::Value* value = instruction->getOperand(0);
		state->add(memLocation,getExpressionTree(state,value));
	}
	else if(instruction->getOpcode()==llvm::Instruction::Load)
	{
		ExpressionTree* exptree = getExpressionTree(state,instruction->getOperand(0));
		state->add(instruction,exptree);
	
		#ifdef DEBUG	
		if(!exptree)
			std::cout << "expression tree not found \n";
		#endif 
	}
	else if(instruction->getOpcode()==llvm::Instruction::Add)
	{
		ExpressionTree* lhs = getExpressionTree(state,instruction->getOperand(0));
		ExpressionTree* rhs = getExpressionTree(state,instruction->getOperand(1));
	
		#ifdef DEBUG
			if (lhs)
				std::cout << "lhs not NULL\n";
			if (rhs)
				std::cout << "rhs not NULL\n";

			std::cout << "lhs: " << lhs->toString() <<"\n";
			std::cout << "rhs: " << rhs->toString() <<"\n";
		#endif
		state->add(instruction,new ExpressionTree("+",lhs,rhs));		
	}
	else if(instruction->getOpcode()==llvm::Instruction::Sub)
	{
		ExpressionTree* lhs = getExpressionTree(state,instruction->getOperand(0));
		ExpressionTree* rhs = getExpressionTree(state,instruction->getOperand(1));
	
		#ifdef DEBUG
			if (lhs)
				std::cout << "lhs not NULL\n";
			if (rhs)
				std::cout << "rhs not NULL\n";

			std::cout << "lhs: " << lhs->toString() <<"\n";
			std::cout << "rhs: " << rhs->toString() <<"\n";
		#endif
		state->add(instruction,new ExpressionTree("-",lhs,rhs));		
	}
	else if (instruction->getOpcode() == llvm::Instruction::ICmp)
	{
		llvm::ICmpInst* cmpInst = llvm::dyn_cast<llvm::ICmpInst>(instruction); 
		std::string op;
		if(cmpInst->getSignedPredicate() == llvm::ICmpInst::ICMP_SGT)
			op = ">";	
		else if(cmpInst->getSignedPredicate() == llvm::ICmpInst::ICMP_SLT)
			op = "<";
		else if(cmpInst->getSignedPredicate() == llvm::ICmpInst::ICMP_SLE)
			op = "<=";
		#ifdef DEBUG
			std::cout << "operand0 == " << getString(instruction->getOperand(0)) << "\n";
			std::cout << "operand1 == " << getString(instruction->getOperand(1)) << "\n";
		#endif
		ExpressionTree* lhs = getExpressionTree(state,instruction->getOperand(0));
		ExpressionTree* rhs = getExpressionTree(state,instruction->getOperand(1));
		#ifdef DEBUG
			if(lhs) std::cout << "lhs == " << lhs->toString() << "\n";
			else std::cout << "lhs is NULL\n";
			if(rhs) std::cout << "rhs == " << rhs->toString() << "\n";
			else std::cout << "rhs is NULL\n";
		#endif
		state->add(instruction,new ExpressionTree(op,lhs,rhs));
		#ifdef DEBUG
			std::cout << "state updated\n";
		#endif
			//state->variables[getString(instruction).c_str()] = state->c.bool_const(getString(instruction).c_str());
			//state->s->add(state->variables[getString(instruction).c_str()] == state->variables[getString(instruction->getOperand(0)).c_str()] > state->variables[getString(instruction->getOperand(1)).c_str()]);
	}
	#ifdef DEBUG
		std::cout << "exiting executeNonBranchingInstruction\n";
	#endif

}




/**
 Executes a branching instruction and determines which block(s) need to be explored depending on the program state
*/
std::vector<SymbolicTreeNode*> 
	SymbolicExecutor::getNextBlocks(llvm::Instruction* inst, SymbolicTreeNode* node)
{
	std::vector<SymbolicTreeNode*> children;
	ProgramState* state = node->state;
	int pid = node->id;
	if(llvm::isa<llvm::BranchInst>(inst))
	{
		llvm::BranchInst* binst = llvm::dyn_cast<llvm::BranchInst>(inst);
		SymbolicTreeNode* returnNode = node->returnNode;
		if(binst->isConditional())
		{
			ProgramState* first = new ProgramState(*state);
			llvm::Value* cond = binst->getCondition();
			int toAddTrue = 1;
			int toAddFalse = 1;
			bool toAddConstraint = true;
			if(state->getMap()[cond]->isConstant())
			{
				toAddTrue = state->getMap()[cond]->getInteger();
				toAddFalse = !toAddTrue;
				toAddConstraint = false;
			}
			if(toAddTrue)
			{
				if(toAddConstraint) 
				{
					first->constraints.push_back(std::make_pair(cond,"true"));
					first->addCondition(state->get(cond)->toString());
				}
				#ifdef DEBUG
					std::cout << "ADDING CONDITION : " << state->get(cond)->toString() << std::endl;
				#endif
				/*first->printZ3Variables(); 
				bool satisfiableFirst = first->Z3solver();
				if (satisfiableFirst)
				{*/
					children.push_back(new SymbolicTreeNode(binst->getSuccessor(0), 
						first, numNodes++,node->id,NULL,returnNode));
				//}
			}
			int numSuccesors = binst->getNumSuccessors();
			if(numSuccesors == 2 && toAddFalse)
			{
				ProgramState* second = new ProgramState(*state);
				if (toAddConstraint)
				{
					second->constraints.push_back(std::make_pair(cond,"false"));
					second->addCondition("not " + state->get(cond)->toString());
				}
				/*second->printZ3Variables();
				bool satisfiableSecond = second->Z3solver();
				if (satisfiableSecond)
				{*/
					children.push_back(new SymbolicTreeNode(binst->getSuccessor(1), 
						second, numNodes++,node->id,NULL,returnNode));
				//}
			}
		}
		else children.push_back(new SymbolicTreeNode(binst->getSuccessor(0),state, 
			numNodes++,node->id,NULL,returnNode));
	}
	else if (llvm::isa<llvm::CallInst>(inst))
	{
		llvm::CallInst* callInst = llvm::dyn_cast<llvm::CallInst>(inst);
		llvm::Function* calledFunction = callInst->getCalledFunction();
		llvm::BasicBlock* funcStart = NULL;
		if(calledFunction->isDeclaration())
		{
			#ifdef DEBUG
				std::cout << "EXTERNAL CALL\n";
			#endif

			node->isModel = true;
			// node->model = LLVM::VALUE;
		}
		else
		{
			std::cout << "INTERNAL CALL\n";
			funcStart = &calledFunction->getEntryBlock();
		}
		std::cout << getString(callInst->getCalledValue());
		std::vector<ExpressionTree*> arguments;
		for(auto arg = callInst->op_begin(); arg != callInst->op_end(); arg++)
		{
			arguments.push_back(state->get(arg->get()));
		}
		ProgramState* newState = new ProgramState(calledFunction->args(),arguments);
		ProgramState::Copy(*state,newState,false);
		children.push_back(new SymbolicTreeNode(funcStart,newState, numNodes++,node->id,NULL,node)); 
		
	}
	else if (llvm::isa<llvm::ReturnInst>(inst))
	{
		if(node->returnNode)
		{
			llvm::ReturnInst* returninst = llvm::dyn_cast<llvm::ReturnInst>(inst);
			SymbolicTreeNode* returnNode = node->returnNode;
			auto pair = getReturnState(node,state->get(returninst->getReturnValue()));
			ProgramState* newState = pair.first;
			InstructionPtr* returnptr = pair.second;
			children.push_back(new SymbolicTreeNode(returnNode->block,newState,
								numNodes++,node->id,returnptr,returnNode->returnNode));	
		}
		
	}

	#ifdef DEBUG
		std::cout << "exiting getNextBlocks\n";
	#endif

	return children;
}
std::pair<ProgramState*,InstructionPtr*> getReturnState(SymbolicTreeNode* node, ExpressionTree* returnValue)
{
	SymbolicTreeNode* returnNode = node->returnNode;
	ProgramState* state = node->state;
	ProgramState* newState = new ProgramState(*(returnNode->state)); // copy everything from parent state
	ProgramState::Copy(*state,newState,false); // replace everything except map from curr state
	InstructionPtr* returnptr = NULL;
	if(returnNode)
	{
		returnptr = new InstructionPtr(returnNode->getPreviousInstruction());
		newState->add(*returnptr,returnValue);
		(*returnptr)++;
	}
	returnNode->getNextInstruction(); // DONT REMOVE
	return std::make_pair(newState,returnptr);
}
bool isSplitPoint(llvm::Instruction* instruction)
{
	return instruction->getOpcode() == llvm::Instruction::Br 
		   || instruction->getOpcode() == llvm::Instruction::Ret
		   || ( llvm::isa<llvm::CallInst>(instruction) 
				&& !llvm::isa<llvm::DbgDeclareInst>(instruction) );
}
std::vector<SymbolicTreeNode*>
	SymbolicExecutor::executeModel(SymbolicTreeNode* symTreeNode)
{
	std::vector<SymbolicTreeNode*> to_ret;
	for (int i = 0; i < (symTreeNode->modelVals).size(); i++)
	{
		// ProgramState * s  = new ProgramState(*(symTreeNode->state));
		auto pair = getReturnState(symTreeNode,symTreeNode->modelVals[i].first);
		ProgramState* newState = pair.first;
		newState->addCondition(symTreeNode->modelVals[i].second);
		InstructionPtr* returnptr = pair.second;
		//s->add((returnvalue, symTreeNode->modelVals[i].first);
		SymbolicTreeNode * node = new SymbolicTreeNode(symTreeNode->returnNode->block,
			newState, numNodes++, symTreeNode->id,returnptr,
			symTreeNode->returnNode->returnNode);
	}
	return to_ret;
}
/**
 executes the basicBlock, updates programstate and returns the next Block(s) to execute if it can be determined that only the "Then" block should be executed then only the "Then" block is returned. Similarly for the else block. Otherwise both are are retuarned. NULL is returned if there's nothing left to execute
 */
std::vector<SymbolicTreeNode*>
	SymbolicExecutor::executeBasicBlock(SymbolicTreeNode* symTreeNode)
{
	if (!symTreeNode->block)
		return executeModel(symTreeNode);
	ProgramState* state = symTreeNode->state;
	llvm::BasicBlock* block = symTreeNode->block;
	
	std::vector<SymbolicTreeNode*> to_ret;
	
	#ifdef DEBUG
		printf("Basic block (name= %s) has %zu instructions\n"
				,block->getName().str().c_str(),block->size());
	#endif

	while(symTreeNode->hasNextInstruction())
	{
		auto instruction = symTreeNode->getNextInstruction();
		if(instruction == symTreeNode->block->end())
		{
			std::cout << "wtf\n";
		}
		#ifdef DEBUG

			std::cout << "printing operands : " << instruction->getNumOperands() << "\n";
			for (int j = 0; j < instruction->getNumOperands(); j++)
			{
				std::cout << "operand # : " << j+1 << " : " << getString(instruction->getOperand(j)) << "\n";
			}
			std::cout << "printing instruction: " << getString(instruction) << "\n";
			std::cout << "getOpcode: " << instruction->getOpcode() << "\n";

		#endif

		if (llvm::MDNode *N = instruction->getMetadata("dbg")) 
		{  
			llvm::DILocation Loc(N);
			unsigned Line = Loc.getLineNumber();
			symTreeNode->minLineNumber = std::min(symTreeNode->minLineNumber,Line);
			symTreeNode->maxLineNumber = std::max(symTreeNode->maxLineNumber,Line);
			std::cout << "instruction == " << getString(instruction) << "\tLine Number == " << Line << "\n";
		}

		if(isSplitPoint(instruction)) 
		{
			#ifdef DEBUG
				std::cout << "Split Point Hit!\n";	
			#endif

			return getNextBlocks(instruction,symTreeNode);
		}
		else 
		{
			#ifdef DEBUG
				std::cout << "non branch instruction to b executed\n";
				if (instruction)
				{ 
					std::cout << getString(instruction) << "\n" << std::endl;
				}
				else
				{
					std::cout << "instruction NULL\n"<< std::endl;
				}
			#endif

			executeNonBranchingInstruction(instruction,symTreeNode);
		}
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
		Json::Value toSend;
		toSend["type"] = Json::Value(MSG_TYPE_EXPANDNODE);
		toSend["fileId"] = Json::Value(filename.c_str());
		reader->updateToSend(toSend);
		reader->initializeJsonArray();

		#ifdef DEBUG
			// std::cout << "prev id: "<< prevId << "\n";
		#endif
		std::deque<SymbolicTreeNode*> deque;
		if (reader->getPrevId() == -1)
		{
			deque.push_back(rootNode);
		}
		else
		{
			SymbolicTreeNode * symTreeNode = BlockStates[reader->getPrevId()];

			if (!symTreeNode)
			{
				#ifdef DEBUG
					std::cout << "tree node is NULL!!\n"; 
				#endif
			}

			if (symTreeNode->left)
			{
				#ifdef DEBUG
					std::cout << "left child started!!\n";
				#endif

				if (reader->getDir())
					deque.push_back(symTreeNode->left);
				else
					deque.push_front(symTreeNode->left);
				
				#ifdef DEBUG
					std::cout << "left child done!!\n"; 
				#endif
			}
			if (symTreeNode->right)
			{
				#ifdef DEBUG
					std::cout << "right child started!!\n"; 
				#endif
				
				if (reader->getDir()) 
					deque.push_back(symTreeNode->right);
				else
					deque.push_front(symTreeNode->right);

				#ifdef DEBUG
					std::cout << "right child done!!\n"; 
				#endif
			}
		}


		for (int i = 0; i < reader->getSteps() || reader->getSteps() == -1; i++)
		{
				int xyz;
			#ifdef DEBUG
				std::cout << "1: size of deque : " << deque.size() << "\n";
				std::cin >> xyz;
			#endif

			if (deque.empty()) break;

			SymbolicTreeNode* symTreeNode = deque.front();
			deque.pop_front();
			// printBlock(symTreeNode->block);

			#ifdef DEBUG
				std::cout << "2: size of deque : " << deque.size() << "\n";
				std::cin >> xyz;
			#endif

			if (excludedNodes.find(symTreeNode->block) != excludedNodes.end())
			{
				#ifdef DEBUG
				std::cout << "is Excluded! : \n";
				std::cin >> xyz;
				#endif
				i--;
				continue;
			}
			if (symTreeNode->isExecuted)
			{
				#ifdef DEBUG
					std::cout << "Already executed" << "\n";
					std::cin >> xyz;
				#endif
				i--;
				for (int j = 0; j < 2; j++)
				{
					int k = j;
					if (reader->getDir()) k = 1 - j;

					SymbolicTreeNode * tempSymTreeNode = NULL;
					if (k == 0)
					{
						tempSymTreeNode = symTreeNode->left;
						std::cout << "left\n";
						// printBlock(tempSymTreeNode->block);
					}
					else
					{
						tempSymTreeNode = symTreeNode->right;
						std::cout << "right\n";
						// printBlock(tempSymTreeNode->block);

					}
					if (reader->getIsBFS())
					{
						if (tempSymTreeNode)
						{
							deque.push_back(tempSymTreeNode);	
							std::cout << "child added!\n";
						}
						else
						{	
							std::cout << "shit happenS!\n";
						}  
						
					}
							
					else
					{
						if (tempSymTreeNode)
						{
							deque.push_front(tempSymTreeNode);
							std::cout << "child added!\n";
						}
						else
						{	
							std::cout << "shit happenS!\n";
						} 
						
					}
				}
				continue;	
			}
			std::vector<SymbolicTreeNode*> new_blocks = executeBasicBlock(symTreeNode);
			symTreeNode->isExecuted = true;
			#ifdef DEBUG
					printBlock(symTreeNode->block);
					std::cout << "Executed !" << "\n";
					std::cin >> xyz;
				#endif
			BlockStates[symTreeNode->id]=symTreeNode;
			
			Json::Value msg;
			msg["node"] = Json::Value(symTreeNode->id);
			msg["parent"] = Json::Value(symTreeNode->getPrevId());
			msg["text"] = Json::Value(symTreeNode->state->toString());
			msg["fin"] = Json::Value("0");
			msg["constraints"] = Json::Value(symTreeNode->state->getPathCondition());
			msg["startLine"] = Json::Value(symTreeNode->minLineNumber);
			msg["endLine"] = Json::Value(symTreeNode->maxLineNumber);
			msg["addModel"] = Json::Value("false");
			reader->addObject(msg);
			
			if (symTreeNode->isModel)
			{
				symTreeNode->modelVals = reader->getModel(symTreeNode->state->getUserVarMap());
			 	toSend.clear();
				toSend["type"] = Json::Value(MSG_TYPE_EXPANDNODE);
				toSend["fileId"] = Json::Value(filename.c_str());
				reader->updateToSend(toSend);
				reader->initializeJsonArray();
			}
			
			
			#ifdef DEBUG
				std::cout << "number of blocks :  " << new_blocks.size() << "\n";
				std::cin >> xyz;
			#endif

			for (int j = 0; j < new_blocks.size(); j++)
			{
				int k = j;
				if (reader->getDir()) k = new_blocks.size() - 1 - j;

				SymbolicTreeNode * tempSymTreeNode;
				if (k == 0)
				{

					symTreeNode->left = new_blocks[k];
					tempSymTreeNode = symTreeNode->left;
					#ifdef DEBUG
						std::cout << "left child done!!\n";
						if (!symTreeNode->right)
							std::cout << "right child NULL!!\n";
						std::cin >> xyz;
					#endif
				}
				else
				{
					symTreeNode->right = new_blocks[k];
					tempSymTreeNode = symTreeNode->right;
					#ifdef DEBUG
						std::cout << "right child done!!\n";
						if (!symTreeNode->left)
							std::cout << "left child NULL!!\n";
						std::cin >> xyz;
					#endif
				}
				if (reader->getIsBFS())
					deque.push_back(tempSymTreeNode);	
				else
					deque.push_front(tempSymTreeNode);
			}
			#ifdef DEBUG
				std::cout << "3: size of deque : " << deque.size() << "\n";
			#endif
		}
		// #ifdef CIN_SERVER
		// 	// std::cout << "proceed? \n";
		// 	// std::cout << "prevId : ";
		// 	// std::cin >> prevId;
		// 	// std::cout << "isBFS : ";
		// 	// std::cin >> isBFS;
		// 	// std::cout << "steps : ";
		// 	// std::cin >> steps;
		// 	// std::cout << "dir : ";
		// 	// std::cin >> dir; 
		// 	continue;
		// #endif
		reader->proceedSymbolicExecution();
		if (reader->getIsExclude() != -1)
			exclude(reader->getExcludedId(), reader->getIsExclude());
		// add exclude
	}
}


/**
	Executes all the possible paths in the given function and returns the programState at the end of every path
*/

void SymbolicExecutor::executeFunction(llvm::Function* function)
{
	

	for (llvm::Function::iterator b = function->begin(), be = function->end(); b != be; ++b)
	{
		minLineNumber = std::numeric_limits<unsigned int>::max();
		maxLineNumber = 0;
		for (auto instruction = b->begin(), e = b->end(); instruction != e; ++instruction)
		{
			if (llvm::MDNode *N = instruction->getMetadata("dbg")) 
			{  
				llvm::DILocation Loc(N);
				unsigned Line = Loc.getLineNumber();
				minLineNumber = std::min(minLineNumber,Line);
				maxLineNumber = std::max(maxLineNumber,Line);
			}
		}
		for(int i = minLineNumber; i <= maxLineNumber; i++)
		{
			lineToBlock[i] = b;
		}

	}

	auto rootState = new ProgramState(function->args());
	auto rootBlock = &function->getEntryBlock();
	rootNode = new SymbolicTreeNode(rootBlock, rootState, numNodes++,-1);	
	symbolicExecute();
	
	// Json::Value msg;
	// msg["fin"] = Json::Value("1");
	// Json::FastWriter fastWriter;
	// std::string output = fastWriter.write(msg);

	// std::cout << "sending this: " << output << std::endl;
	// (*socket) << output;
	// std::cout << "Function executed" << std::endl;
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
void SymbolicExecutor::proceed(Json::Value val)
{
	reader->wakeUp(val);
}

void SymbolicExecutor::execute(Json::Value val)
{
    reader->updateMsg(val);
    reader->setExecutionVars();
	
	llvm::Module* module = loadCode(filename.c_str());

	auto function = module->getFunction("_Z7notmainii");
	executeFunction(function);
}

void SymbolicExecutor::exclude(int input, int isNode)
{ 
	std::cout << "input : " << input << "\n";
	std::cout << "isNode : " << isNode << "\n";
	unsigned int minLine = input;
	unsigned int maxLine = input;
	llvm::BasicBlock * b;
	if (!isNode)
	{
		b = lineToBlock[input-minLineNumber];
	}
	else
	{
		b = BlockStates[input]->block;
		excludedNodes[b] = true;
	}	
	for (auto instruction = b->begin(), e = b->end(); instruction != e; ++instruction)
	{
		if (llvm::MDNode *N = instruction->getMetadata("dbg")) 
		{  
			llvm::DILocation Loc(N);
			unsigned Line = Loc.getLineNumber();
			minLine = std::min(minLine,Line);
			maxLine = std::max(maxLine,Line);
		}
	}
	
	Json::Value val;
	val["fileId"] = Json::Value(filename.c_str());
	val["type"] = Json::Value(MSG_TYPE_EXCLUDENODE);
	val["minLine"] = Json::Value(minLine);
	val["maxLine"] = Json::Value(maxLine);
	reader->updateToSend(val);

	reader->proceedSymbolicExecution();
	if (reader->getIsExclude() != -1)
		exclude(reader->getExcludedId(), reader->getIsExclude());
}

