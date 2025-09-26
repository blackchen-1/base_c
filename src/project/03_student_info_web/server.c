// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#define PORT 8080
#define HTML_FILE "index.html"
#define JSON_FILE "data.json"
#define MANAGEMENT_HTML_FILE "student_management.html"

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

// 解析POST数据
void parse_post_data(char *post_data, char *name, char *gender, char *age, char *class_name) {
    char *token = strtok(post_data, "&");
    while (token != NULL) {
        if (strncmp(token, "name=", 5) == 0) {
            url_decode(token + 5, name);
        } else if (strncmp(token, "gender=", 7) == 0) {
            url_decode(token + 7, gender);
        } else if (strncmp(token, "age=", 4) == 0) {
            url_decode(token + 4, age);
        } else if (strncmp(token, "class_name=", 11) == 0) {
            url_decode(token + 11, class_name);
        }
        token = strtok(NULL, "&");
    }
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

// 生成 JSON 文件
void generate_json() {
    MYSQL *conn = connect_db();
    if (!conn) return;

    MYSQL_RES *res;
    MYSQL_ROW row;
    FILE *fp;

    if (mysql_query(conn, "SELECT id, name, gender, age, class_name FROM students")) {
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
        fprintf(fp, "  {\"id\":%s,\"name\":\"%s\",\"gender\":\"%s\",\"age\":%s,\"class_name\":\"%s\"}",
                row[0], row[1], row[2], row[3], row[4]);
        first = 0;
    }
    fprintf(fp, "\n]\n");

    fclose(fp);
    mysql_free_result(res);
    mysql_close(conn);
}

// 添加学生
int add_student(const char *name, const char *gender, const char *age, const char *class_name) {
    MYSQL *conn = connect_db();
    if (!conn) return 0;

    char query[512];
    snprintf(query, sizeof(query),
             "INSERT INTO students (name, gender, age, class_name) VALUES ('%s', '%s', %s, '%s')",
             name, gender, age, class_name);

    int result = mysql_query(conn, query);
    if (result) {
        fprintf(stderr, "插入失败: %s\n", mysql_error(conn));
    }

    mysql_close(conn);
    return result == 0;
}

// 删除学生
int delete_student(const char *id) {
    MYSQL *conn = connect_db();
    if (!conn) return 0;

    char query[256];
    snprintf(query, sizeof(query), "DELETE FROM students WHERE id = %s", id);

    int result = mysql_query(conn, query);
    if (result) {
        fprintf(stderr, "删除失败: %s\n", mysql_error(conn));
    }

    mysql_close(conn);
    return result == 0;
}

// 更新学生信息
int update_student(const char *id, const char *name, const char *gender, const char *age, const char *class_name) {
    MYSQL *conn = connect_db();
    if (!conn) return 0;

    char query[512];
    snprintf(query, sizeof(query),
             "UPDATE students SET name='%s', gender='%s', age=%s, class_name='%s' WHERE id=%s",
             name, gender, age, class_name, id);

    int result = mysql_query(conn, query);
    if (result) {
        fprintf(stderr, "更新失败: %s\n", mysql_error(conn));
    }

    mysql_close(conn);
    return result == 0;
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

// 发送错误响应
void send_error(int socket, int code, const char *message) {
    char response[256];
    snprintf(response, sizeof(response),
             "{\"error\": \"%s\", \"code\": %d}", message, code);
    send_json_response(socket, response);
}

// 处理API请求
void handle_api_request(int socket, const char *method, const char *path, char *post_data) {
    // 获取学生列表
    if (strcmp(path, "/api/students") == 0 && strcmp(method, "GET") == 0) {
        generate_json();
        char *response = NULL;
        int len = read_file(JSON_FILE, &response);
        if (len) {
            send_response(socket, "application/json", response, len);
            free(response);
        } else {
            send_error(socket, 500, "无法读取学生数据");
        }
        return;
    }

    // 添加学生
    if (strcmp(path, "/api/students") == 0 && strcmp(method, "POST") == 0) {
        char name[100] = {0};
        char gender[10] = {0};
        char age[10] = {0};
        char class_name[100] = {0};

        parse_post_data(post_data, name, gender, age, class_name);

        if (add_student(name, gender, age, class_name)) {
            send_json_response(socket, "{\"success\": true, \"message\": \"学生添加成功\"}");
        } else {
            send_error(socket, 500, "添加学生失败");
        }
        return;
    }

    // 删除学生
    if (strncmp(path, "/api/students/", 14) == 0 && strcmp(method, "DELETE") == 0) {
        const char *id = path + 14;
        if (delete_student(id)) {
            send_json_response(socket, "{\"success\": true, \"message\": \"学生删除成功\"}");
        } else {
            send_error(socket, 500, "删除学生失败");
        }
        return;
    }

    // 更新学生
    if (strncmp(path, "/api/students/", 14) == 0 && strcmp(method, "PUT") == 0) {
        const char *id = path + 14;
        char name[100] = {0};
        char gender[10] = {0};
        char age[10] = {0};
        char class_name[100] = {0};

        parse_post_data(post_data, name, gender, age, class_name);

        if (update_student(id, name, gender, age, class_name)) {
            send_json_response(socket, "{\"success\": true, \"message\": \"学生信息更新成功\"}");
        } else {
            send_error(socket, 500, "更新学生信息失败");
        }
        return;
    }

    // 404 Not Found
    dprintf(socket, "HTTP/1.1 404 Not Found\r\n\r\n{\"error\": \"API接口未找到\"}");
}

// 简单的 HTTP 服务器
int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // 创建 socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket创建失败");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt失败");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind失败");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen失败");
        exit(EXIT_FAILURE);
    }

    printf("🚀 C Web 服务器启动！访问 http://localhost:%d\n", PORT);

    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("accept失败");
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

        // 处理POST数据
        char post_data[2048] = {0};
        if (strcmp(method, "POST") == 0 || strcmp(method, "PUT") == 0) {
            // 查找Content-Length
            char *content_length_str = strstr(buffer, "Content-Length:");
            if (content_length_str) {
                int content_length = atoi(content_length_str + 15);
                // 读取剩余数据
                if (content_length > 0 && content_length < sizeof(post_data)) {
                    char *body_start = strstr(buffer, "\r\n\r\n");
                    if (body_start) {
                        strncpy(post_data, body_start + 4, content_length);
                    }
                }
            }
        }

        printf("请求: %s %s\n", method, path);

        // API路由处理
        if (strncmp(path, "/api/", 5) == 0) {
            handle_api_request(new_socket, method, path, post_data);
        }
        // 静态文件服务
        else if (strcmp(path, "/data.json") == 0) {
            generate_json();  // 每次请求都重新生成 JSON
            char *response = NULL;
            int len = read_file(JSON_FILE, &response);
            if (len) {
                send_response(new_socket, "application/json", response, len);
                free(response);
            } else {
                dprintf(new_socket, "HTTP/1.1 404 Not Found\r\n\r\n404");
            }
        } else if (strcmp(path, "/student_management.html") == 0) {
            char *response = NULL;
            int len = read_file(MANAGEMENT_HTML_FILE, &response);
            if (len) {
                send_response(new_socket, "text/html", response, len);
                free(response);
            } else {
                dprintf(new_socket, "HTTP/1.1 404 Not Found\r\n\r\n404");
            }
        } else {
            // 默认返回 index.html
            char *response = NULL;
            int len = read_file(HTML_FILE, &response);
            if (len) {
                send_response(new_socket, "text/html", response, len);
                free(response);
            } else {
                dprintf(new_socket, "HTTP/1.1 404 Not Found\r\n\r\n404");
            }
        }

        close(new_socket);
    }

    return 0;
}
