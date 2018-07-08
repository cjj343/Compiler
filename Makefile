CXX = g++
CXXFLAGS =  -std=c++11
CXXFLAGS +=   -Wall
CXXFLAGS +=   -pedantic-errors

OBJS =  Compiler_Main.o Tokenizer.o CompilationEngine.o Symbol_Table.o VMWriter.o
SRCS =  Compiler_Main.cpp Tokenizer.cpp CompilationEngine.cpp Symbol_Table.cpp VMWriter.cpp
HEADERS = Tokenizer.hpp CompilationEngine.hpp Symbol_Table.hpp VMWriter.hpp

compile: ${OBJS} ${HEADERS}
	${CXX} ${LDFLAGS} ${OBJS} -o j++

${OBJS}: ${SRCS}
	${CXX} ${CXXFLAGS} -c $(@:.o=.cpp)

clean:
	rm *.o recursive
