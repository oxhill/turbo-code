//#include <stdio.h>
//#include <stdlib.h>
//#include <time.h>
//
//#define BIT_STREAM_LENGTH 640  // ����640������
//
//int main() {
//    FILE* file;
//    int bit_stream[BIT_STREAM_LENGTH];
//
//    // ʹ�õ�ǰʱ����Ϊ���������
//    srand((unsigned int)time(NULL));
//
//    // ���ļ�����д��
//    errno_t err = fopen_s(&file, "input.txt", "w");
//    if (err != 0) {
//        printf("�޷����ļ�\n");
//        return 1;
//    }
//
//
//    // ����640��������أ�0��1��
//    for (int i = 0; i < BIT_STREAM_LENGTH; i++) {
//        bit_stream[i] = rand() % 2;  // ����0��1
//
//        // д���ļ���ÿ�����غ����һ���ո�
//        fprintf(file, "%d ", bit_stream[i]);
//    }
//
//    // �ر��ļ�
//    fclose(file);
//
//    printf("�������ѳɹ����ɲ�������input.txt��\n");
//    return 0;
//}
