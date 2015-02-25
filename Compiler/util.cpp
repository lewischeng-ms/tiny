// *仅用于学习目的，特此声明！
// 文件: util.cpp
// 作者: 成立
// 日期: 2010/3/20
// 描述: TINY编译器的实用函数的实现

#include "globals.h"
#include "util.h"

// 输出记号及其词素至列表文件
void printToken(TokenType token, const char* tokenString)
{
	switch (token)
	{
	case IF:
	case THEN:
	case ELSE:
	case END:
	case REPEAT:
	case UNTIL:
	case READ:
	case WRITE:
		fprintf(listing, "reserved word: %s\n", tokenString);
		break;
	case ASSIGN:
		fprintf(listing, ":=\n");
		break;
	case LT:
		fprintf(listing, "<\n");
		break;
	case EQ:
		fprintf(listing, "=\n");
		break;
	case LPAREN:
		fprintf(listing, "(\n");
		break;
	case RPAREN:
		fprintf(listing, ")\n");
		break;
	case SEMI:
		fprintf(listing, ";\n");
		break;
	case PLUS:
		fprintf(listing, "+\n");
		break;
	case MINUS:
		fprintf(listing, "-\n");
		break;
	case TIMES:
		fprintf(listing, "*\n");
		break;
	case OVER:
		fprintf(listing, "/\n");
		break;
	case ENDFILE:
		fprintf(listing, "EOF\n");
		break;
	case NUM:
		fprintf(listing, "NUM, val = %s\n", tokenString);
		break;
	case ID:
		fprintf(listing, "ID, name = %s\n", tokenString);
		break;
	case ERROR:
		fprintf(listing, "ERROR: %s\n", tokenString);
		break;
	default:
		// 不该来这里！
		fprintf(listing, "Unknown token: %d\n", token);
	}
}

// 为语法树创建新的语句节点
TreeNode* newStmtNode(StmtKind kind)
{
	TreeNode* t = (TreeNode*)malloc(sizeof TreeNode);
	if (t == NULL)
		fprintf(listing, "Out of memory error at line %d\n", lineno);
	else
	{
		for (int i = 0; i < MAXCHILDREN; i++)
			t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = StmtK;
		t->kind.stmt = kind;
		t->lineno = lineno;
	}
	return t;
}

// 为语法树创建新的表达式节点
TreeNode* newExpNode(ExpKind kind)
{
	TreeNode* t = (TreeNode*)malloc(sizeof TreeNode);
	if (t == NULL)
		fprintf(listing, "Out of memory error at line %d\n", lineno);
	else
	{
		for (int i = 0; i < MAXCHILDREN; i++)
			t->child[i] = NULL;
		t->sibling = NULL;
		t->nodekind = ExpK;
		t->kind.exp = kind;
		t->lineno = lineno;
		t->type = Void;
	}
	return t;
}

// 分配并创建某现有字符串的一份拷贝
char* copyString(char* s)
{
	int n;
	char* t;
	if (s == NULL) return NULL;
	n = strlen(s) + 1;
	t = (char*)malloc(n);
	if (t == NULL)
		fprintf(listing, "Out of memory error at line %d\n", lineno);
	else
		strcpy(t, s);
	return t;
}

// 供printTree()使用来存储当前缩进需要的空格数
static int indentno = 0;

// 增加（减少）缩进的宏
#define INDENT indentno += 2
#define UNINDENT indentno -= 2

// 打印空格以缩进
static void printSpaces(void)
{
	for (int i = 0; i < indentno; i++)
		fprintf(listing, " ");
}

// 输出语法树至列表文件（利用缩进表达子树）
void printTree(TreeNode* tree)
{
	INDENT;
	while (tree != NULL)
	{
		printSpaces();
		if (tree->nodekind == StmtK)
		{
			switch (tree->kind.stmt)
			{
			case IfK:
				fprintf(listing, "If\n");
				break;
			case RepeatK:
				fprintf(listing, "Repeat\n");
				break;
			case AssignK:
				fprintf(listing, "Assign to %s\n", tree->attr.name);
				break;
			case ReadK:
				fprintf(listing, "Read: %s\n", tree->attr.name);
				break;
			case WriteK:
				fprintf(listing, "Write\n");
				break;
			default:
				fprintf(listing, "Unknown StmtNode kind\n");
				break;
			}
		}
		else if (tree->nodekind == ExpK)
		{
			switch (tree->kind.exp)
			{
			case OpK:
				fprintf(listing, "Op: ");
				printToken(tree->attr.op, "\0");
				break;
			case ConstK:
				fprintf(listing, "Const: %d\n", tree->attr.val);
				break;
			case IdK:
				fprintf(listing, "Id: %s\n", tree->attr.name);
				break;
			default:
				fprintf(listing, "Unknown ExpNode kind\n");
				break;
			}
		}
		else
			fprintf(listing, "Unknown node kind\n");
		for (int i = 0; i < MAXCHILDREN; i++)
			printTree(tree->child[i]);
		tree = tree->sibling;
	}
	UNINDENT;
}