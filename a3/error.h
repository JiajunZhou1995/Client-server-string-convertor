#if !defined(ERROR_H)
#define ERROR_H 1

// error types
const int TYPE_ERROR = -1;
const int INVALID_LENGTH_ERROR = -2;
const int PROCEDURE_NOT_FOUND = -3;
const int NETWORK_ERROR = -4;
const int SEND_FAILURE = -5;

// const represnt register success or fail
const int REGISTER_SUCCESS = 0;
const int FUNCTION_ALREADY_EXIST = 1;
const int REGISTER_FAILURE = -17;

const int INVALID_BINDER_ADDRESS = -6;
const int INVALID_BINDER_PORT = -7;
const int CREATE_SOCKET_FAILURE = -8;
const int UNKNOWN_HOST = -9;
const int CONNECTION_FAILURE = -10;
const int ADDRINFO_FAILURE = -11;
const int BIND_FAILURE = -12;
const int LISTEN_FAILURE = -13;
const int MSG_NOT_RECEIVED = -14;
const int UNEXPECTED_MSG_TYPE = -15;
const int NO_REGISTERED_PROCEDURES = -16;
const int SEND_EXECUTE_MSG_TO_SERVERS_FAILURE = -18;
#endif
