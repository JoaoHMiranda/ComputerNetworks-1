/*
EthernetSimulator2.c
Authors: Mateus Quezada and João Henrique

This version creates several child processes that remain idle while the parent
continuously sends Ethernet packets through the shared cable.
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
    
    // Determine the number of child processes (between 2 and 17)
    int num_processes = rand() % 16 + 2;
    cable->in_use = 0;
    cable->counter = 1;
    
    printf("NUMBER OF PROCESSES CREATED: %d\n", num_processes);
    
    // Create child processes; they will remain idle
    for (int i = 0; i < num_processes; i++){
        pid_t pid = fork();
        if (pid == 0) { // Child process
            while (1) {
                sleep(10); // Idle loop
            }
        }
    }
    
    // Parent process continuously sends packets
    while (1) {
        if (!cable->in_use) {
            cable->in_use = 1;
            
            EthernetPacket packet;
            for (int j = 0; j < 6; j++) {
                packet.destination_mac[j] = (rand() % 256 + cable->counter++) % 100;
                packet.source_mac[j] = (rand() % 256 + cable->counter++) % 100;
            }
            packet.type = rand() % 256;
            
            for (int j = 0; j < 1500; j++) {
                packet.data[j] = rand() % 256;
            }
            
            memcpy(cable->sendingPacket.destination_mac, packet.destination_mac, sizeof(packet.destination_mac));
            memcpy(cable->sendingPacket.source_mac, packet.source_mac, sizeof(packet.source_mac));
            cable->sendingPacket.type = packet.type;
            memcpy(cable->sendingPacket.data, packet.data, sizeof(packet.data));
            
            // Check if the packet in shared memory matches the sent packet
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
        // Sleep for 1.2 seconds before sending the next packet
        usleep(1200000);
    }
    
    // This part will never be reached due to the infinite loop
    return 0;
}
