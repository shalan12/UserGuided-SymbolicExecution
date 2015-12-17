#include "utils.h"
#include <sstream>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/Constants.h>
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


