#ifndef LLVM_UTILS_H
#define LLVM_UTILS_H
#include <llvm/IR/Value.h>
#include <iostream>
#include "jsoncpp/dist/json/json.h"

/**
 Some helper functions
*/

std::string getString(llvm::Value* val);
int getInteger(llvm::Value* value);
Json::Value getMessage();
unsigned int getMinLineNumber (llvm::BasicBlock* block);
unsigned int getMaxLineNumber (llvm::BasicBlock* block);
#endif