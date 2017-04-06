#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>

#define BUFFSIZE 1024
#define MAX_THREADS 50


void
term(int signum)
{
    printf("\nCaught SIGINT\n");
    pthread_exit(NULL);
}

void *client_handler(void* server_s)
{
    int client_sock = *(int*)server_s;
    while (1)
    {
        int fp;
        int ret = 0;
        long fsize;
        char *message;
        char *token;
        char file[BUFFSIZE];
        char client_message[BUFFSIZE];

        bzero(client_message, BUFFSIZE);

        if (recv(client_sock, client_message, BUFFSIZE, 0) < 0){
            printf("Recvfrom failed\n");
            close(client_sock);
            pthread_exit(NULL);
        }

        token = strtok(client_message, ",");

        if (!strcmp(token, "AUTHORIZE")){
            printf("%u Recvd AUTHORIZE\n", getpid());
        }

        ret = send(client_sock, "AUTHORIZED,STUDENT", strlen("AUTHORIZED,STUDENT"), 0);
        if (ret < 0){
            printf("Sending message failed\n");
            close(fp);
            pthread_exit(NULL);
        }
        /*
        fp = open(client_message, O_RDONLY);
        if (fp == 0){
            message = calloc(1, strlen("ERR_FILE_NEXISTS") + 1);
            sprintf(message, "ERR_FILE_NEXISTS");
            if (send(client_sock, message, strlen(message), 0) < 0){
                printf("Sending message failed\n");
                close(client_sock);
                return NULL;
            }
            pthread_exit(NULL);
        }

        memset(&file, 0, BUFFSIZE);

        while (1)
        {
            fsize = read(fp, file, BUFFSIZE);
            if (fsize > 0){
                ret = send(client_sock, file, fsize, 0);
                if (ret < 0){
                    printf("Sending message failed\n");
                    close(fp);
                    pthread_exit(NULL);
                }
                memset(&file, '\0', BUFFSIZE);
            }
            else{
                break;
            }
        }

        if (recv(client_sock, client_message, BUFFSIZE, 0) < 0){
            printf("Recvfrom failed\n");
            close(client_sock);
            pthread_exit(NULL);
        }

        if (!strcmp(client_message, "ACK")){
            printf("%u Client successfuly recvd file\n", getpid());
            pthread_exit(NULL);
        }

        close(fp);
        */
    }
}


int main(int argc, char *argv[])
{
    int port_number;
    int opt_val = 1;
    int sockaddr_size;
    int server_sock;
    int client_sock;
    pid_t pid;
    pthread_t thread_ids[MAX_THREADS];
    struct sockaddr_in server, client;
    struct sigaction action;

    if(argc < 2){   //check for correct number of args
        printf("Usage ./server PORT\n");
        return -1;
    }

    port_number = atoi(argv[1]);    //convert our argument to an integer port

    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = term;
    sigaction(SIGINT, &action, NULL);
    
    server_sock = socket(AF_INET, SOCK_STREAM, 0);   //create our tcp socket
    if(server_sock < 0){
        printf("Error creating server socket\n");
        return -1;
    }

    //enable reusable addresses option on our socket
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(int)) < 0){
        printf("setsockopt(SO_REUSEADDR) failed\n");
    }

    //populate the server sockaddr_in struct and store the port
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port_number);

    //bind our program to the socket and port
    if (bind(server_sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
	    printf("bind failled\n");
	    return -1;
    }

    listen(server_sock, 10);

    sockaddr_size = sizeof(client);

    while(1){
        pthread_t tid;
    
        client_sock = accept(server_sock, (struct sockaddr *)&client, &sockaddr_size);
        if(client_sock < 0){
            printf("Error on accept\n");
            continue;
        }
    
        if(pthread_create(&tid, NULL, client_handler, (void*)&client_sock) < 0){
            printf("Thread creation failed\n");
        }
    }
}

