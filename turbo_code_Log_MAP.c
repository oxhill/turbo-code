/*---------------------------------------------------------------
* Copyright Nov 2003, Mobile Communication Lab of BUPT
* All rights reserved.
*
* turbo_code_Log_MAP.cpp
*
* This script implements the functions used in turbo coding and decoding.
*
* Version: 1.0
* Programmed By Peng Zhang
* Last updated date: Dec, 13, 2003
---------------------------------------------------------------*/

#include "turbo_code_Log_MAP.h"

/*==================================================*/
/* lookup table used in Log-MAP decoder */
const double lookup_index_Log_MAP[16] = {0.0, 0.08824, 0.19587, 0.31026, 0.43275, 0.56508,
								0.70963, 0.86972, 1.0502, 1.2587, 1.5078, 1.8212,
								2.2522, 2.9706, 3.6764, 4.3758};
const double lookup_table_Log_MAP[16] = {0.69315, 0.65, 0.6, 0.55, 0.5, 0.45, 0.4, 0.35,
								0.3, 0.25, 0.2, 0.15, 0.1, 0.05, 0.025, 0.0125};

/*CDMA2000中Turbo码所使用的交织器要使用该查找表。
各列分别对应于N＝4，5，6，7，8，9，10。N>=log2(Nturbo)-5。*/
const int lookuptable_cdma2000[7][32] = 
{{5,15,5,15,1,9,9,15,13,15,7,11,15,3,15,5,13,15,9,3,1,3,15,1,13,1,9,15,11,3,15,5},
{27,3,1,15,13,17,23,13,9,3,15,3,13,1,13,29,21,19,1,3,29,17,25,29,9,13,23,13,13,1,13,13},
{3,27,15,13,29,5,1,31,3,9,15,31,17,5,39,1,19,27,15,13,45,5,33,15,13,9,15,31,17,5,15,33},
{15,127,89,1,31,15,61,47,127,17,119,15,57,123,95,5,85,17,55,57,15,41,93,87,63,15,13,15,81,57,31,69},
{3,1,5,83,19,179,19,99,23,1,3,13,13,3,17,1,63,131,17,131,211,173,231,171,23,147,243,213,189,51,15,67},
{13,335,87,15,15,1,333,11,13,1,121,155,1,175,421,5,509,215,47,425,295,229,427,83,409,387,193,57,501,313,489,391},
{1,349,303,721,973,703,761,327,453,95,241,187,497,909,769,349,71,557,197,499,409,259,335,253,677,717,313,757,189,15,75,163}};

/*==================================================*/
/* parameters on g matrix */
const int M_num_reg = COLUMN_OF_G-1;		/* number of rigisters */
const int n_states = 8;						/* number of states : pow(2, M) */
/*==================================================*/

/*---------------------------------------------------------------
FUNCTION: 
	TurboCodingTraffic(int *trafficflow_source, int *coded_trafficflow_source,
						int *traffic_source_length)编码程序
	
DESCRIPTION:
	This function encodes the traffic flow bits.

PARAMETERS:
	INPUT:
		trafficflow_source - Contains pointer to the source bits sequence.输入数据流
		traffic_source_length - The pointer that give out the length of the source bits.输入数据流长度
	OUTPUT:
		coded_trafficflow_source - Contains pointer to the coded bits sequence.输出数据流的指针，也就是输出

RETURN VALUE:
	None.
---------------------------------------------------------------*/
void TurboCodingTraffic(int *trafficflow_source, float *coded_trafficflow_source,
						int *traffic_source_length)

{
	int i;

	int *temp_send = NULL;
	int *send = NULL;
	int *send_punc = NULL;

	int length_info = *traffic_source_length;

	switch( length_info/320 )
	{
	case 1:
		printf("1!\n");
	case 2:
		printf("23!\n");
	case 4:
		break;
	default:
		{
			printf("Wrong frame length！!\n");
			exit(1);
		}
	}
	/*----------------------错误检查----------------------------------------*/
	if ((send=(int *)malloc((3*length_info+4*M_num_reg)*sizeof(int)))==NULL)
	{
		printf("\n fail to allocate memory of send \n");
		exit(1);
	}

	if ((send_punc=(int *)malloc(2*length_info*sizeof(int)))==NULL)
	{
		printf("\n fail to allocate memory of send_punc \n");
		exit(1);  
	}

	if (TYPE_INTERLEAVER == 2)
	{
		gen_rand_index(length_info, 1);
	}
	/*------------------------------------------------------------------------*/
	encoderm_turbo(trafficflow_source, send, length_info, 1);	/* encode and BPSK modulate */

	temp_send = send;

	if (TURBO_PUNCTURE)			/* punture the coded bits */
	{
		puncture(send, 3*length_info+4*M_num_reg, send_punc, 3, length_info/4, 4);
		temp_send = send_punc;
	}

	for (i=0; i<((3-TURBO_PUNCTURE)*length_info+4*M_num_reg*(1-TURBO_PUNCTURE)); i++)
	{
		*(coded_trafficflow_source+i) = (float) *(temp_send+i);
	}

	*traffic_source_length = (3-TURBO_PUNCTURE)*length_info+4*M_num_reg*(1-TURBO_PUNCTURE);

	free(send);
	free(send_punc);
}


/*---------------------------------------------------------------
FUNCTION: 
	TurboDecodingTraffic(float *trafficflow_for_decode, int *trafficflow_decoded,
							int *trafficflow_length, float EbN0dB)
DESCRIPTION:
	This function decodes the received traffic flow bits.

PARAMETERS:
	INPUT:
		trafficflow_for_decode - Contains pointer to the received bits sequence.
		trafficflow_length - The pointer that give out the length of the received bits.
		EbN0dB - Eb/N0 in dB.
	OUTPUT:
		trafficflow_decoded - Contains pointer to the decoded bits sequence.

RETURN VALUE:
	None.
---------------------------------------------------------------*/
void TurboDecodingTraffic(float *trafficflow_for_decode, int *trafficflow_decoded,
						  int *trafficflow_length, float EbN0dB)
{
	int i;
	int length_info, length_total;
	int iteration;

	float *receive_punc = NULL;				/*	receiving data	*/
	float *yk_turbo = NULL;					/* include ys & yp */
	float *La_turbo, *Le_turbo, *LLR_all_turbo;		/*	extrinsic information & LLR	*/

	int *tempout;

	float en_rate = (float)pow(10, EbN0dB*0.1);
	float Lc_turbo = 4*en_rate*rate_coding;			/* reliability value of the channel */
	printf("trafficflow_length: %d\n", *trafficflow_length);

	switch((*trafficflow_length)/640)
	{
	case 1:
	case 2:
	case 4:
		break;
	default:
		{
			printf("Wrong frame length！！！!\n");
			exit(1);
			break;
		}
	}

	if (TURBO_PUNCTURE)
	{
		length_info = (*trafficflow_length)/2;
	}
	else
	{
		length_info = (*trafficflow_length-4*M_num_reg)/3;
	}

	length_total = length_info+M_num_reg;

	if ((receive_punc=(float *)malloc((3*length_info+4*M_num_reg)*sizeof(float)))==NULL)
	{
		printf("\n fail to allocate memory of receive_punc \n");
		exit(1);  
	}

	if ((yk_turbo=(float *)malloc(4*length_total*sizeof(float)))==NULL)
	{
	  printf("\n fail to allocate memory of yk_turbo \n");
	  exit(1);  
	}

	if ((La_turbo=(float *)malloc(length_total*sizeof(float)))==NULL)
	{
	  printf("\n fail to allocate memory of La_turbo \n");
	  exit(1);  
	}
	if ((Le_turbo=(float *)malloc(length_total*sizeof(float)))==NULL)
	{
	  printf("\n fail to allocate memory of Le_turbo \n");
	  exit(1);  
	}
	if ((LLR_all_turbo=(float *)malloc(length_total*sizeof(float)))==NULL)
	{
	  printf("\n fail to allocate memory of LLR_all_turbo \n");
	  exit(1);  
	}

	if ((tempout=(int *)malloc(length_total*sizeof(int)))==NULL)
	{
	  printf("\n fail to allocate memory of tempout \n");
	  exit(1);  
	}

	if (TURBO_PUNCTURE)		/* fill in the punctured bits and demultiplex */
	{
		depuncture(trafficflow_for_decode, 2*length_info, receive_punc, 3, length_info/4, 4);
		demultiplex(receive_punc, length_info, yk_turbo, 1);
	}
	else
	{
		demultiplex(trafficflow_for_decode, length_info, yk_turbo, 1);
	}
	
	/*	scale the data	*/
	for (i=0; i<2*length_total*2; i++)
	{
		*(yk_turbo+i) = (float)( *(yk_turbo+i) * Lc_turbo *0.5 );
	}
	
	for (i=0; i<length_total; i++)
	{
		*(La_turbo+i) = *(Le_turbo+i) = *(LLR_all_turbo+i) = 0;
	}

	for (iteration=0; iteration<N_ITERATION; iteration++)		/* start iteration */
	{
		/* decoder one: */
		/* get extrinsic information from decoder two */
		de_interleave_float(La_turbo, Le_turbo, TYPE_INTERLEAVER, length_info, 1);
		
		for (i=length_info; i<length_total; i++)
		{
			*(La_turbo+i) = 0;
		}

		switch(TYPE_DECODER)
		{
		case 1:
			Log_MAP_decoder(yk_turbo, La_turbo, 1, LLR_all_turbo, length_total);
			break;
		case 2:
			MAX_Log_MAP_decoder(yk_turbo, La_turbo, 1, LLR_all_turbo, length_total);
			break;
		default:
			break;
		}

		/* caculate the extrinsic information */
		for (i=0; i<length_total; i++)
		{
			*(Le_turbo+i) = *(LLR_all_turbo+i) - *(La_turbo+i) - 2*(*(yk_turbo+2*i));
		}

		/* decoder two: */
		/* get extrinsic information from decoder one */
		interleave_float(Le_turbo, La_turbo, TYPE_INTERLEAVER, length_info, 1);

		for (i=length_info; i<length_total; i++)
		{
			*(La_turbo+i) = 0;
		}
						
		switch(TYPE_DECODER)
		{
		case 1:
			Log_MAP_decoder(yk_turbo+2*length_total, La_turbo, 1, LLR_all_turbo, length_total);
			break;
		case 2:
			MAX_Log_MAP_decoder(yk_turbo+2*length_total, La_turbo, 1, LLR_all_turbo, length_total);
			break;
		default:
			break;
		}

		/* caculate the extrinsic information */
		for(i=0; i<length_total; i++)
		{
			*(Le_turbo+i) = *(LLR_all_turbo+i) - *(La_turbo+i) - 2*(*(yk_turbo+2*length_total+2*i));
		}
		/* end of decoder two */
	}
	
	/* get the decision bits from LLR gained from these decoder */
	decision(LLR_all_turbo, length_total, tempout);
	/* printf("%d", tempout);*/

	de_interleave_int(trafficflow_decoded, tempout, TYPE_INTERLEAVER, length_info, 1);

	*trafficflow_length = length_info;

	free(receive_punc);
	free(yk_turbo);

	free(La_turbo);
	free(Le_turbo);
	free(LLR_all_turbo);

	free(tempout);
}

void TurboCodingSupflow(int *supflow, float *coded_supflow, int *supflow_length)
{
	int i;

	int *temp_send = NULL;
	int *send = NULL;
	int *send_punc = NULL;

	int length_info = *supflow_length;

	switch( length_info/320 )
	{
	case 1:
	case 2:
	case 4:
		break;
	default:
		{
			printf("Wrong frame length!\n");
			exit(1);
			break;
		}
	}

	if ((send=(int *)malloc((3*length_info+4*M_num_reg)*sizeof(int)))==NULL)
	{
		printf("\n fail to allocate memory of send \n");
		exit(1);  
	}

	if ((send_punc=(int *)malloc(2*length_info*sizeof(int)))==NULL)
	{
		printf("\n fail to allocate memory of send_punc \n");
		exit(1);  
	}

	if (TYPE_INTERLEAVER == 2)
	{
		gen_rand_index(length_info, 0);
	}

	encoderm_turbo(supflow, send, length_info, 0);	/* encode and BPSK modulate */

	temp_send = send;

	if (TURBO_PUNCTURE)			/* punture the coded bits */
	{
		puncture(send, 3*length_info+4*M_num_reg, send_punc, 3, length_info/4, 4);
		temp_send = send_punc;
	}

	for (i=0; i<((3-TURBO_PUNCTURE)*length_info+4*M_num_reg*(1-TURBO_PUNCTURE)); i++)
	{
		*(coded_supflow+i) = (float) *(temp_send+i);
	}

	*supflow_length = (3-TURBO_PUNCTURE)*length_info+4*M_num_reg*(1-TURBO_PUNCTURE);

	free(send);
	free(send_punc);
}

void TurboDecodingSupflow(float *supflow_for_decode, int *supflow_decoded,
						  int *supflow_length, float EbN0dB)
{
	int i;
	int length_info, length_total;
	int iteration;

	float *receive_punc = NULL;				/*	receiving data	*/
	float *yk_turbo = NULL;					/* include ys & yp */
	float *La_turbo, *Le_turbo, *LLR_all_turbo;		/*	extrinsic information & LLR	*/

	int *tempout;

	float en_rate = (float)pow(10, EbN0dB*0.1);
	float Lc_turbo = 4*en_rate*rate_coding;			/* reliability value of the channel */

	switch((*supflow_length)/640)
	{
	case 1:
	case 2:
	case 4:
		break;
	default:
		{
			printf("Wrong frame length!\n");
			exit(1);
			break;
		}
	}

	if (TURBO_PUNCTURE)
	{
		length_info = (*supflow_length)/2;
	}
	else
	{
		length_info = (*supflow_length-4*M_num_reg)/3;
	}

	length_total = length_info+M_num_reg;

	if ((receive_punc=(float *)malloc((3*length_info+4*M_num_reg)*sizeof(float)))==NULL)
	{
		printf("\n fail to allocate memory of receive_punc \n");
		exit(1);  
	}

	if ((yk_turbo=(float *)malloc(4*length_total*sizeof(float)))==NULL)
	{
	  printf("\n fail to allocate memory of yk_turbo \n");
	  exit(1);  
	}

	if ((La_turbo=(float *)malloc(length_total*sizeof(float)))==NULL)
	{
	  printf("\n fail to allocate memory of La_turbo \n");
	  exit(1);  
	}
	if ((Le_turbo=(float *)malloc(length_total*sizeof(float)))==NULL)
	{
	  printf("\n fail to allocate memory of Le_turbo \n");
	  exit(1);  
	}
	if ((LLR_all_turbo=(float *)malloc(length_total*sizeof(float)))==NULL)
	{
	  printf("\n fail to allocate memory of LLR_all_turbo \n");
	  exit(1);  
	}

	if ((tempout=(int *)malloc(length_total*sizeof(int)))==NULL)
	{
	  printf("\n fail to allocate memory of tempout \n");
	  exit(1);  
	}

	if (TURBO_PUNCTURE)		/* fill in the punctured bits and demultiplex */
	{
		depuncture(supflow_for_decode, 2*length_info, receive_punc, 3, length_info/4, 4);
		demultiplex(receive_punc, length_info, yk_turbo, 0);
	}
	else
	{
		demultiplex(supflow_for_decode, length_info, yk_turbo, 0);
	}
	
	/*	scale the data	*/
	for (i=0; i<2*length_total*2; i++)
	{
		*(yk_turbo+i) = (float)( *(yk_turbo+i) * Lc_turbo *0.5 );
	}
	
	for (i=0; i<length_total; i++)
	{
		*(La_turbo+i) = *(Le_turbo+i) = *(LLR_all_turbo+i) = 0;
	}

	for (iteration=0; iteration<N_ITERATION; iteration++)		/* start iteration */
	{
		/* decoder one: */
		/* get extrinsic information from decoder two */
		de_interleave_float(La_turbo, Le_turbo, TYPE_INTERLEAVER, length_info, 0);
		
		for (i=length_info; i<length_total; i++)
		{
			*(La_turbo+i) = 0;
		}
		switch(TYPE_DECODER)
		{
		case 1:
			Log_MAP_decoder(yk_turbo, La_turbo, 1, LLR_all_turbo, length_total);
			break;
		case 2:
			MAX_Log_MAP_decoder(yk_turbo, La_turbo, 1, LLR_all_turbo, length_total);
			break;
		default:
			break;
		}
		/* caculate the extrinsic information */
		for (i=0; i<length_total; i++)
		{
			*(Le_turbo+i) = *(LLR_all_turbo+i) - *(La_turbo+i) - 2*(*(yk_turbo+2*i));
		}

		/* decoder two: */
		/* get extrinsic information from decoder one */
		interleave_float(Le_turbo, La_turbo, TYPE_INTERLEAVER, length_info, 0);

		for (i=length_info; i<length_total; i++)
		{
			*(La_turbo+i) = 0;
		}
						
		switch(TYPE_DECODER)
		{
		case 1:
			Log_MAP_decoder(yk_turbo+2*length_total, La_turbo, 1, LLR_all_turbo, length_total);
			break;
		case 2:
			MAX_Log_MAP_decoder(yk_turbo+2*length_total, La_turbo, 1, LLR_all_turbo, length_total);
			break;
		default:
			break;
		}
		/* caculate the extrinsic information */
		for(i=0; i<length_total; i++)
		{
			*(Le_turbo+i) = *(LLR_all_turbo+i) - *(La_turbo+i) - 2*(*(yk_turbo+2*length_total+2*i));
		}
		/* end of decoder two */
	}
	
	/* get the decision bits from LLR gained from these decoder */
	decision(LLR_all_turbo, length_total, tempout);

	de_interleave_int(supflow_decoded, tempout, TYPE_INTERLEAVER, length_info, 0);

	*supflow_length = length_info;

	free(receive_punc);
	free(yk_turbo);

	free(La_turbo);
	free(Le_turbo);
	free(LLR_all_turbo);

	free(tempout);
}
/*---------------------------------------------------------------
FUNCTION: 
	TurboCodingInit(int *traffic_source_length)Turbo编码初始化(int *traffic source length)
	
DESCRIPTION:
	Initialize the parameters of coding.初始化编码的参数。

PARAMETERS:
	INPUT:
		traffic_source_length - The pointer that give out the length of the source bits.
		输入:traffic source length-表示源比特流的指针。
RETURN VALUE:
	None.
	这段代码是一个用于初始化 Turbo 编码器的函数 `TurboCodingInit`。它包含了以下主要功能：

1. **初始化参数**：
   - 设置 Turbo 编码的行数和列数。
   - 根据是否使用了穿孔 (puncturing) 计算编码速率。

2. **分配内存**：
   - 动态分配存储编码生成矩阵 (`g_matrix`) 的内存。
   - 如果使用了穿孔，还会为不同长度的穿孔矩阵分配内存。

3. **生成生成矩阵**：
   - 调用 `gen_g_matrix` 函数生成编码生成矩阵。

4. **生成穿孔矩阵**：
   - 如果使用了穿孔，调用 `gen_mx_punc` 函数生成穿孔矩阵。

5. **初始化状态机（Trellis）结构**：
   - 分配内存用于 Trellis 状态机的输出和状态。

6. **随机交织器索引的内存分配**：
   - 如果选择了随机交织器类型，则为交织器索引分配内存。

这段代码展示了 Turbo 编码初始化的基本过程，包括内存管理和关键数据结构的设置。每个部分都有错误检查，以确保在内存分配失败时能及时退出。
---------------------------------------------------------------*/
void TurboCodingInit()
{
	turbo_g.N_num_row = 2;		/* initialize number of rows and columns of g */
	turbo_g.K_num_col = COLUMN_OF_G;

	/* caculate the coding rate */
	rate_coding = TURBO_PUNCTURE? (float)(0.5) : (float)(0.333333);

	if ((turbo_g.g_matrix=(int *)malloc(turbo_g.N_num_row*turbo_g.K_num_col*sizeof(int)))==NULL)
	{
		printf("\n fail to allocate memory of turbo_g\n");
		exit(1);  
	}

	/* generate the code generator matrix */
	if (!gen_g_matrix(turbo_g.K_num_col, G_ROW_1, G_ROW_2, turbo_g.g_matrix))
	{
		printf("error number of G\n");
		exit(1);
	}
	/* generate puncture matrix */
	if (TURBO_PUNCTURE)
	{
		if ((mx_puncture_turbo_80=(int *)malloc(sizeof(int)*3*80))==NULL)
		{
			printf("\n fail to allocate memory of mx_puncture_turbo_80\n");
			exit(1);  
		}
		if ((mx_puncture_turbo_160=(int *)malloc(sizeof(int)*3*160))==NULL)
		{
			printf("\n fail to allocate memory of mx_puncture_turbo_160\n");
			exit(1);  
		}
		if ((mx_puncture_turbo_320=(int *)malloc(sizeof(int)*3*320))==NULL)
		{
			printf("\n fail to allocate memory of mx_puncture_turbo_320\n");
			exit(1);  
		}

		if (!gen_mx_punc())	/*	generate the matrix for puncture */
		{
			printf("\nError! Can not generate the puncture matrix properly!\n");
			exit(1);
		}
	}

	/* initialize the trellis struct */
	if ((turbo_trellis.mx_lastout=(int *)malloc(sizeof(int)*n_states*4))==NULL)
	{
	  printf("\n fail to allocate memory of turbo_trellis.mx_lastout \n");
	  exit(1);  
	}
	if ((turbo_trellis.mx_laststat=(int *)malloc(sizeof(int)*n_states*2))==NULL)
	{
	  printf("\n fail to allocate memory of turbo_trellis.mx_laststat \n");
	  exit(1);  
	}
	if ((turbo_trellis.mx_nextout=(int *)malloc(sizeof(int)*n_states*4))==NULL)
	{
	  printf("\n fail to allocate memory of turbo_trellis.mx_nextout \n");
	  exit(1);  
	}
	if ((turbo_trellis.mx_nextstat=(int *)malloc(sizeof(int)*n_states*2))==NULL)
	{
	  printf("\n fail to allocate memory of turbo_trellis.mx_nextstat \n");
	  exit(1);  
	}

	gen_trellis();

	/* malloc memory for the index of random interleaver */
	if (TYPE_INTERLEAVER==2)
	{
		if ((index_randomintlvr=(int *)malloc(MAX_FRAME_LENGTH*sizeof(int)))==NULL)
		{
			printf("\n fail to allocate memory of index_randomintlvr \n");
			exit(1);  
		}
		if ((index_randomintlvr_sup=(int *)malloc(SUP_FRAME_LENGTH*sizeof(int)))==NULL)
		{
			printf("\n fail to allocate memory of index_randomintlvr_sup \n");
			exit(1);  
		}
	}
}

/*---------------------------------------------------------------
FUNCTION: 
	TurboCodingFinish()
	
DESCRIPTION:
	Free the memory of turbo coding.

PARAMETERS:
	None.

RETURN VALUE:
	None.
---------------------------------------------------------------*/
void TurboCodingRelease()
{
	free(turbo_g.g_matrix);

	if (TURBO_PUNCTURE)
	{		
		free(mx_puncture_turbo_80);
		free(mx_puncture_turbo_160);
		free(mx_puncture_turbo_320);
	}

	free(turbo_trellis.mx_lastout);
	free(turbo_trellis.mx_laststat);
	free(turbo_trellis.mx_nextout);
	free(turbo_trellis.mx_nextstat);

	if (TYPE_INTERLEAVER==2)
	{
		free(index_randomintlvr);
		free(index_randomintlvr_sup);
	}
}

/*---------------------------------------------------------------
FUNCTION: 
	int gen_g_matrix(int k_column, int g_row1, int g_row2, int *mx_g_turbo)
	
DESCRIPTION:
	This function generate the code generater g for the random interleaver.

PARAMETERS:
	INPUT:
		k - Number of columns of g matrix.
		g_row1 - An octal number indicate the first row of turbo_g.
		g_row2 - An octal number indicate the second row of turbo_g.
	OUTPUT:
		mx_g - Contains pointer to g matrix.

RETURN VALUE:
	1 - If the matrix was successfully generated.
	0 - Error occurred generating the g matrix.
---------------------------------------------------------------*/
int gen_g_matrix(int k_column, int g_row1, int g_row2, int *mx_g_turbo)
{
	int i, position;
	int high_num, low_num;
	
	/* for the first row */
	high_num = g_row1;
	position = 1;
	while (high_num>0)
	{
		low_num = high_num%10;
		if (low_num>7)
		{
			return 0;
		}
		high_num = high_num/10;

		for (i=k_column-(position-1)*3-1; i>=0 && i>=k_column-position*3; i--)
		{
			*(mx_g_turbo+i) = low_num%2;
			low_num = low_num/2;
		}
		position++;
		if (i<0)
		{
			break;
		}
	}

	/*for the second row */
	high_num = g_row2;
	position = 1;
	while (high_num>0)
	{
		low_num = high_num%10;
		if (low_num>7)
		{
			return 0;
		}
		high_num = high_num/10;

		for (i=k_column-(position-1)*3-1; i>=0 && i>=k_column-position*3; i--)
		{
			*(mx_g_turbo+k_column+i) = low_num%2;
			low_num = low_num/2;
		}
		position++;
		if (i<0)
		{
			break;
		}
	}
	return 1;
}

/*---------------------------------------------------------------
FUNCTION: 
	void gen_trellis()
	
DESCRIPTION:
	This function generate the turbo_trellis structure.

PARAMETERS:
	None

RETURN VALUE:
	None
---------------------------------------------------------------*/
void gen_trellis()
{
	int i, j, k;
	int dk_turbo, ak_turbo, outbit;

	int *tempstat;

	if ((tempstat=(int *)malloc(sizeof(int)*M_num_reg))==NULL)
	{
	  printf("\n fail to allocate memory of tempstat \n");
	  exit(1);  
	}
	/* generate next output and next state matrix */
	for (i=0; i<n_states; i++)
	{
		for (j=0; j<2; j++)
		{
			int2bin(i, tempstat, M_num_reg);
			dk_turbo = j;

			ak_turbo = (*(turbo_g.g_matrix+0)) * dk_turbo;
			for (k=1; k<turbo_g.K_num_col; k++)
			{
				ak_turbo = ak_turbo + (*(turbo_g.g_matrix+k)) * (*(tempstat+k-1));
			}

			ak_turbo = ak_turbo % 2;

			outbit = encode_bit(ak_turbo, tempstat);

			*(turbo_trellis.mx_nextout+i*4+2*j)=2*dk_turbo-1;
			*(turbo_trellis.mx_nextout+i*4+2*j+1)=2*outbit-1;
			*(turbo_trellis.mx_nextstat+i*2+j)=bin2int(tempstat, M_num_reg);
		}

	}
	/* generate last output and last state matrix */
	for (i=0; i<n_states; i++)
	{
		for (j=0; j<2; j++)
		{
			*(turbo_trellis.mx_laststat+(*(turbo_trellis.mx_nextstat+i*2+j))*2+j) = i;
			*(turbo_trellis.mx_lastout+(*(turbo_trellis.mx_nextstat+i*2+j))*4+2*j)
				= *(turbo_trellis.mx_nextout+i*4+2*j);
			*(turbo_trellis.mx_lastout+(*(turbo_trellis.mx_nextstat+i*2+j))*4+2*j+1)
				= *(turbo_trellis.mx_nextout+i*4+2*j+1);
		}
	}

	free(tempstat);
}

/*---------------------------------------------------------------
FUNCTION: 
	void int2bin(int intstat, int *tempstat, int length)
	
DESCRIPTION:
	This function turn a decimal integer to a binary sequence.

PARAMETERS:
	INPUT:
		intstat - Decimal integer needed to be changed.
		length - Length of binary sequence.
	OUTPUT:
		tempstat - Contains pointer to the binary sequence.

RETURN VALUE:
	None
---------------------------------------------------------------*/
void int2bin(int intstat, int *tempstat, int length)
{
	int i, temp;

	temp = intstat;

	for (i=length-1; i>=0; i--)
	{
		*(tempstat+i) = temp%2;
		temp = temp/2;
	}
}

/*---------------------------------------------------------------
FUNCTION: 
	int bin2int(int *binseq, int length)
	
DESCRIPTION:
	This function turn a binary sequence to a decimal integer.

PARAMETERS:
	INPUT:
		binseq - Contains pointer to the binary sequence.
		length - Length of binary sequence.

RETURN VALUE:
	The decimal integer corresponding to the binary sequence.
---------------------------------------------------------------*/
int bin2int(int *binseq, int length)
{
	int i, j, temp;
	int sum = 0;

	for (i=0; i<length; i++)
	{
		temp = 1;

		for (j=1; j<=i; j++)
		{
			temp = temp * 2;
		}
		sum = sum + temp * (*(binseq+length-1-i));
	}

	return sum;
}

/*---------------------------------------------------------------
FUNCTION: 
	random_turbo()
	
DESCRIPTION:
	Generate a random number between 0 and 1.产生随机数（位于0和1之间）

RETURN VALUE:
	The random number between 0 and 1.
---------------------------------------------------------------*/
float random_turbo()
{
long z,k;
static long s1 = 12345L;
static long s2 = 1234546346L;

	k= s1 / 53668L;
	s1 = 40014L * (s1 - k*53668L) - k*12211L;
	if (s1<0)
	  s1 = s1 + 2147483563L;
	k = s2 / 52774;
	s2 = 40692L * (s2 - k*52774L) - k*3791L;
	if (s2<0)
        s2 = s2 + 2147483399L;
 	z=s1 - s2;
	if (z<1)
  	  z = z + 2147483562L;
	return (float) z / (float) 2147483563.0;
}


/*---------------------------------------------------------------
FUNCTION: 
	void gen_rand_index(int length)
	
DESCRIPTION:
	This function generate the index for the random interleaver.

PARAMETERS:
	INPUT:
		length - Length of interleaver.
		type_flow - 1 for traffic flow, 0 for suplement flow.

RETURN VALUE:
	None
---------------------------------------------------------------*/
void gen_rand_index(int length, int type_flow)
{
	int *index_random;
	float *tempindex;
	float tempmax;
	int selectedscr;
	int i, j;

	if ((tempindex=(float *)malloc((length)*sizeof(float)))==NULL)
	{
	  printf("\n fail to allocate memory of tempindex \n");
	  exit(1);  
	}

	index_random = type_flow? index_randomintlvr : index_randomintlvr_sup;

	for (i=0; i<length; i++)
	{
		*(tempindex+i) = random_turbo();
	}

	for (i=0; i<length; i++)
	{
		tempmax = 0.0;

		for (j=0; j<length; j++)
		{
			if (*(tempindex+j) >= tempmax)
			{
				tempmax = *(tempindex+j);
				selectedscr = j;
			}
		}

		*(index_random+i) = selectedscr;
		*(tempindex+selectedscr) = 0.0;
	}

	free(tempindex);
}

/*---------------------------------------------------------------
FUNCTION: 
	void randominterleaver_int(int *data_unintlvr, int *interleaverddata,
								 int length, int type_flow)
	
DESCRIPTION:
	Random interleaver of int.

PARAMETERS:
	INPUT:
		data_unintlvr - Contains pointer to data_unintlvr before interleavered.
		length - Length of interleaver.
		type_flow - 1 for traffic flow, 0 for suplement flow.
	OUTPUT:
		interleaverddata - Contains pointer to interleavered data_unintlvr.

RETURN VALUE:
	None
---------------------------------------------------------------*/
void randominterleaver_int(int *data_unintlvr, int *interleaverddata,
						   int length, int type_flow)
{
	int i;
	int *index_random = type_flow? index_randomintlvr : index_randomintlvr_sup;

	for (i=0; i<length; i++)
	{
		*(interleaverddata+i) = *(data_unintlvr+ (*(index_random+i)));
	}
}

/*---------------------------------------------------------------
FUNCTION: 
	void randominterleaver_float(float *data_unintlvr, float *interleaverddata, int length, int type_flow)
	
DESCRIPTION:
	Random interleaver of float.

PARAMETERS:
	INPUT:
		data_unintlvr - Contains pointer to data_unintlvr before interleavered.
		length - Length of interleaver.
		type_flow - 1 for traffic flow, 0 for suplement flow.
	OUTPUT:
		interleaverddata - Contains pointer to interleavered data_unintlvr.

RETURN VALUE:
	None
--------------------源代码被注释了----------------------------------*/
//void randominterleaver_float(float *data_unintlvr, float *interleaverddata, int length, int type_flow)
//{
//	int i;
//	int *index_random = type_flow? index_randomintlvr : index_randomintlvr_sup;
//
//	for (i=0; i<length; i++)
//	{
//		*(interleaverddata+i) = *(data_unintlvr+ (*(index_random+i)));
//	}
//}
void randominterleaver_float(float* data_unintlvr, float* interleaverddata, int length, int type_flow)
{
	int i;
	int* index_random = type_flow ? index_randomintlvr : index_randomintlvr_sup;

	if (index_random == NULL || data_unintlvr == NULL || interleaverddata == NULL) {
		printf("Error: NULL pointer detected\n");
		return;
	}

	for (i = 0; i < length; i++)
	{
		int index = *(index_random + i);

		// 检查 index 是否在 data_unintlvr 的有效范围内
		if (index < 0 || index >= length) {
			printf("Error: Out of bounds index at position %d\n", i);
			return;
		}

		*(interleaverddata + i) = *(data_unintlvr + index);
	}
}


/*---------------------------------------------------------------
FUNCTION: 
	void random_deinterlvr_int(int *data_unintlvr, int *interleaverddata, int length, int type_flow)
	
DESCRIPTION:
	Random deinterleaver of int.

PARAMETERS:
	INPUT:
		data_unintlvr - Contains pointer to data_unintlvr before interleavered.
		length - Length of interleaver.
		type_flow - 1 for traffic flow, 0 for suplement flow.

	OUTPUT:
		interleaverddata - Contains pointer to interleavered data_unintlvr.

RETURN VALUE:
	None
---------------------------------------------------------------*/
//void random_deinterlvr_int(int *data_unintlvr, int *interleaverddata, int length, int type_flow)
//{
//	int i;
//	int *index_random = type_flow? index_randomintlvr : index_randomintlvr_sup;
//
//	for (i=0; i<length; i++)
//	{
//		*(data_unintlvr+(*(index_random+i))) = *(interleaverddata+i);
//	}
//}
	void random_deinterlvr_int(int* data_unintlvr, int* interleaverddata, int length, int type_flow)
	{
		int i;
		int* index_random = type_flow ? index_randomintlvr : index_randomintlvr_sup;

		for (i = 0; i < length; i++)
		{
			*(data_unintlvr + (*(index_random + i))) = *(interleaverddata + i);
		}
	}
/*---------------------------------------------------------------
FUNCTION: 
	void random_deinterlvr_float(float *data_unintlvr, float *interleaverddata, int length, int type_flow)
	
DESCRIPTION:
	Random interleaver of float.

PARAMETERS:
	INPUT:
		data_unintlvr - Contains pointer to data_unintlvr before interleavered.
		length - Length of interleaver.
		type_flow - 1 for traffic flow, 0 for suplement flow.
	OUTPUT:
		interleaverddata - Contains pointer to interleavered data_unintlvr.

RETURN VALUE:
	None
---------------------------------------------------------------*/
//void random_deinterlvr_float(float *data_unintlvr, float *interleaverddata, int length, int type_flow)
//{
//	int i;
//	int *index_random = type_flow? index_randomintlvr : index_randomintlvr_sup;
//
//	for (i=0; i<length; i++)
//	{
//		*(data_unintlvr+(*(index_random+i))) = *(interleaverddata+i);
//	}
//}

void random_deinterlvr_float(float* data_unintlvr, float* interleaverddata, int length, int type_flow)
{
	int i;
	int* index_random = type_flow ? index_randomintlvr : index_randomintlvr_sup;

	// 空指针检查
	if (index_random == NULL || data_unintlvr == NULL || interleaverddata == NULL) {
		printf("Error: NULL pointer detected\n");
		return;
	}

	for (i = 0; i < length; i++)
	{
		int index = *(index_random + i);

		// 检查 index 是否在 data_unintlvr 的有效范围内
		if (index < 0 || index >= length) {
			printf("Error: Out of bounds index at position %d\n", i);
			return;
		}

		*(data_unintlvr + index) = *(interleaverddata + i);
	}
}


/*
* 函数介绍：float型数据的块交织及解交织
* 输入参数：m_blockintlvr:块的行数
*           n_blockintlvr:块的列数
*           inputdata：交织（解交织）前的数据序列
* 输出参数：poutput：交织（解交织）后的数据序列
* 返回值：  无
* 说明：    块交织与解交织使用同一个函数，行数与列数互相交换。块交织的块大小必须与帧长相等
*/
void blockinterleaver_float(int m_blockintlvr, int n_blockintlvr, float *inputdata, float *outputdata)
{
	int i, j;
	for(i=0; i<m_blockintlvr; i++)
	{
		for(j=0; j<n_blockintlvr; j++)
		{
			outputdata[i*n_blockintlvr+j] = inputdata[j*m_blockintlvr+i];
		}
	}
}

/*
* 函数介绍：int型数据的块交织及解交织
* 输入参数：m_blockintlvr:块的行数
*           n_blockintlvr:块的列数
*           inputdata：交织（解交织）前的数据序列
* 输出参数：poutput：交织（解交织）后的数据序列
* 返回值：  无
* 说明：    块交织与解交织使用同一个函数，行数与列数互相交换。块交织的块大小必须与帧长相等
*/
void blockinterleaver_int(int m_blockintlvr, int n_blockintlvr, int *inputdata, int *outputdata)
{
	int i, j;
	for(i=0; i<m_blockintlvr; i++)
	{
		for(j=0; j<n_blockintlvr; j++)
		{
			outputdata[i*n_blockintlvr+j] = inputdata[j*m_blockintlvr+i];
		}
	}
}

/*
* 函数介绍：int型CDMA2000交织器
* 输入参数：data_unintlvr：交织前的数据
*           nturbo:交织块的大小
* 输出参数：interleaveddata：交织后的数据序列
* 返回值：  无
* 说明：    nturbo必须在256和32768之间
*/
void intlvrcdma2000_int(int *data_unintlvr, int *interleaveddata, int nturbo)
{   
	int i;
	int n = (int)ceil(log(nturbo)/log(2))-5;
	int *sub_2000 = (int*)malloc(sizeof(int)*nturbo);
	subscript(nturbo, sub_2000);

	for(i=0; i<nturbo; i++)
	{  
		interleaveddata[i] = data_unintlvr[sub_2000[i]];
	}
	free(sub_2000);

}

/*
* 函数介绍：float型CDMA2000交织器
* 输入参数：data_unintlvr：交织前的数据
*           nturbo:交织块的大小
* 输出参数：interleaveddata：交织后的数据序列
* 返回值：  无
* 说明：    nturbo必须在256和32768之间
*/
void intlvrcdma2000_float(float *data_unintlvr, float *interleaveddata, int nturbo)
{   
	int i;
	int n = (int)ceil(log(nturbo)/log(2))-5;
	int *sub_2000 = (int*)malloc(sizeof(int)*nturbo);
	subscript(nturbo, sub_2000);

	for(i=0; i<nturbo; i++)
	{  
		interleaveddata[i] = data_unintlvr[sub_2000[i]];
	}
	free(sub_2000);

}

/*
* 函数介绍：float型CDMA2000解交织器
* 输入参数：interleaveddata：解交织前的数据
*           nturbo:交织块的大小
* 输出参数：data_unintlvr：解交织后的数据序列
* 返回值：  无
* 说明：    nturbo必须在256和32768之间
*/
void deintlvrcdma2000_float(float *data_unintlvr, float *interleaveddata, int nturbo)
{   
	int i;
	int n = (int)ceil(log(nturbo)/log(2))-5;
	int *sub_2000 = (int*)malloc(sizeof(int)*nturbo);
	subscript(nturbo, sub_2000);

	for(i=0; i<nturbo; i++)
	{  
		data_unintlvr[sub_2000[i]] = interleaveddata[i];
	}
	free(sub_2000);

}

/*
* 函数介绍：int型CDMA2000解交织器
* 输入参数：interleaveddata：解交织前的数据
*           nturbo:交织块的大小
* 输出参数：data_unintlvr：解交织后的数据序列
* 返回值：  无
* 说明：    nturbo必须在256和32768之间
*/
void deintlvrcdma2000_int(int *data_unintlvr, int *interleaveddata, int nturbo)
{   
	int i;
	int n = (int)ceil(log(nturbo)/log(2))-5;
	int *sub_2000 = (int*)malloc(sizeof(int)*nturbo);
	subscript(nturbo, sub_2000);

	for(i=0; i<nturbo; i++)
	{  
		data_unintlvr[sub_2000[i]] = interleaveddata[i];
	}
	free(sub_2000);

}

/*
* 函数介绍：生成CDMA2000交织器的下标对应关系
* 输入参数：nturbo:交织块的大小
* 输出参数：sub_2000：交织后序列各元素在原序列中的对应位置
* 返回值：  无
* 说明：    nturbo必须在256和32768之间
*/
void subscript(int nturbo, int *sub_2000)
{
	int n;
	int indx;
	int counternum = 0;
	int tentative = 0;
	int temp1, temp2, temp3;
	int num = 0;
	n = (int)ceil(log(nturbo)/log(2))-5;
	counternum = (1<<(n+5))-1;
	for(indx=0; indx<(1<<(n+5)); indx++)
	{
		temp1 = msbs(indx, n+5, n)+1;
		temp1 = lsbs(temp1, n);
		temp2 = lookuptable_cdma2000[n-4][lsbs(indx, 5)];
		temp1 = lsbs(temp1*temp2, n);
		temp3 = bitreverse(lsbs(indx,5),5);
		tentative = (temp3<<n) + temp1;
		if(tentative < nturbo)
		{
			*(sub_2000+num) = tentative;
			num++;
		}
		if(num == nturbo)
		{
			break;
		}
		
	}
		
}

/*
* 函数介绍：取value的高num_bits位
* 输入参数：value：输入数据
*           numofbits: value的位数
*           num_bits: 要取的位数
* 输出参数：无
* 返回值：  value的高num_bits位所组成的十进制数
*/
int msbs(int value, int numofbits, int num_bits)
{
	int result = value>>(numofbits-num_bits);
	return result;
}

/*
* 函数介绍：取value的低num_bits位
* 输入参数：value：输入数据
*           num_bits: 要取的位数
* 输出参数：无
* 返回值：  value的低num_bits位所组成的十进制数
*/
int lsbs(int value, int num_bits)
{
	int result;
	int temp = (1<<num_bits)-1;
	result = value & temp;
	return result;
}


/*
* 函数介绍：比特翻转
* 输入参数：value：输入数据
*           num_bits: value的比特数
* 输出参数：无
* 返回值：  value比特翻转后组成的十进制数
*/
int bitreverse(int value, int num_bits)
{
	int i;
	int *bits = (int*)malloc(sizeof(int)*num_bits);
	int result;
	for(i=0; i<num_bits; i++)
	{
		bits[i] = (value>>i)&1;
	}
	/*将输入值变为num_bits个0，1数据*/
	result = 0;
	for(i=0; i<num_bits; i++)
	{
		result += bits[i]<<(num_bits-1-i);
	}
	/*翻转后组成新的十进制数*/
	free(bits);
	return result;
}


/*
* 函数介绍：int型数据交织函数
* 输入参数：data_unintlvr：交织前数据
*           nturbo: 数据长度
*           typeofinerleaver：交织器类型
* 输出参数：interleaveddata：交织后数据
* 返回值：  无
* 说明：    交织器类型选择：0为CDMA2000交织器，1为块交织，2为随机交织（暂缺），3为卷积交织（暂缺）
*/
void interleave_int(int *data_unintlvr, int *interleaveddata, int typeofinterleave, int nturbo, int type_flow)
{
	switch(typeofinterleave)
	{
	    case 0:
		{
			intlvrcdma2000_int(data_unintlvr, interleaveddata, nturbo);
			break;
		}
		case 1:
		{
			blockinterleaver_int(M_BLOCK_INT, N_BLOCK_INT, data_unintlvr, interleaveddata);
			break;
		}
		case 2:
		{
			randominterleaver_int(data_unintlvr, interleaveddata, nturbo, type_flow);
			break;
		}
		case 3:
		{
			break;
		}
		default:
		{
			printf("error! This type of interleaver doesn't exits!\n");
		}
	}
}

/*
* 函数介绍：float型数据交织函数
* 输入参数：data_unintlvr：交织前数据
*           nturbo: 数据长度
*           typeofinerleaver：交织器类型
* 输出参数：interleaveddata：交织后数据
* 返回值：  无
* 说明：    交织器类型选择：0为CDMA2000交织器，1为块交织，2为随机交织（暂缺），3为卷积交织（暂缺）
*/
void interleave_float(float *data_unintlvr, float *interleaveddata, int typeofinterleave, int nturbo, int type_flow)
{
	switch(typeofinterleave)
	{
	    case 0:
		{
			intlvrcdma2000_float(data_unintlvr, interleaveddata, nturbo);
			break;
		}
		case 1:
		{
			blockinterleaver_float(M_BLOCK_INT, N_BLOCK_INT, data_unintlvr, interleaveddata);
			break;
		}
		case 2:
		{
			randominterleaver_float(data_unintlvr, interleaveddata, nturbo, type_flow);
			break;
		}
		case 3:
		{
			break;
		}
		default:
		{
			printf("error! This type of interleaver doesn't exits!\n");
		}
	}
}

/*
* 函数介绍：int型数据解交织函数
* 输入参数：interleaveddata：解交织前数据
*           nturbo: 数据长度
*           typeofinerleaver：交织器类型
* 输出参数：data_unintlvr：解交织后数据
* 返回值：  无
* 说明：    交织器类型选择：0为CDMA2000交织器，1为块交织，2为随机交织（暂缺），3为卷积交织（暂缺）
*/
void de_interleave_int(int *data_unintlvr, int *interleaveddata, int typeofinterleave, int nturbo, int type_flow)
{
	switch(typeofinterleave)
	{
	    case 0:
		{
			deintlvrcdma2000_int(data_unintlvr, interleaveddata, nturbo);
			break;
		}
		case 1:
		{
			blockinterleaver_int(N_BLOCK_INT, M_BLOCK_INT, interleaveddata, data_unintlvr);
			break;
		}
		case 2:
		{
			random_deinterlvr_int(data_unintlvr, interleaveddata, nturbo, type_flow);
			break;
		}
		case 3:
		{
			break;
		}
		default:
		{
			printf("error! This type of interleaver doesn't exits!\n");
		}
	}
}

/*
* 函数介绍：float型数据解交织函数
* 输入参数：interleaveddata：解交织前数据
*           nturbo: 数据长度
*           typeofinerleaver：交织器类型
* 输出参数：data_unintlvr：解交织后数据
* 返回值：  无
* 说明：    交织器类型选择：0为CDMA2000交织器，1为块交织，2为随机交织（暂缺），3为卷积交织（暂缺）
*/
void de_interleave_float(float *data_unintlvr, float *interleaveddata, int typeofinterleave, int nturbo, int type_flow)
{
	switch(typeofinterleave)
	{
	    case 0:
		{
			deintlvrcdma2000_float(data_unintlvr, interleaveddata, nturbo);
			break;
		}
		case 1:
		{
			blockinterleaver_float(N_BLOCK_INT, M_BLOCK_INT, interleaveddata, data_unintlvr);
			break;
		}
		case 2:
		{	
			random_deinterlvr_float(data_unintlvr, interleaveddata, nturbo, type_flow);
			break;
		}
		case 3:
		{
			break;
		}
		default:
			printf("error! This type of interleaver doesn't exits!\n");
	}
}

/*---------------------------------------------------------------
FUNCTION: 
	void encoderm_turbo(int *source, int *send_turbo, int len_info, int type_flow)
	
DESCRIPTION:
	This function encode the source data and modulate them using BPSK.

PARAMETERS:
	INPUT:
		source - Contains pointer to source bits.
		len_info - Length of the input information bits.
		type_flow - 1 for traffic flow, 0 for suplement flow.

	OUTPUT:
		send_turbo - Contains pointer to the sequence after encoding and modulation.

RETURN VALUE:
	None
---------------------------------------------------------------*/

void encoderm_turbo(int *source, int *send_turbo, int len_info, int type_flow)
{
	int i;
	int len_total = len_info + M_num_reg;

	int *rsc1, *rsc2;
	
	int *input2;

	if ((rsc1=(int *)malloc(2*len_total*sizeof(int)))==NULL)
	{
	  printf("\n fail to allocate memory of rsc1 \n");
	  exit(1);  
	}
	if ((rsc2=(int *)malloc(2*len_total*sizeof(int)))==NULL)
	{
	  printf("\n fail to allocate memory of rsc2 \n");
	  exit(1);  
	}

	if ((input2=(int *)malloc(len_info*sizeof(int)))==NULL)
	{
	  printf("\n fail to allocate memory of input2 \n");
	  exit(1);  
	}

	/* the first RSC encoder */
	rsc_encode(source, rsc1, 1, len_info);

	interleave_int(source, input2, TYPE_INTERLEAVER, len_info, type_flow);

	/* the second RSC encoder */
	rsc_encode(input2, rsc2, 1, len_info);

	for (i=0; i<len_info; i++)
	{
		*(send_turbo+3*i) = *(rsc1+2*i) *2 - 1;
		*(send_turbo+3*i+1) = *(rsc1+2*i+1) *2 - 1;
		*(send_turbo+3*i+2) = *(rsc2+2*i+1) *2 - 1;
	}
	
	for (i=0; i<2*M_num_reg; i++)
	{
		*(send_turbo+3*len_info+i) = *(rsc1+2*len_info+i) *2 - 1;
		*(send_turbo+3*len_info+2*M_num_reg+i) = *(rsc2+2*len_info+i) *2 - 1;
	}
	
	free(rsc1);
	free(rsc2);
	free(input2);
}

/*---------------------------------------------------------------
FUNCTION: 
	void rsc_encode(int *source, int *rsc, int terminated, int len_info)

DESCRIPTION:
	Encodes a block of data to a recursive systematic convolutional code.

PARAMETERS:
	INPUT:
		source - Contains pointer to the input data sequence.
		terminated - Indicate the encoder terminates the turbo_trellis or not.
					 1 - terminated
					 0 - not terminated
		len_info - Length of the input data sequencd.
	OUTPUT:
		rsc - Contains pointer to the output data sequence.

RETURN VALUE:
	None
---------------------------------------------------------------*/
void rsc_encode(int *source, int *rsc, int terminated, int len_info)
{
	int i, j;

	int *state;
	int dk_turbo, ak_turbo, outbit;

	int len_total;

	if ((state=(int *)malloc(M_num_reg*sizeof(int)))==NULL)
	{
	  printf("\n fail to allocate memory of state \n");
	  exit(1);  
	}

	len_total = len_info+M_num_reg;

	for (i=0; i<M_num_reg; i++)
	{
		*(state+i) = 0;
	}

	for (i=0; i<len_total; i++)			/* encoding bit by bit */
	{
		if (!terminated || (terminated && i<len_info))	/* for information bits */
		{
			dk_turbo = *(source+i);
		}
		else											/* terminate the trellis */
		{
			if (terminated && i>=len_info)
			{
				dk_turbo = 0;
				for (j=1; j<turbo_g.K_num_col; j++)
				{
					dk_turbo = dk_turbo + (*(turbo_g.g_matrix+j)) * (*(state+j-1));
				}
				dk_turbo = dk_turbo%2;
			}
		}

		ak_turbo = *(turbo_g.g_matrix+0) * dk_turbo;
		for (j=1; j<turbo_g.K_num_col; j++)
		{
			ak_turbo = ak_turbo + (*(turbo_g.g_matrix+j))*(*(state+j-1));
		}

		ak_turbo = ak_turbo%2;

		outbit = encode_bit(ak_turbo, state);

		*(rsc+2*i) = dk_turbo;
		*(rsc+2*i+1) = outbit;
	}				/* end of encoding bit by bit */

	free(state);
}


/*---------------------------------------------------------------
FUNCTION: 
	int encode_bit(int inbit, int *stat)
	
DESCRIPTION:
	This function can get a coded bit from the memory state and the input bit.
	The memory state is accordingly modified.

PARAMETERS:
	INPUT:
		inbit - The input bit.
	OUTPUT & INPUT:
		stat - Contains pointer to memory state.

RETURN VALUE:
	The coded bit.
---------------------------------------------------------------*/
int encode_bit(int inbit, int *stat)
{
	int j;
	int output;

	output = (*(turbo_g.g_matrix+turbo_g.K_num_col+0)) * inbit;

	for (j=1; j<turbo_g.K_num_col; j++)
	{
		output = (output + (*(turbo_g.g_matrix+turbo_g.K_num_col+j)) * (*(stat+j-1)))%2;
	}

	for (j=turbo_g.K_num_col-2; j>0; j--)
	{
		*(stat+j)=*(stat+j-1);
	}

	*(stat+0) = inbit;

	return output;
}


/*---------------------------------------------------------------
FUNCTION: 
	bool gen_mx_punc()

DESCRIPTION:
	This function generate the puncture matrix.

PARAMETERS:

RETURN VALUE:
	1 - If the matrix was successfully generated.
	0 - Error occurred generating the puncture matrix.
---------------------------------------------------------------*/
int gen_mx_punc()
{
	int i;
	int type_punc;
	int *mx_puncture_turbo=NULL;

	for (type_punc=1; type_punc<5; type_punc++)
	{
		switch (type_punc)
		{
		case 1:
			mx_puncture_turbo = mx_puncture_turbo_80;
			break;
		case 2:
			mx_puncture_turbo = mx_puncture_turbo_160;
			break;
		case 4:
			mx_puncture_turbo = mx_puncture_turbo_320;
			break;
		default:
			break;
		}

		if (type_punc==3)
		{
			continue;
		}
		
		for (i=0; i<type_punc*80; i++)
		{
			*(mx_puncture_turbo+i) = 1;
		}
		for (i=0; i<type_punc*80; i++)
		{
			if(i%2)
			{
				*(mx_puncture_turbo+type_punc*80+i) = 0;
			}
			else
			{
				*(mx_puncture_turbo+type_punc*80+i) = 1;
			}
		}
		for (i=0; i<type_punc*80; i++)
		{
			if(i%2)
			{
				*(mx_puncture_turbo+type_punc*80*2+i) = 1;
			}
			else
			{
				*(mx_puncture_turbo+type_punc*80*2+i) = 0;
			}
		}
		switch (type_punc)
		{
		case 1:
			{
				*(mx_puncture_turbo+type_punc*80*2+20) = 0;
				*(mx_puncture_turbo+type_punc*80+20) = 0;

				*(mx_puncture_turbo+type_punc*80*2+41) = 0;
				*(mx_puncture_turbo+type_punc*80+41) = 0;

				*(mx_puncture_turbo+type_punc*80*2+62) = 0;
				*(mx_puncture_turbo+type_punc*80+62) = 0;

				break;
			}
		case 2:
			{
				*(mx_puncture_turbo+type_punc*80*2+50) = 0;
				*(mx_puncture_turbo+type_punc*80+50) = 0;

				*(mx_puncture_turbo+type_punc*80*2+101) = 0;
				*(mx_puncture_turbo+type_punc*80+101) = 0;
				
				*(mx_puncture_turbo+type_punc*80*2+152) = 0;
				*(mx_puncture_turbo+type_punc*80+152) = 0;
				
				break;
			}
		case 4:
			{
				*(mx_puncture_turbo+type_punc*80*2+90) = 0;
				*(mx_puncture_turbo+type_punc*80+90) = 0;

				*(mx_puncture_turbo+type_punc*80*2+191) = 0;
				*(mx_puncture_turbo+type_punc*80+191) = 0;
				
				*(mx_puncture_turbo+type_punc*80*2+272) = 0;
				*(mx_puncture_turbo+type_punc*80+272) = 0;
				
				break;
			}
		default:
			{
				return 0;
			}
		}
	}
	return 1;

}

/*---------------------------------------------------------------
FUNCTION: 
	puncture(int *data_unpunc, int length_unpunc, int *data_punctured, 
			  int M_mx_punc, int N_mx_punc, int times_punc)

DESCRIPTION:
	This function puncture the sending bits.

PARAMETERS:
	INPUT:
		data_unpunc - Contains pointer to the input sequence.
		length_unpunc - Length of "data_unpunc".
		mx_puncture_turbo - Puncture matrix.
		M_mx_punc - Number of rows of mx_puncture_turbo.
		N_mx_punc - Number of columns of mx_puncture_turbo.
		times_punc - The times_punc by which the puncture matrix is used to the input sequence "data_unpunc".

	OUTPUT:
		data_punctured - Contains pointer to the punctured sequence.

RETURN VALUE:
	None
---------------------------------------------------------------*/
void puncture(int *data_unpunc, int length_unpunc, int *data_punctured, 
			  int M_mx_punc, int N_mx_punc, int times_punc)
{
	int i, j, k=0, temp_time=0;
	int *mx_puncture_turbo = NULL;

	switch (N_mx_punc)
	{
	case 80:
		{
			mx_puncture_turbo = mx_puncture_turbo_80;
			break;
		}
	case 160:
		{
			mx_puncture_turbo = mx_puncture_turbo_160;
			break;
		}
	case 320:
		{
			mx_puncture_turbo = mx_puncture_turbo_320;
			break;
		}
	default:
		{
			printf("error in puncturing!\n");
			exit(1);
		}
	}
		
	for (temp_time=0; temp_time<times_punc; temp_time++)
	{
		for (j=0; j<N_mx_punc && (temp_time*M_mx_punc*N_mx_punc+j*M_mx_punc)<length_unpunc; j++)
		{
			for (i=0; i<M_mx_punc && (temp_time*M_mx_punc*N_mx_punc+j*M_mx_punc+i)<length_unpunc;i++)
			{
				if (*(mx_puncture_turbo+i*N_mx_punc+j))
				{
					*(data_punctured+k) = *(data_unpunc+temp_time*M_mx_punc*N_mx_punc+j*M_mx_punc+i);
					k++;
				}
			}
		}
	}
	if (times_punc*M_mx_punc*N_mx_punc<length_unpunc)
	{
		for (i=times_punc*M_mx_punc*N_mx_punc; i<length_unpunc; i++)
		{
			*(data_punctured+k) = *(data_unpunc+i);
			k++;
		}
	}
}

/*---------------------------------------------------------------
FUNCTION: 
	depuncture(float *receive_punc, int length_punc, float *receive_depunced, 
				int M_mx_punc, int N_mx_punc, int times_punc)
	
DESCRIPTION:
	This function fills in the punctured sequence.

PARAMETERS:
	INPUT:
		receive_punc - Contains pointer to the punctured input sequence.
		length_punc - Length of "receive_punc".
		M_mx_punc - Number of rows of mx_puncture_turbo.
		N_mx_punc - Number of columns of mx_puncture_turbo.
		times_punc - The times_punc by which the puncture matrix is used to the input sequence "receive_punc".

	OUTPUT:
		receive_depunced - Contains pointer to the filled sequence.

RETURN VALUE:
	None
---------------------------------------------------------------*/
void depuncture(float *receive_punc, int length_punc, float *receive_depunced, 
				int M_mx_punc, int N_mx_punc, int times_punc)
{
	int i, j, k=0, temp_time=0;
	int *mx_puncture_turbo;

	switch (N_mx_punc)
	{
	case 80:
		{
			mx_puncture_turbo = mx_puncture_turbo_80;
			break;
		}
	case 160:
		{
			mx_puncture_turbo = mx_puncture_turbo_160;
			break;
		}
	case 320:
		{
			mx_puncture_turbo = mx_puncture_turbo_320;
			break;
		}
	default:
		{
			printf("error in puncturing!\n");
			exit(1);
		}
	}

	for (temp_time=0; temp_time<times_punc; temp_time++)
	{
		for (j=0; j<N_mx_punc; j++)
		{
			for (i=0; i<M_mx_punc && k<length_punc; i++)
			{
				if (*(mx_puncture_turbo+i*N_mx_punc+j))
				{
					*(receive_depunced+temp_time*M_mx_punc*N_mx_punc+j*M_mx_punc+i) = *(receive_punc+k);
					k++;
				}
				else
				{
					*(receive_depunced+temp_time*M_mx_punc*N_mx_punc+j*M_mx_punc+i) = 0;
				}
			}
		}
	}
	if (k<length_punc)
	{
		for (i=times_punc*M_mx_punc*N_mx_punc; k<length_punc; i++)
		{
			*(receive_depunced+i) = *(receive_punc+k);
			k++;
		}
	}
}

/*---------------------------------------------------------------
FUNCTION: 
	void demultiplex(double *rec_turbo, int len_info, double *yk_turbo)
	
DESCRIPTION:
	Demultiplex the receiving data.

PARAMETERS:
	INPUT:
		rec_turbo - Contains pointer to receiving data.
		len_info - Length of the input information bits.
		type_flow - 1 for traffic flow, 0 for suplement flow.

	OUTPUT:
		yk_turbo - Contains pointer to the sequence after demultiplexing.

RETURN VALUE:
	None
---------------------------------------------------------------*/
void demultiplex(float *rec_turbo, int len_info, float *yk_turbo, int type_flow)
{
	int i;

	int len_total = len_info+M_num_reg;

	float *info2, *inted_info2;

	if ((info2=(float *)malloc(len_info*sizeof(float)))==NULL)
	{
	  printf("\n fail to allocate memory of info2 \n");
	  exit(1);  
	}
	if ((inted_info2=(float *)malloc(len_info*sizeof(float)))==NULL)
	{
	  printf("\n fail to allocate memory of inted_info2 \n");
	  exit(1);  
	}
	
	/* for information bits */
	for(i=0; i<len_info; i++)
	{
		*(info2+i) = *(yk_turbo+2*i) = *(rec_turbo+3*i);
		*(yk_turbo+2*i+1) = *(rec_turbo+3*i+1);
		*(yk_turbo+2*len_total+2*i+1) = *(rec_turbo+3*i+2);
	}

	interleave_float(info2, inted_info2, TYPE_INTERLEAVER, len_info, type_flow);

	for (i=0; i<len_info; i++)
	{
		*(yk_turbo+2*len_total+2*i) = *(inted_info2+i);
	}

	/* for tail bits */
	for (i=0; i<2*M_num_reg; i++)
	{
		*(yk_turbo+2*len_info+i) = *(rec_turbo+3*len_info+i);
		*(yk_turbo+2*len_total+2*len_info+i) = *(rec_turbo+3*len_info+2*M_num_reg+i);
	}

	free(info2);
	free(inted_info2);
}

/*---------------------------------------------------------------
FUNCTION: 
	Log_MAP_decoder(float *recs_turbo, float *La_turbo, int terminated, float *LLR_all_turbo, int len_total)

DESCRIPTION:
	Log-MAP decoder which caculate the LLR of input sequence.

PARAMETERS:
	INPUT:
		recs_turbo - Scaled received bits.
		La_turbo - A priori information for the current decoder, 
			scrambled version of extrinsic information of the previous decoder.
		terminated - Indicate the turbo_trellis is terminated or not.
					 1 - terminated
					 0 - not terminated
		len_total - Length of the input data sequence.
	OUTPUT:
		LLR_all_turbo - The caculated LLR information sequence.

RETURN VALUE:
	None
---------------------------------------------------------------*/
void Log_MAP_decoder(float *recs_turbo, float *La_turbo, int terminated, float *LLR_all_turbo, int len_total)
{
	int i, j;

	float *alpha_Log, *beta_Log, *gama_Log;

	float *tempmax;
	float *temp0, *temp1;
	float tempx, tempy;

	if ((alpha_Log=(float *)malloc(n_states*(len_total+1)*sizeof(float)))==NULL)
	{
	  printf("\n fail to allocate memory of alpha_Log \n");
	  exit(1);  
	}
	if ((beta_Log=(float *)malloc(n_states*(len_total+1)*sizeof(float)))==NULL)
	{
	  printf("\n fail to allocate memory of beta_Log \n");
	  exit(1);  
	}
	if ((gama_Log=(float *)malloc(n_states*len_total*2*sizeof(float)))==NULL)
	{
	  printf("\n fail to allocate memory of gama_Log \n");
	  exit(1);  
	}

	if ((tempmax=(float *)malloc((len_total+1)*sizeof(float)))==NULL)
	{
	  printf("\n fail to allocate memory of tempmax \n");
	  exit(1);  
	}

	if ((temp0=(float *)malloc(n_states*sizeof(float)))==NULL)
	{
	  printf("\n fail to allocate memory of temp0 \n");
	  exit(1);  
	}
	if ((temp1=(float *)malloc(n_states*sizeof(float)))==NULL)
	{
	  printf("\n fail to allocate memory of temp1 \n");
	  exit(1);  
	}
	
	/*===== Initialization of alpha_Log and beta_Log =====*/
	*(alpha_Log+0) = 0;
	*(beta_Log+len_total) = 0;

	for (i=1; i<n_states; i++)
	{
		*(alpha_Log+i*(len_total+1)+0) = (float)-INFTY;

		if (terminated)
		{
			*(beta_Log+i*(len_total+1)+len_total) = (float)-INFTY;
		}
		else
		{
			*(beta_Log+i*(len_total+1)+len_total) = 0;
		}
	}
	/*========compute Gama_Log========*/
	for (i=0; i<len_total; i++)			/* 0--len_total-1 代表 1--len_total */
	{
		for (j=0; j<n_states; j++)		/* j->k */
		{
			*(gama_Log+j*len_total*2+i*2+0) 
				= -*(recs_turbo+2*i) + *(recs_turbo+2*i+1)*(*(turbo_trellis.mx_nextout+j*4+1)) - *(La_turbo+i)/2;
			*(gama_Log+j*len_total*2+i*2+1)
				= *(recs_turbo+2*i) + *(recs_turbo+2*i+1)*(*(turbo_trellis.mx_nextout+j*4+3)) + *(La_turbo+i)/2;
		}
	}

	/*========Trace forward, compute Alpha_Log========*/
	for (i=1; i<len_total+1; i++)
	{
		for (j=0; j<n_states; j++)		/* 以j为中心*/
		{
			tempx = *(gama_Log+(*(turbo_trellis.mx_laststat+j*2+0))*len_total*2+(i-1)*2+0)
					+ *(alpha_Log+(*(turbo_trellis.mx_laststat+j*2+0))*(len_total+1)+i-1);
			tempy = *(gama_Log+(*(turbo_trellis.mx_laststat+j*2+1))*len_total*2+(i-1)*2+1)
					+ *(alpha_Log+(*(turbo_trellis.mx_laststat+j*2+1))*(len_total+1)+i-1);
			*(alpha_Log+j*(len_total+1)+i) = E_algorithm(tempx, tempy);
		}

		for (j=0; j<n_states; j++)
		{
			if (*(tempmax+i) < *(alpha_Log+j*(len_total+1)+i))
			{
				*(tempmax+i) = *(alpha_Log+j*(len_total+1)+i);
			}
		}

		for(j=0; j<n_states; j++)
		{
			*(alpha_Log+j*(len_total+1)+i) = *(alpha_Log+j*(len_total+1)+i) - *(tempmax+i);
		}

	}

	/*========Trace backward, compute Beta_Log========*/
	for (i=len_total-1; i>=0; i--)
	{
		for (j=0; j<n_states; j++)		/*j为中心*/
		{
			tempx = *(gama_Log+j*len_total*2+i*2+0) 
					+ *(beta_Log+(*(turbo_trellis.mx_nextstat+j*2+0))*(len_total+1)+i+1);
			tempy = *(gama_Log+j*len_total*2+i*2+1) 
					+ *(beta_Log+(*(turbo_trellis.mx_nextstat+j*2+1))*(len_total+1)+i+1);

			*(beta_Log+j*(len_total+1)+i) = E_algorithm(tempx, tempy);
		}

		for (j=0; j<n_states; j++)
		{
			*(beta_Log+j*(len_total+1)+i) = *(beta_Log+j*(len_total+1)+i) - *(tempmax+i+1);
		}
	}

	/*===Compute the soft output,log-likelihood ratio of symbols in the frame===*/
	for (i=0; i<len_total; i++)	
	{
		for (j=0; j<n_states; j++)
		{
			*(temp0+j) = *(gama_Log+(*(turbo_trellis.mx_laststat+j*2+0))*len_total*2+i*2+0) 
				+ *(alpha_Log+*(turbo_trellis.mx_laststat+j*2+0)*(len_total+1)+i)
				+ *(beta_Log+ j*(len_total+1)+i+1);

			*(temp1+j) = *(gama_Log+(*(turbo_trellis.mx_laststat+j*2+1))*len_total*2+i*2+1) 
				+ *(alpha_Log+*(turbo_trellis.mx_laststat+j*2+1)*(len_total+1)+i)
				+ *(beta_Log+j*(len_total+1)+i+1);
		}

		*(LLR_all_turbo+i) = E_algorithm_seq(temp1, n_states) - E_algorithm_seq(temp0, n_states);
	}

	free(alpha_Log);
	free(beta_Log);
	free(gama_Log);
	free(tempmax);
	free(temp0);
	free(temp1);
}


/*---------------------------------------------------------------
FUNCTION: 
	void MAX_Log_MAP_decoder(float *recs_turbo, float *La_turbo, int terminated, float *LLR_all_turbo, int len_total)

DESCRIPTION:
	MAX_Log-MAP decoder which caculate the LLR of input sequence.

PARAMETERS:
	INPUT:
		recs_turbo - Scaled received bits.
		La_turbo - A priori information for the current decoder, 
			scrambled version of extrinsic information of the previous decoder.
		terminated - Indicate the turbo_trellis is terminated or not.
					 1 - terminated
					 0 - not terminated
		len_total - Length of the input data sequence.
	OUTPUT:
		LLR_all_turbo - The caculated LLR information sequence.

RETURN VALUE:
	None
---------------------------------------------------------------*/
void MAX_Log_MAP_decoder(float *recs_turbo, float *La_turbo, int terminated, float *LLR_all_turbo, int len_total)
{
	int i, j;

	float *alpha_Log, *beta_Log, *gama_Log;

	float *tempmax;
	float *temp0, *temp1;
	float tempx, tempy;

	if ((alpha_Log=(float *)malloc(n_states*(len_total+1)*sizeof(float)))==NULL)
	{
	  printf("\n fail to allocate memory of alpha_Log \n");
	  exit(1);  
	}
	if ((beta_Log=(float *)malloc(n_states*(len_total+1)*sizeof(float)))==NULL)
	{
	  printf("\n fail to allocate memory of beta_Log \n");
	  exit(1);  
	}
	if ((gama_Log=(float *)malloc(n_states*len_total*2*sizeof(float)))==NULL)
	{
	  printf("\n fail to allocate memory of gama_Log \n");
	  exit(1);  
	}

	if ((tempmax=(float *)malloc((len_total+1)*sizeof(float)))==NULL)
	{
	  printf("\n fail to allocate memory of tempmax \n");
	  exit(1);  
	}

	if ((temp0=(float *)malloc(n_states*sizeof(float)))==NULL)
	{
	  printf("\n fail to allocate memory of temp0 \n");
	  exit(1);  
	}
	if ((temp1=(float *)malloc(n_states*sizeof(float)))==NULL)
	{
	  printf("\n fail to allocate memory of temp1 \n");
	  exit(1);  
	}
	
	/*===== Initialization of Alpha and beta_Log =====*/
	*(alpha_Log+0) = 0;
	*(beta_Log+len_total) = 0;

	for (i=1; i<n_states; i++)
	{
		*(alpha_Log+i*(len_total+1)+0) = (float)-INFTY;

		if (terminated)
		{
			*(beta_Log+i*(len_total+1)+len_total) = (float)-INFTY;
		}
		else
		{
			*(beta_Log+i*(len_total+1)+len_total) = 0;
		}
	}
	/*========compute Gama_Log========*/
	for (i=0; i<len_total; i++)			/* 0--len_total-1 代表 1--len_total */
	{
		for (j=0; j<n_states; j++)		/* j->k */
		{
			*(gama_Log+j*len_total*2+i*2+0) 
				= -*(recs_turbo+2*i) + *(recs_turbo+2*i+1)*(*(turbo_trellis.mx_nextout+j*4+1)) - *(La_turbo+i)/2;
			*(gama_Log+j*len_total*2+i*2+1)
				= *(recs_turbo+2*i) + *(recs_turbo+2*i+1)*(*(turbo_trellis.mx_nextout+j*4+3)) + *(La_turbo+i)/2;
		}
	}

	/*========Trace forward, compute Alpha_Log========*/
	for (i=1; i<len_total+1; i++)
	{
		for (j=0; j<n_states; j++)		/* 以j为中心*/
		{
			tempx = *(gama_Log+(*(turbo_trellis.mx_laststat+j*2+0))*len_total*2+(i-1)*2+0)
					+ *(alpha_Log+(*(turbo_trellis.mx_laststat+j*2+0))*(len_total+1)+i-1);
			tempy = *(gama_Log+(*(turbo_trellis.mx_laststat+j*2+1))*len_total*2+(i-1)*2+1)
					+ *(alpha_Log+(*(turbo_trellis.mx_laststat+j*2+1))*(len_total+1)+i-1);
			*(alpha_Log+j*(len_total+1)+i) = tempx>tempy?tempx:tempy;
		}

		for (j=0; j<n_states; j++)
		{
			if (*(tempmax+i) < *(alpha_Log+j*(len_total+1)+i))
			{
				*(tempmax+i) = *(alpha_Log+j*(len_total+1)+i);
			}
		}

		for(j=0; j<n_states; j++)
		{
			*(alpha_Log+j*(len_total+1)+i) = *(alpha_Log+j*(len_total+1)+i) - *(tempmax+i);
		}

	}

	/*========Trace backward, compute Beta_Log========*/
	for (i=len_total-1; i>=0; i--)
	{
		for (j=0; j<n_states; j++)		/*j为中心*/
		{
			tempx = *(gama_Log+j*len_total*2+i*2+0) 
					+ *(beta_Log+(*(turbo_trellis.mx_nextstat+j*2+0))*(len_total+1)+i+1);
			tempy = *(gama_Log+j*len_total*2+i*2+1) 
					+ *(beta_Log+(*(turbo_trellis.mx_nextstat+j*2+1))*(len_total+1)+i+1);

			*(beta_Log+j*(len_total+1)+i) = tempx>tempy?tempx:tempy;
		}

		for (j=0; j<n_states; j++)
		{
			*(beta_Log+j*(len_total+1)+i) = *(beta_Log+j*(len_total+1)+i) - *(tempmax+i+1);
		}
	}

	/*===Compute the soft output,log-likelihood ratio of symbols in the frame===*/
	for (i=0; i<len_total; i++)	
	{
		for (j=0; j<n_states; j++)
		{
			*(temp0+j) = *(gama_Log+(*(turbo_trellis.mx_laststat+j*2+0))*len_total*2+i*2+0) 
				+ *(alpha_Log+*(turbo_trellis.mx_laststat+j*2+0)*(len_total+1)+i)
				+ *(beta_Log+ j*(len_total+1)+i+1);

			*(temp1+j) = *(gama_Log+(*(turbo_trellis.mx_laststat+j*2+1))*len_total*2+i*2+1) 
				+ *(alpha_Log+*(turbo_trellis.mx_laststat+j*2+1)*(len_total+1)+i)
				+ *(beta_Log+j*(len_total+1)+i+1);
		}

		*(LLR_all_turbo+i) = get_max(temp1, n_states) - get_max(temp0, n_states);
	}

	free(alpha_Log);
	free(beta_Log);
	free(gama_Log);
	free(tempmax);
	free(temp0);
	free(temp1);
}

/*---------------------------------------------------------------
FUNCTION: 
	get_max(float *data_seq, int length)

DESCRIPTION:
	Get the maximum value of a data_seq sequence.

PARAMETERS:
	INPUT:
		data_seq - Contains pointer to the input sequence.
		length - Length of "data_seq".

RETURN VALUE:
	The maximum one of this data_seq sequence.
---------------------------------------------------------------*/
float get_max(float *data_seq, int length)
{
	int i;
	float temp;
	temp = *(data_seq+0);
	for (i=1; i<length; i++)
	{
		if (temp < *(data_seq+i))
		{
			temp = *(data_seq+i);
		}
	}
	return temp;
}

/*---------------------------------------------------------------
FUNCTION: 
	E_algorithm(float x, float y)

DESCRIPTION:
	Compute: log(exp(x) + exp(y)) = max(x,y)+log(1+exp(-|y-x|)) 
	where log(1+exp(-|y-x|)) can be implemented in a lookup table.
					
PARAMETERS:
	INPUT:
		x - One number.
		y - The other number.

RETURN VALUE:
	log(exp(x) + exp(y)).
---------------------------------------------------------------*/
float E_algorithm(float x, float y) // log(exp(x) + exp(y)) = max(x,y)+f(-|y-x|) 
{
	float temp = (y-x)>0? (y-x):(x-y);
	int i;

	if (temp>=4.3758)
	{
		temp = 0;
	}
	else
	{
		for (i=0; i<16 && temp>=lookup_index_Log_MAP[i]; i++)
		{
			;
		}
		temp = (float)lookup_table_Log_MAP[i-1];
	}
	
	return ( (x>y?x:y) + temp );
}	

/*---------------------------------------------------------------
FUNCTION: 
	E_algorithm_seq(float *data_seq, int length)

DESCRIPTION:
	E_algorithm for a data_seq sequence.

PARAMETERS:
	INPUT:
		data_seq - Contains pointer to the input sequence.
		length - Length of "data_seq".

RETURN VALUE:
	The result of E_algorithm for this data_seq sequence.
---------------------------------------------------------------*/
float E_algorithm_seq(float *data_seq, int length)
{
	int i;
	float temp;
	temp = E_algorithm(*(data_seq+0), *(data_seq+1));
	for (i=2; i<length; i++)
	{
		temp = E_algorithm(temp, *(data_seq+i));
	}
	return temp;
}

/*---------------------------------------------------------------
FUNCTION: 
	void decision(double *LLR_seq, int length, int *output)
	
DESCRIPTION:
	Make the final decision.

PARAMETERS:
	INPUT:
		LLR_seq - The LLR_seq information sequence.
		length - Length of "LLR_seq".
	OUTPUT:
		output - Contains pointer to the output data sequence.

RETURN VALUE:
	None
---------------------------------------------------------------*/
void decision(float *LLR_seq, int length, int *output)
{
	int i;

	for (i=0; i<length; i++)
	{
		if (*(LLR_seq+i) < 0)
		{
			*(output+i) = 0;
		}
		else
		{
			*(output+i) = 1;
		}
	}		
}