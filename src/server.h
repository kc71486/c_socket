#ifndef SOCKET_SERVER
#define SOCKET_SERVER

#ifndef SOCKET_SIZE
#define SOCKET_SIZE 128
#endif

#define BUFFER_SIZE 5
#define QUEUE_SIZE 20

#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <semaphore.h>

struct UserNode;

struct ObjectSync {
    sem_t writer;
    sem_t reader;
    int count;
};
struct UserList { //double linked list
    struct UserNode *firstUser;
    struct UserNode *lastUser;
    int length;
};
struct UserNode {
    struct UserNode *next;
    struct UserNode *prev;
    int sockfd;
    struct sockaddr_in address;
    pthread_t userThread;
    int connected;
    char message_buffer_arr[BUFFER_SIZE][SOCKET_SIZE];
    struct ObjectSync message_buffer_lock;
};

void write_message(char *message);
void *send_all_handler(void *param);
void *user_handler(void *param);
void *doublebuffer_handler(void *param);

void write_message(struct UserNode *user, char *message);
void closesocket(struct UserNode *user);

void objectsync_init(struct ObjectSync *target);
void reader_start(struct ObjectSync *target);
void reader_end(struct ObjectSync *target);
void writer_start(struct ObjectSync *target);
void writer_end(struct ObjectSync *target);

void add_user(struct UserList *userlist, struct sockaddr_in address, int sockfd);
void remove_user(struct UserList *userlist, struct UserNode *user);
#endif
