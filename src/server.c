#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "server.h"

int sockfd = 0;
struct UserList userlist;
char inputBuffer[128] = {};
char message[128] = {"server says: "};

int main(int argc , char *argv[]) {
    //create socket
    sockfd = socket(AF_INET , SOCK_STREAM , 0);
    
    if (sockfd == -1){
        printf("Fail to create a socket.");
    }
    
    //socket setting
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    
    server_addr.sin_family = PF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8700);
    
    bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(sockfd,5);
    
    // add clients
    while(1){
        struct sockaddr_in client_addr;
        int clientSockfd = 0;
        clientSockfd = accept(sockfd, (struct sockaddr*) &client_addr, &sizeof(client_addr));
        getpeername((clientSockfd, (struct sockaddr*)client_addr, sizeof(client_addr));
        printf("connect to client %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
        add_user(&userlist, client_addr, clientSockfd)
        
        if(pthread_create(&(userlist.lastUser->userThread), NULL, (void *)user_handle, NULL) != 0) {
            printf("thread creation error");
            exit(-1);
        }
    }
    return 0;
}

void user_handle() {
    recv(forClientSockfd,inputBuffer,sizeof(inputBuffer),0);
    snprintf(message, sizeof(message), "repeat: %s\n", inputBuffer);
    send(forClientSockfd,message,sizeof(message),0);
    printf("%s\n",message);
}

void add_user(struct UserList *ulist, struct sockaddr_in addr, int sockfd) {
    struct UserNode *newuser = calloc(1, sizeof(struct UserNode));
    newuser->address = addr;
    newuser->sockfd = sockfd;
    ulist->lastUser->next = newuser;
    ulist->lastUser = newuser;
    ulist->length += 1;
}