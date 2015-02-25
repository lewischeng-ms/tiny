// *仅用于学习目的，特此声明！
// 文件: symtab.cpp
// 作者: 成立
// 日期: 2010/3/20
// 描述: TINY编译器的符号表实现（只允许存在一个符号表）

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

// 哈希表的大小
#define SIZE 211

// 用于哈希函数中的乘数
#define SHIFT 4

// 哈希函数
static int hash(char* key)
{
	int temp = 0;
	int i = 0;
	while (key[i] != '\0')
	{
		temp = ((temp << SHIFT) + key[i]) % SIZE;
		++i;
	}
	return temp;
}

// 源文件行号的列表，用于标记变量被引用的位置
typedef struct LineListRec {
	int lineno;
	struct LineListRec* next;
}* LineList;

// 桶列表中一条记录代表一个变量
// 包含名称，分配的内存地址和它出现在源代码中的行号列表
typedef struct BucketListRec {
	char* name;
	LineList lines;
	// 变量的内存地址
	int memloc;
	struct BucketListRec* next;
}* BucketList;

// 哈希表
static BucketList hashTable[SIZE];

// 将行号与内存地址插入符号表
// loc = 内存地址，仅在首次插入，以后忽略
void st_insert(char* name, int lineno, int loc)
{
	int h = hash(name);
	BucketList l = hashTable[h];
	// 在表中寻找变量
	while (l && strcmp(name, l->name))
		l = l->next;
	// 变量还不在表中
	if (!l)
	{
		l = (BucketList)malloc(sizeof(struct BucketListRec));
		l->name = name;
		l->lines = (LineList)malloc(sizeof(struct LineListRec));
		l->lines->lineno = lineno;
		l->lines->next = NULL;
		l->memloc = loc;
		// 添加到该桶列的首位
		l->next = hashTable[h];
		hashTable[h] = l;
	}
	else
	{ // 在表中找到了，所以仅需要添加行号
		LineList t = l->lines;
		while (t->next)
			t = t->next;
		t->next = (LineList)malloc(sizeof(struct LineListRec));
		t->next->lineno = lineno;
		t->next->next = NULL;
	}
}

// 返回一个变量的内存地址，-1表示未找到
int st_lookup(char* name)
{
	int h = hash(name);
	BucketList l = hashTable[h];
	while (l && strcmp(name, l->name))
		l = l->next;
	if (!l)
		return -1;
	else
		return l->memloc;
}

// 将符号表的内容格式化后输出至列表文件
void printSymTab(FILE* listing)
{
	fprintf(listing, "Variable Name  Location     Line Numbers\n");
	fprintf(listing, "-------------  --------     ------------\n");
	for (int i = 0; i < SIZE; ++i)
	{
		if (hashTable[i])
		{
			BucketList l = hashTable[i];
			while (l)
			{
				LineList t = l->lines;
				fprintf(listing, "%-14s ", l->name);
				fprintf(listing, "%-8d  ", l->memloc);
				while (t)
				{
					fprintf(listing, "%4d ", t->lineno);
					t = t->next;
				}
				fprintf(listing, "\n");
				l = l->next;
			}
		}
	}
}