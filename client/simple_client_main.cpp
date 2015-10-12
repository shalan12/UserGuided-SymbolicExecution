#include "ClientSocket.h"
#include "SocketException.h"
#include <iostream>
#include <string>

int main ( int argc, int argv[] )
{
  try
    {

      ClientSocket client_socket ( "localhost", 6969 );

      std::string reply;

      try
	{
    std::string tosend = "";
    for (int i = 0; i < 1001; i++)
    {
      tosend+='a';
    }
	  client_socket << tosend;
	  client_socket >> reply;
	}
      catch ( SocketException& ) {}

      std::cout << "We received this response from the server:\n\"" << reply << "\n";;

    }
  catch ( SocketException& e )
    {
      std::cout << "Exception was caught:" << e.description() << "\n";
    }

  return 0;
}
