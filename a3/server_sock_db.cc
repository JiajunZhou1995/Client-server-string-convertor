#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <map>
#include <iostream>
#include "server_sock_db.h"
#include "constants.h"
using namespace std;

server_sock_db::server_sock_db() {}

server_sock_db::~server_sock_db() {
	for (map<int, char*>::iterator it = servers.begin(); it != servers.end(); ++it) {
		// delete it->second;
	}
}

void server_sock_db::add_server_socket_id(int sock, char* id) {
	char* server_identifier = new char[256];
	if (servers.find(sock) == servers.end()) {
		// already exist, free the old name
		// delete servers[sock];
	} 
	strncpy(server_identifier, id, 256);
	servers[sock] = server_identifier;
}

void server_sock_db::remove_server_socket_id(int sock) {
	if (servers.find(sock) != servers.end()) {
		// delete servers[sock];
		servers.erase(sock);
	}
}

void server_sock_db::broadcast_terminate_message() {
	for (map<int, char*>::iterator it = servers.begin(); it != servers.end(); ++it) {
		send(it->first, &TERMINATE, sizeof(int), 0);
		//delete it->second;
	}
}

// check if the connection sock is from a server instead of a client
bool server_sock_db::if_socket_from_server(int sock) {
	return servers.find(sock) != servers.end();
}

// pass a sock, return a char * that is the server name
// null is such a server does not exist
char * server_sock_db::server_name_from_sock(int sock) {
	if (servers.find(sock) != servers.end()) {
#ifdef DEBUG
		cerr << "server name found\n";
#endif
		return servers[sock];
	}
	return NULL;
}
