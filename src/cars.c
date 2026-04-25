#include <stdio.h>
#include <string.h>
#include "../includes/cars.h"

/* ------------------------------------------------------------------ */
int car_add(sqlite3 *db, const Car *c)
{
    const char *sql =
        "INSERT INTO FLEET_CARS(license_plate, brand,"
        " initial_mileage, capacity)"
        " VALUES(?, ?, ?, ?);";

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return -1;

    sqlite3_bind_text  (stmt, 1, c->license_plate, -1, SQLITE_STATIC);
    sqlite3_bind_text  (stmt, 2, c->brand,          -1, SQLITE_STATIC);
    sqlite3_bind_int   (stmt, 3, c->initial_mileage);
    sqlite3_bind_double(stmt, 4, c->capacity);

    int rc = (sqlite3_step(stmt) == SQLITE_DONE) ? 0 : -1;
    if (rc != 0)
        fprintf(stderr, "car_add error: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return rc;
}

/* ------------------------------------------------------------------ */
int car_update(sqlite3 *db, const Car *c)
{
    const char *sql =
        "UPDATE FLEET_CARS SET license_plate=?, brand=?,"
        " initial_mileage=?, capacity=? WHERE car_id=?;";

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return -1;

    sqlite3_bind_text  (stmt, 1, c->license_plate, -1, SQLITE_STATIC);
    sqlite3_bind_text  (stmt, 2, c->brand,          -1, SQLITE_STATIC);
    sqlite3_bind_int   (stmt, 3, c->initial_mileage);
    sqlite3_bind_double(stmt, 4, c->capacity);
    sqlite3_bind_int   (stmt, 5, c->car_id);

    int rc = (sqlite3_step(stmt) == SQLITE_DONE) ? 0 : -1;
    sqlite3_finalize(stmt);
    return rc;
}

/* ------------------------------------------------------------------ */
int car_delete(sqlite3 *db, int car_id)
{
    const char *sql = "DELETE FROM FLEET_CARS WHERE car_id = ?;";
    sqlite3_stmt *stmt = NULL;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, car_id);
    int rc = (sqlite3_step(stmt) == SQLITE_DONE) ? 0 : -1;
    sqlite3_finalize(stmt);
    return rc;
}

/* ------------------------------------------------------------------ */
int car_get_by_id(sqlite3 *db, int car_id, Car *out)
{
    const char *sql =
        "SELECT car_id, license_plate, brand,"
        " initial_mileage, capacity"
        " FROM FLEET_CARS WHERE car_id = ?;";

    sqlite3_stmt *stmt = NULL;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, car_id);

    int found = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        out->car_id          = sqlite3_column_int(stmt, 0);
        strncpy(out->license_plate,
                (const char *)sqlite3_column_text(stmt, 1), MAX_STR-1);
        strncpy(out->brand,
                (const char *)sqlite3_column_text(stmt, 2), MAX_STR-1);
        out->initial_mileage = sqlite3_column_int   (stmt, 3);
        out->capacity        = sqlite3_column_double(stmt, 4);
        found = 1;
    }
    sqlite3_finalize(stmt);
    return found ? 0 : -1;
}

/* ------------------------------------------------------------------ */
void car_list_all(sqlite3 *db)
{
    const char *sql =
        "SELECT car_id, license_plate, brand,"
        " initial_mileage, capacity FROM FLEET_CARS ORDER BY car_id;";

    sqlite3_stmt *stmt = NULL;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    printf("\n%-5s %-12s %-20s %-15s %-10s\n",
           "ID", "Plate", "Brand", "Init.Mileage", "Cap(t)");
    printf("%-5s %-12s %-20s %-15s %-10s\n",
           "-----","------------","--------------------",
           "---------------","----------");

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("%-5d %-12s %-20s %-15d %-10.2f\n",
               sqlite3_column_int   (stmt, 0),
               sqlite3_column_text  (stmt, 1),
               sqlite3_column_text  (stmt, 2),
               sqlite3_column_int   (stmt, 3),
               sqlite3_column_double(stmt, 4));
    }
    sqlite3_finalize(stmt);
}

/* ------------------------------------------------------------------ */
/* Report: total mileage and cargo weight for a specific car           */
void car_report_mileage_cargo(sqlite3 *db, const char *license_plate)
{
    const char *sql =
        "SELECT c.license_plate, c.brand,"
        "       COALESCE(SUM(o.distance_km), 0) AS total_mileage,"
        "       COALESCE(SUM(o.cargo_weight), 0) AS total_cargo"
        " FROM FLEET_CARS c"
        " LEFT JOIN FLEET_ORDERS o ON c.car_id = o.car_id"
        " WHERE c.license_plate = ?"
        " GROUP BY c.car_id;";

    sqlite3_stmt *stmt = NULL;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, license_plate, -1, SQLITE_STATIC);

    printf("\n--- Report: Car mileage & cargo ---\n");
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("Plate  : %s\n", sqlite3_column_text  (stmt, 0));
        printf("Brand  : %s\n", sqlite3_column_text  (stmt, 1));
        printf("Total mileage (km) : %.2f\n", sqlite3_column_double(stmt, 2));
        printf("Total cargo   (t)  : %.2f\n", sqlite3_column_double(stmt, 3));
    } else {
        printf("Car not found: %s\n", license_plate);
    }
    sqlite3_finalize(stmt);
}

/* ------------------------------------------------------------------ */
/* Report: car with max total mileage                                  */
void car_max_mileage_report(sqlite3 *db)
{
    const char *sql =
        "SELECT c.car_id, c.license_plate, c.brand,"
        "       c.initial_mileage, c.capacity,"
        "       COALESCE(SUM(o.distance_km), 0) AS total_km"
        " FROM FLEET_CARS c"
        " LEFT JOIN FLEET_ORDERS o ON c.car_id = o.car_id"
        " GROUP BY c.car_id"
        " ORDER BY total_km DESC LIMIT 1;";

    sqlite3_stmt *stmt = NULL;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    printf("\n--- Report: Car with maximum total mileage ---\n");
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("ID           : %d\n",   sqlite3_column_int   (stmt, 0));
        printf("Plate        : %s\n",   sqlite3_column_text  (stmt, 1));
        printf("Brand        : %s\n",   sqlite3_column_text  (stmt, 2));
        printf("Init mileage : %d km\n",sqlite3_column_int   (stmt, 3));
        printf("Capacity     : %.2f t\n",sqlite3_column_double(stmt,4));
        printf("Total run    : %.2f km\n",sqlite3_column_double(stmt,5));
    } else {
        printf("No cars found.\n");
    }
    sqlite3_finalize(stmt);
}
