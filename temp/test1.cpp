#include <fstream>
#include <iostream>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Casting.h>
#include <iostream>


std::string getString(llvm::Value* val)
{
    std::string str;
    llvm::raw_string_ostream TmpStr(str);
    val->print(TmpStr);
    return TmpStr.str();
}
int main()
{
	std::string a = "/media/ACER/Users/Shalan/Dropbox/SHALAN/LUMS/sproj/llvm-stuff/SUT/hello.bc";
	auto Buffer = llvm::MemoryBuffer::getFileOrSTDIN(a.c_str());
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
    llvm::Module* module = *mainModuleOrError;
  	llvm::Function *mainFn = module->getFunction("main");

  	if (!mainFn) 
  	{
    	printf("'main' function not found in module.\n");
    	return -1;
  	}
  	else
  	{
  		printf("%s\n",mainFn->getName().str().c_str());
  	}
    int branches = 0;
  

  for (auto function = module->getFunctionList().begin(), last = module->getFunctionList().end(); function!=last; function++)
  {
        std::cout<<"printing function arguments\n";
        for (auto arg = function->arg_begin(), argLast = function->arg_end(); arg!=argLast; arg++ )
        {

            std::cout << getString(arg) << "\n";
        }
        std::cout << "End Arguments\n\n";
    for (llvm::Function::iterator i = function->begin(), e = function->end(); i != e; i++)
    {  // Print out the name of the basic block if it has one, and then the
        // number of instructions that it contains
        printf("Basic block (name= %s) has %zu instructions\n",i->getName().str().c_str(),i->size());

        for (llvm::BasicBlock::iterator i2 = i->begin(), e2 = i->end(); i2 != e2; i2++)
        {
          if (i2 -> getOpcode() == llvm::Instruction::Br) branches++;
          std::cout << "printing operands ";
          for (int j = 0; j < i2->getNumOperands(); j++)
          {
            std::cout << getString(i2->getOperand(j)) << "\n";
          }
          std::cout << getString(i2) << "\n";
            // std::cout << (*i2).print();
      		   // The next statement works since operator<<(ostream&,...)
      		   // is overloaded for Instruction&
      		 //std::cout << *(i2).str().c_str() << "\n";
  		  }
  	}
  }
	printf("total branches in the code %d \n", branches);
	return 0;
}