/*
 * Unit tests for cars module
 * Uses a temporary in-memory SQLite database.
 * Returns 0 on success, non-zero on failure.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../includes/db.h"
#include "../includes/cars.h"

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
    return db;
}

/* ------------------------------------------------------------------ */
static int test_car_add_valid(void)
{
    sqlite3 *db = setup_db();
    TEST_ASSERT(db != NULL, "DB connection");

    Car c = {0};
    strncpy(c.license_plate, "AA1234BB", MAX_STR-1);
    strncpy(c.brand,         "Volvo",    MAX_STR-1);
    c.initial_mileage = 5000;
    c.capacity        = 10.0;

    int rc = car_add(db, &c);
    TEST_ASSERT(rc == 0, "car_add returns 0 for valid car");

    db_close(db);
    return 0;
}

/* ------------------------------------------------------------------ */
static int test_car_get_by_id(void)
{
    sqlite3 *db = setup_db();
    TEST_ASSERT(db != NULL, "DB connection");

    Car c = {0};
    strncpy(c.license_plate, "BB5678CC", MAX_STR-1);
    strncpy(c.brand,         "MAN",      MAX_STR-1);
    c.initial_mileage = 12000;
    c.capacity        = 15.0;
    car_add(db, &c);

    Car out = {0};
    int rc = car_get_by_id(db, 1, &out);
    TEST_ASSERT(rc == 0, "car_get_by_id returns 0");
    TEST_ASSERT(strcmp(out.brand, "MAN") == 0, "brand matches after retrieval");
    TEST_ASSERT(out.capacity == 15.0, "capacity matches after retrieval");

    db_close(db);
    return 0;
}

/* ------------------------------------------------------------------ */
static int test_car_update(void)
{
    sqlite3 *db = setup_db();
    TEST_ASSERT(db != NULL, "DB connection");

    Car c = {0};
    strncpy(c.license_plate, "CC9999DD", MAX_STR-1);
    strncpy(c.brand,         "Kamaz",    MAX_STR-1);
    c.initial_mileage = 0;
    c.capacity        = 8.0;
    car_add(db, &c);

    c.car_id   = 1;
    c.capacity = 12.0;
    strncpy(c.brand, "Kamaz Pro", MAX_STR-1);
    int rc = car_update(db, &c);
    TEST_ASSERT(rc == 0, "car_update returns 0");

    Car out = {0};
    car_get_by_id(db, 1, &out);
    TEST_ASSERT(out.capacity == 12.0, "capacity updated correctly");
    TEST_ASSERT(strcmp(out.brand, "Kamaz Pro") == 0, "brand updated correctly");

    db_close(db);
    return 0;
}

/* ------------------------------------------------------------------ */
static int test_car_delete(void)
{
    sqlite3 *db = setup_db();
    TEST_ASSERT(db != NULL, "DB connection");

    Car c = {0};
    strncpy(c.license_plate, "DD1111EE", MAX_STR-1);
    strncpy(c.brand,         "GAZ",      MAX_STR-1);
    c.initial_mileage = 1000;
    c.capacity        = 3.0;
    car_add(db, &c);

    int rc = car_delete(db, 1);
    TEST_ASSERT(rc == 0, "car_delete returns 0");

    Car out = {0};
    int get_rc = car_get_by_id(db, 1, &out);
    TEST_ASSERT(get_rc != 0, "deleted car is not found");

    db_close(db);
    return 0;
}

/* ------------------------------------------------------------------ */
static int test_car_duplicate_plate(void)
{
    sqlite3 *db = setup_db();
    TEST_ASSERT(db != NULL, "DB connection");

    Car c = {0};
    strncpy(c.license_plate, "EE2222FF", MAX_STR-1);
    strncpy(c.brand,         "Brand1",   MAX_STR-1);
    c.capacity = 5.0;
    car_add(db, &c);

    /* Second insert with same plate must fail */
    int rc = car_add(db, &c);
    TEST_ASSERT(rc != 0, "duplicate license plate rejected");

    db_close(db);
    return 0;
}

/* ------------------------------------------------------------------ */
int main(void)
{
    printf("=== Running car tests ===\n");
    int failed = 0;
    failed += test_car_add_valid();
    failed += test_car_get_by_id();
    failed += test_car_update();
    failed += test_car_delete();
    failed += test_car_duplicate_plate();

    if (failed == 0)
        printf("\nAll car tests PASSED.\n");
    else
        fprintf(stderr, "\n%d car test(s) FAILED.\n", failed);

    return failed;
}
