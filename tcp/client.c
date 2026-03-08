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
    int sock ;
    struct sockaddr_in address ;
    char buffer[1024];
    
    if ((sock = socket(AF_INET,SOCK_STREAM,0)) < 0 ) {
        perror("Socket failed\n");
        exit(1);    
    }
    
    address.sin_family = AF_INET;
    address.sin_port = htons(8080);
    inet_pton(AF_INET,"127.0.0.1",&address.sin_addr);
    
    if (connect(sock,(struct sockaddr*) &address , sizeof(address))<0) {
        perror("Connect failed\n");
        close(sock);
        exit(1);
    }
    
    printf("Server connected...\n");

    pid_t pid = fork();

    if (pid == 0) {
        while(1) {
            int val = recv(sock, buffer, 1024, 0);
            if (val <= 0) {
                printf("Server disconnected.\n");
                kill(getppid(), SIGTERM);
                exit(0);
            }
            buffer[val] = '\0';
            printf("Server: %s", buffer);
        }
    } else {
        while(1) {
            fgets(buffer, 1024, stdin);
            send(sock, buffer, strlen(buffer), 0);
            if (strcmp(buffer, "exit\n") == 0) {
                kill(pid, SIGTERM);
                break;
            }
        }
    }

    close(sock);
    return 0;
}
