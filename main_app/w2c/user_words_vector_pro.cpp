

#include "user_words_vector_pro.h"


//����COS�н� cosine angle
//��ʽ��cos(ai,bi)=����a*����b/|����a|*|����b|=(a1*b1+a2*b2+...+ai*bi)/(sqrt(a1*a1+a2*a2+...ai*ai)*sqrt(b1*b1+b2*b2+...bi*bi))
float user_w2v_cos_dist(float *a, float *b, int n) {
	float num = 0, dena = 0, denb = 0;

	while (n--) {
		num += *a * *b;//num = (a1*b1+a2*b2+...+ai*bi)
		dena += *a * *a;//dena = sqrt(a1*a1+a2*a2+...ai*ai)
		denb += *b * *b;//denb = sqrt(a1*a1+a2*a2+...ai*ai)
		a++; b++;
	}
	dena = (float)sqrt(dena);
	denb = (float)sqrt(denb);

	return (float)(num / (dena*denb));//cos(ai,bi)=num/(dena*denb)
}
//ŷʽ���� euclidean metric
//��ʽ��dist(a,b)=sqrt((a1-b1)*(a1-b1)+(a2-b2)*(a1-b2)+...+(ai-bi)*(ai-bi))
float user_w2v_eu_dist(float *a, float *b, int n) {
	float dest = 0;
	while (n--) {
		dest += (*a - *b)*(*a - *b);//
		a++; b++;
	}
	return (float)sqrt(dest);
}


//����һ�����ڱ��浥���������Ķ���
user_w2v_words_vector *user_w2v_words_vector_create(void) {
	user_w2v_words_vector *words_vector = NULL;
	words_vector = (user_w2v_words_vector *)malloc(sizeof(user_w2v_words_vector));
	words_vector->prior = NULL;
	words_vector->index = 0;
	words_vector->words_string = NULL;
	words_vector->class_id = 0;
	words_vector->vector_number = 0;
	words_vector->vector_data = NULL;
	words_vector->next = NULL;

	return words_vector;
}
//ɾ��һ�����������ͷ��ڴ�
void user_w2v_words_vector_delete(user_w2v_words_vector *dest) {
	if (dest != NULL) {
		if (dest->vector_data != NULL) {
			free(dest->vector_data);//�ͷ�����
		}
		if (dest->words_string != NULL) {
			free(dest->words_string);//�ͷ�����
		}
		free(dest);//�ͷŽṹ��
	}
}
//ɾ��������������
void user_w2v_words_vector_all_delete(user_w2v_words_vector *dest) {
	user_w2v_words_vector *next = dest->next;
	while (dest != NULL) {
		next = dest->next;
		user_w2v_words_vector_delete(dest);//ɾ����ǰָ������
		dest = next;//����������һ��
	}
}
//���һ�������ṹ��
//dest��������
//index��������
//words���ַ�������
//size����������
//vector������ָ��
//���أ��ɹ���ʧ��
bool user_w2v_words_vector_fill(user_w2v_words_vector *dest,int index, char *words,int classid, int size, float *vector) {
	if (dest == NULL) {
		return false;
	}else{
		dest->index = index;//������ʼλ��
		dest->words_string = (char *)malloc(strlen(words) + 1);//�����ڴ� Ԥ����ӽ������ֽ�
		memcpy(dest->words_string, words, strlen(words) + 1);//�������� ��ӽ�����
		dest->words_string[strlen(words)] = '\0';//���ý�����0
		dest->class_id = classid;//������ID

		dest->vector_number = size;//�����С
		dest->vector_data = (float *)malloc((int)size * sizeof(float));//���������ڴ�
		memcpy(dest->vector_data, vector, (int)size * sizeof(float));//��������		
	}
	return true;
}

//���һ�����ڱ��浥���������Ķ���
//dest��������
//words���ַ�������
//size����������
//classid:���������
//vector������ָ��
//���أ��ɹ���ʧ��
bool user_w2v_words_vector_add(user_w2v_words_vector *dest,char *words,int classid,int size,float *vector) {
	if (dest == NULL) {
		return false;
	}
	else {
			//����ڴ�Ϊ����ôֱ�ӽ������
			if (dest->index == 0) {
				if (user_w2v_words_vector_fill(dest,1, words, classid, size, vector)) {
					return true;
				}
			}
			else {
				while (dest->next != NULL) {
					dest = dest->next;//ָ����һ���ṹ��
				}
				dest->next = user_w2v_words_vector_create();//�������ݿռ�
				dest->next->prior = dest;//������һ���ṹ���λ��
				if (user_w2v_words_vector_fill(dest->next, dest->index+1, words, classid, size, vector)) {
					return true;
				}
			}
	}
	return false;
}

//��������words֮���cos�н�
//model��ģ�Ͷ���
//type�����㷽ʽ
//stra�������ַ���a
//strb�������ַ���b
//���أ�����
float user_w2v_words_similar(user_w2v_words_vector *model, distance_type type,char *stra, char *strb) {
	user_w2v_words_vector *word_a = NULL;
	user_w2v_words_vector *word_b = NULL;
	while (1) {
		if ((word_a != NULL) && (word_b != NULL)) {
			if (word_a->vector_number == word_b->vector_number) {
				/*//���Կ�ʼ
				float result = user_w2v_cos_dist(word_a->vector_data, word_b->vector_data, word_a->vector_number);
				printf("\n %s to %s :%f", stra, strb, result);
				return result;
				//���Խ���*/
				if (type == cosine) {
					return user_w2v_cos_dist(word_a->vector_data, word_b->vector_data, word_a->vector_number);
				}
				else if (type == euclidean) {
					return user_w2v_eu_dist(word_a->vector_data, word_b->vector_data, word_a->vector_number);
				}
			}
			else {
				return 0;
			}
		}
		else {
			if (model == NULL) {
				return 0;//�Ҳ���ģ��
			}
			if (strcmp(model->words_string, stra) == 0) {
				word_a = model;
			}
			if (strcmp(model->words_string, strb) == 0) {
				word_b = model;
			}
			model = model->next;//����������һ��ģ��
		}
	}
	
	return 0;
}

//��ȡ��ǩ
//f���ļ�ָ��
//string���������ݵ�ָ��
//���أ��ɹ���ʧ��
bool user_w2v_read_words_string_txt_utf8(FILE *f, char *string) {
	fscanf(f, "%*[\n|' ']%s", string);//utf8���� ��ȡ��ǩ
	return true;
}
//��ȡ������
//f���ļ�ָ��
//count����ȡfloat���ݸ���
//mem�����ڱ������ݵ��ڴ�
//���أ��ɹ���ʧ��
bool user_w2v_read_words_vector_txt_utf8(FILE *f, int count, float *mem) {
	while (count--) {
		fscanf_s(f, "%*[\n|' ']%f", mem++);//��ȡ����������
	}
	return true;
}

//����һ���ı���ʽ�Ĵ��������ڴ��� ���ҷ���˫������
//path���ļ�·��
//���أ��������ṹ������
user_w2v_words_vector *load_words_vector_model(char *path) {
	user_w2v_words_vector *words_vector_list = NULL;
	FILE *file = fopen(path, "r");   //ʹ�ö����ƴ�һ��ģ���ļ�
	int words = 0, size = 0;

	if (file == NULL) {
		printf("Input file not found\n");
		return NULL;
	}
	//��ȡģ���ļ����ݣ�ת��Ϊ������<->����
	fscanf(file, "%d", &words);    //��ȡ���е��ʸ���
	fscanf(file, "%d", &size);     //��ȡÿ�����ʵ�ά�ȴ�С 
	printf("\ntotal number words:%d \ndimension  size : %d ", words, size);

	words_vector_list = user_w2v_words_vector_create();//����һ���ṹ��

	char  *words_string = (char *)malloc(1024);//����1K�ڴ����ڱ��ֵ��ʱ�ǩ
	float *words_vector = (float *)malloc((int)size * sizeof(float));//���������ڴ�

	for (int i = 0; i<words; i++) {
		memset(words_string, 0, sizeof(words_string));//�������
		user_w2v_read_words_string_txt_utf8(file, words_string);//��ȡ��ǩ
		user_w2v_read_words_vector_txt_utf8(file, (int)size, words_vector);//��ȡ������
		user_w2v_words_vector_add(words_vector_list, words_string, 0, (int)size, words_vector);//�������
	}
	free(words_string);
	free(words_vector);
	fclose(file);

	return words_vector_list;
}

//����һ�����ڱ������������Ķ���
//���أ������ڴ������ƶ�����
user_w2v_similar_list *user_w2v_similar_list_create(void) {
	user_w2v_similar_list *similar_list = NULL;
	similar_list = (user_w2v_similar_list *)malloc(sizeof(user_w2v_similar_list));
	similar_list->prior = NULL;
	similar_list->index = 0;
	similar_list->result_addr = NULL;
	similar_list->result_similar = 0;
	similar_list->next = NULL;

	return similar_list;
}
//���list��
//dest���������
//index�����
//addr���ַ���ָ��
//similar�����ƶ�ֵ
//���أ��ɹ���ʧ��
bool user_w2v_similar_list_fill(user_w2v_similar_list *dest,int index,user_w2v_words_vector *addr, float similar) {
	if (dest == NULL) {
		return false;
	}
	else {
		dest->index = index;//������ʼλ��
		dest->result_addr = addr;
		dest->result_similar = similar;
	}
	return true;
}
//���һ�����ڱ��浥���������Ķ���
//dest���������
//addr���ַ���ָ��
//similar�����ƶ�ֵ
//���أ��ɹ���ʧ��
bool user_w2v_similar_list_add(user_w2v_similar_list *dest, user_w2v_words_vector *addr,float similar) {

	if (dest == NULL) {
		return false;
	}
	else {
		//����ڴ�Ϊ����ôֱ�ӽ������
		if (dest->index == 0) {
			if (user_w2v_similar_list_fill(dest, 1, addr, similar)) {
				return true;
			}
		}
		else {
			while (dest->next != NULL) {
				dest = dest->next;//ָ����һ���ṹ��
			}
			dest->next = user_w2v_similar_list_create();//�������ݿռ�
			dest->next->prior = dest;//������һ���ṹ���λ��
			if (user_w2v_similar_list_fill(dest->next, dest->index + 1, addr, similar)) {
				return true;
			}
		}

	}
	return false;
}
//ɾ��һ���������ͷ��ڴ�
//dest��ɾ������
//����ֵ����
void user_w2v_similar_list_delete(user_w2v_similar_list *dest) {
	if (dest != NULL) {
		free(dest);//�ͷŽṹ��
	}
}
//ɾ�������������������
//dest��ɾ������ʼ����
//����ֵ����
void user_w2v_similar_list_all_delete(user_w2v_similar_list *dest) {
	user_w2v_similar_list *next = dest->next;
	while (dest != NULL) {
		next = dest->next;
		user_w2v_similar_list_delete(dest);//ɾ����ǰָ������
		dest = next;//����������һ��
	}
}
//�ҳ�ģ������ֵ���ڵ��ַ�
//model��ģ�Ͷ���
//type�����뷽ʽ
//string��Ŀ�굥��
//thrd����ֵ
//���أ����
user_w2v_similar_list *user_w2v_words_most_similar(user_w2v_words_vector *model, distance_type type, char *string,float thrd) {
	user_w2v_words_vector *word_a = model;
	user_w2v_words_vector *word_b = model;
	user_w2v_similar_list *result = user_w2v_similar_list_create();
	float similar_vaule = 0;

	while (word_a != NULL) {
		if (strcmp(word_a->words_string, string) == 0) {
			while (word_b != NULL) {
				if (word_a->vector_number == word_b->vector_number) {
					if (type == cosine) {//cosine�����ǼнǾ�����ôԽ��������Խ����ô��ֵ����Сֵ
						similar_vaule = user_w2v_cos_dist(word_a->vector_data, word_b->vector_data, word_a->vector_number);//���м���
						if (similar_vaule >= thrd) {
							user_w2v_similar_list_add(result, word_b, similar_vaule);//�������
						}
					}
					else if (type == euclidean) {//ŷʽ�������ƶ����룬�����ƾ���ԽС����ô��ֵӦ�������ֵ
						similar_vaule = user_w2v_eu_dist(word_a->vector_data, word_b->vector_data, word_a->vector_number);//���м���
						//similar_vaule = 1 - 1 / similar_vaule;
						//printf("\n%f\n", similar_vaule);
						if (similar_vaule <= thrd) {
							user_w2v_similar_list_add(result, word_b, similar_vaule);//�������
						}
					}

				}
				word_b = word_b->next;//����������һ��ģ��
			}
			break;
		}
		word_a = word_a->next;//����������һ��ģ��
	}
	return result;
}


//��������
//dest��Ŀ����� �������Զ�ɾ��
//type����ʾ�����������
//���أ�����������
//
user_w2v_similar_list *user_w2v_similar_sorting(user_w2v_similar_list *dest, sorting_type type) {
	user_w2v_similar_list *result_count = dest;
	user_w2v_similar_list *result_nor = NULL;
	user_w2v_similar_list *result = user_w2v_similar_list_create();
	float similar_nor = 0;//��ʱ����������
	int index_max = 0;//��ʱ����������λ��
	user_w2v_words_vector *addr_max = NULL;//�����ַ

	while (result_count != NULL){
		result_count = result_count->next;
		//��ʼ��ѯһ�μ���ֵ
		if (type == sorting_down) {
			similar_nor = -FLT_MAX;//���ü��ޱ�׼ֵ
		}
		else if (type == sorting_up) {
			similar_nor = FLT_MAX;//���ü��ޱ�׼ֵ
		}
		result_nor = dest;//��ʼ��ֵ����ͷ
		while (result_nor != NULL) {
			if (type == sorting_down) {
				if (similar_nor <= result_nor->result_similar) {
					index_max = result_nor->index;//��ȡָ��
					similar_nor = result_nor->result_similar;//�������ֵ
					addr_max = result_nor->result_addr;
				}
			}
			else if (type == sorting_up) {
				if ( result_nor->result_similar <= similar_nor) {
					index_max = result_nor->index;//��ȡָ��
					similar_nor = result_nor->result_similar;//�������ֵ
					addr_max = result_nor->result_addr;
				}
			}

			result_nor = result_nor->next;//����������һ��
		}
		//������ѯ����ֵ
		//��ʼɾ������ֵ
		result_nor = dest;//��ʼ��ֵ����ͷ
		while (result_nor != NULL) {
			if (index_max == result_nor->index) {
				if (type == sorting_down) {
					result_nor->result_similar = -FLT_MAX;//���ü��ޱ�׼ֵ
				}
				else if (type == sorting_up) {
					result_nor->result_similar = FLT_MAX;//���ü��ޱ�׼ֵ
				}
			}
			result_nor = result_nor->next;//����������һ��
		}
		//����ɾ�����ֵ
		//������º��ֵ
		user_w2v_similar_list_add(result, addr_max, similar_nor);//�������
		//printf("\nv:%s,    %f", addr_max->words_string, similar_nor);
	}
	user_w2v_similar_list_all_delete(dest);//ɾ���������
	return result;
}
//��ӡ��������
//����
//to_file���Ƿ�������ļ�
//similar_list����������
//���� ��
void user_w2v_similar_list_printf(bool to_file, user_w2v_similar_list *similar_list) {
	FILE *debug_file = NULL;
	//debug_file = fopen("debug.txt", "w+");
	if (to_file == true)
		debug_file = fopen("words_vec_debug.txt", "w+");

	while (similar_list != NULL) {
		printf("%s\t\t%-10.6f \n", similar_list->result_addr->words_string, similar_list->result_similar);
		if (debug_file != NULL)
			fprintf(debug_file, "%s\t\t%-10.6f \n", similar_list->result_addr->words_string, similar_list->result_similar);
		similar_list = similar_list->next;
	}
	printf("\n\n");
	if (debug_file != NULL)
		fprintf(debug_file, "\n");
	//fclose(debug_file);
	if (debug_file != NULL)
		fflush(debug_file);
}
