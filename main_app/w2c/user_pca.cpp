

#include "user_pca.h"


//��һ�� �����������ÿһά�ȵ�ƽ��ֵ�����ж�ÿ��ά�ȼ�ȥƽ��ֵ
//�ڶ��� ������Э�������
//������ ��Э���������ֵ����������
//���Ĳ� ����ֵ�������� ѡ��K�����ֵ ��K��Ϊ���������������������
//���岽 ���о���ͶӰ�õ�����ά�Ⱥ�����

//flag1:x11 x12 x13 ... x1n
//flag2:x21 x22 x23 ... x2n
//flag3:x31 x32 x33 ... x3n
//.
//.
//flagi:xi1 xi2 xi3 ... xin
//
//PCA��ά�㷨
//src_matrix��Ŀ�����
//���� �������������� ���α���COEFF, LATENT,SCORE 
//�ο�matlab
user_nn_list_matrix *user_pca_process(user_nn_matrix *src_matrix, float epsilon, eigs_type type) {
	user_nn_list_matrix *result = NULL;//

	user_nn_matrix *result_coeff = NULL;//��������
	user_nn_matrix *result_latent = NULL;//����ֵ�Խ�Ԫ��
	user_nn_matrix *result_score = NULL;//��������
	
	user_nn_matrix *src_matrix_cov = NULL;//����Э�������
	user_nn_matrix *src_matrix_mean = NULL;//�����ֵ����
	user_nn_matrix *src_matrix_mean_ext = NULL;//�����ֵ�������չ����
	user_nn_matrix *src_matrix_zero = NULL;//Ŀ������ȥĿ���ֵ����Ľ�� ���㻯
	user_nn_matrix *src_vector = NULL;//��������
	user_nn_matrix *src_vector_list = NULL;//�Խ�������
	user_nn_list_matrix *src_matrix_qr = NULL;//�����������ֵ����������
	
	src_matrix_zero = user_nn_matrix_create(src_matrix->width, src_matrix->height);//��������
	src_matrix_cov = user_nn_matrix_cov(src_matrix);//��Ŀ�����Ľ�Э�������
	src_matrix_mean = user_nn_matrix_mean(src_matrix);//���Ŀ�����ľ�ֵ
	src_matrix_qr = user_nn_eigs(src_matrix_cov, USER_PCA_EIGS_EPSILON, type);//����eigs�㷨 ��������ֵ����������
	src_matrix_mean_ext = user_nn_matrix_repmat(src_matrix_mean, src_matrix->height, 1);//������չ����
	user_nn_matrix_cum_matrix_mult_alpha(src_matrix_zero, src_matrix, src_matrix_mean_ext,-1.0f);//ԭʼ�����ȥƽ��ֵ����
	src_vector = user_nn_matrix_mult_matrix(src_matrix_zero, src_matrix_qr->matrix->next);//��ȡ����ά�Ⱥ����������
	src_vector_list = user_nn_matrix_diag(src_matrix_qr->matrix);//��ȡQR�ֽ������ֵ����ĶԽ���Ԫ�أ�����һ��

	result_latent = user_nn_matrix_cut_vector(src_vector_list, src_vector_list, epsilon);//��ȡQR�ֽ������ֵ����ĶԽ���Ԫ�أ�����һ��
	result_coeff = user_nn_matrix_cut_vector(src_matrix_qr->matrix->next, src_vector_list, epsilon);//ͨ���Խ�Ԫ�زü�����
	result_score = user_nn_matrix_cut_vector(src_vector, src_vector_list, epsilon);//ͨ���Խ�Ԫ�زü�����

	//��Ӿ���
	result_coeff->next = result_latent; result_latent->next = result_score;//����Ϊ��������
	result = (user_nn_list_matrix *)malloc(sizeof(user_nn_list_matrix));//����ռ�
	result->width = 3; 
	result->height = 1;
	result->matrix = result_coeff;
	//��Ӿ���
	//ɾ���ڴ�
	user_nn_matrix_delete(src_vector);
	user_nn_matrix_delete(src_matrix_cov);
	user_nn_matrix_delete(src_matrix_mean);
	user_nn_matrix_delete(src_matrix_mean_ext);
	user_nn_matrix_delete(src_matrix_zero);
	user_nn_matrices_delete(src_matrix_qr);
	//ɾ���ڴ�
	return result;
}

/*
//srand((unsigned)time(NULL));
	float content[] = {
	0.1,20,3.7,
	3.2,8,6.3,
	0.001,9,1.9 };//��������
	user_nn_list_matrix *result = NULL;
	user_nn_matrix *matrix = user_nn_matrix_create(10, 10);
	user_nn_matrix_rand_vaule(matrix, 1);
	//user_nn_matrix_memcpy(matrix, content);

	result = user_pca_process(matrix, 1.0f, qr_givens);//�Զ�����
	user_nn_matrices_printf(NULL, "givens", result);
	user_nn_matrices_delete(result);
	result = user_pca_process(matrix, 1.0f, qr_householder);//�޷��Զ�����
	user_nn_matrices_printf(NULL, "householder", result);
	getchar();
	return 0;
*/