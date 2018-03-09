CXX = g++										# compiler
CXXFLAGS = -L. -Wall -g -lpthread

OBJECTS1 = error.o common.o binder.o rpcDatabase.o
EXEC1 = binder

OBJECTS2 = rpc.o common.o error.o
EXEC2 = librpc.a

OBJECTS3 = server.o server_functions.o server_function_skels.o rpc.o
EXEC3 = server

OBJECTS4 = client.o rpc.o
EXEC4 = client

OBJECTS = ${OBJECTS1} ${OBJECTS2} ${OBJECTS3}	${OBJECTS4}			# all object files
DEPENDS = ${OBJECTS:.o=.d}						# substitute ".o" with ".d"
EXECS = ${EXEC1} ${EXEC2}	${EXEC3} ${EXEC4}					# all executables

.PHONY : all clean

all : ${EXECS}									# build all executables

#############################################################
client.o: rpc.o
	gcc -c -g -o client.o client.c

server_function_skels.o: rpc.o
	gcc -c -g -o server_function_skels.o server_function_skels.c

${EXEC1} : ${OBJECTS1}							# link step 1st executable
	${CXX} ${CXXFLAGS} $^ -o $@

${EXEC2} : ${OBJECTS2}
	ar rcs librpc.a $^

${EXEC3} : ${OBJECTS3}
	${CXX} ${CXXFLAGS} $^ -lrpc -o $@

${EXEC4} : ${OBJECTS4}
	${CXX} ${CXXFLAGS} $^ -lrpc -o $@

#############################################################

-include ${DEPENDS}

clean :											# remove files that can be regenerated
	rm -f *.d *.o ${EXECS} ImplType
