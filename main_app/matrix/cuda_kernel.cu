
#include "user_nn_matrix_cuda.h"
#include "user_nn_matrix.h"

//__global__ ��������
//__device__ �豸����
//transpose_naive << < 1, matrix_h >> >(sub_matrix, src_matrix, matrix_w, matrix_h);
//extern "C" template <int BLOCK_SIZE>
//extern "C"
__global__ void transposeNaive_array(float *odata, float* idata, int width, int height)
{
	//threadId�̵߳����� blockDim�߳̿��ά�� gridDim�̸߳��ά��
	unsigned int xIndex = blockDim.x * blockIdx.x + threadIdx.x;//grid���ֳ�1ά��block����Ϊ1ά ��ȡ������
	unsigned int yIndex = blockDim.y * blockIdx.y + threadIdx.y;//
	unsigned int index_in = xIndex + width * yIndex;//�����߳����� ���Ե�ַ
	unsigned int index_out = yIndex + height * xIndex;

	unsigned int width_index = 0;
	for (width_index = 0; width_index < height; width_index++){
		odata[index_out + width_index] = idata[index_in + width_index * width];//����һ��������һ�е�λ��
	}
	/*
	unsigned int xIndex = blockDim.x * blockIdx.x + threadIdx.x;
	unsigned int yIndex = blockDim.y * blockIdx.y + threadIdx.y;

	if (xIndex < width && yIndex < height)
	{
		unsigned int index_in = xIndex + width * yIndex;
		unsigned int index_out = yIndex + height * xIndex;
		odata[index_out] = idata[index_in];
	}
	*/
}

__global__ void transposeNaive_block(float *odata, float *idata, int width, int height)
{
	int xIndex = blockIdx.x * TILE_DIM + threadIdx.x;
	int yIndex = blockIdx.y * TILE_DIM + threadIdx.y;

	int index_in = xIndex + width * yIndex;
	int index_out = yIndex + height * xIndex;

	for (int i = 0; i<TILE_DIM; i += BLOCK_ROWS)
	{
		odata[index_out + i] = idata[index_in + i*width];
	}
}

//����ת��
extern "C"
void user_nn_matrix_transpose_cuda(user_nn_matrix *src_matrix){
	int src_count = src_matrix->width * src_matrix->height;
	float *cuda_input_matrix = NULL;
	float *cuda_output_matrix = NULL;

	cudaMalloc((void **)&cuda_input_matrix, src_count * sizeof(float));//�������ݿռ�
	cudaMalloc((void **)&cuda_output_matrix, src_count * sizeof(float));//�������ݿռ�

	cudaMemcpy(cuda_input_matrix, src_matrix->data, src_count * sizeof(float), cudaMemcpyHostToDevice);//��������
	//<<<grid, threads>>> grid��ʾ�̸߳��ж��ٶ��ٸ��߳̿飬threads��ʾһ���߳̿����ж��ٸ��߳�
	if ((src_matrix->width%TILE_DIM == 0) && (src_matrix->height%TILE_DIM == 0) && (src_matrix->width == src_matrix->height)){
		dim3 grid(src_matrix->width / TILE_DIM, src_matrix->height / TILE_DIM), threads(TILE_DIM, BLOCK_ROWS);//��̬��������Ҫ��դ����߳�
		transposeNaive_block << <grid, threads >> >(cuda_output_matrix, cuda_input_matrix, src_matrix->width, src_matrix->height);//���о���ת��
	}
	else{
		transposeNaive_array << <1, ((src_matrix->width > src_matrix->height) ? src_matrix->width : src_matrix->height) >> >(cuda_output_matrix, cuda_input_matrix, src_matrix->width, src_matrix->height);//���о���ת��
	}
	cudaMemcpy(src_matrix->data, cuda_output_matrix, src_count * sizeof(float), cudaMemcpyDeviceToHost);//��������
	//��������λ��
	src_matrix->width  = src_matrix->width ^ src_matrix->height;
	src_matrix->height = src_matrix->width ^ src_matrix->height;
	src_matrix->width  = src_matrix->width ^ src_matrix->height;

	cudaFree(cuda_input_matrix);
	cudaFree(cuda_output_matrix);
}
#ifdef WIN64
user_nn_matrix * user_nn_matrix_mult_matrix_cuda(user_nn_matrix *src_matrix, user_nn_matrix *sub_matrix){
	cublasHandle_t cuda_handle;
	cublasStatus_t status;
	cudaError_t error;

	user_nn_matrix *result = NULL;//�������
	int src_count = src_matrix->width * src_matrix->height;
	int sub_count = sub_matrix->width * sub_matrix->height;
	float *src_matrix_cuda = NULL, *sub_matrix_cuda = NULL, *result_cuda = NULL;
	float alpha = 1.0, beta = 0.0;

	if (src_matrix->width != sub_matrix->height){//����˻�ֻ�е���һ�����������=�ڶ��������������������
		return NULL;
	}
	status = cublasCreate_v2(&cuda_handle);
	if (status != CUBLAS_STATUS_SUCCESS){
		printf("status error");
	}

	result = user_nn_matrix_create(sub_matrix->width, src_matrix->height);//�����µľ���

	error = cudaMalloc((void **)&src_matrix_cuda, src_count * sizeof(float));//�������ݿռ�
	error = cudaMalloc((void **)&sub_matrix_cuda, sub_count * sizeof(float));//�������ݿռ�
	error = cudaMalloc((void **)&result_cuda, result->width * result->height * sizeof(float));//���䱣�����ľ���

	error = cudaMemcpy(src_matrix_cuda, src_matrix->data, src_count * sizeof(float), cudaMemcpyHostToDevice);//��������
	error = cudaMemcpy(sub_matrix_cuda, sub_matrix->data, sub_count * sizeof(float), cudaMemcpyHostToDevice);//��������

	//user_nn_matrix_memset(result,1.2f);
	//cudaMemcpy(result_cuda, result->data, result->width * result->height * sizeof(float), cudaMemcpyHostToDevice);//��������

	//��ʽ��C = alpha*op(A)xop(B)+beta*C
	//�����A�Ƿ�ת�á�B�Ƿ�ת�á�����A��������A������A����������B��alpha��Aָ�롢lda��B��ldb��beta��C��ldc
	status = cublasSgemm_v2(cuda_handle, CUBLAS_OP_T, CUBLAS_OP_T, result->height, result->width, src_matrix->width, &alpha, src_matrix_cuda, src_matrix->width, sub_matrix_cuda, sub_matrix->width, &beta, result_cuda, result->height);
	error = cudaThreadSynchronize();

	error = cudaMemcpy(result->data, result_cuda, result->width * result->height * sizeof(float), cudaMemcpyDeviceToHost);//��������


	cudaFree(src_matrix_cuda);
	cudaFree(sub_matrix_cuda);
	cudaFree(result_cuda);
	cublasDestroy_v2(cuda_handle);

	return result;
}
#endif