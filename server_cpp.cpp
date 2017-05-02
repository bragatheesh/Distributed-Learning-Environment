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
#include <iostream>

#include "uthash.h"
#include "server.h"

#define BUFFSIZE 1024
#define MAX_THREADS 50


struct student* students = NULL;
struct course* courses = NULL;
struct instructor* instructors = NULL;
struct admin* admin1;

void
term(int signum){
	printf("\nCaught SIGINT\n");
    if(admin1 != NULL)
	    free(admin1);
	HASH_CLEAR(hh, students);
	HASH_CLEAR(hh, courses);
	HASH_CLEAR(hh, instructors);
	exit(1);
}

char* 
concat_two_strings(char* string_1, char* string_2){
	char *concat = (char*)malloc(strlen(string_1) + strlen(string_2) + 1);
	strcpy(concat, string_1);
	strcat(concat, string_2);
	return concat;
}

struct admin* 
init_admin(int id, std::string name, std::string password){
	struct admin* myAdmin = (struct admin*) malloc(sizeof(struct admin));
	myAdmin->name = name;
	myAdmin->password = password;
	myAdmin->id = id;

	return myAdmin;
}

struct student* 
init_student(int id, std::string name, std::string password){
	struct student* myStud;
	std::string uniq = name + password;
	HASH_FIND_STR(students, uniq.c_str(), myStud); 
	if (myStud != NULL) {
		printf("Student already exists\n");
		return NULL;
	}
	myStud = (struct student*) malloc(sizeof(struct student));
	myStud->name = name;
	myStud->password = password;
	myStud->uniq = uniq;
	myStud->id = id;
	HASH_ADD_KEYPTR(hh, students, myStud->uniq.c_str(), strlen(myStud->uniq.c_str()), myStud);
	return myStud;
}


struct instructor* 
init_instructor(int id, std::string name, std::string password){
	struct instructor* myInst;
	std::string uniq = name + password;
	HASH_FIND_STR(instructors, uniq.c_str(), myInst);
	if (myInst != NULL) {
		printf("Instructor already exists\n");
		return NULL;
	}
	myInst = (struct instructor*) malloc(sizeof(struct instructor));
	myInst->name = name;
	myInst->password = password;
	myInst->uniq = uniq;
	myInst->id = id;
	HASH_ADD_KEYPTR(hh, instructors, myInst->uniq.c_str(), strlen(myInst->uniq.c_str()), myInst);
	return myInst;
}

struct course* 
init_course(int id, std::string name, struct instructor &inst){
	struct course* myCourse;
	HASH_FIND_INT(courses, &id, myCourse);
	if (myCourse != NULL) {
		printf("Course already exists\n");
		return NULL;
	}
	myCourse = (struct course*) malloc(sizeof(struct course));
	myCourse->name = name;
	myCourse->id = id;
	myCourse->course_instructor = inst;
	HASH_ADD_INT(courses, id, myCourse);
	return myCourse;
}

struct student*
find_student(std::string name, std::string password){
	struct student* myStud;
	std::string uniq = name + password;
	HASH_FIND_STR(students, uniq.c_str(), myStud);  //Check if command is already present in hash
	if (myStud == NULL) {
		//printf("Student does not exist\n");
		return NULL;
	}
	else
		return myStud;
}

struct course*
find_course(int id){
struct course* myCourse;
	HASH_FIND_INT(courses, &id, myCourse);  //Check if command is already present in hash
	if (myCourse == NULL) {
		//printf("Student does not exist\n");
		return NULL;
	}
	else
		return myCourse;
}

struct instructor*
find_instructor(std::string name, std::string password){
	struct instructor* myInst;
	std::string uniq = name + password;
	HASH_FIND_STR(instructors, uniq.c_str(), myInst);  //Check if command is already present in hash
	if (myInst == NULL) {
		//printf("Instructor does not exist\n");
		return NULL;
	}
	else
		return myInst;
}

int
add_student_to_course(std::string studName, std::string studPass, int courseID){
	struct student* myStud = find_student(studName, studPass);
	if(myStud == NULL){
		printf("Error student %s %s does not exist\n", studName.c_str(), studPass.c_str());
		return 0;
	}

	struct course* myCourse = find_course(courseID);
	if(myCourse == NULL){
		printf("Error course with id %d does not exist\n", courseID);
		return 0;
	}

	HASH_ADD_KEYPTR(hh, myCourse->students, myStud->uniq.c_str(), strlen(myStud->uniq.c_str()), myStud);
	return 1;
}

void
student_handler(int client_sock, struct student* myStud){
    std::cout << "ID: " << myStud->id << " Name: " << myStud->name << std::endl;
	char client_message[BUFFSIZE];
	if (recv(client_sock, client_message, BUFFSIZE, 0) < 0){
			printf("Recvfrom failed in student_handler\n");
			close(client_sock);
			return;
	}
	
	if(!strcmp("VA", client_message)){
		printf("Client sent VA\n");
	}
	if(!strcmp("VG", client_message)){
		printf("Client sent VG\n");
	}
	if(!strcmp("ER", client_message)){
		printf("Client sent ER\n");
	}
	
}

void
instructor_handler(int client_sock, struct instructor* myInst){
    std::cout << "ID: " << myInst->id << " Name: " << myInst->name << std::endl;
}

void
admin_handler(int client_sock){
    std::cout << "ID: " << admin1->id << " Name: " << admin1->name << std::endl;
}

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
		char *name;
		char *password;
		char file[BUFFSIZE];
		char client_message[BUFFSIZE];

		bzero(client_message, BUFFSIZE);

		if (recv(client_sock, client_message, BUFFSIZE, 0) < 0){
			printf("Recvfrom failed\n");
			close(client_sock);
			pthread_exit(NULL);
		}
        if(client_message[0] == 0){
            term(0);
        }

		token = strtok(client_message, ",");

		if (!strcmp(token, "AUTHORIZE")){
			printf("%u Recvd AUTHORIZE\n", getpid());
			token = strtok(NULL, ",");
			name = token;
			token = strtok(NULL, ",");
			password = token;
			if(!strcmp(name, admin1->name.c_str()) && !strcmp(token, admin1->password.c_str())){
				ret = send(client_sock, "AUTHORIZED,ADMIN", strlen("AUTHORIZED,ADMIN"), 0);
				if (ret < 0){
					printf("Sending message failed\n");
					pthread_exit(NULL);
				}
				//test_program();
                admin_handler(client_sock);
			}

			struct student* myStud = find_student(std::string(name), std::string(password));
			struct instructor* myInst = find_instructor(std::string(name), std::string(password));
			if(myStud != NULL){
				ret = send(client_sock, "AUTHORIZED,STUDENT", strlen("AUTHORIZED,STUDENT"), 0);
                student_handler(client_sock, myStud);
			} 
			else if(myInst != NULL){
				ret = send(client_sock, "AUTHORIZED,INSTR", strlen("AUTHORIZED,INSTR"), 0);
                instructor_handler(client_sock, myInst);
			}
			else{
				ret = send(client_sock, "NOTAUTHORIZED", strlen("NOTAUTHORIZED"), 0);
				if (ret < 0){
					printf("Sending message failed\n");
					pthread_exit(NULL);
				}
			}
		}
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
    init_student(0, "brag", "bragpassword");
	init_student(1, "akshay", "ak");
	struct instructor* inst = init_instructor(0, "rod", "207");
	init_course(207, "Networking Apps", *inst);
	int result = add_student_to_course("brag", "bragpassword", 207);
	if(result == 0){
		printf("error addint student to course\n");
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

		client_sock = accept(server_sock, (struct sockaddr *)&client, 
				(socklen_t *)&sockaddr_size);
		if(client_sock < 0){
			printf("Error on accept\n");
			continue;
		}

		if(pthread_create(&tid, NULL, client_handler, (void*)&client_sock) < 0){
			printf("Thread creation failed\n");
		}
	}
}

