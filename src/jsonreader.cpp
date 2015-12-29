#include "jsonreader.h"
#include "jsoncpp/dist/json/json.h"
#include "utils.h" 

JsonReader::JsonReader(ServerSocket * s)
{
	socket = s;
}

void JsonReader::sendMessageAndSleep(Json::Value toSend)
{
	Json::FastWriter fastWriter;
	std::string output = fastWriter.write(toSend);
	std::cout << "sending this: " << output << std::endl;
	if (socket)
		(*socket) << output;
	std::cout << "going to sleep" << std::endl;
	std::unique_lock<std::mutex> lck(mtx);
	cv.wait(lck);
	lck.unlock();
	std::cout << "I am UP! " << std::endl;
}

void JsonReader::proceedSymbolicExecution(Json::Value toSend)
{
	#ifdef CIN_SERVER
		Json::FastWriter fastWriter;
		std::string output = fastWriter.write(toSend);
		std::cout << "got this: " << output << std::endl;
		msg = getMessage();
	#else 
		sendMessageAndSleep(toSend);
	#endif
	setExecutionVars();
}
std::vector<std::pair<ExpressionTree*, std::string> > JsonReader::getModel(
	Json::Value toSend, std::map<std::string, llvm::Value*> userVarMap)
{
	std::vector<std::pair<ExpressionTree*, std::string> > to_ret;
	#ifdef CIN_SERVER
		Json::FastWriter fastWriter;
		std::string output = fastWriter.write(toSend);
		std::cout << "got this: " << output << std::endl;
		msg = getMessage();
	#else 
		sendMessageAndSleep(toSend);
	#endif
	for (const Json::Value& pair : msg["pairs"])
    {
    	ExpressionTree * tree = new ExpressionTree(pair["expression"].asString(), userVarMap);
		std::string constraint = pair["constraint"].asString();
		to_ret.push_back(std::make_pair(tree, constraint));    
    }
    return to_ret;
}

void JsonReader::wakeUp(Json::Value val)
{	
	updateMsg(val);
	std::cout << "WAKE UP!!!!!" << std::endl;
	std::unique_lock<std::mutex> lck(mtx);
	cv.notify_all();
	std::cout << "DONE!!!!!" << std::endl;
}

void JsonReader::updateMsg(Json::Value val)
{
	msg = val;
}

void JsonReader::setExecutionVars()
{
	if(msg["exclude"].asString() != "")
    {
    	excludedId =  stoi(msg["exclude"].asString());
    	isExclude = stoi(msg["isNode"].asString());
    	std::cout << "excluding! \n";
      	return;
    }
    isExclude = -1;
    excludedId = -1;

	isBFS = stoi(msg["isBFS"].asString());
	steps = stoi(msg["steps"].asString());
	dir = stoi(msg["branch"].asString());
	prevId = stoi(msg["prevId"].asString()); 

    #ifdef DEBUG
		std::cout << "prevId : " << prevId << "\n";
		std::cout << "isBFS : " << isBFS << "\n";
		std::cout << "branch : " << dir << "\n";
		std::cout << "steps : " << steps << "\n";
		std::cout << "proceed? : \n";
		int abc;
		std::cin >> abc;
    #endif
}