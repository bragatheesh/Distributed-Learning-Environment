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

#include "uthash.h"
#include "server.h"

#define BUFFSIZE 1024
#define MAX_THREADS 50


struct student* students = NULL;
struct course* courses = NULL;
struct instructor* instructors = NULL;
struct admin* admin1;

void
term(int signum)
{
    printf("\nCaught SIGINT\n");
    free(admin1);
    pthread_exit(NULL);
}

char* 
concat_two_strings(char* string_1, char* string_2)
{
    char *concat = malloc(strlen(string_1) + strlen(string_2) + 1);
    strcpy(concat, string_1);
    strcat(concat, string_2);
    return concat;
}

/*int hash(char* name, char* password, int size){
    int i;
    int name_size = strlen(name);
    int password_size = strlen(password);
    int hash = 0;
    for(i = 0; i < name_size; i++){
        hash += (int)name[i];
    }

    for(i = 0; i < password_size; i++){
        hash += (int)password[i];
    }

    return hash % size;
}*/




struct admin* 
init_admin(int id, char* name, char* password){
    struct admin* myAdmin = malloc(sizeof(struct admin));
    myAdmin->name = malloc(strlen(name));
    memset(myAdmin->name, '\0', strlen(name));
    myAdmin->name = name;
    myAdmin->password = malloc(strlen(password));
    memset(myAdmin->password, '\0', strlen(password));
    myAdmin->password = password;
    myAdmin->id = id;

    return myAdmin;
}

struct student* 
init_student(int id, char* name, char* password){
    struct student* myStud;// = malloc(sizeof(struct student));
    char* uniq = concat_two_strings(name, password);
    HASH_FIND_STR(students, uniq, myStud);  //Check if command is already present in hash
    if (myStud != NULL) {
        printf("Student already exists\n");
        return NULL;
    }
    myStud = malloc(sizeof(struct student));
    myStud->name = malloc(strlen(name));
    memset(myStud->name, '\0', strlen(name));
    myStud->name = name;
    myStud->password = malloc(strlen(password));
    memset(myStud->password, '\0', strlen(password));
    myStud->password = password;
    myStud->uniq = uniq;
    myStud->id = id;
    HASH_ADD_KEYPTR(hh, students, myStud->uniq, strlen(myStud->uniq),
                                                            myStud);
    return myStud;
}


/*struct instructor* 
init_instructor(int id, char* name, char* password){
    struct instructor* myInst;// = malloc(sizeof(struct student));
    char* uniq = concat_two_strings(name, password);
    HASH_FIND_STR(instructors, uniq, myInst);  //Check if command is already present in hash
    if (myInst != NULL) {
        printf("Instructor already exists\n");
        return NULL;
    }
    myInst = malloc(sizeof(struct instructor));
    myInst->name = malloc(strlen(name));
    memset(myInst->name, '\0', strlen(name));
    myInst->name = name;
    myInst->password = malloc(strlen(password));
    memset(myInst->password, '\0', strlen(password));
    myInst->password = password;
    myInst->uniq = uniq;
    myInst->id = id;
    HASH_ADD_KEYPTR(hh, instructors, myInst->uniq, strlen(myInst->uniq),
                                                                myInst);
    return myInst;
}

struct course* 
init_course(int id, char* name, struct instructor &inst){
    struct course* myCourse;// = malloc(sizeof(struct student));
    //char* uniq = concat_two_strings(name, password);
    HASH_FIND_STR(courses, id, myCourse); //Check if command is already present in hash
    if (myInst != NULL) {
        printf("Course already exists\n");
        return NULL;
    }
    myCourse = malloc(sizeof(struct course));
    myCourse->name = malloc(strlen(name));
    memset(myCourse->name, '\0', strlen(name));
    myCourse->name = name;
    myCourse->id = id;
    HASH_ADD_KEYPTR(hh, courses, myCourse->id, sizeof(myCourse->id),
                                                                myInst);
    return myCourse;
}*/


void* 
client_handler(void* server_s)
{
    int client_sock = *(int*)server_s;
    while (1)
    {
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
            token = strtok(NULL, ",");
            if(!strcmp(token, admin1->name)){
                token = strtok(NULL, ",");
                if(!strcmp(token, admin1->password)){
                    ret = send(client_sock, "AUTHORIZED,ADMIN",
                                    strlen("AUTHORIZED,ADMIN"), 
                                                            0);
                    if (ret < 0){
                        printf("Sending message failed\n");
                        pthread_exit(NULL);
                    }
                }
            }
            else{
		
                ret = send(client_sock, "NOTAUTHORIZED", 
                            strlen("NOTAUTHORIZED"), 0);
                if (ret < 0){
                    printf("Sending message failed\n");
                    pthread_exit(NULL);
                }
            }
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


int 
main(int argc, char *argv[])
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

     admin1 = init_admin(0, "brag", "root");
     //printf("Admin data: %d, %s, %s\n", admin1->id, admin1->name, admin1->password);
     //int has = hash("sdjhasdh", "skjdhajksd", 10);
     //printf("Admin1 hashed to %d\n", has);

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
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt_val,
                                                sizeof(int)) < 0){
        printf("setsockopt(SO_REUSEADDR) failed\n");
    }

    //populate the server sockaddr_in struct and store the port
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port_number);

    //bind our program to the socket and port
    if (bind(server_sock, (struct sockaddr*)&server, 
                                sizeof(server)) < 0) {
	    printf("bind failled\n");
	    return -1;
    }

    listen(server_sock, 10);

    sockaddr_size = sizeof(client);

    while(1){
        pthread_t tid;
    
        client_sock = accept(server_sock, (struct sockaddr *)&client, (socklen_t *)&sockaddr_size);
        if(client_sock < 0){
            printf("Error on accept\n");
            continue;
        }
    
        if(pthread_create(&tid, NULL, client_handler, (void*)&client_sock) < 0){
            printf("Thread creation failed\n");
        }
    }
}

