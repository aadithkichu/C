#include <stdio.h>

int main() {
    int incoming, outgoing, bucket_size, n, bucket_content = 0;

    printf("--- Leaky Bucket Congestion Control ---\n");
    
    printf("Enter the bucket size: ");
    scanf("%d", &bucket_size);

    printf("Enter the outgoing rate (packets/tick): ");
    scanf("%d", &outgoing);

    printf("Enter the number of inputs: ");
    scanf("%d", &n);

    // Using a for-loop is much cleaner for tracking "Time Ticks"
    for (int i = 1; i <= n; i++) {
        printf("\n--- Time Tick %d ---\n", i);
        printf("Enter the incoming packet size: ");
        scanf("%d", &incoming);

        printf("Incoming packet size: %d\n", incoming);

        // 1. BUFFER CHECK: Does the bucket have enough space?
        if (incoming <= (bucket_size - bucket_content)) {
            bucket_content += incoming; 
            printf("[+] Packet Accepted. Bucket status: %d / %d\n", bucket_content, bucket_size);
        } else {
            // The packet doesn't fit, so it is strictly discarded
            printf("[-] Bucket Overflow! Packet of size %d dropped.\n", incoming);
            printf("    Bucket status remains: %d / %d\n", bucket_content, bucket_size);
        }

        // 2. THE LEAK: The bucket transmits data at a strict, constant rate
        bucket_content -= outgoing;

        // The bucket cannot hold negative packets
        if (bucket_content < 0) {
            bucket_content = 0;
        }

        printf("[>] After transmission, bucket status: %d / %d\n", bucket_content, bucket_size);
    }

    return 0;
}