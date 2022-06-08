#ifndef SOCKET_SERVER
#define SOCKET_SERVER

#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct UserNode;
struct UserList {
    struct UserNode *firstUser;
    struct UserNode *lastUser;
    int length;
};
struct UserNode {
    struct UserNode *next;
    struct UserNode *prev;
    pthread_t userThread;
    struct sockaddr_in address;
    int sockfd;
};
void write_message(char *message);
void send_all_handler();
void *user_handle(void *);
void add_user(struct UserList *, struct sockaddr_in, int);
void send_all();
#endif
