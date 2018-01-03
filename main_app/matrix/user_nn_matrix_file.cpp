
#include "user_nn_matrix_file.h"

//��ָ��λ�ñ���һ������
//file �ļ�����
//offset ƫ�Ƶ�ַ
//matrix ����Ķ���
//���� �ļ�ָ��λ��
long user_nn_model_save_matrix(FILE *file, long offset, user_nn_matrix *matrix) {
	int count = matrix->width * matrix->height;
	fseek(file, offset, SEEK_SET);
	fwrite(matrix->data, count * sizeof(float), 1, file);//д���
	return ftell(file);
}
//��ָ��λ�ñ�����������
//file �ļ�����
//offset ƫ�Ƶ�ַ
//matrices ����Ķ���
//���� �ļ�ָ��λ��
long user_nn_model_save_matrices(FILE *file, long offset, user_nn_list_matrix *matrices) {
	int count = matrices->width * matrices->height;
	user_nn_matrix *matrix = matrices->matrix;
	while (count--) {
		//printf("%d\n", offset);
		offset = user_nn_model_save_matrix(file, offset, matrix);
		//offset = offset + matrix->width * matrix->height * sizeof(float);//�ƶ�λ��
		matrix = matrix->next;//ָ����һ�������λ��
	}
	return ftell(file);
}
//��ָ��λ�ö�ȡһ������
//file �ļ�����
//offset ƫ�Ƶ�ַ
//matrix ����Ķ���
//���� �ļ�ָ��λ��
long user_nn_model_read_matrix(FILE *file, long offset, user_nn_matrix *matrix) {
	int count = matrix->width * matrix->height;
	fseek(file, offset, SEEK_SET);
	fread(matrix->data, count * sizeof(float), 1, file);//д���
	return ftell(file);
}
//��ָ��λ�ö�ȡһ����������
//file �ļ�����
//offset ƫ�Ƶ�ַ
//matrices ����Ķ���
//���� �ļ�ָ��λ��
long user_nn_model_read_matrices(FILE *file, long offset, user_nn_list_matrix *matrices) {
	int count = matrices->width * matrices->height;
	user_nn_matrix *matrix = matrices->matrix;
	while (count--) {
		offset = user_nn_model_read_matrix(file, offset, matrix);
		//offset = offset + matrix->width * matrix->height * sizeof(float);//�ƶ�λ��
		matrix = matrix->next;//ָ����һ�������λ��
	}
	return ftell(file);
}

//�������󱣴����ļ�
//path �ļ�·��
//offset ƫ�Ƶ�ַ
//matrices ��¼�Ķ���
//���� �ɹ���ʧ��
bool user_nn_model_file_save_matrices(const char *path, long offset, user_nn_list_matrix *matrices) {
	FILE *model_file = NULL;
	user_nn_matrix *matrix = NULL;
	int matrices_count = 0;
	int matrix_count = 0;
	fopen_s(&model_file, path, "wb+");//���ö�д��ģ���ļ�
	if (model_file == NULL)return NULL;

	fseek(model_file, offset, SEEK_SET);//�ƶ���ƫ�Ƶ�ַ
	fprintf(model_file,"M");//д���
	fwrite(&matrices->width, sizeof(int), 1, model_file);//д�����ĸ߶�
	fwrite(&matrices->height, sizeof(int), 1, model_file);//д�����ĸ߶�
	//user_nn_model_save_matrices(model_file, offset, matrices);//д�����
	matrices_count = matrices->height * matrices->width;//���������������Ŀ
	matrix = matrices->matrix;//��ȡ����
	while (matrices_count--) {
		//fprintf(model_file, "%d %d ", matrix->height, matrix->width);//д���
		matrix_count = matrix->width * matrix->height;//��ȡ��С
		fprintf(model_file, "m");//д���־
		fwrite(&matrix->width, sizeof(int), 1, model_file);//д�����ĸ߶�
		fwrite(&matrix->height, sizeof(int), 1, model_file);//д�����ĸ߶�	
		fwrite(matrix->data, matrix_count * sizeof(float), 1, model_file);//д���������
		matrix = matrix->next;//ָ����һ������
	}
	fclose(model_file);
	return true;
}
//���ļ�������������
//path �ļ�·��
//offset ƫ�Ƶ�ַ
//���� ��¼����������
user_nn_list_matrix *user_nn_model_file_read_matrices(const char *path, long offset) {
	user_nn_list_matrix *matrices=NULL;
	user_nn_matrix *matrix = NULL;
	user_nn_matrix *matrices_matrix = NULL;
	FILE *model_file = NULL;
	char flag = 0;
	int matrices_count=0;
	int matrices_width = 0;
	int matrices_height = 0;

	int matrix_count = 0;
	int matrix_width = 0;
	int matrix_height = 0;

	fopen_s(&model_file, path, "rb");//���ö�д��ģ���ļ�
	if (model_file == NULL)return NULL;

	fseek(model_file, offset, SEEK_SET);//�ƶ���ƫ�Ƶ�ַ
	fread(&flag, sizeof(char), 1, model_file);//��ȡ��־λ
	if (flag != 'M') {
		fclose(model_file);
		return NULL;
	}
	fread(&matrices_width, sizeof(int), 1, model_file);//д�����ĸ߶�
	fread(&matrices_height, sizeof(int), 1, model_file);//д�����ĸ߶�
	matrices = (user_nn_list_matrix *)malloc(sizeof(user_nn_list_matrix));//����ռ�
	matrices->width = matrices_width;//�����ܾ�����
	matrices->height = matrices_height;//�����ܾ���߶�

	while (TRUE) {
		fread(&flag, sizeof(char), 1, model_file);//��ȡ��־λ
		if (feof(model_file) || (flag != 'm')) {//�ļ���ȡ�����˳����߶�ȡ�����˳�
			break;
		}
		fread(&matrix_width, sizeof(int), 1, model_file);//д�����ĸ߶�
		fread(&matrix_height, sizeof(int), 1, model_file);//д�����ĸ߶�	

		matrix = user_nn_matrix_create(matrix_width, matrix_height);//��������
		fread(matrix->data, matrix_width * matrix_height * sizeof(float), 1, model_file);//��ȡ��������
		
		if (matrices_matrix == NULL) {
			matrices_matrix = matrix;
			matrices->matrix = matrices_matrix;//��¼���ݵ�����������
		}
		else {
			matrices_matrix->next = matrix;
			matrices_matrix = matrices_matrix->next;
		}
	}
	fclose(model_file);
	return matrices;
}

/*
user_nn_list_matrix *input_data = user_nn_matrices_create(1,2,2,2);//������������
user_nn_matrices_rand_vaule(input_data, 1.0f);//��ʼ������
user_nn_matrices_printf(NULL, "input", input_data);//��ӡ
user_nn_model_file_save_matrices("test.bin",0, input_data);//����
input_data  = user_nn_model_file_read_matrices("test.bin", 0);//��ȡ
user_nn_matrices_printf(NULL,"output", input_data);//��ӡ
getchar();
return 0;
*/
