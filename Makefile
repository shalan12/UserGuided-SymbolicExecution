CC = g++

LLVM_MODULES = core jit native

CPPFLAGS = `llvm-config --cppflags $(LLVM_MODULES)`
LDFLAGS = `llvm-config --ldflags $(LLVM_MODULES)`
LIBS = `llvm-config --libs $(LLVM_MODULES)`

all:
	$(CC) *.o $(LDFLAGS) $(LIBS) -o MyOutput
main:
	find -name '*.cpp' -print0 | xargs -0 $(CC) -c $(CPPFLAGS)