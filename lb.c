#include <stdio.h>

int main() {
    int incoming, outgoing, bucket_size, n, bucket_content = 0;

    printf("--- Leaky Bucket Congestion Control ---\n");

    printf("Enter the bucket size: ");
    scanf("%d", &bucket_size);

    printf("Enter the outgoing rate (packets/tick): ");
    scanf("%d", &outgoing);

    printf("Enter the number of inputs to simulate: ");
    scanf("%d", &n);

    int tick = 1;
    int inputs_remaining = n;

    // The clock keeps ticking if we have inputs left OR if the bucket isn't empty yet
    while (inputs_remaining > 0 || bucket_content > 0) {
        printf("\n--- Time Tick %d ---\n", tick);

        // 1. ARRIVAL PHASE (Only ask for input if we still have inputs left)
        if (inputs_remaining > 0) {
            printf("Enter incoming packet size (enter 0 if no packets arrived): ");
            scanf("%d", &incoming);

            // BUFFER CHECK
            if (incoming <= (bucket_size - bucket_content)) {
                bucket_content += incoming; 
                printf("[+] Packet Accepted. Bucket status: %d / %d\n", bucket_content, bucket_size);
            } else {
                printf("[-] Bucket Overflow! Packet of size %d dropped.\n", incoming);
                printf("    Bucket status remains: %d / %d\n", bucket_content, bucket_size);
            }
            inputs_remaining--;
        } else {
            printf("[i] No more incoming packets. Waiting for bucket to drain...\n");
        }

        // 2. THE LEAK PHASE (Happens every single tick, no matter what)
        if (bucket_content > 0) {
            bucket_content -= outgoing;
            // Prevent negative bucket content
            if (bucket_content < 0) {
                bucket_content = 0;
            }
            printf("[>] Bucket leaked. Current status: %d / %d\n", bucket_content, bucket_size);
        } else {
            printf("[>] Bucket is already empty.\n");
        }

        tick++;
    }

    printf("\n--- Simulation Complete. Bucket is empty. ---\n");
    return 0;
}