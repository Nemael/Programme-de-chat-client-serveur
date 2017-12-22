#Mael Panouillot
#000 444 914
#BA-INFO

CIBLE=Serveur Client

normal: $(CIBLE)

Serveur: Serveur.c
	gcc -Wall -Wextra -pthread Serveur.c -o Serveur
	
Client: Client.c
	gcc -Wall -Wextra -pthread Client.c -o Client

clean:
	$(RM) $(CIBLE) 
