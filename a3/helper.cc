#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <iostream>
#include "constants.h"
#include "error.h"
#include "message.h"
#include "rpc.h"

using namespace std;

/*
 * utility functions
 */

// socketId is the socket we are reading from, and the status shows whether the read is successful or not
// receive functions
int readInt(int socketId, int* status) {
    int content;                                        // store read content
    int type;

    // first, read type of the input
    // if status < 0, then the read has failed
    *status = recv(socketId, &type, sizeof(int), 0);    
    if (*status < 0) {
        cerr << "error in readInt of reading type" << endl;
        return -1;
    }

    if (type != INTEGER) {
        *status = TYPE_ERROR;
        cerr << "type error in readInt" << endl;
        return -1;
    }

    // second, read the content
    *status = recv(socketId, &content, sizeof(int), 0);

    return content;
}


char* readString(int socketId, int* status) {
    int strLength;
    int type;
    char* buffer;
    
    // first, read the length of the string
    *status = recv(socketId, &strLength, sizeof(int), 0);
    if (*status < 0) {
        cerr << "error in readString of reading strLength" << endl;
        return NULL;
    }

    if (strLength <= 0) {
        *status = INVALID_LENGTH_ERROR;
        cerr << "Invalid length in readString" << endl;
        return NULL;
    }

    // second, read the type of the input and check if it is string
    *status = recv(socketId, &type, sizeof(int), 0);
    if (*status < 0) {
        cerr << "error in readString of reading type" << endl;
        return NULL;
    }

    if (type != STRING) {
        *status = TYPE_ERROR;
        cerr << "type error in readString" << endl;
        return NULL;
    }

    // step 3 read the actual content of the string
    int byteLength = sizeof(char) * strLength;
    buffer = static_cast<char*>(malloc(byteLength));
    *status = recv(socketId, &buffer, byteLength, 0);

    return buffer;
}

int* readIntArray(int socketId, int* status) {
    int arrayLength;
    int type;
    int* buffer;

    // first, read the legnth of the array
    *status = recv(socketId, &arrayLength, sizeof(int), 0);
    if (*status < 0) {
        cerr << "error in readIntArray of reading legnth" << endl;
        return NULL; 
    }

    if (arrayLength <= 0) {
        *status = INVALID_LENGTH_ERROR;
        cerr << "Invalid length error in readIntArray" << endl;
    }

    // second, read the type of the input
    *status = recv(socketId, &type, sizeof(int), 0);

    if (*status < 0) {
        cerr << "error in readIntArray of reading type" << endl;
        return NULL; 
    }

    if (type != INT_ARRAY) {
        *status = TYPE_ERROR;
        cerr << "Invalid length error in readIntArray" << endl;
        return NULL;
    }

    // third, read the actual int array content
    int byteLength = sizeof(int) * arrayLength;
    buffer = static_cast<int*>(malloc(byteLength));
    *status = recv(socketId, &buffer, byteLength, 0);
    return buffer;
}

/* send functions
 */

int sendInt(int socketId, int content) {
    int status;
    int type = INTEGER;                                   
    // first, send type of the input
    // if status < 0, then the read has failed
    status = send(socketId, &type, sizeof(int), 0);     
    if (status < 0) {
        cerr << "error in sendInt of sending type" << endl;
        return status;
    }

    // second, send the content
    status = send(socketId, &content, sizeof(int), 0);

    return status;
}

int sendIntArray(int socketId, int* a, int len) {
    int type = INT_ARRAY;
    int status;
    // step 1, send the length of the int array
    status = send(socketId, &len, sizeof(int), 0);
    if (status < 0) {
        cerr << "error in sendIntArray when sending type" << endl;
        return status;
    }
    // step 2, send the type
    status = send(socketId, &type, sizeof(int), 0);
    if (status < 0) {
        cerr << "error in sendIntArray when sending type" << endl;
        return status;
    }

    // step 3, send the actual int array
    status = send(socketId, &type, sizeof(int)*len, 0);
    return status;
}

// send string
int sendString(int socketId, char* c, int len) {
    int status;
    int type = STRING;

    // first, send the length of the string
    status = send(socketId, &len, sizeof(int), 0);
    if (status < 0) {
        cerr << "error in sending type in sendString" << endl;
        return status;
    }

    // second, send the type of teh string
    status = send(socketId, &type, sizeof(int), 0);
    if (status < 0) {
        cerr << "error in sending length in send stirng" << endl;
        return status;
    }

    // third, send the actual content of the string
    status = send(socketId, &c, sizeof(char)*len, 0);
    return status;
}

/*
 * functions related to read different type of messages
 */

struct R_Message* readRegisterMessage(int socketId) {
    /* the message is in the format of: server_identifier, port, name, argType
       the corresponding type is string, port, string and int*
     */
    int status = 0;

    // step 1, read string from the 
    char *server_identifier = readString(socketId, &status);
    if (status < 0) {
        cerr << "error in reading server_identifier at function readRegisterMessage" << endl;
        return NULL;
    }

    // step 2, read port number
    int port = readInt(socketId, &status);
    if (status < 0) {
        cerr << "error in reading port number at function readRegisterMessage" << endl;
        return NULL;
    }

    // step 3, read procedure name
    char* name = readString(socketId, &status);
    if (status < 0) {
        cerr << "error in reading procedure name at function readRegisterMessage" << endl;
        return NULL;
    }

    // step 4, read argType
    int* argType = readIntArray(socketId, &status);
    if (status < 0) {
        cerr << "error in reading argType at function readRegisterMessage" << endl;
        return NULL;
    }

    // if the control makes here, that means we have successfully read the input
    // now we just create the struct and return it
    // R_Message* r = create_R_Message(server_identifier, port, name, argType);
    // return r;
    return NULL;    // shut the complier up, need to implement the logic here if we want to use this function
}

bool ifSameArgs(int *a, int len1, int *b, int len2) {
    int t1, t2; // type
    int l1, l2; // length
    if (len1 != len2)
        return false;
    for (int i=0; i < len1 - 1; i++) {
        t1 = ((15 << 16) & a[i]) >> 16;
        t2 = ((15 << 16) & b[i]) >> 16;
        l1 = ((1 << 16) - 1) & a[i];
        l2 = ((1 << 16) - 1) & b[i];
        if (t1 != t2)
            return false;
        if ((l1 == 0 && l2 != 0) || (l1 != 0 && l2 == 0))
            return false;
    }
    return true;
}



/*
 * calculate length of args
 */
int getArgsLength (int* argTypes) {
    int len = 0;
    while (argTypes[len] != 0) {
        len++;
    }
    return len + 1;
}

/*
 * recv arguments with argTypes
 */
void** recvArgs(int sockfd, int len, int* argTypes) {
    int argslen = len - 1;
    void** args = new void*[argslen];
    int retval, size;
    for (int i = 0; i < argslen; i++) {
        retval = recv(sockfd, &size, sizeof(size), 0);
        if (retval < 0) {
            cerr << "failed to receive size of arg when recving args\n";
            for (int j = 0; j < i; j++) {
                free(args[j]);
            }
            return NULL;
        }
 
        void* arg = (void *)malloc(size);
    
        retval = recv(sockfd, arg, size, 0);
        if (retval < 0) {
            cerr << "failed to receive args\n";
            return NULL;
        }
        args[i] = arg;
    }
    return args;
}

/*
 * send arguments with argTypes
 */
int sendArgs(int sockfd, int len, int* argTypes, void** args) {
#ifdef DEBUG
    cerr << "sendArgs - argsLen: "<< len <<endl;
#endif
    int argslen = len - 1;
    int retval, argType, arglen, s_argType, s_arg;
    for (int i = 0; i < argslen; i++) {
        argType = (argTypes[i] & (15 << 16)) >> 16;
        arglen = (argTypes[i] & ((1 << 16) - 1));

#ifdef DEBUG
        cerr << "sendArgs - argType: " << argType <<endl;
        cerr << "sendArgs - arglen: " << arglen <<endl;
#endif

        void* arg = args[i];
        if (arglen == 0) {
            arglen = 1;
        }

        switch (argType) {
            case ARG_CHAR:
                s_argType = sizeof(char);
                break;
            case ARG_SHORT:
                s_argType = sizeof(short);
                break;
            case ARG_INT:
                s_argType = sizeof(int);
                break;
            case ARG_LONG:
                s_argType = sizeof(long);
                break;
            case ARG_DOUBLE:
                s_argType = sizeof(double);
                break;
            case ARG_FLOAT:
                s_argType = sizeof(float);
                break;
        }

        s_arg = s_argType * arglen;
#ifdef DEBUG
        cerr << "sendArgs - s_arg: " <<s_arg<<endl;
#endif
        retval = send(sockfd, &s_arg, sizeof(s_arg), 0);
        if (retval < 0) {
            cerr << "failed to send arg size while sending arguments (void**)\n";
            return retval;
        }
    
        retval = send(sockfd, arg, s_arg, 0);
        if (retval < 0) {
            cerr << "failed to send arg while sending arguments (void**)\n";
            return retval;
        }
    }

    return 0;
}
