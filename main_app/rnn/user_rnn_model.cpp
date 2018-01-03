
#include "user_rnn_model.h"
#include "user_rnn_ffp.h"
#include "user_rnn_bptt.h"
#include "user_rnn_grads.h"

//ͨ����������Ϣ����ģ��
//layer_infor ����ģ�Ͳ���
//���� ������������ģ�Ͷ���
user_rnn_layers *user_rnn_model_create(int *layer_infor) {
	user_rnn_layers			*rnn_layers = NULL;
	rnn_layers = user_rnn_layers_create(u_rnn_layer_type_null, 0);//����һ���ղ�

	while (1) {
		switch (*layer_infor) {
		case 'i':
			user_rnn_layers_input_create(rnn_layers, *(layer_infor + 1), *(layer_infor + 2), *(layer_infor + 3));	//�����
			layer_infor += 4;
			break;
		case 'h':
			user_rnn_layers_hidden_create(rnn_layers, *(layer_infor + 1), *(layer_infor + 2));//������
			layer_infor += 3;
			break;
		case 'o':
			user_rnn_layers_output_create(rnn_layers, *(layer_infor + 1), *(layer_infor + 2));//�����
			layer_infor += 3;
			return rnn_layers;
		default:
			printf("set error\n"); while (1);
			break;
		}
	}
	return NULL;
}
//�����������ݵ�ָ������������������
//layers ���ض����
//src_matrix Ŀ������
//���� ��
void user_rnn_model_load_input_feature(user_rnn_layers *layers, user_nn_list_matrix *src_matrix) {
	user_rnn_layers *rnn_input_layer = user_rnn_layers_get(layers, 1);//��ȡ�����
	user_nn_matrices_cpy_matrices(((user_rnn_input_layers *)rnn_input_layer->content)->feature_matrices, src_matrix);
}
//�����������ݵ�ָ������������������
//layers ���ض����
//src_matrix Ŀ������
//���� ��
void user_rnn_model_load_target_feature(user_rnn_layers *layers, user_nn_list_matrix *src_matrix) {
	user_rnn_layers *rnn_output_layer = user_rnn_model_return_layer(layers, u_rnn_layer_type_output);//��ȡ�����
	user_nn_matrices_cpy_matrices(((user_rnn_output_layers *)rnn_output_layer->content)->target_matrices, src_matrix);
}
//����ִ��һ�ε���
//layers �������Ĳ�
//����ֵ ��
void user_rnn_model_ffp(user_rnn_layers *layers) {
	while (1) {
		switch (layers->type) {
		case u_rnn_layer_type_null:
			break;
		case u_rnn_layer_type_input:
			break;
		case u_rnn_layer_type_hidden:
			user_rnn_ffp_hidden(layers->prior, layers);//�Ӳ�������
			break;
		case u_rnn_layer_type_output:
			user_rnn_ffp_output(layers->prior, layers);//��������
			break;
		default:
			break;
		}
		if (layers->next == NULL) {
			break;
		}
		else {
			layers = layers->next;
		}
	}
}

//���򴫲�һ��
//layers������ʼλ��
//index����ǰ��ǩλ��
//alpha������ϵ��
//����ֵ����
void user_rnn_model_bp(user_rnn_layers *layers, float alpha) {
	//ȡ��ָ�����һ������ָ��
	while (layers->next != NULL) {
		layers = layers->next;
	}
	//�������в�
	while (1) {
		switch (layers->type) {
		case u_rnn_layer_type_null:
			break;
		case u_rnn_layer_type_input:
			break;
		case u_rnn_layer_type_hidden:
			user_rnn_bp_hidden_back_prior(layers->prior, layers);
			break;
		case u_rnn_layer_type_output:
			user_rnn_bp_output_back_prior(layers->prior, layers);
			break;
		default:
			break;
		}
		if (layers->prior == NULL) {
			break;
		}
		else {
			layers = layers->prior;
		}
	}
	//���Ȩ�زв�ֵ
	while (1) {
		switch (layers->type) {
		case u_rnn_layer_type_null:
			break;
		case u_rnn_layer_type_input:
			break;
		case u_rnn_layer_type_hidden:
			user_rnn_grads_hidden(layers, alpha);//����Ȩ��
			break;
		case u_rnn_layer_type_output:
			user_rnn_grads_output(layers, alpha);//����Ȩ��
			break;
		default:
			break;
		}
		if (layers->next == NULL) {
			break;
		}
		else {
			layers = layers->next;
		}
	}
}
//��ȡloss��ʧֵ
//layers ��ȡ�����
//���� ��ʧֵ�Ĵ�С
float user_rnn_model_return_loss(user_rnn_layers *layers) {
	static float loss_function = 0;//ȫ�ֱ�����lossֵ
	while (1) {
		if (layers->type == u_rnn_layer_type_output) {
			if (loss_function == 0) {
				loss_function = ((user_rnn_output_layers *)layers->content)->loss_function;
			}
			else {
				loss_function = (float)0.99f * loss_function + 0.01f * ((user_rnn_output_layers *)layers->content)->loss_function;
			}
		}
		if (layers->next == NULL) {
			break;
		}
		else {
			layers = layers->next;
		}
	}
	return loss_function;
}

//�����������л�ȡһ��ָ���� ��˳�����
//layers ���ҵĶ����
//type Ŀ�������
//���� ��������
user_rnn_layers *user_rnn_model_return_layer(user_rnn_layers *layers, user_rnn_layer_type type) {
	while (1) {
		if (layers->type == type) {
			return layers;//�������ֵ
		}
		if (layers->next == NULL) {
			break;
		}
		else {
			layers = layers->next;
		}
	}
	return NULL;
}
//��ȡ�������
//layers ��ȡ�����
//���� ����ֵ
user_nn_list_matrix *user_rnn_model_return_result(user_rnn_layers *layers) {
	user_nn_list_matrix *result = NULL;//
	while (1) {
		if (layers->type == u_rnn_layer_type_output) {
			result = ((user_rnn_output_layers *)layers->content)->feature_matrices;
		}
		if (layers->next == NULL) {
			break;
		}
		else {
			layers = layers->next;
		}
	}
	return result;
}