#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

int main() {
    int s_sock, c_sock;
    struct sockaddr_in server, other;
    socklen_t add_len = sizeof(other);

    // 1. Setup Socket
    s_sock = socket(AF_INET, SOCK_STREAM, 0);
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(9009);
    server.sin_addr.s_addr = INADDR_ANY;

    // 2. Bind & Listen
    if (bind(s_sock, (struct sockaddr*)&server, sizeof(server)) == -1) {
        perror("Binding failed");
        exit(1);
    }
    listen(s_sock, 10);
    printf("Server Up - Selective Repeat Scheme\nWaiting for client...\n\n");

    // 3. Accept Connection
    c_sock = accept(s_sock, (struct sockaddr*)&other, &add_len);
    printf("Client connected!\n\n");

    int tot = 0;           // Total frames successfully acknowledged
    int window_size = 3;   // Send 3 frames at a time
    int total_frames = 9;  // Target number of frames to send

    while (tot < total_frames) {
        // --- SEND WINDOW ---
        for (int j = tot; j < tot + window_size && j < total_frames; j++) {
            char send_buff[50];
            sprintf(send_buff, "Frame %d", j); // Safe string formatting
            printf("Sending to client: %s\n", send_buff);
            write(c_sock, send_buff, sizeof(send_buff));
            usleep(50000); // 50ms delay to prevent TCP messages sticking together
        }

        // --- RECEIVE ACKs (Selective Repeat Logic) ---
        for (int k = tot; k < tot + window_size && k < total_frames; k++) {
            int ack_received = 0;
            
            // Stay in this loop until frame 'k' gets a positive ACK
            while (!ack_received) {
                fd_set set;
                FD_ZERO(&set);
                FD_SET(c_sock, &set);
                struct timeval timeout = {2, 0}; // 2 second timeout

                int rv = select(c_sock + 1, &set, NULL, NULL, &timeout);

                if (rv == -1) {
                    perror("Select error");
                } else if (rv == 0) {
                    // TIMEOUT
                    printf("[!] Timeout for Frame %d. Resending...\n", k);
                    char resend_buff[50];
                    sprintf(resend_buff, "Frame %d", k);
                    write(c_sock, resend_buff, sizeof(resend_buff));
                } else {
                    // DATA ARRIVED
                    char recv_buff[50];
                    read(c_sock, recv_buff, sizeof(recv_buff));
                    printf("Response from Client: %s\n", recv_buff);

                    // Check if it's a Negative ACK (NAK)
                    if (strstr(recv_buff, "NAK") != NULL) {
                        printf("[!] Corrupt ACK for Frame %d. Resending...\n", k);
                        char resend_buff[50];
                        sprintf(resend_buff, "Frame %d", k);
                        write(c_sock, resend_buff, sizeof(resend_buff));
                    } else {
                        int ack_num;
                        // Extract the number from the string "ACK X"
                        if (sscanf(recv_buff, "ACK %d", &ack_num) == 1) {
                            
                            // ONLY accept it if it matches the 'k' we are waiting for
                            if (ack_num == k) {
                                ack_received = 1;
                                tot++; 
                                printf("[+] Successfully verified ACK for Frame %d\n", k);
                            } else {
                                // If it's the wrong ACK, ignore it and let the timeout handle 'k'
                                printf("[!] Received ACK %d, but still waiting for ACK %d. Ignoring.\n", ack_num, k);
                            }
                        }
                    }
                }
            }
        }
    }

    printf("\nAll frames sent and acknowledged successfully!\n");
    close(c_sock);
    close(s_sock);
    return 0;
}