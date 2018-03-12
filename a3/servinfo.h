#ifndef _SERVER_INFO_
#define _SERVER_INFO_

#include <string>

using namespace std;

struct servinfo {
    int port;
    string sid;

    servinfo(): port(0), sid(""){}
    servinfo(int p, string server_id): port(p), sid(server_id){}
    ~servinfo(){}
};

#endif
