
#include "user_nn_matrix.h"
#include "user_nn_matrix_cuda.h"

//创建一个矩阵
//参数
//dest：指针对象 必需事先初始化
//width：矩阵的宽度
//height：矩阵的高度
user_nn_matrix *user_nn_matrix_create(int width, int height){
	user_nn_matrix *dest;

	dest = (user_nn_matrix *)malloc(sizeof(user_nn_matrix));//分配保存矩阵空间的大小
	dest->width = width;
	dest->height = height;
	dest->data = (float *)malloc(dest->width * dest->height * sizeof(float));//分配矩阵数据空间
	dest->next = NULL;

	memset(dest->data, 0, dest->width * dest->height * sizeof(float));//清空数据

	return dest;
}
//复制一个矩阵
//参数
//dest：指针对象 必需事先初始化
//width：矩阵的宽度
//height：矩阵的高度
//返回新的矩阵
user_nn_matrix *user_nn_matrix_cpy_create(user_nn_matrix *dest_matrix){
	user_nn_matrix *result=NULL;

	result = (user_nn_matrix *)malloc(sizeof(user_nn_matrix));//分配保存矩阵空间的大小
	result->width = dest_matrix->width;
	result->height = dest_matrix->height;
	result->data = (float *)malloc(result->width * result->height * sizeof(float));//分配矩阵数据空间
	result->next = NULL;

	memcpy(result->data,dest_matrix->data,result->width * result->height * sizeof(float));

	return result;
}
//矩阵转置 交换矩阵的width height包括数据
void user_nn_matrix_transpose(user_nn_matrix *src_matrix){
#ifdef USE_CUDA
	user_nn_matrix_transpose_cuda(src_matrix);
#else
	user_nn_matrix *temp_matrix = NULL;
	float *temp_data = NULL;
	float *src_data = src_matrix->data;
	int width = 0;
	int height = 0;

	if ((src_matrix->width == 1) || (src_matrix->height == 1)){
		//如果是一条矩阵向量 那么直接交换横纵坐标 不用交换数据
		src_matrix->width = src_matrix->width ^ src_matrix->height;
		src_matrix->height = src_matrix->width ^ src_matrix->height;
		src_matrix->width = src_matrix->width ^ src_matrix->height;

		return;//直接退出
	}
	temp_matrix = user_nn_matrix_create(src_matrix->width, src_matrix->height);//创建矩阵
	user_nn_matrix_cpy_matrix(temp_matrix, src_matrix);//拷贝数据
	temp_data = temp_matrix->data;//获取缓冲矩阵的数据指针

	for (width = 0; width < temp_matrix->width; width++){
		for (height = 0; height < temp_matrix->height; height++){
			*src_data++ = *user_nn_matrix_ext_value(temp_matrix, width, height);
		}
	}
	src_matrix->height = width;
	src_matrix->width = height;

	user_nn_matrix_delete(temp_matrix);
#endif
}
//返回矩阵中指定位置的值
//参数
//dest：矩阵
//post_index：坐标位置
float *user_nn_matrix_ext_value_index(user_nn_matrix *dest, int post_index){
	float *value = dest->data;
	if (post_index >= (dest->width * dest->height)){
		return NULL;
	}
	return (value + post_index);
}
//返回矩阵中指定位置的值
//参数
//dest：矩阵
//postx：x坐标
//posty：y坐标
float *user_nn_matrix_ext_value(user_nn_matrix *dest, int postx, int posty){
	return user_nn_matrix_ext_value_index(dest,postx + posty * dest->width);
}
//返回矩阵中最大值的系数
//参数
//dest：矩阵
int user_nn_matrix_return_max_index(user_nn_matrix *dest){
	int count = 0,max_index = 0;
	float *value = NULL;
	float max_value = *dest->data;

	count = 0; value = dest->data;
	for(;count<(dest->width * dest->height);count++){
		if (*value > max_value){
			max_value = *value;
			max_index = count;
		}
		value++;
	}
	return max_index;
}
//返回矩阵中最大值的指针
//参数
//dest：矩阵
float *user_nn_matrix_return_max_addr(user_nn_matrix *dest){
	int count = dest->width * dest->height;
	float *value = dest->data;
	float *result = dest->data;
	float max_value = *dest->data;
	
	while (count--){
		if (*value > max_value){
			max_value = *value;
			result = value;
		}
		value++;
	}
	return  result;
}
//返回矩阵中最小值的系数
//参数
//dest：矩阵
int user_nn_matrix_return_min_index(user_nn_matrix *dest){
	int count = 0,min_index = 0;
	float *value = dest->data;
	float min_value = *dest->data;

	for(;count<(dest->width * dest->height);count++){
		if (*value < min_value){
			min_value = *value;
			min_index = count;
		}
		value++;
	}
	return min_index;
}
//返回矩阵中最小值的指针
//参数
//dest：矩阵
float *user_nn_matrix_return_min_addr(user_nn_matrix *dest){
	int count = dest->width * dest->height;
	float *value = dest->data;
	float *result = dest->data;
	float min_value = *dest->data;
	
	while (count--){
		if (*value < min_value){
			min_value = *value;
			result = value;
		}
		value++;
	}
	return  result;
}
//删除矩阵
void user_nn_matrix_delete(user_nn_matrix *dest){
	if (dest != NULL){
		if (dest->data != NULL){
			free(dest->data);//释放数据
		}
		free(dest);//释放结构体
	}
	//dest = NULL;
}
//创建连续的矩阵
//参数
//total_w：连续矩阵的宽度
//total_h：连续矩阵的高度
//matrix_w matrix_h：连续矩阵的单个矩阵尺寸
user_nn_list_matrix *user_nn_matrices_create(int total_w, int total_h, int matrix_w, int matrix_h){
	user_nn_list_matrix *list_matrix = NULL;//连续矩阵
	user_nn_matrix *matrix = NULL;//单个矩阵大小
	int n = total_w * total_h;//计算需要创建多少个矩阵
	if (n == 0){ return NULL; }//创建矩阵如果为空
	list_matrix = (user_nn_list_matrix *)malloc(sizeof(user_nn_list_matrix));//分配空间
	list_matrix->width = total_w;//设置总矩阵宽度
	list_matrix->height = total_h;//设置总矩阵高度
	list_matrix->matrix = user_nn_matrix_create(matrix_w, matrix_h);//创建首个矩阵
	matrix = list_matrix->matrix;//转化矩阵对象

	while (--n) {
		matrix->next = user_nn_matrix_create(matrix_w, matrix_h);//添加一个矩阵
		matrix = matrix->next;//更新指针对象
		//printf("create matrix=%d\n", width * height - n + 1);
	}
	return list_matrix;
}
//创建连续的矩阵头
//参数
//total_w：连续矩阵的宽度
//total_h：连续矩阵的高度
user_nn_list_matrix *user_nn_matrices_create_head(int total_w, int total_h) {
	user_nn_list_matrix *list_matrix = NULL;//连续矩阵
	user_nn_matrix *matrix = NULL;//单个矩阵大小
	list_matrix = (user_nn_list_matrix *)malloc(sizeof(user_nn_list_matrix));//分配空间
	list_matrix->width = total_w;//设置总矩阵宽度
	list_matrix->height = total_h;//设置总矩阵高度
	list_matrix->matrix = NULL;//创建首个矩阵
	return list_matrix;
}
//删除连续矩阵
//src_matrices：连续矩阵对象
//返回值：无
void user_nn_matrices_delete(user_nn_list_matrix *src_matrices){
	int count = src_matrices->width * src_matrices->height;//获取总矩阵大小
	user_nn_matrix *matrix = src_matrices->matrix;//指向单个矩阵
	user_nn_matrix *matrix_next = NULL;

	while (matrix != NULL){
		matrix_next = matrix->next;
		user_nn_matrix_delete(matrix);//删除当前矩阵
		matrix = matrix_next;//更新矩阵
	}
}
//在连续矩阵中末尾添加一个矩阵 限制 连续矩阵必须是单行或者单列矩阵 
//参数
//list_matrix：矩阵链表
//end_matirx：需要添加的矩阵
//返回 成功失败
bool user_nn_matrices_add_matrix(user_nn_list_matrix *list_matrix,user_nn_matrix *end_matirx) {
	user_nn_matrix *matrix = list_matrix->matrix;//获取第一个矩阵对象
	if ((list_matrix->height != 1) && (list_matrix->width != 1)) {
		return false;//添加矩阵需要按照一维矩阵方式添加
	}
	if (matrix == NULL) {
		list_matrix->matrix = end_matirx;
	}
	else {
		while (matrix->next != NULL) {
			matrix = matrix->next;
		}
		matrix->next = end_matirx;
		if (list_matrix->width == 1) {
			list_matrix->height++;
		}
		else {
			list_matrix->width++;
		}	
	}
	return true;
}

//在连续矩阵中返回指定位置矩阵
//参数
//list_matrix：矩阵链表
//index：位置
//参数 矩阵指针
user_nn_matrix *user_nn_matrices_ext_matrix_index(user_nn_list_matrix *list_matrix, int index){
	user_nn_matrix *matrix = list_matrix->matrix;//获取第一个矩阵对象

	if (index >= (list_matrix->width * list_matrix->height) || (index < 0)){
		return NULL;
	}
	while (index--){
		matrix = matrix->next;
	}
	return matrix;
}
//在连续矩阵中返回指定位置矩阵
//参数
//list_matrix：矩阵链表
//postx：x坐标 <=list_matrix->width
//posty：y坐标 <=list_matrix->height
//参数 矩阵指针
user_nn_matrix *user_nn_matrices_ext_matrix(user_nn_list_matrix *list_matrix, int postx, int posty){
	return user_nn_matrices_ext_matrix_index(list_matrix,postx + posty * list_matrix->width);
}

//获取一个随机浮点数0-1.0之间
float user_nn_rand_0_1(void){
	//srand((unsigned)time(NULL));
	return (float)(rand() / double(RAND_MAX));
}
//数据归一化处理 返回数据 -1 - +1 范围
float user_nn_rand__1_1(float post){
	//return 1;
	return (float)((user_nn_rand_0_1() - 0.5) * 2 * sqrt(6 / post));
}
//设置单个居中里面的值 随机设置
void user_nn_matrix_rand_vaule(user_nn_matrix *src_matrix, float post){
	int total = src_matrix->height * src_matrix->width;//设置高度与宽度
	float *data = src_matrix->data;//获取内存指针
	while (total--){
		*data++ = user_nn_rand__1_1(post);//设置值
	}
}
//设置连续矩阵里面的值 随机设置
void user_nn_matrices_rand_vaule(user_nn_list_matrix *list_matrix, float post){
	user_nn_matrix *matrix = list_matrix->matrix;//获取第一个矩阵对象
	float *data = 0;
	int total = 0;

	if (matrix == NULL){
		return;
	}
	while (matrix != NULL){
		user_nn_matrix_rand_vaule(matrix, post);
		matrix = matrix->next;//继续处理下一个数据
	}
}
//按照指定上、下、左、右扩充并创建新的矩阵，并且拷贝数据到新的矩阵中
//参数
//src_matrix ：原矩阵
//above：上面界
//below：下边界
//left：左边界
//right：右边界
//返回 新的矩阵
user_nn_matrix *user_nn_matrix_expand(user_nn_matrix *src_matrix, int above, int below, int left, int right){
	user_nn_matrix *result = NULL;//
	float *src_data = src_matrix->data;//数据指针
	float *result_data;
	int count;//
	int index = 0;

	result = user_nn_matrix_create(left + src_matrix->width + right, above + src_matrix->height + below);//创建矩阵
	result_data = result->data;//取得数据指针

	result_data = result_data + (result->width * above + left);//跳过开头位置
	//拷贝数据
	for (count = 0; count < (src_matrix->width * src_matrix->height); count++){
		*result_data++ = *src_data++;
		if (++index >= src_matrix->width){
			index = 0;
			result_data = result_data + left + right;
		}
	}
	return result;
}
//
//在src_matrix矩阵指定(x,y)位置保存save_data指向的数据其中大小为width*height，返回失败或成功
//参数
//src_matrix：矩阵对象
//save_data：需要保存的矩阵
//x: 起始点 <=src_matrix->width
//y：起始点 <=src_matrix->height
//返回：成功或者失败
bool user_nn_matrix_save_array(user_nn_matrix *dest_matrix, float *src_data, int startx, int starty, int width, int height) {
	user_nn_matrix *result = NULL;
	int post_x, post_y;
	float *dest_data = dest_matrix->data;//数据指针

	if (((startx + width) > dest_matrix->width) || ((starty + height) > dest_matrix->height) || (width == 0) || (height == 0)) {
		return false;//如果超出范围那么直接返回空
	}
	dest_data += starty * dest_matrix->width + startx;//跳转到开始位置
	for (post_y = 0; post_y < height; post_y++) {
		for (post_x = 0; post_x < width; post_x++) {
			*dest_data++ = *src_data++;
		}
		dest_data += dest_matrix->width - width;
	}
	return true;
}
//
//在src_matrix矩阵指定(x,y)位置叠加save_data*alpha指向的数据其中大小为width*height，返回失败或成功
//参数
//src_matrix：矩阵对象
//save_data：需要保存的矩阵
//alpha:系数
//x: 起始点 <=src_matrix->width
//y：起始点 <=src_matrix->height
//返回：成功或者失败
bool user_nn_matrix_sum_array_mult_alpha(user_nn_matrix *dest_matrix, float *src_data, float alpha,int startx, int starty, int width, int height) {
	user_nn_matrix *result = NULL;
	int post_x, post_y;
	float *dest_data = dest_matrix->data;//数据指针

	if (((startx + width) > dest_matrix->width) || ((starty + height) > dest_matrix->height) || (width == 0) || (height == 0)) {
		return false;//如果超出范围那么直接返回空
	}
	dest_data += starty * dest_matrix->width + startx;//跳转到开始位置
	for (post_y = 0; post_y < height; post_y++) {
		for (post_x = 0; post_x < width; post_x++) {
			*dest_data++ += *src_data++ * alpha;
		}
		dest_data += dest_matrix->width - width;
	}
	return true;
}
//
//在src_matrix矩阵指定(x,y)位置保存值，返回失败或成功
//参数
//src_matrix：矩阵对象
//startx: 起始点 <=src_matrix->width
//starty：起始点 <=src_matrix->height
//vaule：需要保存的数据
//返回：成功或者失败
bool user_nn_matrix_save_float(user_nn_matrix *src_matrix, int startx, int starty, float vaule) {

	if ((startx >= src_matrix->width) || (starty >= src_matrix->height)) {
		return false;//如果超出范围那么直接返回空
	}
	src_matrix->data[starty * src_matrix->width + startx] = vaule;

	return true;
}
//
//在src_matrix矩阵指定(x,y)位置保存save_matrix矩阵，返回失败或成功
//参数
//src_matrix：矩阵对象
//save_matrix：需要保存的矩阵
//x: 起始点 <=src_matrix->width
//y：起始点 <=src_matrix->height
//返回：成功或者失败
bool user_nn_matrix_save_matrix(user_nn_matrix *src_matrix, user_nn_matrix *save_matrix, int startx, int starty) {
	user_nn_matrix *result = NULL;
	int post_x, post_y;
	float *src_data = src_matrix->data;//数据指针
	float *save_data = save_matrix->data;

	if (((startx + save_matrix->width) > src_matrix->width) || ((starty + save_matrix->height) > src_matrix->height) || (save_matrix->width == 0) || (save_matrix->height == 0)) {
		return false;//如果超出范围那么直接返回空
	}
	src_data += starty * src_matrix->width + startx;//跳转到开始位置
	for (post_y = 0; post_y < save_matrix->height; post_y++) {
		for (post_x = 0; post_x < save_matrix->width; post_x++) {
			*src_data++ = *save_data++;
		}
		src_data += src_matrix->width - save_matrix->width;
	}
	return true;
}

//
//从矩阵指定(x,y)位置截取(w,h)大小的矩阵。并且返回新截取的矩阵
//参数
//src_matrix：矩阵对象
//x: 起始点 <=src_matrix->width
//y：起始点 <=src_matrix->height
//w：横范围 <=src_matrix->width
//h：纵范围 <=src_matrix->height
//返回：成功或者失败
user_nn_matrix *user_nn_matrix_ext_matrix(user_nn_matrix *src_matrix, int startx, int starty, int width, int height){
	user_nn_matrix *result = NULL;
	int post_x, post_y, post_index;
	float *src_data = src_matrix->data;//数据指针
	float *result_data;

	if (((startx + width) > src_matrix->width) || ((starty + height) > src_matrix->height) || (width == 0) || (height == 0)){
		return NULL;//如果超出范围那么直接返回空
	}
	result = user_nn_matrix_create(width, height);//创建矩阵
	result_data = result->data;//取得数据指针

	//post_index = startx + starty* src_matrix->width;//指向通过(postx,posty)转化一维数组的位置 公式：index=横坐标+纵坐标*矩阵宽度
	for (post_y = 0; post_y < height; post_y++){
		for (post_x = 0; post_x < width; post_x++){
			post_index = (startx + post_x) + (starty + post_y)* src_matrix->width;//指向通过(postx,posty)转化一维数组的位置 公式：index=横坐标+纵坐标*矩阵宽度
			*result_data++ = src_data[post_index];//获取数据
			//printf("x:%d,y:%d,%d ", startx+i, starty+j, post_index);
		}
		//printf("\n");
	}
	return result;
}
//把连续的矩阵的数据拷贝到一个矩阵中
//参数
//src_matrix：被转化矩阵
//返回 无
void user_nn_matrices_to_matrix(user_nn_matrix *src_matrix, user_nn_list_matrix *sub_matrices){
	user_nn_matrix *sub_matrix = sub_matrices->matrix;

	int count_matrix, count_data;//
	float *src_data = src_matrix->data;//指向对象数据
	float *sub_data = NULL;

	//result_data = result->data;//获取数据指针

	for (count_matrix = 0; count_matrix < (sub_matrices->width * sub_matrices->height); count_matrix++){
		//user_nn_matrix_exc_width_height(sub_matrix);//交换矩阵 matlab同步
		sub_data = sub_matrix->data;//获取数据指针
		for (count_data = 0; count_data < (sub_matrix->width * sub_matrix->height); count_data++){
			*src_data++ = *sub_data++;//保存数据
		}
		sub_matrix = sub_matrix->next;
	}

}
//拷贝一个连续的矩阵到另外一个连续矩阵中
void user_nn_matrices_cpy_matrices(user_nn_list_matrix *src_matrices, user_nn_list_matrix *dest_matrices){
	int count = dest_matrices->width * dest_matrices->height;
	user_nn_matrix *src_m = src_matrices->matrix;
	user_nn_matrix *dest_m = dest_matrices->matrix;

	while (count--){
		user_nn_matrix_cpy_matrix(src_m, dest_m);
		src_m  = src_m->next;
		dest_m = dest_m->next;
	}
}
//
//拷贝src_matrix矩阵(x,y)起点大小为width*height的数据至dest_data,其中，返回失败或成功
//参数
//dest_data：需要保存的矩阵
//src_matrix：矩阵对象
//x: 起始点 <=src_matrix->width
//y：起始点 <=src_matrix->height
//返回：成功或者失败
bool user_nn_matrix_cpy_array(float *dest_data, user_nn_matrix *src_matrix, int startx, int starty, int width, int height){
	user_nn_matrix *result = NULL;
	int post_x, post_y;
	float *src_data = src_matrix->data;//数据指针

	if (((startx + width) > src_matrix->width) || ((starty + height) > src_matrix->height) || (width == 0) || (height == 0)) {
		return false;//如果超出范围那么直接返回空
	}
	src_data += starty * src_matrix->width + startx;//跳转到开始位置
	for (post_y = 0; post_y < height; post_y++) {
		for (post_x = 0; post_x < width; post_x++) {
			*dest_data++ = *src_data++;
		}
		src_data += src_matrix->width - width;
	}
	return true;
}
//
//拷贝src_matrix*constant矩阵(x,y)起点大小为width*height的数据至dest_data,其中，返回失败或成功
//参数
//dest_data：需要保存的矩阵
//src_matrix：矩阵对象
//x: 起始点 <=src_matrix->width
//y：起始点 <=src_matrix->height
//返回：成功或者失败
bool user_nn_matrix_cpy_array_mult_constant(float *dest_data, user_nn_matrix *src_matrix, int startx, int starty, int width, int height, float constant) {
	user_nn_matrix *result = NULL;
	int post_x, post_y;
	float *src_data = src_matrix->data;//数据指针

	if (((startx + width) > src_matrix->width) || ((starty + height) > src_matrix->height) || (width == 0) || (height == 0)) {
		return false;//如果超出范围那么直接返回空
	}
	src_data += starty * src_matrix->width + startx;//跳转到开始位置
	for (post_y = 0; post_y < height; post_y++) {
		for (post_x = 0; post_x < width; post_x++) {
			*dest_data++ = *src_data++ * constant;
		}
		src_data += src_matrix->width - width;
	}
	return true;
}
//把一个矩阵转化为连续的矩阵链表
//src_matrix：矩阵对象
//width：目标宽度
//height：目标高度
//返回 连续矩阵
void user_nn_matrix_to_matrices(user_nn_list_matrix *src_matrices, user_nn_matrix *sub_matrix){
	user_nn_matrix *matrix = src_matrices->matrix;
	int count_matrix, count_data;//
	float *result_data = sub_matrix->data;//指向对象数据
	float *src_data = NULL;

	for (count_matrix = 0; count_matrix < (src_matrices->width * src_matrices->height); count_matrix++){
		src_data = matrix->data;//获取保存数据指针
		for (count_data = 0; count_data < (matrix->width * matrix->height); count_data++){
			*src_data++ = *result_data++;//保存数据
		}
		//user_nn_matrix_exc_width_height(matrix);//交换矩阵 matlab同步
		matrix = matrix->next;
	}

}
//矩阵扩充 均值扩充
//src_matrix：
//width：扩充倍数
//height：扩充倍数
//返回 新的矩阵
user_nn_matrix *user_nn_matrix_expand_mult_constant(user_nn_matrix *src_matrix, int width, int height, float constant){
	user_nn_matrix * result = NULL;
	float *src_data = src_matrix->data;
	float *result_data = NULL;
	int x, y,wi,hi;

	result = user_nn_matrix_create(src_matrix->width * width, src_matrix->height * height);//创建扩充后的矩阵
	result_data = result->data;

	for (y = 0; y < src_matrix->height; y++){
		for (hi = 0; hi < height; hi++){
			for (x = 0; x < src_matrix->width; x++){
				for (wi = 0; wi < width; wi++){
					*result_data++ = (float)*src_data * constant;//更新数据
				}
				src_data++;
			}
			src_data = src_data - src_matrix->width;//跳转到开始位置
		}
		src_data = src_data + src_matrix->width;//跳转到结束位置
	}
	return result;
}
//设置矩阵值
//参数
//src_matrix：目标矩阵 求和值会覆盖此矩阵
//v：设置的值
//返回 无
void user_nn_matrix_memset(user_nn_matrix *save_matrix, float constant){
	int count = save_matrix->width * save_matrix->height;//获取矩阵数据大小
	float *src_data = save_matrix->data;
	while (count--){
		*src_data++ = constant;
	}
}
//设置矩阵值
//参数
//src_matrix：目标矩阵 求和值会覆盖此矩阵
//data：数据指针 大于矩阵
//返回 无
void user_nn_matrix_memcpy(user_nn_matrix *save_matrix, float *data){
	int count = save_matrix->width * save_matrix->height;//获取矩阵数据大小
	float *src_data = save_matrix->data;
	while (count--){
		*src_data++ = *data++;
	}
}
//设置矩阵值
//参数
//src_matrix：目标矩阵 求和值会覆盖此矩阵
//data：数据指针 大于矩阵
//返回 无
void user_nn_matrix_memcpy_uchar_mult_constant(user_nn_matrix *save_matrix, unsigned char *input_array, float constant){
	int count = save_matrix->width * save_matrix->height;//获取矩阵数据大小
	float *src_data = save_matrix->data;
	while (count--){
		*src_data++ = *input_array++ * constant;
	}
}
//拷贝数据至数组中
//参数
//src_matrix：目标矩阵 求和值会覆盖此矩阵
//data：数据指针 大于矩阵
//返回 无
void user_nn_matrix_uchar_memcpy(unsigned char *save_array, user_nn_matrix *src_matrix){
	int count = src_matrix->width * src_matrix->height;//获取矩阵数据大小
	float *src_data = src_matrix->data;
	while (count--){
		*save_array++ = (unsigned char)*src_data++;
	}
}
//排序矩阵
//src_matrix：目标对象 排序后会自动删除
//type：表示降序或者升序
//返回：排序后的链表
//
user_nn_matrix *user_nn_matrix_sorting(user_nn_matrix *src_matrix, sorting_type type){
	user_nn_matrix *result = NULL;
	user_nn_matrix *cpy_matrix = NULL;//临时矩阵
	int count = 0;//获取矩阵数据大小
	float *post_index = NULL;
	float *result_data = NULL;

	cpy_matrix = user_nn_matrix_cpy_create(src_matrix);//复制一个矩阵
	count = cpy_matrix->width * cpy_matrix->height;//获取矩阵数据大小

	result = user_nn_matrix_create(cpy_matrix->width,cpy_matrix->height);//创建一个新的矩阵
	result_data = result->data;//

	while(count--){
		if(type == sorting_up){
			post_index = user_nn_matrix_return_min_addr(cpy_matrix);//获取最小值的位置
			*result_data++ = *post_index;//保存最小值
			*post_index = FLT_MAX;//删除最小值 赋值最大即可		
		}else if(type == sorting_down){
			post_index = user_nn_matrix_return_max_addr(cpy_matrix);//获取最小值的位置
			*result_data++ = *post_index;//保存最小值
			*post_index = -FLT_MAX;//删除最大值 赋值最小即可		
		}
	}
	user_nn_matrix_delete(cpy_matrix);

	return result;
}

//求和矩阵与常数
//参数
//src_matrix：目标矩阵 求和值会覆盖此矩阵
//bias：偏置参数
//返回 无
void user_nn_matrix_sum_constant(user_nn_matrix *src_matrix, float constant){
	int count = src_matrix->width * src_matrix->height;//获取矩阵数据大小
	float *src_data = src_matrix->data;

	while (count--){
		*src_data++ = *src_data + constant;
	}
}
//矩阵save_matrix求和矩阵src_matrix与alpha的乘积 save_matrix=save_matrix+src_matrix*alpha
//参数
//save_matrix：目标矩阵 求和值会覆盖此矩阵
//src_matrix：被求和矩阵
//alpha：参数
//返回 无
void user_nn_matrix_sum_matrix_mult_alpha(user_nn_matrix *save_matrix, user_nn_matrix *src_matrix, float alpha){
	int count = save_matrix->width * save_matrix->height;//获取矩阵数据大小
	float *save_data = save_matrix->data;
	float *src_data = src_matrix->data;

	while (count--){
		*save_data++ = *save_data + *src_data++ * alpha;
	}
}
//求和矩阵所有数据
//参数
//src_matrix：
//bias：偏置参数
//返回 无
float user_nn_matrix_cum_element(user_nn_matrix *src_matrix){
	float result = 0;
	float *src_data = src_matrix->data;
	int count = src_matrix->width * src_matrix->height;

	while (count--){
		result += *src_data++;
	}
	return result;
}
//求和两个矩阵  save_matrix = src_matrix + sub_matrix 
//参数
//src_matrix：目标矩阵 求和值会覆盖此矩阵
//sub_matrix：被求和矩阵
//返回 无
void user_nn_matrix_cum_matrix(user_nn_matrix *save_matrix, user_nn_matrix *src_matrix, user_nn_matrix *sub_matrix){
	int count = sub_matrix->width * sub_matrix->height;//获取矩阵数据大小
	float *save_data = save_matrix->data;
	float *src_data = src_matrix->data;
	float *sub_data = sub_matrix->data;

	if ((src_matrix->width != sub_matrix->width) || (src_matrix->height != sub_matrix->height)){
		return;
	}
	while (count--){
		*save_data++ = *src_data++ + (*sub_data++);
	}
}
//求和两个矩阵  save_matrix = src_matrix + sub_matrix * alpha
//参数
//src_matrix：目标矩阵 求和值会覆盖此矩阵
//sub_matrix：被求和矩阵
//返回 无
void user_nn_matrix_cum_matrix_mult_alpha(user_nn_matrix *save_matrix, user_nn_matrix *src_matrix, user_nn_matrix *sub_matrix, float alpha) {
	int count = sub_matrix->width * sub_matrix->height;//获取矩阵数据大小
	float *save_data = save_matrix->data;
	float *src_data = src_matrix->data;
	float *sub_data = sub_matrix->data;

	if ((src_matrix->width != sub_matrix->width) || (src_matrix->height != sub_matrix->height)) {
		return;
	}
	while (count--) {
		*save_data++ = *src_data++ + ((*sub_data++) * alpha);
	}
}
//拷贝sub_matrix矩阵值到src_matrix矩阵
//参数 要求矩阵相同
//src_matrix：矩阵
//sub_matrix：矩阵
//返回 无
void user_nn_matrix_cpy_matrix(user_nn_matrix *save_matrix, user_nn_matrix *sub_matrix){
	int count = sub_matrix->width * sub_matrix->height;//获取矩阵数据大小
	float *save_data = save_matrix->data;
	float *sub_data = sub_matrix->data;

	if ((save_matrix->width != sub_matrix->width) && (save_matrix->height != sub_matrix->height)){
		return;
	}
	while (count--){
		*save_data++ = *sub_data++;
	}
}
//拷贝sub_matrix矩阵值到src_matrix矩阵 并且在给定位置进行求和参数
//参数 要求矩阵相同
//src_matrix：矩阵
//sub_matrix：矩阵
//index：给定位置
//constant：求和参数
//返回 无
void user_nn_matrix_cpy_matrix_sum_constant(user_nn_matrix *save_matrix, user_nn_matrix *sub_matrix, int index, float constant){
	int count = sub_matrix->width * sub_matrix->height;//获取矩阵数据大小
	float *save_data = save_matrix->data;
	float *sub_data = sub_matrix->data;

	for (count = 0; count<(sub_matrix->width * sub_matrix->height); count++){
		if (count == index){
			*save_data++ = *sub_data++ + constant;
		}
		else{
			*save_data++ = *sub_data++;
		}
	}
}
//两个一维矩阵相乘 两个矩阵的大小需要一样且都是一维数组类型
//参数
//src_matrix：矩阵a
//sub_matrix：矩阵b
//返回 结果
float user_nn_matrix_mult_cum_matrix(user_nn_matrix *src_matrix, user_nn_matrix *sub_matrix){
	int count = sub_matrix->width * sub_matrix->height;
	float *src_data = src_matrix->data;
	float *sub_data = sub_matrix->data;
	float result = 0;

	while (count--){
		result += (*src_data++) * (*sub_data++);
	}
	return result;
}

//矩阵乘法
//1.当矩阵A的列数等于矩阵B的行数时，A与B可以相乘。
//2.矩阵C的行数等于矩阵A的行数，C的列数等于B的列数。
//3.乘积C的第m行第n列的元素等于矩阵A的第m行的元素与矩阵B的第n列对应元素乘积之和。
//参数
//src_matrix：矩阵A
//sub_matrix：矩阵B
//返回值 无
user_nn_matrix *user_nn_matrix_mult_matrix(user_nn_matrix *src_matrix, user_nn_matrix *sub_matrix){
#ifdef WIN64
	return user_nn_matrix_mult_matrix_cuda(src_matrix, sub_matrix);
#else

	user_nn_matrix *result = NULL;//结果矩阵
	float *src_data = NULL;//
	float *sub_data = NULL;//

	float *result_data = NULL;

	int width, height,point;//矩阵列数

	if (src_matrix->width != sub_matrix->height){//矩阵乘积只有当第一个矩阵的列数=第二个矩阵的行数才有意义
		return NULL;
	}
	result = user_nn_matrix_create(sub_matrix->width, src_matrix->height);//创建新的矩阵
	result_data = result->data;//获取数据指针

	for (height = 0; height < result->height; height++){
		for (width = 0; width < result->width; width++){
			src_data = src_matrix->data + height * src_matrix->width;//指向行开头
			sub_data = sub_matrix->data + width;//指向列开头
			for (point = 0; point < sub_matrix->height; point++){
				*result_data += *src_data * *sub_data;
				sub_data += sub_matrix->width;
				src_data++;
			}
			result_data++;
		}
	}
	return result;
#endif
}
//两个矩阵进行点乘操作 对应数据进行相乘
//src_matrix：矩阵A
//sub_matrix：矩阵B
//返回值 无
void user_nn_matrix_poit_mult_matrix(user_nn_matrix *save_matrix, user_nn_matrix *src_matrix, user_nn_matrix *sub_matrix){
	int count = sub_matrix->width * sub_matrix->height;
	float *save_data = save_matrix->data;
	float *src_data = src_matrix->data;
	float *sub_data = sub_matrix->data;

	while (count--){
		*save_data++ = (*src_data++) * (*sub_data++);
	}
}
//矩阵每个元素*常数
//参数
//src_matrix：矩阵
//返回值 无
void user_nn_matrix_mult_constant(user_nn_matrix *src_matrix, float constant){
	int count = src_matrix->width * src_matrix->height;//获取矩阵数据大小
	float *src_data = src_matrix->data;

	while (count--){
		*src_data++ = (float)*src_data * constant;
	}
}
//矩阵除法
//参数
//src_matrix：矩阵
//返回值 无
void user_nn_matrix_divi_constant(user_nn_matrix *src_matrix, float constant){
	int count = src_matrix->width * src_matrix->height;//获取矩阵数据大小
	float *src_data = src_matrix->data;

	while (count--){
		*src_data++ = (float)*src_data / constant;
	}
}
//将矩阵旋转180°
//参数
//output：输出图像
//input：输入图像
//返回 成功或失败
//
user_nn_matrix *user_nn_matrix_rotate180(user_nn_matrix *src_matrix){
	user_nn_matrix *result = NULL;
	int count = src_matrix->width * src_matrix->height;
	float *input_data = src_matrix->data;
	float *result_data;

	result = user_nn_matrix_create(src_matrix->width, src_matrix->height);
	result_data = result->data;//取得数据指针

	while (count--){
		*result_data++ = input_data[count];//直接首尾进行交换
	}
	return result;
}
//对矩阵进行pooling操作 此操作针对cnn使用
//参数
//save_matrix:池化后的矩阵对象
//src_matrix：池化对象
//kernel_matrix：池化矩阵大小
//返回 池化后的矩阵
void user_nn_matrix_pooling(user_nn_matrix *save_matrix, user_nn_matrix *src_matrix, user_nn_matrix *kernel_matrix){
	user_nn_matrix *temp_matrix = NULL;//创建一个零时保存被卷积数据的矩阵

	int start_x, start_y;//这里保存开始的x和y位置
	float *save_data = save_matrix->data;//保存数据的指针

	for (start_y = 0; start_y < (src_matrix->height / kernel_matrix->height); start_y++){//纵轴移动一次 横轴需要移动整个行
		for (start_x = 0; start_x < (src_matrix->width / kernel_matrix->width); start_x++){
			temp_matrix = user_nn_matrix_ext_matrix(src_matrix, start_x * kernel_matrix->width, start_y * kernel_matrix->height, kernel_matrix->width, kernel_matrix->height);//从被卷积对象中获取卷积数据 数据大小为模板大小
			*save_data++ = user_nn_matrix_mult_cum_matrix(temp_matrix, kernel_matrix);//乘积累加
			user_nn_matrix_delete(temp_matrix);//删除矩阵
		}
	}

}

//对矩阵进行卷积操作
//参数
//src_matrix：卷积对象
//kernel_matrix：卷积核
//type：卷积类型 full same valid 等
//返回 无
user_nn_matrix *user_nn_matrix_conv2(user_nn_matrix *src_matrix, user_nn_matrix *kernel_matrix, user_nn_conv2_type type){
	user_nn_matrix *conv_matrix = NULL;//被卷积对象
	user_nn_matrix *mode_matrix = NULL;//创建一个零时保存卷积核大小的矩阵 用于矩阵翻转180°
	user_nn_matrix *temp_matrix = NULL;//缓存
	user_nn_matrix *full_matrix = NULL;//缓存
	user_nn_matrix *same_matrix = NULL;//缓存
	user_nn_matrix *result = NULL;//结果矩阵
	
	int start_x, start_y;//这里保存开始的x和y位置
	float *result_data = NULL;

	if (type == u_nn_conv2_type_valid){
		result = user_nn_matrix_create(src_matrix->width - kernel_matrix->width + 1, src_matrix->height - kernel_matrix->height + 1);//创建一个用户返回结果的矩阵
		result_data = result->data;//指向输出矩阵的数据指针
		conv_matrix = src_matrix;//卷积对象指针
	}else if (type == u_nn_conv2_type_full){
		full_matrix = user_nn_matrix_expand(src_matrix, kernel_matrix->height - 1, kernel_matrix->height - 1, kernel_matrix->width - 1, kernel_matrix->width - 1);//边界扩展
		result = user_nn_matrix_create(full_matrix->width - kernel_matrix->width + 1, full_matrix->height - kernel_matrix->height + 1);//创建一个用户返回结果的矩阵
		result_data = result->data;//指向输出矩阵的数据指针
		conv_matrix = full_matrix;//卷积对象指针
	}
	else if (type == u_nn_conv2_type_same){
		same_matrix = user_nn_matrix_expand(src_matrix, (kernel_matrix->height - 1) / 2, (kernel_matrix->height) / 2, (kernel_matrix->width - 1) / 2, (kernel_matrix->width) / 2);//扩充矩阵 返回一个新的矩阵
		result = user_nn_matrix_create(same_matrix->width - kernel_matrix->width + 1, same_matrix->height - kernel_matrix->height + 1);//创建一个用户返回结果的矩阵
		result_data = result->data;//指向输出矩阵的数据指针
		conv_matrix = same_matrix;//卷积对象指针
	}
	else{}
	mode_matrix = user_nn_matrix_rotate180(kernel_matrix);//模板翻转180°
	for (start_y = 0; start_y < result->height; start_y++){//纵轴移动一次 横轴需要移动整个行
		for (start_x = 0; start_x < result->width; start_x++){
			temp_matrix = user_nn_matrix_ext_matrix(conv_matrix, start_x, start_y, kernel_matrix->width, kernel_matrix->height);//从被卷积对象中获取卷积数据 数据大小为模板大小
			*result_data++ = user_nn_matrix_mult_cum_matrix(temp_matrix, mode_matrix);//卷积运算 
			user_nn_matrix_delete(temp_matrix);//删除矩阵
		}
	}
	user_nn_matrix_delete(mode_matrix);
	user_nn_matrix_delete(full_matrix);//删除矩阵
	return result;
}

//计算矩阵的均方误差
//src_matrix：倍计算的矩阵
//返回 损失函数
float user_nn_matrix_get_rms(user_nn_matrix *src_matrix) {
	//user_nn_matrix_poit_mult_matrix(error_matrix_temp, error_matrix_temp, error_matrix_temp);//矩阵乘法
	//*loss_vaule = *loss_vaule + user_nn_matrix_cum_element(error_matrix_temp) / (error_matrix_temp->height*error_matrix_temp->width);//计算损失函数
	float loss = 0.0f;
	float *src_data = src_matrix->data;
	int count = src_matrix->width * src_matrix->height;
	while (count--) {
		loss = *src_data * *src_data;
		src_data++;
	}
	return float(loss / (src_matrix->width*src_matrix->height));
}

//矩阵复制
//dest：原矩阵
//m：复制垂直排列个数
//n：复制水平排列个数
//返回 新的矩阵
user_nn_matrix *user_nn_matrix_repmat(user_nn_matrix *dest, int m, int n) {
	user_nn_matrix *result = user_nn_matrix_create(dest->width * n, dest->height * m);
	int loca_x = 0;
	int loca_y = 0;

	for (loca_y = 0; loca_y < m; loca_y++) {
		for (loca_x = 0; loca_x < n; loca_x++) {
			user_nn_matrix_save_array(result, dest->data, loca_x*dest->width, loca_y*dest->height, dest->width, dest->height);
		}
	}
	return result;
}
//把一个矩阵对角线设置为1 目前仅仅支持方阵
//dest：目标矩阵
//返回 坐标矩阵
void user_nn_matrix_eye(user_nn_matrix *dest) {
	int count = 0;
	for (count = 0; count < dest->width; count++) {
		*user_nn_matrix_ext_value(dest, count, count) = 1.0f;
	}
}

//求解givens 旋转后的值
//x:x值
//y:y值
//参考matlab进行编写
//返回矩阵
user_nn_matrix *user_nn_givens(float x, float y) {
	user_nn_matrix *result = user_nn_matrix_create(2, 2);//创建矩阵2x2
	float c = 0.0f;
	float s = 0.0f;
	float nrm = 0.0f;
	float absx = 0.0f;

	absx = abs(x);
	if (absx == 0.0f) {
		c = 0;
		s = 1;
	}
	else {
		/*
		//matlab
		nrm = (float)hypot(x,y);
		c = absx / nrm;
		s = (float)(x / absx)*(y/ nrm);
		*/
		//维基百科
		nrm = (float)hypot(x, y);
		c = x / nrm;
		s = y / nrm;
	}
	result->data[0] = c; result->data[1] = s;
	result->data[2] = -s; result->data[3] = c;

	return result;
}
//householder reflection方式求解矩阵的QR值
//dest：数据矩阵
//coordinate：坐标数据
//参考：https://en.wikipedia.org/wiki/Householder_transformation
//返回 QR矩阵
user_nn_list_matrix *user_nn_householder_qr(user_nn_matrix *dest) {

	user_nn_list_matrix *result = user_nn_matrices_create(2, 1, dest->width, dest->height);//创建两个连续矩阵第一个保存Q 第二个保存R
	user_nn_matrix *matrix_G = user_nn_matrix_create(dest->width, dest->width);//创建矩阵
	user_nn_matrix *matrix_Q = result->matrix;//第一个矩阵保存Q
	user_nn_matrix *matrix_R = result->matrix->next;//第二个矩阵保存R
	user_nn_matrix *matrix_m = NULL;
	user_nn_matrix *matrix_e = NULL;
	user_nn_matrix *matrix_c = NULL;
	user_nn_matrix *matrix_temp = NULL;

	int index = 0;
	float norm_m = 0.0f;

	user_nn_matrix_cpy_matrix(matrix_R, dest);//拷贝到matrix_R中
	user_nn_matrix_eye(matrix_Q);//设置Q的对角线为1

	for (index = 0; index < dest->height - 1; index++) {
		user_nn_matrix_memset(matrix_G, 0);//设置G为0
		user_nn_matrix_eye(matrix_G);//设置G的对角线为1

		matrix_m = user_nn_matrix_ext_matrix(matrix_R, index, index, 1, matrix_R->height - index);//提取矩阵
		matrix_e = user_nn_matrix_create(matrix_m->width, matrix_m->height);//重新创建矩阵
		matrix_e->data[0] = user_nn_matrix_norm(matrix_m);//求取范数
		user_nn_matrix_cum_matrix_mult_alpha(matrix_m, matrix_m, matrix_e,-1.0f);//计算矩阵之差
		user_nn_matrix_divi_constant(matrix_m, user_nn_matrix_norm(matrix_m));//除法计算
		matrix_c = user_nn_matrix_outer(matrix_m, matrix_m);//求解矩阵outer
		user_nn_matrix_sum_array_mult_alpha(matrix_G, matrix_c->data,-2.0f, index, index, matrix_c->width, matrix_c->height);//叠加数据
		matrix_temp = user_nn_matrix_mult_matrix(matrix_G, matrix_R);//矩阵乘法
		user_nn_matrix_cpy_matrix(matrix_R, matrix_temp);//拷贝到matrix_R中
		user_nn_matrix_delete(matrix_temp);//删除矩阵
		//Q=G1*G2*..Gn  Q就是特征向量Q
		matrix_temp = user_nn_matrix_mult_matrix(matrix_Q, matrix_G);//矩阵乘法
		user_nn_matrix_cpy_matrix(matrix_Q, matrix_temp);//拷贝到matrix_Q中
		user_nn_matrix_delete(matrix_temp);//删除矩阵

		user_nn_matrix_delete(matrix_m);//删除矩阵
		user_nn_matrix_delete(matrix_e);//删除矩阵
		user_nn_matrix_delete(matrix_c);//删除矩阵
	}
	user_nn_matrix_delete(matrix_G);//删除矩阵
	return result;
}
//givens rotation方式求解矩阵的QR值
//dest：数据矩阵
//coordinate：坐标数据
//参考：https://en.wikipedia.org/wiki/Givens_rotation
//返回 QR矩阵
user_nn_list_matrix *user_nn_givens_qr(user_nn_matrix *dest) {
	user_nn_list_matrix *result = user_nn_matrices_create(2, 1, dest->width, dest->height);//创建两个连续矩阵第一个保存Q 第二个保存R
	user_nn_matrix *matrix_G = user_nn_matrix_create(dest->width, dest->height);//创建矩阵
	user_nn_matrix *matrix_Q = result->matrix;//第一个矩阵保存Q
	user_nn_matrix *matrix_R = result->matrix->next;//第二个矩阵保存R
	user_nn_matrix *matrix_temp = NULL;//临时矩阵
	user_nn_matrix *triangle_axis = NULL;//左三角坐标数据
	user_nn_matrix *givens_vaule = NULL;

	int posit_x = 0;
	float *axis_x = NULL;
	float *axis_y = NULL;
	float givens_r = 0.0f;
	float givens_c = 0.0f;
	float givens_s = 0.0f;

	user_nn_matrix_cpy_matrix(matrix_R, dest);//拷贝到matrix_R中
	user_nn_matrix_eye(matrix_Q);//设置Q的对角线为1

	triangle_axis = user_nn_tril_indices(dest->width, dest->height, 0.0f);//求取坐标数据
	axis_x = triangle_axis->data;//获取X坐标
	axis_y = triangle_axis->data + triangle_axis->width;//获取Y坐标
	for (posit_x = 0; posit_x < triangle_axis->width; posit_x++) {	//循环所有坐标
		if (*user_nn_matrix_ext_value(matrix_R, (int)*axis_x, (int)*axis_y) != 0) {//判断目标坐标数据是否为0 如果是0那么不用计算
			user_nn_matrix_memset(matrix_G, 0);//设置G为0
			user_nn_matrix_eye(matrix_G);//设置G的对角线为1

			givens_vaule = user_nn_givens(*user_nn_matrix_ext_value(matrix_R, (int)*axis_x, (int)*axis_x), *user_nn_matrix_ext_value(matrix_R, (int)*axis_x, (int)*axis_y));//求解givens旋转后的值
			*user_nn_matrix_ext_value(matrix_G, (int)*axis_x, (int)*axis_x) = givens_vaule->data[0];
			*user_nn_matrix_ext_value(matrix_G, (int)*axis_y, (int)*axis_x) = givens_vaule->data[1];
			*user_nn_matrix_ext_value(matrix_G, (int)*axis_x, (int)*axis_y) = givens_vaule->data[2];
			*user_nn_matrix_ext_value(matrix_G, (int)*axis_y, (int)*axis_y) = givens_vaule->data[3];
			user_nn_matrix_delete(givens_vaule);//删除矩阵
												//G1*A1=A2 G2*A2=A3 ... Gn-1*An-1=An  An就是特征向量R
			matrix_temp = user_nn_matrix_mult_matrix(matrix_G, matrix_R);//矩阵乘法
			user_nn_matrix_cpy_matrix(matrix_R, matrix_temp);//拷贝到matrix_R中
			user_nn_matrix_delete(matrix_temp);//删除矩阵
											   //Q=G1*G2*..Gn  Q就是特征向量Q
			user_nn_matrix_transpose(matrix_G);//进行转置
			matrix_temp = user_nn_matrix_mult_matrix(matrix_Q, matrix_G);//矩阵乘法
			user_nn_matrix_cpy_matrix(matrix_Q, matrix_temp);//拷贝到matrix_Q中
			user_nn_matrix_delete(matrix_temp);//删除矩阵
		}
		axis_x++;
		axis_y++;
	}
	user_nn_matrix_delete(matrix_G);//删除矩阵
	user_nn_matrix_delete(triangle_axis);//删除矩阵

	return result;
}
//从目标矩阵提取矩阵特征向量值
//dest：目标矩阵
//coordinate：矩阵坐标
//iter：迭代次数
//返回：特征值、特征向量
user_nn_list_matrix *user_nn_eigs(user_nn_matrix *dest, float epsilon, eigs_type type) {
	user_nn_list_matrix *result = user_nn_matrices_create(2, 1, dest->width, dest->height);//创建两个连续矩阵第一个保存LATENT 第二个保存COEFF
	user_nn_list_matrix *QR_list = NULL;
	user_nn_matrix *matrix_latent = NULL;
	user_nn_matrix *matrix_coeff = NULL;
	user_nn_matrix *matrix_tmp = NULL;
	float n_latent_trace = 0.0f;//保存当前特征值和
	float o_latent_trace = 0.0f;//保存历史特征值和

	matrix_latent = result->matrix;//LATENT matrix_latent
	matrix_coeff = result->matrix->next;//COEFF matrix_coeff

	user_nn_matrix_cpy_matrix(matrix_latent, dest);//拷贝数据
	user_nn_matrix_eye(matrix_coeff);//设置G的对角线为1
	for (;;) {
		if (type == qr_givens) {
			QR_list = user_nn_givens_qr(matrix_latent);//采用givens计算一次QR
		}
		else if(type == qr_householder){
			QR_list = user_nn_householder_qr(matrix_latent);//采用householder计算一次QR值
		}
		matrix_tmp = user_nn_matrix_mult_matrix(QR_list->matrix->next, QR_list->matrix);//继续迭代，需要计算新的矩阵
		user_nn_matrix_cpy_matrix(matrix_latent, matrix_tmp);//更新数据
		user_nn_matrix_delete(matrix_tmp);
		//
		matrix_tmp = user_nn_matrix_mult_matrix(matrix_coeff, QR_list->matrix);//求取特征值
		user_nn_matrix_cpy_matrix(matrix_coeff, matrix_tmp);//更新数据
		user_nn_matrix_delete(matrix_tmp);

		user_nn_matrices_delete(QR_list);//删除链表矩阵

		//对角线特征值不在变化那么迭代结束
		n_latent_trace = user_nn_matrix_trace(matrix_latent);//求和特征值的对角线和
		if (abs(n_latent_trace - o_latent_trace) <= epsilon) {
			break;
		}
		else {
			o_latent_trace = n_latent_trace;
		}
	}

	return result;
}
//求解矩阵的平均值
//src_matrix：原始矩阵
//返回平均值矩阵
user_nn_matrix *user_nn_matrix_mean(user_nn_matrix *src_matrix) {
	int height_index = 0;
	user_nn_matrix *matrix_mean = user_nn_matrix_create(src_matrix->width, 1);//创建一个保存平均值的矩阵
	user_nn_matrix *matrix_temp = user_nn_matrix_create(src_matrix->width, 1);//

	for (height_index = 0; height_index < src_matrix->height; height_index++) {
		user_nn_matrix_cpy_array(matrix_temp->data, src_matrix, 0, height_index, matrix_temp->width, matrix_temp->height);//拷贝一行数据
		user_nn_matrix_sum_matrix_mult_alpha(matrix_mean, matrix_temp, 1.0f);//求和矩阵
	}
	user_nn_matrix_divi_constant(matrix_mean, (float)src_matrix->height);//求平均数
	user_nn_matrix_delete(matrix_temp);//删除矩阵

	return matrix_mean;
}
//求解协方差矩阵
//src_matrix：需要求解的矩阵
//返回求解后的结果
user_nn_matrix *user_nn_matrix_cov(user_nn_matrix *src_matrix) {
	user_nn_matrix *result = NULL;
	user_nn_matrix *src_matrix_s = NULL;//临时矩阵
	user_nn_matrix *src_matrix_t = NULL;//转置矩阵
	user_nn_matrix *matrix_mean = user_nn_matrix_create(src_matrix->width, 1);//创建一个保存平均值的矩阵

	int height_index = 0;
	matrix_mean = user_nn_matrix_mean(src_matrix);//求取平均值
	src_matrix_s = user_nn_matrix_cpy_create(src_matrix);//复制矩阵

	for (height_index = 0; height_index < src_matrix_s->height; height_index++) {
		user_nn_matrix_sum_array_mult_alpha(src_matrix_s, matrix_mean->data, -1.0f, 0, height_index, matrix_mean->width, matrix_mean->height);//减去平均值
	}
	src_matrix_t = user_nn_matrix_cpy_create(src_matrix_s);//创建保存转置矩阵AT的矩阵
	user_nn_matrix_transpose(src_matrix_t);//进行转置
	result = user_nn_matrix_mult_matrix(src_matrix_t, src_matrix_s);//矩阵相乘
	user_nn_matrix_divi_constant(result, (float)(result->width - 1));//除以n-1

	user_nn_matrix_delete(matrix_mean);//删除矩阵
	user_nn_matrix_delete(src_matrix_s);//删除矩阵
	user_nn_matrix_delete(src_matrix_t);//删除矩阵

	return result;
}

//求取方阵的三角矩阵坐标 返回左下角所有坐标不包含中线
//width：矩阵宽度
//height：矩阵高度
//details：旋转角度
//返回 坐标矩阵
user_nn_matrix *user_nn_tril_indices(int width, int height, float details) {
	user_nn_matrix *result = NULL;
	float *result_x = NULL;
	float *result_y = NULL;
	float posit_m = 0.0f;
	int posit_x = 0;
	int posit_y = 0;

	result = user_nn_matrix_create((width*(width - 1) / 2), 2);//等差数列求和得到总共多少个x坐标点
	result_x = result->data;
	result_y = result->data + result->width;
	for (posit_y = 0; posit_y < height; posit_y++) {
		posit_m = (float)(posit_y * width) / height;//y=u*x直线函数
		if (modf(posit_m, &posit_m) > 0.0f) {
			posit_m += 1;
			if (posit_m >= width) { posit_m -= 1; }//不能超过最大值坐标
		}
		for (posit_x = 0; posit_x < posit_m; posit_x++) {
			//printf("\n(%d,%d)", post_x, post_y);//取斜线下部左三角矩阵
			*result_y++ = (float)posit_y;
			*result_x++ = (float)posit_x;
		}
	}
	return result;
}
//求和对角线元素
//src_matrix:矩阵 必须是方阵
//返回求和值
float user_nn_matrix_trace(user_nn_matrix *src_matrix) {
	int count = 0;
	float result = 0.0f;

	for (count = 0; count < src_matrix->width; count++) {
		result += *user_nn_matrix_ext_value(src_matrix, count, count);
	}
	return result;
}
//返回对角线元素
//src_matrix:矩阵 必须是方阵
//返回结果矩阵
user_nn_matrix *user_nn_matrix_diag(user_nn_matrix *src_matrix) {
	user_nn_matrix *result = NULL;
	float *result_data = NULL;
	int count = 0;

	result = user_nn_matrix_create(src_matrix->width,1);
	result_data = result->data;
	for (count = 0; count < src_matrix->width; count++) {
		*result_data++ = *user_nn_matrix_ext_value(src_matrix, count, count);
	}

	return result;
}
//求解矩阵的范数 2D方式
//src_matrix:矩阵 必须是方阵
//返回结果矩阵
float user_nn_matrix_norm(user_nn_matrix *src_matrix) {
	int count = src_matrix->width*src_matrix->height;
	float *src_matrix_data = src_matrix->data;
	float result = 0.0f;

	while (count--) {
		result += *src_matrix_data * *src_matrix_data;//平方和开根号
		src_matrix_data++;
	}
	result = sqrt(result);

	return result;
}
//计算两个矩阵的outer值
//src_matrix:矩阵A 
//sub_matrix:矩阵B
//返回结果矩阵
user_nn_matrix *user_nn_matrix_outer(user_nn_matrix *src_matrix, user_nn_matrix *sub_matrix) {
	user_nn_matrix *result = NULL;
	float *src_matrix_data = NULL;
	float *sub_matrix_data = NULL;
	float *result_data = NULL;
	int height = 0;
	int width = 0;

	result = user_nn_matrix_create(sub_matrix->width*sub_matrix->height,src_matrix->width*src_matrix->height);
	result_data = result->data;

	src_matrix_data = src_matrix->data;
	for (height = 0; height < src_matrix->width*src_matrix->height; height++) {
		sub_matrix_data = sub_matrix->data;
		for (width = 0; width < sub_matrix->width*sub_matrix->height; width++) {
			*result_data++ = *src_matrix_data * *sub_matrix_data++;
		}
		src_matrix_data++;
	}

	return result;
}
//通过对角矩阵的元素来从新排序新的矩阵
//src_matrix：矩阵
//diag：排序列表
//epsilon：排序值
//返回 新的值特征值
user_nn_matrix *user_nn_matrix_cut_vector(user_nn_matrix *src_matrix, user_nn_matrix *diag_matrix, float epsilon) {
	user_nn_matrix *result = NULL;
	
	float total_diag_vaule = 0.0f;
	float target_diag_vaule = 0.0f;
	float *diag_vaule_data = NULL;
	int total_width = 0;
		
	total_diag_vaule = user_nn_matrix_cum_element(diag_matrix);
	diag_vaule_data = diag_matrix->data;

	for (;;) {//求出需要多少列
		total_width++;
		target_diag_vaule += *diag_vaule_data++;
		if (float(target_diag_vaule / total_diag_vaule) >= epsilon) {
			break;//跳出
		}
	}
	result = user_nn_matrix_ext_matrix(src_matrix,0,0, total_width, src_matrix->height);
	return result;
}
//打印矩阵数据
//参数
//list_matrix：矩阵数据
//返回 无
void user_nn_matrix_printf(FILE *debug_file, user_nn_matrix *src_matrix){
	int width, height;
	float *input_data = src_matrix->data;
	//FILE *debug_file = NULL;
	//debug_file = fopen("debug.txt", "w+");
	printf("matrix: \n        width:%d,height:%d\n\n", src_matrix->width, src_matrix->height);
	if (debug_file != NULL)
	fprintf(debug_file,"matrix: \n        width:%d,height:%d\n\n", src_matrix->width, src_matrix->height);//保存数据

	for (height = 0; height < src_matrix->height; height++){
		for (width = 0; width < src_matrix->width; width++){
			//printf("%-10.6f ", *input_data);
			if (*input_data == FLT_MAX) {
				printf("%s ", "   max   ");
			}
			else {
				printf("%-10.6f ", *input_data);
			}
			if (debug_file != NULL) {
				if (*input_data == FLT_MAX) {
					fprintf(debug_file, "%s", "   max   ");
				}
				else {
					fprintf(debug_file, "%-10.6f ", *input_data);
				}
			}
			
			input_data++;
		}
		printf("\n\n");
		if (debug_file != NULL)
		fprintf(debug_file, "\n");
	}
	//fclose(debug_file);
	if (debug_file != NULL)
	fflush(debug_file);
}

//打印链表矩阵数据
//参数
//list_matrix：矩阵数据
//返回 无
void user_nn_matrices_printf(FILE *debug_file,char *title, user_nn_list_matrix *src_matrix){
	int width, height;
	int count = src_matrix->width * src_matrix->height;

	printf("%s matrices: \n        width:%d,height:%d\n\n\n", title, src_matrix->width, src_matrix->height);
	if (debug_file != NULL)
	fprintf(debug_file, "%s matrices: \n        width:%d,height:%d\n\n\n", title, src_matrix->width, src_matrix->height);

	for (height = 0; height < src_matrix->height; height++){
		for (width = 0; width < src_matrix->width; width++){
			user_nn_matrix_printf(debug_file, user_nn_matrices_ext_matrix(src_matrix, width, height));
		}
		printf("\n\n");
		if (debug_file != NULL)
		fprintf(debug_file,"\n\n");
	}
}


/*
	//把一个矩阵数据拷贝到另外一个矩阵中
	user_nn_matrix *matrix = NULL;
	user_nn_matrix *dest = NULL;

	matrix = user_nn_matrix_create(6, 6);//创建3*3大小的二维矩阵
	dest   = user_nn_matrix_create(2, 2);//创建2*2大小的二维矩阵
	user_nn_matrix_rand_vaule(matrix,1);

	user_nn_matrix_printf(NULL, matrix);//打印矩阵
	bool is_success = user_nn_matrix_save_matrix(matrix, dest, 1, 4);
	printf("\n%s\n", is_success==true?"true":"false");

	user_nn_matrix_printf(NULL, matrix);//打印矩阵

	getchar();
	return 0;

*/
/*
	//把一个内存数据拷贝到另外一个矩阵中 按照规定参数
	user_nn_matrix *matrix = NULL;
	user_nn_matrix *dest = NULL;

	matrix = user_nn_matrix_create(6, 6);//创建3*3大小的二维矩阵
	dest = user_nn_matrix_create(2, 2);//创建2*2大小的二维矩阵
	user_nn_matrix_rand_vaule(matrix, 1);

	user_nn_matrix_printf(NULL, matrix);//打印矩阵
	bool is_success = user_nn_matrix_save_array(matrix, dest->data, 2, 3, dest->width, dest->height);
	printf("\n%s\n", is_success == true ? "true" : "false");

	user_nn_matrix_printf(NULL, matrix);//打印矩阵

	getchar();
	return 0;
*/

/* 池化矩阵
	user_nn_matrix *src_matrix = NULL;
	user_nn_matrix *sub_matrix = NULL;
	user_nn_matrix *res_matrix = NULL;
	float *result = NULL;


	src_matrix = user_nn_matrix_create(28, 28);
	sub_matrix = user_nn_matrix_create(2, 2);

	user_nn_matrix_memset(src_matrix, 2.5);//设置矩阵值
	user_nn_matrix_memset(sub_matrix, 0.25);//设置矩阵值

	result = user_nn_matrix_ext_value(src_matrix, 27, 27);
	*result = 1;

	res_matrix = user_nn_matrix_pool(src_matrix, sub_matrix);
	if (res_matrix != NULL){
	user_nn_matrix_printf(res_matrix);//打印矩阵
	}
	else{
	printf("null\n");
	}
*/
/* 矩阵卷积测试
	user_nn_matrix *src_matrix = NULL;
	user_nn_matrix *sub_matrix = NULL;
	user_nn_matrix *res_matrix = NULL;

	src_matrix = user_nn_matrix_create(28, 28);
	sub_matrix = user_nn_matrix_create(5, 5);

	user_nn_matrix_memset(src_matrix, 2.5);//设置矩阵值
	user_nn_matrix_memset(sub_matrix, 1.0);//设置矩阵值

	res_matrix = user_nn_matrix_conv2(src_matrix, sub_matrix, u_nn_conv2_type_valid);
	if (res_matrix != NULL){
	user_nn_matrix_printf(res_matrix);//打印矩阵
	}
	else{
	printf("null\n");
	}
*/
/* 二维矩阵 提取
	user_nn_list_matrix *list = NULL;
	user_nn_matrix *dest = NULL;

	list = user_nn_matrices_create(2, 2, 1, 1);//创建2*2个1*1大小的二维矩阵
	user_nn_matrix_memset(list->matrix, 1);
	user_nn_matrix_memset(list->matrix->next, 2);
	user_nn_matrix_memset(list->matrix->next->next, 3);
	user_nn_matrix_memset(list->matrix->next->next->next, 4);

	dest = user_nn_matrices_ext_matrix(list, 1, 0);//获取其中一个矩阵

	if (dest != NULL){
	user_nn_matrix_printf(NULL,dest);//打印矩阵
	}
	else{
	printf("null\n");
	}
*/
/* 矩阵截取测试
	user_nn_matrix *matrix = NULL;
	user_nn_matrix *dest = NULL;

	matrix = user_nn_matrix_create(2, 2);//创建2*2大小的二维矩阵
	matrix->data[0] = 1.0;
	matrix->data[1] = 2.0;
	matrix->data[2] = 3.0;
	matrix->data[3] = 4.0;

	dest = user_nn_matrix_ext_matrix(matrix, 1, 1, 1, 1);//截取矩阵

	if (dest != NULL){
	user_nn_matrix_printf(NULL,dest);//打印矩阵
	}
	else{
	printf("null\n");
	}
*/


/*获取矩阵中的一个值
	user_nn_matrix *matrix = NULL;
	float *p;

	matrix = user_nn_matrix_create(2, 2);//创建2*2大小的二维矩阵
	matrix->data[0] = 1.0;
	matrix->data[1] = 2.0;
	matrix->data[2] = 3.0;
	matrix->data[3] = 4.0;

	p = user_nn_matrix_ext_value(matrix, 1, 1);//获取矩阵值
	if (p != NULL)
	printf("%f \n", *p);
*/
/*偏置参数测试
	user_nn_list_biases *biases = NULL;
	user_nn_bias *dest = NULL;

	biases = user_nn_biases_create(4);

	biases->bias->bias = 1.0;
	biases->bias->next->bias = 2.0;
	biases->bias->next->next->bias = 3.0;
	biases->bias->next->next->next->bias = 4.0;

	dest = user_nn_biases_ext_bias(biases,0);
	if (dest != NULL)
	printf("%f \n", dest->bias);
*/
/* 矩阵乘法
	user_nn_matrix *src_matrix = NULL;
	user_nn_matrix *sub_matrix = NULL;
	user_nn_matrix *res_matrix = NULL;

	src_matrix = user_nn_matrix_create(1, 1);
	sub_matrix = user_nn_matrix_create(1, 1);

	user_nn_matrix_memset(src_matrix, 2.5);//设置矩阵值
	user_nn_matrix_memset(sub_matrix, 1.0);//设置矩阵值

	res_matrix = user_nn_matrix_mult_matrix(src_matrix, sub_matrix);//矩阵相乘
	if (res_matrix != NULL){
	user_nn_matrix_printf(res_matrix);//打印矩阵
	}
	else{
	printf("null\n");
	}
*/
/*单个矩阵转化为 高度为1的连续链表矩阵
	user_nn_list_matrix *list = NULL;
	user_nn_matrix *dest = NULL;

	dest = user_nn_matrix_create(2,2);
	dest->data[0] = 1.0;
	dest->data[1] = 2.0;
	dest->data[2] = 3.0;
	dest->data[3] = 4.0;

	list = user_nn_matrix_to_matrices(dest,1,1);//获取其中一个矩阵

	if (list != NULL){
	user_nn_matrix_printf(list->matrix);//打印矩阵
	user_nn_matrix_printf(list->matrix->next);//打印矩阵
	user_nn_matrix_printf(list->matrix->next->next);//打印矩阵
	user_nn_matrix_printf(list->matrix->next->next->next);//打印矩阵
	}
	else{
	printf("null\n");
	}
*/
/*链表矩阵转化为高度为1的单个矩阵
	user_nn_list_matrix *list = NULL;
	user_nn_matrix *dest = NULL;

	list = user_nn_matrices_create(5, 2, 1, 1);//创建2*2个1*1大小的二维矩阵
	user_nn_matrix_memset(list->matrix, 1);
	user_nn_matrix_memset(list->matrix->next, 2);
	user_nn_matrix_memset(list->matrix->next->next, 3);
	user_nn_matrix_memset(list->matrix->next->next->next, 4);

	dest = user_nn_matrices_to_matrix(list);//获取其中一个矩阵

	if (dest != NULL){
	user_nn_matrix_printf(NULL,dest);//打印矩阵
	}
	else{
	printf("null\n");
	}
*/
/*链表偏置参数转矩阵测试
	user_nn_list_biases *list = NULL;
	user_nn_matrix *dest = NULL;

	list = user_nn_biases_create(1);

	list->bias->bias = 1.0;
	list->bias->next->bias = 2.0;
	list->bias->next->next->bias = 3.0;
	list->bias->next->next->next->bias = 4.0;
	list->bias->next->next->next->next->bias = 5.0;

	dest = user_nn_biases_to_matrix(list);

	if (dest != NULL){
	user_nn_matrix_printf(NULL,dest);//打印矩阵
	}
	else{
	printf("null\n");
	}
*/
/*矩阵均值扩充
	user_nn_matrix *matrix = NULL;
	user_nn_matrix *dest = NULL;

	matrix = user_nn_matrix_create(2, 2);//创建2*2大小的二维矩阵
	matrix->data[0] = 1.0;
	matrix->data[1] = 2.0;
	matrix->data[2] = 3.0;
	matrix->data[3] = 4.0;

	dest = user_nn_matrix_expand_matrix(matrix, 2, 3);//

	if (dest != NULL){
	user_nn_matrix_printf(NULL,dest);//打印矩阵
	}
	else{
	printf("null\n");
	}
*/
/*矩阵扩充 边框扩充
	user_nn_matrix *matrix = NULL;
	user_nn_matrix *dest = NULL;

	matrix = user_nn_matrix_create(1, 1);//创建2*2大小的二维矩阵
	matrix->data[0] = 1.0;
	//matrix->data[1] = 2.0;
	//matrix->data[2] = 3.0;
	//matrix->data[3] = 4.0;

	dest = user_nn_matrix_expand(matrix, 1, 1);//

	if (dest != NULL){
	user_nn_matrix_printf(NULL,dest);//打印矩阵
	}
	else{
	printf("null\n");
	}
*/
/*连续矩阵拷贝
	user_nn_list_matrix *list = NULL;
	user_nn_list_matrix *dest = NULL;

	list = user_nn_matrices_create(5, 2, 1, 1);//创建2*2个1*1大小的二维矩阵
	dest = user_nn_matrices_create(5, 2, 1, 1);//创建2*2个1*1大小的二维矩阵

	user_nn_matrix_memset(list->matrix, 1);
	user_nn_matrix_memset(list->matrix->next, 2);
	user_nn_matrix_memset(list->matrix->next->next, 3);
	user_nn_matrix_memset(list->matrix->next->next->next, 4);

	user_nn_matrices_cpy_matrices(dest, list);//获取其中一个矩阵

	if (dest != NULL){
	user_nn_matrices_printf(NULL,"TEST", dest);//打印矩阵
	}
	else{
	printf("null\n");
	}
*/
/* 矩阵交换
	unsigned char src[] = { 1, 2, 3, 4, 5, 6 };
	unsigned char sub[] = { 1, 2, 3 };

	user_nn_matrix *src_matrix = NULL;
	user_nn_matrix *sub_matrix = NULL;
	user_nn_matrix *res_matrix = NULL;

	src_matrix = user_nn_matrix_create(2, 3);
	sub_matrix = user_nn_matrix_create(1, 3);

	user_nn_matrix_memcpy_char(src_matrix, src);
	user_nn_matrix_memcpy_char(sub_matrix, sub);

	user_nn_matrix_exc_width_height(src_matrix);//交换output_kernel_maps 的 width与height
	res_matrix = user_nn_matrix_mult_matrix(src_matrix, sub_matrix);//计算feature vector delta  ######如果不能相乘需要变化横纵大小######
	user_nn_matrix_exc_width_height(src_matrix);//交换output_kernel_maps 的 width与height 交换回来

	if (res_matrix != NULL){
	user_nn_matrix_printf(NULL,res_matrix);//打印矩阵
	}
	else{
	printf("null\n");
	}

*/
/* 矩阵与连续矩阵变化
	unsigned char src[192] ;
	int i = 0;
	for (i = 0; i < 192; i++){
	src[i] = i;
	}
	user_nn_list_matrix *list = NULL;
	user_nn_matrix *dest = NULL;
	dest = user_nn_matrix_create(1, 192);
	user_nn_matrix_memcpy_char(dest, src);
	list = user_nn_matrix_to_matrices(dest, 4, 4);//获取其中一个矩阵
	dest = user_nn_matrices_to_matrix(list);
	if (dest != NULL){
	user_nn_matrix_printf(NULL, dest);//打印矩阵
	}
	else{
	printf("null\n");
	}

*/
/*矩阵返回最大值测试
	user_nn_matrix *matrix = NULL;

	matrix = user_nn_matrix_create(6, 6);//创建2*2大小的二维矩阵
	user_nn_matrix_rand_vaule(matrix, 1);

	int max_value_index = user_nn_matrix_return_max_index(matrix);//
	int min_value_index = user_nn_matrix_return_min_index(matrix);//

	user_nn_matrix_printf(NULL,matrix);//打印矩阵

	printf("\nmax_value_index=%d,vaule=%f\n",max_value_index,*user_nn_matrix_return_max_addr(matrix));
	printf("\nmin_value_index=%d,vaule=%f\n",min_value_index,*user_nn_matrix_return_min_addr(matrix));
*/


/* 矩阵转置时间测试

	FILE *debug_file = NULL;
	debug_file = fopen("debug.txt", "w+");
	user_nn_matrix *src_matrix = NULL;
	user_nn_matrix *sub_matrix = NULL;
	user_nn_matrix *res_matrix = NULL;
	int matrix_w = 128, matrix_h = 128;
	src_matrix = user_nn_matrix_create(matrix_w, matrix_h);
	sub_matrix = user_nn_matrix_create(matrix_w, matrix_h);
	clock_t test_start_time, test_end_time;
	float *src_data = src_matrix->data;
	float *sub_data = sub_matrix->data;

	int count = 0,total_count=499;

	while (count++ < (src_matrix->width * src_matrix->height)){
	*src_data++ = count * 0.01f;
	*sub_data++ = count * 0.01f;
	}
	printf("test start:\n");
	printf("width:%d,height:%d\n", src_matrix->width ,src_matrix->height);
	user_nn_matrix_transpose_cuda(sub_matrix);//启动显卡
	Sleep(1000);
	//user_nn_matrix_printf(debug_file, src_matrix);//打印矩阵
	while (1){
	test_start_time = clock();
	count = total_count;
	while (count--){
	//user_nn_matrix_transpose_cuda(src_matrix);//GPU加速
	user_nn_matrix_transpose(src_matrix);//CPU转置
	}
	src_data = src_matrix->data;//获取数据
	sub_data = sub_matrix->data;//获取数据
	count = 0;
	while (count++ < (src_matrix->width * src_matrix->height)){
	if (*src_data++ != *sub_data++){
	break;
	}
	}
	if ((count - 1) != src_matrix->width * src_matrix->height){
	printf("count:%d ", count);
	printf("test error \n");
	break;
	}
	else{
	test_end_time = (clock() - test_start_time);//获取结束时间
	printf("total time : %f ms\n", (float)test_end_time / total_count);
	break;
	}
	}

	//res_matrix = user_nn_matrix_mult_matrix(src_matrix, sub_matrix);//矩阵相乘
	//user_nn_matrix_transpose(src_matrix);
	if (src_matrix != NULL){
	//user_nn_matrix_printf(debug_file, src_matrix);//打印矩阵
	}
	else{
	printf("null\n");
	}
	fclose(debug_file);
	getchar();
	return 1;

*/
