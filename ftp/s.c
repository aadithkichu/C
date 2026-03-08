#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int sd, newsd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    char filename[100], buffer[1024];

    // 1. Setup Socket
    sd = socket(AF_INET, SOCK_STREAM, 0);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(8080); // Hardcoded for easier testing

    bind(sd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    listen(sd, 5);
    
    printf("File Transfer Server Up. Waiting for client on port 8080...\n");

    // 2. Accept Connection
    newsd = accept(sd, (struct sockaddr *)&cliaddr, &clilen);
    printf("[+] Client Connected.\n");

    // 3. Receive the requested filename
    int n = recv(newsd, filename, sizeof(filename) - 1, 0);
    filename[n] = '\0';
    
    // Clean up any trailing newlines from the client input
    filename[strcspn(filename, "\n")] = 0; 

    // 4. Open and Send File
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("[-] File not found: %s\n", filename);
        strcpy(buffer, "ERROR");
        send(newsd, buffer, strlen(buffer), 0);
    } else {
        printf("[+] Sending file: %s\n", filename);
        
        // Read chunks and send immediately (No sleep required!)
        int bytes_read;
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
            send(newsd, buffer, bytes_read, 0);
        }
        
        fclose(fp);
        printf("[+] File sent successfully.\n");
    }

    // 5. Close connection (This acts as the EOF signal to the client)
    close(newsd);
    close(sd);
    return 0;
}