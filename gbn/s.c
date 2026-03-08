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
    printf("Server Up - Go-Back-N ARQ\nWaiting for frames...\n\n");

    int expected_frame = 0;

    while (1) {
        char recv_buff[50];
        int n = recvfrom(s_sock, recv_buff, sizeof(recv_buff), 0, (struct sockaddr*)&client, &client_len);
        if (n <= 0) continue;
        recv_buff[n] = '\0';

        int frame_num;
        sscanf(recv_buff, "Frame %d", &frame_num);

        // --- Simulate Network Issues (20% Failure Rate) ---
        int fault = rand() % 5; 
        char response[50];

        if (fault == 0) {
            // Drop packet (simulated loss)
            printf("[-] Dropping '%s' (Simulated packet loss)\n", recv_buff);
        } else {
            // Packet arrived safely. Is it the one we want?
            if (frame_num == expected_frame) {
                // Perfect order
                printf("[+] Received in-order '%s'. Sending ACK %d.\n", recv_buff, expected_frame);
                sprintf(response, "ACK %d", expected_frame);
                sendto(s_sock, response, strlen(response), 0, (struct sockaddr*)&client, client_len);
                expected_frame++; // Move to next expected frame
            } else {
                // Out of order! (e.g., received Frame 3, but wanted Frame 2)
                printf("[!] Out-of-order Frame %d (Expected %d). Discarding.\n", frame_num, expected_frame);
                
                // GBN Rule: Resend ACK for the LAST correctly received frame
                if (expected_frame > 0) {
                    printf("[!] Resending ACK %d\n", expected_frame - 1);
                    sprintf(response, "ACK %d", expected_frame - 1);
                    sendto(s_sock, response, strlen(response), 0, (struct sockaddr*)&client, client_len);
                }
            }
        }
    }

    close(s_sock);
    return 0;
}