#include "jsoncpp/dist/json/json.h"
#include "server/ServerSocket.h"
#include "llvmExpressionTree.h"
#include <condition_variable>
#include <utility>      
#include <mutex>              // std::mutex, std::unique_lock

class JsonReader
{
	private:
		Json::Value msg;
	    ServerSocket * socket;
	    bool isBFS, dir;
    	int steps, prevId;
		std::mutex mtx;
	    std::condition_variable cv;

		void sendMessageAndSleep(Json::Value toSend);
	
	public:
		JsonReader(ServerSocket * s);
		void proceedSymbolicExecution(Json::Value toSend);
		std::vector<std::pair<ExpressionTree*, std::string> > getModel(
			Json::Value toSend, std::map<std::string, llvm::Value*> userVarMap);
		void wakeUp(Json::Value val);
		void setExecutionVars();
		void updateMsg(Json::Value val);
		bool getIsBFS(){return isBFS;}
		bool getDir(){return dir;}
		int getSteps(){return steps;}
		int getPrevId(){return prevId;}
};