/*************************************************************************
*
* File Name: app/apps/syslog/syslog.c
* Author: cwc-vguang
* mail: chenwencheng@vguang.cn
* Created Time: 2025年09月28日 星期日 10时27分45秒
*
*************************************************************************/

#include <stdio.h>

#include "mysql_common_sql.h"

#define MYSQL_USERNAME "root"
#define MYSQL_PASSWORD "root"
#define MYSQL_DATABASE "test_db"

int syslog_sql_connect(void)
{
    MYSQL *conn;

    // 1. 连接数据库
    conn = common_sql_connect("localhost", MYSQL_USERNAME, MYSQL_PASSWORD,  MYSQL_DATABASE);
    if (!conn) {
        fprintf(stderr, "Failed to connect to database\n");
        return -1;
    }

    printf("Database connected successfully!\n");
    return 0;
}

int syslog_sql_close()
{

}

int syslog_table_create(void)
{
    // 2. 创建表
    const char *create_table_sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INT AUTO_INCREMENT PRIMARY KEY, "
        "name VARCHAR(64) NOT NULL, "
        "age INT, "
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ")";

    if (common_sql_create_table(conn, create_table_sql) != 0) {
        fprintf(stderr, "Failed to create table\n");
        common_sql_close(conn);
        return -1;
    }

    printf("Table created successfully!\n");

    return 0;
}

int syslog_api_insert(const char *name, int age)
{
    // 3. 使用预处理语句插入数据（安全方式）
    const char *insert_sql = "INSERT INTO users (name, age) VALUES (?, ?)";

    char user_name[] = name;
    int user_age = age;

    MYSQL_BIND bind[2];
    memset(bind, 0, sizeof(bind));

    // 绑定name参数
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = user_name;
    bind[0].buffer_length = strlen(user_name);
    bind[0].length = &bind[0].buffer_length;

    // 绑定age参数
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = &user_age;
    bind[1].is_unsigned = 0;

    if (common_sql_execute_prepared(conn, insert_sql, bind, 2) != 0) {
        fprintf(stderr, "Failed to insert data\n");
    } else {
        printf("Data inserted successfully!\n");
    }
    return 0;
}

int syslog_api_update(const char *name, int age)
{
    // 5. 更新数据
    const char *update_sql = "UPDATE users SET age = ? WHERE name = ?";

    int new_age = age;
    bind[0].buffer = &new_age;
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = name;
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer_length = strlen(name);
    bind[1].length = &bind[1].buffer_length;

    if (common_sql_execute_prepared(conn, update_sql, bind, 2) != 0) {
        fprintf(stderr, "Failed to update data\n");
    } else {
        printf("Data updated successfully!\n");
    }

}

int syslog_api_select()
{
    // 6. 查询数据（使用普通查询）
    const char *select_sql = "SELECT id, name, age FROM users";
    if (mysql_query(conn, select_sql)) {
        fprintf(stderr, "SELECT failed: %s\n", mysql_error(conn));
    } else {
        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            MYSQL_ROW row;
            int num_fields = mysql_num_fields(result);

            printf("\n=== User Records ===\n");
            while ((row = mysql_fetch_row(result))) {
                for (int i = 0; i < num_fields; i++) {
                    printf("%s\t", row[i] ? row[i] : "NULL");
                }
                printf("\n");
            }
            mysql_free_result(result);
        }
    }

}

