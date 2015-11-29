#include "symbolicexecutor.h"
#include "server/SocketException.h"
#include "utils.h"
#include "jsoncpp/dist/json/json.h"





std::map< std::string, std::pair<std::thread,SymbolicExecutor*> > threads_sym;

void runOnThread(SymbolicExecutor * sym, int isBFS, int isRightBranch, int steps, int prevId)
{
  sym->execute(isBFS, steps, isRightBranch, prevId);
}

int executeSym(int isBFS, int isRightBranch, int steps, int prevId, std::string id, ServerSocket * s)
{
  if (threads_sym.find(id) == threads_sym.end())
  {
    SymbolicExecutor * sym = new SymbolicExecutor(id, s);
    threads_sym[id] = std::make_pair(std::thread(runOnThread,sym,isBFS,isRightBranch,steps,prevId),sym);
  }
  else
  {
    threads_sym[id].second->proceed(isBFS, steps, isRightBranch, prevId);
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
            std::cout << "prevId : " << val["prevId"].asString() << "\n";
            std::cout << "id : " << val["id"].asString() << "\n";
            std::cout << "proceed? : \n";
            if(val["exclude"].asString())
            {
               threads_sym[id].second->exclude(stoi(val["prevId"].asString()));
               continue;
            }
            std::cout << "isBFS : " << val["isBFS"].asString() << "\n";
            std::cout << "branch : " << val["branch"].asString() << "\n";
            std::cout << "steps : " << val["steps"].asString() << "\n";
            #ifdef DEBUG
              int abc;
              std::cin >> abc;
            #endif
            executeSym(stoi(val["isBFS"].asString()), stoi(val["branch"].asString()),
               stoi(val["steps"].asString()), stoi(val["prevId"].asString()), 
               val["id"].asString(), new_sock);
        }
    }
    delete new_sock;
}


#ifndef CIN_SERVER
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
#else
// if debugging, skip communicating with nodejs server
int main()
{
 SymbolicExecutor sym("build/hello.bc", NULL);
 sym.execute(true, 1, 0, -1);
 std::cout << "still working" << std::endl;
 return 0;
}
#endif