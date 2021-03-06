#ifndef _user_config_H
#define _user_config_H

#include <string.h>
//#include <stdbool.h>
//#include <cv.h>
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <windows.h>   
#include <direct.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <basetsd.h>
#include <float.h>

//#include <cuda_runtime.h>
//#include <cublas_v2.h>
//#include <device_launch_parameters.h>

//#include "CvxText.h"
//#include "user_nn_matrix.h"
//#include "user_cnn_create.h"
#include "user_types.h"

//CNN配置开始
#define		user_nn_cnn_softmax				activation_tanh

#define		user_nn_cnn_training_folder		"digital"
#define		user_nn_cnn_training_type		".jpg"
#define		user_nn_model_cnn_file_name		"cnn_model.bin"
#define		user_nn_model_cnn_layer_addr	 0x0		//保存层的基地址
#define		user_nn_model_cnn_content_addr	 0x800		//保存层对象的基地址
#define		user_nn_model_cnn_data_addr		 0x1000		//保存数据的基地址
//CNN配置结束
//RNN配置开始
#define		user_nn_rnn_softmax				activation_tanh

#define		user_nn_model_rnn_file_name		"rnn_model.bin"
#define		user_nn_model_rnn_layer_addr	 0x0		//保存层的基地址
#define		user_nn_model_rnn_content_addr	 0x800		//保存层对象的基地址
#define		user_nn_model_rnn_data_addr		 0x1000		//保存数据的基地址
//RNN配置结束
//NN配置开始
#define		user_nn_nn_softmax				activation_tanh

#define		user_nn_model_nn_file_name		"nn_model.bin"
#define		user_nn_model_nn_layer_addr		 0x0		//保存层的基地址
#define		user_nn_model_nn_content_addr	 0x800		//保存层对象的基地址
#define		user_nn_model_nn_data_addr		 0x1000		//保存数据的基地址
//NN配置结束
#endif