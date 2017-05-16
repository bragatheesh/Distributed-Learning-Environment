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
#include "c_server.h"

#define BUFFSIZE 1024
#define MAX_THREADS 50

void test_program();

struct student* students = NULL;
struct course* courses = NULL;
struct instructor* instructors = NULL;
struct admin* admin1;

void
term(int signum){
	printf("\nCaught SIGINT\n");
    if(admin1 != NULL)
	    free(admin1);

	//HASH_CLEAR(hh, courses);
	//HASH_CLEAR(hh, instructors);
	//HASH_CLEAR(hh, students);
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
init_admin(int id, char* name, char* password){
	struct admin* myAdmin = (struct admin*) malloc(sizeof(struct admin));
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
	struct student* myStud;
	char* uniq = concat_two_strings(name, password);
	HASH_FIND_STR(students, uniq, myStud);
	if (myStud != NULL) {
		printf("Student already exists\n");
		return NULL;
	}
	myStud = (struct student*) malloc(sizeof(struct student));
	myStud->name = malloc(strlen(name));
	memset(myStud->name, '\0', strlen(name));
	myStud->name = name;
	myStud->password = malloc(strlen(password));
	memset(myStud->password, '\0', strlen(password));
	myStud->password = password;
	myStud->uniq = uniq;
	myStud->id = id;
	HASH_ADD_KEYPTR(hh, students, uniq, strlen(uniq), myStud);
	return myStud;
}


struct instructor* 
init_instructor(int id, char* name, char* password){
	struct instructor* myInst;
	char* uniq = concat_two_strings(name, password);
	HASH_FIND_STR(instructors, uniq, myInst);
	if (myInst != NULL) {
		printf("Instructor already exists\n");
		return NULL;
	}
	myInst = (struct instructor*) malloc(sizeof(struct instructor));
	myInst->name = malloc(strlen(name));
	memset(myInst->name, '\0', strlen(name));
	myInst->name = name;
	myInst->password = malloc(strlen(password));
	memset(myInst->password, '\0', strlen(password));
	myInst->password = password;
	myInst->uniq = uniq;
	myInst->id = id;
	HASH_ADD_KEYPTR(hh, instructors, myInst->uniq, strlen(myInst->uniq), myInst);
	return myInst;
}

struct course* 
init_course(int id, char* name, struct instructor *inst){
	struct course* myCourse;
	HASH_FIND_INT(courses, &id, myCourse);
	if (myCourse != NULL) {
		printf("Course already exists\n");
		return NULL;
	}
	myCourse = (struct course*) malloc(sizeof(struct course));
	myCourse->name = malloc(strlen(name));
	memset(myCourse->name, '\0', strlen(name));
	myCourse->name = name;
	myCourse->id = id;
	myCourse->course_instructor = *inst;
	
	HASH_ADD_INT(courses, id, myCourse);
	HASH_ADD_INT(inst->instructor_courses, id, myCourse);
	return myCourse;
}

struct student*
find_student(char* name, char* password){
	struct student* myStud;
	char* uniq = concat_two_strings(name, password);
	HASH_FIND_STR(students, uniq, myStud);  //Check if command is already present in hash
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
find_instructor(char* name, char* password){
	struct instructor* myInst;
	char* uniq = concat_two_strings(name, password);
	HASH_FIND_STR(instructors, uniq, myInst);  //Check if command is already present in hash
	if (myInst == NULL) {
		//printf("Instructor does not exist\n");
		return NULL;
	}
	else
		return myInst;
}

int
add_assignment_to_course(int id, char* assignment_name, int value){
	struct course* myCourse = find_course(id);
	if(myCourse == NULL){
		printf("Error no such course\n");
		return -1;
	}

	struct assignment* myAssignment;
	HASH_FIND_STR(myCourse->course_assignments, assignment_name, myAssignment);  //Check if command is already present in hash
	if (myAssignment != NULL) {
		printf("Assignment already exists");
		return -2;
	}

	myAssignment = malloc(sizeof(struct assignment*));
	myAssignment->name = malloc(strlen(assignment_name));
	memset(myAssignment, '\0', strlen(assignment_name));
	myAssignment->name = assignment_name;
	myAssignment->value = value;

	HASH_ADD_KEYPTR(hh, myCourse->course_assignments, myAssignment->name, strlen(myAssignment->name), myAssignment);
	return 1;

}

int
add_student_to_course(char* studName, char* studPass, int courseID){
	struct student* myStud = find_student(studName, studPass);
	if(myStud == NULL){
		printf("Error student %s %s does not exist\n", studName, studPass);
		return 0;
	}

	struct course* myCourse = find_course(courseID);
	if(myCourse == NULL){
		printf("Error course with id %d does not exist\n", courseID);
		return 0;
	}

	HASH_ADD_KEYPTR(hh, myCourse->course_students, myStud->uniq, strlen(myStud->uniq), myStud);
	HASH_ADD_INT(myStud->student_courses, id, myCourse);
	return 1;
}

int
grade_student(char* studName, char* studPass, int courseID, char* assignmentName, int grade){
	struct student* myStud = find_student(studName, studPass);
	if(myStud == NULL){
		printf("Error student %s %s does not exist\n", studName, studPass);
		return -1;
	}

	struct course* myCourse;
	HASH_FIND_INT(myStud->student_courses, &courseID, myCourse);  //Check if command is already present in hash
	if (myCourse == NULL) {
		//printf("Student does not exist\n");
		return -1;
	}
	struct assignment* ass, *tmpass;
	HASH_ITER(hh, myCourse->course_assignments, ass, tmpass){
		if(!strcmp(ass->name, assignmentName)){
			ass->grade = grade;
			return 1;
		}
	}
	return -1;
}

void
student_handler(int client_sock, struct student* myStud){
	printf("ID: %d  Name: %s\n", myStud->id, myStud->name);
	char client_message[BUFFSIZE];
	bzero(client_message, BUFFSIZE);
	if (recv(client_sock, client_message, BUFFSIZE, 0) < 0){
			printf("Recvfrom failed in student_handler\n");
			close(client_sock);
			return;
	}
	
	if(!strcmp("VA", client_message)){
		printf("Client sent VA\n");
		struct assignment* ass, *tmpass;
		struct course* myCourse, *tmpcourse;
		char buffer[4096];
		int ret;

		memset(buffer, '\0', 4096);
		HASH_ITER(hh, myStud->student_courses, myCourse, tmpcourse){
			printf("\tCourse ID: %d Name: %s \n", myCourse->id, myCourse->name);
			sprintf(buffer, "\tCourse ID: %d Name: %s \n", myCourse->id, myCourse->name);
			ret = send(client_sock, buffer, strlen(buffer), 0);
			if (ret < 0){
				printf("Sending message failed\n");
				pthread_exit(NULL);
			}
			memset(buffer, '\0', 4096);
			HASH_ITER(hh, myCourse->course_assignments, ass, tmpass){
				printf("\t\t\tName: %s Value: %d\n",ass->name, ass->value);
				sprintf(buffer, "\t\t\tName: %s Value: %d\n",ass->name, ass->value);
				ret = send(client_sock, buffer, strlen(buffer), 0);
				
				if (ret < 0){
					printf("Sending message failed\n");
					pthread_exit(NULL);
				}
				memset(buffer, '\0', 4096);
			}
		}
		//printf("buffer: %s\n", buffer);
	}
	if(!strcmp("VG", client_message)){
		printf("Client sent VG\n");
		struct assignment* ass, *tmpass;
		struct course* myCourse, *tmpcourse;
		char buffer[4096];
		int ret;

		HASH_ITER(hh, myStud->student_courses, myCourse, tmpcourse){
			memset(buffer, '\0', 4096);
			sprintf(buffer, "\tCourse ID: %d Name: %s \n", myCourse->id, myCourse->name);
			printf("\tCourse ID: %d Name: %s \n", myCourse->id, myCourse->name);
			
			ret = send(client_sock, buffer, strlen(buffer), 0);
			if (ret < 0){
			    printf("Sending message failed\n");
			    pthread_exit(NULL);
			}
			HASH_ITER(hh, myCourse->course_assignments, ass, tmpass){
				memset(buffer, '\0', 4096);
				sprintf(buffer, "\t\t\tName: %s Value: %d Grade: %d\n",ass->name, ass->value, ass->grade);
							
				ret = send(client_sock, buffer, strlen(buffer), 0);
				if (ret < 0){
			   		printf("Sending message failed\n");
			    	pthread_exit(NULL);
				}
				printf("\t\t\tName: %s Value: %d Grade: %d\n",ass->name, ass->value, ass->grade);
			}
		}
	}
	if(!strcmp("ER", client_message)){
		printf("Client sent ER\n");
		test_program();
	}
	
}

void
instructor_handler(int client_sock, struct instructor* myInst)
{
	printf("Welcome instructor %s\n", myInst->name);
	while (1){
	    printf("ID: %d  Name: %s\n", myInst->id, myInst->name);
	    char client_message[BUFFSIZE];
	    bzero(client_message, BUFFSIZE);
	    if (recv(client_sock, client_message, BUFFSIZE, 0) < 0){
			printf("Recvfrom failed in instructor_handler\n");
			close(client_sock);
			return;
	    }

		if(!strcmp("VA", client_message)){
			struct assignment* ass, *tmpass;
			struct course* myCourse, *tmpcourse;
			char buffer[4096];
			int ret;
			
			HASH_ITER(hh, myInst->instructor_courses, myCourse, tmpcourse){
			printf("\tCourse ID: %d Name: %s \n", myCourse->id, myCourse->name);
			sprintf(buffer, "\tCourse ID: %d Name: %s \n", myCourse->id, myCourse->name);
			ret = send(client_sock, buffer, strlen(buffer), 0);
			if (ret < 0){
				printf("Sending message failed\n");
				pthread_exit(NULL);
			}
			memset(buffer, '\0', 4096);
			HASH_ITER(hh, myCourse->course_assignments, ass, tmpass){
				printf("\t\t\tName: %s Value: %d\n",ass->name, ass->value);
				sprintf(buffer, "\t\t\tName: %s Value: %d\n",ass->name, ass->value);
				ret = send(client_sock, buffer, strlen(buffer), 0);
				if (ret < 0){
					printf("Sending message failed\n");
					pthread_exit(NULL);
				}
				memset(buffer, '\0', 4096);
			}
		}
		}
		else if(!strcmp("AA", client_message)){
			
		}
		else if(!strcmp("VG", client_message)){
			
		}	
		else if(!strcmp("AA", client_message)){
			
		}
		else{
			//error here
		}

	}
    
}

void
admin_handler(int client_sock){
    printf("Welcome admin\n");
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
			if(!strcmp(name, admin1->name) && !strcmp(token, admin1->password)){
				ret = send(client_sock, "AUTHORIZED,ADMIN", strlen("AUTHORIZED,ADMIN"), 0);
				if (ret < 0){
					printf("Sending message failed\n");
					pthread_exit(NULL);
				}
				//test_program();
                //admin_handler(client_sock);
			}

			struct student* myStud = find_student(name, password);
			struct instructor* myInst = find_instructor(name, password);
			if(myStud != NULL){
				ret = send(client_sock, "AUTHORIZED,STUDENT", strlen("AUTHORIZED,STUDENT"), 0);
                student_handler(client_sock, myStud);
			} 
			else if(myInst != NULL){
				ret = send(client_sock, "AUTHORIZED,INSTR", strlen("AUTHORIZED,INSTR"), 0);
                //instructor_handler(client_sock, myInst);
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

void
test_program(){
	struct instructor* inst;
	struct student* c, *tmp;
	struct course* myCourse;
	struct assignment* ass, *tmpass;

	printf("number of students: %d\n", HASH_COUNT(students));
	printf("number of instructors: %d\n", HASH_COUNT(instructors));
	printf("number of courses: %d\n", HASH_COUNT(courses));

    printf("\nprinting students:\n");
	/*for(c = students; c != NULL; c = (struct student*) c->hh.next){
		printf("ID: %d Name: %s\n",c->id, c->name);
	}*/
	HASH_ITER(hh, students, c, tmp){
		printf("ID: %d Name: %s\n",c->id, c->name);
	}

    printf("printing instructors:\n");
	for(inst = instructors; inst != NULL; inst = (struct instructor* )inst->hh.next){
		if(inst->id % 2 != 0){
			printf("Student ");
			printf("ID: %d Name: %s\n",inst->id, inst->name);
		}
		else{
			printf("ID: %d Name: %s\n",inst->id, inst->name);
			for(myCourse = inst->instructor_courses; myCourse != NULL; myCourse = (struct course* ) myCourse->hh.next){
				printf("\tCourse ID: %d Name: %s \n", myCourse->id, myCourse->name);
				printf("\t\tAssignments: ");
				HASH_ITER(hh, myCourse->course_assignments, ass, tmpass){
					printf("\t\t\tName: %s Value: %d\n",ass->name, ass->value);
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
	struct sockaddr_in server, client;
	struct sigaction action;

	if(argc < 2){   //check for correct number of args
		printf("Usage ./server PORT\n");
		return -1;
	}

	admin1 = init_admin(0, "brag", "root");
	//test_program();
    
	init_student(1, "akshay", "ak");
	init_student(3, "brag", "bragpassword");
	init_student(5, "kriti", "kriti");
	init_student(7, "yash", "bash");
	init_instructor(2, "juan", "gomez");
	init_instructor(1, "brag", "bragpassword");
	init_instructor(3, "kriti", "kriti");
	init_instructor(5, "yash", "bash");
	struct instructor* inst = init_instructor(4, "rod", "207");
	init_course(207, "Networking Apps", inst);
	int result = add_student_to_course("brag", "bragpassword", 207);
	if(result == 0){
		printf("error addint student to course\n");
		return -1;
	}
	result = add_student_to_course("yash", "bash", 207);
	if(result == 0){
		printf("error addint student to course\n");
		return -1;
	}
	result = add_student_to_course("kriti", "kriti", 207);
	if(result == 0){
		printf("error addint student to course\n");
		return -1;
	}
	result = add_student_to_course("akshay", "ak", 207);
	if(result == 0){
		printf("error addint student to course\n");
		return -1;
	}

	result = add_assignment_to_course(207, "Lab1", 100);
	if(result != 1){
		printf("error adding lab1 to 207\n");
		return -1;
	}

	result = add_assignment_to_course(207, "Lab2", 50);
	if(result != 1){
		printf("error adding lab2 to 207\n");
		return -1;
	}

	result = add_assignment_to_course(207, "Lab3", 100);
	if(result != 1){
		printf("error adding lab3 to 207\n");
		return -1;
	}

	result = grade_student("yash", "bash", 207, "Lab1", 95);
	if(result != 1){
		printf("error grading lab1\n");
		return -1;
	}
	result = grade_student("kriti", "kriti", 207, "Lab1", 100);
	if(result != 1){
		printf("error grading lab1\n");
		return -1;
	}
	result = grade_student("brag", "bragpassword", 207, "Lab1", 20);
	if(result != 1){
		printf("error grading lab1\n");
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

