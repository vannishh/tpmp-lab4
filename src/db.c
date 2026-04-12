#include <stdio.h>
#include <stdlib.h>
#include "../includes/db.h"

/* ------------------------------------------------------------------ */
sqlite3 *db_connect(const char *db_path)
{
    sqlite3 *db = NULL;
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n",
                sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }
    return db;
}

/* ------------------------------------------------------------------ */
void db_close(sqlite3 *db)
{
    if (db) sqlite3_close(db);
}

/* ------------------------------------------------------------------ */
int db_execute(sqlite3 *db, const char *sql)
{
    char *err_msg = NULL;
    int   rc      = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return DB_ERROR;
    }
    return DB_OK;
}

/* ------------------------------------------------------------------ */
int db_init(sqlite3 *db)
{
    const char *schema =
        "PRAGMA foreign_keys = ON;"

        "CREATE TABLE IF NOT EXISTS FLEET_USERS ("
        "  user_id   INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  username  TEXT    NOT NULL UNIQUE,"
        "  password  TEXT    NOT NULL,"
        "  role      TEXT    NOT NULL CHECK(role IN ('admin','driver')),"
        "  driver_id INTEGER DEFAULT 0,"
        "  FOREIGN KEY (driver_id) REFERENCES FLEET_DRIVERS(driver_id)"
        ");"

        "CREATE TABLE IF NOT EXISTS FLEET_CARS ("
        "  car_id          INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  license_plate   TEXT    NOT NULL UNIQUE,"
        "  brand           TEXT    NOT NULL,"
        "  initial_mileage INTEGER NOT NULL DEFAULT 0,"
        "  capacity        REAL    NOT NULL"
        ");"

        "CREATE TABLE IF NOT EXISTS FLEET_DRIVERS ("
        "  driver_id   INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  employee_id TEXT    NOT NULL UNIQUE,"
        "  last_name   TEXT    NOT NULL,"
        "  category    TEXT    NOT NULL,"
        "  experience  INTEGER NOT NULL,"
        "  address     TEXT,"
        "  birth_year  INTEGER NOT NULL"
        ");"

        "CREATE TABLE IF NOT EXISTS FLEET_ORDERS ("
        "  order_id       INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  order_date     DATE    NOT NULL,"
        "  driver_id      INTEGER NOT NULL,"
        "  car_id         INTEGER NOT NULL,"
        "  distance_km    REAL    NOT NULL,"
        "  cargo_weight   REAL    NOT NULL,"
        "  transport_cost REAL    NOT NULL,"
        "  FOREIGN KEY (driver_id) REFERENCES FLEET_DRIVERS(driver_id),"
        "  FOREIGN KEY (car_id)    REFERENCES FLEET_CARS(car_id)"
        ");"

        /* Earnings summary table (populated by fn_calculate_earnings) */
        "CREATE TABLE IF NOT EXISTS FLEET_EARNINGS ("
        "  earnings_id   INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  driver_id     INTEGER NOT NULL,"
        "  period_start  DATE    NOT NULL,"
        "  period_end    DATE    NOT NULL,"
        "  total_earnings REAL   NOT NULL,"
        "  FOREIGN KEY (driver_id) REFERENCES FLEET_DRIVERS(driver_id)"
        ");"

        /* Trigger: prevent order if cargo > car capacity */
        "CREATE TRIGGER IF NOT EXISTS trg_check_cargo "
        "BEFORE INSERT ON FLEET_ORDERS "
        "BEGIN "
        "  SELECT CASE "
        "    WHEN NEW.cargo_weight > (SELECT capacity FROM FLEET_CARS "
        "                             WHERE car_id = NEW.car_id) "
        "    THEN RAISE(ABORT,'Cargo weight exceeds vehicle capacity') "
        "  END; "
        "END;"

        /* Seed admin user (password = 'admin123') */
        "INSERT OR IGNORE INTO FLEET_USERS(username,password,role)"
        "  VALUES('admin','admin123','admin');";

    return db_execute(db, schema);
}
