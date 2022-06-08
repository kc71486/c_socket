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
int sockfd = 0;

int main(int argc , char *argv[]) {
    //initialize
    char nickname[20];
    //create socket 
    sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if (sockfd == -1){
        printf("Fail to create a socket.");
    }
    
    //socket setting
    struct sockaddr_in server_addr, client_addr;
    int s_addrlen = sizeof(server_addr); //for getpeername()
    int c_addrlen = sizeof(client_addr); //for getsockname()
    memset(&server_addr, 0, s_addrlen);
    memset(&client_addr, 0, c_addrlen);
    
    server_addr.sin_family = PF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ADDRESS);
    server_addr.sin_port = htons(8700);
    
    //connect to socket
    int err = connect(sockfd,(struct sockaddr *)&server_addr, s_addrlen);
    if(err == -1){
        printf("Connection error\n");
    }
    
    //enter nickname
    printf("enter your nickname: ");
    fgets(nickname, sizeof(nickname), stdin);
    nickname[strcspn(nickname, "\n")] = 0;
    fflush(stdin);
    
    //get name
    getpeername(sockfd, (struct sockaddr*)&server_addr, &s_addrlen);
    getsockname(sockfd, (struct sockaddr*)&client_addr, &c_addrlen);
    printf("connect to server %s:%d\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
    printf("you are %s your ip is %s:%d\n", nickname, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    send(sockfd, nickname, sizeof(nickname), 0);

    pthread_t send_thread, recv_thread;
    if(pthread_create(&send_thread, NULL, (void *)send_func, NULL) != 0) {
        printf("thread creation error\n");
        exit(-1);
    }
    if(pthread_create(&recv_thread, NULL, (void *)recv_func, NULL) != 0) {
        printf("thread creation error\n");
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

void send_func() {
    char message[60];
    while(1)  {
        fgets(message, sizeof(message), stdin);
        message[strcspn(message, "\n")] = 0;
        fflush(stdin);
        send(sockfd, message, sizeof(message),0);
    }
}

void recv_func() {
    char receiveMessage[128];
    while(1) {
        recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
        printf("%s\n",receiveMessage);
    }
}
