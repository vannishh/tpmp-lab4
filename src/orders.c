#include <stdio.h>
#include <string.h>
#include "../includes/orders.h"

/* ------------------------------------------------------------------ */
int order_add(sqlite3 *db, const Order *o)
{
    const char *sql =
        "INSERT INTO FLEET_ORDERS"
        "(order_date, driver_id, car_id,"
        " distance_km, cargo_weight, transport_cost)"
        " VALUES(?, ?, ?, ?, ?, ?);";

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "order_add prepare: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text  (stmt, 1, o->order_date,    -1, SQLITE_STATIC);
    sqlite3_bind_int   (stmt, 2, o->driver_id);
    sqlite3_bind_int   (stmt, 3, o->car_id);
    sqlite3_bind_double(stmt, 4, o->distance_km);
    sqlite3_bind_double(stmt, 5, o->cargo_weight);
    sqlite3_bind_double(stmt, 6, o->transport_cost);

    int rc = (sqlite3_step(stmt) == SQLITE_DONE) ? 0 : -1;
    if (rc != 0)
        fprintf(stderr, "order_add error: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return rc;
}

/* ------------------------------------------------------------------ */
int order_update(sqlite3 *db, const Order *o)
{
    const char *sql =
        "UPDATE FLEET_ORDERS SET order_date=?, driver_id=?, car_id=?,"
        " distance_km=?, cargo_weight=?, transport_cost=?"
        " WHERE order_id=?;";

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return -1;

    sqlite3_bind_text  (stmt, 1, o->order_date,    -1, SQLITE_STATIC);
    sqlite3_bind_int   (stmt, 2, o->driver_id);
    sqlite3_bind_int   (stmt, 3, o->car_id);
    sqlite3_bind_double(stmt, 4, o->distance_km);
    sqlite3_bind_double(stmt, 5, o->cargo_weight);
    sqlite3_bind_double(stmt, 6, o->transport_cost);
    sqlite3_bind_int   (stmt, 7, o->order_id);

    int rc = (sqlite3_step(stmt) == SQLITE_DONE) ? 0 : -1;
    sqlite3_finalize(stmt);
    return rc;
}

/* ------------------------------------------------------------------ */
int order_delete(sqlite3 *db, int order_id)
{
    const char *sql = "DELETE FROM FLEET_ORDERS WHERE order_id=?;";
    sqlite3_stmt *stmt = NULL;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, order_id);
    int rc = (sqlite3_step(stmt) == SQLITE_DONE) ? 0 : -1;
    sqlite3_finalize(stmt);
    return rc;
}

/* ------------------------------------------------------------------ */
void order_list_all(sqlite3 *db)
{
    const char *sql =
        "SELECT o.order_id, o.order_date,"
        "       d.last_name, c.license_plate,"
        "       o.distance_km, o.cargo_weight, o.transport_cost"
        " FROM FLEET_ORDERS o"
        " JOIN FLEET_DRIVERS d ON o.driver_id = d.driver_id"
        " JOIN FLEET_CARS    c ON o.car_id    = c.car_id"
        " ORDER BY o.order_date;";

    sqlite3_stmt *stmt = NULL;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    printf("\n%-6s %-12s %-20s %-12s %-10s %-10s %-12s\n",
           "ID","Date","Driver","Plate","Dist(km)","Cargo(t)","Cost");
    printf("%-6s %-12s %-20s %-12s %-10s %-10s %-12s\n",
           "------","------------","--------------------","------------",
           "----------","----------","------------");

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("%-6d %-12s %-20s %-12s %-10.2f %-10.2f %-12.2f\n",
               sqlite3_column_int   (stmt, 0),
               sqlite3_column_text  (stmt, 1),
               sqlite3_column_text  (stmt, 2),
               sqlite3_column_text  (stmt, 3),
               sqlite3_column_double(stmt, 4),
               sqlite3_column_double(stmt, 5),
               sqlite3_column_double(stmt, 6));
    }
    sqlite3_finalize(stmt);
}

/* ------------------------------------------------------------------ */
void order_list_by_driver_id(sqlite3 *db, int driver_id)
{
    const char *sql =
        "SELECT o.order_id, o.order_date, c.license_plate,"
        "       o.distance_km, o.cargo_weight, o.transport_cost,"
        "       o.transport_cost * 0.20 AS my_pay"
        " FROM FLEET_ORDERS o"
        " JOIN FLEET_CARS c ON o.car_id = c.car_id"
        " WHERE o.driver_id = ?"
        " ORDER BY o.order_date;";

    sqlite3_stmt *stmt = NULL;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, driver_id);

    printf("\n%-6s %-12s %-12s %-10s %-10s %-12s %-10s\n",
           "ID","Date","Plate","Dist(km)","Cargo(t)","Cost","MyPay");
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
    if (rows == 0) printf("No orders found.\n");
    sqlite3_finalize(stmt);
}

/* ------------------------------------------------------------------ */
/* Function 5: calculate earnings for ALL drivers for a period and     */
/* store results in FLEET_EARNINGS                                     */
int fn_calculate_earnings(sqlite3 *db,
                          const char *date_from, const char *date_to)
{
    const char *sel_sql =
        "SELECT driver_id,"
        "       SUM(transport_cost * 0.20) AS earnings"
        " FROM FLEET_ORDERS"
        " WHERE order_date BETWEEN ? AND ?"
        " GROUP BY driver_id;";

    const char *ins_sql =
        "INSERT INTO FLEET_EARNINGS"
        "(driver_id, period_start, period_end, total_earnings)"
        " VALUES(?, ?, ?, ?);";

    sqlite3_stmt *sel = NULL, *ins = NULL;
    sqlite3_prepare_v2(db, sel_sql, -1, &sel, NULL);
    sqlite3_bind_text(sel, 1, date_from, -1, SQLITE_STATIC);
    sqlite3_bind_text(sel, 2, date_to,   -1, SQLITE_STATIC);

    sqlite3_prepare_v2(db, ins_sql, -1, &ins, NULL);

    int count = 0;
    printf("\n--- Earnings report [%s — %s] ---\n", date_from, date_to);
    printf("%-5s %-12s\n", "DrvID","Earnings");
    printf("%-5s %-12s\n", "-----","------------");

    while (sqlite3_step(sel) == SQLITE_ROW) {
        int    did      = sqlite3_column_int   (sel, 0);
        double earnings = sqlite3_column_double(sel, 1);

        printf("%-5d %-12.2f\n", did, earnings);

        sqlite3_bind_int   (ins, 1, did);
        sqlite3_bind_text  (ins, 2, date_from, -1, SQLITE_STATIC);
        sqlite3_bind_text  (ins, 3, date_to,   -1, SQLITE_STATIC);
        sqlite3_bind_double(ins, 4, earnings);
        sqlite3_step(ins);
        sqlite3_reset(ins);
        count++;
    }
    if (count == 0) printf("No orders in this period.\n");

    sqlite3_finalize(sel);
    sqlite3_finalize(ins);
    printf("Saved %d records to FLEET_EARNINGS.\n", count);
    return count;
}

/* ------------------------------------------------------------------ */
/* Function 6: earnings for a specific driver in a period              */
int fn_calculate_driver_earnings(sqlite3 *db,
                                 const char *last_name,
                                 const char *date_from,
                                 const char *date_to)
{
    const char *sql =
        "SELECT d.last_name,"
        "       SUM(o.transport_cost * 0.20) AS earnings,"
        "       COUNT(o.order_id) AS trips"
        " FROM FLEET_ORDERS o"
        " JOIN FLEET_DRIVERS d ON o.driver_id = d.driver_id"
        " WHERE d.last_name = ?"
        "   AND o.order_date BETWEEN ? AND ?"
        " GROUP BY d.driver_id;";

    sqlite3_stmt *stmt = NULL;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, last_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, date_from, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, date_to,   -1, SQLITE_STATIC);

    printf("\n--- Earnings for driver [%s] from %s to %s ---\n",
           last_name, date_from, date_to);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("Driver  : %s\n",   sqlite3_column_text  (stmt, 0));
        printf("Earnings: %.2f\n", sqlite3_column_double(stmt, 1));
        printf("Trips   : %d\n",   sqlite3_column_int   (stmt, 2));
        sqlite3_finalize(stmt);
        return 0;
    }
    printf("No data found.\n");
    sqlite3_finalize(stmt);
    return -1;
}
