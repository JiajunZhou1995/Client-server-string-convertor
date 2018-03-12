#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include "constants.h"
#include "message.h"
#include "binder_db.h"
#include "server_sock_db.h"
#include "message_handler.h"

using namespace std;

/*
struct sockaddr_in {
    short            sin_family;   // e.g. AF_INET
    unsigned short   sin_port;     // e.g. htons(3490), The htonl() function converts the unsigned integer hostlong from host byte order to network byte order.
    struct in_addr   sin_addr;     // see struct in_addr, ip address
    char             sin_zero[8];  // zero this if you want to   we will cast this struct to another struct, this helps make the size the same
};

struct in_addr {
    unsigned long s_addr;  // load with inet_aton()
};  
*/

/*
    helper function that changes the string into title format
*/  

int main() {
    int master_socket, max_instances = 30;                                  // binder will handle at max 15 nodes and 15 servers
    int input_sockets[max_instances];                                       // all the possible input sockets
    struct sockaddr_in server_address;                                      // used to specify the ip and the port that the server socket should bind to
    char hostname[1024];                                                    // hostname (machine name tha will be printed to the console)
    fd_set read_fds;                                                        // a set of sockets
    int max_socket_descriptor;                                              // highest socket descriptor
    int select_result;                                                      // indicator whether select has been successful
    int new_socket;                                                         // used to create new socket
    int len;                                                                // length of the server_address object
    int read_result;                                                        // result of reading the input
    int descriptor;
    struct binder_db *db = new binder_db();                                        // the db used to save on the register messages
    struct server_sock_db *s_db = new server_sock_db();                            // db that saves all the mapping between server socketid to server name 
    // AF_INET means the socket is towards internet, and SOCK_STREAM means it is an TCP socket, 0 means let the OS choose what protocal to use
    master_socket = socket(AF_INET, SOCK_STREAM, 0);  // the master socket that keeps listening to new connections
    if (master_socket == -1) {
        cerr << "could not create server socket, program exit with exit code 0\n";
        exit(0);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(0);               // bound to the next avaliable port
    server_address.sin_addr.s_addr = INADDR_ANY;      // This field contains the IP address of the host.

    len = sizeof(server_address);
    // Now, try to bind master_socket
    if (bind(master_socket, (struct sockaddr *)&server_address, len) == -1) {
        cerr << "could not bind server socket, program exit with exit code 0\n";
        exit(0);
    }

    // print server address and server port
    hostname[1023] = '\0';
    gethostname(hostname, 1023);
    getsockname(master_socket, (struct sockaddr *)&server_address, (socklen_t *)&len);
    printf("BINDER_ADDRESS %s\n", hostname);
    printf("BINDER_PORT %d\n", ntohs(server_address.sin_port));

    // make sure there is no garbage value in the input_sockets array
    for (int i = 0; i < max_instances; i++) {
        input_sockets[i] = 0;
    }

    // make sure there are maximally 1000 connections
    if (listen(master_socket, 1000) < 0) {
        perror("there are more than 1000 connections at the same time, program exit with 0");
        exit(0);
    }

    // now start infinite loop waiting for clients and servers to come
    while(1) {
        FD_ZERO(&read_fds);                     // clear the node_fds
        FD_SET(master_socket, &read_fds);
        max_socket_descriptor = master_socket;

        for (int i=0; i < max_instances; i++) {
            descriptor = input_sockets[i];

            if (descriptor > 0) {
                FD_SET(descriptor, &read_fds);
            }

            if (descriptor > max_socket_descriptor) {
                max_socket_descriptor = descriptor;
            }   
        }

        // use select function call to listen for activities on one of the sockets
        select_result = select(max_socket_descriptor + 1, &read_fds, NULL, NULL, NULL);
        if (select_result < 0) {
            cerr << "error with calling select function\n";
        }
        /*
            a new connection, create a new socket calling accept, add it fd set
            also add the descriptor to the array of descriptors (input_sockets)
        */
        if (FD_ISSET(master_socket, &read_fds)) {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&server_address,  (socklen_t*)&len)) == -1) {
                cerr << "error creating new node socket with accept function call, binder continue to run\n";
                continue;
            }

            // find an open spot and add it to the array
            for (int i = 0; i < max_instances; i++) {
                if (input_sockets[i] == 0) {
                    input_sockets[i] = new_socket;
                    break;
                }
            }
        }

        /*
            if contorl flow runs here, there are two possibilities:
                1: connection disconnect
                2: connection sends message
            the following loop checks for input socket input events, it will send back the response 
        */
        for (int i= 0; i < max_instances; i++) {
            int node = input_sockets[i];
            if (FD_ISSET(node, &read_fds)) {
                int message_type;
                char *buffer;
                int sent;
                if ((read_result = recv(node, &message_type, sizeof(int), 0)) == 0) { // node closing the socket
#ifdef DEBUG
                    cerr << "closing socket"<<endl;
#endif
                    handle_node_disconnect(node, db, s_db);
#ifdef DEBUG
                    cerr << "disconnect handled"<<endl;
#endif
                    close(node);
                    input_sockets[i] = 0;
                } else {
                    // node sending actual input
                    // first, read first 4 bytes to determine what type of message it is
                    switch(message_type) {
                        case REGISTER:
                            handle_register_message(db, s_db, node);
                            break;
                        case LOC_REQUEST: 
#ifdef DEBUG
                            cerr << "loc request message recieved\n";
#endif
                            handle_loc_request_message(db, node);
                            break;
                        case CACHE_LOC_REQUEST:
#ifdef DEBUG
                            cerr <<  "cache loc request message recieved\n";
#endif
                            handle_cache_loc_request_message(db, node);
                            break;
                        default:
                            // this suppose to be terminate message
                            // everything should end here, tell everybody to shut down, clean up
                            s_db->broadcast_terminate_message();
                            return 0;
                            // delete db;
                            // delete s_db;
                    }
                }
            }
        }
    }
}
