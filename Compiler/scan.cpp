// *仅用于学习目的，特此声明！
// 文件: scan.h
// 作者: 成立
// 日期: 2010/3/20
// 描述: TINY编译器的扫描器实现

#include "globals.h"
#include "util.h"
#include "scan.h"

// 扫描器DFA的状态
typedef enum {
	START,
	INASSIGN,
	INCOMMENT,
	INNUM,
	INID,
	DONE
} StateType;

// 标识符或保留字的词素
char tokenString[MAXTOKENLEN + 1];

// 源代码行输入缓存的长度
#define BUFLEN 256

// 是否已到文件末尾
static bool endfile;
// 保存当前行
static char lineBuf[BUFLEN];
// 在lineBuf中的当前位置
static int linepos = 0;
// 当前缓存中字符串的长度
static int bufsize = 0;

// 从lineBuf中取回下一个字符
// 如果已读完，则读入新的一行
static char getNextChar(void)
{
	if (linepos >= bufsize)
	{
		lineno++;
		if (fgets(lineBuf, BUFLEN - 1, source))
		{
			if (EchoSource)
				fprintf(listing, "%4d: %s", lineno, lineBuf);
			bufsize = strlen(lineBuf);
			linepos = 0;
			return lineBuf[linepos++];
		}
		else
			return EOF;
	}
	else
		return lineBuf[linepos++];
}

// 在lineBuf中回溯一个字符
static void ungetNextChar(void)
{
	linepos--;
}

// 保留字的查找表
static struct {
	char* str;
	TokenType tok;
} reservedWords[MAXRESERVED] = {
	{"if", IF},
	{"then", THEN},
	{"else", ELSE},
	{"end", END},
	{"repeat", REPEAT},
	{"until", UNTIL},
	{"read", READ},
	{"write", WRITE}
};

// 线性查找上表判断一个标识符是否是保留字
static TokenType reservedLookup(char* s)
{
	for (int i = 0; i < MAXRESERVED; i++)
		if (!strcmp(s, reservedWords[i].str))
			return reservedWords[i].tok;
	return ID;
}

//-----------------------------扫描器的主要函数-------------------------------
// 返回源文件中的下一个记号
TokenType getToken(void)
{
	// 存入tokenString时的下标
	int tokenStringIndex = 0;
	// 保存当前标记以备返回
	TokenType currentToken;
	// 当前的状态，总是从START开始
	StateType state = START;
	// 标志：指示是否保存到tokenString
	bool save;
	while (state != DONE)
	{
		char c = getNextChar();
		save = true;
		switch (state)
		{
		case START:
			if (isdigit(c))
				state = INNUM;
			else if (isalpha(c))
				state = INID;
			else if (c == ':')
				state = INASSIGN;
			else if (c == ' ' || c == '\t' || c == '\n')
				save = false;
			else if (c == '{')
			{
				save = false;
				state = INCOMMENT;
			}
			else
			{
				state = DONE;
				switch (c)
				{
				case EOF:
					save = false;
					currentToken = ENDFILE;
					break;
				case '=':
					currentToken = EQ;
					break;
				case '<':
					currentToken = LT;
					break;
				case '+':
					currentToken = PLUS;
					break;
				case '-':
					currentToken = MINUS;
					break;
				case '*':
					currentToken = TIMES;
					break;
				case '/':
					currentToken = OVER;
					break;
				case '(':
					currentToken = LPAREN;
					break;
				case ')':
					currentToken = RPAREN;
					break;
				case  ';':
					currentToken = SEMI;
					break;
				default:
					currentToken = ERROR;
					break;
				}
			}
			break;
		case INCOMMENT:
			save = false;
			if (c == '}') state = START;
			break;
		case INASSIGN:
			state = DONE;
			if (c == '=')
				currentToken = ASSIGN;
			else
			{
				// 放回到输入中
				ungetNextChar();
				save = false;
				currentToken = ERROR;
			}
			break;
		case INNUM:
			if (!isdigit(c))
			{
				// 放回输入中
				ungetNextChar();
				save = false;
				state = DONE;
				currentToken = NUM;
			}
			break;
		case INID:
			if (!isalpha(c))
			{
				// 放回输入中
				ungetNextChar();
				save = false;
				state = DONE;
				currentToken = ID;
			}
			break;
		case DONE:
		default:
			// 不该到这来！
			fprintf(listing, "Scanner Bug: state = %d\n", state);
			state = DONE;
			currentToken = ERROR;
			break;
		}
		if (save && tokenStringIndex <= MAXTOKENLEN)
			tokenString[tokenStringIndex++] = c;
		if (state == DONE)
		{
			tokenString[tokenStringIndex] = '\0';
			if (currentToken == ID)
				currentToken = reservedLookup(tokenString);
		}
	}
	if (TraceScan)
	{
		fprintf(listing, "\t%d: ", lineno);
		printToken(currentToken, tokenString);
	}
	return currentToken;
} // getToken结束