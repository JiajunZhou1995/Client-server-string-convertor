#if !defined(SERVER_SOCK_DB_H)
#define SERVER_SOCK_DB_H 1
#include <vector>
#include <map>
#include <cstring>

using namespace std;

// remember those servers and which servers are still alive
struct server_sock_db {
    server_sock_db();
    ~server_sock_db();
    void add_server_socket_id(int sock, char* server_id);
    void remove_server_socket_id(int sock);
    void broadcast_terminate_message();
    bool if_socket_from_server(int sock);
    char* server_name_from_sock(int sock);

    map<int, char*> servers;
};

#endif