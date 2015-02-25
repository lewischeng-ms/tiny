// *仅用于学习目的，特此声明！
// 文件: code.h
// 作者: 成立
// 日期: 2010/3/20
// 描述: TINY编译器的代码释放功能，以及对TM机器的接口

#ifndef _CODE_H_
#define _CODE_H_

// 程序计数器
#define pc 7

// 内存指针，指向内存顶端（临时存储用）
#define mp 6

// 全局指针，指向内存底端（全局存储用）
#define gp 5

// 累加器
#define ac 0

// 第二累加器
#define ac1 1

//---------------------------代码释放功能-----------------------------

// 释放一个仅寄存器间的TM指令
//	op = 操作代码
//	r = 目标寄存器
//	s = 第一源寄存器
//	t = 第二源寄存器
//	c = 当TraceCode = true时要打印的注释
void emitRO(char* op, int r, int s, int t, char* c);

// 释放一个寄存器到内存的TM指令
//	op = 操作代码
//	r = 目标寄存器
//	d = 偏移量
//	s = 基寄存器
//	c = 当TraceCode = true时要打印的注释
void emitRM(char* op, int r, int d, int s, char* c);

// 跳过howMany指定的代码地址数，以便以后回填（backpatch）
// 它还返回当前代码的地址
int emitSkip(int howMany);

// 跳回loc指定的先前跳过的地址
void emitBackup(int loc);

// 恢复当前代码地址到最高的先前未被释放的地址
void emitRestore(void);

// 在代码文件中打印c指定的注释行
void emitComment(char* c);

// 当释放一个寄存器到内存的TM指令时，转换一个绝对引用到一个以pc为基址的相对引用
void emitRM_Abs(char* op, int r, int a, char* c);

#endif