#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <signal.h>

int main() {
    int sockfd , newsockfd ;
    struct sockaddr_in address;
    int sockaddrlen = sizeof(address);
    char buffer[1024];
    
    if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
        perror("Socket failed\n");
        exit(1);    
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(8080);
    address.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(sockfd,(struct sockaddr*) &address , sizeof(address)) < 0) {
        perror("Bind failed\n");
        close(sockfd);
        exit(1);
    }
    
    if (listen(sockfd,3) < 0 ) {
        perror("Listen failed\n");
        close(sockfd);
        exit(1);
    }

    printf("Listening on port 8080...\n");

    if ((newsockfd=accept(sockfd,(struct sockaddr*) &address , (socklen_t*)&sockaddrlen)) < 0) {
        perror("Accept failed\n");
        close(sockfd);
        exit(1);
    }

    printf("Connection successfull...\n");

    pid_t pid = fork();

    if (pid == 0) {
        while(1) {
            int val = recv(newsockfd, buffer, 1024, 0);
            if (val <= 0) {
                printf("Client disconnected.\n");
                kill(getppid(), SIGTERM);
                exit(0);
            }
            buffer[val] = '\0';
            printf("Client: %s", buffer);
        }
    } else {
        while(1) {
            fgets(buffer, 1024, stdin);
            send(newsockfd, buffer, strlen(buffer), 0);
            if (strcmp(buffer, "exit\n") == 0) {
                kill(pid, SIGTERM);
                break;
            }
        }
    }

    close(newsockfd);
    close(sockfd);
    return 0;
}
