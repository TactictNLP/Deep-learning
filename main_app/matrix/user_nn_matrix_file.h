#ifndef _user_nn_matrix_file_H
#define _user_nn_matrix_file_H

#include "user_nn_matrix.h"
long user_nn_model_save_matrix(FILE *file, long offset, user_nn_matrix *matrix);//д������ļ�������
long user_nn_model_save_matrices(FILE *file, long offset, user_nn_list_matrix *matrices);//д�����������ļ�������
long user_nn_model_read_matrix(FILE *file, long offset, user_nn_matrix *matrix);//��ȡһ��������ļ�������
long user_nn_model_read_matrices(FILE *file, long offset, user_nn_list_matrix *matrices);//��ȡ����������ļ�������

bool user_nn_model_file_save_matrices(const char *path, long offset, user_nn_list_matrix *matrices);//�������������ļ���
user_nn_list_matrix *user_nn_model_file_read_matrices(const char *path, long offset);//���ļ��ж�ȡ��������

#endif