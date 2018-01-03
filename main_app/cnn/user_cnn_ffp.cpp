
#include "user_cnn_ffp.h"

//Feedforward Passǰ�򴫲�
//����Ϊ�����
//����
//pre_layer��ǰһ�� pool ���� input��
//dest_layer������
void user_cnn_ffp_convolution(user_cnn_layers *prior_layer, user_cnn_layers *conv_layer){
	user_nn_list_matrix   *input_feature_matrices = NULL;//����������������
	user_nn_matrix        *input_feature_matrix = NULL;//������������  ����
	user_cnn_conv_layers  *conv_layers = (user_cnn_conv_layers  *)conv_layer->content;//��ȡ����������
	user_nn_matrix        *conv_kernel_matrix = NULL;//����˾���
	float				  *conv_bias_value = ((user_cnn_conv_layers  *)conv_layer->content)->biases_matrix->data;//ƫ�ò���
	user_nn_list_matrix   *output_feature_matrices = ((user_cnn_conv_layers  *)conv_layer->content)->feature_matrices;//���������������
	user_nn_matrix        *output_feature_matrix = NULL;//�����������  ����
	user_nn_matrix        *_conv_matrix = NULL;//����������
	user_nn_matrix        *_result_matrix = NULL;//�������

	int input_feture_index, output_feture_index;//

	//��ȡǰһ�������
	if (prior_layer->type == u_cnn_layer_type_input){
		input_feature_matrices = ((user_cnn_input_layers *)prior_layer->content)->feature_matrices;//ת����������
	}
	else if (prior_layer->type == u_cnn_layer_type_pool){
		input_feature_matrices = ((user_cnn_pool_layers *)prior_layer->content)->feature_matrices;//ת����������
	}
	else if (prior_layer->type == u_cnn_layer_type_conv){
		input_feature_matrices = ((user_cnn_conv_layers *)prior_layer->content)->feature_matrices;//ת����������
	}
	else{
		return ;
	}
	_result_matrix = user_nn_matrix_create(conv_layers->feature_width, conv_layers->feature_height);//����һ�����������һ���������

	for (output_feture_index = 0; output_feture_index < conv_layers->feature_number; output_feture_index++){
		user_nn_matrix_memset(_result_matrix, 0);//��վ���ֵ
		for (input_feture_index = 0; input_feture_index < conv_layers->input_feature_number; input_feture_index++){
			input_feature_matrix = user_nn_matrices_ext_matrix_index(input_feature_matrices, input_feture_index);//��ȡ��������
			conv_kernel_matrix = user_nn_matrices_ext_matrix(conv_layers->kernel_matrices, input_feture_index, output_feture_index);//ȡ��������Ӧ�ľ���� 
			_conv_matrix = user_nn_matrix_conv2(input_feature_matrix, conv_kernel_matrix, u_nn_conv2_type_valid);//���ݽ��о������
			user_nn_matrix_cum_matrix(_result_matrix, _result_matrix, _conv_matrix);//����һ������о����������ۼ�
			user_nn_matrix_delete(_conv_matrix);//ɾ���������
		}
		output_feature_matrix = user_nn_matrices_ext_matrix_index(output_feature_matrices, output_feture_index);
		user_nn_activate_matrix_sum_constant(output_feature_matrix, _result_matrix, *conv_bias_value++, user_nn_cnn_softmax);//����ƫ�ò������к�������
	}
	user_nn_matrix_delete(_result_matrix);//ɾ������
}
//�ػ���������
//
void user_cnn_ffp_pooling(user_cnn_layers *prior_layer, user_cnn_layers *pool_layer){
	user_cnn_pool_layers  *pool_layers				= (user_cnn_pool_layers  *)pool_layer->content;//��ȡ����ػ�������
	user_nn_matrix        *pool_kernel_matrix		= ((user_cnn_pool_layers  *)pool_layer->content)->kernel_matrix;//����˾���
	user_nn_list_matrix   *output_feature_matrices	= ((user_cnn_pool_layers  *)pool_layer->content)->feature_matrices;//���������������
	user_nn_matrix        *output_feature_matrix		= NULL;//�����������  ����
	user_nn_list_matrix   *input_feature_matrices	= NULL;//���������������
	user_nn_matrix        *input_feature_matrix		= NULL;//������������  ����

	int input_feature_index = 0;

	//��ȡǰһ�������
	if (prior_layer->type == u_cnn_layer_type_input){
		input_feature_matrices = ((user_cnn_input_layers *)prior_layer->content)->feature_matrices;//ת����������
	}
	else if (prior_layer->type == u_cnn_layer_type_pool){
		input_feature_matrices = ((user_cnn_pool_layers *)prior_layer->content)->feature_matrices;//ת����������
	}
	else if (prior_layer->type == u_cnn_layer_type_conv){
		input_feature_matrices = ((user_cnn_conv_layers *)prior_layer->content)->feature_matrices;//ת����������
	}
	else{
		return;
	}

	for (input_feature_index = 0; input_feature_index < pool_layers->input_feature_number; input_feature_index++){
		input_feature_matrix = user_nn_matrices_ext_matrix_index(input_feature_matrices, input_feature_index);//ȡ��ָ��λ����������
		output_feature_matrix = user_nn_matrices_ext_matrix_index(output_feature_matrices, input_feature_index);//ȡ��ָ��λ����������ָ��
		user_nn_matrix_pooling(output_feature_matrix, input_feature_matrix, pool_kernel_matrix);//���������ݽ��гػ�����
	}
}
//����ȫ���Ӳ�
void user_cnn_ffp_fullconnect(user_cnn_layers *prior_layer, user_cnn_layers *full_layer){
	user_nn_matrix			*full_input_feture_matrix = ((user_cnn_full_layers *)full_layer->content)->input_feature_matrix;
	user_nn_matrix			*full_feature_matrix = ((user_cnn_full_layers *)full_layer->content)->feature_matrix;//��������������
	user_nn_matrix			*full_kernel_matrix = ((user_cnn_full_layers *)full_layer->content)->kernel_matrix;//�������������
	user_nn_matrix		    *full_bias_matrix = ((user_cnn_full_layers *)full_layer->content)->biases_matrix;//ƫ�ò��� ����
	user_nn_list_matrix		*input_feature_matrices		= NULL;//����������������
	user_nn_matrix			*_output_feature_matrix		= NULL;//�����������  ����

	//��ȡǰһ�������
	if (prior_layer->type == u_cnn_layer_type_input){
		user_nn_matrices_to_matrix(full_input_feture_matrix, ((user_cnn_input_layers *)prior_layer->content)->feature_matrices);//����һ������뵽�������������ת����һ������
	}
	else if (prior_layer->type == u_cnn_layer_type_pool){
		user_nn_matrices_to_matrix(full_input_feture_matrix, ((user_cnn_pool_layers *)prior_layer->content)->feature_matrices);//����һ������뵽�������������ת����һ������
	}
	else if (prior_layer->type == u_cnn_layer_type_conv){
		user_nn_matrices_to_matrix(full_input_feture_matrix, ((user_cnn_conv_layers *)prior_layer->content)->feature_matrices);//����һ������뵽�������������ת����һ������
	}
	else{
		return;
	}
	//y=simoid(w*x+b)
	_output_feature_matrix = user_nn_matrix_mult_matrix(full_kernel_matrix, full_input_feture_matrix);//����˷���������ת��
	user_nn_activate_matrix_sum_matrix(full_feature_matrix, _output_feature_matrix, full_bias_matrix, user_nn_cnn_softmax);//����������ݽ�����ƫ�ò�������ڽ���sigmoid����

	user_nn_matrix_delete(_output_feature_matrix);//ɾ������
}
//�������������
void user_cnn_ffp_output(user_cnn_layers *prior_layer, user_cnn_layers *output_layer){
	user_nn_matrix			*output_input_feture_matrix = ((user_cnn_output_layers *)output_layer->content)->input_feature_matrix;
	user_nn_matrix			*output_feature_matrix = ((user_cnn_output_layers *)output_layer->content)->feature_matrix;//��������������
	user_nn_matrix			*output_kernel_matrix = ((user_cnn_output_layers *)output_layer->content)->kernel_matrix;//�������������
	user_nn_matrix		    *output_bias_matrix = ((user_cnn_output_layers *)output_layer->content)->biases_matrix;//ƫ�ò��� ����
	user_nn_list_matrix		*input_feature_matrices = NULL;//����������������
	user_nn_matrix			*_output_feature_matrix = NULL;//�����������  ����

	//��ȡǰһ�������
	if (prior_layer->type == u_cnn_layer_type_input){
		user_nn_matrices_to_matrix(output_input_feture_matrix, ((user_cnn_input_layers *)prior_layer->content)->feature_matrices);//����һ������뵽�������������ת����һ������
	}
	else if (prior_layer->type == u_cnn_layer_type_pool){
		user_nn_matrices_to_matrix(output_input_feture_matrix, ((user_cnn_pool_layers *)prior_layer->content)->feature_matrices);//����һ������뵽�������������ת����һ������
	}
	else if (prior_layer->type == u_cnn_layer_type_conv){
		user_nn_matrices_to_matrix(output_input_feture_matrix, ((user_cnn_conv_layers *)prior_layer->content)->feature_matrices);//����һ������뵽�������������ת����һ������
	}
	else if (prior_layer->type == u_cnn_layer_type_full){
		user_nn_matrix_cpy_matrix(output_input_feture_matrix, ((user_cnn_full_layers *)prior_layer->content)->feature_matrix);//ֱ�ӿ����������ݼ���
	}
	else{
		return;
	}
	_output_feature_matrix = user_nn_matrix_mult_matrix(output_kernel_matrix, output_input_feture_matrix);//����˷���������ת��
	user_nn_activate_matrix_sum_matrix(output_feature_matrix, _output_feature_matrix, output_bias_matrix, user_nn_cnn_softmax);//����������ݽ�����ƫ�ò�������ڽ���sigmoid����
	
	user_nn_matrix_delete(_output_feature_matrix);//ɾ������
}


