#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

const char *ADDRESS = "127.0.0.1";
const char *NICKNAME = "client1";

char pre_message[] = {"client says: "};
char message[100];
char receiveMessage[100] = {};
char receiveMessage2[100] = {};

void send_func() {
    scanf("%s", message);
    fflush(stdin);
    send(sockfd,pre_message,sizeof(pre_message),0);
    send(sockfd,message,sizeof(message),0);
}

void recv_func() {
    recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
    recv(sockfd,receiveMessage2,sizeof(receiveMessage2),0);
     printf("%s%s\n",receiveMessage, receiveMessage2);
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
    
    //get name
    getpeername(sockfd, (struct sockaddr*)server_addr, sizeof(server_addr));
    getsockname(sockfd, (struct sockaddr*)client_addr, sizeof(client_addr));
    printf("connect to server %s:%d\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
    printf("you are %s your ip is%s:%d\n", NICKNAME, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    send(sockfd, NICKNAME, sizeof(NICKNAME), 0);
    
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
