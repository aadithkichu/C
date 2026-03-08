#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

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

    int count = 0;
    int total_frames = 9;

    while (count < total_frames) {
        char recv_buff[100];
        memset(recv_buff, 0, sizeof(recv_buff));

        // Read incoming frame from server
        int n = read(c_sock, recv_buff, sizeof(recv_buff));
        if (n <= 0) break;

        printf("Message received from server: %s\n", recv_buff);

        // Extract the frame number using sscanf (much safer than array indexing)
        int frame_num;
        sscanf(recv_buff, "Frame %d", &frame_num);

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
            
            // Only increment our count if it's the expected next frame
            if (frame_num == count) {
                count++;
            }
        }
    }

    printf("\nAll frames received successfully!\n");
    close(c_sock);
    return 0;
}