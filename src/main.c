#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../includes/db.h"
#include "../includes/auth.h"
#include "../includes/cars.h"
#include "../includes/drivers.h"
#include "../includes/orders.h"

/* ─────────────────────────── helpers ──────────────────────────── */
static void clear_stdin(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

static void read_line(const char *prompt, char *buf, int size)
{
    printf("%s", prompt);
    if (fgets(buf, size, stdin)) {
        size_t len = strlen(buf);
        if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
    }
}

/* ─────────────────────────── admin menus ──────────────────────── */
static void menu_cars(sqlite3 *db)
{
    int choice;
    for (;;) {
        printf("\n=== Cars ===\n"
               " 1. List all cars\n"
               " 2. Add car\n"
               " 3. Update car\n"
               " 4. Delete car\n"
               " 5. Report: mileage & cargo by plate\n"
               " 6. Report: car with max mileage\n"
               " 0. Back\n"
               ">>> ");
        if (scanf("%d", &choice) != 1) { clear_stdin(); continue; }
        clear_stdin();

        if (choice == 0) break;

        if (choice == 1) {
            car_list_all(db);

        } else if (choice == 2) {
            Car c = {0};
            read_line("Plate   : ", c.license_plate, MAX_STR);
            read_line("Brand   : ", c.brand,         MAX_STR);
            printf("Init mileage (km): "); scanf("%d", &c.initial_mileage); clear_stdin();
            printf("Capacity (tons)  : "); scanf("%lf",&c.capacity);        clear_stdin();
            printf(car_add(db, &c) == 0 ? "Added.\n" : "Error.\n");

        } else if (choice == 3) {
            Car c = {0};
            printf("Car ID to update: "); scanf("%d", &c.car_id); clear_stdin();
            read_line("New plate  : ", c.license_plate, MAX_STR);
            read_line("New brand  : ", c.brand,         MAX_STR);
            printf("New init mileage: "); scanf("%d",  &c.initial_mileage); clear_stdin();
            printf("New capacity    : "); scanf("%lf", &c.capacity);        clear_stdin();
            printf(car_update(db, &c) == 0 ? "Updated.\n" : "Error.\n");

        } else if (choice == 4) {
            int id; printf("Car ID to delete: "); scanf("%d", &id); clear_stdin();
            printf(car_delete(db, id) == 0 ? "Deleted.\n" : "Error.\n");

        } else if (choice == 5) {
            char plate[MAX_STR];
            read_line("License plate: ", plate, MAX_STR);
            car_report_mileage_cargo(db, plate);

        } else if (choice == 6) {
            car_max_mileage_report(db);
        }
    }
}

static void menu_drivers(sqlite3 *db)
{
    int choice;
    for (;;) {
        printf("\n=== Drivers ===\n"
               " 1. List all drivers\n"
               " 2. Add driver\n"
               " 3. Update driver\n"
               " 4. Delete driver\n"
               " 5. Report: orders by driver & period\n"
               " 6. Report: all driver stats\n"
               " 7. Report: driver with min trips\n"
               " 0. Back\n"
               ">>> ");
        if (scanf("%d", &choice) != 1) { clear_stdin(); continue; }
        clear_stdin();

        if (choice == 0) break;

        if (choice == 1) {
            driver_list_all(db);

        } else if (choice == 2) {
            Driver d = {0};
            read_line("Employee ID : ", d.employee_id, MAX_STR);
            read_line("Last name   : ", d.last_name,   MAX_STR);
            read_line("Category    : ", d.category,    MAX_STR);
            printf("Experience (yr): "); scanf("%d", &d.experience); clear_stdin();
            read_line("Address     : ", d.address,     MAX_STR);
            printf("Birth year     : "); scanf("%d", &d.birth_year); clear_stdin();
            printf(driver_add(db, &d) == 0 ? "Added.\n" : "Error.\n");

        } else if (choice == 3) {
            Driver d = {0};
            printf("Driver ID to update: "); scanf("%d", &d.driver_id); clear_stdin();
            read_line("Employee ID : ", d.employee_id, MAX_STR);
            read_line("Last name   : ", d.last_name,   MAX_STR);
            read_line("Category    : ", d.category,    MAX_STR);
            printf("Experience (yr): "); scanf("%d", &d.experience); clear_stdin();
            read_line("Address     : ", d.address,     MAX_STR);
            printf("Birth year     : "); scanf("%d", &d.birth_year); clear_stdin();
            printf(driver_update(db, &d) == 0 ? "Updated.\n" : "Error.\n");

        } else if (choice == 4) {
            int id; printf("Driver ID to delete: "); scanf("%d", &id); clear_stdin();
            printf(driver_delete(db, id) == 0 ? "Deleted.\n" : "Error.\n");

        } else if (choice == 5) {
            char name[MAX_STR], dfrom[MAX_STR], dto[MAX_STR];
            read_line("Last name  : ", name,  MAX_STR);
            read_line("Date from  (YYYY-MM-DD): ", dfrom, MAX_STR);
            read_line("Date to    (YYYY-MM-DD): ", dto,   MAX_STR);
            driver_orders_by_period(db, name, dfrom, dto);

        } else if (choice == 6) {
            driver_stats_all(db);

        } else if (choice == 7) {
            driver_min_trips_report(db);
        }
    }
}

static void menu_orders(sqlite3 *db)
{
    int choice;
    for (;;) {
        printf("\n=== Orders ===\n"
               " 1. List all orders\n"
               " 2. Add order\n"
               " 3. Update order\n"
               " 4. Delete order\n"
               " 5. Calculate earnings (all, period) → save\n"
               " 6. Calculate earnings (one driver, period)\n"
               " 0. Back\n"
               ">>> ");
        if (scanf("%d", &choice) != 1) { clear_stdin(); continue; }
        clear_stdin();

        if (choice == 0) break;

        if (choice == 1) {
            order_list_all(db);

        } else if (choice == 2) {
            Order o = {0};
            read_line("Date (YYYY-MM-DD): ", o.order_date, MAX_STR);
            printf("Driver ID  : "); scanf("%d",  &o.driver_id);      clear_stdin();
            printf("Car ID     : "); scanf("%d",  &o.car_id);         clear_stdin();
            printf("Distance   : "); scanf("%lf", &o.distance_km);    clear_stdin();
            printf("Cargo (t)  : "); scanf("%lf", &o.cargo_weight);   clear_stdin();
            printf("Cost       : "); scanf("%lf", &o.transport_cost); clear_stdin();
            int rc = order_add(db, &o);
            if (rc == 0) printf("Order added.\n");
            else         printf("Error: cargo may exceed capacity.\n");

        } else if (choice == 3) {
            Order o = {0};
            printf("Order ID to update: "); scanf("%d", &o.order_id); clear_stdin();
            read_line("Date (YYYY-MM-DD): ", o.order_date, MAX_STR);
            printf("Driver ID  : "); scanf("%d",  &o.driver_id);      clear_stdin();
            printf("Car ID     : "); scanf("%d",  &o.car_id);         clear_stdin();
            printf("Distance   : "); scanf("%lf", &o.distance_km);    clear_stdin();
            printf("Cargo (t)  : "); scanf("%lf", &o.cargo_weight);   clear_stdin();
            printf("Cost       : "); scanf("%lf", &o.transport_cost); clear_stdin();
            printf(order_update(db, &o) == 0 ? "Updated.\n" : "Error.\n");

        } else if (choice == 4) {
            int id; printf("Order ID: "); scanf("%d", &id); clear_stdin();
            printf(order_delete(db, id) == 0 ? "Deleted.\n" : "Error.\n");

        } else if (choice == 5) {
            char dfrom[MAX_STR], dto[MAX_STR];
            read_line("Date from (YYYY-MM-DD): ", dfrom, MAX_STR);
            read_line("Date to   (YYYY-MM-DD): ", dto,   MAX_STR);
            fn_calculate_earnings(db, dfrom, dto);

        } else if (choice == 6) {
            char name[MAX_STR], dfrom[MAX_STR], dto[MAX_STR];
            read_line("Last name : ", name,  MAX_STR);
            read_line("Date from (YYYY-MM-DD): ", dfrom, MAX_STR);
            read_line("Date to   (YYYY-MM-DD): ", dto,   MAX_STR);
            fn_calculate_driver_earnings(db, name, dfrom, dto);
        }
    }
}

/* ─────────────────────── driver self-service ──────────────────── */
static void menu_driver_self(sqlite3 *db, User *u)
{
    int choice;
    for (;;) {
        printf("\n=== Welcome, %s ===\n"
               " 1. My orders\n"
               " 2. My car info\n"
               " 3. Change password\n"
               " 0. Logout\n"
               ">>> ", u->username);
        if (scanf("%d", &choice) != 1) { clear_stdin(); continue; }
        clear_stdin();

        if (choice == 0) break;

        if (choice == 1) {
            order_list_by_driver_id(db, u->driver_id);

        } else if (choice == 2) {
            /* Find car assigned to latest order of this driver */
            const char *sql =
                "SELECT c.car_id, c.license_plate, c.brand,"
                " c.initial_mileage, c.capacity"
                " FROM FLEET_CARS c"
                " JOIN FLEET_ORDERS o ON c.car_id = o.car_id"
                " WHERE o.driver_id = ?"
                " ORDER BY o.order_date DESC LIMIT 1;";
            sqlite3_stmt *stmt = NULL;
            sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
            sqlite3_bind_int(stmt, 1, u->driver_id);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                printf("\n--- My Car ---\n");
                printf("ID    : %d\n",    sqlite3_column_int   (stmt, 0));
                printf("Plate : %s\n",    sqlite3_column_text  (stmt, 1));
                printf("Brand : %s\n",    sqlite3_column_text  (stmt, 2));
                printf("Init mileage: %d km\n", sqlite3_column_int(stmt,3));
                printf("Capacity    : %.2f t\n",sqlite3_column_double(stmt,4));
            } else {
                printf("No car found (no orders yet).\n");
            }
            sqlite3_finalize(stmt);

        } else if (choice == 3) {
            char old[MAX_STR], nw[MAX_STR];
            read_line("Current password: ", old, MAX_STR);
            read_line("New password    : ", nw,  MAX_STR);
            printf(auth_change_password(db, u->user_id, old, nw) == 0
                   ? "Password changed.\n" : "Error.\n");
        }
    }
}

/* ─────────────────────────── admin root ───────────────────────── */
static void menu_admin(sqlite3 *db)
{
    int choice;
    for (;;) {
        printf("\n=== Admin Panel ===\n"
               " 1. Cars\n"
               " 2. Drivers\n"
               " 3. Orders\n"
               " 4. Register user\n"
               " 0. Logout\n"
               ">>> ");
        if (scanf("%d", &choice) != 1) { clear_stdin(); continue; }
        clear_stdin();

        if (choice == 0) break;
        if (choice == 1) menu_cars(db);
        if (choice == 2) menu_drivers(db);
        if (choice == 3) menu_orders(db);
        if (choice == 4) {
            char uname[MAX_STR], pass[MAX_STR], role[MAX_STR];
            int  drv_id = 0;
            read_line("Username : ", uname, MAX_STR);
            read_line("Password : ", pass,  MAX_STR);
            read_line("Role (admin/driver): ", role, MAX_STR);
            if (strcmp(role, "driver") == 0) {
                printf("Driver ID: "); scanf("%d", &drv_id); clear_stdin();
            }
            printf(auth_register(db, uname, pass, role, drv_id) == 0
                   ? "User registered.\n" : "Error registering user.\n");
        }
    }
}

/* ─────────────────────────── entry point ───────────────────────── */
int main(void)
{
    printf("=== Autopark Management System ===\n");
    printf("    Team: Короткевич Д. & Близник К.\n\n");

    sqlite3 *db = db_connect(DB_PATH);
    if (!db) return 1;

    if (db_init(db) != DB_OK) {
        fprintf(stderr, "Failed to initialize database.\n");
        db_close(db);
        return 1;
    }

    User current = {0};
    char uname[MAX_STR], pass[MAX_STR];

    for (;;) {
        printf("\n--- Login ---\n");
        read_line("Username (0 to quit): ", uname, MAX_STR);
        if (strcmp(uname, "0") == 0) break;
        read_line("Password            : ", pass,  MAX_STR);

        if (auth_login(db, uname, pass, &current) == 0) {
            printf("Welcome, %s! Role: %s\n", current.username, current.role);
            if (strcmp(current.role, ROLE_ADMIN) == 0)
                menu_admin(db);
            else
                menu_driver_self(db, &current);
        } else {
            printf("Invalid username or password.\n");
        }
    }

    db_close(db);
    printf("Goodbye!\n");
    return 0;
}
