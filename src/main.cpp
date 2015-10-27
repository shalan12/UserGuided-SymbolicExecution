//#define DEBUG 1
#include "symbolicexecutor.h"
#include "server/SocketException.h"



std::map< std::string, std::pair<std::thread,SymbolicExecutor*> > threads_sym;

void runOnThread(SymbolicExecutor * sym)
{
  // execute(bool isbfs, int stps, int d, int prev)
  sym->execute(false, 1, 0, -1);
}
void createNewSym(std::string file, ServerSocket * s)
{
  SymbolicExecutor * sym = new SymbolicExecutor(file, s);
   // std::thread th = std::thread(rnd, sym);
   // std::thread th = std::thread([sym](){
   //      sym->execute();
   //      };);
  threads_sym[file] = std::make_pair(std::thread(runOnThread,sym),sym);
}

int executeSym(std::string id)
{
  threads_sym[id].second->proceed();
}

int communicate(ServerSocket* new_sock)
{
  std::cout << "new connection established\n";
  std::string message;
  while(true)
  {
      message = "";
     (*new_sock) >> message;
     std::cout << "recieved " << message << "\n";
     if(message == "FIN")
     {
        break;
     }
     if (message.length() > 4)
     {
          std::string type = message.substr(0,4);
          if (type == "file")
          {
            createNewSym(message.substr(5), new_sock);
          }
          else if (type == "exec")
          {
            executeSym(message.substr(5)); 
          }
      }
  }
  delete new_sock;
}

/*int main ()
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
}*/