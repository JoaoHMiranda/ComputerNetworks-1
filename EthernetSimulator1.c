/*
EthernetSimulator1.c
Authors: Mateus Quezada and João Henrique

This program simulates Ethernet packet sending using multiple processes.
Each child process continuously attempts to send a packet through a shared cable.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <string.h>

typedef struct {
    int destination_mac[6];
    int source_mac[6];
    int type;
    int data[1500];
} EthernetPacket;

typedef struct {
    int in_use;
    int counter;
    EthernetPacket sendingPacket;
} EthernetCable;

int main(){
    // Seed the random number generator
    srand(time(NULL));

    // Create shared memory for the EthernetCable structure
    int memoryID = shmget(IPC_PRIVATE, sizeof(EthernetCable), IPC_CREAT | 0666);
    if (memoryID == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }
    EthernetCable *cable = (EthernetCable*)shmat(memoryID, NULL, 0);
    if (cable == (void*) -1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }
    
    // Determine the number of processes to create (between 2 and 17)
    int num_processes = rand() % 16 + 2;    
    cable->in_use = 0;
    cable->counter = 1;
    
    printf("NUMBER OF PROCESSES CREATED: %d\n", num_processes);
    
    pid_t pids[num_processes];
    
    // Create child processes
    for (int i = 0; i < num_processes; i++) {
        pids[i] = fork();    
        if (pids[i] == 0) { // Child process
            while (1) {
                if (!cable->in_use) {
                    cable->in_use = 1;
                    
                    EthernetPacket packet;
                    
                    // Fill MAC addresses using a simple counter and random values
                    for (int j = 0; j < 6; j++) {
                        packet.destination_mac[j] = (rand() % 256 + cable->counter++) % 100;
                        packet.source_mac[j] = (rand() % 256 + cable->counter++) % 100;
                    }
                    packet.type = rand() % 256;
                    
                    // Fill data with random values
                    for (int j = 0; j < 1500; j++) {
                        packet.data[j] = rand() % 256;
                    }
                    
                    // Copy the packet into shared memory using memcpy
                    memcpy(cable->sendingPacket.destination_mac, packet.destination_mac, sizeof(packet.destination_mac));
                    memcpy(cable->sendingPacket.source_mac, packet.source_mac, sizeof(packet.source_mac));
                    cable->sendingPacket.type = packet.type;
                    memcpy(cable->sendingPacket.data, packet.data, sizeof(packet.data));
                    
                    // Verify that the packet in shared memory matches the sent packet
                    if (memcmp(cable->sendingPacket.destination_mac, packet.destination_mac, sizeof(packet.destination_mac)) != 0 ||
                        memcmp(cable->sendingPacket.source_mac, packet.source_mac, sizeof(packet.source_mac)) != 0 ||
                        cable->sendingPacket.type != packet.type ||
                        memcmp(cable->sendingPacket.data, packet.data, sizeof(packet.data)) != 0) {
                        
                        printf("COLLISION DETECTED\n");
                    } else {
                        printf("PACKET SENT\n");
                        printf("Source MAC Address: ");
                        for (int j = 0; j < 6; j++){
                            printf("%d ", packet.source_mac[j]);
                        }
                        printf("\nDestination MAC Address: ");
                        for (int j = 0; j < 6; j++){
                            printf("%d ", packet.destination_mac[j]);
                        }
                        printf("\n\n");
                    }
                    
                    cable->in_use = 0;
                }
                // Sleep for 1.2 seconds before trying again
                usleep(1200000);
            }
        }
    }
    
    // Parent process waits for all child processes (this code will actually never be reached due to the infinite loop)
    for (int i = 0; i < num_processes; i++) {
        wait(NULL);
    }
    
    return 0;
}
