#include <stdio.h>
#include <stdlib.h>
#pragma warning(disable: 4996)
void encodeToBinary(const unsigned char* input, FILE* outputFile) {
    while (*input) {
        // �ж� UTF-8 ���ֽ��ַ����ֽ���
        int bytes = 1;
        if ((*input & 0x80) == 0x00) { // 1�ֽ��ַ�
            bytes = 1;
        }
        else if ((*input & 0xE0) == 0xC0) { // 2�ֽ��ַ�
            bytes = 2;
        }
        else if ((*input & 0xF0) == 0xE0) { // 3�ֽ��ַ�
            bytes = 3;
        }
        else if ((*input & 0xF8) == 0xF0) { // 4�ֽ��ַ�
            bytes = 4;
        }

        // ���ÿ���ֽڵĶ����Ʊ�ʾ��д���ļ�
        for (int i = 0; i < bytes; i++) {
            for (int bit = 7; bit >= 0; bit--) {
                fprintf(outputFile, "%d ", (input[i] >> bit) & 1);  // ��ÿ��������λ���һ���ո�
            }
        }
        input += bytes;
    }
    fprintf(outputFile, "\n");  // ����
}

int utf8encode() {
    FILE* file = fopen("asciiinput.txt", "r");
    if (file == NULL) {
        perror("�޷����ļ�");
        return 1;
    }

    unsigned char input[1024];
    size_t len = fread(input, 1, sizeof(input) - 1, file);
    input[len] = '\0'; // ȷ���ַ����Կ��ַ���β
    fflush(file);  // ˢ���ļ����棬ȷ������д��

    fclose(file);

    // �򿪻򴴽� outputFile �ļ�
    FILE* outputFile = fopen("asciiencoded.txt", "w");
    if (outputFile == NULL) {
        perror("�޷�������ļ�");
        return 1;
    }

    printf("������01�����ѱ��浽 asciiencoded.txt\n");
    encodeToBinary(input, outputFile);
    fflush(file);  // ˢ���ļ����棬ȷ������д��

    fclose(outputFile);

    return 0;
}
