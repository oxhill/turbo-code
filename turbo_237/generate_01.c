//#include <stdio.h>
//#include <stdlib.h>
//#include <time.h>
//
//#define BIT_STREAM_LENGTH 640  // 生成640个比特
//
//int main() {
//    FILE* file;
//    int bit_stream[BIT_STREAM_LENGTH];
//
//    // 使用当前时间作为随机数种子
//    srand((unsigned int)time(NULL));
//
//    // 打开文件用于写入
//    errno_t err = fopen_s(&file, "input.txt", "w");
//    if (err != 0) {
//        printf("无法打开文件\n");
//        return 1;
//    }
//
//
//    // 生成640个随机比特（0或1）
//    for (int i = 0; i < BIT_STREAM_LENGTH; i++) {
//        bit_stream[i] = rand() % 2;  // 生成0或1
//
//        // 写入文件，每个比特后面加一个空格
//        fprintf(file, "%d ", bit_stream[i]);
//    }
//
//    // 关闭文件
//    fclose(file);
//
//    printf("比特流已成功生成并保存在input.txt中\n");
//    return 0;
//}
