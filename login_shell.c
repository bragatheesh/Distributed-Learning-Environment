#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

#define BUFFSIZE 1024

int server_sock;

int init_connection_to_server(char* hostname, char* port){
    int sockaddr_size;

    int ret = 0;
    int size = 0;
    int port_number;
    struct sockaddr_in server;
    struct hostent *host;
    struct timeval timeout;

    port_number = atoi(port);

    host = gethostbyname(hostname);
    if(host == NULL){
        printf("Host %s could not be resolved\n", hostname);
        return -1;
    }

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(server_sock < 0){
        printf("Unable to create a socket\n");
        return -1;
    }

    /*timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    if (setsockopt (server_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0){
        printf("setsockopt(SO_RCVTIMEO) failed\n");
    }*/

    sockaddr_size = sizeof(server);
    //zero out the sockaddr_in struct in preparation of storing connection info
    bzero(&server, sockaddr_size);
    server.sin_family = AF_INET;                                                    //specify internet family (IPV4)
    bcopy((char *)host->h_addr, (char *)&server.sin_addr.s_addr, host->h_length);   //copy the resolved ip address into the struct
    server.sin_port = htons(port_number);                                           //copy the port into the struct

    //connect to the server
    if(connect(server_sock, (struct sockaddr*)&server, sockaddr_size) < 0){
        printf("Failed to connect to %s\n", hostname);
        close(server_sock);
        return -1;
    }

    return 1;
}

int main(int argc, char** argv){
    char userName[256];
    //char password[256];
    char* password;
    char buffer[BUFFSIZE];
    char* token;
    int ret;


    if(argc < 3){
        printf("./login_shell hostname port\n");
        return -1;
    }
    
    ret = init_connection_to_server(argv[1], argv[2]);
    if(ret != 1){
        printf("Error initializing connection to Server %s on port %s\nExiting.", argv[1], argv[2]);
        return -1;
    }

    while (1){

        memset(userName, '\0', 256);
        //memset(password, '\0', 256);
        memset(buffer, '\0', BUFFSIZE);
        printf("Welcome to the School Portal\nUsername: ");
        scanf("%s", userName);
        //printf("Password: ");
        //scanf("%s", password);
        password = getpass("Password: ");

        //send username and password to server and wait for authorization
        //confirmation and authorization type

        sprintf(buffer, "AUTHORIZE,%s,%s", userName, password);

        if (send(server_sock, buffer, strlen(buffer), 0) < 0){
            printf("Could not Authorize\n");
            close(server_sock);
            return -1;
        }

        memset(&buffer, '\0', BUFFSIZE);
        ret = recv(server_sock, buffer, BUFFSIZE, 0);
        if (ret < 0){
            printf("Could not Authorize\n");
            close(server_sock);
            return -1;
        }
        token = strtok(buffer, ",");
        if (!strcmp(token, "AUTHORIZED")){
            printf("Welcome %s\n", userName);
            token = strtok(NULL, ",");
            if (!strcmp(token, "ADMIN")){
                //pass information to GUI
                printf("%s is a Admin\n", userName);
            }
            else if (!strcmp(token, "INSTR")){
                //pass info to GUI
                printf("%s is a Instructor\n", userName);
            }
            else if (!strcmp(token, "STUDENT")){
                //pass info to GUI
                printf("%s is a Student\n", userName);
            }
        }
        else if(!strcmp(token, "NOTAUTHORIZED")){
            printf("Invalid username or password\n");
        }
        else{
            printf("Unrecognized username and password\n");
        }
    }
    return 0;
}