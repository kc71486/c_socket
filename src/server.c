#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "server.h"

int sockfd = 0;
UserList userlist;
char message_send_arr[QUEUE_SIZE][SOCKET_SIZE];
pthread_t send_all_thread, doublebuffer_thread;
ObjectSync message_send_lock, userlist_lock;

int main(int argc , char *argv[]) {
    //initialize
    memset(&userlist, 0, sizeof(struct UserList));
    objectsync_init(&message_send_lock);
    objectsync_init(&userlist_lock);
    
    //create socket
    sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if (sockfd == -1){
        printf("Fail to create a socket.\n");
    }
    
    //socket setting
    struct sockaddr_in server_addr, client_addr;
    int s_addrlen = sizeof(server_addr);
    int c_addrlen = sizeof(client_addr); //for accept() and getpeername()
    memset(&server_addr, 0, s_addrlen);
    server_addr.sin_family = PF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8700);
    bind(sockfd, (struct sockaddr *)&server_addr, s_addrlen);
    listen(sockfd, 20);
    
    // add send_all thread
    if(pthread_create(&(send_all_thread), NULL, (void *) send_all_handler, NULL) != 0) {
        printf("thread creation error\n");
        exit(-1);
    }
    
    // add doublebuffer thread
    if(pthread_create(&(doublebuffer_thread), NULL, doublebuffer_handler, NULL) != 0) {
        printf("thread creation error\n");
        exit(-1);
    }
    
    // add clients
    while(1){
        int clientSockfd = 0;
        clientSockfd = accept(sockfd, (struct sockaddr*) &client_addr, &c_addrlen);
        getpeername(clientSockfd, (struct sockaddr*) &client_addr, &c_addrlen);
        printf("connect to client %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        add_user(&userlist, client_addr, clientSockfd);
        sched_yield();
    }
    return 0;
}

/*handler*/

void *user_handler(void *param) {
    UserNode *user = (struct UserNode *)param;
    char inputBuffer[SOCKET_SIZE] = {};
    char message[SOCKET_SIZE] = {};
    char nickname[20];
    int recvbyte = 0;
    
    recv(user->sockfd, inputBuffer, sizeof(inputBuffer), 0);
    strncpy(nickname, inputBuffer, sizeof(nickname));
    snprintf(message, sizeof(message), "[%s has joined]", nickname);
    write_message(user, message);
    while(1) {
        recvbyte = recv(user->sockfd, inputBuffer, SOCKET_SIZE, 0);
        if(recvbyte == 0 || strcmp(inputBuffer, "?exit") == 0) {
            snprintf(message, SOCKET_SIZE, "[%s has leaved]", nickname);
            write_message(user, message);
            break;
        }
        snprintf(message, SOCKET_SIZE, "<%s> %s", nickname, inputBuffer);
        write_message(user, message);
        sched_yield();
    }
    closesocket(user);
    
    return NULL;
}

void *send_all_handler(void *none) {
    UserNode *currentUser;
    int idx;
    while(1) {
        writer_start(&message_send_lock);
        {
            for(idx = 0; idx < QUEUE_SIZE && message_send_arr[idx][0] != 0; idx ++) { //expect this to be very slow
                reader_start(&userlist_lock);
                {
                    currentUser = userlist.firstUser;
                    while(currentUser != NULL) {
                        if(currentUser->connected) {
                            send(currentUser->sockfd, message_send_arr[idx], sizeof(message_send_arr[idx]), 0);
                            currentUser = currentUser->next;
                        }
                        else {
                            reader_end(&userlist_lock);
                            remove_user(&userlist, currentUser);
                            UserNode *temp = currentUser->next;
                            free(currentUser);
                            currentUser = temp;
                            reader_start(&userlist_lock);
                        }
                    }
                }
                reader_end(&userlist_lock);
                printf("%s\n", message_send_arr[idx]);
            }
            memset(&message_send_arr, 0, sizeof(message_send_arr));
        }
        writer_end(&message_send_lock);
        usleep(100);//max update rate 10000/s
    }
    return NULL;
}

/**
 * buffer between send and recieve
 * requires both send and recieve lock to be released
 * prevents send from stalling recieve
 */
void *doublebuffer_handler(void *none) {
    UserNode *currentUser;
    int idx;
    while(1) {
        writer_start(&message_send_lock);
        {
            for(idx = 0; idx < QUEUE_SIZE && message_send_arr[idx][0] != 0; idx ++);
            if(idx < QUEUE_SIZE) {
                reader_start(&userlist_lock);
                {
                    currentUser = userlist.firstUser;
                    while(currentUser != NULL) {
                        writer_start(&(currentUser->message_buffer_lock));
                        {
                            while(currentUser->message_buffer_arr[0][0] != 0) {
                                strncpy(message_send_arr[idx], currentUser->message_buffer_arr[0], SOCKET_SIZE);
                                memmove(currentUser->message_buffer_arr[0], currentUser->message_buffer_arr[1], (BUFFER_SIZE-1)*SOCKET_SIZE);
                                memset(currentUser->message_buffer_arr[BUFFER_SIZE-1], 0, SOCKET_SIZE);
                                idx ++;
                                if(idx >= QUEUE_SIZE) {
                                    writer_end(&(currentUser->message_buffer_lock));
                                    goto outer;
                                }
                            }
                        }
                        writer_end(&(currentUser->message_buffer_lock));
                    }
                    outer:;
                }
                reader_end(&userlist_lock);
            }
        }
        writer_end(&message_send_lock);
        sched_yield();
    }
    return NULL;
}

/*utility function*/

void write_message(UserNode *user, char *message) {
    int idx;
    writer_start(&(user->message_buffer_lock));
    {
        for(idx = 0; idx < BUFFER_SIZE && user->message_buffer_arr[idx][0] != 0; idx++);
        if(idx < BUFFER_SIZE) {
            strncpy(user->message_buffer_arr[idx], message, SOCKET_SIZE);
        }
        else {
            const char *err_msg = "[buffer exceeded, please wait and send again]";
            printf("user buffer exceeded (address=%s:%d)\n", inet_ntoa(user->address.sin_addr), ntohs(user->address.sin_port));
            send(user->sockfd, err_msg, strlen(err_msg), 0);
        }
    }
    writer_end(&(user->message_buffer_lock));
}

void closesocket(UserNode *user) {
    writer_start(&userlist_lock);
    {
        close(user->sockfd);
        user->connected = 0;
    }
    writer_end(&userlist_lock);
}

/*ObjectSync operation*/

void objectsync_init(ObjectSync *target) {
    sem_init(&(target->reader), 0, 1);
    sem_init(&(target->writer), 0, 1);
    target->count = 0;
}

void reader_start(ObjectSync *target) {
    sem_wait(&(target->reader));
    target->count += 1;
    if (target->count == 1)
        sem_wait(&(target->writer));
    sem_post(&(target->reader));
}

void reader_end(ObjectSync *target) {
    sem_wait(&(target->reader));
    target->count -= 1;
    if (target->count == 0)
        sem_post(&(target->writer));
    sem_post(&(target->reader));
}

void writer_start(ObjectSync *target) {
    sem_wait(&(target->writer));
}

void writer_end(ObjectSync *target) {
    sem_post(&(target->writer));
}

/*list operation*/

void add_user(UserList *ulist, struct sockaddr_in addr, int sockfd) {
    UserNode *newuser = (UserNode *) calloc(1, sizeof(UserNode));
    newuser->address = addr;
    newuser->sockfd = sockfd;
    newuser->connected = 1;
    objectsync_init(&(newuser->message_buffer_lock));
    writer_start(&userlist_lock);
    {
        if(ulist->lastUser == NULL) { //only 1 user
            ulist->firstUser = newuser;
            ulist->lastUser = newuser;
            ulist->length = 1;
        }
        else {
            newuser->prev = ulist->lastUser;
            ulist->lastUser->next = newuser;
            ulist->lastUser = newuser;
            ulist->length += 1;
        }
    }
    writer_end(&userlist_lock);
    if(pthread_create(&(newuser->userThread), NULL, user_handler, (void *) newuser) != 0) {
        printf("thread creation error\n");
        exit(-1);
    }
}

void remove_user(UserList *ulist, UserNode *user) {
    writer_start(&userlist_lock);
    {
        if(ulist->firstUser == user) {
            if(user->next == NULL) { //only 1 user
                ulist->firstUser = NULL;
            }
            else { //first user
                user->next->prev = NULL;
                ulist->firstUser = user->next;
            }
        }
        else if(ulist->lastUser == user) { //last user
            user->prev->next = NULL;
            ulist->lastUser = user->prev;
        }
        else {
            user->next->prev = user->prev;
            user->prev->next = user->next;
        }
        ulist->length -= 1;
    }
    writer_end(&userlist_lock);
}
