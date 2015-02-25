// *仅用于学习目的，特此声明！
// 文件: symtab.h
// 作者: 成立
// 日期: 2010/3/20
// 描述: TINY编译器的符号表接口

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

// 将行号与内存地址插入符号表
// loc = 内存地址，仅在首次插入，以后忽略
void st_insert(char* name, int lineno, int loc);

// 返回一个变量的内存地址，-1表示未找到
int st_lookup(char* name);

// 将符号表的内容格式化后输出至列表文件
void printSymTab(FILE* listing);

#endif