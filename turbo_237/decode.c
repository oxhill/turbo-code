//#include <stdlib.h>
//#include <stdio.h>
//#include <math.h>
//#include <time.h>
//
//#include "turbo_code_Log_MAP.h"
//#include "other_functions.h"
//
//#define FRAME_LENGTH    320  // �������ݵ�֡����
//
//extern float rate_coding;
//extern TURBO_G turbo_g;
//
//void main()
//{
//    /*----------------------��ʼ����--------------------------------*/
//    int* trafficflow_source = NULL;
//    float* coded_trafficflow_source = NULL;
//    int traffic_source_length;
//
//    float* trafficflow_for_decode = NULL;
//    int* trafficflow_decoded = NULL;
//    int trafficflow_length;
//
//    int* supflow_source = NULL;
//    float* coded_supflow_source = NULL;
//    int supflow_source_length;
//
//    float en, sgma;
//
//    // ����ȵ��趨����ʵ��Ӧ�û����ͽ��ն˵Ĳ������
//    float EbN0dB = 1.0;  // ����һ���̶��������ֵ
//
//    /*------------------------------------*/
//    // ��������������ʳ�ʼ��
//    int err_bit_num_traffic = 0;
//    float err_bit_rate_traffic = 0.0;
//    int err_bit_num_sup = 0;
//    float err_bit_rate_sup = 0.0;
//
//    clock_t start, end;
//
//    // �ڴ����
//    trafficflow_source = (int*)malloc(FRAME_LENGTH * sizeof(int));
//    if (!trafficflow_source) { printf("\n fail to allocate memory of trafficflow_source \n"); exit(1); }
//    coded_trafficflow_source = (float*)malloc(2 * FRAME_LENGTH * sizeof(float));
//    if (!coded_trafficflow_source) { printf("\n fail to allocate memory of coded_trafficflow_source \n"); exit(1); }
//
//    trafficflow_for_decode = (float*)malloc(2 * FRAME_LENGTH * sizeof(float));
//    if (!trafficflow_for_decode) { printf("\n fail to allocate memory of trafficflow_for_decode \n"); exit(1); }
//    trafficflow_decoded = (int*)malloc(FRAME_LENGTH * sizeof(int));
//    if (!trafficflow_decoded) { printf("\n fail to allocate memory of trafficflow_decoded\n"); exit(1); }
//
//    supflow_source = (int*)malloc(320 * sizeof(int));
//    if (!supflow_source) { printf("\n fail to allocate memory of supflow_source \n"); exit(1); }
//    coded_supflow_source = (float*)malloc(640 * sizeof(float));
//    if (!coded_supflow_source) { printf("\n fail to allocate memory of coded_supflow_source \n"); exit(1); }
//
//    // ���ļ�����ȡԴ����
//    FILE* input_file;
//    errno_t err_input = fopen_s(&input_file, "input115.txt", "r");
//    if (err_input != 0) {
//        printf("\nError! Cannot open input.txt\n");
//        exit(1);
//    }
//    for (int i = 0; i < FRAME_LENGTH; i++) {
//        fscanf_s(input_file, "%d", &trafficflow_source[i]);
//    }
//    fclose(input_file);
//
//    // ���б��벢�����������浽 output115.txt
//    FILE* output_file;
//    errno_t err_output = fopen_s(&output_file, "output115.txt", "w");
//    if (err_output != 0) {
//        printf("\nError! Cannot open output115.txt\n");
//        exit(1);
//    }
//
//    traffic_source_length = FRAME_LENGTH;
//    TurboCodingInit();  // ��ʼ��Turbo����
//
//    // ���ù̶��������
//    EbN0dB = 1.0;
//    en = (float)pow(10, (EbN0dB) / 10);  // ����Eb/N0�����ź�����
//    sgma = (float)(1.0 / sqrt(2 * rate_coding * en));  // ����������׼��
//
//    printf("Turbo code simulation:\n");
//    printf("Eb/N0 = %f \n", EbN0dB);
//    printf("Frame length: %d\n", traffic_source_length);
//    printf("Turbo Encoding: %d / %d\n", turbo_g.N_num_row, turbo_g.K_num_col);
//
//    // ��ʼ����������֡
//    start = clock();
//
//    // �������
//    TurboCodingTraffic(trafficflow_source, coded_trafficflow_source, &traffic_source_length);
//
//    // ��������д���ļ�
//    for (int i = 0; i < 2 * FRAME_LENGTH; i++) {
//        fprintf(output_file, "%f\n", coded_trafficflow_source[i]);
//    }
//    fclose(output_file);
//
//    // ��ȡ�������������ڽ���
//    FILE* input_coded_file;
//    errno_t err_input_coded = fopen_s(&input_coded_file, "output115.txt", "r");
//    if (err_input_coded != 0) {
//        printf("\nError! Cannot open output115.txt for reading\n");
//        exit(1);
//    }
//
//    for (int i = 0; i < 2 * FRAME_LENGTH; i++) {
//        fscanf_s(input_coded_file, "%f", &trafficflow_for_decode[i]);
//    }
//    fclose(input_coded_file);
//
//    // ����
//    trafficflow_length = 2 * FRAME_LENGTH;
//    TurboDecodingTraffic(trafficflow_for_decode, trafficflow_decoded, &trafficflow_length, EbN0dB);
//
//    // ����������
//    for (int i = 0; i < FRAME_LENGTH; i++) {
//        if (*(trafficflow_source + i) != *(trafficflow_decoded + i)) {
//            err_bit_num_traffic++;
//        }
//    }
//
//    // ����������
//    err_bit_rate_traffic = (float)err_bit_num_traffic / (traffic_source_length);
//    err_bit_rate_sup = (float)err_bit_num_sup / (320);
//
//    printf("Total Errors for Traffic: %d\n", err_bit_num_traffic);
//    printf("Bit Error Rate for Traffic: %f\n", err_bit_rate_traffic);
//    printf("Total Errors for Supflow: %d\n", err_bit_num_sup);
//    printf("Bit Error Rate for Supflow: %f\n", err_bit_rate_sup);
//
//    // ����ʱ��
//    end = clock();
//    printf("Time taken: %f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
//
//    // �ͷ���Դ
//    TurboCodingRelease();
//
//    free(trafficflow_source);
//    free(coded_trafficflow_source);
//    free(trafficflow_for_decode);
//    free(trafficflow_decoded);
//    free(supflow_source);
//    free(coded_supflow_source);
//}
