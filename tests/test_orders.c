/*
 * Unit tests for orders module
 * Tests CRUD, cargo-capacity trigger, and earnings functions.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../includes/db.h"
#include "../includes/cars.h"
#include "../includes/drivers.h"
#include "../includes/orders.h"

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "FAIL [%s:%d] %s\n", __FILE__, __LINE__, msg); \
            return 1; \
        } \
        printf("PASS: %s\n", msg); \
    } while (0)

static sqlite3 *setup_db(void)
{
    sqlite3 *db = db_connect(":memory:");
    if (!db) return NULL;
    db_init(db);

    /* Seed one car and one driver for order tests */
    Car c = {0};
    strncpy(c.license_plate, "TEST001A", MAX_STR-1);
    strncpy(c.brand, "TestBrand", MAX_STR-1);
    c.initial_mileage = 0;
    c.capacity = 5.0; /* 5 tons max */
    car_add(db, &c);

    Driver d = {0};
    strncpy(d.employee_id, "EMP_TEST", MAX_STR-1);
    strncpy(d.last_name,   "TestDriver", MAX_STR-1);
    strncpy(d.category,    "B", MAX_STR-1);
    strncpy(d.address,     "TestCity", MAX_STR-1);
    d.experience = 5;
    d.birth_year = 1990;
    driver_add(db, &d);

    return db;
}

/* ------------------------------------------------------------------ */
static int test_order_add_valid(void)
{
    sqlite3 *db = setup_db();
    TEST_ASSERT(db != NULL, "DB connection");

    Order o = {0};
    strncpy(o.order_date, "2025-01-15", MAX_STR-1);
    o.driver_id      = 1;
    o.car_id         = 1;
    o.distance_km    = 200.0;
    o.cargo_weight   = 4.5; /* within capacity */
    o.transport_cost = 5000.0;

    int rc = order_add(db, &o);
    TEST_ASSERT(rc == 0, "order_add returns 0 for valid order");

    db_close(db);
    return 0;
}

/* ------------------------------------------------------------------ */
static int test_order_cargo_exceeds_capacity(void)
{
    sqlite3 *db = setup_db();
    TEST_ASSERT(db != NULL, "DB connection");

    Order o = {0};
    strncpy(o.order_date, "2025-01-16", MAX_STR-1);
    o.driver_id      = 1;
    o.car_id         = 1;
    o.distance_km    = 100.0;
    o.cargo_weight   = 10.0; /* EXCEEDS capacity of 5 tons */
    o.transport_cost = 3000.0;

    int rc = order_add(db, &o);
    TEST_ASSERT(rc != 0, "order rejected when cargo > capacity (trigger)");

    db_close(db);
    return 0;
}

/* ------------------------------------------------------------------ */
static int test_order_delete(void)
{
    sqlite3 *db = setup_db();
    TEST_ASSERT(db != NULL, "DB connection");

    Order o = {0};
    strncpy(o.order_date, "2025-02-10", MAX_STR-1);
    o.driver_id = 1; o.car_id = 1;
    o.distance_km = 150.0; o.cargo_weight = 3.0; o.transport_cost = 4000.0;
    order_add(db, &o);

    int rc = order_delete(db, 1);
    TEST_ASSERT(rc == 0, "order_delete returns 0");

    db_close(db);
    return 0;
}

/* ------------------------------------------------------------------ */
static int test_fn_calculate_earnings(void)
{
    sqlite3 *db = setup_db();
    TEST_ASSERT(db != NULL, "DB connection");

    /* Add two orders */
    Order o1 = {0};
    strncpy(o1.order_date, "2025-03-01", MAX_STR-1);
    o1.driver_id=1; o1.car_id=1;
    o1.distance_km=100; o1.cargo_weight=2.0; o1.transport_cost=1000.0;
    order_add(db, &o1);

    Order o2 = {0};
    strncpy(o2.order_date, "2025-03-15", MAX_STR-1);
    o2.driver_id=1; o2.car_id=1;
    o2.distance_km=200; o2.cargo_weight=3.0; o2.transport_cost=2000.0;
    order_add(db, &o2);

    int cnt = fn_calculate_earnings(db, "2025-03-01", "2025-03-31");
    TEST_ASSERT(cnt == 1, "fn_calculate_earnings: 1 driver record saved");

    /* Verify saved earnings: (1000 + 2000) * 0.20 = 600 */
    const char *sql =
        "SELECT total_earnings FROM FLEET_EARNINGS WHERE driver_id=1;";
    sqlite3_stmt *stmt = NULL;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_step(stmt);
    double earnings = sqlite3_column_double(stmt, 0);
    sqlite3_finalize(stmt);
    TEST_ASSERT(earnings == 600.0, "earnings value is 600.00 (20% of 3000)");

    db_close(db);
    return 0;
}

/* ------------------------------------------------------------------ */
static int test_fn_calculate_driver_earnings_no_data(void)
{
    sqlite3 *db = setup_db();
    TEST_ASSERT(db != NULL, "DB connection");

    int rc = fn_calculate_driver_earnings(db, "NoSuchDriver",
                                          "2025-01-01","2025-12-31");
    TEST_ASSERT(rc != 0, "fn_calculate_driver_earnings returns -1 for unknown driver");

    db_close(db);
    return 0;
}

/* ------------------------------------------------------------------ */
int main(void)
{
    printf("=== Running order tests ===\n");
    int failed = 0;
    failed += test_order_add_valid();
    failed += test_order_cargo_exceeds_capacity();
    failed += test_order_delete();
    failed += test_fn_calculate_earnings();
    failed += test_fn_calculate_driver_earnings_no_data();

    if (failed == 0)
        printf("\nAll order tests PASSED.\n");
    else
        fprintf(stderr, "\n%d order test(s) FAILED.\n", failed);

    return failed;
}
