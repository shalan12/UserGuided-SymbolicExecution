#include "jsoncpp/dist/json/json.h"
#include "server/ServerSocket.h"
#include "llvmExpressionTree.h"
#include <condition_variable>
#include <utility>      
#include <mutex>              // std::mutex, std::unique_lock

/**
* A helper class for Symbolic Executor to handle all the JSON objects that contain information about how to proceed on
* symbolic execution and also to communicate with the webserver 
*/

class JsonReader
{
	private:
		Json::Value toSend, nodes, msg;
	    ServerSocket * socket;
	    bool isBFS, dir;
    	int steps, prevId, excludedId, isExclude, jsonArrSize;
		std::mutex mtx;
	    std::condition_variable cv;

		void sendMessageAndSleep();
	
	public:
		JsonReader(ServerSocket * s);
		void proceedSymbolicExecution();
		std::vector<std::pair<ExpressionTree*, std::string> > getModel(std::map<std::string,
		 llvm::Value*> userVarMap,
		 std::map<llvm::Value*, std::string> llvmVarMap,
		 std::map<llvm::Value*,ExpressionTree*> map);
		void wakeUp(Json::Value val);
		void setExecutionVars();
		void updateMsg(Json::Value val);
		void updateToSend(Json::Value val);
		bool getIsBFS(){return isBFS;}
		bool getDir(){return dir;}
		int getSteps(){return steps;}
		int getPrevId(){return prevId;}
		int getIsExclude(){return isExclude;}
		int getExcludedId(){return excludedId;}
		void addObject(Json::Value obj);
		void modelRequiredForLast();
		void initializeJsonArray();
		void clearNodes();
};