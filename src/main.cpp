#define DEBUG 1
#include "symbolicexecutor.h"
#include "server/SocketException.h"
#include "utils.h"




std::map< std::string, std::pair<std::thread,SymbolicExecutor*> > threads_sym;

void runOnThread(SymbolicExecutor * sym, Json::Value val)
{
  sym->execute(val["isBFS"].asInt(), val["steps"].asInt(), 0, val["prevId"].asInt());
}

int executeSym(Json::Value val, ServerSocket * s)
{
  if (threads_sym.find(val["id"].asString()) == threads_sym.end())
  {
    SymbolicExecutor * sym = new SymbolicExecutor(val["id"].asString(), s);
    threads_sym[val["id"].asString()] = std::make_pair(std::thread(runOnThread,sym,val),sym);
  }
  else
  {
    threads_sym[val["id"].asString()].second->proceed(val["isBFS"].asInt(), 
        val["steps"].asInt(), 0, val["prevId"].asInt());
  }
}

int communicate(ServerSocket* new_sock)
{
    std::cout << "new connection established\n";
    std::string message;
    while(true)
    {
        message = "";
        (*new_sock) >> message;
        std::cout << "recieved : \n" << message << "\n";
        if(message == "FIN")
        {
            break;
        }
        Json::Reader reader;
        Json::Value val;
        bool isParsed = reader.parse(message, val);
        if (isParsed)
        {
            std::cout << "isBFS : " << val["isBFS"].asInt() << "\n";
            std::cout << "branch : " << val["branch"].asInt() << "\n";
            std::cout << "steps : " << val["steps"].asInt() << "\n";
            std::cout << "prevId : " << val["prevId"].asInt() << "\n";
            std::cout << "id : " << val["id"].asString() << "\n";
            std::cout << "proceed? : \n";
            int abc;
            std::cin >> abc;
            executeSym(val, new_sock);
        }
    }
    delete new_sock;
}

int main ()
{
  try
  {
    ServerSocket server ( 30000 );
    std::thread tempThread;
    ServerSocket* new_sock;
    while(true)
    {
        new_sock = new ServerSocket();
        try
        {
          server.accept ( *new_sock );
          tempThread = std::thread(communicate,new_sock);
          // tempThread.join();
          std::cout << "Reached here";
        }
        catch ( SocketException& ) {}
    }
  }
  catch ( SocketException& e )
  {
    std::cout << "Exception was caught:" << e.description() << "\n";
  }
  return 0;
}
