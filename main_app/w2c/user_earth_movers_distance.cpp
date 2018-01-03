#include "user_earth_movers_distance.h"


//�������ж��庯��
int user_emd_float_sort(const void *a, const void *b){
	return (*(float*)b < *(float*)a)?1:-1;  
}

//��ȡ�ܷ���
float user_emd_matrix_get_cost_value(user_nn_matrix *src_matrix, user_nn_matrix *sub_matrix) {
	int total = src_matrix->width * src_matrix->height;
	float result = 0.0f;
	float *src_data = src_matrix->data;
	float *sub_data = sub_matrix->data;

	while (total--) {
		if (*sub_data != 0) {
			result += *src_data * *sub_data;
		}
		sub_data++;
		src_data++;
	}
	return result;
}
//���þ�����к��еĹ̶�ֵ
void user_emd_matrix_set_row_col(user_nn_matrix *save_matrix, float value) {
	int index = 0;
	float *save_data = save_matrix->data;
	for (index=0; index<save_matrix->width; index++) {
		*save_data++ = value;
	}
	for (index = 0; index < save_matrix->height - 1; index++) {
		*save_data = value;
		save_data += save_matrix->width;
	}
}
//ͨ�������Ƿ�Ϊ0��������
void user_emd_matrix_unzero_mapping_cpy(user_nn_matrix *save_matrix, user_nn_matrix *sub_matrix) {
	int post_y = 0, post_x = 0;
	for (post_y = 1; post_y < save_matrix->height; post_y++) {
		for (post_x = 1; post_x < save_matrix->width; post_x++) {
			if (*user_nn_matrix_ext_value(save_matrix, post_x, post_y) != 0) {
				*user_nn_matrix_ext_value(save_matrix, post_x, post_y) = *user_nn_matrix_ext_value(sub_matrix, post_x, post_y);
			}
		}
	}
}
//ͨ�������Ƿ�Ϊ0��������
void user_emd_matrix_zero_mapping_cpy(user_nn_matrix *save_matrix, user_nn_matrix *sub_matrix) {
	int post_y = 0, post_x = 0;
	for (post_y = 1; post_y < save_matrix->height; post_y++) {
		for (post_x = 1; post_x < save_matrix->width; post_x++) {
			if (*user_nn_matrix_ext_value(save_matrix, post_x, post_y) == 0) {
				*user_nn_matrix_ext_value(save_matrix, post_x, post_y) = *user_nn_matrix_ext_value(sub_matrix, post_x, post_y);
			}
			else {
				*user_nn_matrix_ext_value(save_matrix, post_x, post_y) = 0;
			}
		}
	}
}

//����һ��emd�ṹ����
//dist_array������ֵ
//width�����������
//height����������
//w_size���������
//h_size��Ŀ�ĸ���
//���أ��µľ���
user_nn_matrix *user_emd_object_create(float *dist_array,float *height_array,int height,float *width_array,int width){
	user_nn_matrix *result = NULL;
	float total_input = 0.0f;
	float total_output=0.0f;
	int index = 0;

	//У����������Ȩ����Ҫһ��
	for(index=0;index < height;index++){
		total_input += *height_array++;
	}
	height_array -= height;
	for(index=0;index < width;index++){
		total_output += *width_array++;
	}
	if(total_input != total_output){
		return NULL;
	}
	width_array -= width;
	for(index = 0;index<(height*width);index++){
		if(*dist_array++ < 0.0f){
			return NULL;
		}
	}
	dist_array -= height*width;
	
	result = user_nn_matrix_create(width + 1,height + 1);//�����µ����ݾ���
	user_nn_matrix_save_array(result, dist_array, 1, 1, width, height);//���ؾ�������
	user_nn_matrix_save_array(result, height_array, 0, 1, 1, height);//��������Ȩ��
	user_nn_matrix_save_array(result, width_array, 1, 0, width, 1);//�������Ȩ��

	return result;
}

/*   
		o1	o2 o3 o4 
	00	w1 w2 w3 w4 00
i1	w1	a1 a2 a3 a4 ta 
i2	w2	b1 b2 b3 b4 tb
i3	w3	c1 c2 c3 c4 tc
	00	t1 t2 t3 t4 00
	ix����ʾ����ֿ�
	ox����ʾ����ֿ�
	wx����ʾ��������
	ax����ʾ����ɱ����߾���
	tx�����ڱ������е���Сֵ���Сֵ�Ĳ�
*/
//ͨ���������һ�δ���һ��emd����
//src_matrix���������
//���أ�������� float�����ֵ �������ֵ��ʾ��Ч���߾����Լ��������
//���� volge�㷨 VAM��
//�㷨ʵ�֣�
//�׶�һ��ȡ�����л����� ������ȡ���е�λ��
//�׶ζ����������д����������
float user_emd_vogel_plan_init_martix(user_nn_matrix *src_matrix, user_nn_matrix *tra_matrix){
	user_nn_matrix *row_matrix = user_nn_matrix_create(src_matrix->width - 1,1);//���������ݵľ���
	user_nn_matrix *col_matrix = user_nn_matrix_create(1,src_matrix->height - 1);//���������ݵľ���
	float result = FLT_MAX, transfer_value = 0.0f;
	bool max_vaule_is_row = true;//�����Ƿ����л�����
	int max_vaule_index = 1;//��ʼ��Ϊ1 ��0����0�б������Ȩ��
	float max_vaule = -FLT_MAX;//���ڱ���ֵ
	int index = 0;

	float constant_value = 0.0f;//����ֵ
	float *variable_value = NULL;//�б�ֵ
	float *distance_vaule = NULL;//����ֵ
	int min_vaule_index = 0;

	//��һ�׶��ҳ���Ҫ�������
	for(index=0;index<col_matrix->height;index++){
		if(*user_nn_matrix_ext_value(src_matrix,0,index + 1) != 0.0f){//�ж�Ȩ���Ƿ���0 �������ô������һ��
			if(user_nn_matrix_cpy_array(row_matrix->data,src_matrix, 1,index + 1,row_matrix->width,1)){//����һ�о������� �ɹ�������һ������
				qsort(row_matrix->data,row_matrix->width*row_matrix->height,sizeof(row_matrix->data),user_emd_float_sort);//�������� ��������
				//���������һ����С�����С����Ч ����ֻ����С��Ч����С��Ч
				if((row_matrix->data[0] != FLT_MAX) && (row_matrix->data[1] != FLT_MAX)){
					//user_nn_matrix_save_float(src_matrix,src_matrix->width - 2 + 1,index + 1,min_vaule);//���ò�ֵ �����Ҫ�򿪴˺�����ô��Ҫ���þ����ȼ�2 ��һ��user_nn_matrix_create(src_matrix->width - 1,1)����Ϊuser_nn_matrix_create(src_matrix->width - 2,1) �����������Ҳ��Ҫ����
					//row_matrix->data[1] - row_matrix->data[0];//ȡ�ò�ֵ
					if(max_vaule < (row_matrix->data[1] - row_matrix->data[0])){
						max_vaule = (row_matrix->data[1] - row_matrix->data[0]);
						max_vaule_is_row = true;
						max_vaule_index = index + 1;
						//printf("\nmin_vaule %d,%f\n",index+1,min_vaule);
					}
				}else if((row_matrix->data[0] != FLT_MAX) && (row_matrix->data[1] == FLT_MAX)){
					max_vaule = row_matrix->data[0];
					max_vaule_is_row = true;
					max_vaule_index = index + 1;
					//user_nn_matrix_save_float(src_matrix,src_matrix->width - 2 + 1,index + 1,0);//����Ϊ0
				}
			}
		}
	}
	for(index=0;index<row_matrix->width;index++){
		if(*user_nn_matrix_ext_value(src_matrix,index + 1,0) != 0.0f){	//�ж�Ȩ���Ƿ���0 �������ô��һ������
			if(user_nn_matrix_cpy_array(col_matrix->data,src_matrix,index + 1,1,1,col_matrix->height)){
				qsort(col_matrix->data,col_matrix->width*col_matrix->height,sizeof(col_matrix->data),user_emd_float_sort);//��������
				//���������һ����С�����С����Ч ����ֻ����С��Ч����С��Ч
				if((col_matrix->data[0] != FLT_MAX) && (col_matrix->data[1] != FLT_MAX)){
					//user_nn_matrix_save_float(src_matrix,index + 1,src_matrix->height - 2 + 1,min_vaule);//����ֵ	
					//min_vaule = col_matrix->data[1] - col_matrix->data[0];//ȡ�ò�ֵ
					if(max_vaule < (col_matrix->data[1] - col_matrix->data[0])){
						max_vaule = col_matrix->data[1] - col_matrix->data[0];
						max_vaule_is_row = false;
						max_vaule_index = index + 1;
						//printf("\nmin_vaule %d,%f\n",index+1,min_vaule);
					}
				}else if((col_matrix->data[0] != FLT_MAX) && (col_matrix->data[1] == FLT_MAX)){
					max_vaule = col_matrix->data[0];
					max_vaule_is_row = false;
					max_vaule_index = index + 1;
					//user_nn_matrix_save_float(src_matrix,src_matrix->width - 2 + 1,index + 1,0);//����Ϊ0
				}
			}
		}
	}
	//printf("\n%s,index=%d,max value=%f\n",max_vaule_is_row==true?"row":"col",max_vaule_index,max_vaule);
	//�ڶ��׶� �������ݶ���
	if(max_vaule_is_row){
		user_nn_matrix_cpy_array(row_matrix->data,src_matrix,1,0,row_matrix->width,1);//��ȡ������������
		constant_value = *user_nn_matrix_ext_value(src_matrix,0,max_vaule_index);//ȡ������Ȩ�ص�Ψһ����ֵ
		variable_value = user_nn_matrix_ext_value(src_matrix,1,0);//ȡ�����Ȩ�ص���ʼλ��
		distance_vaule = user_nn_matrix_ext_value(src_matrix,1,max_vaule_index);
		for(index=0;index < (row_matrix->height * row_matrix->width);index++){
			//printf("\nrow constant_value=%f,*variable_value=%f��*distance_vaule=%f\n",constant_value,*variable_value,*distance_vaule);
			if((*distance_vaule !=FLT_MAX) && (constant_value != 0) && (*variable_value != 0)){			
				if(*variable_value > constant_value){
					if(result > (*distance_vaule * constant_value)){
						result = *distance_vaule * constant_value;
						min_vaule_index = index;
					}	
				}else{
					if(result > (*distance_vaule * (*variable_value))){
						result = *distance_vaule * (*variable_value);
						min_vaule_index = index;
					}	
				}
			}
			distance_vaule++;
			variable_value++;
		}
		
		if ((constant_value != 0) && (row_matrix->data[min_vaule_index] != 0)){//���в�������ܽ��м���
			if (constant_value > row_matrix->data[min_vaule_index]) {
				//������ֵ��������ֵ ɾ��һ��
				transfer_value = row_matrix->data[min_vaule_index];////��ȡת��ֵ
				user_nn_matrix_save_float(src_matrix, 0, max_vaule_index, constant_value - row_matrix->data[min_vaule_index]);//�޸�����ֵ
				user_nn_matrix_save_float(src_matrix, min_vaule_index + 1, 0, 0);//����ֵ

				user_nn_matrix_memset(col_matrix, FLT_MAX);//�����о���Ϊ���ֵ
				user_nn_matrix_save_matrix(src_matrix, col_matrix, min_vaule_index + 1, 1);//�������ֵ����в��ڴ���
			}
			else {
				//������ֵС������ֵ ɾ����
				transfer_value = constant_value;////��ȡת��ֵ
				user_nn_matrix_save_float(src_matrix, 0, max_vaule_index, 0);//����ֵ
				user_nn_matrix_save_float(src_matrix, min_vaule_index + 1, 0, row_matrix->data[min_vaule_index] - constant_value);//����ֵ

				user_nn_matrix_memset(row_matrix, FLT_MAX);//�����о���Ϊ���ֵ
				user_nn_matrix_save_matrix(src_matrix, row_matrix, 1, max_vaule_index);//�������ֵ����в��ڴ���
			}
			*user_nn_matrix_ext_value(tra_matrix, min_vaule_index + 1, max_vaule_index) = transfer_value;
		}
		//printf("change: x:%d y��%d v:%f\n",  min_vaule_index + 1, max_vaule_index, transfer_value);
		//user_nn_matrix_printf(NULL, tra_matrix);//
		//result = result;
	}else{
		user_nn_matrix_cpy_array(col_matrix->data,src_matrix,0,1,1,col_matrix->height);//��ȡ������������
		constant_value = *user_nn_matrix_ext_value(src_matrix,max_vaule_index,0);//ȡ������Ȩ�ص�Ψһ����ֵ
		variable_value = user_nn_matrix_ext_value(src_matrix,0,1);//ȡ�����Ȩ�ص���ʼλ��
		distance_vaule = user_nn_matrix_ext_value(src_matrix,max_vaule_index,1);
		for(index=0;index < (col_matrix->height * col_matrix->width);index++){
			//printf("\ncol constant_value=%f,*variable_value=%f��*distance_vaule=%f\n",constant_value,*variable_value,*distance_vaule);
			if((*distance_vaule != FLT_MAX) && (constant_value != 0) && (*variable_value != 0)){		
				if(*variable_value > constant_value){
					if(result > (*distance_vaule * constant_value)){
						result = *distance_vaule * constant_value;
						min_vaule_index = index;
					}
				}else{
					if(result > (*distance_vaule * (*variable_value))){
						result = *distance_vaule * (*variable_value);
						min_vaule_index = index;
					}
				}	
			}
			distance_vaule += src_matrix->width;//����Ҫ�ƶ�width�ľ���
			variable_value += src_matrix->width;//��Ȩ��ͬ����Ҫ�ƶ�width�ľ���
		}
		if ((constant_value != 0) && (row_matrix->data[min_vaule_index] != 0)){//���в�������ܽ��м���
			if (col_matrix->data[min_vaule_index] > constant_value) {
				//�������ֵ ��������ֵ ��� ��
				transfer_value = constant_value;//��ȡת��ֵ
				user_nn_matrix_save_float(src_matrix, 0, min_vaule_index + 1, col_matrix->data[min_vaule_index] - constant_value);//����ֵ
				user_nn_matrix_save_float(src_matrix, max_vaule_index, 0, 0);//����ֵ

				user_nn_matrix_memset(col_matrix, FLT_MAX);//�����о���Ϊ���ֵ
				user_nn_matrix_save_matrix(src_matrix, col_matrix, max_vaule_index, 1);//�������ֵ����в��ڴ���
			}
			else {
				//�������ֵ С������ֵ ��� ��
				transfer_value = col_matrix->data[min_vaule_index];//��ȡת��ֵ
				user_nn_matrix_save_float(src_matrix, 0, min_vaule_index + 1, 0);//����ֵ
				user_nn_matrix_save_float(src_matrix, max_vaule_index, 0, constant_value - col_matrix->data[min_vaule_index]);//����ֵ

				user_nn_matrix_memset(row_matrix, FLT_MAX);//�����о���Ϊ���ֵ
				user_nn_matrix_save_matrix(src_matrix, row_matrix, 1, min_vaule_index + 1);//�������ֵ����в��ڴ���
			}
			*user_nn_matrix_ext_value(tra_matrix, max_vaule_index, min_vaule_index + 1) = transfer_value;
		}
		//printf("change: x:%d y��%d v:%f\n", max_vaule_index, min_vaule_index + 1, transfer_value);
		//user_nn_matrix_printf(NULL, tra_matrix);//
		//result = result;
	}
	//
	user_nn_matrix_delete(row_matrix);//ɾ������
	user_nn_matrix_delete(col_matrix);//ɾ������

	//user_nn_matrix_printf(NULL, src_matrix);
	
	return  result;
}

//����������㷨��ȡ��ʼ���� 
user_nn_matrix *user_emd_get_vogel_init_martix(user_nn_matrix *src_matrix){
	user_nn_matrix *temp_matrix = NULL,*result_matirx = NULL;

	temp_matrix = user_nn_matrix_cpy_create(src_matrix);//���ƾ���
	result_matirx = user_nn_matrix_create(src_matrix->width, src_matrix->height);//ת�����ľ���
	//float socre = 0;
	if (temp_matrix != NULL) {
		while (user_emd_vogel_plan_init_martix(temp_matrix, result_matirx) < FLT_MAX) {
		}
	}
//	printf("\n%f\n", socre);
	//user_nn_matrix_printf(NULL, result_matirx);//

	user_nn_matrix_delete(temp_matrix);//ɾ������
	return result_matirx;
}
//����λ�Ʋ��ż�������������������ż����
user_nn_matrix *user_emd_potential_plan_matrix(user_nn_matrix *src_matrix) {
	user_nn_matrix *result_matirx = NULL;
	int post_x = 0, post_y = 0;
	float *variable_value = NULL, *variable_row = NULL, *variable_col = NULL;

	result_matirx = user_nn_matrix_cpy_create(src_matrix);//���ƾ���
	//printf("\n�������������\n");
	//user_nn_matrix_printf(NULL, src_matrix);//
	while (*user_nn_matrix_ext_value(result_matirx, 0, 0) == 0) {
		*user_nn_matrix_ext_value(result_matirx, 0, 0) = 1;//����ѭ��һ��
//		user_nn_matrix_printf(NULL, result_matirx);//
		for (post_y = 1; post_y < result_matirx->height; post_y++) {
			for (post_x = 1; post_x < result_matirx->width; post_x++) {//ѭ�����ֵ
				variable_value = user_nn_matrix_ext_value(result_matirx, post_x, post_y);
				variable_row = user_nn_matrix_ext_value(result_matirx, post_x, 0);
				variable_col = user_nn_matrix_ext_value(result_matirx, 0, post_y);
				if (*variable_value != 0) {
					//������ǵ�һ�У����л���ֵ����0����ô���м���
					if (post_y == 1) {//�������׸�ֵΪ0
						if (*variable_row == 0) {
							*variable_row = *variable_value;
						}
					}
					else {
						//printf("\n%f\n", fabs(*variable_value - *variable_row - *variable_col));
						//if ((*variable_row + *variable_col) != *variable_value) {
						if (fabs(*variable_value - *variable_row - *variable_col) > emd_potential_accuracy) {
							if ((*variable_row == 0) && (*variable_col != 0)) {
								*variable_row = *variable_value - *variable_col;
							}
							else if ((*variable_row != 0) && (*variable_col == 0)) {
								*variable_col = *variable_value - *variable_row;
							}
							else {
								//printf("\n%f %f\n", *variable_row + *variable_col, *variable_value);
								*user_nn_matrix_ext_value(result_matirx, 0, 0) = 0;//����Ϊ����δ���
							}
						}
					}
				}				
			}
		}
	}
	result_matirx->data[0] = 0;//����Ϊ0
	return result_matirx;
}
//�����ż�����Ƿ���ȷ
user_nn_matrix *user_emd_censor_potential_matrix(user_nn_matrix *src_matrix) {
	user_nn_matrix *result_matirx = NULL;
	int post_x = 0, post_y = 0;
	float *variable_value = NULL, *variable_row = NULL, *variable_col = NULL;

	result_matirx = user_nn_matrix_cpy_create(src_matrix);//���ƾ���
	while (*user_nn_matrix_ext_value(result_matirx, 0, 0) == 0) {
		*user_nn_matrix_ext_value(result_matirx, 0, 0) = 1;//����ѭ��һ��
		for (post_y = 1; post_y < result_matirx->height; post_y++) {
			for (post_x = 1; post_x < result_matirx->width; post_x++) {//ѭ�����ֵ
				variable_value = user_nn_matrix_ext_value(result_matirx, post_x, post_y);
				variable_row = user_nn_matrix_ext_value(result_matirx, post_x, 0);
				variable_col = user_nn_matrix_ext_value(result_matirx, 0, post_y);
				if (*variable_value != 0) {
					*variable_value = *variable_value - *variable_row - *variable_col;
				}
			}
		}
	}
	result_matirx->data[0] = 0;//����Ϊ0
	return result_matirx;
}
//����ֵ
void user_emd_adjust_matrix_value(user_nn_matrix *src_matrix, user_nn_matrix *path_matix) {
	float *path_matix_data = path_matix->data;
	int count = 0;
	float min_value = FLT_MAX;

	path_matix_data = path_matix->data;
	int start_point = 1;//�������
	float dest_point = 0.0f;
	bool is_row = false;
	if (fabs(path_matix_data[0] - path_matix_data[1]) < path_matix->width) {//��ʼ������
		is_row = true;
	}
	//user_nn_matrix_printf(NULL, path_matix);//
	//ɾ��·���ϷǶ���ĵ�
	for (; start_point < (path_matix->height * path_matix->width - 1); start_point++) {
		if (path_matix_data[start_point + 1] != 0) {
			dest_point = fabs(path_matix_data[start_point] - path_matix_data[start_point + 1]);//�����ֵ
		}else {
			dest_point = fabs(path_matix_data[start_point] - path_matix_data[0]);//�����ֵ
		}
		if (dest_point < path_matix->width) {//������
			if (is_row == true) {
				*user_nn_matrix_ext_value_index(path_matix, start_point) = 0;//
			}
			else {
				is_row = true;
			}
		}else {
			if (is_row == false) {
				*user_nn_matrix_ext_value_index(path_matix, start_point) = 0;//
			}
			else {
				is_row = false;
			}
		}
	}
	//��ȡ���е��������Сֵ
	path_matix_data = path_matix->data;
	for (start_point = 0; start_point < path_matix->height * path_matix->width; start_point++) {
		//printf("\path_matix_data��%f\n", *path_matix_data);
		if (*path_matix_data != 0) {
			if ((count++ % 2) == 1) {
				//*user_nn_matrix_ext_value_index(src_matrix, (int)*path_matix_data) -= value;
				if ((*user_nn_matrix_ext_value_index(src_matrix, (int)*path_matix_data) < min_value)&&
					*user_nn_matrix_ext_value_index(src_matrix, (int)*path_matix_data) != 0) {
					min_value = *user_nn_matrix_ext_value_index(src_matrix, (int)*path_matix_data);//
				}
			}
		}
		path_matix_data++;
	}
	//printf("\nmin_value��%f\n", min_value);
	//user_nn_matrix_printf(NULL, path_matix);//
	//�޸�Ŀ��ֵ
	count = 0;
	path_matix_data = path_matix->data;
	for (start_point = 0; start_point < path_matix->height * path_matix->width; start_point++) {
		if (*path_matix_data != 0) {
			if ((count++ % 2) == 1) {//����
				*user_nn_matrix_ext_value_index(src_matrix, (int)*path_matix_data) -= min_value;
			}
			else {//ż��
				*user_nn_matrix_ext_value_index(src_matrix, (int)*path_matix_data) += min_value;
			}
		}
		path_matix_data++;
	}
	//******* ��Ҫ �б�Ҫ��� 
	//�����·��ż�������ͬʱ���������������ϱ仯ֵ��ͬ����ô�ڵ���������һ����δ�ո�������0��
	//******* ��Ҫ �б�Ҫ���
}
//����һ������·��
user_nn_matrix *user_emd_get_loop_path_list(user_nn_matrix *maps_matrix, int centor_point) {
	//����һ����������·����
	user_nn_matrix *path_matrix = user_nn_matrix_create(maps_matrix->width, maps_matrix->height);//����·������
	user_nn_matrix *result_matrix = user_nn_matrix_create(maps_matrix->width, maps_matrix->height);
	int start_end_point_array[4], next_point_array[4], path_index = 1, new_point = 0;

	*user_nn_matrix_ext_value_index(maps_matrix, centor_point) = 1;//ɾ��maps_matrix���ĵ�
	*user_nn_matrix_ext_value_index(path_matrix, centor_point) = 1;//ɾ��path_matrix���ĵ�
	result_matrix->data[0] = (float)centor_point;
	result_matrix->data[1] = (float)user_emd_get_center_around_point(maps_matrix, centor_point, centor_point, start_end_point_array);//���������µķ�ʽ�������ڵ���Χ�ĸ���
	*user_nn_matrix_ext_value_index(path_matrix, (int)result_matrix->data[1]) = 1.0f;//���õ�Ϊ·����

	//printf("\ncentor_point:%d\n", centor_point);
	//user_nn_matrix_printf(NULL, maps_matrix);//
	for (;;) {
		path_index++;
		user_emd_get_center_around_point(maps_matrix, centor_point, (int)result_matrix->data[path_index - 1], next_point_array);//��ȡ��Χ�ĵ�
		new_point = user_emd_check_vaild_point(path_matrix, next_point_array);//У����Щ����Ч ͬʱɾ����Ч�ĵ�
		if (new_point == -1) {//���е㶼��Ч
			*user_nn_matrix_ext_value_index(maps_matrix, (int)result_matrix->data[path_index - 1]) = 1.0f;//ɾ���Ѿ�������·����
			path_index = 1;
			user_nn_matrix_memset(result_matrix, 0);//���·��
			user_nn_matrix_memset(path_matrix, 0);//���·��
			result_matrix->data[0] = (float)centor_point;
			result_matrix->data[1] = (float)user_emd_get_center_around_point(maps_matrix, centor_point, centor_point, start_end_point_array);//���������µķ�ʽ�������ڵ���Χ�ĸ���
			if ((-1 == start_end_point_array[0]) && (-1 == start_end_point_array[1]) &&
				(-1 == start_end_point_array[2]) && (-1 == start_end_point_array[3])) {
				//�Ҳ����κ�����·���� �����˳�
				//user_nn_matrix_printf(NULL, path_matrix);//
				user_nn_matrix_delete(path_matrix);//ɾ������
				user_nn_matrix_delete(result_matrix);//ɾ������
				return NULL;//���ؿ�
			}
			if (result_matrix->data[1] != -1) {
				*user_nn_matrix_ext_value_index(path_matrix, (int)result_matrix->data[1]) = 1.0f;//����Ϊ·����
			}
		}
		else {
			result_matrix->data[path_index] = (float)new_point;//�����µ���Ч��
			if ((new_point == start_end_point_array[0]) || (new_point == start_end_point_array[1]) ||
				(new_point == start_end_point_array[2]) || (new_point == start_end_point_array[3])) {
				//�ҵ��յ� ����·��
				user_nn_matrix_delete(path_matrix);//ɾ������
				return result_matrix;
			}
		}
		//user_nn_matrix_printf(NULL, maps_matrix);//
		//user_nn_matrix_printf(NULL, path_matrix);//
	}
	return NULL;
}

//���·�����Ƿ�Ϊ�Ѵ���
int user_emd_check_vaild_point(user_nn_matrix *path_matrix, int *around_array) {
	for (int index = 0; index < 4; index++) {
		if (around_array[index] != -1) {//����Ƿ������Χ��
			if (*user_nn_matrix_ext_value_index(path_matrix, around_array[index]) == 0) {//����˵㲻��·��ֱ�ӷ���
				*user_nn_matrix_ext_value_index(path_matrix, around_array[index]) = 1;//����Ϊ·����
				return around_array[index];
			}
			else {
				around_array[index] = -1;//ɾ��Ŀ���
			}
		}
	}
	return -1;
}
//���ҿ�ʼ���߽�����
int user_emd_get_center_around_point(user_nn_matrix *matrix, int obstacle_point, int center_point, int *around_array) {
	int point_x = center_point % matrix->width;
	int point_y = center_point / matrix->width;
	int next_point = 0;
	int result = -1;
	around_array[0] = -1; around_array[1] = -1; around_array[2] = -1; around_array[3] = -1;	//������ҵ�

																							//���²��ҵ�
	for (int point_h = point_y + 1; point_h < matrix->height; point_h++) {
		//printf("\n v:%f\n", *user_nn_matrix_ext_value(matrix, point_w, point_y));
		next_point = point_h * matrix->width + point_x;
		if (obstacle_point == next_point) {//�����ϰ���
			break;
		}
		if (*user_nn_matrix_ext_value(matrix, point_x, point_h) == 0)
		{
			around_array[3] = next_point;
			result = next_point;
			break;
		}
	}
	//���Ҳ��ҵ�
	for (int point_w = point_x + 1; point_w < matrix->width; point_w++) {
		//printf("\n v:%f\n", *user_nn_matrix_ext_value(matrix, point_w, point_y));
		next_point = point_y * matrix->width + point_w;
		if (obstacle_point == next_point) {//�����ϰ���
			break;
		}
		if (*user_nn_matrix_ext_value(matrix, point_w, point_y) == 0)
		{
			around_array[2] = next_point;
			result = next_point;
			break;
		}
	}
	//���ϲ��ҵ�
	for (int point_h = point_y - 1; point_h >= 0; point_h--) {
		//printf("\n v:%f\n", *user_nn_matrix_ext_value(matrix, point_w, point_y));
		next_point = point_h * matrix->width + point_x;
		if (obstacle_point == next_point) {//�����ϰ���
			break;
		}
		if (*user_nn_matrix_ext_value(matrix, point_x, point_h) == 0)
		{
			around_array[1] = next_point;//
			result = next_point;
			break;
		}
	}
	//������ҵ�
	for (int point_w = point_x - 1; point_w >= 0; point_w--) {
		//printf("\n v:%f\n", *user_nn_matrix_ext_value(matrix, point_w, point_y));
		next_point = point_y * matrix->width + point_w;
		if (obstacle_point == next_point) {//�����ϰ���
			break;
		}
		if (*user_nn_matrix_ext_value(matrix, point_w, point_y) == 0)
		{
			around_array[0] = next_point;//
			result = next_point;
			break;
		}
	}
	return result;
}

//���������㷨����Сֵ����
float user_emd_earth_movers_distance(float *dist_array, float *height_array, int height, float *width_array, int width) {
	user_nn_matrix *src_matrix = NULL;//ԭʼ����
	user_nn_matrix *vogel_matirx = NULL;//��һ�׶�vogel����
	user_nn_matrix *vogel_matirx_t = NULL;//��һ�׶�vogel����ĸ���
	user_nn_matrix *potential_matrix_t = NULL;//����λ�Ʋ�ľ���
	user_nn_matrix *censor_matrix_t = NULL;//У��λ�Ʋ�ľ���
	user_nn_matrix *path_matrix_t = NULL;//��·�滮����
	float result = 0.0f;

	src_matrix = user_emd_object_create(dist_array, height_array, height, width_array, width);
	vogel_matirx = user_emd_get_vogel_init_martix(src_matrix);//����vogel��ʽ�����״����
	for (;;) {
		vogel_matirx_t = user_nn_matrix_cpy_create(vogel_matirx);//���ƾ���
		user_emd_matrix_unzero_mapping_cpy(vogel_matirx_t, src_matrix);//vogel_matirx_t����0��λ�ñ�src_matrix��ͬλ�����ݸ���
		potential_matrix_t = user_emd_potential_plan_matrix(vogel_matirx_t);//����λ�Ʋ�
		user_emd_matrix_zero_mapping_cpy(potential_matrix_t, src_matrix);//potential_matrix_t��0��λ�ñ�src_matrix��ͬλ�����ݸ��� ����0������Ϊ0
		censor_matrix_t = user_emd_censor_potential_matrix(potential_matrix_t);//���ж�ż������
		user_emd_matrix_set_row_col(censor_matrix_t, *user_nn_matrix_return_max_addr(censor_matrix_t));//��һ�����һ������Ϊ���ֵ�����洦����Ҫ������Сֵ
		if (*user_nn_matrix_return_min_addr(censor_matrix_t) >= 0) {//�Ҳ�����Сֱֵ���˳�
			break;
		}
		path_matrix_t = user_emd_get_loop_path_list(censor_matrix_t, user_nn_matrix_return_min_index(censor_matrix_t));//��ȡ·��
		user_emd_adjust_matrix_value(vogel_matirx, path_matrix_t);//����ֵ

		user_nn_matrix_delete(vogel_matirx_t);
		user_nn_matrix_delete(potential_matrix_t);
		user_nn_matrix_delete(censor_matrix_t);
		user_nn_matrix_delete(path_matrix_t);
	}
	result =  user_emd_matrix_get_cost_value(src_matrix, vogel_matirx);//��ȡ���

	user_nn_matrix_delete(src_matrix); 
	user_nn_matrix_delete(vogel_matirx);
	/*user_nn_matrix_delete(vogel_matirx_t);
	user_nn_matrix_delete(potential_matrix_t);
	user_nn_matrix_delete(censor_matrix_t);
	user_nn_matrix_delete(path_matrix_t);*/

	return result;
}
/*
	����emd����
	//float content[]={0.001,0.359,0.224,0.394,1.024,0.1,0.05,0.99,0.7,0.07,0.07,0.07};//��������
	//float height_weight[]={0.2,0.7,0.1};//�������ݵ�Ȩ��
	//float width_weight[]={0.45,0.05,0.05,0.45};//������ݵ�Ȩ��
	
	//float content[]={0.5,11, 3, 6,5, 9,10, 2,18, 7, 4, 1};//��������
	//float height_weight[]={5,10,15};//�������ݵ�Ȩ��
	//float width_weight[]={3,3,12,12};//������ݵ�Ȩ��

	float content[]={0.1,1,1,1,1,1,1,1,1,1,1,1};//��������
	float height_weight[]={0.2,0.6,0.2};//�������ݵ�Ȩ��
	float width_weight[]={0.3,0.3,0.3,0.1};//������ݵ�Ȩ��
	
	float total_distence=0;

	user_nn_matrix *data_object = NULL;
	data_object = user_emd_object_create(content,height_weight,sizeof(height_weight)/sizeof(float)
												,width_weight,sizeof(width_weight)/sizeof(float));
	if(data_object != NULL){
		while(1){
			float dis = user_emd_calculate_once(data_object);
			//user_nn_matrix_printf(NULL,data_object);
			printf("\n%f\n",dis);
			if(dis < FLT_MAX){
				total_distence += dis;
			}else{
				break;
			}
			
		}
		printf("\ntotal distence : %f\n",total_distence);
	}else{
		printf("\ninput data error\n");
	}

*/

