#include "turbo_code_Log_MAP.h"
#include "other_functions.h"

/*---------------------------------------------------------------
FUNCTION: 
	void gen_source(int *data, int length)
	
DESCRIPTION:
	This function generate the source bits for simulation.������������Ҫ��01������

PARAMETERS:
	INPUT:
		length - Length of needed data.  ���룺����ı���������
	OUTPUT:
		data - Contains pointer to source data sequence. ���������ָ��Դ�������е�ָ�루Ҳ����Դ�������У�

RETURN VALUE:
	None    ����ֵ����
---------------------------------------------------------------*/
void gen_source(int *data, int length)
{
	double temp;
	int i;

	for (i=0; i<length; i++)
	{
		//temp = (double)rand()/RAND_MAX;
		temp = random_turbo();
		if (temp <= 0.5)
		{
			*(data+i) = 0;
		}
		else 
		{
			*(data+i) = 1;
		}
	}

}


/*---------------------------------------------------------------
FUNCTION: 
	void AWGN(int *send, double *r, double sigma, int totallength)
	
DESCRIPTION:
	This function simulate a AWGN channel.

PARAMETERS:
	INPUT:
		send - Input bit sequence need to add noise.
		sigma - Standard deviation of AWGN noise
		totallength - Length of "send".
	OUTPUT:
		r - Contains pointer to the data sequence added with gaussian white noise.

RETURN VALUE:
	None
---------------------------------------------------------------*/
void AWGN(float *send, float *r, float sigma, int totallength)
{
	int i;
	double *noise = (double *)malloc(sizeof(double)*totallength);
	double seed =  3.0 - (double)((rand() & RAND_MAX)/(double)RAND_MAX)/10e6;
	mgrns(0,sigma,seed,totallength,noise);
	for(i=0; i<totallength; i++)
	{
		*(r+i) = (float)( *(send+i) + *(noise+i) );
	}
	free(noise);
}

/*
* �������ܣ���������Ϊn�ĸ�˹�������
* ���������mean����ֵ
            sigma����׼��
			seed��һ���������
* ���������a������Ϊn�ĸ�˹�������
* ����ֵ��  ����һ��trellis�ṹ
*/
void mgrns(double mean,double sigma,double seed,int n,double *a)
{ int i,k,m;
    double s,w,v,t;
    s=65536.0; w=2053.0; v=13849.0;
    for (k=0; k<=n-1; k++)
	{
		t=0.0;
		for (i=1; i<=12; i++)
        { 
			seed=seed*w+v; m=(int)(seed/s);
            seed=seed-m*s; t=t+(seed)/s;
        }/*�������ļ��޶���������Ӹ�˹�ֲ��������*/
        *(a+k)=mean+sigma*(t-6.0);
    }
    return;
}


/*��ȡ�ļ����ݵĺ���,�����������ļ�������ʾҪ��ȡ���ݵ��ļ�·������һ��ָ�򸡵��������ָ�루���ڴ洢���ļ���ȡ����ֵ����һ��ָ��������ָ�루���ڴ洢��ȡ����ֵ������*/
void read_encoded_data(const char* filename, float* trafficflow_for_decode, int* length) {
	FILE* file;
	errno_t err = fopen_s(&file, filename, "r");
	if (err != 0 || !file) {
		printf("Error opening file: %s\n", filename);
		exit(1);
	}

	*length = 0;
	while (fscanf_s(file, "%f", &trafficflow_for_decode[*length]) == 1) {
		(*length)++;
	}

	fclose(file);
	printf("Total numbers read: %d\n", *length); // �������
}




/*��ȡ�ļ����ݵĺ���,�����������ļ�������ʾҪ��ȡ���ݵ��ļ�·������һ��ָ�������������ָ�루���ڴ洢���ļ���ȡ����ֵ����һ��ָ��������ָ�루���ڴ洢��ȡ����ֵ������*/
void read_data(const char* filename, int* trafficflow_source, int* length) {
	FILE* file;
	errno_t err = fopen_s(&file, filename, "r");
	if (err != 0 || !file) {
		printf("Error opening file: %s\n", filename);
		exit(1);
	}

	*length = 0;
	while (fscanf_s(file, "%d", &trafficflow_source[*length]) == 1) { // ��ȡ����
		
		//printf("%d\n", &trafficflow_source[*length]); // �����ȡ�ı���
		(*length)++;
	}
	// Step 2: ������ʴ�С�������ڴ�
	*trafficflow_source = (float*)malloc(*length * sizeof(float));
	// Step 3: �ص��ļ���ʼλ�ã����¶�ȡ����
	rewind(file);
	
	while (fscanf_s(file, "%d", &trafficflow_source[*length]) == 1) {
		length++;
	}
	fclose(file);
	printf("Total bits read: %d\n", *length); // �������
}
#define MAX_SIZE 5096  // ����������������

int read_integers_from_file(const char* filename, int* array, int max_size) {
	FILE* file;
	if (fopen_s(&file, filename, "r") != 0) {  // ʹ�� fopen_s ���ļ�
		perror("�ļ���ʧ��");  // ����ļ��򿪴������ϸ��Ϣ
		return -1;
	}

	int count = 0;
	int result;
	while (count < max_size) {
		result = fscanf_s(file, "%d", &array[count]);  // ʹ�� fscanf_s ��ȡ����
		if (result == 1) {  // �ɹ���ȡһ������
			count++;
		}
		else if (result == EOF) {  // �ļ���ȡ��ĩβ
			break;
		}
		else {  // fscanf_s ��ȡʧ��
			fprintf(stderr, "��ȡ�����޷������ļ��е�����\n");
			fclose(file);  // �ر��ļ�
			return -1;
		}
	}

	fclose(file);  // �ر��ļ�
	return count;  // ���ض�ȡ��Ԫ�ظ���
}