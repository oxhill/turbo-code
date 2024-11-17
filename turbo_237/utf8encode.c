#include <stdio.h>
#include <stdlib.h>
#pragma warning(disable: 4996)
void encodeToBinary(const unsigned char* input, FILE* outputFile) {
    while (*input) {
        // 判断 UTF-8 多字节字符的字节数
        int bytes = 1;
        if ((*input & 0x80) == 0x00) { // 1字节字符
            bytes = 1;
        }
        else if ((*input & 0xE0) == 0xC0) { // 2字节字符
            bytes = 2;
        }
        else if ((*input & 0xF0) == 0xE0) { // 3字节字符
            bytes = 3;
        }
        else if ((*input & 0xF8) == 0xF0) { // 4字节字符
            bytes = 4;
        }

        // 输出每个字节的二进制表示并写入文件
        for (int i = 0; i < bytes; i++) {
            for (int bit = 7; bit >= 0; bit--) {
                fprintf(outputFile, "%d ", (input[i] >> bit) & 1);  // 在每个二进制位后加一个空格
            }
        }
        input += bytes;
    }
    fprintf(outputFile, "\n");  // 换行
}

int utf8encode() {
    FILE* file = fopen("asciiinput.txt", "r");
    if (file == NULL) {
        perror("无法打开文件");
        return 1;
    }

    unsigned char input[1024];
    size_t len = fread(input, 1, sizeof(input) - 1, file);
    input[len] = '\0'; // 确保字符串以空字符结尾
    fflush(file);  // 刷新文件缓存，确保数据写入

    fclose(file);

    // 打开或创建 outputFile 文件
    FILE* outputFile = fopen("asciiencoded.txt", "w");
    if (outputFile == NULL) {
        perror("无法打开输出文件");
        return 1;
    }

    printf("编码后的01序列已保存到 asciiencoded.txt\n");
    encodeToBinary(input, outputFile);
    fflush(file);  // 刷新文件缓存，确保数据写入

    fclose(outputFile);

    return 0;
}
