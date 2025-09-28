/*************************************************************************
*
* File Name: mysql_base.c
* Author: cwc-vguang
* mail: chenwencheng@vguang.cn
* Created Time: 2025年09月26日 星期五 11时14分29秒
*
*************************************************************************/

#include <stdio.h>
#include <string.h>

#include "mysql_common_sql.h"

MYSQL *common_sql_connect(const char *host, const char *user, const char *password, const char *database)
{

    MYSQL *conn = NULL;

    conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init failed\n");
        return NULL;
    }

    if (!mysql_real_connect(conn, host, user, password, database, 0, NULL, 0)) {
        fprintf(stderr, "mysql_connect failed: %s\n", mysql_error(conn));
        return NULL;
    }

    return conn;
}

int common_sql_close(MYSQL *conn)
{
    if (!conn) {
        fprintf(stderr, "conn is null \n");
        return -1;
    }

    mysql_close(conn);

    return 0;
}

int common_sql_create_table(MYSQL *conn, const char *query)
{
    if (!conn || !query) {
        fprintf(stderr, "invalied param\n");
        return -1;
    }

    if (mysql_query(conn, query)) {
        fprintf(stderr, "create failed: %s\n", mysql_error(conn));
        return -1;
    }

    return 0;
}

/* 统一的预处理语句执行接口(增删改查，都可以用此接口进行预处理操作，防止sql注入。) */
int common_sql_execute_prepared(MYSQL *conn, const char *query, MYSQL_BIND *bind, int param_count)
{
    if (!conn || !query) {
        fprintf(stderr, "invalid param\n");
        return -1;
    }

    MYSQL_STMT *stmt = NULL;

    /* 1. 初始化预处理语句 */
    stmt = mysql_stmt_init(conn);
    if (!stmt) {
        fprintf(stderr, "mysql_stmt_init failed: %s\n", mysql_error(conn));
        goto cleanup;
    }

    /* 2. 准备SQL语句 */
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "mysql_stmt_prepare failed: %s\n", mysql_stmt_error(stmt));
        goto cleanup;
    }

    /* 3. 如果有参数则绑定参数 */
    if (bind && param_count > 0) {
        if (mysql_stmt_bind_param(stmt, bind)) {
            fprintf(stderr, "mysql_stmt_bind_param failed: %s\n", mysql_stmt_error(stmt));
            goto cleanup;
        }
    }

    /* 4. 执行预处理语句 */
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "mysql_stmt_execute failed: %s\n", mysql_stmt_error(stmt));
        goto cleanup;
    }

    /* 5. 获取执行信息 */
    my_ulonglong affected_rows = mysql_stmt_affected_rows(stmt);
    if (affected_rows > 0) {
        printf("预处理语句执行成功，影响行数: %lu\n", (unsigned long)affected_rows);
    }


    return 0;
cleanup:
    if (stmt) {
        mysql_stmt_close(stmt);
    }

    return -1;
}
