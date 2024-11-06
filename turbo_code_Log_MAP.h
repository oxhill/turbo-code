/*---------------------------------------------------------------
* Copyright Nov 2003, Mobile Communication Lab of BUPT
* All rights reserved.
*
* turbo_code_Log_MAP.h
*
* Header file for turbo_code_Log_MAP.cpp.
*
* Version: 1.0
* Programmed By Peng Zhang
* Last updated date: Dec, 13, 2003
---------------------------------------------------------------*/

#ifndef	TRUBO_CODE_LOG_MAP_H
#define	TRUBO_CODE_LOG_MAP_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

/*==================================================*/
/*	parameters for simulation	*/
#define TYPE_INTERLEAVER	2				/* type of interleaver */
#define TYPE_DECODER		1				/* type of decoder :
												 1 for Log_MAP
												 2 for MAX_Log_MAP */
#define N_ITERATION			6				/* number of iteration times */
#define TURBO_PUNCTURE		1				/* puncture or not
												1 for punctured
												0 for unpunctured */
#define MAX_FRAME_LENGTH	1280
#define SUP_FRAME_LENGTH	320
/*==================================================*/
/* parameters for the code generator */
#define COLUMN_OF_G		4	
#define G_ROW_1			13
#define G_ROW_2			15

/*==================================================*/
/*==================================================*/

/* structures */

/* code generator */
typedef struct
{
	int N_num_row;
	int K_num_col;
	int *g_matrix;
} TURBO_G;

/* trellis */
typedef struct
{
	int *mx_nextout;
	int *mx_nextstat;
	int *mx_lastout;
	int *mx_laststat;
	
} TURBO_TRELLIS;

/*==================================================*/
/* parameters of block interleavers */

#define		M_BLOCK_INT		40
#define		N_BLOCK_INT		8


/*==================================================*/
/* A number used in Log-MAP decoder */
#define INFTY 1E20

/*==================================================*/

/* global memory */

int *index_randomintlvr;		/* index of random intleaver */
int *index_randomintlvr_sup;	/* index of random intleaver for suplement flow*/

/* puncture matrix */
int *mx_puncture_turbo_80;
int *mx_puncture_turbo_160;
int *mx_puncture_turbo_320;

TURBO_G turbo_g;				/* code generator struct */

TURBO_TRELLIS turbo_trellis;	/* turbo_trellis struct */	

float rate_coding;				/* the rate of coding */

/*==================================================*/
/*==================================================*/
/* interfaces to outside */

/* Turbo码业务编码函数 */
void TurboCodingTraffic(int *trafficflow_source, float *coded_trafficflow_source,
						int *traffic_source_length);

/* Turbo码业务译码函数 */
void TurboDecodingTraffic(float *trafficflow_for_decode, int *trafficflow_decoded,
						  int *trafficflow_length, float EbN0dB);

/* Turbo码补充编码函数 */
void TurboCodingSupflow(int *supflow, float *coded_supflow, int *supflow_length);

/* Turbo码补充译码函数 */
void TurboDecodingSupflow(float *supflow_for_decode, int *supflow_decoded,
						  int *supflow_length, float EbN0dB);

/* Turbo码初始化函数 */
void TurboCodingInit();

/* 释放初始化时的内存 */
void TurboCodingRelease();

/*==================================================*/
/*==================================================*/
/* functions used inside */
int gen_g_matrix(int k_column, int g_row1, int g_row2, int *mx_g_turbo);

void gen_trellis();

void int2bin(int intstat, int *tempstat, int length);

int bin2int(int *binseq, int length);

float random_turbo();

void gen_rand_index(int length, int type_flow);

void randominterleaver_int(int *data_unintlvr, int *interleaverddata, int length, int type_flow);

void randominterleaver_float(float *data_unintlvr, float *interleaverddata, int length, int type_flow);

void random_deinterlvr_int(int *data_unintlvr, int *interleaverddata, int length, int type_flow);

void random_deinterlvr_float(float *data_unintlvr, float *interleaverddata, int length, int type_flow);

void blockinterleaver_float(int m_blockintlvr, int n_blockintlvr, float *inputdata, float *outputdata);

void blockinterleaver_int(int m_blockintlvr, int n_blockintlvr, int *inputdata, int *outputdata);

void intlvrcdma2000_int(int *data_unintlvr, int *interleaveddata, int nturbo);

void intlvrcdma2000_float(float *data_unintlvr, float *interleaveddata, int nturbo);

void deintlvrcdma2000_float(float *data_unintlvr, float *interleaveddata, int nturbo);

void deintlvrcdma2000_int(int *data_unintlvr, int *interleaveddata, int nturbo);

void subscript(int nturbo, int *sub_2000);

int msbs(int value, int numofbits, int num_bits);

int lsbs(int value, int num_bits);

int bitreverse(int value, int num_bits);

void interleave_int(int *data_unintlvr, int *interleaveddata, int typeofinterleave, int nturbo, int type_flow);

void interleave_float(float *data_unintlvr, float *interleaveddata, int typeofinterleave, int nturbo, int type_flow);

void de_interleave_int(int *data_unintlvr, int *interleaveddata, int typeofinterleave, int nturbo, int type_flow);

void de_interleave_float(float *data_unintlvr, float *interleaveddata, int typeofinterleave, int nturbo, int type_flow);

void encoderm_turbo(int *source, int *send_turbo, int len_info, int type_flow);

void rsc_encode(int *source, int *rsc, int terminated, int len_info);

int encode_bit(int inbit, int *stat);

int gen_mx_punc();

void puncture(int *data_unpunc, int length_unpunc, int *data_punctured, 
			  int M_mx_punc, int N_mx_punc, int times_punc);

void depuncture(float *receive_punc, int length_punc, float *receive_depunced, 
				int M_mx_punc, int N_mx_punc, int times_punc);

void demultiplex(float *rec_turbo, int len_info, float *yk_turbo, int type_flow);

void Log_MAP_decoder(float *recs_turbo, float *La_turbo, int terminated, float *LLR_all_turbo, int len_total);

void MAX_Log_MAP_decoder(float *recs_turbo, float *La_turbo, int terminated, float *LLR_all_turbo, int len_total);

float get_max(float *data_seq, int length);

float E_algorithm(float x, float y);

float E_algorithm_seq(float *data_seq, int length);

void decision(float *LLR_seq, int length, int *output);

/*==================================================*/

#endif