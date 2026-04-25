#include <stdio.h>
#include <string.h>
#include "../includes/drivers.h"

/* ------------------------------------------------------------------ */
int driver_add(sqlite3 *db, const Driver *d)
{
    const char *sql =
        "INSERT INTO FLEET_DRIVERS"
        "(employee_id, last_name, category, experience, address, birth_year)"
        " VALUES(?, ?, ?, ?, ?, ?);";

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "driver_add prepare: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, d->employee_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, d->last_name,   -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, d->category,    -1, SQLITE_STATIC);
    sqlite3_bind_int (stmt, 4, d->experience);
    sqlite3_bind_text(stmt, 5, d->address,     -1, SQLITE_STATIC);
    sqlite3_bind_int (stmt, 6, d->birth_year);

    int rc = (sqlite3_step(stmt) == SQLITE_DONE) ? 0 : -1;
    if (rc != 0)
        fprintf(stderr, "driver_add error: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return rc;
}

/* ------------------------------------------------------------------ */
int driver_update(sqlite3 *db, const Driver *d)
{
    const char *sql =
        "UPDATE FLEET_DRIVERS SET employee_id=?, last_name=?,"
        " category=?, experience=?, address=?, birth_year=?"
        " WHERE driver_id=?;";

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return -1;

    sqlite3_bind_text(stmt, 1, d->employee_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, d->last_name,   -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, d->category,    -1, SQLITE_STATIC);
    sqlite3_bind_int (stmt, 4, d->experience);
    sqlite3_bind_text(stmt, 5, d->address,     -1, SQLITE_STATIC);
    sqlite3_bind_int (stmt, 6, d->birth_year);
    sqlite3_bind_int (stmt, 7, d->driver_id);

    int rc = (sqlite3_step(stmt) == SQLITE_DONE) ? 0 : -1;
    sqlite3_finalize(stmt);
    return rc;
}

/* ------------------------------------------------------------------ */
int driver_delete(sqlite3 *db, int driver_id)
{
    const char *sql = "DELETE FROM FLEET_DRIVERS WHERE driver_id=?;";
    sqlite3_stmt *stmt = NULL;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, driver_id);
    int rc = (sqlite3_step(stmt) == SQLITE_DONE) ? 0 : -1;
    sqlite3_finalize(stmt);
    return rc;
}

/* ------------------------------------------------------------------ */
int driver_get_by_id(sqlite3 *db, int driver_id, Driver *out)
{
    const char *sql =
        "SELECT driver_id, employee_id, last_name, category,"
        " experience, address, birth_year"
        " FROM FLEET_DRIVERS WHERE driver_id=?;";

    sqlite3_stmt *stmt = NULL;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, driver_id);

    int found = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        out->driver_id  = sqlite3_column_int(stmt, 0);
        strncpy(out->employee_id,
                (const char *)sqlite3_column_text(stmt, 1), MAX_STR-1);
        strncpy(out->last_name,
                (const char *)sqlite3_column_text(stmt, 2), MAX_STR-1);
        strncpy(out->category,
                (const char *)sqlite3_column_text(stmt, 3), MAX_STR-1);
        out->experience = sqlite3_column_int(stmt, 4);
        strncpy(out->address,
                (const char *)sqlite3_column_text(stmt, 5), MAX_STR-1);
        out->birth_year = sqlite3_column_int(stmt, 6);
        found = 1;
    }
    sqlite3_finalize(stmt);
    return found ? 0 : -1;
}

/* ------------------------------------------------------------------ */
void driver_list_all(sqlite3 *db)
{
    const char *sql =
        "SELECT driver_id, employee_id, last_name, category,"
        " experience, birth_year FROM FLEET_DRIVERS ORDER BY driver_id;";

    sqlite3_stmt *stmt = NULL;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    printf("\n%-4s %-10s %-20s %-8s %-6s %-6s\n",
           "ID","EmpID","Last Name","Cat","Exp","Born");
    printf("%-4s %-10s %-20s %-8s %-6s %-6s\n",
           "----","----------","--------------------",
           "--------","------","------");

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("%-4d %-10s %-20s %-8s %-6d %-6d\n",
               sqlite3_column_int (stmt, 0),
               sqlite3_column_text(stmt, 1),
               sqlite3_column_text(stmt, 2),
               sqlite3_column_text(stmt, 3),
               sqlite3_column_int (stmt, 4),
               sqlite3_column_int (stmt, 5));
    }
    sqlite3_finalize(stmt);
}

/* ------------------------------------------------------------------ */
/* Report: list of orders for a driver in a date range                 */
void driver_orders_by_period(sqlite3 *db, const char *last_name,
                             const char *date_from, const char *date_to)
{
    const char *sql =
        "SELECT o.order_id, o.order_date, c.license_plate,"
        "       o.distance_km, o.cargo_weight, o.transport_cost,"
        "       o.transport_cost * 0.20 AS driver_pay"
        " FROM FLEET_ORDERS o"
        " JOIN FLEET_DRIVERS d ON o.driver_id = d.driver_id"
        " JOIN FLEET_CARS    c ON o.car_id    = c.car_id"
        " WHERE d.last_name = ?"
        "   AND o.order_date BETWEEN ? AND ?"
        " ORDER BY o.order_date;";

    sqlite3_stmt *stmt = NULL;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, last_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, date_from, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, date_to,   -1, SQLITE_STATIC);

    printf("\n--- Orders for driver [%s] from %s to %s ---\n",
           last_name, date_from, date_to);
    printf("%-6s %-12s %-12s %-10s %-10s %-12s %-10s\n",
           "OrdID","Date","Plate","Dist(km)","Cargo(t)","Cost","MyPay");
    printf("%-6s %-12s %-12s %-10s %-10s %-12s %-10s\n",
           "------","------------","------------",
           "----------","----------","------------","----------");

    int rows = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("%-6d %-12s %-12s %-10.2f %-10.2f %-12.2f %-10.2f\n",
               sqlite3_column_int   (stmt, 0),
               sqlite3_column_text  (stmt, 1),
               sqlite3_column_text  (stmt, 2),
               sqlite3_column_double(stmt, 3),
               sqlite3_column_double(stmt, 4),
               sqlite3_column_double(stmt, 5),
               sqlite3_column_double(stmt, 6));
        rows++;
    }
    if (rows == 0) printf("No orders found for this period.\n");
    sqlite3_finalize(stmt);
}

/* ------------------------------------------------------------------ */
/* Report: per-driver stats — trips, total cargo, earned money         */
void driver_stats_all(sqlite3 *db)
{
    const char *sql =
        "SELECT d.last_name,"
        "       COUNT(o.order_id)          AS trips,"
        "       COALESCE(SUM(o.cargo_weight),0)   AS total_cargo,"
        "       COALESCE(SUM(o.transport_cost*0.20),0) AS earnings"
        " FROM FLEET_DRIVERS d"
        " LEFT JOIN FLEET_ORDERS o ON d.driver_id = o.driver_id"
        " GROUP BY d.driver_id"
        " ORDER BY trips DESC;";

    sqlite3_stmt *stmt = NULL;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    printf("\n--- Driver statistics ---\n");
    printf("%-20s %-8s %-12s %-12s\n",
           "Last Name","Trips","Cargo(t)","Earnings");
    printf("%-20s %-8s %-12s %-12s\n",
           "--------------------","--------",
           "------------","------------");

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("%-20s %-8d %-12.2f %-12.2f\n",
               sqlite3_column_text  (stmt, 0),
               sqlite3_column_int   (stmt, 1),
               sqlite3_column_double(stmt, 2),
               sqlite3_column_double(stmt, 3));
    }
    sqlite3_finalize(stmt);
}

/* ------------------------------------------------------------------ */
/* Report: driver with minimum number of trips                         */
void driver_min_trips_report(sqlite3 *db)
{
    const char *sql =
        "SELECT d.driver_id, d.employee_id, d.last_name, d.category,"
        "       d.experience, d.address, d.birth_year,"
        "       COUNT(o.order_id) AS trips,"
        "       COALESCE(SUM(o.transport_cost*0.20),0) AS earnings"
        " FROM FLEET_DRIVERS d"
        " LEFT JOIN FLEET_ORDERS o ON d.driver_id = o.driver_id"
        " GROUP BY d.driver_id"
        " ORDER BY trips ASC LIMIT 1;";

    sqlite3_stmt *stmt = NULL;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    printf("\n--- Driver with minimum trips ---\n");
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("ID         : %d\n",  sqlite3_column_int   (stmt, 0));
        printf("Emp.ID     : %s\n",  sqlite3_column_text  (stmt, 1));
        printf("Last name  : %s\n",  sqlite3_column_text  (stmt, 2));
        printf("Category   : %s\n",  sqlite3_column_text  (stmt, 3));
        printf("Experience : %d yr\n",sqlite3_column_int  (stmt, 4));
        printf("Address    : %s\n",  sqlite3_column_text  (stmt, 5));
        printf("Born       : %d\n",  sqlite3_column_int   (stmt, 6));
        printf("Trips      : %d\n",  sqlite3_column_int   (stmt, 7));
        printf("Earnings   : %.2f\n",sqlite3_column_double(stmt, 8));
    } else {
        printf("No drivers found.\n");
    }
    sqlite3_finalize(stmt);
}
