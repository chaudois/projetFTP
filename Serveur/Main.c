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

int main()
{
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		perror("socket()");
		exit(-1);
	}
	printf("Socket créé : %d\n", sock);
	SOCKADDR_IN sin = {0};

	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	sin.sin_family = AF_INET;

	sin.sin_port = htons(999);

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
		printf("connexion entrante : socket n° %d\n", csock);
	}
	closesocket(sock);
	closesocket(csock);

	return 0;
}
