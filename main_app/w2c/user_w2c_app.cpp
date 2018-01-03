
#include "user_w2c_app.h"


void user_w2c_app_test(int argc, const char** argv) {
	user_w2v_words_vector *model = load_words_vector_model("word2vec_model.bin");//����ģ��
	user_w2v_similar_list *result_list = user_w2v_words_most_similar(model, euclidean, "С��Ů", 5.0f);//���������ֵ���ַ�����������ȡ����
	printf("\n");
	user_w2v_similar_list_printf(false, result_list);//��ӡ����
	result_list = user_w2v_similar_sorting(result_list, sorting_down);//�������н���ɾ�����
	float dist = user_w2v_words_similar(model, cosine, "��", "��");//���������ַ���֮��ľ���
	printf("\n dist=%f\n", dist);
	user_w2v_similar_list_printf(false, result_list);//��ӡ����

	user_w2v_words_vector_all_delete(model);//ɾ��ģ��
	user_w2v_similar_list_all_delete(result_list);//ɾ���������

	getchar();
}