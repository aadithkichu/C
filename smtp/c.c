#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        exit(1);
    }

    struct sockaddr_in server;
    char str[1024], mail_f[50], mail_to[50], msg[1024];
    socklen_t l = sizeof(server);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(atoi(argv[2]));

    printf("SMTP Client Started...\n\n");

    // 1. Initial connection trigger
    strcpy(str, "hi");
    sendto(sockfd, str, strlen(str), 0, (struct sockaddr*)&server, sizeof(server));
    
    // 2. Receive 220 Ready
    int n = recvfrom(sockfd, str, sizeof(str), 0, (struct sockaddr *)&server, &l);
    str[n] = '\0';
    printf("Server: %s\n", str);
    if (strncmp(str, "220", 3) != 0) printf("[!] Connection not established.\n");

    // 3. Send HELO
    strcpy(str, "HELO 127.0.0.1");
    printf("Client: %s\n", str);
    sendto(sockfd, str, strlen(str), 0, (struct sockaddr *)&server, sizeof(server));
    
    n = recvfrom(sockfd, str, sizeof(str), 0, (struct sockaddr *)&server, &l);
    str[n] = '\0';
    printf("Server: %s\n", str);

    // 4. Send MAIL FROM
    printf("\nEnter FROM address: ");
    scanf("%s", mail_f);
    sprintf(str, "MAIL FROM: <%s>", mail_f);
    printf("Client: %s\n", str);
    sendto(sockfd, str, strlen(str), 0, (struct sockaddr *)&server, sizeof(server));
    
    n = recvfrom(sockfd, str, sizeof(str), 0, (struct sockaddr *)&server, &l);
    str[n] = '\0';
    printf("Server: %s\n", str);

    // 5. Send RCPT TO
    printf("\nEnter TO address: ");
    scanf("%s", mail_to);
    sprintf(str, "RCPT TO: <%s>", mail_to);
    printf("Client: %s\n", str);
    sendto(sockfd, str, strlen(str), 0, (struct sockaddr *)&server, sizeof(server));
    
    n = recvfrom(sockfd, str, sizeof(str), 0, (struct sockaddr *)&server, &l);
    str[n] = '\0';
    printf("Server: %s\n", str);

    // 6. Send DATA command
    strcpy(str, "DATA");
    printf("Client: %s\n", str);
    sendto(sockfd, str, strlen(str), 0, (struct sockaddr *)&server, sizeof(server));
    
    n = recvfrom(sockfd, str, sizeof(str), 0, (struct sockaddr *)&server, &l);
    str[n] = '\0';
    printf("Server: %s\n", str);

    // 7. Input and send Mail Body
    printf("\nEnter mail body (End with '$'):\n");
    int t = 0;
    char c;
    while (1) {
        c = getchar();
        if (c == '$') {
            msg[t] = '\0';
            break;
        }
        if (c != '\0') { // Ignore null chars from stdin
            msg[t++] = c;
        }
    }
    
    sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&server, sizeof(server));
    printf("Client: [Body Sent]\n");

    // 8. Send QUIT
    strcpy(str, "QUIT");
    printf("Client: %s\n", str);
    sendto(sockfd, str, strlen(str), 0, (struct sockaddr *)&server, sizeof(server));
    
    n = recvfrom(sockfd, str, sizeof(str), 0, (struct sockaddr *)&server, &l);
    str[n] = '\0';
    printf("Server: %s\n", str);

    printf("\nBye. Connection Closed.\n");
    close(sockfd);
    return 0;
}