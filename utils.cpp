#ifndef LLVM_UTILS_H
#define LLVM_UTILS_H
#include <llvm/Support/raw_ostream.h>
#include <iostream>
#include <string>

std::string getString(llvm::Value* val)
{
    std::string str;
    llvm::raw_string_ostream TmpStr(str);
    val->print(TmpStr);
    return TmpStr.str();
}
/*
void sendMessage(std::string tosend)
{
	try
    {
		ClientSocket client_socket ( "localhost", 6969 );
		try
		{
			client_socket << tosend;
		}
		catch ( SocketException& ) {}
	}
	catch ( SocketException& e )
    {
      std::cout << "Exception was caught:" << e.description() << "\n";
    }
}
*/
#endif