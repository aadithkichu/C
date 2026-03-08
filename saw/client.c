#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

int main() {
    int c_sock;
    struct sockaddr_in server;

    // 1. Setup UDP Socket
    c_sock = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(9009);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    int frame_id = 0;
    int total_frames = 5; // Let's send 5 frames

    printf("Client Up - Stop-and-Wait ARQ\n\n");

    while (frame_id < total_frames) {
        char send_buff[50];
        sprintf(send_buff, "Frame %d", frame_id);
        
        int ack_received = 0;

        // Stay in this loop until we get a successful ACK
        while (!ack_received) {
            printf("Sending: %s\n", send_buff);
            sendto(c_sock, send_buff, strlen(send_buff), 0, (struct sockaddr*)&server, sizeof(server));

            // Setup the Timeout Timer (2 seconds)
            fd_set set;
            FD_ZERO(&set);
            FD_SET(c_sock, &set);
            struct timeval timeout = {2, 0};

            // Wait for Server to reply
            int rv = select(c_sock + 1, &set, NULL, NULL, &timeout);

            if (rv == -1) {
                perror("Select error");
            } else if (rv == 0) {
                // TIMEOUT
                printf("[!] Timeout! No ACK received. Resending...\n");
            } else {
                // DATA ARRIVED
                char recv_buff[50];
                recvfrom(c_sock, recv_buff, sizeof(recv_buff), 0, NULL, NULL);
                printf("Server replied: %s\n", recv_buff);

                // Check if it is a Negative ACK
                if (strstr(recv_buff, "NAK") != NULL) {
                    printf("[!] Received NAK. Resending...\n");
                } else {
                    // Success!
                    printf("[+] Success! Moving to next frame.\n\n");
                    ack_received = 1;
                    frame_id++; // Move to the next frame
                }
            }
        }
    }

    printf("All frames sent successfully!\n");
    close(c_sock);
    return 0;
}