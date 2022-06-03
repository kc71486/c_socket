#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


int main(int argc , char *argv[]) {
    //socket的建立
    int sockfd = 0;    
    sockfd = socket(AF_INET , SOCK_STREAM , 0);

    if (sockfd == -1){
        printf("Fail to create a socket.");
    }

    //socket的連線

    struct sockaddr_in info;
    bzero(&info,sizeof(info));
    info.sin_family = PF_INET;

    //localhost test
    info.sin_addr.s_addr = inet_addr("127.0.0.1");
    info.sin_port = htons(8700);


    int err = connect(sockfd,(struct sockaddr *)&info,sizeof(info));
    if(err==-1){
        printf("Connection error");
    }


    char pre_message[] = {"client says: "};
    char message[100];
    char receiveMessage[100] = {};
    char receiveMessage2[100] = {};
    
    while(1) {
        //Send message to server
        scanf("%s", message);
        fflush(stdin);
        send(sockfd,pre_message,sizeof(pre_message),0);
        recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
        send(sockfd,message,sizeof(message),0);
        recv(sockfd,receiveMessage2,sizeof(receiveMessage2),0);
        printf("%s%s\n",receiveMessage, receiveMessage2);
    }
    printf("close Socket\n");
    close(sockfd);
    return 0;
}
