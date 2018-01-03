

#include "user_k_means.h"


//1.����N��������
//2.��N����������ʼ��ֵ
//3.���������ݽ���1-N���з�����㣬���ұ��
//4.���ѷ�������ݽ������Ĳ���
//5.���·���������
//6.������������Ƿ����·��䣬��û���˳���������תֵ��3��

//����������
//target �������
//num �������
//���� �ջ�����
//ע�⣺���ص�class�ṹ�������class_id �Ǳ���ķ�������Ԫ�ظ��� �����ݽṹ�屣�����ID��һ��
user_w2v_words_vector *user_k_means_create_n_class(user_w2v_words_vector *target,int classnum) {
	user_w2v_words_vector *result = NULL;
	int classid = 0;
	if (classnum <= 0) {
		return NULL;
	}
	result = user_w2v_words_vector_create();//����������
	while(classnum--) {
		user_w2v_words_vector_add(result, "class", 0, target->vector_number, target->vector_data);//���ò��� ����Ԫ��Ϊ0
		target = target->next;//����Ŀ��
	}
	return result;
}

//ͨ�����������������ݽ��б�� ������ȡ�µķ�������
//target��������Ķ���
//nclass�������������
//type��������㷽ʽ
//���أ����е��Ƿ�ת����ɣ������ɷ���true
bool user_k_means_mark_data(user_w2v_words_vector *target, user_w2v_words_vector *nclass, distance_type type) {
	user_w2v_words_vector *target_object = target;
	user_w2v_words_vector *class_object = nclass;
	float distance_max = FLT_MAX;//��¼���ֵ
	int class_id = 0;//��¼���
	float distance_vaule = 0;//����ֵ
	bool result = true;//����ת�����

	//�������Ԫ�ظ��� �������·���
	class_object = nclass;//��ʼ��ֵΪ0
	while (class_object != NULL) {
		class_object->class_id = 0;//�������Ԫ��Ϊ0
		class_object = class_object->next;//
	}
	//��������Ԫ�����·���
	target_object = target;
	while (target_object != NULL) {
		//����Ԫ��������һ��
		class_object = nclass;//��ʼ����������
		distance_max = FLT_MAX;//�������ֵ
		while (class_object != NULL) {
			//�������
			if (type == cosine) {
				distance_vaule = user_w2v_cos_dist(target_object->vector_data, class_object->vector_data, class_object->vector_number);
			}
			else if (type == euclidean) {
				distance_vaule = user_w2v_eu_dist(target_object->vector_data, class_object->vector_data, class_object->vector_number);
			}
			//�ҳ���С�����ֵ
			if (distance_vaule < distance_max) {
				distance_max = distance_vaule;//��¼��Сֵ
				class_id = class_object->index;//��¼ID
			}
			class_object = class_object->next;//ָ����һ�����
		}

		//��¼�������Ԫ�ظ���
		class_object = nclass;//��ʼ����������
		while (class_object != NULL) {
			if (class_object->index == class_id) {
				class_object->class_id++;//�������ĵ�����¼���Ƿ���������Ԫ�ظ���
			}
			class_object = class_object->next;//ָ����һ�����
		}
		//�����ݽ��б��
		if (target_object->class_id != class_id) {
			target_object->class_id = class_id;//�������б��
			result = false;
		}	

		target_object = target_object->next;//������һ��
	}
	return result;
}
//����һ�η���������
//target��Ŀ�����
//nclass���������
//���� �ɹ���ʧ��
bool user_k_means_compute_class(user_w2v_words_vector *target, user_w2v_words_vector *nclass) {
	user_w2v_words_vector *target_object = target;
	user_w2v_words_vector *class_object = nclass;
	float *target_vaule = NULL;
	float *nclass_vaule = NULL;
	int count = 0;

	class_object = nclass;//��ʼ��ֵΪ0
	while (class_object != NULL) {
		memset(class_object->vector_data, 0, class_object->vector_number * sizeof(float));//����ڴ�
		class_object = class_object->next;//
	}
	//�����µ�����
	target_object = target;
	while (target_object != NULL) {
		target_vaule = target_object->vector_data;//ȡ�õ�ǰ���ݵ�ֵ
		class_object = nclass;//��ʼ�������
		while (class_object != NULL) {
			if (target_object->class_id == class_object->index) {//���������������� ��������ֵ��� ��ƽ����
				nclass_vaule = class_object->vector_data;//ȡ�õ�ǰ���ĵ�ֵ
				//ƽ��ֵ��Ϊ����
				for (count = 0; count < target_object->vector_number; count++) {
					*nclass_vaule++ += (float) (*target_vaule++ / class_object->class_id);//����ֵ
				}
			}
			class_object = class_object->next;//
		}
		target_object = target_object->next;//������һ��
	}
	return true;
}

//�������������
//
void user_k_means_class_fprintf(char *path,user_w2v_words_vector *target,int classnum) {
	user_w2v_words_vector *target_object = target;
	FILE *debug_file = NULL;
	int count = 0;
	debug_file = fopen(path, "w+");
	
	for (count = 1; count < classnum + 1; count++) {
		fprintf(debug_file, "class id :%d \n", count);
		target_object = target;
		while (target_object != NULL) {
			if (target_object->class_id == count) {
				fprintf(debug_file,"%s \n",target_object->words_string);
			}
			target_object = target_object->next;
		}
	}
	fclose(debug_file);
}


/*
	int class_number = 30;//�������
	user_w2v_words_vector *model = load_words_vector_model("word2vec_model.bin");//����ģ��
	user_w2v_words_vector *class_center = user_k_means_create_n_class(model, class_number);

	while (user_k_means_mark_data(model, class_center, euclidean) == false) {
	user_w2v_words_vector *nc = class_center;
	printf("\n ");
	while (nc!=NULL) {
	printf("%d ",nc->class_id);
	nc = nc->next;
	}
	user_k_means_compute_class(model, class_center);//���¼�������ֵ
	}
	user_k_means_class_fprintf("k-means.txt", model, class_number);
	printf("\n k-means class compute complete!!");
	user_w2v_words_vector_all_delete(model);//ɾ��ģ��
	user_w2v_words_vector_all_delete(class_center);//ɾ���������ĵ�
	getchar();
	return 0;
*/