//Mateus Quezada e João Henrique.


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/shm.h>




struct EthernetPacket {
	int destination_mac[6];
	int source_mac[6];
	int type;
	int data[1500];
};


struct EthernetCable{
	int in_use;
	int contador;
	struct EthernetPacket sendingPacket;
	
	
};




int main(){
	
	int memoryID = shmget(IPC_PRIVATE, sizeof(struct EthernetCable), IPC_CREAT | 0666);
	struct EthernetCable *teste = (struct EthernetCable*)shmat(memoryID, NULL, 0);
	int num_processes = rand() % 16 + 2;	
	teste->in_use = 0;
	teste->contador = 1;
	

	printf("NUMERO DE PROCESSOS CRIADOS: %d\n", num_processes);
	
	
	
	for(int i = 0; i<num_processes; i++){
		fork();	
	}
	while(1){
		if(!teste->in_use){
			teste->in_use = 1;
			struct EthernetPacket packet;
			for (int i = 0; i < 6; i++) {
				packet.destination_mac[i] = (rand() % 256 + teste->contador++)%100;
				
				
				packet.source_mac[i] = (rand() % 256 + teste->contador++)%100;
			}
			packet.type = rand() % 256;
			
			for (int i = 0; i < 1500; i++) {
				packet.data[i] = rand() % 256;
			}
			*(teste->sendingPacket.destination_mac) = *(packet.destination_mac);
			*(teste->sendingPacket.source_mac) = *(packet.source_mac);
			teste->sendingPacket.type = packet.type;
			*(teste->sendingPacket.data) = *(packet.data);
			
			//pacote enviado
			
			if(*(teste->sendingPacket.destination_mac) != *(packet.destination_mac) || //verifica se o pacote que esta no meio é o mesmo que foi enviado
			*(teste->sendingPacket.source_mac) != *(packet.source_mac) ||
			teste->sendingPacket.type != packet.type||
			*(teste->sendingPacket.data) != *(packet.data)){
				
				printf("COLISION\n"); // colisão
			}
			else{ //sem colisão
				printf("PACKET SENDED\n");
				printf("Endereço MAC Remetente: ");
				for(int i = 0; i<6; i++){
					printf("%d ", packet.source_mac[i]);
				}
				printf("\n");
				printf("Endereço MAC Destino: ");
				for(int i = 0; i<6; i++){
					printf("%d ", packet.destination_mac[i]);
				}
				printf("\n");
				printf("\n");
				teste->in_use = 0;
				
			}
		}
		sleep(2);
		
	}

	
	

}

