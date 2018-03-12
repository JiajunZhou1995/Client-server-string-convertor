#if !defined(HELPER_H)
#define HELPER_H 1

int readInt(int socketId, int* status);
char* readString(int socketId, int* status);
int* readIntArray(int socketId, int* status);
bool ifSameArgs(int *a, int len1, int *b, int len2);
struct R_Message* readRegisterMessage(int socketId);

int getArgsLength (int* argTypes);
void** recvArgs(int sockfd, int len, int* argTypes);
int sendArgs(int sockfd, int len, int* argTypes, void** args);

#endif
