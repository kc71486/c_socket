#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "server.h"

int sockfd = 0;
struct UserList userlist;
char allmessage_arr[50][128];

int main(int argc , char *argv[]) {
    //initialize
    memset(&userlist, 0, sizeof(struct UserList));
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
    listen(sockfd,5);
    // add send all
    
    pthread_t send_all_thread;
    if(pthread_create(&(send_all_thread), NULL, (void *) send_all_handler, (void *) newuser) != 0) {
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
    }
    return 0;
}

void *user_handle(void *param) {
    struct UserNode *user = (struct UserNode *)param;
    char inputBuffer[128] = {};
    char message[128] = {};
    char nickname[20];
    int recvbyte = 0;
    
    recv(user->sockfd, inputBuffer, sizeof(inputBuffer), 0);
    strncpy(nickname, inputBuffer, sizeof(nickname));
    snprintf(message, sizeof(message), "[%s has joined]", nickname);
    send_all(&userlist, message, sizeof(message));
    
    while(1) {
        recvbyte = recv(user->sockfd, inputBuffer, sizeof(inputBuffer), 0);
        if(recvbyte == 0 || strcmp(inputBuffer, "?exit") == 0) {
            snprintf(message, sizeof(message), "[%s has leaved]", nickname);
            send_all(&userlist, message, sizeof(message));
            break;
        }
        snprintf(message, sizeof(message), "<%s> %s", nickname, inputBuffer);
        write_message(message);
    }
    close(user->sockfd);
    if(userlist.firstUser == user) {
        user->next->prev = NULL;
        userlist.firstUser = user->next;
    }
    else if(userlist.lastUser == user) {
        user->prev->next = NULL;
        userlist.lastUser = user->prev;
    }
    else {
        user->next->prev = user->prev;
        user->prev->next = user->next;
    }
    //free(user);
    
    return NULL;
}

void send_all(struct UserList *ulist, char *message, int messageSize) {
    struct UserNode *currentUser = ulist->firstUser;
    while(currentUser != NULL) {
        send(currentUser->sockfd, message, messageSize, 0);
        currentUser = currentUser->next;
    }
    printf("%s\n", message);
}

void write_message(char *message) {
    int idx;
    //write lock start
    while(allmessage_arr[idx][0] != 0) {
        idx += 1;
    }
    allmessage_arr[idx] = message;
    //write lock end
}

void send_all_handler() {
    struct UserList *ulist = userlist;
    struct UserNode *currentUser = ulist->firstUser;
    int idx;
    while(1) {
        //write lock start
        while(currentUser != NULL) {
            idx = 0;
            while(allmessage_arr[idx][0] != 0) {
                send(currentUser->sockfd, allmessage_arr[idx], sizeof(allmessage_arr[idx]), 0);
                idx += 1;
            }
            currentUser = currentUser->next;
        }
        memset(&allmessage_arr, 0, sizeof(allmessage_arr));
        //write lock end
        printf("%s\n", message);
    }
}

void add_user(struct UserList *ulist, struct sockaddr_in addr, int sockfd) {
    struct UserNode *newuser = (struct UserNode *) calloc(1, sizeof(struct UserNode));
    newuser->address = addr;
    newuser->sockfd = sockfd;
    if(ulist->lastUser == NULL) {
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
    if(pthread_create(&(userlist.lastUser->userThread), NULL, user_handle, (void *) newuser) != 0) {
        printf("thread creation error\n");
        exit(-1);
    }
}
