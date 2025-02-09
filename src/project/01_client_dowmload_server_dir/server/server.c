#include <stdio.h>
#include <stdlib.h>
#include <string.h> // strcat的头文件
#include <unistd.h> // read 和 write的头文件

#include <netinet/in.h> // sockaddr_in 的头文件

#include <sys/types.h>  // socket的头文件
#include <sys/socket.h> // socket的头文件
#include <arpa/inet.h>  // inet_addr的头文件

#define ERRLOG(msg) do{\
    printf("%s %s(%d):", __FILE__, __func__, __LINE__);\
    printf("\n");\
    perror(msg);\
    exit(0);\
}while(0)


struct tcp_server_info {
    int sock_fd;
    int accept_fd;
    struct sockaddr_in server_info;
    socklen_t server_info_len;
    struct sockaddr_in client_info; // 用于获取客户端信息
    socklen_t client_info_len;
};

/* 1. 获取套接字 */
int get_socket_fd(struct tcp_server_info *info)
{
    if (info == NULL) {
        ERRLOG("info is null\n");
        return -1;
    }
    info->sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (info->sock_fd < 0) {
        ERRLOG("create sock_fd failed\n");
        return -1;
    }
    return 0;
}

/* 2. 构建服务器结构体*/
int create_server_info(struct tcp_server_info *info)
{
    if (info == NULL) {
        ERRLOG("info is null\n");
        return -1;
    }
    memset(&info->server_info, 0, sizeof(info->server_info));
    info->server_info.sin_family = AF_INET;
    info->server_info.sin_port   = htons(8080);
    info->server_info.sin_addr.s_addr = inet_addr("10.102.106.45");
    info->server_info_len = sizeof(info->server_info);
    return 0;
}

/* 3. 设置套接字为被动监听状态 */
int set_status_listened_for_sockfd(struct tcp_server_info *info)
{
    if (info == NULL) {
        ERRLOG("info is null");
        return -1;
    }

    int ret = listen(info->sock_fd, 5);
    if (ret < 0) {
        ERRLOG("listen error\n");
        return -1;
    }
    return 0;
}

/* 4.将服务器结构体与套接字绑定 */
int bind_info_with_fd(struct tcp_server_info *info)
{
    if (info == NULL) {
        ERRLOG("info is NULL");
        return -1;
    }

    printf("fd: %d\n", info->sock_fd);
    printf("AF_INET：%d\n", info->server_info.sin_family);
    printf("port: %d\n", ntohs(info->server_info.sin_port));
    printf("addr: %s\n", inet_ntoa(info->server_info.sin_addr));
    printf("server_info len: %d\n", info->server_info_len);
    int ret = bind(info->sock_fd, (struct sockaddr *)&(info->server_info), info->server_info_len);
    if (ret < 0) {
        ERRLOG("bind error");
        return -1;
    }
    return 0;
}

/* 5. 开始监听用户连接 */
int check_client_connect(struct tcp_server_info *info)
{
    if (info == NULL) {
        ERRLOG("info is null");
        return -1;
    }
    int ret = -1;
    char buf[128] = {0};
    memset(buf, 0, sizeof(buf));
    memset(&info->client_info, 0, sizeof(info->client_info));
    int clinet_info_len = sizeof(info->client_info);

    while(1) {
        info->accept_fd = accept(info->sock_fd, (struct sockaddr *)&info->client_info, &clinet_info_len);
        if (info->accept_fd < 0) {
            ERRLOG("accept error\n");
            return -1;
        }
       ret = read(info->accept_fd, buf, sizeof(buf));
       if (ret < 0) {
           ERRLOG("client Disconnect\n");
           return -1;
       }
       printf("%s\n", buf);
       strcat(buf, "----cwc");
       write(info->sock_fd, buf, sizeof(buf));
    }
    return 0;
}

/* 获取客户端信息 */
int get_client_info(struct tcp_server_info *info) {
    if (info == NULL) {
        ERRLOG("info is null");
        return -1;
    }
    printf("%s\n", inet_ntoa(info->client_info.sin_addr));
    return 0;
}

void main(void)
{
    int ret = -1;
    struct tcp_server_info *info = (struct tcp_server_info *)malloc(sizeof(struct tcp_server_info));

    ret = get_socket_fd(info);
    if (ret < 0) {
        goto fail;
    }

    ret = create_server_info(info);
    if (ret < 0) {
        goto fail;
    }

    ret = set_status_listened_for_sockfd(info);
    if (ret < 0) {
        goto fail;
    }
    ret = bind_info_with_fd(info);
    if (ret < 0) {
        goto fail;
    }

    ret = check_client_connect(info);
    if (ret < 0) {
        goto fail;
    }
    return;

fail:
    free(info);
    info = NULL;
}
