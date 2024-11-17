
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#pragma warning(disable: 4996)

// ��8λ�Ķ������ַ���ת��Ϊ�ֽڣ��ַ���
unsigned char binary_to_char(const char* binary) {
    unsigned char ch = 0;
    for (int i = 0; i < 8; ++i) {
        ch = (ch << 1) | (binary[i] - '0');
    }
    return ch;
}

void decode_text_from_binary_file(FILE* file, FILE* output_file) {
    char binary[9];  // �洢һ��8λ�Ķ������ַ��������� '\0' ��ֹ��
    binary[8] = '\0';

    int bit_count = 0;
    while (!feof(file)) {
        int bit;
        if (fscanf(file, "%d", &bit) == 1) {  // ��ȡ����λ
            binary[bit_count] = '0' + bit;
            bit_count++;

            if (bit_count == 8) {  // ����ȡ��8λʱ��ת��Ϊ�ַ�
                unsigned char ch = binary_to_char(binary);
                fputc(ch, output_file);  // д�뵽�ļ�
                bit_count = 0;  // ���ü�����
            }
        }
    }

    // ����Ƿ���δ�����λ����λ������8�ı���������£�
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

    FILE* output_file = fopen("final_output.txt", "w");  // ������ļ�
    if (!output_file) {
        perror("Error opening output file");
        fclose(file);
        return 1;
    }

    decode_text_from_binary_file(file, output_file);  // ���������ַ�д�뵽 final_output.txt
    fflush(output_file);  // ˢ���ļ����棬ȷ������д��
    fclose(file);
    fclose(output_file);  // �ر�����ļ�

    return 0;
}


//#include <stdio.h> 
//#include <stdlib.h>
//#include <string.h>
//#pragma warning(disable: 4996)
//// ��8λ�Ķ������ַ���ת��Ϊ�ֽڣ��ַ���
//unsigned char binary_to_char(const char* binary) {
//    unsigned char ch = 0;
//    for (int i = 0; i < 8; ++i) {
//        ch = (ch << 1) | (binary[i] - '0');
//    }
//    return ch;
//}
//
//void decode_text_from_binary_file(FILE* file) {
//    char binary[9];  // �洢һ��8λ�Ķ������ַ��������� '\0' ��ֹ��
//    binary[8] = '\0';
//
//    int bit_count = 0;
//    while (!feof(file)) {
//        int bit;
//        if (fscanf(file, "%d", &bit) == 1) {  // ��ȡ����λ
//            binary[bit_count] = '0' + bit;
//            bit_count++;
//
//            if (bit_count == 8) {  // ����ȡ��8λʱ��ת��Ϊ�ַ�
//                unsigned char ch = binary_to_char(binary);
//                putchar(ch);  // �����Ӧ���ַ�
//                bit_count = 0;  // ���ü�����
//            }
//        }
//    }
//
//    // ����Ƿ���δ�����λ����λ������8�ı���������£�
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
//    fflush(file);  // ˢ���ļ����棬ȷ������д��
//    fclose(file);
//    return 0;
//}
