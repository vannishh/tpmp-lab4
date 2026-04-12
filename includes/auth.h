#ifndef AUTH_H
#define AUTH_H

#include <sqlite3.h>

#define ROLE_ADMIN  "admin"
#define ROLE_DRIVER "driver"
#define MAX_STR     256

typedef struct {
    int  user_id;
    char username[MAX_STR];
    char role[MAX_STR];
    int  driver_id; /* 0 if admin */
} User;

int auth_login(sqlite3 *db, const char *username,
               const char *password, User *out_user);
int auth_register(sqlite3 *db, const char *username,
                  const char *password, const char *role,
                  int driver_id);
int auth_change_password(sqlite3 *db, int user_id,
                         const char *old_pass, const char *new_pass);

#endif /* AUTH_H */
