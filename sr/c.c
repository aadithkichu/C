#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define TOTAL_FRAMES 9

// Returns 1 if faulty (simulated corruption), 0 if okay
int is_faulty() {
    int d = rand() % 4;
    return (d > 2); // 25% chance to simulate a fault
}

int main() {
    srand(time(0));
    int c_sock;
    struct sockaddr_in client;

    // 1. Setup Socket
    c_sock = socket(AF_INET, SOCK_STREAM, 0);
    memset(&client, 0, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_port = htons(9009);
    client.sin_addr.s_addr = inet_addr("127.0.0.1");

    // 2. Connect
    if (connect(c_sock, (struct sockaddr*)&client, sizeof(client)) == -1) {
        perror("Connection failed");
        exit(1);
    }

    printf("Connected to server - Selective Repeat Scheme\n\n");

    // --- RECEIVER BUFFER ---
    // Array to track which frames have arrived successfully.
    // 0 means missing, 1 means received safely.
    int received_frames[TOTAL_FRAMES] = {0}; 
    int frames_successfully_received = 0;

    // Loop until ALL unique frames are checked off the list
    while (frames_successfully_received < TOTAL_FRAMES) {
        char recv_buff[100];
        memset(recv_buff, 0, sizeof(recv_buff));

        // Read incoming frame from server
        int n = read(c_sock, recv_buff, sizeof(recv_buff));
        if (n <= 0) {
            printf("\nServer disconnected prematurely.\n");
            break;
        }

        printf("Message received from server: %s\n", recv_buff);

        // Extract the frame number safely
        int frame_num;
        if (sscanf(recv_buff, "Frame %d", &frame_num) != 1) {
            continue; // Skip if we couldn't parse a number
        }

        // Exam Pro-Tip: Prevent out-of-bounds array access (segfault protection)
        if (frame_num < 0 || frame_num >= TOTAL_FRAMES) {
            continue; 
        }

        char response[50];
        int fault = is_faulty();

        if (fault) {
            printf("[-] Simulating corruption for Frame %d\n", frame_num);
            sprintf(response, "NAK %d", frame_num); // Negative ACK
            write(c_sock, response, sizeof(response));
        } else {
            printf("[+] Sending ACK for Frame %d\n", frame_num);
            sprintf(response, "ACK %d", frame_num); // Positive ACK
            write(c_sock, response, sizeof(response));
            
            // --- SELECTIVE REPEAT LOGIC ---
            // Mark it as received ONLY if we haven't counted it yet
            if (received_frames[frame_num] == 0) {
                received_frames[frame_num] = 1;
                frames_successfully_received++;
                printf("    (Total unique frames received: %d/%d)\n", frames_successfully_received, TOTAL_FRAMES);
            } else {
                printf("    (Duplicate frame %d ignored. Already checked off.)\n", frame_num);
            }
        }
    }

    if (frames_successfully_received == TOTAL_FRAMES) {
        printf("\nAll %d frames received successfully!\n", TOTAL_FRAMES);
    }
    
    close(c_sock);
    return 0;
}