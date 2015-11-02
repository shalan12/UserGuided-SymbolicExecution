#ifndef LLVM_UTILS_H
#define LLVM_UTILS_H
#include <llvm/IR/Value.h>
#include <iostream>
#include "jsoncpp/dist/json/json.h"


std::string getString(llvm::Value* val);
int getInteger(llvm::Value* value);

std::string toString(Json::Value val);

#endif