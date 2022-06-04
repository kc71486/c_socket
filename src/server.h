#ifndef SOCKET_SERVER
#define SOCKET_SERVER

#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct UserNode;
struct UserList {
    struct *UserNode firstUser;
    struct *UserNode lastUser;
    int length;
};
struct UserNode {
    struct *UserNode next;
    pthread_t userThread;
    struct sockaddr_in address;
    int sockfd;
};
void user_handle();
void add_user(struct UserList *, struct sockaddr_in, int);
#endif