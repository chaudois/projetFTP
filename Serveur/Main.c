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
SOCKET socketServeur, socketDownl;
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
void removeResultatCommande()
{
	int pid = fork();
	if (pid == 0)
	{

		char *arguments[] = {"rm", "/resultatCommande.txt", NULL};

		if (execv("/bin/rm", arguments) == -1)
		{
			printf("\n---fuckedup---\n");
		}
		exit(0);
	}
	wait();
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
void diagnoseExecFail(int retourExec, int target)
{
	switch (retourExec)
	{
	case E2BIG:
		send(target, "The number of bytes in the new process's argument list is larger than the system-imposed limit", 2048, 0);
		break;
	case EACCES:
		send(target, "Search permission is denied for a component of the path prefix", 2048, 0);
		break;
	case EFAULT:
		send(target, "The new process file is not as long as indicated by the size values in its header", 2048, 0);
		break;
	case EIO:
		send(target, "An I/O error occurred while reading from the file system", 2048, 0);
		break;
	case ELOOP:
		send(target, "Too many symbolic links were encountered in translating the pathname. This is taken to be indicative of a looping symbolic link.", 2048, 0);
		break;
	case ENAMETOOLONG:
		send(target, "A component of a pathname exceeded {NAME_MAX} characters, or an entire path name exceeded {PATH_MAX} characters.", 2048, 0);
		break;
	case ENOENT:
		send(target, "The new process file does not exist.", 2048, 0);
		break;
	case ENOEXEC:
		send(target, "The new process file has the appropriate access permission, but has an unrecognized format (e.g., an invalid magic number in its header).", 2048, 0);
		break;
	case ENOMEM:
		send(target, "The new process requires more virtual memory than is allowed by the imposed maximum (getrlimit(2)).", 2048, 0);
		break;
	case ENOTDIR:
		send(target, " A component of the path prefix is not a directory.", 2048, 0);
		break;
	case ETXTBSY:
		send(target, "The new process file is a pure procedure (shared text) file that is currently open for writing or reading by some process.", 2048, 0);
		break;
	case EINVAL:
		send(target, "Invalid argument", 2048, 0);
		break;
	}
}
void readCommandClient(int socketClient)
{

	int tailleRecue = 0;
	do
	{
		char *message = malloc(512);
		tailleRecue = readClient(socketClient, message);

		char *commande = strtok(message, " ");
		char *parametres = strtok(NULL, "");
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

				int resultCommande = open("/resultatCommande.txt", O_CREAT | O_RDWR, 0666);
				dup2(resultCommande, STDOUT_FILENO);
				char *arguments[] = {"ls", parametres, NULL};
				close(resultCommande);

				if (execv("/bin/ls", arguments) == -1)
				{
					exit(errno);
				}
				exit(0);
			}
			int retourExec = 0;
			wait(&retourExec);
			retourExec = retourExec / 256;
			if (retourExec != 0)
			{
				removeResultatCommande();
				diagnoseExecFail(retourExec, socketClient);
			}
			else
			{

				int resultCommande = open("/resultatCommande.txt", O_RDONLY, 0666);

				char *fileContent = malloc(2048);
				int nblu = read(resultCommande, fileContent, 2048);

				close(resultCommande);
				removeResultatCommande();
				send(socketClient, fileContent, nblu, 0);
				free(fileContent);
			}
		}
		else if (strcmp(commande, "rpwd") == 0)
		{
			int pid = fork();
			if (pid == 0)
			{

				int resultCommande = open("/resultatCommande.txt", O_CREAT | O_RDWR, 0666);
				dup2(resultCommande, STDOUT_FILENO);
				char *arguments[] = {"pwd", parametres, NULL};

				if (execv("/bin/pwd", arguments) == -1)
				{
					exit(errno);
				}
				close(resultCommande);
				exit(0);
			}
			int retourExec = 0;
			wait(&retourExec);
			retourExec = retourExec / 256;
			if (retourExec != 0)
			{
				removeResultatCommande();
				diagnoseExecFail(retourExec, socketClient);
			}
			else
			{

				int resultCommande = open("/resultatCommande.txt", O_RDONLY, 0666);

				char *fileContent = malloc(2048);
				int nblu = read(resultCommande, fileContent, 2048);

				close(resultCommande);
				removeResultatCommande();
				send(socketClient, fileContent, nblu, 0);
				free(fileContent);
			}
		}
		else if (strcmp(commande, "downl") == 0)
		{
			int socketDownl = socket(AF_INET, SOCK_STREAM, 0);
			if (socketDownl == INVALID_SOCKET)
			{
				perror("socket()");
				exit(-1);
			}
			SOCKADDR_IN sin = {0};
			sin.sin_addr.s_addr = htonl(INADDR_ANY);
			sin.sin_family = AF_INET;
			int portDownl = 999;
			sin.sin_port = htons(portDownl);
			while (bind(socketDownl, (SOCKADDR *)&sin, sizeof sin) == SOCKET_ERROR)
			{
				portDownl = portDownl + 1;
				if (portDownl > 10000)
				{
					printf("\nAucun port ne permet de bind le socket : \n");
					perror("bind()");
					exit(0);
				}
				sin.sin_port = htons(portDownl);
			}
			char *commandRdy[512];
			strcpy(commandRdy, "RDY");

			strcat(commandRdy, " ");
			char tab[50];
			sprintf(tab, "%d", portDownl);

			strcat(commandRdy, tab);
			send(socketClient, commandRdy, 512, 0);
			if (listen(socketDownl, 0) == SOCKET_ERROR)
			{
				perror("listen()");
				exit(-1);
			}
			SOCKADDR_IN csin = {0};
			SOCKET csock;
			int sinsize = sizeof csin;
			csock = accept(socketDownl, (SOCKADDR *)&csin, &sinsize);

			int fileToTransert = open(parametres, O_RDONLY, 0666);
			char *chunk = malloc(2048);
			int lu = 0;

			do
			{
				lu = read(fileToTransert, chunk, 2048);
				send(csock, chunk, lu, 0);
			} while (lu > 2047);
			close(csock);
			close(fileToTransert);
			free(chunk);
		}
		else if (strcmp(commande, "upld") == 0)
		{
			char *nomFichier[512];
			strcpy(nomFichier, parametres);
			char *reponsePort[512];
			readClient(socketClient, reponsePort);
			commande = strtok(reponsePort, " ");
			char *parametresNew = strtok(NULL, "");
			int portNumDownl = 0;
			if (!strstr(commande, "RDY") || parametresNew == NULL)
			{
				printf("\nerreur : resultat incoherant\n");
				continue;
			}
			portNumDownl = atoi(parametresNew);
			int sockDownl = socket(AF_INET, SOCK_STREAM, 0);
			if (sockDownl == INVALID_SOCKET)
			{
				perror("socket()");
				continue;
			}
			struct hostent *hostinfo = NULL;
			SOCKADDR_IN sin = {0};

			hostinfo = gethostbyname("127.0.0.1");
			if (hostinfo == NULL)
			{
				fprintf(stderr, "Unknown host %s.\n", "127.0.0.1");
				continue;
			}
			sin.sin_addr = *(IN_ADDR *)hostinfo->h_addr; 
			sin.sin_port = htons(portNumDownl);			 
			sin.sin_family = AF_INET;

			if (connect(sockDownl, (SOCKADDR *)&sin, sizeof(SOCKADDR)) == SOCKET_ERROR)
			{
				perror("connect()");
				continue;
			}

			FILE *saveFileDownl = fopen(parametres, "w+");
			char *chunk = malloc(2048);
			int lu = 0;
			do
			{
				lu = read(sockDownl, chunk, 2048);

				fwrite(chunk, 1, lu, saveFileDownl);

			} while (lu == 2048);
			fclose(saveFileDownl);
			free(chunk);
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
	printf("\nLe serveur écoute sur le port %d \n", PORT_ECOUTE);
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
