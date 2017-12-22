//Mael Panouillot
//000 444 914
//BA-INFO

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <pthread.h>
#include "Serveur.h"

#define MYPORT 5555
#define BACKLOG 10

void *gere_client(void *client){
	/*Fonction executee par un thread qui sert a s'occuper d'un client qui s'est connecte*/
	int want_send = 1;
	char buffer[1024];
	char pseudo_from[32];
	char message[1024];
	//Initialise les variables utiles
	
	client_type *connected = (client_type *) client;
	//L'argument passe en parametre est caste en type client
	while(want_send){
		//Tant que le client veut envoyer des messages
		if (recv(connected->client_filedescr, buffer, 32, 0) == -1) {
			perror("Erreur: Receive");
		}
		if (!strcmp(buffer,"%%QUIT")){
			//Signale que l'utilisateur veut quitter la session, et cela termine la boucle
			want_send = 0;
		}
		else if (!strcmp(buffer,"%%CONNECTES")){
			//Signale que l'utilisateur veut voir la liste des gens connectes
			for (int i = 0; i < connected_identifier; i++){
				//Envoie un par un le pseudo de tout les clients connectés
				if (send(connected->client_filedescr, connectes[i].pseudo,32,0) == -1){
					perror("Erreur: Envoie list connectes");
				}
			}
			if (send(connected->client_filedescr, "%%END",32,0) == -1){
				perror("Erreur: Envoie signal fin liste_connectes");
			}
			//Signale que la liste des gens connectes est terminee
		}
		else{
			//Si n'importe quel autre message a ete recu, cela veut dire que le client a envoye le pseudo d'un destinataire
			int other_connect_id = -1;
			for (int i=0; i < connected_identifier;i++){
				if (!strcmp(connectes[i].pseudo,buffer)){
					//Compare le pseudo du destinataire avec le pseudo de tout les autres clients connectes
					other_connect_id = i;
				}
			}

			if (other_connect_id != -1){
				//Si le destinataire est connecte
				if (send(connected->client_filedescr, "%%OK",32,0) == -1){
					perror("Erreur: Envoie confirmation");
				}
				
				if (recv(connected->client_filedescr, buffer, 1024, 0) == -1){
					perror("Erreur: Reception message");
				}
				//Recois le message a transmettre
				strcpy(message,buffer);
				strcpy(pseudo_from,connected->pseudo);
				/*Met le pseudo de la personne qui va envoyer le message dans la variable pseudo_from,
				pour que celui qui receptionne le message sache de qui il vient*/
				for (int i = 0; i < connected_identifier; i++){
					if (connectes[i].connect_identifier == other_connect_id){
						//Trouve les informations du destinataire et envoie le message
						if (send(connectes[i].receive_filedescr, pseudo_from, 32, 0) == -1){
							perror("Erreur: Envoie confirmation");
						};
						if (send(connectes[i].receive_filedescr, message,1024,0) == -1){
							perror("Erreur: Envoie confirmation");
						};
					}
				}
			}
			else{
				//Si le destinataire n'est pas connecte
				if (send(connected->client_filedescr, "%%DECONNECTE",32,0) == -1){
					perror("Erreur: Envoie signal fin liste_connectes");
				}
			}
		}
	}
	//Fin de la boucle de reception des messages du client
	close(connected->client_filedescr);
	close(connected->receive_filedescr);
	printf("Deconnexion du client: %s\n",connected->pseudo);

	//Supprime le client de la liste des clients connectes
	connected_identifier -= 1;
	for (int i = connected->connect_identifier; i < connected_identifier; i++){
		connectes[i] = connectes[i+1];
		connected[i].connect_identifier -= 1;
	}
	pthread_detach(pthread_self());
	connectes = realloc(connectes, (connected_identifier+2)* sizeof(client_type));
	//Realoue le tableau pour qu'il puisse accueillir un client de moins
	return NULL;
}

int main(){
	connectes = malloc(2*sizeof(client_type));
	//Alloue le tableau pour qu'il puisse accueillir deux clients
	pthread_t dedicated_thread;
	//Declare le thread qui va s'occuper des client
	int listen_socket, connexion_socket, receive_socket;
	char buffer[32];
	struct sockaddr_in mon_sockaddr;
	struct sockaddr_in autre_sockaddr;
	struct sockaddr_in receive_sockaddr;
	
	//Definit le socket
	if ((listen_socket = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Erreur: Socket creation");
		return EXIT_FAILURE;
	}
	mon_sockaddr.sin_family = AF_INET;
	mon_sockaddr.sin_port = htons(MYPORT);
	mon_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	//Definit les attributs lies au struct
	
	//Bind le socket
	if(bind(listen_socket, (struct sockaddr*)&mon_sockaddr, sizeof(mon_sockaddr)) < 0){
		perror("Erreur: Socket Binding");
		return EXIT_FAILURE;
	}
	
	printf("Serveur demarre\n");
	
	while(1){
		//Boucle infinie qui accepte de nouveaux clients et cree plusieurs thread, dont chacun s'occupera d'un client
		int pseudo_already_connected = 0;
		//Listen
		if(listen(listen_socket, BACKLOG) == -1){
			perror("Serveur: listen");
			return EXIT_FAILURE;
		}

		//Definit les sockets de connexion au client
		socklen_t autre_len = sizeof(autre_sockaddr);
		connexion_socket = accept(listen_socket, (struct sockaddr*)&autre_sockaddr, &autre_len);
		
		if (recv(connexion_socket, buffer, 32, 0) == -1) {
			perror("Erreur: Receive nickname");
		}
		for (int i = 0; i < connected_identifier; i++){
			if (!strcmp(connectes[i].pseudo, buffer)){
				pseudo_already_connected = 1;
			}
		}
		if (pseudo_already_connected || (!strncmp("%", buffer, 1))){
			if(send(connexion_socket, "%%WRONG", 32, 0) == -1){
				perror("Serveur: Send Wrong");
			}
		}
		else{
			if(send(connexion_socket, "%%OK", 32, 0) == -1){
				perror("Serveur: Ok");
			}
			//Recois le pseudo de la personne qui vient de se connecter
			printf("Nouvelle connexion: ");
			printf("%s\n",buffer);
			
			client_type *nv_client = (client_type *)malloc(sizeof(client_type));
			nv_client->client_addr = autre_sockaddr;
			nv_client->connect_identifier = connected_identifier;
			nv_client->client_filedescr = connexion_socket;
			strncpy(nv_client->pseudo,buffer,32);
			//Definit les attributs de la connexion a ce client
			
			if(listen(listen_socket, BACKLOG) == -1){
				perror("Serveur: listen");
				return EXIT_FAILURE;
			}
			socklen_t receive_len = sizeof(receive_sockaddr);
			receive_socket = accept(listen_socket, (struct sockaddr*)&receive_sockaddr, &receive_len);
			nv_client->receive_addr = receive_sockaddr;
			nv_client->receive_filedescr = receive_socket;
			//Le socket receive_filedescr servira a envoyer des messages au client
			
			connectes[connected_identifier] = *nv_client;
			//Ajoute le nouveau client a la liste des clients connectes
			connectes = realloc(connectes, (connected_identifier+2)* sizeof(client_type));
			//Rallonge la liste pour pouvoir accepter de nouveaux clients
			
			connected_identifier++;
			pthread_create(&dedicated_thread, NULL, &gere_client, (void*)nv_client);
			//Cree un thread qui sera dedie au client
		}
	}
}
