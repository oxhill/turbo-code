#ifndef	OTHER_FUNCTIONS_H
#define	OTHER_FUNCTIONS_H

#include <stdlib.h>
#include "turbo_code_Log_MAP.h"

void gen_source(int *data, int length);

void AWGN(float *send, float *r, float sigma, int totallength);

void mgrns(double mean, double sigma, double seed, int n, double *a);

void read_encoded_data(const char* filename, int* trafficflow_for_decode, int* length);

void read_data(const char* filename, int* trafficflow_source, int* length);

int read_integers_from_file(const char* filename, int* array, int max_size);
#endif