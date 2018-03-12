// constructing vectors
#include <iostream>
#include <vector>
#include "message.h"
#include "helper.h"
#include "binder_db.h"
#include "error.h"

using namespace std;

binder_db::binder_db() {}

binder_db::~binder_db() {
    for (vector<struct R_Message *>:: iterator it=content.begin(); it!= content.end(); ++it) {
        //delete *it;
    }
}

void binder_db::add_r_message(struct R_Message* r, int* status){
    // we have to check if the r message is already in the content
    // if so, remove the last one and add latestest one.
    struct R_Message* cur;
    int size = content.size();
    bool added = false;

#ifdef DEBUG
    cerr << "the procedure name is " << r->procedure_name << endl;
    cerr << "the server name " << r->server_identifier << endl;
#endif

    for (int i=0; i < size; i++) {
        cur = content[i];
      
        if (strcmp(cur->procedure_name, r->procedure_name) == 0 && ifSameArgs(r->argTypes, r->length, cur->argTypes, cur->length) && strcmp(r->server_identifier, cur->server_identifier) == 0) {
#ifdef DEBUG
    cerr << "the procedure name of cur is " << cur->procedure_name << endl;
    cerr << "the server name of cur is " << cur->server_identifier << endl;
#endif 
            // delete cur;
            content[i] = r;
            added = true;
            *status = FUNCTION_ALREADY_EXIST;
            return;
        }
    }
    if(!added) {
        content.push_back(r);
        *status = REGISTER_SUCCESS; 
    }

#ifdef DEBUG
    cerr << *status << endl;
#endif

};

// find the function signiture that matches the function message that is requested
struct R_Message* binder_db::find_r_message(struct LOC_R_Message* r){
    struct R_Message* cur;
    int size = content.size();
    for(int i=0; i < size; i++) {
        cur = content[i];

#ifdef DEBUG
        cerr << cur->procedure_name <<endl;
        cerr << r->procedure_name << endl;
        cerr << strcmp(cur->procedure_name, r->procedure_name) << endl;
        cerr << ifSameArgs(r->argTypes, r->length, cur->argTypes, cur->length) << endl;
#endif

        if (strcmp(cur->procedure_name, r->procedure_name) == 0 && ifSameArgs(r->argTypes, r->length, cur->argTypes, cur->length)) {
            content.erase(content.begin() + i);
            content.push_back(cur);             // this way we can ensure the round robin
            return cur;
        }
    }
    return NULL;    // indicate that we did not find a match
};

void binder_db::remove_message_from_particular_server(char* server_id) {
    for (vector<struct R_Message*> ::iterator it=content.begin(); it != content.end();) {
        if (strcmp((*it)->server_identifier, server_id) == 0)
            it = content.erase(it);
        else
            ++it;
    }
}

vector<struct R_Message *> binder_db::find_all(struct C_LOC_R_Message* r) {
    vector<struct R_Message *> a;
    for (vector<struct R_Message*> ::iterator it=content.begin(); it != content.end(); ++it) {
        if (strcmp((*it)->procedure_name, r->procedure_name) == 0 && ifSameArgs(r->argTypes, r->length, (*it)->argTypes, (*it)->length)) {
            a.push_back(*it);
        } 
    }
    return a;
}
