//#include <stdlib.h>
//#include <stdio.h>
//#include <math.h>
//#include <time.h>
//
//#include "turbo_code_Log_MAP.h"
//#include "other_functions.h"
//
//#define FRAME_LENGTH 320//(原来代码中是指生成的一帧输入中的01比特个数，我的这个代码里其实用不到他)
//#define MAX_SIZE 5096
//extern float rate_coding;
//extern TURBO_G turbo_g;
//
//void main()
//{
//    /*----------------------初始定义--------------------------------*/
//
//    int* trafficflow_source = NULL;
//    float* coded_trafficflow_source = NULL;
//    int traffic_source_length = 0;
//
//    float* trafficflow_for_decode = NULL;
//    int* array = NULL;
//    int* trafficflow_decoded = NULL;
//    int trafficflow_length;
//
//
//#define MAX_BITS 5096
//    //要先分配内存
//    if ((trafficflow_source = (int*)malloc(2 * FRAME_LENGTH * sizeof(int))) == NULL)
//    {
//        printf("\n fail to allocate memory of trafficflow_for_decode \n");
//        exit(1);
//    }
//
//    clock_t start, end;
//
//    int i, j, nf, ien;
//    // 在代码的开头定义一个新的文件指针
//    FILE* stream;    /* point to "simu_report.txt" which is used to record the results */
//    FILE* coded_stream; /* point to "coded_output.txt" which is used to record the coded results */
//    FILE* read; /*指向要读取的文件*/
//
//    errno_t err;
//    // 打开读取数据
//    int* traffic_array = NULL;  // 使用不同的变量名
//    traffic_array = (int*)malloc(MAX_SIZE * sizeof(int));  // 动态分配内存
//    const char* filename = "input.txt";  // txt文件名称
//    int num_elements = read_integers_from_file(filename, traffic_array, MAX_SIZE);  // 读取整数并返回元素个数
//
//    // 检查读取是否成功
//    if (num_elements > 0) {
//        printf("数组中元素个数: %d\n", num_elements);
//        printf("数组中的元素: ");
//        for (int i = 0; i < num_elements; i++) {
//            printf("%d ", traffic_array[i]);
//        }
//        printf("\n");
//    }
//    else {
//        printf("未能读取任何整数或文件读取出错。\n");
//    }
//
//    // 使用 fopen_s 打开文件
//    err = fopen_s(&stream, "simu_report.txt", "w");
//    if (err != 0 || stream == NULL) {
//        printf("\nError! Cannot open file simu_report.txt\n");
//        exit(1);
//    }
//
//    if (err == 0)
//        printf("\nwin\n");
//
//    TurboCodingInit();
//
//    /*====   output the simulation parameters =====*/
//    /* to screen */
//    printf("/*=================================*/\n");
//    printf("Turbo code simulation:\n");
//    printf("/*=================================*/\n");
//    printf("====log map decoder====\n");
//    printf("frame length : %d \n", num_elements);
//    printf("g = \n");
//    for (i = 0; i < turbo_g.N_num_row; i++)
//    {
//        for (j = 0; j < turbo_g.K_num_col; j++)
//        {
//            printf("%d ", *(turbo_g.g_matrix + i * turbo_g.K_num_col + j));
//        }
//        printf("\n");
//    }
//    if (TURBO_PUNCTURE)
//    {
//        printf("punctured to rate 1/2\n");
//    }
//    else
//    {
//        printf("unpunctured\n");
//    }
//    printf("iteration number : %d \n", N_ITERATION);
//    printf("/*=================================*/\n");
//
//    /* to "simu_report.txt"
//    这段代码负责将 Turbo 编码仿真的参数输出到名为 "simu_report.txt" 的文件中。
//    主要流程如下：
//    1. **打开文件流**：确保 `stream` 已正确打开指向 "simu_report.txt" 的文件。
//    2. **输出仿真信息**：
//        - 打印分隔符和仿真标题。
//        - 输出使用的解码器类型（在此为 log map decoder）。
//        - 打印帧长度（`traffic_source_length`）。
//    3. **输出生成矩阵**：
//        - 使用嵌套循环遍历 `turbo_g.g_matrix`，并将每个值写入文件中，确保格式整齐。
//    4. **输出 Eb/N0 值**：打印当前的信噪比值。
//    5. **输出穿孔状态**：
//        - 根据 `TURBO_PUNCTURE` 的值输出当前是否为穿孔模式。
//    6. **输出迭代次数**：打印迭代次数（`N_ITERATION`）。
//    7. **打印结束分隔符**：在文件末尾添加分隔符，以便于后续的仿真记录。
//    这段代码的目的在于生成详细的仿真报告，便于后续分析和调试。
//    */
//
//    start = clock();
//
//    // 打开 "coded_output.txt" 文件进行编码结果的记录
//    err = fopen_s(&coded_stream, "output.txt", "w");
//    if (err != 0 || coded_stream == NULL) {
//        printf("\nError! Cannot open file coded_output.txt\n");
//        exit(1);
//    }
//
//    /*------------------编码------------------*/
//     /*传递读取到的数组和元素个数到 TurboCodingTraffic*/
//    if ((coded_trafficflow_source = (float*)malloc(2 * num_elements * sizeof(float))) == NULL)//此处原来是2 * FRAME_LENGTH * sizeof(float))
//	{
//		printf("\n fail to allocate memory of coded_trafficflow_source \n");
//		exit(1);
//	}
//    TurboCodingTraffic(traffic_array, coded_trafficflow_source, &num_elements);
//
//    // 将编码后的数据写入文件
//    for (i = 0; i < num_elements; i++) {
//        fprintf(coded_stream, "%f ", coded_trafficflow_source[i]);
//    }
//    fprintf(coded_stream, "\n");
//
//    // 关闭文件流
//    fclose(coded_stream);
//
//    end = clock();
//    printf("\n%5f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
//
//    TurboCodingRelease();
//
//    fclose(stream);
//
//    // 释放已分配的内存
//    free(trafficflow_source);
//    free(coded_trafficflow_source);
//    free(trafficflow_for_decode);
//    free(trafficflow_decoded);
//
//}
