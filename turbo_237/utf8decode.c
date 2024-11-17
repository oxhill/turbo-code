
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#pragma warning(disable: 4996)

// 将8位的二进制字符串转换为字节（字符）
unsigned char binary_to_char(const char* binary) {
    unsigned char ch = 0;
    for (int i = 0; i < 8; ++i) {
        ch = (ch << 1) | (binary[i] - '0');
    }
    return ch;
}

void decode_text_from_binary_file(FILE* file, FILE* output_file) {
    char binary[9];  // 存储一个8位的二进制字符串，加上 '\0' 终止符
    binary[8] = '\0';

    int bit_count = 0;
    while (!feof(file)) {
        int bit;
        if (fscanf(file, "%d", &bit) == 1) {  // 读取单个位
            binary[bit_count] = '0' + bit;
            bit_count++;

            if (bit_count == 8) {  // 当读取到8位时，转换为字符
                unsigned char ch = binary_to_char(binary);
                fputc(ch, output_file);  // 写入到文件
                bit_count = 0;  // 重置计数器
            }
        }
    }

    // 检查是否有未处理的位（在位数不是8的倍数的情况下）
    if (bit_count > 0) {
        fprintf(output_file, "\nWarning: Incomplete byte at end of file.\n");
    }
}

int utf8decode() {
    FILE* file = fopen("turbodecoded.txt", "r");
    if (!file) {
        perror("Error opening input file");
        return 1;
    }

    FILE* output_file = fopen("final_output.txt", "w");  // 打开输出文件
    if (!output_file) {
        perror("Error opening output file");
        fclose(file);
        return 1;
    }

    decode_text_from_binary_file(file, output_file);  // 将解码后的字符写入到 final_output.txt
    fflush(output_file);  // 刷新文件缓存，确保数据写入
    fclose(file);
    fclose(output_file);  // 关闭输出文件

    return 0;
}


//#include <stdio.h> 
//#include <stdlib.h>
//#include <string.h>
//#pragma warning(disable: 4996)
//// 将8位的二进制字符串转换为字节（字符）
//unsigned char binary_to_char(const char* binary) {
//    unsigned char ch = 0;
//    for (int i = 0; i < 8; ++i) {
//        ch = (ch << 1) | (binary[i] - '0');
//    }
//    return ch;
//}
//
//void decode_text_from_binary_file(FILE* file) {
//    char binary[9];  // 存储一个8位的二进制字符串，加上 '\0' 终止符
//    binary[8] = '\0';
//
//    int bit_count = 0;
//    while (!feof(file)) {
//        int bit;
//        if (fscanf(file, "%d", &bit) == 1) {  // 读取单个位
//            binary[bit_count] = '0' + bit;
//            bit_count++;
//
//            if (bit_count == 8) {  // 当读取到8位时，转换为字符
//                unsigned char ch = binary_to_char(binary);
//                putchar(ch);  // 输出对应的字符
//                bit_count = 0;  // 重置计数器
//            }
//        }
//    }
//
//    // 检查是否有未处理的位（在位数不是8的倍数的情况下）
//    if (bit_count > 0) {
//        printf("\nWarning: Incomplete byte at end of file.\n");
//    }
//}
//
//int utf8decode() {
//    FILE* file = fopen("turbodecoded.txt", "r");
//    if (!file) {
//        perror("Error opening file");
//        return 1;
//    }
//
//    decode_text_from_binary_file(file);
//    fflush(file);  // 刷新文件缓存，确保数据写入
//    fclose(file);
//    return 0;
//}
