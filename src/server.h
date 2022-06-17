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
struct ObjectSync;

typedef struct ObjectSync {
    sem_t writer;
    sem_t reader;
    int count;
} ObjectSync;
/*double linked list*/
typedef struct UserList {
    struct UserNode *firstUser;
    struct UserNode *lastUser;
    int length;
} UserList;
typedef struct UserNode {
    struct UserNode *next;
    struct UserNode *prev;
    int sockfd;
    struct sockaddr_in address;
    pthread_t userThread;
    int connected;
    char message_buffer_arr[BUFFER_SIZE][SOCKET_SIZE];
    ObjectSync message_buffer_lock;
} UserNode;

/*thread handler*/
void *user_handler(void *param);
void *send_all_handler(void *none);
void *doublebuffer_handler(void *none);

/*utility*/
void write_message(UserNode *user, char *message);
void closesocket(UserNode *user);

/*ObjectSync method*/
void objectsync_init(ObjectSync *target);
void reader_start(ObjectSync *target);
void reader_end(ObjectSync *target);
void writer_start(ObjectSync *target);
void writer_end(ObjectSync *target);

/*UserList method*/
void add_user(UserList *userlist, struct sockaddr_in address, int sockfd);
void remove_user(UserList *userlist, UserNode *user);
#endif
