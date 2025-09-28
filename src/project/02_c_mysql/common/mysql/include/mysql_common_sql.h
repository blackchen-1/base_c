#ifndef __MYSQL_COMMON_SQL_H__
#define __MYSQL_COMMON_SQL_H__

#include <mysql/mysql.h>

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @brief 连接MySQL数据库
 *
 * 初始化MySQL连接并建立到指定数据库的连接
 *
 * @param host 数据库服务器地址
 * @param user 数据库用户名
 * @param password 数据库密码
 * @param database 要连接的数据库名称
 *
 * @return 成功返回MYSQL连接句柄，失败返回NULL
 *
 * @note 使用后需要调用common_sql_close()关闭连接
 * @note 返回的连接句柄需要手动释放
 */
MYSQL *common_sql_connect(const char *host, const char *user, const char *password, const char *database);

/**
 * @brief 关闭MySQL数据库连接
 *
 * 关闭并释放MySQL连接资源
 *
 * @param conn MySQL连接句柄
 *
 * @return 成功返回0，失败返回-1
 *
 * @note 调用后conn指针变为无效，不应再使用
 */
int common_sql_close(MYSQL *conn);

/**
 * @brief 创建数据表
 *
 * 执行CREATE TABLE SQL语句创建数据表
 *
 * @param conn MySQL连接句柄
 * @param query CREATE TABLE SQL语句
 *
 * @return 成功返回0，失败返回-1
 *
 * @note 该函数使用mysql_query执行，不支持参数绑定
 * @note 建议使用"CREATE TABLE IF NOT EXISTS"避免重复创建
 */
int common_sql_create_table(MYSQL *conn, const char *query);

/**
 * @brief 统一的预处理语句执行接口
 *
 * 使用预处理语句执行SQL操作，防止SQL注入攻击。
 * 支持INSERT、UPDATE、DELETE、SELECT等所有类型的SQL语句。
 *
 * @param conn MySQL连接句柄
 * @param query SQL语句，使用?作为参数占位符
 * @param bind 参数绑定数组，包含参数的类型、值等信息
 * @param param_count 参数数量，应与bind数组大小一致
 *
 * @return 成功返回0，失败返回-1
 *
 * @note 自动处理预处理语句的初始化、准备、参数绑定、执行和清理
 * @note 对于SELECT查询，如果需要结果集应使用专门的查询接口
 * @note 参数绑定使用MYSQL_BIND结构体，需正确设置各字段
 *
 * @example
 * const char *sql = "INSERT INTO users (name, age) VALUES (?, ?)";
 * char name[] = "张三";
 * int age = 25;
 * MYSQL_BIND bind[2];
 * // 设置bind[0]为name参数
 * // 设置bind[1]为age参数
 * common_sql_execute_prepared(conn, sql, bind, 2);
 */
int common_sql_execute_prepared(MYSQL *conn, const char *query, MYSQL_BIND *bind, int param_count);

#ifdef  __cplusplus
}
#endif

#endif
