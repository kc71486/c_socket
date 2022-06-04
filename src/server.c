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
    //create socket
    sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if (sockfd == -1){
        printf("Fail to create a socket.");
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
    char message[128] = {"server says: "};
    
    while(1) {
        recv(user->sockfd, inputBuffer, sizeof(inputBuffer), 0);
        snprintf(message, sizeof(message), "repeat: %s\n", inputBuffer);
        send(user->sockfd, message, sizeof(message), 0);
        printf("%s\n",message);
    }
    
    return NULL;
}

void add_user(struct UserList *ulist, struct sockaddr_in addr, int sockfd) {
    printf("%d\n",sizeof(struct UserNode));
    malloc(sizeof(struct UserNode));
    printf("bp5");
    struct UserNode *newuser = malloc(sizeof(struct UserNode));
    printf("bp6");
    memset(&newuser, 0, sizeof(struct UserNode));
    printf("bp7");
    newuser->address = addr;
    newuser->sockfd = sockfd;
    printf("bp8");
    ulist->lastUser->next = newuser;
    ulist->lastUser = newuser;
    ulist->length += 1;
    printf("bp9");
    if(pthread_create(&(userlist.lastUser->userThread), NULL, user_handle, (void *) newuser) != 0) {
        printf("thread creation error");
        exit(-1);
    }
}