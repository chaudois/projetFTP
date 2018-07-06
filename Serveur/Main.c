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
#include <assert.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;
const int PORT_ECOUTE = 998;
const int tailleBuffer=512;
SOCKET socketServeur;
int saveFile;
void readClient(int socket, char *message)
{
	memset(message, '0', sizeof(message));
	read(socket, message, tailleBuffer);
	printf("\n(%d) [%s]\n", socket, message);
}

void sig_handler(int signo)
{
	if (signo == SIGINT)
	{
		printf("\nIntériuption du serveur par l'utilisateur\n");
		close(socketServeur);
	}
	exit(-1);
}

void closeClient(int socket)
{
	close(socket);
	printf("\nclient %d déconnecté\n", socket);

	exit(-1);
}
int loginClient(int socketClient)
{
	int loginok = 0;
	int cptTry = 0;
	char message[tailleBuffer];
	char nomClient[tailleBuffer];
	memset(message, '0', sizeof(message));
	memset(nomClient, '0', sizeof(nomClient));
	int tailleRecue = 0;

	readClient(socketClient, message);

	if (strstr(message, "BONJ"))
	{

		while (!loginok && cptTry < 3)
		{

			if (send(socketClient, "WHO", tailleBuffer, 0) > 0)
			{

				readClient(socketClient, message);
			}
			else
			{
				closeClient(socketClient);
			}
			if (send(socketClient, "PASSWD\0", tailleBuffer, 0) > 0)
			{
				readClient(socketClient, message);
				if (message[0] != '\0' && message[0] != '\n')
				{

					cptTry = cptTry + 1;

					if (1)
					{
						send(socketClient, "WELC", tailleBuffer, 0);
						return 1;
					}
					else if (cptTry > 2)
					{
						send(socketClient, "BYE", tailleBuffer, 0);
					}
					else
					{

						send(socketClient, "NOPE", tailleBuffer, 0);
					}
				}
			}
			else
			{
				closeClient(socketClient);
			}
		}
	}

	return 0;
}
void readCommandClient(int socketClient)
{
 
	char letter = '0';
	char buffer[1024];
	memset(buffer, '0', sizeof(buffer));

	int tailleRecue = 0;
	while ((tailleRecue = read(socketClient, buffer, sizeof(buffer) - 1) > 0))
	{
		if (buffer[0] != '\0')
		{
			printf("\n(%d) : [ %s ]\n", socketClient, buffer);
		}
	}
}
void startServeur()
{
	socketServeur = socket(AF_INET, SOCK_STREAM, 0);
	if (socketServeur == INVALID_SOCKET)
	{
		perror("socket()");
		exit(-1);
	}
	SOCKADDR_IN sin = {0};

	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	sin.sin_family = AF_INET;

	sin.sin_port = htons(PORT_ECOUTE);

	if (bind(socketServeur, (SOCKADDR *)&sin, sizeof sin) == SOCKET_ERROR)
	{
		perror("bind()");
		exit(-1);
	}
	if (listen(socketServeur, 0) == SOCKET_ERROR)
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
		csock = accept(socketServeur, (SOCKADDR *)&csin, &sinsize);

		if (csock == INVALID_SOCKET)
		{
			perror("accept()");
			exit(0);
		}

		int pid = fork();
		if (pid == 0)
		{
			printf("connexion d'un client avec l'id %d \n", csock);
 			if(loginClient(csock)){

				readCommandClient(csock);
			 }else{
				 printf("\nlogin failed\n");
			 }
			printf("\nDéconexion du client %d\n", csock);
			exit(EXIT_SUCCESS);
		}
	}
	closesocket(socketServeur);
	closesocket(csock);
}
int main()
{

	signal(SIGINT, sig_handler);
	startServeur();
	return 0;
}
