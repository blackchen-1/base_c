#include <stdio.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

/*
主机字节序概念:
不同类型CPU的主机中, 内存存储多字节数据(例如:整型，浮点型等)时, 字节的存储顺序。 这称为主机字节序。
而主机字节序又分为 大端序 和 小端序两种类型。

1. 小端字节序:
                 将数据的低位字节存储在地址的低字节，数据的高位字节存储在地址的高字节。
2. 大端字节序:
                 将数据的高位字节存储在地址的低字节，数据的低位字节存储在地址的高字节。
注:
    （1）在网络传输中网络字节序通常使用的是大端序。
    （2）什么时候需要考虑字节序转换呢?
            1> 如果数据是1字节的，无需考虑字节序的问题, 若为多字节时，则需要考虑。
            2> 如果明确通信双方的主机字节序一样，也可以不考虑。
            3> 网络数据传输时:
                在进行网络数据交互时，规定的网络字节序通常为大端序，而我们主机为小端序时，则需要将字节序转为大端序。
                在接受时，若主机字节序也为小端序， 接受到数据需要从网络字节序(大端序)转换为本主机的主机字节序。
*/


/*
 *  一. 接口介绍：用于测试主机的主机字节序时大端字节序还是小端字节序
 */
void test_host_endianess(void)
{
    int m = 0x12345678;
    // char *p 存的是m的首地址(即低地址)。
    char *p = (char *)&m;

    if (*p == 0x78) {
        // 若低地址存的是0x78，本地主机则是小端序
        printf("本地主机是小端序\n");
    } else if (*p == 0x12) {
        printf("本地主机是大端序\n");
    }
    return;
}

/*
 * 二. 接口介绍用于字节序(大端和小端)之间进行转换
        1. 字节序转换函数:
            接口名（字母表示含义）: h  host  主机   n  network  网络    l   长    s  短
                uint32_t htonl(uint32_t hostlong);//主机转网络  4字节
                uint16_t htons(uint16_t hostshort);//主机转网络  2字节
                uint32_t ntohl(uint32_t netlong);//网络转主机   4字节
                uint16_t ntohs(uint16_t netshort);//网络转主机  2字节
        2. 本机为小端序
 */
void test_host_endianess_change(void)
{
    unsigned int m = 0x12345678;
    unsigned int n = htonl(m);
    printf("%#x -- > %#x\n", m, n);
    return;
}

/*
 * 三. 接口介绍: 将点分十进制IP地址字符串 转换为 32位的网络字节序整数值
 *     1. 使用到的C的函数 inet_addr
 *          函数完整定义:
 *          in_addr_t inet_addr(const char *strptr);
 * */
void ipstr_to_NetEndianess(void)
{
    unsigned char ip_str[] = "10.102.104.57";
    unsigned int  ip_int = 0;

    // 将字符串的IP地址转换为网络字节序的32位整数值
    ip_int = inet_addr(ip_str);
    unsigned char *p = (unsigned char *)&ip_int;
    printf("%d.%d.%d.%d\n", *p, *(p+1), *(p+2), *(p+3));
    return;
}

/*
 * 四. 接口介绍: 将32位整数网络字节序转换为点分十进制IP地址字符串
 *          1. 使用到C的函数: inet_ntoa
 *          函数完整定义:
 *          char *inet_ntoa(stuct in_addr inaddr);
 * */
void NetEndianess_to_ipstr(void)
{
    uint32_t ip_int = 963143178; // 10.102.104.57的网络字节序整数值表示

    // 将网络字节序整数转换为点分十进制的IP地址字符串
    struct in_addr addr;
    addr.s_addr = ip_int;
    char *ip_str = inet_ntoa(addr);

    if (ip_str == NULL) {
        printf("转换失败\n");
    }

    printf("IP地址字符串： %s\n", ip_str);
    return;
}

void main(void)
{
    /* test_host_endianess(); */
    /* test_host_endianess_change(); */
    /* ipstr_to_NetEndianess(); */
    NetEndianess_to_ipstr();
}
