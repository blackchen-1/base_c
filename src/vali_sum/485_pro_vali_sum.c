#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/*
01 33 01 50 08 60 19 57 56 FB 00 00 00 03 B1 04
soh  是 01
type 是 33
id   是 01
cmd  是 50
dlen 是 08
data 是 60 19 58 56 FB 00 00 00
etx  是 03
check是 B1
eot  是 04
*/


struct vg485_package {
     unsigned char soh;      // 起始字符
     unsigned char type;     // 设备类型
     unsigned char id;       // 地址ID
     unsigned char cmd;      // 命令
     uint16_t dlen;          // 数据长度
     unsigned char etx;      // 数据结束
     unsigned char check;    // 数据校验位
     unsigned char eot;      // 结束字符
     unsigned char data[0];  // 数据数组
};

/* 计算485一拖多协议， 协议校验值 */
static int vg485_get_vali_sum(struct vg485_package *pack)
{
     if (!pack) {
         fprintf(stderr, "param is null\n");
         return -1;
     }

     int sum = 0;
     int i = 0;

     sum += pack->soh;   // 1
     sum += pack->type;  // 33
     sum += pack->id;    // 1 - 255
     sum += pack->cmd;   // 01 - 60
     sum += (pack->dlen & 0xff); // 两个字节存储数据长度
     sum += (pack->dlen & 0xff00) >> 8;

     for (i = 0; i < pack->dlen; i++) {
         sum += pack->data[i];
     }
     sum += pack->etx; // 数据结束
     return (uint8_t) (sum & 0xff); // 所有数据的累加和， 取第一字节作为校验
}

// 解析连续字符串为字节数组
static void parse_continuous_input(const char *input_str, unsigned char *output, size_t max_length) {
    size_t len = strlen(input_str);
    if (len % 2 != 0 || len > max_length * 2) {
        printf("Error: Invalid continuous input format\n");
        exit(-1);
    }

    for (size_t i = 0; i < len; i += 2) {
        sscanf(&input_str[i], "%2hhx", &output[i / 2]);
    }
}

/* 这个填充的判断条件，仅在协议正确，但缺少数据长度0x00的时候好用 */
static void fill_missing_high_byte(unsigned char *input, size_t length) {
    int tmp = -1;
    int i = -1;
    if (length == 0) return;

    if (input[5] != 0 || input[14] != 0x03 || input[16] != 0x04) {
        memmove(&input[6], &input[5], length - 1);
        input[5] = 0;
    }
}

int main(int argc, char *argv[])
{
    // 检查命令行参数数量
    if (argc < 2) {
        printf("Usage: %s <hex_string> OR %s <byte1> <byte2> ... <byteN>\n", argv[0], argv[0]);
        return -1;
    }

    unsigned char input[50]; // 假设最多 50 字节
    memset(input, 0, sizeof(input));

    // 判断输入形式
    if (argc == 2) {
        // 连续形式：./a.out 013301500860195756FB00000003B104
        parse_continuous_input(argv[1], input, sizeof(input));
    } else {
        // 分隔形式：./a.out 01 33 01 50 08 60 19 57 56 FB 00 00 00 03 B1 04
        if (argc < 17) { // 至少需要 程序名 + 16 个字节
            printf("Error: Not enough arguments for separated format\n");
            return -1;
        }

        for (int i = 0; i < 16; i++) {
            if (argc != 17 && input[16] != 0x04) {

            }
            sscanf(argv[i + 1], "%hhx", &input[i]); // 将字符串转换为十六进制字节
        }
    }

    if (input[16] != 0x04) {
        return -1;
    }

    /* 如果缺失数据长度高位0x00的话(按道理应该是0x80 0x00)，自动填充 */
    /* 这个填充的判断条件，仅在协议正确，但缺少数据长度0x00的时候好用 */
    fill_missing_high_byte(input, 16);

    // 动态分配内存以容纳整个结构体和数据
    size_t total_size = sizeof(struct vg485_package) + 8; // 8 是 data 的长度
    struct vg485_package *pack = (struct vg485_package *)malloc(total_size);

    if (!pack) {
        printf("Error: Memory allocation failed\n");
        return -1;
    }

    // 填充结构体字段
    pack->soh = input[0];   // 0x01
    pack->type = input[1];  // 0x33
    pack->id = input[2];    // 0x01
    pack->cmd = input[3];   // 0x50
    pack->dlen = input[4] | (input[5] << 8); // 小端存储，组合 dlen
    memcpy(pack->data, &input[6], 8);        // 复制 data
    pack->etx = input[14];  // 0x03
    pack->check = input[15]; // 0xB1
    pack->eot = input[16];   // 0x04

    // 调用校验函数
    int checksum = vg485_get_vali_sum(pack);
    printf("Calculated checksum: 0x%02X\n", checksum);

    /* // 检查校验值是否匹配 */
    /* if (checksum == pack->check) { */
    /*     printf("!\n"); */
    /* } else { */
    /*     printf("Checksum does not match!\n"); */
    /* } */

    // 释放内存
    free(pack);

    return 0;
}
