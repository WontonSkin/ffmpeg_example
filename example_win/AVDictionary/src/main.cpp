#include <string>
#include <memory>
#include <thread>
#include <iostream>

#include "pch.h"

using namespace std;

/*
 * AVDictionary��һ����ֵ�Դ洢���ߣ�������c++�е�map��ffmpeg���кܶ� API ͨ���������ݲ�����
 */

//���
int test1()
{
	AVDictionary *d = NULL;

	//���
	av_dict_set(&d, "version", "1.0", 0);

	//�ͷ�AVDictionary����������ļ�ֵ���ڴ�
	av_dict_free(&d);

	return 0;
}

//����
int test2()
{
	AVDictionary *d = NULL;
	AVDictionaryEntry *t = NULL;

	//���
	av_dict_set(&d, "version", "1.0", 0);
	av_dict_set_int(&d, "count", 3, 0);    //ע���̨����Ϊ�ַ���

	//����
	t = av_dict_get(d, "version", NULL, 0);
	av_log(NULL, AV_LOG_DEBUG, "version: %s\n", t->value);

	t = av_dict_get(d, "count", NULL, 0);
	av_log(NULL, AV_LOG_DEBUG, "count: %s\n", t->value);
	av_log(NULL, AV_LOG_DEBUG, "count: %d\n", stoi(string(t->value)));

	av_dict_free(&d);
	return 0;
}

//����
int test3()
{
	AVDictionary *d = NULL;
	AVDictionaryEntry *t = NULL;

	//���
	av_dict_set(&d, "version", "1.0", 0);
	av_dict_set_int(&d, "count", 3, 0);

	//����
	//AV_DICT_IGNORE_SUFFIX�����Ժ�׺��
	//�����keyֵ���ֵ���ĳ����Ŀkeyֵ��ǰ׺������Ϊ��ƥ��ġ�
	//���������ַ����������ַ�����ǰ׺�����Կɻ�ȡ���ֵ��е�ÿһ����Ŀ��
	while ((t = av_dict_get(d, "", t, AV_DICT_IGNORE_SUFFIX))) {
		av_log(NULL, AV_LOG_DEBUG, "%s: %s\n", t->key, t->value);
	}

	av_dict_free(&d);
	return 0;
}

//����
int test4()
{
	AVDictionary *d = NULL;
	AVDictionaryEntry *t = NULL;

	//���
	av_dict_set(&d, "version", "1.0", 0);

	//����
	av_dict_set(&d, "version", "2.0", 0);

	//����
	t = av_dict_get(d, "version", NULL, 0);
	av_log(NULL, AV_LOG_DEBUG, "version: %s\n", t->value);

	av_dict_free(&d);
	return 0;
}

//ɾ��
int test5()
{
	AVDictionary *d = NULL;
	AVDictionaryEntry *t = NULL;

	//���
	av_dict_set(&d, "version", "1.0", 0);
	av_dict_set_int(&d, "count", 3, 0);

	//����
	while ((t = av_dict_get(d, "", t, AV_DICT_IGNORE_SUFFIX))) {
		av_log(NULL, AV_LOG_DEBUG, "%s: %s\n", t->key, t->value);
	}

	//ɾ��
	cout << "delete key: version." << endl;
	av_dict_set(&d, "version", NULL, 0);

	//����
	while ((t = av_dict_get(d, "", t, AV_DICT_IGNORE_SUFFIX))) {
		av_log(NULL, AV_LOG_DEBUG, "%s: %s\n", t->key, t->value);
	}

	//ɾ��
	cout << "delete key: count." << endl;
	av_dict_set(&d, "count", NULL, 0);

	//����
	while ((t = av_dict_get(d, "", t, AV_DICT_IGNORE_SUFFIX))) {
		av_log(NULL, AV_LOG_DEBUG, "%s: %s\n", t->key, t->value);
	}

	av_dict_free(&d);
	return 0;
}


//���� AV_DICT_MATCH_CASE ˵��
//AV_DICT_MATCH_CASE ���ֵ��м���key����Ҫ���ִ�Сд��Ĭ���ǲ����ִ�Сд�ģ������Ҫ�ر�ע�⡣
int test6()
{
	AVDictionary *d = NULL;
	AVDictionaryEntry *t = NULL;

	cout << "->Default" << endl;

	//0��Ĭ�������ǲ����ִ�Сд��
	av_dict_set(&d, "version", "1.0", 0);
	av_dict_set(&d, "VERsion", "2.0", 0);
	
	//����
	while ((t = av_dict_get(d, "", t, AV_DICT_IGNORE_SUFFIX))) {
		av_log(NULL, AV_LOG_DEBUG, "%s: %s\n", t->key, t->value);
	}

	av_dict_free(&d);
	d = NULL;

	cout << "->AV_DICT_MATCH_CASE" << endl;

	//1��AV_DICT_MATCH_CASE�����ֵ��м���key����Ҫ���ִ�Сд��Ĭ���ǲ����ִ�Сд�ģ������Ҫ�ر�ע�⡣
	av_dict_set(&d, "version", "1.0", AV_DICT_MATCH_CASE);
	av_dict_set(&d, "VERsion", "2.0", AV_DICT_MATCH_CASE);

	//����
	while ((t = av_dict_get(d, "", t, AV_DICT_IGNORE_SUFFIX))) {
		av_log(NULL, AV_LOG_DEBUG, "%s: %s\n", t->key, t->value);
	}

	av_dict_free(&d);
	return 0;
}

//���� AV_DICT_IGNORE_SUFFIX ˵��
//���keyֵ���ֵ���ĳ����Ŀkeyֵ��ǰ׺������Ϊ��ƥ��ġ�
int test7()
{
	cout << "->AV_DICT_IGNORE_SUFFIX" << endl;

	AVDictionary *dict = NULL;

	av_dict_set(&dict, "language", "ch", 0);
	av_dict_set(&dict, "version", "1.0", 0);
	AVDictionaryEntry *t1 = av_dict_get(dict, "ver", NULL, AV_DICT_IGNORE_SUFFIX);
	AVDictionaryEntry *t2 = av_dict_get(dict, "", NULL, AV_DICT_IGNORE_SUFFIX); //���������ַ����������ַ�����ǰ׺�����Ի�ȡ���ֵ��еĵ�һ����Ŀ��
	av_log(NULL, AV_LOG_DEBUG, "t1, %s: %s\n", t1->key, t1->value);
	av_log(NULL, AV_LOG_DEBUG, "t2, %s: %s\n", t2->key, t2->value);

	av_dict_free(&dict);
	return 0;
}

//���� AV_DICT_DONT_STRDUP_KEY �� AV_DICT_DONT_STRDUP_VAL ˵��
//�ӹ�ʹ��key��valueָ��ָ����ڴ�Ĺ���Ȩ������key��value��ֵ���и��ơ�
int test8()
{
	cout << "->AV_DICT_DONT_STRDUP_KEY" << endl;
	cout << "->AV_DICT_DONT_STRDUP_VAL" << endl;

	AVDictionary *d = NULL;
	AVDictionaryEntry *t = NULL;

	//�ַ����ڴ�����
	char *k = av_strdup("key->name");
	char *v = av_strdup("value->wonton"); 

	//if your strings are already allocated, you can avoid copying them like this
	//�ַ����ڴ��Ѿ���������ˣ���Ч������ʹ�� AV_DICT_DONT_STRDUP_KEY | AV_DICT_DONT_STRDUP_VAL
	av_dict_set(&d, k, v, AV_DICT_DONT_STRDUP_KEY | AV_DICT_DONT_STRDUP_VAL);
	while (t = av_dict_get(d, "", t, AV_DICT_IGNORE_SUFFIX)) {
		av_log(NULL, AV_LOG_DEBUG, "%s: %s\n", t->key, t->value);
	}

	//�ַ����ڴ��ͷ�
	av_dict_free(&d);  //��ֵ���ڴ���Ȼ����������(av_strdup)�����ڴ˴��ڲ��ͷ�

	return 0;
}

//���� AV_DICT_DONT_OVERWRITE ˵��
//������������Ŀ
int test9()
{
	cout << "->AV_DICT_DONT_OVERWRITE" << endl;

	AVDictionary *d = NULL;
	AVDictionaryEntry *t = NULL;

	av_dict_set(&d, "version", "1.0", 0);
	av_dict_set(&d, "version", "2.0", AV_DICT_DONT_OVERWRITE);

	while (t = av_dict_get(d, "", t, AV_DICT_IGNORE_SUFFIX)) {
		av_log(NULL, AV_LOG_DEBUG, "%s: %s\n", t->key, t->value);
	}

	av_dict_free(&d);
	return 0;
}

//���� AV_DICT_APPEND ˵��
//���Ŀ�Ѿ����ڣ���valueֱֵ��ƴ�ӵ�֮ǰ��ֵ�ĺ��档
//ע�⣬û����ӷָ������ַ���ֻ��������һ��
int test10()
{
	cout << "->AV_DICT_APPEND" << endl;

	AVDictionary *d = NULL;
	AVDictionaryEntry *t = NULL;

	av_dict_set(&d, "version", "1.0", 0);
	av_dict_set(&d, "version", "2.0", AV_DICT_APPEND);

	while (t = av_dict_get(d, "", t, AV_DICT_IGNORE_SUFFIX)) {
		av_log(NULL, AV_LOG_DEBUG, "%s: %s\n", t->key, t->value);
	}

	av_dict_free(&d);
	return 0;
}

//���� AV_DICT_MULTIKEY ˵��
//�������ֵ��д洢������ȵ�key��
int test11()
{
	cout << "->AV_DICT_MULTIKEY" << endl;

	AVDictionary *dict = NULL;
	AVDictionaryEntry *t = NULL;

	av_dict_set(&dict, "version", "1.0", 0);
	av_dict_set(&dict, "VERsion", "2.0", AV_DICT_MULTIKEY);
	av_dict_set(&dict, "version", "3.0", AV_DICT_MULTIKEY);
	
	while ((t = av_dict_get(dict, "", t, AV_DICT_IGNORE_SUFFIX))) {
		av_log(NULL, AV_LOG_DEBUG, "%s: %s\n", t->key, t->value);
	}
	
	av_dict_free(&dict);
	return 0;
}


int main(int argc, char* argv[])
{
	av_log_set_level(AV_LOG_DEBUG);

	cout << "===>test1<===" << endl;
	test1();

	cout << "===>test2<===" << endl;
	test2();

	cout << "===>test3<===" << endl;
	test3();

	cout << "===>test4<===" << endl;
	test4();

	cout << "===>test5<===" << endl;
	test5();

	cout << "===>test6<===" << endl;
	test6();

	cout << "===>test7<===" << endl;
	test7();

	cout << "===>test8<===" << endl;
	test8();

	cout << "===>test9<===" << endl;
	test9();

	cout << "===>test10<===" << endl;
	test10();

	cout << "===>test11<===" << endl;
	test11();

	return 0;
}

