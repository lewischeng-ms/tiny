// *仅用于学习目的，特此声明！
// 文件: globals.h
// 作者: 成立
// 日期: 2010/3/20
// 描述: TINY编译器的全局类型和变量，必须出现在其他头文件之前

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

// 保留字的个数
#define MAXRESERVED 8

typedef enum {
	// 簿记记号
	ENDFILE, ERROR,
	// 保留字
	IF, THEN, ELSE, END, REPEAT, UNTIL, READ, WRITE,
	// 多字符记号
	ID, NUM,
	// 特殊记号，即符号
	ASSIGN, EQ, LT, PLUS, MINUS, TIMES, OVER, LPAREN, RPAREN, SEMI
} TokenType;

// 源文件(文本)
extern FILE* source;
// 输出列表文件(文本)
extern FILE* listing;
// TM模拟机代码文件(文本)
extern FILE* code;

// 源文件中的行号
extern int lineno;

//----------------------语法分析中的语法树--------------------------

typedef enum { StmtK, ExpK } NodeKind;
typedef enum { IfK, RepeatK, AssignK, ReadK, WriteK } StmtKind;
typedef enum { OpK, ConstK, IdK } ExpKind;

// ExpType被用于类型检查
typedef enum { Void, Integer, Boolean } ExpType;

#define MAXCHILDREN 3

typedef struct treeNode {
	struct treeNode* child[MAXCHILDREN];
	struct treeNode* sibling;
	int lineno;
	// 节点类型：语句or表达式？
	NodeKind nodekind;
	// 如果是语句（表达式），那是哪种语句（表达式）？
	union { StmtKind stmt; ExpKind exp; } kind;
	// 节点属性值：操作or整数值or标识符名。
	union {
		TokenType op;
		int val;
		char* name;
	} attr;
	// 说明表达式结果的类型，用于类型检查
	ExpType type;
} TreeNode;

//-----------------------Flags for tracing----------------------------

// EchoSource = true使得语法分析时，源程序及其行号会被输出到列表文件中
extern bool EchoSource;

// TraceScan = true使得每个被扫描出来的记号的信息会被输出到列表文件中
extern bool TraceScan;

// TraceParse = true使得语法树会被线性化并输出到列表文件中（对子节点缩进）
extern bool TraceParse;

// TraceAnalyze = true 使得符号表的插入和查询操作均被记录到列表文件中
extern bool TraceAnalyze;

// TraceCode = true 使得编译器在产生TM代码文件时会自动添加注释
extern bool TraceCode;

// Error = true 使得当一个错误产生时编译器不会跳过它
extern bool Error;

#endif