#include <string>
#include <memory>
#include <thread>
#include <iostream>

#include "pch.h"

using namespace std;

/*
 * AVDictionary是一个健值对存储工具，类似于c++中的map，ffmpeg中有很多 API 通过它来传递参数。
 */

//添加
int test1()
{
	AVDictionary *d = NULL;

	//添加
	av_dict_set(&d, "version", "1.0", 0);

	//释放AVDictionary中所有申请的键值对内存
	av_dict_free(&d);

	return 0;
}

//查找
int test2()
{
	AVDictionary *d = NULL;
	AVDictionaryEntry *t = NULL;

	//添加
	av_dict_set(&d, "version", "1.0", 0);
	av_dict_set_int(&d, "count", 3, 0);    //注意后台保存为字符串

	//查找
	t = av_dict_get(d, "version", NULL, 0);
	av_log(NULL, AV_LOG_DEBUG, "version: %s\n", t->value);

	t = av_dict_get(d, "count", NULL, 0);
	av_log(NULL, AV_LOG_DEBUG, "count: %s\n", t->value);
	av_log(NULL, AV_LOG_DEBUG, "count: %d\n", stoi(string(t->value)));

	av_dict_free(&d);
	return 0;
}

//遍历
int test3()
{
	AVDictionary *d = NULL;
	AVDictionaryEntry *t = NULL;

	//添加
	av_dict_set(&d, "version", "1.0", 0);
	av_dict_set_int(&d, "count", 3, 0);

	//遍历
	//AV_DICT_IGNORE_SUFFIX：忽略后缀。
	//即如果key值是字典中某个条目key值的前缀，则认为是匹配的。
	//而“”空字符串是所有字符串的前缀，所以可获取到字典中的每一个条目。
	while ((t = av_dict_get(d, "", t, AV_DICT_IGNORE_SUFFIX))) {
		av_log(NULL, AV_LOG_DEBUG, "%s: %s\n", t->key, t->value);
	}

	av_dict_free(&d);
	return 0;
}

//更新
int test4()
{
	AVDictionary *d = NULL;
	AVDictionaryEntry *t = NULL;

	//添加
	av_dict_set(&d, "version", "1.0", 0);

	//更新
	av_dict_set(&d, "version", "2.0", 0);

	//查找
	t = av_dict_get(d, "version", NULL, 0);
	av_log(NULL, AV_LOG_DEBUG, "version: %s\n", t->value);

	av_dict_free(&d);
	return 0;
}

//删除
int test5()
{
	AVDictionary *d = NULL;
	AVDictionaryEntry *t = NULL;

	//添加
	av_dict_set(&d, "version", "1.0", 0);
	av_dict_set_int(&d, "count", 3, 0);

	//遍历
	while ((t = av_dict_get(d, "", t, AV_DICT_IGNORE_SUFFIX))) {
		av_log(NULL, AV_LOG_DEBUG, "%s: %s\n", t->key, t->value);
	}

	//删除
	cout << "delete key: version." << endl;
	av_dict_set(&d, "version", NULL, 0);

	//遍历
	while ((t = av_dict_get(d, "", t, AV_DICT_IGNORE_SUFFIX))) {
		av_log(NULL, AV_LOG_DEBUG, "%s: %s\n", t->key, t->value);
	}

	//删除
	cout << "delete key: count." << endl;
	av_dict_set(&d, "count", NULL, 0);

	//遍历
	while ((t = av_dict_get(d, "", t, AV_DICT_IGNORE_SUFFIX))) {
		av_log(NULL, AV_LOG_DEBUG, "%s: %s\n", t->key, t->value);
	}

	av_dict_free(&d);
	return 0;
}


//属性 AV_DICT_MATCH_CASE 说明
//AV_DICT_MATCH_CASE 在字典中检索key是需要区分大小写，默认是不区分大小写的，这点需要特别注意。
int test6()
{
	AVDictionary *d = NULL;
	AVDictionaryEntry *t = NULL;

	cout << "->Default" << endl;

	//0、默认属性是不区分大小写的
	av_dict_set(&d, "version", "1.0", 0);
	av_dict_set(&d, "VERsion", "2.0", 0);
	
	//遍历
	while ((t = av_dict_get(d, "", t, AV_DICT_IGNORE_SUFFIX))) {
		av_log(NULL, AV_LOG_DEBUG, "%s: %s\n", t->key, t->value);
	}

	av_dict_free(&d);
	d = NULL;

	cout << "->AV_DICT_MATCH_CASE" << endl;

	//1、AV_DICT_MATCH_CASE：在字典中检索key是需要区分大小写，默认是不区分大小写的，这点需要特别注意。
	av_dict_set(&d, "version", "1.0", AV_DICT_MATCH_CASE);
	av_dict_set(&d, "VERsion", "2.0", AV_DICT_MATCH_CASE);

	//遍历
	while ((t = av_dict_get(d, "", t, AV_DICT_IGNORE_SUFFIX))) {
		av_log(NULL, AV_LOG_DEBUG, "%s: %s\n", t->key, t->value);
	}

	av_dict_free(&d);
	return 0;
}

//属性 AV_DICT_IGNORE_SUFFIX 说明
//如果key值是字典中某个条目key值的前缀，则认为是匹配的。
int test7()
{
	cout << "->AV_DICT_IGNORE_SUFFIX" << endl;

	AVDictionary *dict = NULL;

	av_dict_set(&dict, "language", "ch", 0);
	av_dict_set(&dict, "version", "1.0", 0);
	AVDictionaryEntry *t1 = av_dict_get(dict, "ver", NULL, AV_DICT_IGNORE_SUFFIX);
	AVDictionaryEntry *t2 = av_dict_get(dict, "", NULL, AV_DICT_IGNORE_SUFFIX); //而“”空字符串是所有字符串的前缀，所以获取到字典中的第一个条目。
	av_log(NULL, AV_LOG_DEBUG, "t1, %s: %s\n", t1->key, t1->value);
	av_log(NULL, AV_LOG_DEBUG, "t2, %s: %s\n", t2->key, t2->value);

	av_dict_free(&dict);
	return 0;
}

//属性 AV_DICT_DONT_STRDUP_KEY 或 AV_DICT_DONT_STRDUP_VAL 说明
//接管使用key或value指针指向的内存的管理权，不对key或value的值进行复制。
int test8()
{
	cout << "->AV_DICT_DONT_STRDUP_KEY" << endl;
	cout << "->AV_DICT_DONT_STRDUP_VAL" << endl;

	AVDictionary *d = NULL;
	AVDictionaryEntry *t = NULL;

	//字符串内存申请
	char *k = av_strdup("key->name");
	char *v = av_strdup("value->wonton"); 

	//if your strings are already allocated, you can avoid copying them like this
	//字符串内存已经被申请过了，无效再申请使用 AV_DICT_DONT_STRDUP_KEY | AV_DICT_DONT_STRDUP_VAL
	av_dict_set(&d, k, v, AV_DICT_DONT_STRDUP_KEY | AV_DICT_DONT_STRDUP_VAL);
	while (t = av_dict_get(d, "", t, AV_DICT_IGNORE_SUFFIX)) {
		av_log(NULL, AV_LOG_DEBUG, "%s: %s\n", t->key, t->value);
	}

	//字符串内存释放
	av_dict_free(&d);  //键值对内存虽然在外面申请(av_strdup)，但在此处内部释放

	return 0;
}

//属性 AV_DICT_DONT_OVERWRITE 说明
//不覆盖现有条目
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

//属性 AV_DICT_APPEND 说明
//如果目已经存在，则value值直接拼接到之前的值的后面。
//注意，没有添加分隔符，字符串只是连接在一起。
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

//属性 AV_DICT_MULTIKEY 说明
//允许在字典中存储几个相等的key。
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

