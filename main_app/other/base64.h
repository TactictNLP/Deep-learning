#ifndef _base64_H
#define _base64_H

#include<stdio.h>
#include<stdlib.h> 
#include<string.h>

#define xor_code 0xb7

int base64_encode(const char *input, char *base64);//base����
int base64_decode(const char *base64, char *output);//base����

void encode_xor_map_base64(char *input, char *output);//��������
void decode_base64_map_xor(char *input, char *output);//��������

#endif