#ifndef DRIVERS_H
#define DRIVERS_H

#include <sqlite3.h>

#define MAX_STR 256

typedef struct {
    int  driver_id;
    char employee_id[MAX_STR];
    char last_name[MAX_STR];
    char category[MAX_STR];
    int  experience;
    char address[MAX_STR];
    int  birth_year;
} Driver;

int  driver_add(sqlite3 *db, const Driver *d);
int  driver_update(sqlite3 *db, const Driver *d);
int  driver_delete(sqlite3 *db, int driver_id);
int  driver_get_by_id(sqlite3 *db, int driver_id, Driver *out);
void driver_list_all(sqlite3 *db);

/* Reports */
void driver_orders_by_period(sqlite3 *db, const char *last_name,
                             const char *date_from, const char *date_to);
void driver_stats_all(sqlite3 *db);
void driver_min_trips_report(sqlite3 *db);

#endif /* DRIVERS_H */
