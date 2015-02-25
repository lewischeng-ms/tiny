// *仅用于学习目的，特此声明！
// 文件: cgen.cpp
// 作者: 成立
// 日期: 2010/3/20
// 描述: TINY编译器的代码生成器实现

#include "globals.h"
#include "symtab.h"
#include "code.h"
#include "cgen.h"

// 临时变量的内存偏移
// 每当一个临时变量被储存时，它会被减1
// 并当该临时变量被读取时，它会被加1
static int tmpOffset = 0;

// 内部递归代码生成器的原型
static void cGen(TreeNode* tree);

// 在一个语句节点处生成代码
static void genStmt(TreeNode* tree)
{
	TreeNode* p1, * p2, * p3;
	int savedLoc1, savedLoc2, currentLoc;
	int loc;
	switch (tree->kind.stmt)
	{
	case IfK:
		if (TraceCode)
			emitComment("-> if");
		p1 = tree->child[0];
		p2 = tree->child[1];
		p3 = tree->child[2];
		// 为测试表达式产生代码
		cGen(p1);
		savedLoc1 = emitSkip(1);
		emitComment("if: jump to else belongs here");
		// 在then部分上递归
		cGen(p2);
		savedLoc2 = emitSkip(1);
		emitComment("if: jump to end belongs here");
		currentLoc = emitSkip(0);
		emitBackup(savedLoc1);
		emitRM_Abs("JEQ", ac, currentLoc, "if: jmp to else");
		emitRestore();
		// 在else部分上递归
		cGen(p3);
		currentLoc = emitSkip(0);
		emitBackup(savedLoc2);
		emitRM_Abs("LDA", pc, currentLoc, "jmp to end");
		emitRestore();
		if (TraceCode)
			emitComment("<- if");
		break;
	case RepeatK:
		if (TraceCode)
			emitComment("-> repeat");
		p1 = tree->child[0];
		p2 = tree->child[1];
		savedLoc1 = emitSkip(0);
		emitComment("repeat: jump after body comes back here");
		// 产生循环体的代码
		cGen(p1);
		// 产生测试条件的代码
		cGen(p2);
		emitRM_Abs("JEQ", ac, savedLoc1, "repeat: jmp back to body");
		if (TraceCode)
			emitComment("<- repeat");
		break;
	case AssignK:
		if (TraceCode)
			emitComment("-> assign");
		// 为右操作数产生代码
		cGen(tree->child[0]);
		// 现在存储值
		loc = st_lookup(tree->attr.name);
		emitRM("ST", ac, loc, gp, "assign: store value");
		if (TraceCode)
			emitComment("<- assign");
		break;
	case ReadK:
		emitRO("IN", ac, 0, 0, "read integer value");
		loc = st_lookup(tree->attr.name);
		emitRM("ST", ac, loc, gp, "read: store value");
		break;
	case WriteK:
		// 为表达式产生代码
		cGen(tree->child[0]);
		// 现在输出它
		emitRO("OUT", ac, 0, 0, "write ac");
		break;
	}
}

// 在表达式结点产生代码
static void genExp(TreeNode* tree)
{
	int loc;
	TreeNode* p1, * p2;
	switch (tree->kind.exp)
	{
	case ConstK:
		if (TraceCode)
			emitComment("-> Const");
		// 使用LDC指令载入整型常量
		emitRM("LDC", ac, tree->attr.val, 0, "load const");
		if (TraceCode)
			emitComment("<- Const");
		break;
	case IdK:
		if (TraceCode)
			emitComment("-> Id");
		loc = st_lookup(tree->attr.name);
		emitRM("LD", ac, loc, gp, "load id value");
		if (TraceCode)
			emitComment("-> Id");
		break;
	case OpK:
		if (TraceCode)
			emitComment("-> Op");
		p1 = tree->child[0];
		p2 = tree->child[1];
		// 为ac = 左参数产生代码
		cGen(p1);
		// 左操作数入栈代码
		emitRM("ST", ac, tmpOffset--, mp, "op: push left");
		// 为ac = 右参数产生代码
		cGen(p2);
		// 现在载入左操作数
		emitRM("LD", ac1, ++tmpOffset, mp, "op: load left");
		switch (tree->attr.op)
		{
		case PLUS:
			emitRO("ADD", ac, ac1, ac, "op +");
			break;
		case MINUS:
			emitRO("SUB", ac, ac1, ac, "op -");
			break;
		case TIMES:
			emitRO("MUL", ac, ac1, ac, "op *");
			break;
		case OVER:
			emitRO("DIV", ac, ac1, ac, "op /");
			break;
		case LT:
			emitRO("SUB", ac, ac1, ac, "op <");
			emitRM("JLT", ac, 2, pc, "br if true");
			emitRM("LDC", ac, 0, ac, "false case");
			emitRM("LDA", pc, 1, pc, "unconditional jmp");
			emitRM("LDC", ac, 1, ac, "true case");
			break;
		case EQ:
			emitRO("SUB", ac, ac1, ac, "op ==");
			emitRM("JEQ", ac, 2, pc, "br if true");
			emitRM("LDC", ac, 0, ac, "false case");
			emitRM("LDA", pc, 1, pc, "unconditional jmp");
			emitRM("LDC", ac, 1, ac, "true case");
			break;
		default:
			emitComment("BUG: Unknown operator");
			break;
		}
		if (TraceCode)
			emitComment("-> Op");
		break;
	}
}

static void cGen(TreeNode* tree)
{
	if (tree)
	{
		switch (tree->nodekind)
		{
		case StmtK:
			genStmt(tree);
			break;
		case ExpK:
			genExp(tree);
			break;
		}
		cGen(tree->sibling);
	}
}

//----------------------------代码生成器的主函数--------------------------------

// 通过遍历语法树产生代码文件
// 参数codefile指定代码文件的文件名，并被用于注释中的文件名
void codeGen(TreeNode* syntaxTree, char* codefile)
{
	char* s = (char*)malloc(strlen(codefile) + 7);
	strcpy(s, "File: ");
	strcat(s, codefile);
	emitComment("TINY Compilation to TM Code");
	emitComment(s);
	// 产生标准入口代码
	emitComment("Standard prelude:");
	emitRM("LD", mp, 0, ac, "load maxaddress from location 0");
	emitRM("ST", ac, 0, ac, "clear location 0");
	emitComment("End of standard prelude");
	// 产生TINY程序的代码
	cGen(syntaxTree);
	// 完成
	emitComment("End of execution.");
	emitRO("HALT", 0, 0, 0, "");
}