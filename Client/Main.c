#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
#define h_addr h_addr_list[0]
typedef int SOCKET;
const int TARGET_PORT = 998;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;
SOCKET sock;
char *dir[512];
void sig_handler(int signo)
{
    if (signo == SIGINT)
    {
        printf("\nFermeture...\n");
        close(sock);
    }
    exit(0);
}
void diagnoseExecFail(int retourExec)
{
    switch (retourExec)
    {
    case E2BIG:
        printf("The number of bytes in the new process's argument list is larger than the system-imposed limit");
        break;
    case EACCES:
        printf("Search permission is denied for a component of the path prefix");
        break;
    case EFAULT:
        printf("The new process file is not as long as indicated by the size values in its header");
        break;
    case EIO:
        printf("An I/O error occurred while reading from the file system");
        break;
    case ELOOP:
        printf("Too many symbolic links were encountered in translating the pathname. This is taken to be indicative of a looping symbolic link.");
        break;
    case ENAMETOOLONG:
        printf("A component of a pathname exceeded {NAME_MAX} characters, or an entire path name exceeded {PATH_MAX} characters.");
        break;
    case ENOENT:
        printf("The new process file does not exist.");
        break;
    case ENOEXEC:
        printf("The new process file has the appropriate access permission, but has an unrecognized format (e.g., an invalid magic number in its header).");
        break;
    case ENOMEM:
        printf("The new process requires more virtual memory than is allowed by the imposed maximum (getrlimit(2)).");
        break;
    case ENOTDIR:
        printf(" A component of the path prefix is not a directory.");
        break;
    case ETXTBSY:
        printf("The new process file is a pure procedure (shared text) file that is currently open for writing or reading by some process.");
        break;
    }
}

int login()
{
    char *message[512];
    char *login[512];
    int tailleRecue = 0;

    int carSent = send(sock, "BONJ\0", 512, 0);
    do
    {
        read(sock, message, sizeof(message));
    } while (!strstr(message, "WHO"));

    while (1)
    {
        do
        {
            printf("\nlogin : ");
            gets(login);

        } while (login[0] == '\0' || login[0] == '\n');

        write(sock, login, 512);
        do
        {

            read(sock, message, sizeof(message));
        } while (!strstr(message, "PASSWD"));
        do
        {
            printf("\npassword : ");
            gets(message);

        } while (message[0] == '\0' || message[0] == '\n');
        write(sock, message, 512);

        read(sock, message, sizeof(message));
        if (strstr(message, "BYE"))
        {
            break;
        }
        if (strstr(message, "NOPE"))
        {
            printf("\nutilisateur inconnu\n");
        }
        else if (strstr(message, "WELC"))
        {
            system("clear");
            printf("\nconnecté en temps que [%s]\n", login);

            return 1;
        }
    }
    return 0;
}
void readCommandes(char *ipServeur)
{
    char *message = malloc(512);

    do
    {
        printf("\n>");
        gets(message);
        if (message[0] != '\0' && !strstr(message, "stop"))
        {
            char *commande = strtok(message, " ");
            char *parametres = strtok(NULL, "");
            if (strcmp(commande, "ls") == 0)
            {
                int pid = fork();
                if (pid == 0)
                {

                    char *arguments[] = {"ls", parametres, NULL};

                    if (execv("/bin/ls", arguments) == -1)
                    {
                        diagnoseExecFail(errno);
                    }
                    exit(0);
                }
                wait();
            }
            else if (strcmp(message, "pwd") == 0)
            {

                int pid = fork();
                if (pid == 0)
                {

                    char *arguments[] = {"pwd", parametres, NULL};

                    if (execv("/bin/pwd", arguments) == -1)
                    {
                        diagnoseExecFail(errno);
                    }
                    exit(0);
                }
                wait();
            }
            else if (strcmp(message, "cd") == 0)
            {
                char *totalCommande[512];
                strcpy(totalCommande, message);
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
                        printf("\naccès refusé. Lancez le client en mode administrateur\n");
                        break;
                    case ENOENT:
                        printf("\nCe repertoire n'existe pas\n");
                        break;
                    }
                }
            }
            else if (strcmp(message, "rm") == 0)
            {

                int pid = fork();
                if (pid == 0)
                {

                    char *arguments[] = {"rm", parametres, NULL};

                    if (execv("/bin/rm", arguments) == -1)
                    {
                        diagnoseExecFail(errno);
                    }
                    exit(0);
                }
                wait();
            }
            else if (strcmp(message, "rls") == 0)
            {
                char *totalCommande[512];
                strcpy(totalCommande, message);
                if (parametres != NULL)
                {

                    strcat(totalCommande, " ");
                    strcat(totalCommande, parametres);
                }
                write(sock, totalCommande, 512);
                char *retour[2048];
                int lu = read(sock, retour, 2048);
                char *total = malloc(lu + 1);
                strncpy(total, retour, lu);
                printf("\n%s\n", total);
            }
            else if (strcmp(message, "rcd") == 0)
            {

                char *totalCommande[512];
                strcpy(totalCommande, message);
                if (parametres != NULL)
                {

                    strcat(totalCommande, " ");
                    strcat(totalCommande, parametres);
                }
                write(sock, totalCommande, 512);
                char *retour[512];
                read(sock, retour, 512);
                if (!strstr(retour, "OK"))
                {
                    printf("\n[%s]\n", retour);
                }
            }
            else if (strcmp(message, "rpwd") == 0)
            {
                char *totalCommande[512];
                strcpy(totalCommande, message);
                if (parametres != NULL)
                {

                    strcat(totalCommande, " ");
                    strcat(totalCommande, parametres);
                }
                write(sock, totalCommande, 512);
                char *retour[2048];
                int lu = read(sock, retour, 2048);
                char *total = malloc(lu + 1);
                strncpy(total, retour, lu);
                printf("\n%s\n", total);
            }
            else if (strcmp(message, "upld") == 0)
            {
                char *totalCommande[512];
                strcpy(totalCommande, message);
                if (parametres != NULL)
                {
                    strcat(totalCommande, " ");
                    strcat(totalCommande, parametres);
                }
                else
                {
                    printf("veuillez préciser quelle fichier télécharger\n");
                    exit(0);
                }
                send(sock, totalCommande, 512, 0);
                int socketUpld = socket(AF_INET, SOCK_STREAM, 0);
                if (socketUpld == INVALID_SOCKET)
                {
                    perror("socket()");
                    exit(-1);
                }
                SOCKADDR_IN sin = {0};
                sin.sin_addr.s_addr = htonl(INADDR_ANY);
                sin.sin_family = AF_INET;
                int portDownl = 999;
                sin.sin_port = htons(portDownl);
                while (bind(socketUpld, (SOCKADDR *)&sin, sizeof sin) == SOCKET_ERROR)
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
                send(sock, commandRdy, 512, 0);
                if (listen(socketUpld, 0) == SOCKET_ERROR)
                {
                    perror("listen()");
                    exit(-1);
                }
                SOCKADDR_IN csin = {0};
                SOCKET ssock;
                int sinsize = sizeof csin;
                ssock = accept(socketUpld, (SOCKADDR *)&csin, &sinsize);

                int fileToTransert = open(parametres, O_RDONLY, 0666);
                char *chunk = malloc(2048);
                int lu = 0;

                do
                {
                    lu = read(fileToTransert, chunk, 2048);
                    send(ssock, chunk, lu, 0);
                } while (lu > 2047);
                close(ssock);
                close(fileToTransert);
                free(chunk);
            }
            else if (strcmp(message, "downl") == 0)
            {
                char *totalCommande[512];
                strcpy(totalCommande, message);
                if (parametres != NULL)
                {
                    strcat(totalCommande, " ");
                    strcat(totalCommande, parametres);
                }
                else
                {
                    printf("veuillez préciser quelle fichier télécharger\n");
                    exit(0);
                }
                write(sock, totalCommande, 512);
                char *reponsePort[512];
                read(sock, reponsePort, 512);
                char *commande = strtok(reponsePort, " ");
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

                hostinfo = gethostbyname(ipServeur);
                if (hostinfo == NULL)
                {
                    fprintf(stderr, "Unknown host %s.\n", ipServeur);
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
                } while (lu > 0);
                fclose(saveFileDownl);
                free(chunk);
            }
            else
            {

                printf("commande inconnue : [%s]", message);
            }
        }

    } while (!strstr(message, "stop"));
    free(message);
}
int connection(char *ipserveur)
{
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        perror("socket()");
        return 0;
    }
    struct hostent *hostinfo = NULL;
    SOCKADDR_IN sin = {0};

    hostinfo = gethostbyname(ipserveur);
    if (hostinfo == NULL)
    {
        fprintf(stderr, "Unknown host %s.\n", ipserveur);
        return 0;
    }

    sin.sin_addr = *(IN_ADDR *)hostinfo->h_addr;
    sin.sin_port = htons(TARGET_PORT);
    sin.sin_family = AF_INET;

    if (connect(sock, (SOCKADDR *)&sin, sizeof(SOCKADDR)) == SOCKET_ERROR)
    {
        perror("connect()");
        return 0;
    }

    printf("connecté  sur l'adresse %s:%d\n", ipserveur, TARGET_PORT);
    return 1;
}
int main()
{
    system("clear");
    signal(SIGINT, sig_handler);
    char *ip = malloc(20);
    do
    {
        printf("\nip du serveur?\n");
        gets(ip);

    } while (!connection(ip));

        if (!login())
    {
        printf("\ntrop d'essais infructueux,deconnection\n");
        exit(0);
    }
    readCommandes(ip);
}