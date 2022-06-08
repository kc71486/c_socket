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
        }
        snprintf(message, sizeof(message), "<%s> %s", nickname, inputBuffer);
        send_all(&userlist, message, sizeof(message));
    }
    close(user->sockfd);
    
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

void add_user(struct UserList *ulist, struct sockaddr_in addr, int sockfd) {
    struct UserNode *newuser = (struct UserNode *) calloc(1, sizeof(struct UserNode));
    //printf("bp7\n");
    newuser->address = addr;
    newuser->sockfd = sockfd;
    //printf("bp8\n");
    if(ulist->lastUser == NULL) {
        ulist->firstUser = newuser;
        ulist->lastUser = newuser;
        ulist->length = 1;
    }
    else {
        ulist->lastUser->next = newuser;
        ulist->lastUser = newuser;
        ulist->length += 1;
    }
    //printf("bp9\n");
    if(pthread_create(&(userlist.lastUser->userThread), NULL, user_handle, (void *) newuser) != 0) {
        printf("thread creation error\n");
        exit(-1);
    }
}
