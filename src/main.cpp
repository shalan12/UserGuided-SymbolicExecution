#include "symbolicexecutor.h"
#include "server/SocketException.h"
#include "utils.h"
#include "jsoncpp/dist/json/json.h"




std::map< std::string, std::pair<std::thread,SymbolicExecutor*> > threads_sym;

void runOnThread(SymbolicExecutor * sym, Json::Value val)
{
  sym->execute(val);
}

int communicate(ServerSocket* new_sock)
{
    #ifdef DEBUG
      std::cout << "new connection established\n";
    #endif  
    std::string message;
    while(new_sock)
    {
        message = "";
        (*new_sock) >> message;
        #ifdef DEBUG
          std::cout << "recieved : \n" << message << "\n";
        #endif
        Json::Reader reader;
        Json::Value val;
        bool isParsed = reader.parse(message, val);
        if (isParsed)
        {
          #ifdef DEBUG
            std::cout << "successfully parsed\n";
          #endif
          std::string id = val["id"].asString();
          Json::Value val1 = val;
          val = val["val"];
          
          if (threads_sym.find(id) != threads_sym.end() && val1["type"].asString() == "300")
          {
            #ifdef DEBUG
              std::cout << "aleady present\n erasing it \n"; 
            #endif
            SymbolicExecutor * sym = new SymbolicExecutor(id, new_sock);
            std::thread t1 = std::thread(runOnThread,sym,val);
            std::swap(threads_sym[id].first, t1);
            threads_sym[id].second = sym;
            #ifdef DEBUG
              std::cout << "done \n";
            #endif
            t1.detach();
          }
          else if (threads_sym.find(id) != threads_sym.end())
          {
            threads_sym[id].second->proceed(val);
          }
          else
          {
            SymbolicExecutor * sym = new SymbolicExecutor(id, new_sock);
            threads_sym[id] = std::make_pair(std::thread(runOnThread,sym,val), sym);
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
  sym.execute(getMessage());
  std::cout << "still working" << std::endl;
  return 0;
}
#endif