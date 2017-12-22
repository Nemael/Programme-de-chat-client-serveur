//Mael Panouillot
//000 444 914
//BA-INFO

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <pthread.h>
#include "Client.h"

#define MYPORT 5555
#define MAXADDRESSSIZE 16
#define MAXNAMESIZE 24
#define MAXMESSAGESIZE 1024

const char* ask_ip_adress(){
	/*Demande l'addresse ip du serveur a l'utilisateur*/
		printf("%s\n","Bienvenu dans MeetNinja!");
		printf("%s","Entrez l'adresse du serveur: ");
		fgets(address,MAXADDRESSSIZE,stdin);
		strtok(address,"\n");
		//Enleve le \n a la fin de ip qui a ete capture par fgets()
		return address;
	}

int send_message_to_server(int mode, int socketfd){
	/*Envoie des messages au serveur, prend en parametre un mode, qui enverra certain messages au serveur selon la valeur de mode*/
	char pseudo_buffer[32];
	if (mode == 0){
		//Si l'utilisateur veut envoyer un message a un autre client		
		char *pseudo = malloc(MAXNAMESIZE);
		fgets(pseudo,MAXNAMESIZE,stdin);
		strtok(pseudo,"\n");
		//Scanne le terminal jusqu'a ce qu'il y ait un "\n"
		
		if (send(socketfd, pseudo, 32, 0) == -1){
			//Envoie au serveur le pseudo du destinataire
			perror("Client: Send pseudo");
			return EXIT_FAILURE;
		}
		if (recv(socketfd, buffer, 32, 0) == -1){
			//Recois le resultat du serveur
			return EXIT_FAILURE;
		}
		if (!strcmp(buffer,"%%DECONNECTE")){
			//Si le destinataire n'est pas connecte
			printf("Cette personne n'est pas connectee\n");
		}
		else{
			//Si le destinataire est connecte
			printf("Entrez le message a envoyer: ");
			char *message = malloc(MAXMESSAGESIZE);
			fgets(message,MAXMESSAGESIZE,stdin);
			//Scanne le terminal jusqu'a ce qu'il y ait un "\n"
			strtok(message,"\n");
			//Enleve le "\n" de la string

			
			if (send(socketfd, message,1024,0) == -1){
				//Envoie le message au serveur, qui le transmettra
				perror("Client: send");
				return EXIT_FAILURE;
			}
		}
	}
	else if (mode == 1){
		//Si le client veut voir la liste des clients connectes
		if (send(socketfd, "%%CONNECTES",32,0) == -1){
			perror("Client: send");
			return EXIT_FAILURE;
		}
		int continue_receive = 1;
		while (continue_receive){
			//Tant que le serveur ne signale pas que la liste est terminee
			if (recv(socketfd, pseudo_buffer, 32, 0) == -1){
				perror("Cliend: recv");
				return EXIT_FAILURE;
			}
			if (!strcmp(pseudo_buffer,"%%END")){
				//Si c'est vrai, alors cela veut dire que la liste est complete, et que la boucle peut etre arretee
				continue_receive = 0;
			}
			else{
				printf("-%s\n",pseudo_buffer);
			}
		}
	}
	else if (mode == 2){
		//Si le client veut quitter l'application
		if (send(socketfd, "%%QUIT",12,0) == -1){
			perror("Client: send");
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}	

void *receive_message(void* he_old){
	/*Methode executee par un thread, qui sert a recevoir des messages venant d'autre clients,
	sans que cela n'interfere avec les messages echanges dans la fonction send_message_to_serveur()*/
	int receiver_fd;
	struct sockaddr_in receiver_sockaddr;
	struct hostent *he = (struct hostent*) he_old;
	
	if ((he=gethostbyname(address)) == NULL) {
		//Obtient les valeurs pour se connecter au serveur
		perror("Receiver: gethostbyname");
		printf("%s\n","Erreur dans le nom du serveur");
	}
	
	if ((receiver_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Receiver: socket");
	}
	receiver_sockaddr.sin_family = AF_INET;    
	receiver_sockaddr.sin_port = htons(MYPORT);
	receiver_sockaddr.sin_addr = *((struct in_addr*)he->h_addr);
	memset(&(receiver_sockaddr.sin_zero), '\0', 8);
	//Definit les variables permettant la connexion avec le serveur
	
	if (connect(receiver_fd, (struct sockaddr *)&receiver_sockaddr, sizeof(struct sockaddr)) == -1) {
		//Se connecte au serveur
		perror("Receiver: connect");
	}
	while (1){
		//Boucle infinie de reception des messages
		if (recv(receiver_fd, buffer, 32, 0) == -1){
			//Recois le pseudo de l'envoyeur dans le socket special receiver_fd
			perror("Client: recv");
		}
		printf("Message recu de %s:\n", buffer);
		if (recv(receiver_fd, buffer, 1024, 0) == -1){
			//Recois le message dans le socket special receiver_fd
			perror("Client: recv");
		}
		printf("%s\n",buffer);
	}
	return NULL;
}
	
int main(){
	int socketfd;
	char pseudo[32];
	struct hostent *he;
	pthread_t receiver_thread;
	//C'est le thread qui s'occupera de recevoir des messages venant d'autres clients
	struct sockaddr_in autre_sockaddr;
	
	const char* ipadress=ask_ip_adress();
	if ((he=gethostbyname(ipadress)) == NULL) {
		//Obtient les identifiants du serveur avec l'addresse fournie par le client
		perror("Client: gethostbyname");
		printf("%s\n","Erreur dans le nom du serveur");
		return EXIT_FAILURE;
	}
	
	if ((socketfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Client: socket");
		return EXIT_FAILURE;
	}
	autre_sockaddr.sin_family = AF_INET;    
	autre_sockaddr.sin_port = htons(MYPORT);
	autre_sockaddr.sin_addr = *((struct in_addr*)he->h_addr);
	memset(&(autre_sockaddr.sin_zero), '\0', 8);
    //Definit les variables de connexion avec le serveur
	
    if (connect(socketfd, (struct sockaddr *)&autre_sockaddr, sizeof(struct sockaddr)) == -1) {
		//Se connecte au serveur par le socket
    	perror("Client: connect");
		return EXIT_FAILURE;
	}
	
	printf("%s\n","Connection reussie, veuillez entrer votre pseudonyme (Max 18 lettres): ");
	fgets(pseudo,20,stdin);
	strtok(pseudo,"\n");
	//Demande le pseudonyme a l'utilisateur
	if (send(socketfd, pseudo,32,0) == -1){
			perror("Client: send");
			return EXIT_FAILURE;
		}
	if (recv(socketfd, buffer, 32, 0) == -1){
		//Recois du serveur "%%OK" si le pseudo est libre, si c'est autre chose, alors le pseudo est deja pris
		return EXIT_FAILURE;
	}
	if ((!strcmp(buffer,"%%OK")) || (!strncmp(buffer,"%",1))){
		//Si le pseudo n'est pas deja prit et est conforme, alors le programme continue
		pthread_create(&receiver_thread, NULL, &receive_message, (void*)&he);
		//Cree le thread qui va s'occuper de recevoir les messages venant des autres clients
		int want_continue = 1;
		while(want_continue){
			//Tant que l'utilisateur ne veut pas quitter l'application
			char choix[4];
			printf("%s","Que voulez vous faire?:\n1) Consulter la liste d'utilisateurs en ligne\n2) Envoyer un message\n3) Quitter MeetNinja\n");
			fgets(choix,4,stdin);
			strtok(choix,"\n");
			if (!strcmp(choix,"1")){
				//Si l'utilisateur veut voir la liste des gens connectes
				printf("Liste des gens connectes:\n");
				send_message_to_server(1, socketfd);
				//Le 1 signale a la fonction que l'utilisateur veut voir la liste des gens connectes
			}
			else if (!strcmp(choix,"2")){
				//Si l'utilisateur veut envoyer un message a un autre client
				send_message_to_server(1, socketfd);
				//Le 1 signale a la fonction que l'utilisateur veut voir la liste des gens connectes
				printf("Entrez le pseudo du destinataire: ");
				send_message_to_server(0,socketfd);
				//Le 0 signale que l'utilisateur veut envoyer un message a un autre client
			}
			else if (!strcmp(choix,"3")){
				send_message_to_server(2,socketfd);
				//Le 2 signale que l'utilisateur veut quitter l'application
				want_continue = 0;
				//Termine la boucle
			}
			else{
				printf("%s\n","Choix incorrect");
			}
		}
	}
	else{
		//Si le pseudo est pris ou n'est pas conforme, ferme le socket et quitte le programme
		printf("Ce pseudo est deja utilise sur ce serveur, ou il commence par %s, ce qui est interdit.\n","%");
	}
	printf("Fermeture du client, au revoir.\n");
	close(socketfd);
	//Ferme les sockets de connexion avec le serveur et termine l'application
}
