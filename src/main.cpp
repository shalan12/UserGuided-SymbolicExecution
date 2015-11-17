#define DEBUG 1
#include "symbolicexecutor.h"
#include "server/SocketException.h"
#include "utils.h"




std::map< std::string, std::pair<std::thread,SymbolicExecutor*> > threads_sym;

void runOnThread(SymbolicExecutor * sym, Json::Value val)
{
  // execute(bool isbfs, int stps, int d, int prev)
  sym->execute(atoi(toString(val["isBFS"]).c_str()), atoi(toString(val["steps"]).c_str()), 
      atoi(toString(val["branch"]).c_str()), atoi(toString(val["prev"]).c_str()));
}

// void createNewSym(std::string file, ServerSocket * s)
// {
//   SymbolicExecutor * sym = new SymbolicExecutor(file, s);
//    // std::thread th = std::thread(rnd, sym);
//    // std::thread th = std::thread([sym](){
//    //      sym->execute();
//    //      };);
//   threads_sym[file] = std::make_pair(std::thread(runOnThread,sym),sym);
// }

int executeSym(Json::Value val, ServerSocket * s)
{
  if (threads_sym.find(toString(val["id"])) == threads_sym.end())
  {
    SymbolicExecutor * sym = new SymbolicExecutor(toString(val["id"]), s);
    threads_sym[toString(val["id"])] = std::make_pair(std::thread(runOnThread,sym,val),sym);
  }
  else
  {
    threads_sym[toString(val["id"])].second->proceed(atoi(toString(val["isBFS"]).c_str()), atoi(toString(val["steps"]).c_str()), 
      atoi(toString(val["branch"]).c_str()), atoi(toString(val["prev"]).c_str()));
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
     std::cout << "recieved " << message << "\n";
     if(message == "FIN")
     {
        break;
     }
     Json::Reader reader;
     Json::Value val;
     reader.parse(message, val);


     if (message.length() > 4)
     {
          std::string type = message.substr(0,4);


          // if (type == "file")
          // {
          //   #ifdef DEBUG
          //     std::cout << "filename recvd : " + message + "\n";
          //   #endif
          //   createNewSym(message.substr(5), new_sock);
          // }
          // else 
          if (type == "exec")
          {
            executeSym(val, new_sock); 
          }
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