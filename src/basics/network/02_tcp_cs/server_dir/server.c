#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>


/* 创建流式套接字: 这相当于买了部手机*/
int create_sock_fd(void)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET: IPV4使用, SOCK_STREAM: TCP使用
    if (sockfd == -1) {
        fprintf(stderr, "create socket fd error\n");
        return -1;
    }
    return sockfd;
}

/* 填充服务器网络信息结构体, 并进行绑定: 绑定操作这相当于给手机插卡 */
int set_and_bind_server_sockfd_info(int sockfd)
{
    if (sockfd <= 0) {
        fprintf(stderr, "socketfd 小于或等于0\n");
        return -1;
    }
    //1. 填充服务器网络信息结构体      ---办卡
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET; //IPV4使用
    serveraddr.sin_port = htons(8080); // 主机字节序8080转换为网络字节序端口号, 标志这数据可以往这个8080端口的进程收发
    serveraddr.sin_addr.s_addr = inet_addr("10.102.106.45"); // 将点分十进制IP地址字符串转换为网络字节序的IP地址。 填本机IP地址
    int sockaddr_len = sizeof(serveraddr); //服务器网络信息结构体的大小

    // 2. 将套接字和网络信息结构体绑定 ---插卡
    int ret = bind(sockfd, (struct sockaddr *)&serveraddr, sockaddr_len);
    if (ret == -1) {
        fprintf(stderr, "bind error\n");
        return -1;
    }
    return 0;
}

/* 将套接字设置成被动监听状态: 这个操作相当于让卡具有接电话的功能(这种状态才能接受客户端的连接) */
int set_server_sockfd_listen_status(int sockfd, int connect_num)
{
    if (connect_num <= 0 || sockfd <= 0) {
        fprintf(stderr, "sockfd 或 客户端连接数 小于或等于0");
        return -1;
    }


    if (-1 == listen(sockfd, connect_num)) {
        fprintf(stderr, "listen error\n");
        return -1;
    }
    return 0;
}

/* 阻塞等待客户端连接,获取新的套接字。当有客户端连接后，就可以通过新的套接字进行数据收发 */
int wait_client_connect(int sockfd)
{
    if (sockfd <= 0) {
        fprintf(stderr, "sockfd is vali\n");
        return -1;
    }
    int acceptfd = accept(sockfd, NULL, NULL);
    if (acceptfd == -1) {
        fprintf(stderr, "accept error\n");
        return -1;
    }
    return acceptfd;
}

int main(void)
{
    int ret = -1;
    int sockfd = -1;
    int acceptfd = -1;

    // 创建套接字
    sockfd = create_sock_fd();
    if (sockfd == -1) {
        return -1;
    }

    // 填充服务器信息结构体，并绑定该结构体到fd
    ret = set_and_bind_server_sockfd_info(sockfd);
    if (ret == -1) {
        return -1;
    }

    // 设置fd为监听状态
    ret = set_server_sockfd_listen_status(sockfd, 5);
    if (ret == -1 ) {
        return -1;
    }

    // 阻塞等待客户端连接
    acceptfd = wait_client_connect(sockfd);
    if (ret == -1) {
        return -1;
    }

    printf("新的客户端连接\n");
    char buff[1024] = {0};

    // 收发数据
    while (1) {
        memset(buff, 0, 1024);
        int ret = read(acceptfd, buff, 1024);
        if (ret == -1) {
            fprintf(stderr, "read error \n");
            return -1;
        }
        printf("buff:[%s]\n", buff);
        // 组装应答
        strcat(buff, "----cwc");
        write(acceptfd, buff, 1024);
    }
    close(acceptfd);
    close(sockfd);

    return 0;
}
