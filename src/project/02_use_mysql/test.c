#include <stdio.h>
#include <stdlib.h>

#include "db_mysql.h"

// 接口: mysql连接
MYSQL *mysql_conn(void)
{
    static MYSQL *conn;
    // 初始化MYSQL连接
    conn = mysql_init(NULL);
    // 检查是否连接成功
    if (conn == NULL) {
        fprintf(stderr, "mysql init() failed\n");
        return NULL;
    } 
    
    // 连接到 MYSQL 服务器
    conn = mysql_real_connect(conn, "localhost", "root", "root", "my_test", 0, NULL, 0);
    if (conn == NULL) {
        fprintf(stderr, "connect mysql server is falied\n");
        mysql_close(conn);
        return NULL;
    }
    
    fprintf(stderr, "connect mysql db is successful\n");
    return conn;
}

int main()
{
    int ret = -1;

    // mysql 连接
    MYSQL *conn = mysql_conn(); 
    if (conn == NULL) {
        fprintf(stderr, "mysql conn is NULL\n");
        return -1;
    }

    // 根据名字查询数据
    ret = select_info_by_username(conn, "lisi");
    if (ret < 0) {
        fprintf(stderr, "select_info_by_username failed\n");
        return -1;
    }
    // 关闭连接
    mysql_close(conn);

    return 0;
}
