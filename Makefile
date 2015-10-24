objects = main.o symbolicexecutor.o ProgramState.o llvmExpressionTree.o utils.o jsoncpp.o Socket.o ServerSocket.o
objectPaths = $(shell find build/*.o)
llvmPath = /usr/lib/llvm-3.5/build/include
INCLUDES = -I$(llvmPath)

llvmFlags = `llvm-config-3.5 --libs core jit native` `llvm-config-3.5 --cxxflags --ldflags --libs` `llvm-config --libs core` `llvm-config --ldflags`  -ltinfo -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS
exceptionFlag = -fexceptions
z3Flag = -lz3
BUILD_DIR = build
SRC_DIR = src
VPATH = ./:$(SRC_DIR):$(SRC_DIR)/server:$(SRC_DIR)/jsoncpp/dist:$(SRC_DIR)/jsoncpp/dist/json:$(BUILD_DIR)
CXX = g++ -std=c++0x
PREFIX = $(CXX) $(SRC_DIR)/
SUFFIX = $(BUILD_DIR)/$@ 

all: sfse.o

sfse.o: $(objects)
	$(CXX) $(objectPaths) $(INCLUDES) -o sfse.o $(llvmFlags) $(exceptionFlag) $(z3Flag)

main.o: main.cpp symbolicexecutor.h SocketException.h
	$(PREFIX)main.cpp $(INCLUDES) -c -o $(SUFFIX)

Socket.o: Socket.h Socket.cpp
	$(PREFIX)server/Socket.cpp -c -o $(SUFFIX)

ServerSocket.o: ServerSocket.h ServerSocket.cpp Socket.h SocketException.h
	$(PREFIX)server/ServerSocket.cpp -c -o $(SUFFIX)

symbolicexecutor.o: symbolicexecutor.h symbolicexecutor.cpp json.h llvmExpressionTree.h ProgramState.h
	$(PREFIX)symbolicexecutor.cpp $(INCLUDES) -c -o $(SUFFIX)
ProgramState.o: ProgramState.h ProgramState.cpp llvmExpressionTree.h
	$(PREFIX)ProgramState.cpp $(INCLUDES) -c -o $(SUFFIX)
llvmExpressionTree.o: utils.h llvmExpressionTree.h llvmExpressionTree.cpp
	$(PREFIX)llvmExpressionTree.cpp $(INCLUDES) -c -o $(SUFFIX)
utils.o: utils.h utils.cpp
	$(PREFIX)utils.cpp $(INCLUDES) -c -o $(SUFFIX)
jsoncpp.o: json.h jsoncpp.cpp
	$(PREFIX)jsoncpp/dist/jsoncpp.cpp -c -o $(SUFFIX)
setup:
	@mkdir -p build
clean:
	@rm -f $(BUILD_DIR)/*.o