#ifndef USER_H
#define USER_H
const unsigned int MAX_USERNAME_LEN = 14;
const unsigned int MAX_PASSWORD_LEN = 14;

struct User
{
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    unsigned short uid;
    unsigned short gid;
};

#endif