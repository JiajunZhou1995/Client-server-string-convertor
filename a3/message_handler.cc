#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "message_handler.h"
#include "error.h"
#include "constants.h"

void handle_register_message(struct binder_db* db, struct server_sock_db* s_db, int node) {
    struct R_Message *r = R_Message::read(node);
    if (r== NULL) {
        cerr << "error in reading server register message" << endl;
        R_F_Message *rf = new R_F_Message(REGISTER_FAILURE);
        rf->sendmsg(node);
    } else {
        int status;                                     // detect how the register goes here, if the replacement happnes or not
#ifdef DEBUG
        cerr << "recived a register message" << endl;
#endif
        db->add_r_message(r, &status);
        s_db->add_server_socket_id(node, r->server_identifier);
        R_S_Message *rs = new R_S_Message(status);
        rs->sendmsg(node);
    }
}

void handle_loc_request_message(struct binder_db* db, int node) {
    struct LOC_F_Message* lf=NULL;                                                // loc fail message, use if needed
    struct LOC_S_Message* ls=NULL;                                                // loc success message, use if needed
    struct LOC_R_Message* lr=NULL;                                                // loc register message
    struct R_Message *r;                                                          // register message in the db

    // step 1, read the lr message
    lr = LOC_R_Message::read(node);
    

    /*check message recived properly
     * if no, return net work error
     * if yes, try to search in db
     *      if find fail, return reason code
     *      else, return success message
     */
    if (lr==NULL) {
        cerr << "error in reading locaction request message" << endl;
        lf = new LOC_F_Message(NETWORK_ERROR);
        lf->sendmsg(node);
    } else {
        r = db->find_r_message(lr);
        if (r== NULL) {
            cerr << "procedure not found" << endl;
            lf = new LOC_F_Message(PROCEDURE_NOT_FOUND);
            lf->sendmsg(node);
        } else {
            char* server_identifier = new char[256];
            strncpy(server_identifier, r->server_identifier, 256);
            ls = new LOC_S_Message(server_identifier, r->port);
            ls->sendmsg(node);
        }
    }
//    delete lf;
//    delete ls;
//    delete lr;
}

void handle_node_disconnect(int sock, struct binder_db* db, struct server_sock_db* s_db) {
    /*
     * check if the disconnected is a server
     *    if yes,  get the name of that server, and erase all the procedures registered by it
     *.   else, doing nothing
     */
    if (s_db->if_socket_from_server(sock)) {
        char *server_name = s_db->server_name_from_sock(sock);
#ifdef DEBUG
        cerr << "handle dc - server name: " << server_name << endl;
#endif
        db->remove_message_from_particular_server(server_name);
#ifdef DEBUG
        cerr << "handle dc - message removed: " << endl;
#endif
        s_db->remove_server_socket_id(sock);
#ifdef DEBUG
        cerr << "handle dc - message removed: " << endl;
#endif
    }
}

// void handle_cache_loc_request_message(struct binder_db * db, int node) {
//     struct LOC_S_Message* ls = NULL;
// #ifdef DEBUG
//     cerr << "handle cache loc r msg" << endl;
//     cerr << "node when receiving :" <<node <<endl;
// #endif
//     struct C_LOC_R_Message *lr = C_LOC_R_Message::read(node);
//     if (lr == NULL) {
//         cerr << "error in reading cache loc_request message" << endl;
//     }
// #ifdef DEBUG
//     cerr << "successfully read cache loc r msg" << endl;
//     cerr << "proc name :" << lr->procedure_name<<endl;
// #endif
//     vector<struct R_Message*> a = db->find_all(lr);
//     int size = a.size();
// #ifdef DEBUG
//     cerr << "size of results :" << size << endl;
//     cerr << "node when sending :" <<node <<endl;
// #endif
//     int status = send(node, &size, sizeof(int), 0);
//     if (status < 0) {
//         cerr << "error in sending the size of the cache log request response" << endl;
//         return;
//     }
//     for (vector<struct R_Message *>::iterator it= a.begin(); it!=a.end(); ++it) {
// #ifdef DEBUG 
//         cerr << "server_identifier of a result: "<< (*it)->server_identifier <<endl;
//         cerr << "port of a result: "<< (*it)->port <<endl;
// #endif
//         char* server_identifier = new char[256];
//         strncpy(server_identifier, (*it)->server_identifier, 256);
//         ls = new LOC_S_Message(server_identifier, (*it)->port);
// #ifdef DEBUG
//         cerr << "server_identifier of loc s msg before sending: "<< ls->server_identifier <<endl;
//         cerr << "port of loc s msg before sending: "<< ls->port <<endl;
// #endif        
//         ls->sendmsg(node);
//     }
// }
