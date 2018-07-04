#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;
const int PORT_ECOUTE=999;
void ReadXBytes(int socket, unsigned int x, void* buffer)
{
    int bytesRead = 0;
    int result;
    while (bytesRead < x)
    {
        result = read(socket, buffer + bytesRead, x - bytesRead);
        if (result < 1 )
        {
            // Throw your error.
        }

        bytesRead += result;
    }
}

void communicateWithClient(int socketClient)
{
	int continuer=1;
	while(continuer){
		char* dataRead;
		printf("le serveur écoute\n");
		recv(socketClient,dataRead,12,0);
		printf("data reçues : %s\n",dataRead);
		continuer=1;
	}
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
	printf("Le serveur écoute sur le port %d \n",PORT_ECOUTE);
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
			printf("connexion d'un client avec l'id %d. PID créé : %d\n",csock,pid);
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
