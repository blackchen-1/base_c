#ifndef _DB_MYSQL_H_
#define _DB_MYSQL_H_

#include <mysql/mysql.h>

int select_info_all(MYSQL *conn);
int select_info_by_username(MYSQL *conn, const char *username);
int check_column_exists(MYSQL *conn, const char *db_name, const char *table_name, const char *column_name);
int insert_single_row(MYSQL *conn, char *username, char *phone_number, char *password);

#endif