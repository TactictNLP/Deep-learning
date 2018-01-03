#ifndef _user_k_means_H
#define _user_k_means_H

#include "../user_config.h"
#include "../matrix/user_nn_matrix.h"
#include "../w2c/user_words_vector_pro.h"

user_w2v_words_vector *user_k_means_create_n_class(user_w2v_words_vector *target, int classnum);//��������
bool user_k_means_mark_data(user_w2v_words_vector *target, user_w2v_words_vector *nclass, distance_type type);//�������
bool user_k_means_compute_class(user_w2v_words_vector *target, user_w2v_words_vector *nclass);//�����µķ�������

void user_k_means_class_fprintf(char *path, user_w2v_words_vector *target, int classnum);//���±������������
#endif