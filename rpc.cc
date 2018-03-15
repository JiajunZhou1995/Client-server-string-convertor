
#include <string>
#include <errno.h>
#include <iostream>
#include <netdb.h>
#include <unistd.h>
#include "common.h"
#include "rpc.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>
#include <pthread.h>
#include <vector>
#include <queue>
#include <cstring>
#include <assert.h>

int terminate = -1;
int server_fd = -1;
int binder_fd = -1;
char * binder_port = getenv("BINDER_PORT");
const char* binder_hostname = getenv("BINDER_ADDRESS");

struct Procedure
{
    char *name;
    int *argTypes;
    skeleton f;
};
std::vector<Procedure> func_container;
std::vector<skeleton> skels;
// std::queue<pthread_t *> execute_children;

int connect_to(int port, const char* binder_host) {
	assert(server_fd != -1);
	struct hostent *host;
	if (binder_host == NULL) {
		return HOSTNAME_ERROR;
	}
	host = gethostbyname(binder_host);
	if (host == NULL) {
		return HOSTNAME_ERROR;
	}
	char * ip;
	ip = inet_ntoa(*((struct in_addr *)host->h_addr_list[0]));
	if (ip == NULL) {
		return FAIL_TO_CONNECT;
	}
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		return INVALID_FD;
	}
    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_port = htons(port);
	if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) == -1){
		close(fd);
		return FAIL_TO_CONNECT;
	}
	return fd;
}

int rpcInit(void) {
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in sock;
	sock.sin_family = AF_INET;
	sock.sin_addr.s_addr = INADDR_ANY;
	sock.sin_port = htons(0);
	int ret = bind(server_fd,(struct sockaddr*) &sock, sizeof(sock));
	if (ret < 0) {
		close(server_fd);
		return FAIL_TO_BIND;
	}
	ret = listen(server_fd, 10);
	if (ret < 0) {
		close(server_fd);
		return FAIL_TO_LISTEN;
	}
	if (binder_port == NULL) {
		return INVALID_PORT;
	}
	binder_fd = connect_to(atol(binder_port),binder_hostname);
	if (binder_fd < 0) {
		return binder_fd;
	}
	return COMPLETE;
}

//  the following functions are used to debug
// int checkArgs(char *name, int* types, skeleton f, int* name_len) {
// 	if (name == NULL || name[0] == '\n' || strlen(name) > 64) {
// 		return INVALID_NAME;
// 	} else if (types == NULL) {
// 		return INVALID_ARGTYPE;
// 	} else if (f == NULL) {
// 		return INVALID_SKELETON;
// 	} 
// 	for (unsigned i = 0; name[i] != 0; i++) {
// 		if (name[i] < 1 || name[i] > 6) {
// 			return INVALID_ARGTYPE;
// 		}
// 	}
// 	return COMPLETE;
// }

int checkSkeleton(skeleton f) {
	for (unsigned i = 0; i < skels.size(); i++) {
		if (skels[i] == f) {
			return SKELETON_HAS_ALREADY_REGISTERED;
		} 
	}
	return COMPLETE;
} 


int sendRegisterInfo(char *name, int* argTypes) {
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	getsockname(server_fd, (struct sockaddr *)&sin, &len);
	size_t host_size = 256;
	char hostname[host_size];
	gethostname(hostname, host_size);
	int port = ntohs(sin.sin_port);
    int ip_len = 128;
    int i;
    // count argType length except the last item
    for (i = 0; argTypes[i] != 0; i++)
    {
    }
    int name_len = 64;
    int total_len = sizeof(int) + sizeof(MessageType) + ip_len * sizeof(char) + sizeof(int) + name_len * sizeof(char) + i * sizeof(int);
    // send total length
    send(binder_fd, &total_len, sizeof(int), 0);
    MessageType r = REGISTER;
	// send REGISTER
	send(binder_fd,&r,sizeof(MessageType),0);
	// send ip length and hostname
    
	// send(binder_fd, &ip_len ,sizeof(int),0);
	send(binder_fd, hostname ,ip_len * sizeof(char),0);
	// send  port number
	send(binder_fd, &port,sizeof(int),0);
	// send name length and name
	//send(binder_fd,&name_len,sizeof(int),0);
	send(binder_fd, name ,name_len,0);
	

	// send length of argtypes and argtypes
	send(binder_fd,&i,sizeof(int), 0);
	send(binder_fd, argTypes, i * sizeof(int), 0);
	// receive messageTpe returned from binder
	recv(binder_fd,&r,sizeof(int),0);
	recv(binder_fd,&i,sizeof(int),0);
    ErrorMsg returnValue;
    int ret = recv(binder_fd, &returnValue, sizeof(ErrorMsg), 0);
    // switch (r) {
	// 	case REGISTER_SUCCESS:
	// 		return COMPLETE;
	// 	case REGISTER_FAILURE:
	// 		return returnValue;

	// }
	// returnValue could be 0, positive for warning or negative for errors
	return returnValue;
	
}
int addProcedure(char *name, int* argTypes, skeleton f) {
	struct Procedure new_proc = {name,argTypes,f};
	assert(func_container.size() == skels.size());
	func_container.push_back(new_proc);
	skels.push_back(f);
	return COMPLETE;
}
int updateProcedure(char *name, int* argTypes, skeleton f){
	for (unsigned i = 0; i < func_container.size(); i++) {
		struct Procedure p = func_container[i];
		if (p.name == name) {
			std::vector<int> types;
			unsigned j;
			for (unsigned j = 0; argTypes[j] != 0 && p.argTypes[j] != 0; i++) {
				if (argTypes[j] != p.argTypes[j]) {
					break;
				}
				
			}
			if (p.argTypes[j] == 0 && argTypes[j] == 0) {
				if (j == 0 || argTypes[j-1] == p.argTypes[j-1]) {
					skels.erase(skels.begin()+i);
					skels.insert(skels.begin()+i,f);
					func_container[i].f = f;
					return COMPLETE;
				}
			}
			
		}
	}
	return SKELETON_NOT_FOUND;
}
int recvRequestFromBinder(int binder, int * port, char * hostname) {
	MessageType answer;
	recv(binder,&answer,sizeof(MessageType),0);
	if (answer == LOC_SUCCESS) {
		int ip_len;
		recv(binder,&ip_len,sizeof(int),0);
		recv(binder,hostname,ip_len * sizeof(char),0);
		// port number
		recv(binder,&port,sizeof(int),0);
		return 0;
	} else if (answer == LOC_FAILURE) {
		int reason;
		recv(binder,&reason,sizeof(int),0);
		return reason;
	} else {
		return UNEXPECTED_MESSAGE;
	}
}
void send_name_and_argtypes_from_client(int fd, int len, MessageType request, char * name, int * argTypes) {
	send(fd,&request,sizeof(MessageType),0);
	// name 
	send(fd,name,64 * sizeof(char),0);
	send(fd,&len,sizeof(int),0);
	send(fd,argTypes,len * sizeof(int),0);
}
void receive_name_argtype_and_args(int fd, int *len, char *name, int *argtypes, void **args)
{
    recv(fd, name, 64 * sizeof(char), 0);
    recv(fd, &len, sizeof(int), 0);
    recv(fd, argtypes, (*len) * sizeof(int), 0);
    *len -= 1;
    for (int i = 0; i < *len; i++)
    {
        int arg_len, arg_tp, arg_size;
        recv(fd, &arg_len, sizeof(int), 0);
        recv(fd, &arg_tp, sizeof(int), 0);
        arg_size = arg_len * arg_tp;
        *(args + i) = malloc(arg_size);
        recv(fd, *(args + i), arg_size, 0);
    }
}

int rpcCall(char* name, int* argTypes, void** args) {
    if (binder_port == NULL)
    {
        return INVALID_PORT;
    }
    int binder = connect_to(atol(binder_port),binder_hostname);
	if (binder < 0) {
		return binder;
	}
    unsigned i;
    int argType_length;
     for (i = 0; argTypes[i] != 0; i++) {}
    argType_length = i;
    MessageType request = LOC_REQUEST;
	send_name_and_argtypes_from_client(binder, argType_length, request, name, argTypes);
	// int name_len;
	// int argType_length;
	// send(binder,&request,sizeof(MessageType),0);
	// // name 
	// send(binder,name,64 * sizeof(char),0);
	// send(binder,&i,sizeof(int),0);
	// send(binder,argTypes,argType_length * sizeof(int),0);
	char ip[256];
	int port;
	int ret = recvRequestFromBinder(binder,&port,ip);
	if (ret < 0) {
		// LOC_FAILURE
		return ret;
	}
	int to_server = connect_to(port,ip);
	if (to_server < 0) {
		return FAIL_TO_CONNECT;
	}
	request = EXECUTE;
	send_name_and_argtypes_from_client(to_server, argType_length, request, name, argTypes);
	//send length, type and arg of each
	for (int i = 0; i < argType_length-1; i++) {
		int arg_tp = (argTypes[i] >> 8) & 0xff; // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		int arg_len = argTypes[i] & 0x0000FFFF;  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		send(to_server,&arg_len,sizeof(int),0);
		send(to_server,&arg_tp,sizeof(int),0);
		int arg_size = arg_len*arg_tp;
		send(to_server, (args+i),arg_size,0);
	}
	// receive part 
	int len;
	MessageType answer;
	char func_name[64];
	int * argtypes;
	void ** func_args =(void **) malloc(sizeof (void *));
	recv(to_server,&answer,sizeof(MessageType),0);
	switch(answer){
		int reason;
		case EXECUTE_FAILURE:
			recv(to_server,&reason,sizeof(int),0);
			break;
		case EXECUTE_SUCCESS:
			receive_name_argtype_and_args(to_server,&len,func_name,argtypes,func_args);
			break;
		default:
			return UNEXPECTED_MESSAGE;
	} 
	return COMPLETE;
}



int rpcCacheCall(char* name, int* argTypes, void** args){
	return COMPLETE;
}

int rpcRegister(char *name, int* argTypes, skeleton f) {
	if (binder_fd < 0 || server_fd < 0) {
		return NOT_INIT;
	}
	// Used for debugging
	// int ret = checkArgs(name,argTypes,f);
	// if (ret < 0) {
	// 	return ret;
	// }
	// ret = connect_to(binder_port,binder_hostname);
	// if (ret < 0) {
	// 	return ret;
	// } 
	// ret = checkSkeleton(f);
	// if (ret != 0) {
	// 	// In this case, 
	// 	return ret;
	// }
    assert(binder_port);
	int fd = connect_to(atol(binder_port),binder_hostname);
	if (fd < 0) {
		return fd;
	}
	int ret = sendRegisterInfo(name,argTypes);
	if (ret >= 0) {
		int s = updateProcedure(name,argTypes,f);
		if (s > 0) {
			addProcedure(name,argTypes,f);
		}
	}
	return ret;
}

void * receive_termination(void* fd) {
	int sock = *((int *) fd);
	assert(sock >= 0);
	MessageType ret;
	while (true) {
		recv(sock,&ret,sizeof(MessageType),0);
		if (ret == TERMINATE) {
			terminate = 1;
			break;
		}
	}
	pthread_exit(NULL);
}

int look_for_matched_skeleton(char *given_name, int *argtypes)
{
    int ret = -1;
    for (unsigned i = 0; i < func_container.size(); i++)
    {
        bool found = true;
        char *name = (func_container.at(i)).name;
        int *types = (func_container.at(i)).argTypes;
        if (strcmp(given_name, name))
        {
            for (unsigned j = 0; argtypes[j] != 0 && types[j] != 0; j++)
            {
                if (argtypes[j] != types[j])
                {
                    found = false;
                    break;
                }
            }
            if (found)
            {
                return i;
            }
        }
    }
    return ret;
}

void * execute(void* sock) {
	int fd = *((int *) sock);
	MessageType msg_type;
	recv(fd,&msg_type,sizeof(MessageType),0);
	if (msg_type == EXECUTE) {
		// receive 
		int len;
		char name[64];
		int * argtypes;
        void **args = (void **) malloc(len * sizeof(void *));
        receive_name_argtype_and_args(fd, &len, name, argtypes, args);
		// recv(fd,name,64*sizeof(char),0);
		// recv(fd,&len,sizeof(int),0);
		// recv(fd,argtypes,len*sizeof(int),0);
		// len -= 1;
		// for (int i = 0; i < len; i++) {
		// 	int arg_len,arg_tp,arg_size;
		// 	recv(fd,&arg_len,sizeof(int),0);
		// 	recv(fd,&arg_tp,sizeof(int),0);
		// 	arg_size = arg_len * arg_tp;
		// 	*(args+i) = malloc(arg_size);
		// 	recv(fd, *(args+i),arg_size,0);
		// }
		// find skeleton
		ErrorMsg returnValue;
		MessageType return_type;
		bool send_back = false;
		int index = look_for_matched_skeleton(name,argtypes);
		if (index) {
			//skeleton * f = & (skels.at(index));
            returnValue = (ErrorMsg)(skels.at(index))(argtypes, args);
            if (returnValue) {
				return_type = EXECUTE_SUCCESS;
				// SUCCESS
				send(fd,&return_type,sizeof(int),0);
				//  name
				send(fd,name,64*sizeof(char),0);
				len += 1;
				// argtypes_len and argtypes
				send(fd,&len,sizeof(int),0);
				send(fd,argtypes,len*sizeof(int),0);
				// args
				for (unsigned k = 0; k < len-1; k++) {
                    int arg_tp = (argtypes[k] >> 8) & 0xff; // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                    int arg_len = argtypes[k] & 0x0000FFFF; // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                    send(fd, &arg_len, sizeof(int), 0);
                    send(fd, &arg_tp, sizeof(int), 0);
                    int arg_size = arg_len * arg_tp;
					send(fd,(args+k),arg_size,0);
				}
				send_back = true;
			}
		} 
		if (!send_back) {
			return_type = EXECUTE_FAILURE;
			send(fd, &return_type,sizeof(int),0);
			send(fd,&returnValue,sizeof(int),0);
		}

	}
	close(fd);
	pthread_exit(NULL);
}
int rpcExecute(void) {
	if (server_fd < 0 || binder_fd < 0) {
		return NOT_INIT;
	}
	if (skels.size() == 0) {
		return NOT_REGISTER;
	}
    pthread_t terminate_thread;
    pthread_create(&terminate_thread, NULL, receive_termination,(void *) &binder_fd);
	while (terminate < 0) {
		struct sockaddr_in addr;
		socklen_t addr_len = sizeof(struct sockaddr_in);
		int fd = accept(server_fd,(struct sockaddr *) &addr, &addr_len);
		if (fd >= 0) {
			pthread_t child;
			pthread_create(&child, NULL, execute, (void *)&fd);
		} else if (terminate < 0) {
			break;
		}
		
		// int msg_type;
		// recv(fd,&msg_type,sizeof(int),0);
		// if (msg_type == EXECUTE) {
		// 	// receive 
		// 	int name_len;
		// 	int len;
		// 	char name[name_len];
		// 	recv(fd,&name_len,sizeof(int),0);
		// 	recv(fd,name,name_len*sizeof(char),0);

		// 	recv(fd,&len,sizeof(int),0);
		// 	int * argtypes;
		// 	recv(fd,argtypes,len*sizeof(int),0);
		// 	len -= 1;
		// 	void ** args = new void*[len];
		// 	std::queue<int> arg_sizes;
		// 	for (unsigned i = 0; i < len; i++) {
		// 		int arg_len,arg_tp;
		// 		recv(fd,&arg_len,sizeof(int),0);
		// 		recv(fd,&arg_tp,sizeof(int),0);
		// 		int arg_size = arg_len*arg_tp;
		// 		arg_sizes.push(arg_size);
		// 		args[i] = malloc(arg_size);
		// 		recv(fd, (args+i),arg_size,0);
		// 	}
		// 	// find skeleton
		// 	int returnValue;
		// 	int return_type;
		// 	bool send_back = false;
		// 	int index = look_for_matched_skeleton(name,argtypes);
		// 	if (index) {
		// 		skeleton * f = skels.at(index);
		// 		returnValue = f(argtypes,args);
		// 		if (returnValue) {
		// 			return_type = EXECUTE_SUCCESS;
		// 			reason = SKELETON_NOT_FOUND;
		// 			// SUCCESS
		// 			send(fd,return_type,sizeof(int),0);
		// 			// name_len && name
		// 			send(fd,name_len,sizeof(int),0);
		// 			send(fd,name,name_len*sizeof(char),0);
		// 			len += 1;
		// 			// argtypes_len and argtypes
		// 			send(fd,&len,sizeof(int),0);
		// 			send(fd,argtypes,len*sizeof(int),0);
		// 			// args
		// 			for (unsigned k = 0; k < len-1; k++) {
		// 				int size = arg_sizes.pop()
		// 				send(fd,size,sizeof(int),0);
		// 				send(fd,(args+k),size,0);
		// 			}
		// 			send_back = true;
		// 		}
		// 	} 
		// 	if (!send_back) {
		// 		return_type = EXECUTE_FAILURE;
		// 		send(fd, &return_type,sizeof(int),0);
		// 		send(fd,&returnValue,sizeof(int),0);
		// 	}

		// }
	}
	close(server_fd);
	return COMPLETE;

}

int rpcTerminate(void) {
	if (binder_fd < 0 || server_fd < 0) {
		return NOT_INIT;
	} else {
		MessageType m = TERMINATE;
		send(binder_fd,&m,sizeof(MessageType),0);
		close(binder_fd);
		return COMPLETE;
	}
}


