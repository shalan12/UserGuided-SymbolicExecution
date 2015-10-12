#include "ClientSocket.h"
#include "SocketException.h"
#include "../symbolicexecutor.cpp"
#include <thread>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <utility>      // std::pair, std::make_pair



std::map< std::string, std::pair<std::thread,SymbolicExecutor*> > threads_sym;

int createNewSym(std::string file)
{
  SymbolicExecutor * s = new SymbolicExecutor(file); 
  threads_sym[file] = std::pair<std::thread,SymbolicExecutor*>(std::thread([s](){
        s->execute();
        }),s);
}

int executeSym(std::string id)
{
  threads_sym[id].first;
}

int main ()
{
  try
    {
      ClientSocket client_socket ( "localhost", 6969 );
      std::string message;

      while(true)
      {
        try
        {
          client_socket >> message;
          if (message.length() > 4)
          {
            std::string type = message.substr(0,4);
            if (type == "file")
            {
              createNewSym(message.substr(4));
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
