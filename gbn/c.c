#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define WINDOW_SIZE 3
#define TOTAL_FRAMES 10

int main() {
    int c_sock;
    struct sockaddr_in server;

    // 1. Setup UDP Socket
    c_sock = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(9009);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    int base = 0;           // Oldest un-ACKed frame
    int next_seq_num = 0;   // Next frame to send

    printf("Client Up - Go-Back-N ARQ\n\n");

    while (base < TOTAL_FRAMES) {
        // --- 1. SEND ENTIRE WINDOW ---
        while (next_seq_num < base + WINDOW_SIZE && next_seq_num < TOTAL_FRAMES) {
            char send_buff[50];
            sprintf(send_buff, "Frame %d", next_seq_num);
            
            printf("Sending: %s\n", send_buff);
            sendto(c_sock, send_buff, strlen(send_buff), 0, (struct sockaddr*)&server, sizeof(server));
            
            next_seq_num++;
            usleep(50000); // 50ms delay so packets don't smash together
        }

        // --- 2. WAIT FOR ACKs ---
        fd_set set;
        FD_ZERO(&set);
        FD_SET(c_sock, &set);
        struct timeval timeout = {2, 0}; // 2-second timeout

        int rv = select(c_sock + 1, &set, NULL, NULL, &timeout);

        if (rv == -1) {
            perror("Select error");
        } else if (rv == 0) {
            // --- 3. TIMEOUT: THE "GO-BACK-N" LOGIC ---
            printf("\n[!] Timeout! Window from Frame %d lost.\n", base);
            printf("[!] GO-BACK-N: Rewinding to Frame %d...\n\n", base);
            
            // Reset our sending pointer back to the oldest un-ACKed frame
            next_seq_num = base; 
        } else {
            // --- 4. DATA ARRIVED (Cumulative ACKs) ---
            char recv_buff[50];
            recvfrom(c_sock, recv_buff, sizeof(recv_buff), 0, NULL, NULL);
            printf("Server replied: %s\n", recv_buff);

            int ack_num;
            if (sscanf(recv_buff, "ACK %d", &ack_num) == 1) {
                // If we get ACK 2, it means frames 0, 1, and 2 are ALL successful
                if (ack_num >= base) {
                    base = ack_num + 1; // Slide the window forward
                    printf("[+] Window shifted. Base is now %d\n\n", base);
                }
            }
        }
    }

    printf("All frames sent successfully!\n");
    close(c_sock);
    return 0;
}