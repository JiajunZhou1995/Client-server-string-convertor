#if !defined(MESSAGE_H)
#define MESSAGE_H 1

#include <cstddef>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// server/binder messages
// register massage
struct R_Message {
    char *server_identifier;
    int port;
    char* procedure_name;
    int* argTypes;
    int length;

    R_Message() : server_identifier (NULL), port (0), procedure_name (NULL), argTypes(NULL), length(0) {}
    R_Message(char * server_id, int portnum, char* proc_name, int* arg_types, int l)
        : server_identifier (server_id), port (portnum), procedure_name (proc_name), argTypes(arg_types), length(l) {
        }
    ~R_Message() {
       /* delete [] server_identifier;
        delete [] procedure_name;
        delete [] argTypes;*/
    }

    static struct R_Message* read(int sockfd);
    int sendmsg(int sockfd);
};

// register success message
struct R_S_Message {
    int warning_code;

    R_S_Message() : warning_code (0) {}
    R_S_Message(int wc): warning_code (wc) {}
    ~R_S_Message() {}

    static struct R_S_Message* read(int sockfd);
    int sendmsg(int sockfd);
};

// register failure message
struct R_F_Message {
    int warning_code;

    R_F_Message() : warning_code (0) {}
    R_F_Message(int wc): warning_code (wc) {}
    ~R_F_Message() {}

    static struct R_F_Message* read(int sockfd);
    int sendmsg(int sockfd);
};


// client/binder messages

// location request message, from client to binder
struct C_LOC_R_Message {
    char* procedure_name;
    int* argTypes;
    int length;

    C_LOC_R_Message(): procedure_name(NULL), argTypes(NULL), length(0) {}
    C_LOC_R_Message(char* proc_name, int* arg_types, int l) : procedure_name(proc_name), argTypes(arg_types), length(l) {}
    ~C_LOC_R_Message() {
/*
        delete [] procedure_name;
        delete [] argTypes;
*/
    }

    static struct C_LOC_R_Message* read(int sockfd);
    int sendmsg(int sockfd);
};


// location request message, from client to binder
struct LOC_R_Message {
    char* procedure_name;
    int* argTypes;
    int length;

    LOC_R_Message(): procedure_name(NULL), argTypes(NULL), length(0) {}
    LOC_R_Message(char* proc_name, int* arg_types, int l) : procedure_name(proc_name), argTypes(arg_types), length(l) {}
    ~LOC_R_Message() {
/*
        delete [] procedure_name;
        delete [] argTypes;
*/
    }

    static struct LOC_R_Message* read(int sockfd);
    int sendmsg(int sockfd);
};

// location success message, from binder to client
struct LOC_S_Message {
    char *server_identifier;
    int port;

    LOC_S_Message(): server_identifier(NULL), port(0) {}
    LOC_S_Message(char* server_id, int portnum) : server_identifier (server_id), port(portnum) {}
    ~LOC_S_Message() {
  //      delete [] server_identifier;
    }

    static struct LOC_S_Message* read(int sockfd);
    int sendmsg(int sockfd);
};

// location failure message, from binder to client
struct LOC_F_Message {
    int reason_code;

    LOC_F_Message(): reason_code(0) {}
    LOC_F_Message(int rc) : reason_code(rc) {}
    ~LOC_F_Message() {}

    static struct LOC_F_Message* read(int sockfd);
    int sendmsg(int sockfd);
};


// client/server messages
// execute message
struct E_Message {
    char * name;
    int* argTypes;
    void** args;
    int length;

    E_Message() : name(NULL), argTypes(NULL), args(NULL), length(0) {} 
    E_Message(char* n, int* arg_types, void** a, int l) : name(n), argTypes(arg_types), args(a), length(l) {}
    ~E_Message() {
    /*    delete [] name;
        delete [] args;
        delete [] argTypes;
*/
    }

    static struct E_Message* read(int sockfd);
    int sendmsg(int sockfd);
};

// execute success message
struct E_S_Message {
    char * name;
    int* argTypes;
    void** args;
    int length;

    E_S_Message() : name(NULL), argTypes(NULL), args(NULL), length(0) {}
    E_S_Message(char* n, int* arg_types, void** a, int l) : name(n), argTypes(arg_types), args(a), length(l) {}
    ~E_S_Message() {
/*
        delete [] name;
        delete [] args;
        delete [] argTypes;
*/
    }

    static struct E_S_Message* read(int sockfd);
    int sendmsg(int sockfd);
};

// execute failure message
struct E_F_Message {
    int reason_code;

    E_F_Message() : reason_code(0) {}
    E_F_Message(int rc) : reason_code(rc) {}
    ~E_F_Message() {}

    static struct E_F_Message* read(int sockfd);
    int sendmsg(int sockfd);
};

#endif
