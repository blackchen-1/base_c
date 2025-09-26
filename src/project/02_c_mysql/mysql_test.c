#include <stdio.h>
#include <mysql/mysql.h>



int main()
{
    MYSQL *conn;
    MYSQL_RES *result;
    MYSQL_ROW row;

    /* 初始化连接句柄 */
    conn = mysql_init(NULL);

    /* 设置编码(可选) */
    mysql_options(conn, MYSQL_SET_CHARSET_NAME, "utf8");

    /* 连接数据库 */
    if (!mysql_real_connect(conn, "localhost", "root", "root", "mysql", 0, NULL, 0)) {
        fprintf(stderr, "连接失败: %s \n", mysql_error(conn));
        return -1;
    }

    fprintf(stderr, "连接成功\n");

    return 0;
}
