#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;
const int PORT_ECOUTE = 998;
SOCKET sock;
void sig_handler(int signo)
{
	if (signo == SIGINT)
	{
		printf("\nIntériuption du serveur par l'utilisateur\n");
		close(sock);
	}
	exit(-1);
}

void readMessageClient(int socketClient)
{
	char letter = '0';
    char buffer[1024];
	memset(buffer, '0',sizeof(buffer));

	int tailleRecue = 0;
	while ((tailleRecue = read(socketClient, buffer, sizeof(buffer) - 1) > 0))
	{
 		printf("\n(%d) : [ %s ]\n",socketClient, buffer);
 	}
 
 } 
void startServeur()
{
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		perror("socket()");
		exit(-1);
	}
 	SOCKADDR_IN sin = {0};

	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	sin.sin_family = AF_INET;

	sin.sin_port = htons(PORT_ECOUTE);

	if (bind(sock, (SOCKADDR *)&sin, sizeof sin) == SOCKET_ERROR)
	{
		perror("bind()");
		exit(-1);
	}
	if (listen(sock, 0) == SOCKET_ERROR)
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
			exit(0);
		}

		int pid = fork();
		if (pid == 0)
		{
			printf("connexion d'un client avec l'id %d \n", csock);
			readMessageClient(csock);
			printf("Déconexion du client %d\n",csock);
			exit(EXIT_SUCCESS);
		}
	}
	closesocket(sock);
	closesocket(csock);
}
int main()
{
	signal(SIGINT, sig_handler);
	startServeur();
	return 0;
}
