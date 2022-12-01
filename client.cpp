
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <vector>

#define PORT 2908

#include "Network.h"
/* codul de eroare returnat de anumite apeluri */
extern int errno;
typedef struct thData{
    int idThread; //id-ul thread-ului tinut in evidenta de acest program
    int cl; //descriptorul intors de accept
    std::vector<Node>* connectedNodes;
}thData;
/* portul de conectare la server*/

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
int port;
int main (int argc, char *argv[]) {
    int sd;            // descriptorul de socket
    struct sockaddr_in server;    // structura folosita pentru conectare
    struct sockaddr_in from;
    int on = 1;

    Node currentNode;
    // mesajul trimis
    int nr = 0;
    char buf[255];
    pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
    std::vector<Node> connectedNodes;

    //exista toate argumentele in linia de comanda?

    if (argc != 3) {
        printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }

    //stabilim portul

    port = atoi(argv[2]);


    if (getIp().compare(argv[1]) > 0) {
        printf("Node connected to itself\n");
        currentNode.isFirstNode = currentNode.isSuperNode = true;
        currentNode.ip = inet_addr(argv[1]);
        currentNode.port = htons(port);


        //cream socketul
        if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("Eroare la socket().\n");
            return errno;
        }
        setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        //umplem structura folosita pentru realizarea conexiunii cu serverul

        //familia socket-ului

        server.sin_family = AF_INET;
// adresa IP a serverului

        server.sin_addr.s_addr = inet_addr(argv[1]);
        //portul de conectare

        server.sin_port = htons(port);

        //ne conectam la server

        if (listen(sd, 2) == -1) {
            perror("[server]Eroare la listen().\n");
            return errno;
        }
        /* servim in mod concurent clientii...folosind thread-uri */
        //pthread_create(&th[i], NULL, &interfata, NULL);
        //i++;
        int i = 0;
        while (1) {
            int client;
            thData *td; //parametru functia executata de thread
            int length = sizeof(from);

            printf("[server]Asteptam la portul %d...\n", PORT);
            fflush(stdout);

            // client= malloc(sizeof(int));
            /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
            if ((client = accept(sd, (struct sockaddr *) &from, reinterpret_cast<socklen_t *>(&length))) < 0) {
                perror("[server]Eroare la accept().\n");
                continue;
            }

            /* s-a realizat conexiunea, se astepta mesajul */

            // int idThread; //id-ul threadului
            // int cl; //descriptorul intors de accept

            td = (struct thData *) malloc(sizeof(struct thData));
            td->idThread = i++;
            td->cl = client;
            td->connectedNodes = &connectedNodes;
            Node newNode;
            newNode.ip = from.sin_addr.s_addr;
            newNode.port = from.sin_port;
            connectedNodes.push_back(newNode);
            pthread_create(&th[i], NULL, &treat, td);

        }//while
    } else {
/* stabilim portul */
        port = atoi(argv[2]);

        /* cream socketul */
        if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("[client] Eroare la socket().\n");
            return errno;
        }


        /* umplem structura folosita pentru realizarea conexiunii cu serverul */
        /* familia socket-ului */
        server.sin_family = AF_INET;
        /* adresa IP a serverului */
        server.sin_addr.s_addr = inet_addr(argv[1]);
        /* portul de conectare */
        server.sin_port = htons(port);

        /* ne conectam la server */
        if (connect(sd, (struct sockaddr *) &server, sizeof(struct sockaddr)) == -1) {
            perror("[client]Eroare la connect().\n");
            return errno;
        }

        /* citirea mesajului */
        /* bzero (msg, 100);
         printf ("[client]Introduceti un nume: ");
         fflush (stdout);
         read (0, msg, 100);

         *//* trimiterea mesajului la server *//*
        if (write (sd, msg, 100) <= 0)
        {
            perror ("[client]Eroare la write() spre server.\n");
            return errno;
        }

        *//* citirea raspunsului dat de server
           (apel blocant pina cind serverul raspunde) *//*
        if (read (sd, msg, 100) < 0)
        {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
        }
        *//* afisam mesajul primit *//*
        printf ("[client]Mesajul primit este: %s\n", msg);
*/
        /* inchidem conexiunea, am terminat */
        close(sd);




        //cream socketul
        if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("Eroare la socket().\n");
            return errno;
        }
        setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        //umplem structura folosita pentru realizarea conexiunii cu serverul

        //familia socket-ului

        server.sin_family = AF_INET;
// adresa IP a serverului

        server.sin_addr.s_addr = inet_addr(argv[1]);
        //portul de conectare

        server.sin_port = htons(port);

        //ne conectam la server

        if (listen(sd, 2) == -1) {
            perror("[server]Eroare la listen().\n");
            return errno;
        }
        /* servim in mod concurent clientii...folosind thread-uri */
        //pthread_create(&th[i], NULL, &interfata, NULL);
        //i++;
        int i = 0;
        while (1) {
            int client;
            thData *td; //parametru functia executata de thread
            int length = sizeof(from);

            printf("[server]Asteptam la portul %d...\n", PORT);
            fflush(stdout);

            // client= malloc(sizeof(int));
            /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
            if ((client = accept(sd, (struct sockaddr *) &from, reinterpret_cast<socklen_t *>(&length))) < 0) {
                perror("[server]Eroare la accept().\n");
                continue;
            }

            /* s-a realizat conexiunea, se astepta mesajul */

            // int idThread; //id-ul threadului
            // int cl; //descriptorul intors de accept

            td = (struct thData *) malloc(sizeof(struct thData));
            td->idThread = i++;
            td->cl = client;
            td->connectedNodes = &connectedNodes;
            Node newNode;
            newNode.ip = from.sin_addr.s_addr;
            newNode.port = from.sin_port;
            connectedNodes.push_back(newNode);
            pthread_create(&th[i], NULL, &treat, td);
        }
    }
}
static void *treat(void * arg)
{
    struct thData tdL;
    tdL= *((struct thData*)arg);
    printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
    fflush (stdout);
  /*  if (read (tdL.cl, utilizatorCurent,nr * sizeof(char)) <= 0)
    {
        printf("[Thread %d]\n",tdL.idThread);
        perror ("Eroare la read() de la client.\n");

    }*/
   // tdL.
    pthread_detach(pthread_self());
    //raspunde((struct thData*)arg);
    /* am terminat cu acest client, inchidem conexiunea */
    close ((intptr_t)arg);
    return(NULL);

};