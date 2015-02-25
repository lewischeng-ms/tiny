// *仅用于学习目的，特此声明！
// 文件: main.c
// 作者: 成立
// 日期: 2010/3/20
// 描述: TINY编译器的主程序

#pragma warning(disable:4996)

#include "globals.h"

// 设置NO_PARSE为true以产生仅有词法分析的编译器
#define NO_PARSE FALSE

// 设置NO_ANALYZE为true以产生仅有语法分析的编译器
#define NO_ANALYZE FALSE

// 设置NO_CODE为true，编译器将不会生成目标代码
#define NO_CODE FALSE

#include "util.h"
#if NO_PARSE
#include "scan.h"
#else
#include "parse.h"
#if !NO_ANALYZE
#include "analyze.h"
#if !NO_CODE
#include "cgen.h"
#endif
#endif
#endif

// 分配全局变量
int lineno = 0;
FILE* source;
FILE* listing;
FILE* code;

// 分配和设置各跟踪标志
bool EchoSource = true;
bool TraceScan = true;
bool TraceParse = true;
bool TraceAnalyze = true;
bool TraceCode = true;

bool Error = false;

int main(int argc, char* argv[])
{
	TreeNode* syntaxTree;
	// 源文件名
	char pgm[20];
	if (argc != 2)
	{
		fprintf(stderr, "usage: %s <filename>\n", argv[0]);
		exit(1);
	}
	strcpy(pgm, argv[1]);
	if (strchr(pgm, '.') == NULL)
		strcat(pgm, ".tny");
	source = fopen(pgm, "r");
	if (source == NULL)
	{
		fprintf(stderr, "File %s not found\n", pgm);
		exit(1);
	}
	// 将列表文件设为标准输出
	listing =  stdout;
	fprintf(listing, "\nTINY COMPILATION: %s\n", pgm);
#if NO_PARSE
	while (getToken() != ENDFILE);
#else
	syntaxTree = parse();
	if (TraceParse)
	{
		fprintf(listing, "\nSyntax tree:\n");
		printTree(syntaxTree);
	}
#if !NO_ANALYZE
	if (!Error)
	{
		fprintf(listing, "\nBuilding Symbol Table...\n");
		buildSymtab(syntaxTree);
		fprintf(listing, "\nChecking Types...\n");
		typeCheck(syntaxTree);
		fprintf(listing, "\nType Checking Finished\n");
	}
#if !NO_CODE
	if (!Error)
	{
		char* codefile;
		int fnlen = strcspn(pgm, ".");
		codefile = (char*)calloc(fnlen + 4, sizeof(char));
		strncpy(codefile, pgm, fnlen);
		strcat(codefile, ".tm");
		code = fopen(codefile, "w");
		if (code == NULL)
		{
			printf("Unable to open %s\n", codefile);
			exit(1);
		}
		codeGen(syntaxTree, codefile);
		fclose(code);
	}
#endif
#endif
#endif
	return 0;
}