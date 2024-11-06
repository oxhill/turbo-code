#include "turbo_code_Log_MAP.h"
#include "other_functions.h"

/*---------------------------------------------------------------
FUNCTION: 
	void gen_source(int *data, int length)
	
DESCRIPTION:
	This function generate the source bits for simulation.产生仿真所需要的01比特流

PARAMETERS:
	INPUT:
		length - Length of needed data.  输入：所需的比特流长度
	OUTPUT:
		data - Contains pointer to source data sequence. 输出：包含指向源数据序列的指针（也就是源数据序列）

RETURN VALUE:
	None    返回值：无
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
* 函数介绍：产生长度为n的高斯随机序列
* 输入参数：mean：均值
            sigma：标准差
			seed：一个随机种子
* 输出参数：a：长度为n的高斯随机序列
* 返回值：  返回一个trellis结构
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
        }/*按照中心极限定理产生服从高斯分布的随机数*/
        *(a+k)=mean+sigma*(t-6.0);
    }
    return;
}


/*读取文件数据的函数,变量依次是文件名（表示要读取数据的文件路径），一个指向浮点型数组的指针（用于存储从文件读取的数值），一个指向整数的指针（用于存储读取的数值个数）*/
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
	printf("Total numbers read: %d\n", *length); // 调试输出
}




/*读取文件数据的函数,变量依次是文件名（表示要读取数据的文件路径），一个指向整数型数组的指针（用于存储从文件读取的数值），一个指向整数的指针（用于存储读取的数值个数）*/
void read_data(const char* filename, int* trafficflow_source, int* length) {
	FILE* file;
	errno_t err = fopen_s(&file, filename, "r");
	if (err != 0 || !file) {
		printf("Error opening file: %s\n", filename);
		exit(1);
	}

	*length = 0;
	while (fscanf_s(file, "%d", &trafficflow_source[*length]) == 1) { // 读取整数
		
		//printf("%d\n", &trafficflow_source[*length]); // 输出读取的比特
		(*length)++;
	}
	// Step 2: 分配合适大小的数组内存
	*trafficflow_source = (float*)malloc(*length * sizeof(float));
	// Step 3: 回到文件开始位置，重新读取数据
	rewind(file);
	
	while (fscanf_s(file, "%d", &trafficflow_source[*length]) == 1) {
		length++;
	}
	fclose(file);
	printf("Total bits read: %d\n", *length); // 调试输出
}
#define MAX_SIZE 5096  // 定义数组的最大容量

int read_integers_from_file(const char* filename, int* array, int max_size) {
	FILE* file;
	if (fopen_s(&file, filename, "r") != 0) {  // 使用 fopen_s 打开文件
		perror("文件打开失败");  // 输出文件打开错误的详细信息
		return -1;
	}

	int count = 0;
	int result;
	while (count < max_size) {
		result = fscanf_s(file, "%d", &array[count]);  // 使用 fscanf_s 读取整数
		if (result == 1) {  // 成功读取一个整数
			count++;
		}
		else if (result == EOF) {  // 文件读取到末尾
			break;
		}
		else {  // fscanf_s 读取失败
			fprintf(stderr, "读取错误：无法解析文件中的数据\n");
			fclose(file);  // 关闭文件
			return -1;
		}
	}

	fclose(file);  // 关闭文件
	return count;  // 返回读取的元素个数
}