/*
 * Unit tests for drivers module
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../includes/db.h"
#include "../includes/drivers.h"

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

static Driver make_driver(const char *emp_id, const char *last_name,
                          const char *cat, int exp, int born)
{
    Driver d = {0};
    strncpy(d.employee_id, emp_id,    MAX_STR-1);
    strncpy(d.last_name,   last_name, MAX_STR-1);
    strncpy(d.category,    cat,       MAX_STR-1);
    strncpy(d.address,     "City",    MAX_STR-1);
    d.experience = exp;
    d.birth_year = born;
    return d;
}

/* ------------------------------------------------------------------ */
static int test_driver_add(void)
{
    sqlite3 *db = setup_db();
    TEST_ASSERT(db != NULL, "DB connection");

    Driver d = make_driver("EMP001", "Ivanov", "B", 5, 1985);
    int rc = driver_add(db, &d);
    TEST_ASSERT(rc == 0, "driver_add returns 0 for valid driver");

    db_close(db);
    return 0;
}

/* ------------------------------------------------------------------ */
static int test_driver_get_by_id(void)
{
    sqlite3 *db = setup_db();
    TEST_ASSERT(db != NULL, "DB connection");

    Driver d = make_driver("EMP002", "Petrov", "C", 10, 1980);
    driver_add(db, &d);

    Driver out = {0};
    int rc = driver_get_by_id(db, 1, &out);
    TEST_ASSERT(rc == 0, "driver_get_by_id returns 0");
    TEST_ASSERT(strcmp(out.last_name, "Petrov") == 0, "last_name matches");
    TEST_ASSERT(out.experience == 10, "experience matches");

    db_close(db);
    return 0;
}

/* ------------------------------------------------------------------ */
static int test_driver_update(void)
{
    sqlite3 *db = setup_db();
    TEST_ASSERT(db != NULL, "DB connection");

    Driver d = make_driver("EMP003", "Sidorov", "B", 3, 1990);
    driver_add(db, &d);

    d.driver_id  = 1;
    d.experience = 7;
    strncpy(d.category, "C", MAX_STR-1);
    int rc = driver_update(db, &d);
    TEST_ASSERT(rc == 0, "driver_update returns 0");

    Driver out = {0};
    driver_get_by_id(db, 1, &out);
    TEST_ASSERT(out.experience == 7, "experience updated correctly");
    TEST_ASSERT(strcmp(out.category, "C") == 0, "category updated correctly");

    db_close(db);
    return 0;
}

/* ------------------------------------------------------------------ */
static int test_driver_delete(void)
{
    sqlite3 *db = setup_db();
    TEST_ASSERT(db != NULL, "DB connection");

    Driver d = make_driver("EMP004", "Kozlov", "B", 2, 1992);
    driver_add(db, &d);

    int rc = driver_delete(db, 1);
    TEST_ASSERT(rc == 0, "driver_delete returns 0");

    Driver out = {0};
    int get_rc = driver_get_by_id(db, 1, &out);
    TEST_ASSERT(get_rc != 0, "deleted driver is not found");

    db_close(db);
    return 0;
}

/* ------------------------------------------------------------------ */
static int test_driver_unique_employee_id(void)
{
    sqlite3 *db = setup_db();
    TEST_ASSERT(db != NULL, "DB connection");

    Driver d = make_driver("EMP005", "First",  "B", 1, 1995);
    driver_add(db, &d);

    Driver d2 = make_driver("EMP005", "Second", "C", 2, 1994);
    int rc = driver_add(db, &d2);
    TEST_ASSERT(rc != 0, "duplicate employee_id rejected");

    db_close(db);
    return 0;
}

/* ------------------------------------------------------------------ */
int main(void)
{
    printf("=== Running driver tests ===\n");
    int failed = 0;
    failed += test_driver_add();
    failed += test_driver_get_by_id();
    failed += test_driver_update();
    failed += test_driver_delete();
    failed += test_driver_unique_employee_id();

    if (failed == 0)
        printf("\nAll driver tests PASSED.\n");
    else
        fprintf(stderr, "\n%d driver test(s) FAILED.\n", failed);

    return failed;
}
