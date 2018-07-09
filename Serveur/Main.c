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
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;
const int PORT_ECOUTE = 998;
const int taillemessage = 512;
SOCKET socketServeur;
int readClient(int socket, char *message)
{
	memset(message, '0', taillemessage);
	int lu = read(socket, message, taillemessage);
	return lu;
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
int logUser(char *login, char *password)
{
	FILE *saveFile = fopen("saveFile.txt", "r+");
	char *ligne[taillemessage];
	while (fgets(ligne, taillemessage, saveFile) > 0)
	{
		char *loginLigne = strtok(ligne, " ");
		char *passwordLigne = strtok(NULL, "\n");
		if (strcmp(loginLigne, login) == 0 && strcmp(passwordLigne, password) == 0)
		{
			return 1;
		}
	}

	close(saveFile);
	return 0;
}
int loginClient(int socketClient)
{
	int loginok = 0;
	int cptTry = 0;
	char message[taillemessage];
	char *login[taillemessage], password[taillemessage];
	readClient(socketClient, message);

	if (strstr(message, "BONJ"))
	{

		while (!loginok && cptTry < 3)
		{

			if (write(socketClient, "WHO", taillemessage) > 0)
			{

				readClient(socketClient, login);
			}
			else
			{
				closeClient(socketClient);
			}
			if (write(socketClient, "PASSWD\0", taillemessage) > 0)
			{
				readClient(socketClient, password);
				cptTry = cptTry + 1;

				if (logUser(login, password))
				{
					write(socketClient, "WELC", taillemessage);
					return 1;
				}
				else if (cptTry > 2)
				{
					write(socketClient, "BYE", taillemessage);
					closeClient(socketClient);
				}
				else
				{

					write(socketClient, "NOPE", taillemessage);
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

	int tailleRecue = 0;
	do
	{
		char message[1024];
		memset(message, '0', sizeof(message));
		tailleRecue = readClient(socketClient, message);

		char *commande = strtok(message, " ");
		char *parametres = strtok(NULL, "");

		printf("\n(%d)>[%s]+[%s]\n", socketClient, commande,parametres);
		if (strcmp(commande, "rcd") == 0)
		{
			char *totalCommande[512];
			strcpy(totalCommande, "cd");
			if (parametres != NULL)
			{

				strcat(totalCommande, " ");
				strcat(totalCommande, parametres);
			}
			if (chdir(parametres) == -1)
			{
				switch (errno)
				{
				case EACCES:
					send(socketClient, "accès refusé. Lancez le client en mode administrateur", 512, 0);
					break;
				case ENOENT:
					send(socketClient, "Ce repertoire n'existe pas", 512, 0);

					break;
				}
			}
			else
			{
				send(socketClient, "OK", 512, 0);
			}
		}
		else if (strcmp(commande, "rls") == 0)
		{
			int pid = fork();
			if (pid == 0)
			{
				char *totalCommande[512];
				strcpy(totalCommande, "ls");
				if (parametres != NULL)
				{

					strcat(totalCommande, " ");
					strcat(totalCommande, parametres);
				}

				int resultCommande = open("resultatCommande.txt", O_CREAT | O_RDWR, 0666);
				dup2(resultCommande, STDOUT_FILENO);
				system(totalCommande);
				lseek(resultCommande, 0, 0);
				char fileContent[2048];
				int nblu = read(resultCommande, fileContent, 2048);
				close(resultCommande);
				// system("rm resultatCommande.txt");
				send(socketClient, fileContent, 2048, 0);
				exit(0);
			}
			wait();
		}
		else if (strcmp(commande, "rpwd") == 0)
		{
 			int pid = fork();
			if (pid == 0)
			{
				char *totalCommande[512];
				strcpy(totalCommande, "pwd");
				if (parametres != NULL)
				{

					strcat(totalCommande, " ");
					strcat(totalCommande, parametres);
				}


				int resultCommande = open("resultatCommande.txt", O_CREAT | O_RDWR, 0666);
				dup2(resultCommande, STDOUT_FILENO);
				system(totalCommande);
				lseek(resultCommande, 0, 0);
				char fileContent[2048];
				int nblu = read(resultCommande, fileContent, 2048);
				close(resultCommande);
				// system("rm resultatCommande.txt");
				send(socketClient, fileContent, 2048, 0);
				exit(0);
			}
			wait();
		}
	} while (tailleRecue > 0);
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
			if (loginClient(csock))
			{

				readCommandClient(csock);
			}
			else
			{
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
