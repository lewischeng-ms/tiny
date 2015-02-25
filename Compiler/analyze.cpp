// *仅用于学习目的，特此声明！
// 文件: analyze.cpp
// 作者: 成立
// 日期: 2010/3/20
// 描述: TINY编译器的语义分析器实现

#include "globals.h"
#include "symtab.h"
#include "analyze.h"

// 变量内存地址的计数器
static int location = 0;

// 遍历语法树的通用递归函数
// 它将preProc函数应用于前序遍历中
// 它将postProc函数应用于后序遍历中
// 语法树由指针t指定
static void traverse(TreeNode* t,
					 void (* preProc)(TreeNode*),
					 void (* postProc)(TreeNode*))
{
	if (!t) return;
	if (preProc)
		preProc(t);
	for (int i = 0; i < MAXCHILDREN; i++)
		traverse(t->child[i], preProc, postProc);
	if (postProc)
		postProc(t);
	// 仅分析语句序列时有用
	traverse(t->sibling, preProc, postProc);
}

// 将t结点中存放的标识符插入符号表
static void insertNode(TreeNode* t)
{
	switch (t->nodekind)
	{
	case StmtK:
		if (t->kind.stmt == AssignK || t->kind.stmt == ReadK)
		{
			// 还不在符号表中，所以看作新的定义
			if (st_lookup(t->attr.name) == -1)
				st_insert(t->attr.name, t->lineno, location++);
			else
				// 已经在符号表中，所以跳过地址，仅把行号添加进去
				st_insert(t->attr.name, t->lineno, 0);
		}
		break;
	case ExpK:
		if (t->kind.exp == IdK)
		{
			if (st_lookup(t->attr.name) == -1)
				st_insert(t->attr.name, t->lineno, location++);
			else
				st_insert(t->attr.name, t->lineno, 0);
		}
		break;
	}
}

// 前序遍历语法树并构造符号表
void buildSymtab(TreeNode* syntaxTree)
{
	traverse(syntaxTree, insertNode, NULL);
	if (TraceAnalyze)
	{
		fprintf(listing, "\nSymbol table:\n\n");
		printSymTab(listing);
	}
}

static void typeError(TreeNode* t, char* message)
{
	fprintf(listing, "Type error at line %d: %s\n", t->lineno, message);
	Error = true;
}

// 在单个结点执行类型检查
static void checkNode(TreeNode* t)
{
	switch (t->nodekind)
	{
	case ExpK:
		switch (t->kind.exp)
		{
		case OpK:
			if (t->child[0]->type != Integer || t->child[1]->type != Integer)
				typeError(t, "Op applied to non-integer");
			if (t->attr.op == EQ || t->attr.op == LT)
				t->type = Boolean;
			else
				t->type = Integer;
			break;
		case ConstK:
		case IdK:
			t->type = Integer;
			break;
		}
		break;
	case StmtK:
		switch (t->kind.stmt)
		{
		case IfK:
			if (t->child[0]->type == Integer)
				typeError(t->child[0], "if test is not Boolean");
			break;
		case AssignK:
			if (t->child[0]->type != Integer)
				typeError(t->child[0], "assignment of non-integer value");
			break;
		case WriteK:
			if (t->child[0]->type != Integer)
				typeError(t->child[0], "write of non-integer value");
			break;
		case RepeatK:
			if (t->child[1]->type == Integer)
				typeError(t->child[1], "repeat test is not Boolean");
			break;
		}
		break;
	}
}

// 后序遍历语法树并执行类型检查
void typeCheck(TreeNode* syntaxTree)
{
	traverse(syntaxTree, NULL, checkNode);
}