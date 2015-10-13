#include "../symbolicexecutor.cpp"
#include <thread>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <utility>      // std::pair, std::make_pair



std::map< std::string, std::pair<std::thread,SymbolicExecutor*> > threads_sym;

int createNewSym(std::string file, ServerSocket * s)
{
  SymbolicExecutor * sym = new SymbolicExecutor(file, s); 
  threads_sym[file] = std::pair<std::thread,SymbolicExecutor*>(std::thread([sym](){
        sym->execute();
        }),sym);
}

int executeSym(std::string id)
{
  threads_sym[id].second->proceed();
}

int main ()
{

  try
  {
    ServerSocket server ( 30000 );
    std::string message;

    while(true)
    {
      try
      {
        ServerSocket new_sock;
        server.accept ( new_sock );
        new_sock >> message;
        if (message.length() > 4)
        {
          std::string type = message.substr(0,4);
          if (type == "file")
          {
            createNewSym(message.substr(4), &new_sock);
          }
          else if (type == "exec")
          {
            executeSym(message.substr(4)); 
          }
        }
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

// pthread_mutex_t mut=PTHREAD_MUTEX_INITIALIZER;
// pthread_cond_t con=PTHREAD_COND_INITIALIZER;
