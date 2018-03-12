CXX = g++
CXXFLAGS = -L. -Wall -g -pthread -DDEBUG

OBJECTS1 = helper.o message.o binder.o message_handler.o binder_db.o server_sock_db.o
EXEC1 = binder

OBJECTS2 = rpc.o helper.o procSignature.o message.o 
EXEC2 = librpc.a

OBJECTS3 = server.o server_functions.o server_function_skels.o rpc.o
EXEC3 = server

OBJECTS4 = client.o rpc.o
EXEC4 = client

OBJECTS = ${OBJECTS1} ${OBJECTS2} ${OBJECTS3} ${OBJECTS4}
DEPENDS = ${OBJECTS:.o=.d}
EXECS = ${EXEC1} ${EXEC2} ${EXEC3} ${EXEC4}

.PHONY : all clean

all : ${EXECS}

${EXEC1} : ${OBJECTS1}
	${CXX} ${CXXFLAGS} $^ -o $@

${EXEC2} : ${OBJECTS2}
	ar rcs librpc.a $^

${EXEC3} : ${OBJECTS3}
	${CXX} ${CXXFLAGS} $^ -lrpc -o $@

${EXEC4} : ${OBJECTS4}
	${CXX} ${CXXFLAGS} $^ -lrpc -o $@

-include ${DEPENDS}

clean :
	rm -f *.d *.o ${EXECS} ImplType
