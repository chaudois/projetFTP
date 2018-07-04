#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;
const int PORT_ECOUTE = 999;

char*  readMessageClient(int socketClient)
{
	char* letter=malloc(1);
	char* buffer=malloc(512);
	int tailleRecu = 0;
	printf("reading...\n");
	while(*letter!="\n"){
		read(socketClient,letter,1);
		printf("%s",letter);
		strcat(buffer,letter);
  	}
 	printf("%s\n",buffer);

	return buffer;
}
void communicateWithClient(int socketClient){
	readMessageClient(socketClient);
 

}
void startServeur()
{
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		perror("socket()");
		exit(-1);
	}
	printf("Serveur créé\n", sock);
	SOCKADDR_IN sin = {0};

	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	sin.sin_family = AF_INET;

	sin.sin_port = htons(PORT_ECOUTE);

	if (bind(sock, (SOCKADDR *)&sin, sizeof sin) == SOCKET_ERROR)
	{
		perror("bind()");
		exit(-1);
	}
	if (listen(sock, 5) == SOCKET_ERROR)
	{
		perror("listen()");
		exit(-1);
	}
	printf("Le serveur écoute sur le port %d \n", PORT_ECOUTE);
	SOCKADDR_IN csin = {0};
	SOCKET csock;

	int sinsize = sizeof csin;

	int continuer = 1;
	while (continuer)
	{
		csock = accept(sock, (SOCKADDR *)&csin, &sinsize);

		if (csock == INVALID_SOCKET)
		{
			perror("accept()");
		}

		int pid = fork();
		if (pid == 0)
		{
			printf("connexion d'un client avec l'id %d \n", csock);
			communicateWithClient(csock);
			exit(0);
		}
	}
	closesocket(sock);
	closesocket(csock);
}
int main()
{
	startServeur();
	return 0;
}
