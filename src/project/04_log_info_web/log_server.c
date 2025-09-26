// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <pthread.h>
#include <time.h>

#include <mysql/mysql.h>

#define HTTP_PORT 8080
#define SYSLOG_PORT 514  // 改为大于1024的端口
#define HTML_FILE "index.html"
#define JSON_FILE "data.json"

// 数据库连接信息
#define DB_HOST "localhost"
#define DB_USER "root"
#define DB_PASS "root"
#define DB_NAME "school_db"

// URL解码函数
void url_decode(char *src, char *dst) {
    char *p = dst;
    while (*src) {
        if (*src == '%' && src[1] && src[2]) {
            sscanf(src + 1, "%2x", (unsigned int *)p);
            src += 3;
        } else if (*src == '+') {
            *p = ' ';
            src++;
        } else {
            *p = *src++;
        }
        p++;
    }
    *p = '\0';
}

// 连接数据库
MYSQL* connect_db() {
    MYSQL *conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0)) {
        fprintf(stderr, "MySQL连接失败: %s\n", mysql_error(conn));
        return NULL;
    }
    return conn;
}

// 创建syslog表
void create_syslog_table() {
    MYSQL *conn = connect_db();
    if (!conn) return;

    const char *query = "CREATE TABLE IF NOT EXISTS syslog_logs ("
                        "id INT AUTO_INCREMENT PRIMARY KEY, "
                        "timestamp VARCHAR(32), "
                        "hostname VARCHAR(64), "
                        "app_name VARCHAR(64), "
                        "message TEXT, "
                        "level VARCHAR(16), "
                        "func_name VARCHAR(128), "
                        "line_number INT, "
                        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
                        "INDEX idx_timestamp (timestamp), "
                        "INDEX idx_level (level), "
                        "INDEX idx_app_name (app_name)"
                        ")";

    if (mysql_query(conn, query)) {
        fprintf(stderr, "创建表失败: %s\n", mysql_error(conn));
    }

    mysql_close(conn);
}

// 清理旧日志（保留最新的1000条）
void cleanup_old_logs() {
    MYSQL *conn = connect_db();
    if (!conn) return;

    // 获取当前日志总数
    if (mysql_query(conn, "SELECT COUNT(*) FROM syslog_logs")) {
        mysql_close(conn);
        return;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (res) {
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row) {
            int count = atoi(row[0]);
            if (count > 1000) {
                // 使用TRUNCATE清空整个表（会重置自增ID）
                // 注意：这会删除所有数据，适用于测试环境
                mysql_query(conn, "TRUNCATE TABLE syslog_logs");
            }
        }
        mysql_free_result(res);
    }

    mysql_close(conn);
}

// 插入syslog消息到数据库
int insert_syslog_message(const char *timestamp, const char *hostname,
                         const char *app_name, const char *message,
                         const char *level, const char *func_name,
                         int line_number) {
    MYSQL *conn = connect_db();
    if (!conn) return 0;

    char query[1024];
    snprintf(query, sizeof(query),
             "INSERT INTO syslog_logs (timestamp, hostname, app_name, message, level, func_name, line_number) "
             "VALUES ('%s', '%s', '%s', '%s', '%s', '%s', %d)",
             timestamp ? timestamp : "",
             hostname ? hostname : "",
             app_name ? app_name : "",
             message ? message : "",
             level ? level : "",
             func_name ? func_name : "",
             line_number);

    int result = mysql_query(conn, query);
    if (result) {
        fprintf(stderr, "插入syslog消息失败: %s\n", mysql_error(conn));
    }

    mysql_close(conn);

    // 每次插入后检查是否需要清理旧日志
    if (result == 0) {
        cleanup_old_logs();
    }

    return result == 0;
}

// 生成 syslog JSON 文件
void generate_syslog_json() {
    MYSQL *conn = connect_db();
    if (!conn) return;

    MYSQL_RES *res;
    MYSQL_ROW row;
    FILE *fp;

    if (mysql_query(conn, "SELECT id, timestamp, hostname, app_name, message, level, func_name, line_number FROM syslog_logs ORDER BY id DESC LIMIT 1000")) {
        fprintf(stderr, "查询失败: %s\n", mysql_error(conn));
        mysql_close(conn);
        return;
    }

    res = mysql_store_result(conn);
    fp = fopen(JSON_FILE, "w");
    if (!fp) {
        perror("无法创建 data.json");
        mysql_free_result(res);
        mysql_close(conn);
        return;
    }

    fprintf(fp, "[\n");
    int first = 1;
    while ((row = mysql_fetch_row(res))) {
        if (!first) fprintf(fp, ",\n");
        fprintf(fp, "  {\"id\":%s,\"timestamp\":\"%s\",\"hostname\":\"%s\",\"app_name\":\"%s\",\"message\":\"%s\",\"level\":\"%s\",\"func_name\":\"%s\",\"line_number\":%s}",
                row[0], row[1], row[2], row[3], row[4], row[5], row[6], row[7]);
        first = 0;
    }
    fprintf(fp, "\n]\n");

    fclose(fp);
    mysql_free_result(res);
    mysql_close(conn);
}

// 读取文件内容
int read_file(const char *filename, char **buffer) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return 0;

    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    *buffer = malloc(len + 1);
    fread(*buffer, 1, len, fp);
    (*buffer)[len] = '\0';

    fclose(fp);
    return len;
}

// 发送HTTP响应
void send_response(int socket, const char *content_type, const char *content, int content_length) {
    dprintf(socket, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n",
            content_type, content_length);
    write(socket, content, content_length);
}

// 发送JSON响应
void send_json_response(int socket, const char *json) {
    send_response(socket, "application/json", json, strlen(json));
}

// 处理HTTP API请求
void handle_http_api_request(int socket, const char *method, const char *path) {
    // 获取syslog消息列表
    if (strcmp(path, "/api/syslog") == 0 && strcmp(method, "GET") == 0) {
        generate_syslog_json();
        char *response = NULL;
        int len = read_file(JSON_FILE, &response);
        if (len) {
            send_response(socket, "application/json", response, len);
            free(response);
        } else {
            char *empty_response = "[]";
            send_json_response(socket, empty_response);
        }
        return;
    }

    // 404 Not Found
    dprintf(socket, "HTTP/1.1 404 Not Found\r\n\r\n{\"error\": \"API接口未找到\"}");
}

// 处理HTTP静态文件请求
void handle_http_static_request(int socket, const char *path) {
    if (strcmp(path, "/data.json") == 0) {
        generate_syslog_json();
        char *response = NULL;
        int len = read_file(JSON_FILE, &response);
        if (len) {
            send_response(socket, "application/json", response, len);
            free(response);
        } else {
            dprintf(socket, "HTTP/1.1 404 Not Found\r\n\r\n404");
        }
    } else {
        // 默认返回 index.html
        char *response = NULL;
        int len = read_file(HTML_FILE, &response);
        if (len) {
            send_response(socket, "text/html", response, len);
            free(response);
        } else {
            dprintf(socket, "HTTP/1.1 404 Not Found\r\n\r\n404");
        }
    }
}

// 解析syslog消息 (针对你的特定格式)
void parse_syslog_message(const char *raw_message, char *timestamp, char *hostname,
                         char *app_name, char *log_message, char *level,
                         char *func_name, int *line_number) {
    // 获取当前时间作为timestamp
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(timestamp, 32, "%Y-%m-%d %H:%M:%S", tm_info);

    // 默认值
    strcpy(hostname, "unknown");
    strcpy(app_name, "unknown");
    strcpy(level, "INFO");
    strcpy(func_name, "");
    *line_number = 0;

    // 解析你的日志格式: vbar: vgmqtt_message_manager_async_report():509:info| async report start - topic_str: access_device/v2/event/access, uplink_data: 0x9ab5d918, serial: 00000000O4

    char message_copy[4096];
    strncpy(message_copy, raw_message, sizeof(message_copy) - 1);
    message_copy[sizeof(message_copy) - 1] = '\0';

    // 查找第一个冒号，分离应用名
    char *first_colon = strchr(message_copy, ':');
    if (first_colon) {
        *first_colon = '\0';
        strncpy(app_name, message_copy, 63);
        app_name[63] = '\0';

        // 处理冒号后的部分
        char *after_app = first_colon + 2; // 跳过冒号和空格

        // 查找函数名和行号 (xxx():line:)
        char *paren_close = strchr(after_app, ')');
        if (paren_close && *(paren_close + 1) == ':' && isdigit(*(paren_close + 2))) {
            // 提取函数名
            char *func_start = after_app;
            *paren_close = '\0';
            strncpy(func_name, func_start, 127);
            func_name[127] = '\0';

            // 提取行号
            char *line_start = paren_close + 2;
            char *colon_after_line = strchr(line_start, ':');
            if (colon_after_line) {
                *colon_after_line = '\0';
                *line_number = atoi(line_start);

                // 处理日志级别和消息内容
                char *level_and_message = colon_after_line + 1;
                char *pipe = strchr(level_and_message, '|');
                if (pipe) {
                    *pipe = '\0';
                    // 提取日志级别
                    strncpy(level, level_and_message, 15);
                    level[15] = '\0';

                    // 转换级别为大写
                    for (int i = 0; level[i]; i++) {
                        level[i] = toupper(level[i]);
                    }

                    // 提取消息内容
                    strncpy(log_message, pipe + 1, 511);
                    log_message[511] = '\0';

                    // 去除消息开头的空格
                    char *msg_start = log_message;
                    while (*msg_start == ' ') msg_start++;
                    if (msg_start != log_message) {
                        memmove(log_message, msg_start, strlen(msg_start) + 1);
                    }
                } else {
                    strncpy(log_message, level_and_message, 511);
                    log_message[511] = '\0';
                }
            }
        } else {
            // 简单处理
            strncpy(log_message, after_app, 511);
            log_message[511] = '\0';
        }
    } else {
        strncpy(log_message, raw_message, 511);
        log_message[511] = '\0';
    }
}

// Syslog服务器线程函数
void *syslog_server_thread(void *arg) {
    int syslog_fd;
    struct sockaddr_in syslog_addr;
    int opt = 1;

    // 创建syslog socket
    syslog_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (syslog_fd == 0) {
        perror("syslog socket创建失败");
        return NULL;
    }

    if (setsockopt(syslog_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("syslog setsockopt失败");
        close(syslog_fd);
        return NULL;
    }

    syslog_addr.sin_family = AF_INET;
    syslog_addr.sin_addr.s_addr = INADDR_ANY;
    syslog_addr.sin_port = htons(SYSLOG_PORT);

    if (bind(syslog_fd, (struct sockaddr *)&syslog_addr, sizeof(syslog_addr)) < 0) {
        perror("syslog bind失败");
        close(syslog_fd);
        return NULL;
    }

    printf("🚀 Syslog服务器启动！监听端口 %d\n", SYSLOG_PORT);

    while (1) {
        char buffer[4096] = {0};
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int bytes_read = recvfrom(syslog_fd, buffer, sizeof(buffer) - 1, 0,
                                 (struct sockaddr *)&client_addr, &client_len);

        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("收到syslog消息: %s\n", buffer);

            // 解析并存储syslog消息
            char timestamp[32] = {0};
            char hostname[64] = {0};
            char app_name[64] = {0};
            char message[512] = {0};
            char level[16] = {0};
            char func_name[128] = {0};
            int line_number = 0;

            parse_syslog_message(buffer, timestamp, hostname, app_name, message, level, func_name, &line_number);

            // 插入到数据库
            insert_syslog_message(timestamp, hostname, app_name, message, level, func_name, line_number);
        }
    }

    close(syslog_fd);
    return NULL;
}

// HTTP服务器主线程
void run_http_server() {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // 创建HTTP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("HTTP socket创建失败");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("HTTP setsockopt失败");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(HTTP_PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("HTTP bind失败");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("HTTP listen失败");
        exit(EXIT_FAILURE);
    }

    printf("🚀 HTTP服务器启动！访问 http://localhost:%d\n", HTTP_PORT);

    while (1) {
        int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("HTTP accept失败");
            continue;
        }

        char buffer[4096] = {0};
        int bytes_read = read(new_socket, buffer, sizeof(buffer) - 1);
        if (bytes_read <= 0) {
            close(new_socket);
            continue;
        }

        // 解析HTTP请求
        char *method = strtok(buffer, " \r\n");
        char *path = strtok(NULL, " \r\n");

        if (!method || !path) {
            dprintf(new_socket, "HTTP/1.1 400 Bad Request\r\n\r\n");
            close(new_socket);
            continue;
        }

        printf("HTTP请求: %s %s\n", method, path);

        // API路由处理
        if (strncmp(path, "/api/", 5) == 0) {
            handle_http_api_request(new_socket, method, path);
        } else {
            // 静态文件服务
            handle_http_static_request(new_socket, path);
        }

        close(new_socket);
    }
}

int main() {
    // 创建数据库表
    create_syslog_table();

    // 启动syslog服务器线程
    pthread_t syslog_thread;
    if (pthread_create(&syslog_thread, NULL, syslog_server_thread, NULL) != 0) {
        perror("创建syslog线程失败");
        exit(EXIT_FAILURE);
    }

    // 运行HTTP服务器主线程
    run_http_server();

    // 等待syslog线程结束（实际上不会结束）
    pthread_join(syslog_thread, NULL);

    return 0;
}
