#include "rpc.h"
#include "servinfo.h"
#include "procSignature.h"
#include "helper.h"
#include "error.h"
#include "constants.h"
#include "message.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <algorithm>
#include <map>
#include <vector>
#include <iostream>
using namespace std;

int binder_sockfd, server_sockfd, server_port;
char* server_identifier;
map <procSignature, skeleton > procedure_db;
map <procSignature, vector<servinfo> > cache;
vector <int> connections;
vector <int> connections_to_remove;

struct thread_data {
    int sockfd;
    char *name;
    int *argTypes;
    void** args;
    skeleton f;
};

void *executeProc(void *t_arg) {
    struct thread_data *data = (struct thread_data *) t_arg;
#ifdef DEBUG
    cerr<<"sock in thread: " << data->sockfd <<endl;
#endif

    int retval = data->f(data->argTypes, data->args);
    if (retval == 0) {
        // send the execute success msg
        struct E_S_Message msg (data->name, data->argTypes, data->args, getArgsLength(data->argTypes));
        msg.sendmsg(data->sockfd);
    } else {
        // send the execute failure message
        struct E_F_Message msg (retval);
        msg.sendmsg(data->sockfd);
    }

    pthread_exit(NULL);
}

int sendExecMessageToServers(vector<servinfo> &v, char* name, int* argTypes, void** args) {
    int retval, msg_type;
    for (vector<servinfo>::iterator it = v.begin(); it != v.end(); it++) {
#ifdef DEBUG
        cerr << "sending exec message\n";
        cerr << "name : "<< name << "\n";
        cerr << "server_id: "<< it->sid << "\n";
        cerr << "port: " << it->port << "\n";
#endif

        // create socket connection to server with proc
        struct sockaddr_in p_server_addr;

        int p_server_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (p_server_sockfd < 0) {
            continue;
        }

        struct hostent *p_server = gethostbyname(it->sid.c_str());
        if (p_server == NULL) {
            continue;
        }

        bzero((char *) &p_server_addr, sizeof(p_server_addr));
        p_server_addr.sin_family = AF_INET;
        bcopy((char *)p_server->h_addr,
            (char *)&p_server_addr.sin_addr.s_addr,
            p_server->h_length);
        p_server_addr.sin_port = htons(it->port);

        if (connect(p_server_sockfd,(struct sockaddr *)&p_server_addr, sizeof(p_server_addr)) < 0) {
            continue;
        }

        // send execute message
        struct E_Message e_msg(name, argTypes, args, getArgsLength(argTypes));
        retval = e_msg.sendmsg(p_server_sockfd);
        if (retval < 0) {
            continue;
        }

#ifdef DEBUG
        cerr << "execute message successfully sent\n";
#endif
        // recv execute status message
        retval = recv(p_server_sockfd, &msg_type, sizeof (msg_type), 0);
        if (retval < 0) {
            cerr << "failed to receive msg type" << endl;
            continue;
        }

        struct E_S_Message* e_success_msg = NULL;
        if (msg_type == EXECUTE_SUCCESS) {
            e_success_msg = E_S_Message::read(p_server_sockfd);
            if (e_success_msg == NULL) {
                continue;
            }
        } else if (msg_type == EXECUTE_FAILURE) {
            struct E_F_Message* e_failure_msg = E_F_Message::read(p_server_sockfd);
            if (e_failure_msg == NULL) {
                continue;
            }
            continue;
        } else {
            continue;
        }

        // set the returned args
        for (int i=0; i<e_success_msg->length - 1; i++) {
            args[i] = e_success_msg->args[i];
        }
        close(p_server_sockfd);
        return 0;
    }

    return SEND_EXECUTE_MSG_TO_SERVERS_FAILURE;
}

int rpcInit() {
    char *binderAddress = getenv("BINDER_ADDRESS");
    if (binderAddress == NULL) {
        return INVALID_BINDER_ADDRESS;
    }
    char *binderPort = getenv("BINDER_PORT");
    if (binderPort == NULL) {
        return INVALID_BINDER_PORT;
    }

    // create socket connection to binder
    struct sockaddr_in binder_addr;
    int port = atoi(binderPort);

    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        return CREATE_SOCKET_FAILURE;
    }

    struct hostent *binder = gethostbyname(binderAddress);
    if (binder == NULL) {
        return UNKNOWN_HOST;
    }

    bzero((char *) &binder_addr, sizeof(binder_addr));
    binder_addr.sin_family = AF_INET;
    bcopy((char *)binder->h_addr,
        (char *)&binder_addr.sin_addr.s_addr,
        binder->h_length);
    binder_addr.sin_port = htons(port);

    if (connect(sockfd,(struct sockaddr *)&binder_addr, sizeof(binder_addr)) < 0) {
        return CONNECTION_FAILURE;
    }
    binder_sockfd = sockfd;
#ifdef DEBUG
    cerr << "bindersock: " << binder_sockfd<<endl;
#endif
    // accept connections from clients
    int retval;
    struct addrinfo hints;
    struct addrinfo* servinfo;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    retval = getaddrinfo(NULL, "0", &hints, &servinfo);
    if (servinfo == NULL) {
        return ADDRINFO_FAILURE;
    }
    server_sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (server_sockfd < 0) {
        return CREATE_SOCKET_FAILURE;
    }
    retval = bind(server_sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
    if (retval < 0) {
        return BIND_FAILURE;
    }
    retval = listen(server_sockfd, 5);
    if (retval < 0) {
        return LISTEN_FAILURE;
    }

    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    getsockname(server_sockfd, (struct sockaddr *)&sin, &len);
    server_port = ntohs(sin.sin_port);
#ifdef DEBUG
    cerr << "serverport:" <<server_port << endl;
#endif
    server_identifier = new char[256];
    getsockname(binder_sockfd, (struct sockaddr *)&sin, &len);
    getnameinfo((struct sockaddr*)&sin, len, server_identifier, 256, NULL, 0, 0);
#ifdef DEBUG
    cerr << "serverid:" << server_identifier << endl;
#endif
    return 0;
}

int rpcCall(const char* name, int* argTypes, void** args) {
#ifdef DEBUG
    cerr << "rpcCalling..." << endl;
#endif
    char *binderAddress = getenv("BINDER_ADDRESS");
    if (binderAddress == NULL) {
        return INVALID_BINDER_ADDRESS;
    }
    char *binderPort = getenv("BINDER_PORT");
    if (binderPort == NULL) {
        return INVALID_BINDER_PORT;
    }

    // create socket connection to binder
    struct sockaddr_in binder_addr;
    int port = atoi(binderPort);

    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        return CREATE_SOCKET_FAILURE;
    }

    struct hostent *binder = gethostbyname(binderAddress);
    if (binder == NULL) {
        return UNKNOWN_HOST;
    }

    bzero((char *) &binder_addr, sizeof(binder_addr));
    binder_addr.sin_family = AF_INET;
    bcopy((char *)binder->h_addr,
        (char *)&binder_addr.sin_addr.s_addr,
        binder->h_length);
    binder_addr.sin_port = htons(port);

    if (connect(sockfd,(struct sockaddr *)&binder_addr, sizeof(binder_addr)) < 0) {
        return CONNECTION_FAILURE;
    }
    binder_sockfd = sockfd;

    // send location request to binder
    int len = strlen(name) + 1;
    char * namecpy = new char[len];
    strcpy(namecpy, name);
    struct LOC_R_Message msg(namecpy, argTypes, getArgsLength(argTypes));
    int retval = msg.sendmsg(binder_sockfd);
    if (retval < 0) {
        cerr << "failed to send loc request msg\n";
        return retval;
    }

    // recv loc request status
    int msg_type;
    retval = recv(binder_sockfd, &msg_type, sizeof(msg_type), 0);
    if (retval < 0) {
        cerr << "failed to recv loc request status\n";
        return retval;
    }

    struct LOC_S_Message* success_msg = NULL;
    switch (msg_type) {
        case LOC_SUCCESS: 
        {
#ifdef DEBUG
            cerr << "rpcCall - loc success message received\n";
#endif
            success_msg = LOC_S_Message::read(binder_sockfd);
            if (success_msg== NULL) {
                return MSG_NOT_RECEIVED;
            }
#ifdef DEBUG
            cerr << "rpcCall - loc suc msg read successfully\n";
#endif
            break;
        }
        case LOC_FAILURE:
        {
#ifdef DEBUG
            cerr << "rpcCall - loc failure message received\n";
#endif
            struct LOC_F_Message* failure_msg = LOC_F_Message::read(binder_sockfd);
            if (failure_msg == NULL) {
                return MSG_NOT_RECEIVED;
            }
            return failure_msg->reason_code;
        }
        default:
            return UNEXPECTED_MSG_TYPE;
    }

    close(binder_sockfd);

    // create socket connection to server with proc
    struct sockaddr_in p_server_addr;

    int p_server_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (p_server_sockfd < 0) {
        return CREATE_SOCKET_FAILURE;
    }
    
#ifdef DEBUG
    cerr <<"rpcCall - server_id: " << success_msg->server_identifier<<endl;
#endif
    struct hostent *p_server = gethostbyname(success_msg->server_identifier);
    if (p_server == NULL) {
        return UNKNOWN_HOST;
    }

    bzero((char *) &p_server_addr, sizeof(p_server_addr));
    p_server_addr.sin_family = AF_INET;
    bcopy((char *)p_server->h_addr,
        (char *)&p_server_addr.sin_addr.s_addr,
        p_server->h_length);
    p_server_addr.sin_port = htons(success_msg->port);

    if (connect(p_server_sockfd,(struct sockaddr *)&p_server_addr, sizeof(p_server_addr)) < 0) {
        return CONNECTION_FAILURE;
    }

    // send execute message
    struct E_Message e_msg(namecpy, argTypes, args, getArgsLength(argTypes));
    retval = e_msg.sendmsg(p_server_sockfd);
    if (retval < 0) {
        return retval;
    }

#ifdef DEBUG
    cerr << "execute message successfully sent\n";
#endif
    // recv execute status message
    retval = recv(p_server_sockfd, &msg_type, sizeof (msg_type), 0);
    if (retval < 0) {
        cerr << "failed to receive msg type" << endl;
        return retval;
    }

    struct E_S_Message* e_success_msg = NULL;
    switch (msg_type) {
        case EXECUTE_SUCCESS:
        {
            e_success_msg = E_S_Message::read(p_server_sockfd);
            if (e_success_msg == NULL) {
                return MSG_NOT_RECEIVED;
            }
            break;
        }
        case EXECUTE_FAILURE:
        {
            struct E_F_Message* e_failure_msg = E_F_Message::read(p_server_sockfd);
            if (e_failure_msg == NULL) {
                return MSG_NOT_RECEIVED;
            }
            return e_failure_msg->reason_code;
        }
        default:
            return UNEXPECTED_MSG_TYPE;
    }

    // set the returned args
    for (int i=0; i<e_success_msg->length - 1; i++) {
        args[i] = e_success_msg->args[i];
    }

    // close connection
    close(p_server_sockfd);

    return 0;
}

// int rpcCacheCall(const char* name, int* argTypes, void** args) {
//     string sname(name);
//     int len = strlen(name)+1;
//     char * namecpy = new char[len];
//     strcpy(namecpy, name);
//     int retval = 0;
//     struct procSignature procKey;
//     procKey.name = sname;
//     procKey.argTypes = argTypes;
    
//     // check cache
//     if(cache.find(procKey) != cache.end())
//     {
//         // send execute to all servers
//         retval = sendExecMessageToServers(cache[procKey], namecpy, argTypes, args);

//         if(retval == 0) return 0;
//         cache.erase(procKey);
//     }
//     // getting here means key does not exist in cache or all servers of key fail to execute, need to fetch from binder
//     // send request
//     char *binderAddress = getenv("BINDER_ADDRESS");
//     if (binderAddress == NULL) {
//         return INVALID_BINDER_ADDRESS;
//     }
//     char *binderPort = getenv("BINDER_PORT");
//     if (binderPort == NULL) {
//         return INVALID_BINDER_PORT;
//     }

//     // create socket connection to binder
//     struct sockaddr_in binder_addr;
//     int port = atoi(binderPort);

//     int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//     if (sockfd < 0) {
//         return CREATE_SOCKET_FAILURE;
//     }

//     struct hostent *binder = gethostbyname(binderAddress);
//     if (binder == NULL) {
//         return UNKNOWN_HOST;
//     }

//     bzero((char *) &binder_addr, sizeof(binder_addr));
//     binder_addr.sin_family = AF_INET;
//     bcopy((char *)binder->h_addr,
//         (char *)&binder_addr.sin_addr.s_addr,
//         binder->h_length);
//     binder_addr.sin_port = htons(port);

//     if (connect(sockfd,(struct sockaddr *)&binder_addr, sizeof(binder_addr)) < 0) {
//         return CONNECTION_FAILURE;
//     }
//     binder_sockfd = sockfd;

//     // send cache location request to binder
//     struct C_LOC_R_Message cache_msg(namecpy, argTypes, getArgsLength(argTypes));
//     retval = cache_msg.sendmsg(binder_sockfd);
//     if (retval < 0) {
//         cerr << "failed to send cache loc request msg\n";
//         return retval;
//     }

//     // recv length
//     int numServers;
//     retval = recv(binder_sockfd, &numServers, sizeof(numServers), 0);
//     if (retval < 0) {
//         cerr << "failed to recv number of servers\n";
//         return retval;
//     }

//     if (numServers == 0) {
//         cerr << "no registered procedure found in binder\n";
//         return NO_REGISTERED_PROCEDURES; 
//     }

// #ifdef DEBUG
//     cerr << "numServers : "<< numServers <<endl;
// #endif
//     int msg_type;
//     vector <servinfo> servinfos;
//     for (int i = 0; i < numServers; i++) {
//         // recv msg type
//         retval = recv(binder_sockfd, &msg_type, sizeof (msg_type), 0);
//         if (retval < 0) {
//             cerr << "failed to recv msg type\n";
//             return retval; 
//         }

//         // read loc_s_messages
//         struct LOC_S_Message* loc_s_msg = LOC_S_Message::read(binder_sockfd);
//         if (loc_s_msg == NULL) {
//             cerr << "failed to recv loc success msg\n";
//             return retval;
//         }
        
// #ifdef DEBUG
//         cerr << "loc_s_msg port: " << loc_s_msg->port<<endl;
//         cerr << "loc_s_msg sid: " << loc_s_msg->server_identifier<<endl;
// #endif
        
//         struct servinfo si(loc_s_msg->port, string(loc_s_msg->server_identifier));
// #ifdef DEBUG
//         cerr << "servinfo port: " << si.port<<endl;
//         cerr << "servinfo sid: " << si.sid<<endl;
// #endif
//         servinfos.push_back(si);
//     }
//     close(binder_sockfd);
//     // store in cache
//     cache[procKey] = servinfos;

// #ifdef DEBUG
//     cerr << "servinfos size: " << servinfos.size() <<endl;
// #endif

//     retval = sendExecMessageToServers(servinfos, namecpy, argTypes, args);
//     if (retval != 0) {
//         cerr << "servers recently fetched from binders are no longer executing, no registered procedures\n";
//         return NO_REGISTERED_PROCEDURES;
//     }
//     return 0;
// }

int rpcRegister(const char* name, int* argTypes, skeleton f) {
    int len = strlen(name) + 1;
    char * namecpy = new char[len];
    strcpy(namecpy, name);
    // send register message to binder
#ifdef DEBUG
    cerr << "Server id" << server_identifier << endl;
    cerr << "server port" << server_port << endl;
    cerr << name << endl;
    cerr << "procedure name: " << namecpy << endl;
#endif
    struct R_Message msg(server_identifier, server_port, namecpy, argTypes, getArgsLength(argTypes));
#ifdef DEBUG
    cerr << "procedure name2: " << msg.procedure_name << endl;
    cerr << "binder sockfd " << binder_sockfd << endl;
#endif
    int retval = msg.sendmsg(binder_sockfd);
    if (retval < 0) {
        cerr << "failed to send register message\n";
        return retval;
    }

    // recv msg type
    int msg_type;
    retval = recv(binder_sockfd, &msg_type, sizeof (msg_type), 0);
    if (retval < 0) {
        cerr << "failed to recv msg type\n";
        return retval;
    }

#ifdef DEBUG
    cerr << "msg_type: " <<msg_type<<endl; 
#endif
    if (msg_type == REGISTER_SUCCESS) {
        // make an entry in local database with function name and args
        struct R_S_Message* success_msg = R_S_Message::read(binder_sockfd);
        string sname(name);
        struct procSignature procKey;
        procKey.name = sname;
        procKey.argTypes = argTypes;
        procedure_db[procKey] = f;
        return success_msg->warning_code;
    } else if (msg_type == REGISTER_FAILURE) {
        struct R_F_Message* failure_msg = R_F_Message::read(binder_sockfd);
#ifdef DEBUG
        cerr << "rpcRegister failed\n";
#endif
        return failure_msg->warning_code;
    }
#ifdef DEBUG
    cerr << "unexpected msg type\n";
#endif
    return UNEXPECTED_MSG_TYPE;
}

int rpcExecute() {
    if (procedure_db.empty()) return NO_REGISTERED_PROCEDURES;

    struct sockaddr_storage remoteaddr;
    socklen_t addrlen;

    int retval, msg_type;
    fd_set master_fds, read_fds;

    FD_ZERO(&master_fds);
    FD_ZERO(&read_fds);
    FD_SET(server_sockfd, &master_fds);
    FD_SET(binder_sockfd, &master_fds);
    int fdmax = server_sockfd > binder_sockfd ? server_sockfd : binder_sockfd;

    while(true) {
        read_fds = master_fds;
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) <= 0) {
            continue;
        }

        if (FD_ISSET(server_sockfd, &read_fds)) { 
            // handle new connections
#ifdef DEBUG
            cerr << "Handling connections...\n";
#endif
            addrlen = sizeof remoteaddr;
            int newSocket_fd = accept(server_sockfd, (struct sockaddr *) &remoteaddr, &addrlen);
            if (newSocket_fd < 0) {
#ifdef DEBUG
                cerr << "ERROR: Failure when accepting connection\n";
#endif
                close(newSocket_fd);
                continue;
            }
            FD_SET(newSocket_fd, &master_fds); // add to master set
            if (newSocket_fd > fdmax) {    // keep track of the max
                fdmax = newSocket_fd;
            }
            connections.push_back(newSocket_fd);
            
        } else if (FD_ISSET(binder_sockfd, &read_fds)) {
            // receive terminate message
#ifdef DEBUG
            cerr << "Receiving terminate message...\n";
#endif
            retval = recv(binder_sockfd, &msg_type, sizeof (msg_type), 0);
            if (retval == 0 || msg_type == TERMINATE) {
                close(binder_sockfd);
                return 0;
            } else {
                cerr << "failed to recv terminate message\n";
            } 
        } else {
#ifdef DEBUG
            cerr << "check with connections... \n";
#endif
            for (vector<int>::iterator it = connections.begin(); it != connections.end(); ++it) {
                if (FD_ISSET(*it, &read_fds)) {
                    // handle data from a client
#ifdef DEBUG
                    cerr << "handle data from client... \n";
#endif
                    retval = recv(*it, &msg_type, sizeof (msg_type), 0);
#ifdef DEBUG
                    cerr << "rpcExec - msg_type: " << msg_type<<endl;
#endif
                    if (retval == 0) {
#ifdef DEBUG
                        cerr << "need to remove connection\n";
#endif
                        connections_to_remove.push_back(*it);
                        close(*it);
                        FD_CLR(*it, &master_fds);
                        continue;
                    } else if (retval < 0) {
                        cerr << "failed to recv msg type\n";
                        continue;
                    } else {
#ifdef DEBUG
                        cerr<<msg_type<<endl;
#endif
                        if (msg_type == EXECUTE) {
#ifdef DEBUG
                            cerr << "received execute message type\n";
#endif
                            struct E_Message* e_msg = E_Message::read(*it);
#ifdef DEBUG
                            cerr << "received entire execute message\n";
#endif
                            for (map<struct procSignature, skeleton>::iterator map_it = procedure_db.begin(); map_it != procedure_db.end(); ++map_it) {
#ifdef DEBUG
                                cerr << "verifying signature\n";
#endif
                                if (string (map_it->first.name) == string(e_msg->name) && 
                                        ifSameArgs(e_msg->argTypes, e_msg->length, map_it->first.argTypes, getArgsLength(map_it->first.argTypes))) {
                                    struct thread_data * td = new thread_data();
                                    td->sockfd = *it;
#ifdef DEBUG
                                    cerr << "SOCKET: "<<td->sockfd;
#endif
                                    td->name = e_msg->name;
                                    td->argTypes = e_msg->argTypes;
                                    td->args = e_msg->args;
                                    td->f = map_it->second;

                                    pthread_t thread;
                                    pthread_create(&thread, NULL, &executeProc, (void *)td);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        // close connections
        for (vector<int>::iterator it = connections_to_remove.begin(); it != connections_to_remove.end(); ++it) {
            connections.erase(remove(connections.begin(), connections.end(), *it), connections.end());
            close(*it);
        }

        connections_to_remove.clear();

    }
    return 0;
}

int rpcTerminate() {
    char *binderAddress = getenv("BINDER_ADDRESS");
    if (binderAddress == NULL) {
        return INVALID_BINDER_ADDRESS;
    }
    char *binderPort = getenv("BINDER_PORT");
    if (binderPort == NULL) {
        return INVALID_BINDER_PORT;
    }

    // open connection to binder
    struct sockaddr_in binder_addr;
    int port = atoi(binderPort);
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        return CREATE_SOCKET_FAILURE;
    }
    struct hostent* binder = gethostbyname(binderAddress);
    if (binder == NULL) {
        return UNKNOWN_HOST;
    }

    bzero((char *) &binder_addr, sizeof(binder_addr));
    binder_addr.sin_family = AF_INET;
    bcopy((char *)binder->h_addr,
        (char *)&binder_addr.sin_addr.s_addr,
        binder->h_length);
    binder_addr.sin_port = htons(port);

    connect(sockfd,(struct sockaddr *)&binder_addr, sizeof(binder_addr));
    binder_sockfd = sockfd;

    // Get the host name
    char hostname[256];
    gethostname(hostname, 256);

    // send terminate message
    int msg_type = TERMINATE;

    // send the msg type
    int retval = send(sockfd, &msg_type, sizeof(msg_type), 0);
    if (retval < 0) {
        cerr << "failed to send terminate msg type\n";
        return retval;
    }

    // close connection
    close(binder_sockfd);
    return 0;
}
