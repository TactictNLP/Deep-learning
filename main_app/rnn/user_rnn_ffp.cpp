
#include "user_rnn_ffp.h"

//intput->hidden->output
//hidden=tanh(input*w1+hidden_t*w2+bias1)
//
void user_rnn_ffp_hidden(user_rnn_layers *prior_layer, user_rnn_layers *hidden_layer) {
	user_nn_list_matrix   *input_feature_matrices = NULL;//����������������
	user_rnn_hidden_layers *hidden_layers = (user_rnn_hidden_layers  *)hidden_layer->content;//��ȡ����Ĳ���
	user_nn_matrix *hidden_kernel_matrix = NULL;
	user_nn_matrix *intput_feature_matrix = NULL;
	user_nn_matrix *hidden_kernel_matrix_t = NULL;
	user_nn_matrix *hidden_feature_matrix_t = NULL;
	user_nn_matrix *hidden_bias_matrix = NULL;
	user_nn_matrix *hidden_feature_matrix = NULL;

	user_nn_matrix *intput_to_hidden_feature = NULL;
	user_nn_matrix *hidden_to_hidden_feature = NULL;
	int time_index = 0;

	//��ȡǰһ�������
	if (prior_layer->type == u_rnn_layer_type_input) {
		input_feature_matrices = ((user_rnn_input_layers *)prior_layer->content)->feature_matrices;//ת����������
	}
	else if (prior_layer->type == u_rnn_layer_type_hidden) {
		input_feature_matrices = ((user_rnn_hidden_layers *)prior_layer->content)->feature_matrices;//ת����������
	}
	else {
		return;
	}
	
	for (time_index = 0; time_index < hidden_layers->time_number; time_index++) {
		hidden_feature_matrix = user_nn_matrices_ext_matrix_index(hidden_layers->feature_matrices, time_index);//��������	
		hidden_kernel_matrix = hidden_layers->kernel_matrix;//��ȡǰһ����赼�������ݵ�Ȩ��
		intput_feature_matrix = user_nn_matrices_ext_matrix_index(input_feature_matrices, time_index);//��ȡǰһ�����뵽�������������
		hidden_kernel_matrix_t = hidden_layers->kernel_matrix_t;//���ز㵽���ز��Ȩ��
		hidden_feature_matrix_t = hidden_layers->feature_matrix_t;//�ϸ�ʱ��Ƭ���ز������
		hidden_bias_matrix = hidden_layers->biases_matrix;//��ȡƫ�ò�������
		//Hi=act_function(np.dot(Wi,i)+np.dot(Wh,Ht_1)+bh)
		
		intput_to_hidden_feature = user_nn_matrix_mult_matrix(hidden_kernel_matrix, intput_feature_matrix);//np.dot(Wi,i)
		hidden_to_hidden_feature = user_nn_matrix_mult_matrix(hidden_kernel_matrix_t, hidden_feature_matrix_t);//np.dot(Wh,Ht_1)
		user_nn_matrix_cum_matrix(hidden_feature_matrix, intput_to_hidden_feature, hidden_to_hidden_feature);//Ȩ�����
		user_nn_matrix_cum_matrix(hidden_feature_matrix, hidden_feature_matrix, hidden_bias_matrix);//+bh����ƫ�ò���
		user_nn_activate_matrix(hidden_feature_matrix, user_nn_rnn_softmax);//���ü�������м���
		//Ht_1=Hi
		user_nn_matrix_cpy_matrix(hidden_feature_matrix_t, hidden_feature_matrix);//�������һ��ʱ��Ƭ�����ز�����ֵ

		user_nn_matrix_delete(intput_to_hidden_feature);//ɾ������
		user_nn_matrix_delete(hidden_to_hidden_feature);//ɾ������
	}
}
//intput->hidden->output
//output=tanh(hidden*w2+bias2)
//
void user_rnn_ffp_output(user_rnn_layers *prior_layer, user_rnn_layers *output_layer) {
	user_nn_list_matrix   *input_feature_matrices = NULL;//����������������
	user_rnn_output_layers *output_layers = (user_rnn_output_layers  *)output_layer->content;//��ȡ����Ĳ���
	user_nn_matrix *output_kernel_matrix = NULL;
	user_nn_matrix *input_feature_matrix = NULL;
	user_nn_matrix *output_bias_matrix = NULL;
	user_nn_matrix *output_feature_matrix = NULL;

	user_nn_matrix *intput_to_output_feature = NULL;
	int time_index = 0;

	//��ȡǰһ�������
	if (prior_layer->type == u_rnn_layer_type_input) {
		input_feature_matrices = ((user_rnn_input_layers *)prior_layer->content)->feature_matrices;//ת����������
	}
	else if (prior_layer->type == u_rnn_layer_type_hidden) {
		input_feature_matrices = ((user_rnn_hidden_layers *)prior_layer->content)->feature_matrices;//ת����������
	}
	else {
		return;
	}

	for (time_index = 0; time_index < output_layers->time_number; time_index++) {
		output_kernel_matrix = output_layers->kernel_matrix;//ȡ��Ȩ��
		input_feature_matrix = user_nn_matrices_ext_matrix_index(input_feature_matrices, time_index);//��ȡ�ϲ��������������
		output_bias_matrix = output_layers->biases_matrix;//ȡ��ƫ�ò���
		output_feature_matrix = user_nn_matrices_ext_matrix_index(output_layers->feature_matrices, time_index);//��ȡ�������
		//Oh=act_function(np.dot(Wo,Hi)+bo)
		intput_to_output_feature = user_nn_matrix_mult_matrix(output_kernel_matrix,input_feature_matrix);//np.dot(Wo,Hi)		
		user_nn_matrix_cum_matrix(output_feature_matrix, intput_to_output_feature, output_bias_matrix);//+bo����ƫ�ò���
		user_nn_activate_matrix(output_feature_matrix, user_nn_rnn_softmax);//���ݽ��м���,���汾��������������

		user_nn_matrix_delete(intput_to_output_feature);//ɾ����ʱ����	
	}
}