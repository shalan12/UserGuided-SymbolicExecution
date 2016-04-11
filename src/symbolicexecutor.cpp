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
	else return new ExpressionTree(value, state->getUserVarMap(), state->getLLVMVarMap());
}

/**
* Updates the program state according to the instruction executed
*/
void SymbolicExecutor::executeNonBranchingInstruction(llvm::Instruction* instruction,SymbolicTreeNode* symTreeNode)
{
	ProgramState* state = symTreeNode->state;
	#ifdef DEBUG	
		std::cout << " executing :" << instruction->getOpcodeName() << " instruction \n";
		std::cout << "State at this point == \n------------------" << state->toString() << "\n";
	#endif

	if (llvm::isa<llvm::BinaryOperator>(instruction))
	{
		#ifdef DEBUG	
			std::cout << "Binary Operator";
		#endif
		std::string op;
		switch(instruction->getOpcode())
		{

			case llvm::Instruction::Add :
				op = "+";
				break;
			case llvm::Instruction::Sub :
				op = "-";
				break;
			case llvm::Instruction::Mul :
				op = "*";
				break;
			case llvm::Instruction::SDiv :
				op = "/";
				break;
			case llvm::Instruction::Shl :
				op = "<<";
				break;
			case llvm::Instruction::AShr :
				op = ">>";
				break;
			case llvm::Instruction::SRem :
				op = "%";
				break;
			default :
				#ifdef DEBUG	
					std::cout << "Not Implemented"; // change std::cout to some log file
				#endif
				break;
		}
		ExpressionTree* lhs = getExpressionTree(state,instruction->getOperand(0));
		ExpressionTree* rhs = getExpressionTree(state,instruction->getOperand(1));
		state->add(instruction, new ExpressionTree(op,lhs,rhs));
	}

	else if (instruction->getOpcode() == 49)
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
		state->addStore(value, memLocation);
		state->add(memLocation,getExpressionTree(state,value));
	}
	else if(instruction->getOpcode()==llvm::Instruction::Load)
	{
		ExpressionTree* exptree = getExpressionTree(state,instruction->getOperand(0));
		state->add(instruction,exptree);
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
		else if(cmpInst->getSignedPredicate() == llvm::ICmpInst::ICMP_EQ)
			op = "==";
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
* Executes a branching instruction and determines which block(s) need to be explored 
* depending on the program state
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
			bool toAddConstraint = true;
			if(state->getMap()[cond]->isConstant())
			{
				toAddConstraint = false;
			}
			if(toAddConstraint) 
			{
				first->addCondition(state->get(cond)->toStringHumanReadable(state->getLLVMVarMap(), state->getStoreMap()));
			}
			
			first->z3Constraints.push_back(std::make_pair(state->get(cond)->toZ3Expression(first->z3Variables, first->context),"true")); // added for z3
			
			#ifdef DEBUG
				std::cout << "ADDING CONDITION : " << state->get(cond)->toString() << std::endl;
			#endif

			// added for constraint checking
			bool satisfiableFirst = first->Z3solver();

			#ifdef DEBUG
				if (satisfiableFirst)
					std::cout << "branch is satisfiable \n ";
				else
					std::cout << "branch is not satisfiable \n ";
			#endif
			bool hasRunMaxExecs = (node->loopInfo.loopStartPoint != NULL
							&& node->loopInfo.loopStartPoint == node->block
							&& node->loopInfo.numExecutions >= node->loopInfo.maxExecutions);
				
			if (!hasRunMaxExecs)
			{
				// added for limiting loop executions
				SymbolicTreeNode* toPush = new SymbolicTreeNode(binst->getSuccessor(0), 
						first, numNodes++,node->id,NULL,returnNode);
				toPush->setLoopInfo(node->loopInfo.loopStartPoint,
									node->loopInfo.numExecutions);
				if(!satisfiableFirst)
				{
					node->setSATInfo(false);
				}			
				children.push_back(toPush);
			}

			int numSuccesors = binst->getNumSuccessors();
			if(numSuccesors == 2)
			{
				ProgramState* second = new ProgramState(*state);
				
				second->z3Constraints.push_back(std::make_pair(state->get(cond)->toZ3Expression(second->z3Variables, second->context),"false")); // added for z3
				
				if (toAddConstraint)
				{
					second->addCondition("!" + state->get(cond)->toStringHumanReadable(state->getLLVMVarMap(), state->getStoreMap()));
				}
				// added for constraint checking
				bool satisfiableSecond = second->Z3solver();
				SymbolicTreeNode* toPush = new SymbolicTreeNode(binst->getSuccessor(1), 
								second, numNodes++,node->id,NULL,returnNode);
				toPush->setLoopInfo(node->loopInfo.loopStartPoint,
								node->loopInfo.numExecutions);	
				if (!satisfiableSecond)
				{
					toPush->setSATInfo(false);
				}
				children.push_back(toPush);

			}
		}
		else 
		{
			llvm::BasicBlock * successorZero = binst->getSuccessor(0);
			std::cout << "startLine of successor = " << getMinLineNumber(successorZero) << "\n";
			std::cout << "startLine of curr = " << getMinLineNumber(node->block) << "\n";
			SymbolicTreeNode* toPush = new SymbolicTreeNode(successorZero,state, 
			numNodes++,node->id,NULL,returnNode);
			if (getMinLineNumber(successorZero) < getMinLineNumber(node->block))
			{
				toPush->setLoopInfo(successorZero, node->loopInfo.numExecutions+1);
				std::cout << "Back Jump\nNumExecs = " << node->loopInfo.numExecutions+1 << "\n";
			}
			else
			{
				toPush->setLoopInfo(node->loopInfo.loopStartPoint,
				node->loopInfo.numExecutions);
			}
			children.push_back(toPush);
		}
	}
	else if (llvm::isa<llvm::CallInst>(inst))
	{
	llvm::CallInst* callInst = llvm::dyn_cast<llvm::CallInst>(inst);
		llvm::Function* calledFunction = callInst->getCalledFunction();
		auto attrs = calledFunction->getAttributes();
		unsigned int attrsCount = attrs.getNumSlots();
		std::cout << "get numSlots : " << attrsCount << "\n";
		std::cout << "get function name : " << calledFunction->getName().str() << "\n";
		#ifdef DEBUG
			for (auto i = 0; i < attrsCount; i++)
			{
				int xxyyzz;
				std::cout << "attr : " << attrs.getAsString(i) << "\n";
				std::cin >> xxyyzz;
			}
		#endif

		#ifdef DEBUG
			for (auto arg = calledFunction->getArgumentList().begin(); arg != calledFunction->getArgumentList().end(); arg++ )
			{
				llvm::DbgValueInst * val = llvm::dyn_cast<llvm::DbgValueInst>(arg);
				if (val)
				{
					int xxyyzz;
					std::cout << "something happened\n"; 
	 				std::cin >>  xxyyzz;
				}
				else
				{	
					int xxyyzz;
					std::cout << arg->getName().str(); 
					std::cout << "something bad happened\n"; 
					std::cin >>  xxyyzz;
				}
			}
		#endif
		llvm::BasicBlock* funcStart = NULL;
		int xxyyzz;
		if(calledFunction->isDeclaration())
		{
			#ifdef DEBUG
				std::cout << "EXTERNAL CALL\n";
			#endif

			for(auto arg = callInst->op_begin(); arg != callInst->op_end(); arg++)
			{
				ExpressionTree* tree = state->get(arg->get());

				std::cout << "llvm Value in external call : " << getString(arg->get()) << "\n";
				std::vector<llvm::Value*> vals;
				for (auto& pr : state->getMap())
				{
					std::cout << "llvm Value in user var map : " << getString(pr.first) << "\n";

					if (pr.second == tree)
					{
						vals.push_back(pr.first);
						#ifdef DEBUG
							std::cout << "discovered : " << getString(pr.first) << "\n";
							std::cin >> xxyyzz;
						#endif
					}
				}

				for (auto v : vals)
				{
					for (auto& pr : state->getUserVarMap())
					{
						std::cout << "llvm Value in user var map : " << getString(pr.second) << "\n";

						if (pr.second == v)
						{
							#ifdef DEBUG
								std::cout << "discovered : " << pr.first << "\n";
								std::cin >> xxyyzz;
							#endif
						}
					}	
				}

				// state->get(arg->get());

				// arguments.push_back(state->get(arg->get()));
			}
			#ifdef DEBUG
				for (auto arg = calledFunction->getArgumentList().begin(); arg != calledFunction->getArgumentList().end(); arg++ )
				{
					llvm::Value * val = llvm::dyn_cast<llvm::Value>(arg);
					if (val)
					{
						std::cout << "llvm Value : " << getString(val) << "\n";
						std::cin >> xxyyzz;
						std::cout << "something happened\n"; 
		 				std::cin >>  xxyyzz;
		 				for (auto& pr : state->getUserVarMap())
						{
							std::cout << "llvm Value in user var map : " << getString(pr.second) << "\n";

							if (pr.second == val)
							{

								std::cout << "discovered : " << pr.first << "\n";
								std::cin >> xxyyzz;
							}
						}
					}
					else
					{	
						int xxyyzz;
						std::cout << arg->getName().str(); 
						std::cout << "something bad happened\n"; 
						std::cin >>  xxyyzz;
					}
				}
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

/**
* Detect whether a branch is conditional branch instruction of an unconditional branch
*/

bool isSplitPoint(llvm::Instruction* instruction)
{
	return instruction->getOpcode() == llvm::Instruction::Br 
		   || instruction->getOpcode() == llvm::Instruction::Ret
		   || ( llvm::isa<llvm::CallInst>(instruction) 
				&& !llvm::isa<llvm::DbgDeclareInst>(instruction) );
}

/**
* executes the model for a function provided by the user, updates programstate 
* and returns the next Block(s) to execute
* if it can be determined that only the "Then" block should be executed then only the 
* "Then" block is returned. Similarly for the else block. 
 */

std::vector<SymbolicTreeNode*>
	SymbolicExecutor::executeModel(SymbolicTreeNode* symTreeNode)
{
	int xyz;
	#ifdef DEBUG
		std::cout << "executing Model\n";
	#endif

	std::vector<SymbolicTreeNode*> to_ret;
	for (int i = 0; i < (symTreeNode->modelVals).size(); i++)
	{
		// ProgramState * s  = new ProgramState(*(symTreeNode->state));
		auto pair = getReturnState(symTreeNode,symTreeNode->modelVals[i].first);
		ProgramState* newState = pair.first;
		newState->addCondition(symTreeNode->modelVals[i].second);
		InstructionPtr* returnptr = pair.second;
		//s->add((returnvalue, symTreeNode->modelVals[i].first);
		to_ret.push_back(new SymbolicTreeNode(symTreeNode->returnNode->block,
			newState, numNodes++, symTreeNode->id,returnptr,
			symTreeNode->returnNode->returnNode));
	}
	#ifdef DEBUG
		std::cout << "exiting execute Model with blocks : " << to_ret.size() << " \n";
		std::cin >> xyz;
	#endif
	return to_ret;
}

/**
* executes the basicBlock, updates programstate and returns the next Block(s) to execute
* if it can be determined that only the "Then" block should be executed then only the 
* "Then" block is returned. Similarly for the else block. 
* Otherwise both are are returned. NULL is returned if there's nothing left to execute
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
			if (llvm::CallInst* callInst = llvm::dyn_cast<llvm::CallInst>(instruction))
			{
				llvm::Function* calledFunction = callInst->getCalledFunction();
				if(calledFunction->getName().str() == "_ZN2se13unknown_errorEv")
				{
					#ifdef DEBUG
					std::cout << "Error State Reached";
					#endif
					continue;
				}
			}
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

/**
* Symbolically execute blocks till its out of instructions on how to proceed and then
* waits for the webserver to communicate further instructions
*/
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
				#ifdef DEBUGEXCLUDE
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
				if (symTreeNode->satInfo.isSatisfiable)
				{ 
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
				}
				continue;	
			}
			std::vector<SymbolicTreeNode*> new_blocks;
			if(symTreeNode->satInfo.isSatisfiable)
				new_blocks = executeBasicBlock(symTreeNode);
			
			symTreeNode->isExecuted = true;
			#ifdef DEBUG
				// printBlock(symTreeNode->block);
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
			if (symTreeNode->block)
			{
				msg["startLine"] = Json::Value(getMinLineNumber(symTreeNode->block));
				msg["endLine"] = Json::Value(getMaxLineNumber(symTreeNode->block));
			}
			else
			{
				msg["startLine"] = Json::Value(0);
				msg["endLine"] = Json::Value(0);
			}
			
			msg["addModel"] = Json::Value("false");
			if(! symTreeNode->satInfo.isSatisfiable)
			{
				Json::Value extra;
				extra["isSatisfiable"] = Json::Value(false);
				msg["extra"] = extra;
			}
			reader->addObject(msg);
			
			
			
			
			
			#ifdef DEBUG
				std::cout << "number of blocks :  " << new_blocks.size() << "\n";
				std::cin >> xyz;
			#endif

			std::cout << "iterating blocks!\n";
			for (int j = 0; j < new_blocks.size(); j++)
			{
				int k = j;
				if (reader->getDir()) k = new_blocks.size() - 1 - j;

				SymbolicTreeNode * x = new_blocks[j];
				if (x->satInfo.isSatisfiable == false)
				{
					std:: cout << "no statisfiable";
				}

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

				if (symTreeNode->isModel)
				{
					tempSymTreeNode->modelVals = reader->getModel(
						symTreeNode->state->getUserVarMap(),
						symTreeNode->state->getLLVMVarMap(),
						symTreeNode->state->getMap());
				 	std::cout << "modelVals size : " << tempSymTreeNode->modelVals.size() << "\n";
				 	toSend.clear();
					toSend["type"] = Json::Value(MSG_TYPE_EXPANDNODE);
					toSend["fileId"] = Json::Value(filename.c_str());
					reader->updateToSend(toSend);
					reader->initializeJsonArray();
					// i = 0;
				}
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
* Extracts the names of all the functions that are being called in the given program
*/
void SymbolicExecutor::printAllFunctions(llvm::Function* function, int index, Json::Value & functions)
{
	std::cout << "get function name : " << function->getName().str() << "\n";
	// if (function->getAllMetadata())
	// {
	// 	std::cout << "this func has some thing special!\n";
	// }
	std::string name = function->getName().str();
	unsigned int minLineNumber = std::numeric_limits<unsigned int>::max();
	unsigned int maxLineNumber = 0;
	for (llvm::Function::iterator b = function->begin(), be = function->end(); b != be; ++b)
	{
		for (auto instruction = b->begin(), e = b->end(); instruction != e; ++instruction)
		{
			if (llvm::MDNode *N = instruction->getMetadata("dbg")) 
			{  
				llvm::DILocation Loc(N);
				unsigned Line = Loc.getLineNumber();
				minLineNumber = std::min(minLineNumber,Line);
				maxLineNumber = std::max(maxLineNumber,Line);
			}

			if (( llvm::isa<llvm::CallInst>(instruction) 
				&& !llvm::isa<llvm::DbgDeclareInst>(instruction) ))
			{
				llvm::CallInst* callInst = llvm::dyn_cast<llvm::CallInst>(instruction);
				llvm::Function* calledFunction = callInst->getCalledFunction();
				llvm::Value* v=callInst->getCalledValue();
			    llvm::Value* sv = v->stripPointerCasts();
			    std::cout << "called function name : " << sv->getName().str() << "\n";
				printAllFunctions(calledFunction, index++, functions);
			}
		}
	}
	Json::Value val;
	val["name"] = Json::Value(name);
	val["minLine"] = Json::Value(minLineNumber);
	val["maxLine"] = Json::Value(maxLineNumber);
	functions[index] = val;
}

/**
* Start symbolic execution of given function in the input program
*/
void SymbolicExecutor::executeFunction(llvm::Function* function)
{
	int xyz;
	std::cout << "start printing all the  functions that could be  called \n";
	Json::Value functions;
	functions["type"] = MSG_TYPE_FUNCNAMES;
	functions["fileId"] = Json::Value(filename.c_str());
	functions["functions"] =  Json::arrayValue;
	printAllFunctions(function, 0, functions["functions"]);
	reader->updateToSend(functions);
	reader->proceedSymbolicExecution();
	#ifdef DEBUG
		std::cout << "done printing all the  functions that could be  called \n";
		std::cin >> xyz;
	#endif
	for (llvm::Function::iterator b = function->begin(), be = function->end(); b != be; ++b)
	{

		unsigned int minLineNumber = std::numeric_limits<unsigned int>::max();
		unsigned int maxLineNumber = 0;
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
			if (lineToBlock.find(i) == lineToBlock.end())
				lineToBlock[i] = std::vector< llvm::BasicBlock* >(0);
			lineToBlock[i].push_back(b);
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


/**
* Excutes the function containing 'notamin' in its name in the input program -- 
* just for testing purposes
*/
void SymbolicExecutor::execute(Json::Value val)
{
	llvm::Module* module = loadCode(filename.c_str());
	std::string name = "";

	for (auto f = module->begin(); f!= module->end(); f++)
	{
		name = f->getName().str();
		if (name.find("notmain")!=std::string::npos) break;
	}
	if (name != "") 
	{
		auto function = module->getFunction(name);
		executeFunction(function);
	}
	else std::cout << "[DEBUG] function not found";
}

/**
* Marks lines or blocks of code excluded for the Symbolic executor, which user 
* dont want to explore 
*/
void SymbolicExecutor::exclude(int input, int isNode)
{ 
	std::cout << "input : " << input << "\n";
	std::cout << "isNode : " << isNode << "\n";
	unsigned int minLine = std::numeric_limits<unsigned int>::max();
	unsigned int maxLine = std::numeric_limits<unsigned int>::min();
	std::vector< llvm::BasicBlock* > blocks;
	
	if (!isNode)
	{
		blocks = lineToBlock[input];
	}
	else
	{
		llvm::BasicBlock * b = BlockStates[input]->block;
		blocks.push_back(b);
	}
	for (auto b : blocks)
	{
		excludedNodes[b] = true;
		std::cout << "excluded block : \n";
			int xyz;
		#ifdef DEBUG
			printBlock(b);
			std::cin >> xyz;
		#endif
		for (auto instruction = b->begin(), e = b->end(); instruction != e; instruction++)
		{
			if (llvm::MDNode *N = instruction->getMetadata("dbg")) 
			{  
				llvm::DILocation Loc(N);
				unsigned int Line = Loc.getLineNumber();
				minLine = std::min(minLine,Line);
				maxLine = std::max(maxLine,Line);
			}
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

