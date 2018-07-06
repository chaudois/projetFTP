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
#define h_addr h_addr_list[0]
typedef int SOCKET;
const char *TARGET_IP = "localhost";
const int TARGET_PORT = 998;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;
SOCKET sock;
void login()
{
    char *message[512];
    int tailleRecue = 0;

    int carSent = send(sock, "BONJ\0", 512, 0);
    do
    {
        read(sock, message, sizeof(message));
    } while (!strstr(message, "WHO"));

    do
    {
        do
        {
            printf("\nlogin : ");
            gets(message);

        } while (message[0] == '\0' || message[0] == '\n');

        send(sock, message, 512, 0);
        do
        {

            read(sock, message, sizeof(message));
        } while (!strstr(message, "PASSWD"));
        do
        {
            printf("\npassword : ");
            gets(message);

        } while (message[0] == '\0' || message[0] == '\n');
        send(sock, message, 512, 0);

        read(sock, message, sizeof(message));
        printf("\n[%s]\n");
        if (strstr(message, "BYE"))
        {
            printf("\ntrop d'essais infructueux,deconnection\n");
            close(socket);
            exit(-1);
        }
        printf("\nutilisateur inconnu\n");
    } while (!strstr(message, "WELC"));
}
int main()
{
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        perror("socket()");
        exit(-1);
    }
    struct hostent *hostinfo = NULL;
    SOCKADDR_IN sin = {0};

    hostinfo = gethostbyname(TARGET_IP);
    if (hostinfo == NULL)
    {
        fprintf(stderr, "Unknown host %s.\n", TARGET_IP);
        exit(EXIT_FAILURE);
    }

    sin.sin_addr = *(IN_ADDR *)hostinfo->h_addr; /* l'adresse se trouve dans le champ h_addr de la structure hostinfo */
    sin.sin_port = htons(TARGET_PORT);           /* on utilise htons pour le port */
    sin.sin_family = AF_INET;

    if (connect(sock, (SOCKADDR *)&sin, sizeof(SOCKADDR)) == SOCKET_ERROR)
    {
        perror("connect()");
        exit(-1);
    }

    char *message = malloc(512);
    printf("connecté  sur l'adresse %s:%d\n", TARGET_IP, TARGET_PORT);

    message[0] = '\0';

    login();
    printf("\nlogin OK\n");
    do
    {

        gets(message);
        if (message[0] != '\0')
        {

            printf("\nsending [ %s ]\n", message);
            int carSent = send(sock, message, 512, 0);
        }

    } while (!strstr(message, "stop"));
}