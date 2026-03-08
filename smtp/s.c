#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    struct sockaddr_in server, client;
    char str[1024], msg[1024];
    socklen_t client_len = sizeof(client);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(atoi(argv[1]));

    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Error in bind()");
        exit(1);
    }

    printf("SMTP Server started. Waiting for client...\n\n");

    // 1. Wait for initial connection/greeting from client
    recvfrom(sockfd, str, sizeof(str), 0, (struct sockaddr *)&client, &client_len);
    printf("Got message from client: %s\n", str);
    
    // 2. Send SMTP Ready (220)
    strcpy(str, "220 127.0.0.1 SMTP Service Ready");
    sendto(sockfd, str, strlen(str), 0, (struct sockaddr*)&client, client_len);
    printf("Sent: %s\n", str);

    // 3. Wait for HELO
    recvfrom(sockfd, str, sizeof(str), 0, (struct sockaddr*)&client, &client_len);
    if (strncmp(str, "HELO", 4) != 0) printf("\n[!] 'HELO' expected.\n");
    printf("Received: %s\n", str);
    
    strcpy(str, "250 OK");
    sendto(sockfd, str, strlen(str), 0, (struct sockaddr*)&client, client_len);

    // 4. Wait for MAIL FROM
    recvfrom(sockfd, str, sizeof(str), 0, (struct sockaddr *)&client, &client_len);
    if (strncmp(str, "MAIL FROM", 9) != 0) printf("\n[!] 'MAIL FROM' expected.\n");
    printf("Received: %s\n", str);
    
    strcpy(str, "250 OK");
    sendto(sockfd, str, strlen(str), 0, (struct sockaddr *)&client, client_len);

    // 5. Wait for RCPT TO
    recvfrom(sockfd, str, sizeof(str), 0, (struct sockaddr *)&client, &client_len);
    if (strncmp(str, "RCPT TO", 7) != 0) printf("\n[!] 'RCPT TO' expected.\n");
    printf("Received: %s\n", str);
    
    strcpy(str, "250 OK");
    sendto(sockfd, str, strlen(str), 0, (struct sockaddr *)&client, client_len);

    // 6. Wait for DATA command
    recvfrom(sockfd, str, sizeof(str), 0, (struct sockaddr *)&client, &client_len);
    if (strncmp(str, "DATA", 4) != 0) printf("\n[!] 'DATA' expected.\n");
    printf("Received: %s\n", str);
    
    strcpy(str, "354 Start mail input; end with <CRLF>.<CRLF>");
    sendto(sockfd, str, strlen(str), 0, (struct sockaddr*)&client, client_len);

    // 7. Receive the actual email body
    int n = recvfrom(sockfd, msg, sizeof(msg), 0, (struct sockaddr *)&client, &client_len);
    msg[n] = '\0';
    printf("\n--- Mail Body Received ---\n%s\n--------------------------\n", msg);

    // 8. Wait for QUIT
    recvfrom(sockfd, str, sizeof(str), 0, (struct sockaddr *)&client, &client_len);
    if (strncmp(str, "QUIT", 4) != 0) printf("\n[!] 'QUIT' expected.\n");
    printf("Received: %s\n", str);
    
    strcpy(str, "221 127.0.0.1 Service closing transmission channel");
    sendto(sockfd, str, strlen(str), 0, (struct sockaddr *)&client, client_len);
    printf("Sent: 221 QUIT response. Closing Server.\n");

    close(sockfd);
    return 0;
}