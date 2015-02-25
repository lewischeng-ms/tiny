// *仅用于学习目的，特此声明！
// 文件: code.cpp
// 作者: 成立
// 日期: 2010/3/20
// 描述: TINY编译器的代码释放功能的实现

#include "globals.h"
#include "code.h"

// 当前指令释放的TM地址号
static int emitLoc = 0;

// 到现在为止释放出的最高的TM地址
// 为了与emitSkip、emitBackup、emitRestore联合使用
static int highEmitLoc = 0;

// 在代码文件中打印c指定的注释行
void emitComment(char* c)
{
	if (TraceCode)
		fprintf(code, "* %s\n", c);
}

// 释放一个仅寄存器间的TM指令
//	op = 操作代码
//	r = 目标寄存器
//	s = 第一源寄存器
//	t = 第二源寄存器
//	c = 当TraceCode = true时要打印的注释
void emitRO(char* op, int r, int s, int t, char* c)
{
	fprintf(code, "%3d:  %5s  %d,%d,%d ", emitLoc++, op, r, s, t);
	if (TraceCode)
		fprintf(code, "\t%s", c);
	fprintf(code, "\n");
	if (highEmitLoc < emitLoc)
		highEmitLoc = emitLoc;
}

// 释放一个寄存器到内存的TM指令
//	op = 操作代码
//	r = 目标寄存器
//	d = 偏移量
//	s = 基寄存器
//	c = 当TraceCode = true时要打印的注释
void emitRM(char* op, int r, int d, int s, char* c)
{
	fprintf(code, "%3d:  %5s  %d,%d(%d) ", emitLoc++, op, r, d, s);
	if (TraceCode)
		fprintf(code, "\t%s", c);
	fprintf(code, "\n");
	if (highEmitLoc < emitLoc)
		highEmitLoc = emitLoc;
}

// 跳过howMany指定的代码地址数，以便以后回填（backpatch）
// 它还返回当前代码的地址
int emitSkip(int howMany)
{
	int i = emitLoc;
	emitLoc += howMany;
	if (highEmitLoc < emitLoc)
		highEmitLoc = emitLoc;
	return i;
}

// 跳回loc指定的先前跳过的地址
void emitBackup(int loc)
{
	if (loc > highEmitLoc)
		emitComment("Bug in emitBackup");
	emitLoc = loc;
}

// 恢复当前代码地址到最高的先前未被释放的地址
void emitRestore(void)
{
	emitLoc = highEmitLoc;
}

// 当释放一个寄存器到内存的TM指令时，转换一个绝对引用到一个以pc为基址的相对引用
void emitRM_Abs(char* op, int r, int a, char* c)
{
	fprintf(code, "%3d:  %5s  %d,%d(%d) ", emitLoc, op, r, a - (emitLoc + 1), pc);
	++emitLoc;
	if (TraceCode)
		fprintf(code, "\t%s", c);
	fprintf(code, "\n");
	if (highEmitLoc < emitLoc)
		highEmitLoc = emitLoc;
}