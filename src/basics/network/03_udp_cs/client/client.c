#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_IP "10.102.106.46"
#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // 创建 UDP 套接字
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址信息
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    printf("Enter message to send to server: ");
    fgets(buffer, BUFFER_SIZE, stdin);

    // 发送数据到服务器
    sendto(sockfd, buffer, strlen(buffer), 0,
           (const struct sockaddr *)&server_addr, sizeof(server_addr));

    printf("Message sent to server.\n");

    // 接收服务器响应
    ssize_t bytes_received = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);
    if (bytes_received < 0) {
        perror("Recvfrom failed");
    } else {
        buffer[bytes_received] = '\0'; // 确保字符串以 null 结尾
        printf("Server response: %s", buffer);
    }

    close(sockfd);
    return 0;
}
