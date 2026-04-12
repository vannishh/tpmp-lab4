#ifndef DB_H
#define DB_H

#include <sqlite3.h>

#define DB_PATH "autopark.db"
#define DB_OK    0
#define DB_ERROR (-1)

sqlite3 *db_connect(const char *db_path);
void     db_close(sqlite3 *db);
int      db_init(sqlite3 *db);
int      db_execute(sqlite3 *db, const char *sql);

#endif /* DB_H */
