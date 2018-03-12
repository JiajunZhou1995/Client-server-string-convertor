#if !defined(MESSAGE_HANDLER_H)
#define MESSAGE_HANDLER_H 1

#include "binder_db.h"
#include "server_sock_db.h"
#include "message.h"

void handle_register_message(struct binder_db* db, struct server_sock_db* s_db, int node);
void handle_loc_request_message(struct binder_db* db, int node);
void handle_node_disconnect(int sock, struct binder_db* db, struct server_sock_db* s_db);
void handle_cache_loc_request_message(struct binder_db * db, int node);

#endif