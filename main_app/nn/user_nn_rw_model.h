#ifndef _user_nn_rw_model_H
#define _user_nn_rw_model_H

#include "../user_config.h"
#include "../matrix/user_nn_matrix_file.h"
#include "../nn/user_nn_layers.h"

#define		user_nn_model_nn_file_name		"nn_model.bin"
#define		user_nn_model_nn_layer_addr		 0x0		//�����Ļ���ַ
#define		user_nn_model_nn_content_addr	 0x800		//��������Ļ���ַ
#define		user_nn_model_nn_data_addr		 0x1000		//�������ݵĻ���ַ

bool user_nn_model_save_model(const char *path,user_nn_layers *layers);//����ģ��
user_nn_layers	*user_nn_model_load_model(const char *path);//�����ģ��

#endif