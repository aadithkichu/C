#include <stdio.h>

struct Router {
    unsigned cost[20];
    unsigned from[20]; // Acts as the "Next Hop"
} routingTable[20];

int main() {
    int costmat[20][20];
    int routers, i, j, k;

    printf("--- Distance Vector Routing ---\n");
    printf("Enter the number of routers: ");
    scanf("%d", &routers);

    printf("\nEnter the cost matrix (Use 999 to represent infinity/no link):\n");
    for (i = 0; i < routers; i++) {
        for (j = 0; j < routers; j++) {
            scanf("%d", &costmat[i][j]);
            
            // The cost to itself is always 0
            costmat[i][i] = 0; 
            
            // Initialize routing table with direct costs
            routingTable[i].cost[j] = costmat[i][j];
            routingTable[i].from[j] = j; 
        }
    }

    int table_updated;
    do {
        table_updated = 0;
        
        // i = source router, j = destination router, k = intermediate neighbor
        for (i = 0; i < routers; i++) {
            for (j = 0; j < routers; j++) {
                for (k = 0; k < routers; k++) {
                    
                    // Core Bellman-Ford Logic: 
                    // Is the cost via neighbor 'k' strictly less than current known cost?
                    if (routingTable[i].cost[j] > costmat[i][k] + routingTable[k].cost[j]) {
                        
                        // Update cost and next hop
                        routingTable[i].cost[j] = costmat[i][k] + routingTable[k].cost[j];
                        routingTable[i].from[j] = k;
                        
                        table_updated = 1; // Flag that a change happened
                    }
                }
            }
        }
    } while (table_updated != 0); // Keep iterating until network converges

    // Print final routing tables clearly
    for (i = 0; i < routers; i++) {
        printf("\n\nRouting Table for Router %d:\n", i + 1);
        printf("Dest\tNext Hop\tDistance\n");
        printf("--------------------------------\n");
        for (j = 0; j < routers; j++) {
            printf("%d\t%d\t\t%d\n", j + 1, routingTable[i].from[j] + 1, routingTable[i].cost[j]);
        }
    }
    
    printf("\n");
    return 0;
}