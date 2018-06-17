CXX = g++
CXXFLAGS =  -std=c++11
CXXFLAGS +=   -Wall
CXXFLAGS +=   -pedantic-errors

OBJS =  Compiler_Main.o Tokenizer.o CompilationEngine.o
SRCS =  Compiler_Main.cpp Tokenizer.cpp CompilationEngine.cpp
HEADERS = Tokenizer.hpp CompilationEngine.hpp

compile: ${OBJS} ${HEADERS}
	${CXX} ${LDFLAGS} ${OBJS} -o compile

${OBJS}: ${SRCS}
	${CXX} ${CXXFLAGS} -c $(@:.o=.cpp)

clean:
	rm *.o recursive
