#include <stdio.h>
#include <string.h>

#include "db_mysql.h"

// 接口: 查询表中所有数据
int select_info_all(MYSQL *conn)
{
    char sql[1024];
    MYSQL_RES *res;
    MYSQL_ROW row;
    sprintf(sql, "select * from users");
    
    // 执行sql语句
    if (mysql_query(conn, sql)) {
        fprintf(stderr, "Query failed: %s\n", mysql_error(conn));
        return -1; // 返回 1 表示失败
    };
    
    // 获取查询结果集
    res = mysql_store_result(conn);
    if (res == NULL) {
        fprintf(stderr, "mysql_store_result failed: %s\n", mysql_error(conn));
        return -1;
    }
    
    // 打印查询结果
    while ((row = mysql_fetch_row(res)) != NULL) {
        printf("ID: %s, Username: %s, phone_number: %s, password: %s, create_at: %s\n", row[0], row[1], row[2], row[3], row[4]);
    }
    
    // 释放结果集
    mysql_free_result(res);
}

// 接口: 查询表中数据，根据用户名
int select_info_by_username(MYSQL *conn, const char *username)
{
    char sql[1024];
    MYSQL_RES *res;
    MYSQL_ROW row;
    
    // 构建查询语句
    sprintf(sql, "select *from users where username = '%s'", username);
    
    // 执行sql语句
    if (mysql_query(conn, sql)) {
        fprintf(stderr, "Query failed: %s\n", mysql_error(conn));
        return -1; // 返回 1 表示失败
    };

    // 获取结果集
    res = mysql_store_result(conn);
    if (!res) {
        fprintf(stderr, "mysql_store_result failed \n");
        return -1;
    }

    // 打印结果集
    while ((row = mysql_fetch_row(res)) != NULL) {
        fprintf(stderr, "ID: %s, Username: %s, phone_number: %s, password: %s, create_at: %s\n", row[0], row[1], row[2], row[3], row[4]);
    }

    return 0;
}

// 接口: 查询数据，判断列名是否存在
// 返回 -1 表示sql语句错误, 1 表示存在，0 表示不存在,
int check_column_exists(MYSQL *conn, const char *db_name, const char *table_name, const char *column_name)
{
    char sql[1024];
    MYSQL_RES *res;
    int exists;

    sprintf(sql, "select * from %s.%s where username = '%s'",
                 db_name, table_name, column_name);

    if (mysql_query(conn, sql)) {
        fprintf(stderr, "Query failed: %s\n", mysql_error(conn));
        return -1; 
    }
    
     // 获取查询结果
    res = mysql_store_result(conn);
    if (res == NULL) {
        fprintf(stderr, "mysql_store_result failed: %s\n", mysql_error(conn));
        return -1;
    }

    // 检查 结果集 中是否有行
    exists = mysql_num_rows(res) > 0;

    // 释放结果集
    mysql_free_result(res);

    return exists;
}

// 接口：插入单行数据
int insert_single_row(MYSQL *conn, char *username, char *phone_number, char *password)
{
    // 判断用户名是否被注册
    char sql[1024];
    const char *db_name = "my_test";
    const char *table_name = "users";
    const char *column_name = username;
    int exists;

    exists = check_column_exists(conn, db_name, table_name, column_name);

    if (exists == 1) {
        fprintf(stderr, "Column '%s' exists in table '%s'\n", column_name, table_name);
        return -1;
    } else if (exists == 0) {
        fprintf(stderr, "Column '%s' does not exist in table '%s'\n", column_name, table_name);
    } else {
        fprintf(stderr, "select_username is exit sql failed \n");
        return -1;
    }
    
    // 插入数据
    sprintf(sql, "INSERT INTO users (username, phone_number, password) VALUES ('%s', '%s', '%s')", username, phone_number, password);
    if (mysql_query(conn, sql)) {
        fprintf(stderr, "Insert failed: %s\n", mysql_error(conn));
        return -1;
    }
    return 0;
}