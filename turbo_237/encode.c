//#include <stdlib.h>
//#include <stdio.h>
//#include <math.h>
//#include <time.h>
//
//#include "turbo_code_Log_MAP.h"
//#include "other_functions.h"
//
//#define FRAME_LENGTH 320//(ԭ����������ָ���ɵ�һ֡�����е�01���ظ������ҵ������������ʵ�ò�����)
//#define MAX_SIZE 5096
//extern float rate_coding;
//extern TURBO_G turbo_g;
//
//void main()
//{
//    /*----------------------��ʼ����--------------------------------*/
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
//    //Ҫ�ȷ����ڴ�
//    if ((trafficflow_source = (int*)malloc(2 * FRAME_LENGTH * sizeof(int))) == NULL)
//    {
//        printf("\n fail to allocate memory of trafficflow_for_decode \n");
//        exit(1);
//    }
//
//    clock_t start, end;
//
//    int i, j, nf, ien;
//    // �ڴ���Ŀ�ͷ����һ���µ��ļ�ָ��
//    FILE* stream;    /* point to "simu_report.txt" which is used to record the results */
//    FILE* coded_stream; /* point to "coded_output.txt" which is used to record the coded results */
//    FILE* read; /*ָ��Ҫ��ȡ���ļ�*/
//
//    errno_t err;
//    // �򿪶�ȡ����
//    int* traffic_array = NULL;  // ʹ�ò�ͬ�ı�����
//    traffic_array = (int*)malloc(MAX_SIZE * sizeof(int));  // ��̬�����ڴ�
//    const char* filename = "input.txt";  // txt�ļ�����
//    int num_elements = read_integers_from_file(filename, traffic_array, MAX_SIZE);  // ��ȡ����������Ԫ�ظ���
//
//    // ����ȡ�Ƿ�ɹ�
//    if (num_elements > 0) {
//        printf("������Ԫ�ظ���: %d\n", num_elements);
//        printf("�����е�Ԫ��: ");
//        for (int i = 0; i < num_elements; i++) {
//            printf("%d ", traffic_array[i]);
//        }
//        printf("\n");
//    }
//    else {
//        printf("δ�ܶ�ȡ�κ��������ļ���ȡ����\n");
//    }
//
//    // ʹ�� fopen_s ���ļ�
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
//    ��δ��븺�� Turbo �������Ĳ����������Ϊ "simu_report.txt" ���ļ��С�
//    ��Ҫ�������£�
//    1. **���ļ���**��ȷ�� `stream` ����ȷ��ָ�� "simu_report.txt" ���ļ���
//    2. **���������Ϣ**��
//        - ��ӡ�ָ����ͷ�����⡣
//        - ���ʹ�õĽ��������ͣ��ڴ�Ϊ log map decoder����
//        - ��ӡ֡���ȣ�`traffic_source_length`����
//    3. **������ɾ���**��
//        - ʹ��Ƕ��ѭ������ `turbo_g.g_matrix`������ÿ��ֵд���ļ��У�ȷ����ʽ���롣
//    4. **��� Eb/N0 ֵ**����ӡ��ǰ�������ֵ��
//    5. **�������״̬**��
//        - ���� `TURBO_PUNCTURE` ��ֵ�����ǰ�Ƿ�Ϊ����ģʽ��
//    6. **�����������**����ӡ����������`N_ITERATION`����
//    7. **��ӡ�����ָ���**�����ļ�ĩβ��ӷָ������Ա��ں����ķ����¼��
//    ��δ����Ŀ������������ϸ�ķ��汨�棬���ں��������͵��ԡ�
//    */
//
//    start = clock();
//
//    // �� "coded_output.txt" �ļ����б������ļ�¼
//    err = fopen_s(&coded_stream, "output.txt", "w");
//    if (err != 0 || coded_stream == NULL) {
//        printf("\nError! Cannot open file coded_output.txt\n");
//        exit(1);
//    }
//
//    /*------------------����------------------*/
//     /*���ݶ�ȡ���������Ԫ�ظ����� TurboCodingTraffic*/
//    if ((coded_trafficflow_source = (float*)malloc(2 * num_elements * sizeof(float))) == NULL)//�˴�ԭ����2 * FRAME_LENGTH * sizeof(float))
//	{
//		printf("\n fail to allocate memory of coded_trafficflow_source \n");
//		exit(1);
//	}
//    TurboCodingTraffic(traffic_array, coded_trafficflow_source, &num_elements);
//
//    // ������������д���ļ�
//    for (i = 0; i < num_elements; i++) {
//        fprintf(coded_stream, "%f ", coded_trafficflow_source[i]);
//    }
//    fprintf(coded_stream, "\n");
//
//    // �ر��ļ���
//    fclose(coded_stream);
//
//    end = clock();
//    printf("\n%5f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
//
//    TurboCodingRelease();
//
//    fclose(stream);
//
//    // �ͷ��ѷ�����ڴ�
//    free(trafficflow_source);
//    free(coded_trafficflow_source);
//    free(trafficflow_for_decode);
//    free(trafficflow_decoded);
//
//}
