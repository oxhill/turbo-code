#include <stdio.h>

void print_bit_streams(int* trafficflow_source, int traffic_source_length, int* trafficflow_decoded, int decoded_length) {
    // ��ӡ���������
    printf("Trafficflow Source: ");
    for (int i = 0; i < traffic_source_length; i++) {
        printf("%d ", trafficflow_source[i]);
    }
    printf("\n");

    // ��ӡ�����ı�����
    printf("Trafficflow Decoded: ");
    for (int i = 0; i < decoded_length; i++) {
        printf("%d ", trafficflow_decoded[i]);
    }
    printf("\n");
}
