#ifndef __SERVER_H__
#define __SERVER_H__

#include <string>

struct student{
    int id;
    std::string name;
    std::string password;
    std::string uniq;
    struct course* courses = NULL;
    UT_hash_handle hh;
};

struct instructor{
    int id;
    std::string name;
    std::string password;
    std::string uniq;
    struct course* courses = NULL;
    UT_hash_handle hh;
};

struct course{
    int id;
    std::string name;
    struct student* students = NULL;
    struct instructor course_instructor;
    struct assignment* assignments = NULL;
    UT_hash_handle hh;
};

struct admin{
    int id;
    std::string name;
    std::string password;
};

struct assignment{
    std::string name;
    int value;
};

/*void
test_program(){
	struct instructor* inst;
	struct student* c;
	struct course* course;
	init_student(0, "brag", "bragpassword");
	init_student(1, "akshay", "ak");
	init_student(0, "brag", "bragpassword");
	inst = init_instructor(0, "rod", "fatoohi");
	init_course(207, "207", *inst);
	init_instructor(1, "juan", "gomez");
	init_instructor(0, "rod", "fatoohi");

    printf("printing students:\n");
	for(c = students; c != NULL; c = (struct student*) c->hh.next){
		std::cout << "ID: " << c->id << " Name: " << c->name << std::endl;
	}
    printf("printing instructors:\n");
	for(inst = instructors; inst != NULL; inst = (struct instructor* )inst->hh.next){
		std::cout << "ID: " << inst->id << " Name: " << inst->name << std::endl;
	}
    printf("printing courses:\n");
	for(course = courses; course != NULL; course = (struct course* ) course->hh.next){
		std::cout << "ID: " << course->id << " Name: " << course->name << " Taught by: " << course->course_instructor.name <<std::endl;
	}
}*/

#endif /* __SERVER_H__ */