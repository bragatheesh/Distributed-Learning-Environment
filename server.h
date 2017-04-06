#ifndef __SERVER_H__
#define __SERVER_H__

struct student{
    int id;
    char name[100];
    struct course* courses;

};

struct instructor{
    int id;
    char name[100];
    struct course* courses;
};

struct course{
    int id;
    char name[100];
    struct sudent* students;
    struct instructor course_instructor;

};

struct admin{
    int id;
    char name[100];
};

#endif /* __SERVER_H__ */