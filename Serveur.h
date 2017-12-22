//Mael Panouillot
//000 444 914
//BA-INFO

#ifndef Serveur_H
#define Serveur_H

int connected_identifier = 0;
//Nombre et id des clients connectes
typedef struct{
	int connect_identifier, client_filedescr, receive_filedescr;
	char pseudo[32];
	struct sockaddr_in client_addr;
	struct sockaddr_in receive_addr;
} client_type;
//Structure contenant toutes les informations necessaires pour se connecter a un client
client_type * connectes;
//Pointeur vers une variable qui sera unhe liste des clients connectes

void *gere_client(void *client);
//Fonction executee par un thread qui s'occupera d'un client, et repondra aux messages envoyes par celui-ci

#endif
