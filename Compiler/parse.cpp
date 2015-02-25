// *仅用于学习目的，特此声明！
// 文件: parse.cpp
// 作者: 成立
// 日期: 2010/3/20
// 描述: TINY编译器的语法分析器实现

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

// 当前的记号
static TokenType token;

// 递归调用函数的原型
static TreeNode* stmt_sequence(void);
static TreeNode* statement(void);
static TreeNode* if_stmt(void);
static TreeNode* repeat_stmt(void);
static TreeNode* assign_stmt(void);
static TreeNode* read_stmt(void);
static TreeNode* write_stmt(void);
static TreeNode* exp(void);
static TreeNode* simple_exp(void);
static TreeNode* term(void);
static TreeNode* factor(void);

// 输出语法错误
static void syntaxError(char* message)
{
	fprintf(listing, "\n>> ");
	fprintf(listing, "Syntax error at line %d: %s", lineno, message);
	Error = true;
}

// “吃”掉一个记号，如果为所期望的，则自动读入下一个记号，否则报错
static void eat(TokenType expected)
{
	if (token == expected)
		token = getToken();
	else
	{
		syntaxError("unexpected token -> ");
		printToken(token, tokenString);
		fprintf(listing, "	");
	}
}

// 分析语句序列
TreeNode* stmt_sequence(void)
{
	TreeNode* t = statement();
	TreeNode* p = t;
	while (token != ENDFILE && token != END && token != ELSE && token != UNTIL)
	{
		TreeNode* q;
		eat(SEMI);
		q = statement();
		if (q != NULL)
		{
			if (t == NULL)
				t = p = q;
			else
			{
				// 现在p也不能为NULL
				p->sibling = q;
				p = q;
			}
		}
	}
	return t;
}

// 分析语句
TreeNode* statement(void)
{
	TreeNode* t = NULL;
	switch (token)
	{
	case IF:
		t = if_stmt();
		break;
	case REPEAT:
		t = repeat_stmt();
		break;
	case ID:
		t = assign_stmt();
		break;
	case READ:
		t = read_stmt();
		break;
	case WRITE:
		t = write_stmt();
		break;
	default:
		syntaxError("unexpected token -> ");
		printToken(token, tokenString);
		token = getToken();
		break;
	}
	return t;
}

// 分析if语句
TreeNode* if_stmt(void)
{
	TreeNode* t = newStmtNode(IfK);
	eat(IF);
	if (t != NULL)
		t->child[0] = exp();
	eat(THEN);
	if (t != NULL)
		t->child[1] = stmt_sequence();
	if (token == ELSE)
	{
		eat(ELSE);
		if (t != NULL)
			t->child[2] = stmt_sequence();
	}
	eat(END);
	return t;
}

// 分析repeat语句
TreeNode* repeat_stmt(void)
{
	TreeNode* t = newStmtNode(RepeatK);
	eat(REPEAT);
	if (t != NULL)
		t->child[0] = stmt_sequence();
	eat(UNTIL);
	if (t != NULL)
		t->child[1] = exp();
	return t;
}

// 分析赋值语句
TreeNode* assign_stmt(void)
{
	TreeNode* t = newStmtNode(AssignK);
	if (t != NULL && token == ID)
		t->attr.name = copyString(tokenString);
	eat(ID);
	eat(ASSIGN);
	if (t != NULL)
		t->child[0] = exp();
	return t;
}

// 分析read语句
TreeNode* read_stmt(void)
{
	TreeNode* t = newStmtNode(ReadK);
	eat(READ);
	if (t != NULL && token == ID)
		t->attr.name = copyString(tokenString);
	eat(ID);
	return t;
}

// 分析write语句
TreeNode* write_stmt(void)
{
	TreeNode* t = newStmtNode(WriteK);
	eat(WRITE);
	if (t != NULL)
		t->child[0] = exp();
	return t;
}

// 分析表达式：逻辑表达式或算数表达式
TreeNode* exp(void)
{
	TreeNode* t = simple_exp();
	if (token == LT || token == EQ)
	{
		TreeNode* p = newExpNode(OpK);
		if (p != NULL)
		{
			p->child[0] = t;
			p->attr.op = token;
			t = p;
		}
		eat(token);
		if (t != NULL)
			t->child[1] = simple_exp();
	}
	return t;
}

// 分析简单表达式：项 <+|-> 项
TreeNode* simple_exp(void)
{
	TreeNode* t = term();
	while (token == PLUS || token == MINUS)
	{
		TreeNode* p = newExpNode(OpK);
		if (p != NULL)
		{
			p->child[0] = t;
			p->attr.op = token;
			t = p;
			eat(token);
			t->child[1] = term();
		}
	}
	return t;
}

// 分析项：因子 <*|/> 因子
TreeNode* term(void)
{
	TreeNode* t = factor();
	while (token == TIMES || token == OVER)
	{
		TreeNode* p = newExpNode(OpK);
		if (p != NULL)
		{
			p->child[0] = t;
			p->attr.op = token;
			t = p;
			eat(token);
			t->child[1] = factor();
		}
	}
	return t;
}

// 分析因子：常数、标识符或子表达式
TreeNode* factor(void)
{
	TreeNode* t = NULL;
	switch (token)
	{
	case NUM:
		t = newExpNode(ConstK);
		if (t != NULL && token == NUM)
			t->attr.val = atoi(tokenString);
		eat(NUM);
		break;
	case ID:
		t = newExpNode(IdK);
		if (t != NULL && token == ID)
			t->attr.name = copyString(tokenString);
		eat(ID);
		break;
	case LPAREN:
		eat(LPAREN);
		t = exp();
		eat(RPAREN);
		break;
	default:
		syntaxError("unexpected token -> ");
		printToken(token, tokenString);
		token = getToken();
		break;
	}
	return t;
}

//------------------------------语法分析器的主要函数-----------------------------

// 返回最近构造出的语法树
TreeNode* parse(void)
{
	TreeNode* t;
	token = getToken();
	t = stmt_sequence();
	if (token != ENDFILE)
		syntaxError("Code ends before file\n");
	return t;
}