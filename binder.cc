#include <errno.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <netdb.h>
#include <unistd.h>
#include <algorithm>
#include <map>
#include "common.h"
#include <list>
#include <cstring>


/*
*	Sends the message to socket fd with package size of 64 bytes each
* returns 0 is execution did not encounter error
* else return the error status code
*/
int sendMessage(int socket_fd,MessageType msg_type, char msg_data[])
{
    // Format of the data is
    // Send the length
    send(socket_fd, &msg_type, sizeof(MessageType), 0);
    if (msg_type == LOC_FAILURE){
        send(socket_fd, msg_data, 4, 0);
    }else if(msg_type == LOC_SUCCESS){
        send(socket_fd, msg_data, 4 + 128, 0);
    }
    return 0;
}

//can be removed after clean up
int receiveMessage(int socket_fd, int expect_len, char buf[])
{
	// Reads until expect_len is reached
	char* ptr = buf;
	while(expect_len > 0) {
		int rcv_len = recv(socket_fd, ptr, expect_len, 0);
		if(rcv_len == 0)
			break;
		else if(rcv_len < 0)
			return -404;

		expect_len -= rcv_len;
		ptr += rcv_len;
	}

	return 0;
}

struct FuncSignature {
    std::string name;
    int* argTypes;
    int argSize;
    FuncSignature(std::string name, int* argTypes, int argSize) : name(name), argTypes(argTypes), argSize(argSize) {}
};

// bool operator == (const FuncSignature &l, const FuncSignature &r) {
//     if (l.name == r.name && l.argSize == r.argSize) {
//         int i = 0;
//         while (i < l.argSize) {
//             if (l.argTypes[i] != r.argTypes[i]) {
//                 return false;
//             }
//             i++;
//         }
//         return true;
//     }
//     return false;
// }

struct ServerLoc {
    std::string serverId;
    int port;
    int socketFd;
    ServerLoc(std::string serverId, int port, int socketFd) : serverId(serverId), port(port), socketFd(socketFd) {}
};

// bool operator == (const ServerLoc &l, const ServerLoc &r) {
//     return l.serverId == r.serverId && l.port == r.port && l.socketFd == r.socketFd;
// }

bool terminating;
std::map <FuncSignature*, std::list<ServerLoc *> > funcDict;
std::list<ServerLoc *> serverQueue;

void registerServer(std::string serverId, unsigned short port, int socketFd) {
    ServerLoc *location = new ServerLoc(serverId, port, socketFd);

    for (std::list<ServerLoc *>::iterator it=serverQueue.begin(); it!=serverQueue.end(); ++it){
        if (*it == location) {
            // server is registered and in queue
            return;
        }
    }
    // push the server to round robin queue
    serverQueue.push_back(location);

    std::cout << "registered server:" << serverId << std::endl;
    // for (int i = 0; i < serverQueue.size(); i++) {
    //     if (*(serverQueue[i]) == *location) {
    //         // server is registered and in queue
    //         return;
    //     }
    // }
    // // push the server to round robin queue
    // serverQueue.push_back(location);
}

void registerFunc(std::string name, int* argTypes, int argSize, std::string serverId, unsigned short port, int socketFd) {
    bool found = false;
    ServerLoc *location = new ServerLoc(serverId, port, socketFd);
    FuncSignature *func = new FuncSignature(name, argTypes, argSize/4);
    // Look up function in the dictionary
    for (std::map<FuncSignature *, std::list<ServerLoc *> >::iterator it = funcDict.begin(); it != funcDict.end(); it ++) {
        if (func->name == it->first->name && func->argSize == it->first->argSize) {
            found = true;
            for (int i = 0; i < func->argSize; i++){
                if (func->argTypes[i] != it->first->argTypes[i]) {
                    found = false;
                    break;
                }
            }
        }

        if (found) {
            for (std::list<ServerLoc *>::iterator listit=it->second.begin(); listit!=it->second.end(); ++listit){
                if (*listit == location) {
                    // same function with same serverloc
                    return;
                }
            }
            it->second.push_back(location);
        }
    }

    if (!found) {
        // adds function to the dictionary
        funcDict[func].push_back(location);

        std::cout << "added function name:" << name << std::endl;
    }

    // register server to queue if not already
    registerServer(serverId, port, socketFd);
    return;
}


void handleRegisterRequest(int clientSocketFd) {
    // char buffer[msgLength];
    ReasonCode reason;
    //char responseMsg [sizeof(int)];
    // // reads message to buffer
    // int status = receiveMessage(clientSocketFd, msgLength, buffer);
    // if (status == -404) {
    //     // corrupt message
    //     reason = MESSAGE_CORRUPTED;
    //     memcpy(responseMsg, &reason, sizeof(int));
    //     sendMessage(clientSocketFd, 3 * sizeof(int), REGISTER_FAILURE, responseMsg);
    //     return;
    // }

    int serverlength;
    recv(clientSocketFd,&serverlength,sizeof(int),0);
    char server[128];
    recv(clientSocketFd,&server,serverlength * sizeof(char),0);

    // get port
    // int portlength;
    // recv(clientSocketFd,&portlength,sizeof(int),0);
    int port;
    recv(clientSocketFd,&port,sizeof(int),0);

    // get funcName
    int funcNamelength;
    recv(clientSocketFd,&funcNamelength,sizeof(int),0);
    char funcName[funcNamelength];
    recv(clientSocketFd,&funcName,funcNamelength*sizeof(char),0);

    // get argType
    int argTypelength;
    recv(clientSocketFd,&argTypelength,sizeof(int),0);
    int* argType = new int[argTypelength];
    int argSize = (argTypelength - 1) * sizeof(int);
    recv(clientSocketFd,&argType,argSize,0);

    // char server[128];
    // unsigned short port;
    // char funcName[128];
    // int argSize = ((msgLength - 2 * 128 - sizeof(unsigned short))/ sizeof(int));
    // int *argTypes = new int[argSize];

    // reads server and function info
    // memcpy(server, buffer, 128);
    // memcpy(&port, buffer + 128, sizeof(unsigned short));
    // memcpy(funcName, buffer + 128 + sizeof(unsigned short), 128);
    // memcpy(argTypes, buffer + 2 * 128 + sizeof(unsigned short), argSize * sizeof(int));

    std::string name(funcName);
    std::string serverId(server);

    char responseMsg[sizeof(int)];
    registerFunc(name, argType, argSize, serverId, port, clientSocketFd);
    std::cout << "register name:" << name << std::endl;
}

ServerLoc *lookupAvailableServer(std::string name, int *argTypes, int argSize) {
    ServerLoc *selectedServer = NULL;
    FuncSignature *func = new FuncSignature(name, argTypes, argSize/4);
    bool argmatch = false;
    for (std::map<FuncSignature *, std::list<ServerLoc *> >::iterator it = funcDict.begin(); it != funcDict.end(); it ++) {
        
        // if (func->name == it->first->name && func->argSize == it->first->argSize) {
        //     argmatch = true;
        //     for (int i = 0; i < func->argSize; i++){
        //         if ((func->argTypes)[i] != (it->first->argTypes)[i]) {
        //             argmatch = false;
        //             break;
        //         }
        //     }
        // }

        if (func->name == it->first->name)
        {
            argmatch = true;
        }

        if (argmatch) {
            std::list<ServerLoc *> availServers = it->second;
            // Look up server queue in round robin fashion
            for (int i = 0; i < serverQueue.size(); i++) {
                ServerLoc * server = serverQueue.front();

                for (std::list<ServerLoc *>::iterator listit=availServers.begin(); listit!=availServers.end(); ++listit){
                    if (server->serverId == (*listit)->serverId && server->port == (*listit)->port && server->socketFd == (*listit)->socketFd) {
                        // found the first available server
                        selectedServer = server;
                        break;
                    }
                }

                // for (int j = 0; j < availServers.size(); j++) {
                //     if (*server == *(availServers[j])) {
                //         // found the first available server
                //         selectedServer = server;
                //         break;
                //     }
                // }

                // move the server to the back of the queue if cannot service the function
                // rotate(serverQueue.begin(), serverQueue.end()-1, serverQueue.end());
                serverQueue.pop_front();
                serverQueue.push_back(server);
            }


        }
    }
    return selectedServer;
}

void handleLocationRequest(int clientSocketFd) {
    // char buffer[msgLength];
    // read message to buffer
    // int status = receiveMessage(clientSocketFd, msgLength, buffer);
    // if (status == -404) {
    //     // corrupt message
    //     char responseMsg [sizeof(int)];
    //     ReasonCode reason = MESSAGE_CORRUPTED;
    //     memcpy(responseMsg, &reason, sizeof(int));
    //     sendMessage(clientSocketFd, 3 * sizeof(int), LOC_FAILURE, responseMsg);
    //     return;
    // }

    // char funcName[128];
    // int argSize = ((msgLength - 128) / sizeof(int));
    // int *argTypes = new int[argSize];

    // // reads function name and args
    // memcpy(funcName, buffer, 128);
    // memcpy(argTypes, buffer + 128, argSize * sizeof(int));

    // get funcName
    int funcNamelength;
    recv(clientSocketFd,&funcNamelength,sizeof(int),0);
    char funcName[funcNamelength];
    recv(clientSocketFd,&funcName,funcNamelength*sizeof(char),0);

    // get argType
    int argTypelength;
    recv(clientSocketFd,&argTypelength,sizeof(int),0);
    int* argType = new int[argTypelength];;
    recv(clientSocketFd, &argType, argTypelength*sizeof(int), 0);
    int argSize = (argTypelength - 1) * sizeof(int);

    std::string name(funcName);

    ServerLoc * availServer = lookupAvailableServer(name, argType, argSize);

    if (!availServer) {
        // function not found, return failure
        char responseMsg [sizeof(int)];
        ReasonCode reason = FUNCTION_NOT_FOUND;
        memcpy(responseMsg, &reason, sizeof(int));
        sendMessage(clientSocketFd, LOC_FAILURE, responseMsg);
    } else {
        // return server info if found
        char responseMsg [128 + sizeof(int)];
        memcpy(responseMsg, availServer->serverId.c_str(), 128);
        memcpy(responseMsg + 128, &(availServer->port), sizeof(unsigned short));
        sendMessage(clientSocketFd, LOC_SUCCESS, responseMsg);
    }
}



// void removeServer(int closingSocketFd) {
//     for (int i = 0; i < serverQueue.size(); i++) {
//         if (serverQueue[i]->socketFd == closingSocketFd) {
//             delete serverQueue[i];
//             serverQueue.erase(serverQueue.begin() + i);
//         }
//     }
    
//     for (std::map<FuncSignature *, std::vector<ServerLoc *> >::iterator it = funcDict.begin(); it != funcDict.end(); it++) {
//         for (std::vector<ServerLoc *>::iterator it2 = it->second.begin(); it2 != it->second.end();) {
//             if (closingSocketFd == (*it2)->socketFd) {
//                 delete *it2;
//                 it2 = it->second.erase(it2);
//                 break;
//             } else {
//                 it2++;
//             }
//         }
//     }
// }

// void cleanup() {
//     // clean up database and queue
//     for (int i = 0; i < serverQueue.size(); i++) {
//         delete serverQueue[i];
//     }
//     serverQueue.clear();

//     for (std::map<FuncSignature *, std::vector<ServerLoc *> >::iterator it = funcDict.begin(); it != funcDict.end(); it ++) {
//         delete it->first;
//         std::vector<ServerLoc *> v = it->second;
//         for (std::vector<ServerLoc *>::iterator v_it = v.begin() ; v_it != v.end(); v_it++) {
//             delete *v_it;
//         }
//         v.clear();
//     }
//     funcDict.clear();
// }

int handleRequest_helper_length(int clientSocketFd,fd_set* masterFds){
    int msgLength;
    if (read(clientSocketFd, &msgLength, 4) <= 0) {
        close(clientSocketFd);
        FD_CLR(clientSocketFd, masterFds);
        //removeServer(clientSocketFd);
        return -10;
    }
    return msgLength;
}


void handleRequest(int clientSocketFd, fd_set *masterFds) {
    // // read message length
    // int msgLength;
    // int bytes = read(clientSocketFd, &msgLength, 4);
    // if (bytes <= 0) {
    //     close(clientSocketFd);
    //     FD_CLR(clientSocketFd, masterFds);
    //     removeServer(clientSocketFd);
    //     // if (serverQueue.size() == 0 && terminating) {
    //     //     //cleanup();
    //     //     exit(0);
    //     // }
    //     return;
    // }

    // int msgLength = handleRequest_helper_length(clientSocketFd, masterFds);
    // if (msgLength == -10){
    //     return;
    // }

    // read message type
    MessageType msgType;
    if (read(clientSocketFd, &msgType, 4) <= 0) {
        close(clientSocketFd);
        FD_CLR(clientSocketFd, masterFds);
        //removeServer(clientSocketFd);
        return;
    }

    if (msgType == REGISTER) {
        handleRegisterRequest(clientSocketFd);
    } else if (msgType == LOC_REQUEST) {
        handleLocationRequest(clientSocketFd);
    } else if (msgType == TERMINATE) {
        // terminate and clean up
        for (std::list<ServerLoc *>::iterator it=serverQueue.begin(); it!=serverQueue.end(); ++it){
            sendMessage((*it)->socketFd, TERMINATE, NULL);
        }
        // for (int i = 0; i < serverQueue.size(); i++) {
        //     sendMessage(serverQueue[i]->socketFd, 2 * sizeof(int), TERMINATE, NULL);
        // }
        terminating = true;
    } else {
    }
}

int main() {
    terminating = false;
    struct sockaddr_in svrAddr, clntAddr;
    fd_set masterFds;
    fd_set readFds;

    int socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd < 0) {
        std::cerr << "Error: cannot open a socket" << std::endl;
        return 1;
    }

    memset((char*) &svrAddr, 0, sizeof(svrAddr));
    svrAddr.sin_family = AF_INET;
    svrAddr.sin_addr.s_addr = htons(INADDR_ANY);
    svrAddr.sin_port = htons(0);

    // if (bind(socketFd, (struct sockaddr *)&svrAddr, sizeof(svrAddr)) < 0) {
    //     std::cerr << "Error: cannot bind socket" << std::endl;
    //     return 1;
    // }

    // if (listen(socketFd, 5) < 0) {
    //     std::cerr << "Error: cannot listen to socket" << std::endl;
    //     return 1;
    // }

    bind(socketFd, (struct sockaddr *)&svrAddr, sizeof(svrAddr));
    listen(socketFd, 5);


    socklen_t size = sizeof(svrAddr);
    char server_name [128];
    getsockname(socketFd, (struct sockaddr *)&svrAddr, &size);
    gethostname(server_name, 128);

    std::cout << "BINDER_ADDRESS " << server_name << std::endl;
    std::cout << "BINDER_PORT " << ntohs(svrAddr.sin_port) << std::endl;

    FD_ZERO(&masterFds);
    FD_SET(socketFd, &masterFds);
    int fdmax = socketFd;

    while(1) {
        if (terminating) exit(0);
        readFds = masterFds;

        // if (select(fdmax+1, &readFds, NULL, NULL, NULL) < 0) {
        //     std::cerr << "Error: fail to select" << std::endl;
        //     return 1;
        // }

        select(fdmax+1, &readFds, NULL, NULL, NULL);

        for (int i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &readFds)) {
                if (i == socketFd) {
                    socklen_t size = sizeof(clntAddr);
                    int newFd = accept(socketFd, (struct sockaddr *)&clntAddr, &size);
                    if (newFd < 0) {
                        std::cerr << "Error: cannot establish new connection" << std::endl;
                        return 1;
                    }
                    FD_SET(newFd, &masterFds);
                    if (newFd > fdmax) fdmax = newFd;

                } else {
                    int clientSocketFd = i;
                    handleRequest(clientSocketFd, &masterFds);
                }
            }
        }
    }


}
