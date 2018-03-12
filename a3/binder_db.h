#if !defined(BINDER_DB_H)
#define BINDER_DB_H 1

#include <vector>
#include <cstring>
using namespace std;

struct binder_db {
    binder_db();
    ~binder_db();
    void add_r_message(struct R_Message* r, int* status);
    struct R_Message* find_r_message(struct LOC_R_Message* r);
    void remove_message_from_particular_server(char *s);
    vector<struct R_Message*> find_all(struct C_LOC_R_Message* r);
    
    vector<struct R_Message *> content;
};

#endif
