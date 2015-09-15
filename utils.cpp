#ifndef LLVM_UTILS_H
#define LLVM_UTILS_H
#include <llvm/Support/raw_ostream.h>
#include <iostream>

std::string getString(llvm::Value* val)
{
    std::string str;
    llvm::raw_string_ostream TmpStr(str);
    val->print(TmpStr);
    return TmpStr.str();
}
#endif