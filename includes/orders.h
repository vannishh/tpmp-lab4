#ifndef ORDERS_H
#define ORDERS_H

#include <sqlite3.h>

#define MAX_STR 256

typedef struct {
    int    order_id;
    char   order_date[MAX_STR];
    int    driver_id;
    int    car_id;
    double distance_km;
    double cargo_weight; /* tons */
    double transport_cost;
} Order;

int  order_add(sqlite3 *db, const Order *o);
int  order_update(sqlite3 *db, const Order *o);
int  order_delete(sqlite3 *db, int order_id);
void order_list_all(sqlite3 *db);
void order_list_by_driver_id(sqlite3 *db, int driver_id);

/* Business-logic functions */
int  fn_calculate_earnings(sqlite3 *db,
                           const char *date_from,
                           const char *date_to);
int  fn_calculate_driver_earnings(sqlite3 *db,
                                  const char *last_name,
                                  const char *date_from,
                                  const char *date_to);

#endif /* ORDERS_H */
