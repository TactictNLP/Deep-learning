#ifndef _user_words_vector_pro_H
#define _user_words_vector_pro_H

#include <string.h>  
#include <math.h>  
#include <malloc.h>  
#include <stdio.h>  
#include "../matrix/user_nn_matrix.h"

float user_w2v_cos_dist(float *a, float *b, int n);//����cos�н�
float user_w2v_eu_dist(float *a, float *b, int n);//����ŷʽ����

//����һ��list�ṹ�����ڱ��浥�ʻ���ӵ�ά����Ϣ
typedef struct words_vector {
	struct words_vector *prior;//��һ��ceng
	int index;//ָ��
	char *words_string;//�����ַ���
	int class_id;//�������
	int  vector_number;//��������
	float *vector_data;//��������
	struct words_vector *next;//��һ��
}user_w2v_words_vector;

user_w2v_words_vector *user_w2v_words_vector_create(void);//����һ���������Ľṹ��
void user_w2v_words_vector_delete(user_w2v_words_vector *dest);//ɾ��һ���������Ľṹ��
void user_w2v_words_vector_all_delete(user_w2v_words_vector *dest);//ɾ�����������Ĵ������ṹ��
bool user_w2v_words_vector_add(user_w2v_words_vector *dest, char *words, int classid, int size, float *vector);//���������


enum distance_type {
	cosine = 0,//cos����
	euclidean = 1//ŷʽ����
};
typedef struct similar_list {
	struct similar_list *prior;//��һ��
	int index;//ָ��
	struct words_vector *result_addr;//����ĵ��ʶ����ַ
	float  result_similar;//�������
	struct similar_list *next;//��һ��
}user_w2v_similar_list;
bool user_w2v_read_words_string_txt_utf8(FILE *f, char *string);//��һ����ȡһ���ַ��� �ո����
bool user_w2v_read_words_vector_txt_utf8(FILE *f, int count, float *mem);//��ȡһ������� ���з�Ϊ����
user_w2v_words_vector *load_words_vector_model(char *path);//����һ��ģ���ļ� ��˫����������
user_w2v_similar_list *user_w2v_similar_list_create(void);//����һ��������������
void user_w2v_similar_list_delete(user_w2v_similar_list *dest);//ɾ��һ��������������
void user_w2v_similar_list_all_delete(user_w2v_similar_list *dest);//ɾ�����б�����������
float user_w2v_words_similar(user_w2v_words_vector *model, distance_type type, char *stra, char *strb);//������������֮��ľ���
user_w2v_similar_list *user_w2v_words_most_similar(user_w2v_words_vector *model, distance_type type, char *string, float thrd);//�ҳ�ָ�����������һϵ�д��ﲢ�ҷ���
user_w2v_similar_list *user_w2v_similar_sorting(user_w2v_similar_list *dest, sorting_type type);//�����б� ��������

void user_w2v_similar_list_printf(bool to_file, user_w2v_similar_list *similar_list);//��ӡ����

#endif