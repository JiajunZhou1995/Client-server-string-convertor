#include <string.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "message.h"
#include "constants.h"
#include "helper.h"
#include "error.h"
#include "helper.h"
using namespace std;

struct R_Message* R_Message::read(int sockfd) {
    struct R_Message* res = new R_Message();

    res->server_identifier = new char[256];
    int retval = recv(sockfd, res->server_identifier, 256, 0);
    if (retval < 0) {
        cerr << "failed to recv server identifier when reading register message\n";
        //delete res;
        return NULL;
    }

    retval = recv(sockfd, &(res->port), sizeof(res->port), 0);
    if (retval < 0) {
        cerr << "failed to recv server port when reading register message\n";
        //delete res;
        return NULL;
    }

    int len;
    retval = recv(sockfd, &len, sizeof(len), 0);
    if (retval < 0) {
        cerr << "failed to recv length of proc name when reading register message\n";
        //delete res;
        return NULL;
    }

    res->procedure_name = new char[len];
    retval = recv(sockfd, res->procedure_name, len, 0);
    if (retval < 0) {
        cerr << "failed to recv proc name when reading register message\n";
        //delete res;
        return NULL;
    }

#ifdef DEBUG
    cerr << "recv procedure name: " << res->procedure_name << endl;
#endif

    retval = recv(sockfd, &len, sizeof(len), 0);
    if (retval < 0) {
        cerr << "failed to recv args len when reading register message\n";
        //delete res;
        return NULL;
    }

    res->length = len;
    res->argTypes = new int[len];
    retval = recv(sockfd, res->argTypes, len * sizeof(int), 0);
    if (retval < 0) {
        cerr << "failed to recv argTypes when reading register message\n";
        //delete res;
        return NULL;
    }

    return res;
}

int R_Message::sendmsg(int sockfd) {
    int msg_type = REGISTER;

    int retval = send(sockfd, &msg_type, sizeof(msg_type), 0);
    if (retval < 0) {
        cerr << "failed to send msg type when sending register message\n";
        return SEND_FAILURE;
    }

    retval = send(sockfd, server_identifier, 256, 0);
    if (retval < 0) {
        cerr << "failed to send server id when sending register message\n";
        return SEND_FAILURE;
    }

    retval = send(sockfd, &port, sizeof(port), 0);
    if (retval < 0) {
        cerr << "failed to send port when sending register message\n";
        return SEND_FAILURE;
    }

    int len = strlen(procedure_name) + 1;
    retval = send(sockfd, &len, sizeof(len), 0);
    if (retval < 0) {
        cerr << "failed to send len of proc name when sending register message\n";
        return SEND_FAILURE;
    }


    retval = send(sockfd, procedure_name, len, 0);
    if (retval < 0) {
        cerr << "failed to send proc name when sending register message\n";
        return SEND_FAILURE;
    }

#ifdef DEBUG
    cerr << "sending procedure name: " << procedure_name << endl;
#endif

    retval = send(sockfd, &length, sizeof(length), 0);
    if (retval < 0) {
        cerr << "failed to send args length when sending register message\n";
        return SEND_FAILURE;
    }

    length = length*(sizeof(int));
    retval = send(sockfd, argTypes, length, 0);
    if (retval < 0) {
        cerr << "failed to send arg types when sending register message\n";
        return SEND_FAILURE;
    }

    return 0;
}

struct C_LOC_R_Message* C_LOC_R_Message::read(int sockfd) {
    struct C_LOC_R_Message* res = new C_LOC_R_Message();
    int len, retval;

    retval = recv(sockfd, &len, sizeof(len), 0);
    if (retval < 0) {
        cerr << "failed to recv length of proc name when reading cache loc request message\n";
        //delete res;
        return NULL;
    }

    res->procedure_name = new char[len];
    retval = recv(sockfd, res->procedure_name, len, 0);
    if (retval < 0) {
        cerr << "failed to recv proc namewhen reading cache loc request message\n";
        //delete res;
        return NULL;
    }

    retval = recv(sockfd, &len, sizeof(len), 0);
    if (retval < 0) {
        cerr << "failed to recv length of arg types when reading cache loc request message\n";
        //delete res;
        return NULL;
    }

    res->length = len;
    res->argTypes = new int[len];
    retval = recv(sockfd, res->argTypes, len * sizeof(int), 0);
    if (retval < 0) {
        cerr << "failed to recv arg types when reading cache loc request message\n";
        //delete res;
        return NULL;
    }

    return res;
}

int C_LOC_R_Message::sendmsg(int sockfd) {
    int msg_type = CACHE_LOC_REQUEST;

    int retval = send(sockfd, &msg_type, sizeof(msg_type), 0);
    if (retval < 0) {
        cerr << "failed to send msg type when sending cache loc request message\n";
        return SEND_FAILURE;
    }

    int len = strlen(procedure_name) + 1;
    retval = send(sockfd, &len, sizeof(len), 0);
    if (retval < 0) {
        cerr << "failed to send len of proc name when sending cache loc request message\n";
        return SEND_FAILURE;
    }

    retval = send(sockfd, procedure_name, len, 0);
    if (retval < 0) {
        cerr << "failed to send proc name when sending cache loc request message\n";
        return SEND_FAILURE;
    }

    retval = send(sockfd, &length, sizeof(length), 0);
    if (retval < 0) {
        cerr << "failed to send args length when sending cache loc request message\n";
        return SEND_FAILURE;
    }

    length = length*(sizeof(int));
    retval = send(sockfd, argTypes, length, 0);
    if (retval < 0) {
        cerr << "failed to send arg types when sending cache loc request message\n";
        return SEND_FAILURE;
    }

    return 0;
}

struct LOC_R_Message* LOC_R_Message::read(int sockfd) {
    struct LOC_R_Message* res = new LOC_R_Message();
    int len, retval;

    retval = recv(sockfd, &len, sizeof(len), 0);
    if (retval < 0) {
        cerr << "failed to recv length of proc name when reading loc request message\n";
        //delete res;
        return NULL;
    }

    res->procedure_name = new char[len];
    retval = recv(sockfd, res->procedure_name, len, 0);
    if (retval < 0) {
        cerr << "failed to recv proc namewhen reading loc request message\n";
        //delete res;
        return NULL;
    }

    retval = recv(sockfd, &len, sizeof(len), 0);
    if (retval < 0) {
        cerr << "failed to recv length of arg types when reading loc request message\n";
        //delete res;
        return NULL;
    }

    res->length = len;
    res->argTypes = new int[len];
    retval = recv(sockfd, res->argTypes, len * sizeof(int), 0);
    if (retval < 0) {
        cerr << "failed to recv arg types when reading loc request message\n";
        //delete res;
        return NULL;
    }

    return res;
}

int LOC_R_Message::sendmsg(int sockfd) {
    int msg_type = LOC_REQUEST;

    int retval = send(sockfd, &msg_type, sizeof(msg_type), 0);
    if (retval < 0) {
        cerr << "failed to send msg type when sending loc request message\n";
        return SEND_FAILURE;
    }

    int len = strlen(procedure_name) + 1;
    retval = send(sockfd, &len, sizeof(len), 0);
    if (retval < 0) {
        cerr << "failed to send len of proc name when sending loc request message\n";
        return SEND_FAILURE;
    }

    retval = send(sockfd, procedure_name, len, 0);
    if (retval < 0) {
        cerr << "failed to send proc name when sending loc request message\n";
        return SEND_FAILURE;
    }

    retval = send(sockfd, &length, sizeof(length), 0);
    if (retval < 0) {
        cerr << "failed to send args length when sending loc request message\n";
        return SEND_FAILURE;
    }

    length = length*(sizeof(int));
    retval = send(sockfd, argTypes, length, 0);
    if (retval < 0) {
        cerr << "failed to send arg types when sending loc request message\n";
        return SEND_FAILURE;
    }

    return 0;
}

struct R_S_Message* R_S_Message::read(int sockfd) {
    struct R_S_Message* res = new R_S_Message();

    int retval = recv(sockfd, &res->warning_code, sizeof(res->warning_code), 0);
    if (retval < 0) {
        cerr << "failed to recv warning_code when reading register success message\n";
        //delete res;
        return NULL;
    }

    return res;
}

int R_S_Message::sendmsg(int sockfd) {
    int msg_type = REGISTER_SUCCESS;
#ifdef DEBUG
    cerr << "msg_type: " << msg_type << endl;
#endif
    int retval = send(sockfd, &msg_type, sizeof(msg_type), 0);
    if (retval < 0) {
        cerr << "failed to send msg type when sending register success message\n";
        return SEND_FAILURE;
    }

    retval = send(sockfd, &warning_code, sizeof(warning_code), 0);
#ifdef DEBUG
    cerr << "warning_code: " << warning_code << endl;
#endif
    if (retval < 0) {
        cerr << "failed to send warning_code when sending register success message\n";
        return SEND_FAILURE;
    }

    return 0;
}

struct R_F_Message* R_F_Message::read(int sockfd) {
    struct R_F_Message* res = new R_F_Message();

    int retval = recv(sockfd, &res->warning_code, sizeof(res->warning_code), 0);
    if (retval < 0) {
        cerr << "failed to recv warning_code when reading register failure message\n";
        //delete res;
        return NULL;
    }

    return res;
}

int R_F_Message::sendmsg(int sockfd) {
    int msg_type = REGISTER_FAILURE;

    int retval = send(sockfd, &msg_type, sizeof(msg_type), 0);
    if (retval < 0) {
        cerr << "failed to send msg type when sending register failure message\n";
        return SEND_FAILURE;
    }

    retval = send(sockfd, &warning_code, sizeof(warning_code), 0);
    if (retval < 0) {
        cerr << "failed to send warning_code when sending register failure message\n";
        return SEND_FAILURE;
    }

    return 0;
}


struct LOC_S_Message* LOC_S_Message::read(int sockfd) {
    struct LOC_S_Message* res = new LOC_S_Message();
    int len, retval, port;

    res->server_identifier = new char[256];
    retval = recv(sockfd, res->server_identifier, 256, 0);
    if (retval < 0) {
        cerr << "failed to recv server id when reading loc success message\n";
        //delete res;
        return NULL;
    }

    retval = recv(sockfd, &res->port, sizeof(res->port), 0);
    if (retval < 0) {
        cerr << "failed to recv port when reading loc success message\n";
        //delete res;
        return NULL;
    }

    return res;
}

int LOC_S_Message::sendmsg(int sockfd) {
    int msg_type = LOC_SUCCESS;

    int retval = send(sockfd, &msg_type, sizeof(msg_type), 0);
    if (retval < 0) {
        cerr << "failed to send msg type when sending loc success message\n";
        return SEND_FAILURE;
    }

#ifdef DEBUG
    cerr << "loc s msg send: sid " << server_identifier <<endl;
    cerr << "loc s msg send: port " << port <<endl;
#endif
    retval = send(sockfd, server_identifier, 256, 0);
    if (retval < 0) {
        cerr << "failed to send server id when sending loc success message\n";
        return SEND_FAILURE;
    }

    retval = send(sockfd, &port, sizeof(port), 0);
    if (retval < 0) {
        cerr << "failed to send port when sending loc success message\n";
        return SEND_FAILURE;
    }

    return 0;
}

struct LOC_F_Message* LOC_F_Message::read(int sockfd) {
    struct LOC_F_Message* res = new LOC_F_Message();

    int retval = recv(sockfd, &res->reason_code, sizeof(res->reason_code), 0);
    if (retval < 0) {
        cerr << "failed to recv reason_code when reading loc success message\n";
        //delete res;
        return NULL;
    }

    return res;
}

int LOC_F_Message::sendmsg(int sockfd) {
    int msg_type = LOC_FAILURE;

    int retval = send(sockfd, &msg_type, sizeof(msg_type), 0);
    if (retval < 0) {
        cerr << "failed to send msg type when sending loc failure message\n";
        return SEND_FAILURE;
    }

    retval = send(sockfd, &reason_code, sizeof(reason_code), 0);
    if (retval < 0) {
        cerr << "failed to send reason_code when sending loc failure message\n";
        return SEND_FAILURE;
    }

    return 0;
}

struct E_Message* E_Message::read(int sockfd) {
    struct E_Message* res = new E_Message();

    int retval, len;
    retval = recv(sockfd, &len, sizeof(len), 0);
    if (retval < 0) {
        cerr << "failed to recv length of proc name when reading execute message\n";
        //delete res;
        return NULL;
    }

    res->name = new char[len];
    retval = recv(sockfd, res->name, len, 0);
    if (retval < 0) {
        cerr << "failed to recv proc name when reading execute message\n";
        //delete res;
        return NULL;
    }

    retval = recv(sockfd, &len, sizeof(len), 0);
    if (retval < 0) {
        cerr << "failed to recv length of arg types when reading execute message\n";
        //delete res;
        return NULL;
    }

    res->length = len;
    res->argTypes = new int[len];
    retval = recv(sockfd, res->argTypes, len * sizeof(int), 0);
    if (retval < 0) {
        cerr << "failed to recv arg types when reading execute message\n";
        //delete res;
        return NULL;
    }

    res->args = recvArgs(sockfd, res->length, res->argTypes);
    if (res->args == NULL) {
        cerr << "failed to recv args when reading execute message\n";
        //delete res;
        return NULL;
    }

    return res;
}

int E_Message::sendmsg(int sockfd) {
    int msg_type = EXECUTE;

    int retval = send(sockfd, &msg_type, sizeof(msg_type), 0);
    if (retval < 0) {
        cerr << "failed to send msg type when sending execute message\n";
        return SEND_FAILURE;
    }

    int len = strlen(name) + 1;
    retval = send(sockfd, &len, sizeof(len), 0);
    if (retval < 0) {
        cerr << "failed to send len of proc name when sending execute message\n";
        return SEND_FAILURE;
    }

    retval = send(sockfd, name, len, 0);
    if (retval < 0) {
        cerr << "failed to send proc name when sending execute message\n";
        return SEND_FAILURE;
    }

    retval = send(sockfd, &length, sizeof(length), 0);
    if (retval < 0) {
        cerr << "failed to send args length when sending execute message\n";
        return SEND_FAILURE;
    }

    int argsSize = length*(sizeof(int));
    retval = send(sockfd, argTypes, argsSize, 0);
    if (retval < 0) {
        cerr << "failed to send arg types when sending execute message\n";
        return SEND_FAILURE;
    }
    
    retval = sendArgs(sockfd, length, argTypes, args);
    if (retval < 0) {
        cerr << "failed to send args when reading execute message\n";
        return SEND_FAILURE;
    }
    return 0;
}

struct E_S_Message* E_S_Message::read(int sockfd) {
    struct E_S_Message* res = new E_S_Message();

    int retval, len;
    retval = recv(sockfd, &len, sizeof(len), 0);
    if (retval < 0) {
        cerr << "failed to recv length of proc name when reading execute success message\n";
        //delete res;
        return NULL;
    }

    res->name = new char[len];
    retval = recv(sockfd, res->name, len, 0);
    if (retval < 0) {
        cerr << "failed to recv proc name when reading execute success message\n";
        //delete res;
        return NULL;
    }

    retval = recv(sockfd, &len, sizeof(len), 0);
    if (retval < 0) {
        cerr << "failed to recv length of arg types when reading execute success message\n";
        //delete res;
        return NULL;
    }

    res->length = len;
    res->argTypes = new int[len];
    retval = recv(sockfd, res->argTypes, len * sizeof(int), 0);
    if (retval < 0) {
        cerr << "failed to recv arg types when reading execute success message\n";
        //delete res;
        return NULL;
    }

    res->args = recvArgs(sockfd, len, res->argTypes);
    if (res->args == NULL) {
        cerr << "failed to recv args when reading execute success message\n";
        //delete res;
        return NULL;
    }

    return res;
}

int E_S_Message::sendmsg(int sockfd) {
    int msg_type = EXECUTE_SUCCESS;

    int retval = send(sockfd, &msg_type, sizeof(msg_type), 0);
    if (retval < 0) {
        cerr << "failed to send msg type when sending execute success message\n";
        return SEND_FAILURE;
    }

    int len = strlen(name) + 1;
    retval = send(sockfd, &len, sizeof(len), 0);
    if (retval < 0) {
        cerr << "failed to send len of proc name when sending execute success message\n";
        return SEND_FAILURE;
    }

    retval = send(sockfd, name, len, 0);
    if (retval < 0) {
        cerr << "failed to send proc name when sending execute success message\n";
        return SEND_FAILURE;
    }

    retval = send(sockfd, &length, sizeof(length), 0);
    if (retval < 0) {
        cerr << "failed to send args length when sending execute success message\n";
        return SEND_FAILURE;
    }

    int argsSize = length*(sizeof(int));
    retval = send(sockfd, argTypes, argsSize, 0);
    if (retval < 0) {
        cerr << "failed to send arg types when sending execute success message\n";
        return SEND_FAILURE;
    }

    retval = sendArgs(sockfd, length, argTypes, args);
    if (retval < 0) {
        cerr << "failed to recv args when reading execute message\n";
        return SEND_FAILURE;
    }

    return 0;
}

struct E_F_Message* E_F_Message::read(int sockfd) {
    struct E_F_Message* res = new E_F_Message();

    int retval = recv(sockfd, &res->reason_code, sizeof(res->reason_code), 0);
    if (retval < 0) {
        cerr << "failed to recv reason_code when reading loc success message\n";
        //delete res;
        return NULL;
    }

    return res;
}

int E_F_Message::sendmsg(int sockfd) {
    int msg_type = EXECUTE_FAILURE;

    int retval = send(sockfd, &msg_type, sizeof(msg_type), 0);
    if (retval < 0) {
        cerr << "failed to send msg type when sending execution failure message\n";
        return SEND_FAILURE;
    }

    retval = send(sockfd, &reason_code, sizeof(reason_code), 0);
    if (retval < 0) {
        cerr << "failed to send reason_code when sending execution failure message\n";
        return SEND_FAILURE;
    }

    return 0;
}


