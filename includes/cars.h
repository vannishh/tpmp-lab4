#ifndef CARS_H
#define CARS_H

#include <sqlite3.h>

#define MAX_STR 256

typedef struct {
    int    car_id;
    char   license_plate[MAX_STR];
    char   brand[MAX_STR];
    int    initial_mileage;
    double capacity; /* tons */
} Car;

int  car_add(sqlite3 *db, const Car *car);
int  car_update(sqlite3 *db, const Car *car);
int  car_delete(sqlite3 *db, int car_id);
int  car_get_by_id(sqlite3 *db, int car_id, Car *out);
void car_list_all(sqlite3 *db);

/* Reports */
void car_report_mileage_cargo(sqlite3 *db, const char *license_plate);
void car_max_mileage_report(sqlite3 *db);

#endif /* CARS_H */
