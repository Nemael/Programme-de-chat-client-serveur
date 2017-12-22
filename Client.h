//Mael Panouillot
//000 444 914
//BA-INFO

#ifndef Client_h
#define Client_h
/*Client qui se connecte au serveur par addresse ip ou nom de domaine*/

char address[24];
//Contient l'addresse ip de connexion au serveur
char buffer[1024];
//Contient les messages recus par le server

const char* ask_ip_adress();
//Demande a l'utilisateur l'addresse ip du serveur
int send_message_to_server(int mode, int socketfd);
//Envoie des messages au serveur
void *receive_message(void* he_old);
//Thread qui recois des message du serveur

#endif
