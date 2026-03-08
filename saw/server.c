#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

int main() {
    srand(time(0));
    int s_sock;
    struct sockaddr_in server, client;
    socklen_t client_len = sizeof(client);

    // 1. Setup UDP Socket & Bind
    s_sock = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(9009);
    server.sin_addr.s_addr = INADDR_ANY;

    bind(s_sock, (struct sockaddr*)&server, sizeof(server));
    printf("Server Up - Stop-and-Wait ARQ\nWaiting for frames...\n\n");

    int expected_frame = 0;

    while (1) {
        char recv_buff[50];
        memset(recv_buff, 0, sizeof(recv_buff));

        // Wait for incoming frame
        int n = recvfrom(s_sock, recv_buff, sizeof(recv_buff), 0, (struct sockaddr*)&client, &client_len);
        if (n <= 0) continue;
        
        recv_buff[n] = '\0';

        // Extract the frame number securely
        int frame_num;
        sscanf(recv_buff, "Frame %d", &frame_num);

        // --- Simulate Network Issues ---
        int fault = rand() % 4; // Generates 0, 1, 2, or 3
        char response[50];

        if (fault == 0) {
            // 25% chance: Drop the packet entirely
            printf("[-] Dropping '%s' to simulate packet loss.\n", recv_buff);
            // Do nothing. The client will timeout and resend.
            
        } else if (fault == 1) {
            // 25% chance: Corrupted data
            printf("[-] Simulating corruption. Sending NAK %d\n", frame_num);
            sprintf(response, "NAK %d", frame_num);
            sendto(s_sock, response, strlen(response), 0, (struct sockaddr*)&client, client_len);
            
        } else {
            // 50% chance: Perfect transmission
            printf("[+] Received '%s' cleanly. Sending ACK.\n", recv_buff);
            sprintf(response, "ACK %d", frame_num);
            sendto(s_sock, response, strlen(response), 0, (struct sockaddr*)&client, client_len);

            // Only update our expected frame if it's the one we are waiting for
            if (frame_num == expected_frame) {
                expected_frame++;
            }
        }
    }

    close(s_sock);
    return 0;
}