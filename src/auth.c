#include <stdio.h>
#include <string.h>
#include "../includes/auth.h"

/* ------------------------------------------------------------------ */
int auth_login(sqlite3 *db, const char *username,
               const char *password, User *out_user)
{
    const char *sql =
        "SELECT user_id, username, role, driver_id "
        "FROM FLEET_USERS "
        "WHERE username = ? AND password = ?;";

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return -1;

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);

    int found = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        out_user->user_id   = sqlite3_column_int(stmt, 0);
        strncpy(out_user->username,
                (const char *)sqlite3_column_text(stmt, 1), MAX_STR - 1);
        strncpy(out_user->role,
                (const char *)sqlite3_column_text(stmt, 2), MAX_STR - 1);
        out_user->driver_id = sqlite3_column_int(stmt, 3);
        found = 1;
    }

    sqlite3_finalize(stmt);
    return found ? 0 : -1;
}

/* ------------------------------------------------------------------ */
int auth_register(sqlite3 *db, const char *username,
                  const char *password, const char *role,
                  int driver_id)
{
    const char *sql =
        "INSERT INTO FLEET_USERS(username, password, role, driver_id)"
        " VALUES(?, ?, ?, ?);";

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Register prepare error: %s\n",
                sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, username,  -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password,  -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, role,      -1, SQLITE_STATIC);
    sqlite3_bind_int (stmt, 4, driver_id);

    int rc = (sqlite3_step(stmt) == SQLITE_DONE) ? 0 : -1;
    if (rc != 0)
        fprintf(stderr, "Register error: %s\n", sqlite3_errmsg(db));

    sqlite3_finalize(stmt);
    return rc;
}

/* ------------------------------------------------------------------ */
int auth_change_password(sqlite3 *db, int user_id,
                         const char *old_pass, const char *new_pass)
{
    /* Verify old password first */
    const char *check_sql =
        "SELECT COUNT(*) FROM FLEET_USERS"
        " WHERE user_id = ? AND password = ?;";

    sqlite3_stmt *stmt = NULL;
    sqlite3_prepare_v2(db, check_sql, -1, &stmt, NULL);
    sqlite3_bind_int (stmt, 1, user_id);
    sqlite3_bind_text(stmt, 2, old_pass, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    int cnt = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    if (cnt == 0) {
        printf("Incorrect current password.\n");
        return -1;
    }

    const char *upd_sql =
        "UPDATE FLEET_USERS SET password = ? WHERE user_id = ?;";
    sqlite3_prepare_v2(db, upd_sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, new_pass, -1, SQLITE_STATIC);
    sqlite3_bind_int (stmt, 2, user_id);
    int rc = (sqlite3_step(stmt) == SQLITE_DONE) ? 0 : -1;
    sqlite3_finalize(stmt);
    return rc;
}
