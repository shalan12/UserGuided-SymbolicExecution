#include "utils.h"
#include <sstream>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/BasicBlock.h>
#include <exception>
std::string getString(llvm::Value* val)
{
	if(val == NULL)
		return "value is NULL";
    std::string str;
    llvm::raw_string_ostream TmpStr(str);
    val->print(TmpStr);
    return TmpStr.str();
}
int getInteger(llvm::Value* value)
{	
    if (llvm::ConstantInt* cl = llvm::dyn_cast<llvm::ConstantInt>(value))
    {
      	return cl->getSExtValue();
    }

    else throw std::invalid_argument("not a constant");
}

Json::Value getMessage()
{
    Json::Value value;
    std::string key;
    std::string val;
    int count;
    std::cout << "how many key vall pairs you want? : ";
    std::cin >> count;
    // std::cout << "\n";

    for (int i = 0; i < count; i++)
    {

        std::cout << "Enter a Key : ";
        std::cin >> key;
        std::cout << "Enter a Value : ";
        std::cin >>val;
        value[key] = Json::Value(val);
    }
    return value;
}
unsigned int getMinLineNumber (llvm::BasicBlock* block)
{
    for (auto i = block->begin(); i != block->end(); i++)
    {
        if (llvm::MDNode *N = i->getMetadata("dbg")) 
        {  
            llvm::DILocation Loc(N);
            return Loc.getLineNumber();
        }
    }
    throw "no line number found";
}
unsigned int getMaxLineNumber (llvm::BasicBlock* block)
{
    unsigned int maxLineNumber = 0;
    for (auto i = block->begin(); i != block->end(); i++)
    {
        if (llvm::MDNode *N = i->getMetadata("dbg")) 
        {  
            llvm::DILocation Loc(N);
            maxLineNumber = std::max(maxLineNumber, Loc.getLineNumber());
        }
    }
    return maxLineNumber;
}