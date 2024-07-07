#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>

/* 客户端 */

int main(void)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        fprintf(stderr, "socket error\n");
        return -1;
    }

    int serveraddr_len = 0;
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port  = htons(8080);
    serveraddr.sin_addr.s_addr = inet_addr("10.102.106.45");
    serveraddr_len = sizeof(serveraddr);

    int ret = connect(sockfd, (struct sockaddr *)&serveraddr, serveraddr_len);
    if (ret == -1) {
        fprintf(stderr, "connect erorr\n");
        return -1;
    }
    printf("与服务器连接成功\n");

    char buff[1024] = {0};
    // 收发数据
    while (1) {
        memset(buff, 0, 1024);
        // 在终端获取数据
        fgets(buff, 1024, stdin);

        // 给服务器发送数据
        if (-1 == send(sockfd, buff, 1024, 0)){
            fprintf(stderr, "send error\n");
            return -1;
        }

        // 接受服务器的应答
        if (-1 == recv(sockfd, buff, 1024, 0)) {
            fprintf(stderr, "recv error\n");
            return -1;
        }

        // 打印应答
        printf("应答: [%s]\n", buff);
    }

    // 关闭套接字
    close(sockfd);
}
