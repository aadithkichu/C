#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int csd;
    struct sockaddr_in servaddr;
    char remote_file[100], local_file[100], buffer[1024];

    // 1. Setup Socket
    csd = socket(AF_INET, SOCK_STREAM, 0);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(8080);

    // 2. Connect
    if (connect(csd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("[-] Connection failed");
        exit(1);
    }
    printf("[+] Connected to server.\n\n");

    // 3. Get User Input
    printf("Enter the file name to request from server: ");
    scanf("%s", remote_file);
    printf("Enter the local file name to save as: ");
    scanf("%s", local_file);

    // 4. Send Request
    send(csd, remote_file, strlen(remote_file), 0);

    // 5. Receive First Chunk (Check for Error)
    int s = recv(csd, buffer, sizeof(buffer) - 1, 0);
    
    if (s > 0) {
        buffer[s] = '\0'; // Null terminate to safely check strings
        
        if (strcmp(buffer, "ERROR") == 0) {
            printf("\n[!] Server: The requested file is not available.\n");
        } else {
            // File exists! Open local file and write the first chunk
            FILE *fp = fopen(local_file, "w");
            fwrite(buffer, 1, s, fp);
            
            // 6. Loop and receive the rest of the stream
            // When the server closes the socket, recv() returns 0 and breaks the loop.
            while ((s = recv(csd, buffer, sizeof(buffer), 0)) > 0) {
                fwrite(buffer, 1, s, fp);
            }
            
            fclose(fp);
            printf("\n[+] File transfer completed successfully. Saved as '%s'\n", local_file);
        }
    }

    close(csd);
    return 0;
}