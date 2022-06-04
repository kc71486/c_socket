#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "client.h"

const char *ADDRESS = "127.0.0.1";
char NICKNAME[20];

char totalmessage[80];
char message[60];
char receiveMessage[128] = {};

void send_func() {
    fgets(message, sizeof(message), stdin);
    fflush(stdin);
    snprintf(totalmessage, sizeof(totalmessage), "<%s> %s", NICKNAME, message);
    send(sockfd, totalmessage, sizeof(totalmessage),0);
}

void recv_func() {
    recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
    printf("%s\n",receiveMessage);
}

int main(int argc , char *argv[]) {
    //create socket
    int sockfd = 0;    
    sockfd = socket(AF_INET , SOCK_STREAM , 0);

    if (sockfd == -1){
        printf("Fail to create a socket.");
    }

    //socket setting
    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));
    
    server_addr.sin_family = PF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ADDRESS);
    server_addr.sin_port = htons(8700);
    
    //connect to socket
    int err = connect(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr))
    if(err == -1){
        printf("Connection error");
    }
    
    //enter nickname
    printf("enter your nickname: ");
    fgets(nickname, sizeof(nickname), stdin);
    fflush(stdin);
    
    //get name
    getpeername(sockfd, (struct sockaddr*)server_addr, sizeof(server_addr));
    getsockname(sockfd, (struct sockaddr*)client_addr, sizeof(client_addr));
    printf("connect to server %s:%d\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
    printf("you are %s your ip is%s:%d\n", nickname, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    //send(sockfd, nickname, sizeof(nickname), 0);

    pthread_t send_thread, recv_thread;
    if(pthread_create(&send_thread, NULL, (void *)send_func, NULL) != 0) {
        printf("thread creation error");
        exit(-1);
    }
    if(pthread_create(&recv_thread, NULL, (void *)recv_func, NULL) != 0) {
        printf("thread creation error");
        exit(-1);
    }
    while(1) {
        if(0) {
            printf("close Socket\n");
            break;
        }
    }
    close(sockfd);
    return 0;
}
