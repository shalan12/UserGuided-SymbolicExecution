#include "jsonreader.h"
#include "jsoncpp/dist/json/json.h"
#include "utils.h" 

JsonReader::JsonReader(ServerSocket * s)
{
	socket = s;
	jsonArrSize = 0;
}

void JsonReader::sendMessageAndSleep()
{
	toSend["nodes"] = nodes;
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
	jsonArrSize = 0;
	toSend.clear();
}

void JsonReader::proceedSymbolicExecution()
{
	#ifdef CIN_SERVER
		Json::FastWriter fastWriter;
		std::string output = fastWriter.write(toSend);
		std::cout << "got this: " << output << std::endl;
		msg = getMessage();
	#else 
		sendMessageAndSleep();
	#endif
	setExecutionVars();
}
std::vector<std::pair<ExpressionTree*, std::string> > JsonReader::getModel(
	std::map<std::string, llvm::Value*> userVarMap)
{
	modelRequiredForLast();
	std::vector<std::pair<ExpressionTree*, std::string> > to_ret;
	#ifdef CIN_SERVER
		Json::FastWriter fastWriter;
		std::string output = fastWriter.write(toSend);
		std::cout << "got this: " << output << std::endl;
		msg = getMessage();
	#else 
		sendMessageAndSleep();
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

void JsonReader::updateToSend(Json::Value val)
{
	toSend = val;
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

void JsonReader::addObject(Json::Value obj)
{
	nodes[jsonArrSize++] = obj;
}
void JsonReader::modelRequiredForLast()
{
	nodes[jsonArrSize-1]["addModel"] = Json::Value("true");
}
void JsonReader::initializeJsonArray()
{
	nodes = Json::arrayValue;
}