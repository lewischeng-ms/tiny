// *本编译器及TM机源程序是从Kenneth C. Louden的《编译原理及实践》一书中的版本改写而来
// *仅用于学习目的，特此声明！
// 文件: tm.cpp 
// 作者: 成立
// 日期: 2010/3/20
// 描述: TINY 计算机

#pragma warning(disable:4996)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// 常量
#define IADDR_SIZE 1024 // 已为大程序扩大
#define DADDR_SIZE 1024 // 已为大程序扩大
#define NO_REGS 8
#define PC_REG  7

#define LINESIZE 121
#define WORDSIZE 20

// 类型定义
typedef enum {
	opclRR, // 寄存器操作数 r,s,t
	opclRM, // 寄存器 r, 内存 d + s
	opclRA  // 寄存器 r, 整数 d + s
} OPCLASS;

typedef enum {
	// RR指令
	opHALT, // RR 停机, 操作数被忽略
	opIN,   // RR 读入寄存器(r); s和t被忽略
	opOUT,  // RR 从寄存器(r)写, s和t被忽略
	opADD,  // RR 寄存器r = s + t
	opSUB,  // RR 寄存器r = s - t
	opMUL,  // RR 寄存器r = s * t
	opDIV,  // RR 寄存器r = s / t
	opRRLim, // RR操作的个数

	// RM指令
	opLD, // RM 寄存器(r) = 内存(d + 寄存器(s))
	opST, // RM 内存(d + 寄存器(s)) = 寄存器(r)
	opRMLim, // RM操作的个数

	// RA指令
	opLDA, // RA 寄存器(r) = d + 寄存器(s)
	opLDC, // RA 寄存器(r)= d; 寄存器(s)被忽略
	opJLT, // RA 如果寄存器(r) < 0那么寄存器(7) = d + 寄存器(s)
	opJLE, // RA 如果寄存器(r) <= 0那么寄存器(7) = d + 寄存器(s)
	opJGT, // RA 如果寄存器(r) > 0那么寄存器(7) = d + 寄存器(s)
	opJGE, // RA 如果寄存器(r) >= 0那么寄存器(7) = d + 寄存器(s)
	opJEQ, // RA 如果寄存器(r) == 0那么寄存器(7) = d + 寄存器(s)
	opJNE, // RA 如果寄存器(r) != 0那么寄存器(7) = d + 寄存器(s)
	opRALim // RA操作的个数
} OPCODE;

typedef enum {
	srOKAY,
	srHALT,
	srIMEM_ERR,
	srDMEM_ERR,
	srZERODIVIDE
} STEPRESULT;

typedef struct {
	int iop;
	int iarg1;
	int iarg2;
	int iarg3;
} INSTRUCTION;

// 变量
int iloc = 0;
int dloc = 0;
bool traceflag = false;
bool icountflag = false;

INSTRUCTION iMem[IADDR_SIZE];
int dMem[DADDR_SIZE];
int reg[NO_REGS];

char* opCodeTab[] = {
	// RR操作
	"HALT", "IN", "OUT", "ADD", "SUB", "MUL", "DIV", "????",
	// RM操作
    "LD","ST","????",
	// RA操作
    "LDA","LDC","JLT","JLE","JGT","JGE","JEQ","JNE","????"
};

char* stepResultTab[] = {
	"OK",
	"Halted","Instruction Memory Fault",
    "Data Memory Fault","Division by 0"
};

char pgmName[20];
FILE* pgm;

char in_Line[LINESIZE];
int lineLen;
int inCol;
int num;
char word[WORDSIZE];
char ch;
int done;

int opClass(int c)
{
	if (c <= opRRLim)
		return opclRR;
	else if (c <= opRMLim)
		return opclRM;
	else
		return opclRA;
}

void writeInstruction(int loc)
{
	printf("%5d: ", loc);
	if (loc >= 0 && loc < IADDR_SIZE)
	{
		printf("%6s%3d,", opCodeTab[iMem[loc].iop], iMem[loc].iarg1);
		switch (opClass(iMem[loc].iop))
		{
		case opclRR:
			printf("%1d,%1d", iMem[loc].iarg2, iMem[loc].iarg3);
			break;
		case opclRM:
		case opclRA:
			printf("%3d(%1d)", iMem[loc].iarg2, iMem[loc].iarg3);
			break;
		}
		printf("\n");
	}
}

void getCh(void)
{
	if (++inCol < lineLen)
		ch = in_Line[inCol];
	else
		ch = ' ';
}

bool nonBlank(void)
{
	while (inCol < lineLen && in_Line[inCol] == ' ')
		inCol++;
	if (inCol < lineLen)
	{
		ch = in_Line[inCol];
		return true;
	}
	else
	{
		ch = ' ';
		return false;
	}
}

bool getNum(void)
{
	int sign;
	int term;
	bool temp = false;
	num = 0;
	do {
		sign = 1;
		while (nonBlank() && (ch == '+' || ch == '-'))
		{
			temp = false;
			if (ch == '-')
				sign = - sign;
			getCh();
		}
		term = 0;
		nonBlank();
		while (isdigit(ch))
		{
			temp = true;
			term = term * 10 + (ch - '0');
			getCh();
		}
		num = num + (term * sign);
	} while (nonBlank() && (ch == '+' || ch == '-'));
	return temp;
}

bool getWord(void)
{
	bool temp = false;
	int length = 0;
	if (nonBlank())
	{
		while (isalnum(ch))
		{
			if(length < WORDSIZE - 1)
				word[length++] = ch;
			getCh();
		}
		word[length] = '\0';
		temp = (length != 0);
	}
	return temp;
}

bool skipCh(char c)
{
	bool temp = false;
	if (nonBlank() && ch == c)
	{
		getCh();
		temp = true;
	}
	return temp;
}

int atEOL(void)
{
	return !nonBlank();
}

bool error(char * msg, int lineNo, int instNo)
{
	printf("Line %d", lineNo);
	if (instNo >= 0)
		printf("(Instruction %d)", instNo);
	printf("   %s\n", msg);
	return false;
}

bool readInstructions(void)
{
	int op;
	int arg1, arg2, arg3;
	int loc, regNo, lineNo;
	for (regNo = 0; regNo < NO_REGS; regNo++)
		reg[regNo] = 0;
	dMem[0] = DADDR_SIZE - 1;
	for (loc = 1; loc < DADDR_SIZE; loc++)
		dMem[loc] = 0;
	for (loc = 0; loc < IADDR_SIZE; loc++)
	{
		iMem[loc].iop = opHALT;
		iMem[loc].iarg1 = 0;
		iMem[loc].iarg2 = 0;
		iMem[loc].iarg3 = 0;
	} 
	lineNo = 0;
	while (!feof(pgm))
	{
		fgets(in_Line, LINESIZE-2, pgm);
		inCol = 0; 
		lineNo++;
		lineLen = strlen(in_Line) - 1;
		if (in_Line[lineLen] == '\n')
			in_Line[lineLen] = '\0';
		else
			in_Line[++lineLen] = '\0';
		if ( nonBlank() && in_Line[inCol] != '*')
		{
			if (!getNum())
				return error("Bad location", lineNo, -1);
			loc = num;
			if (loc > IADDR_SIZE)
				return error("Location too large", lineNo, loc);
			if (!skipCh(':'))
				return error("Missing colon", lineNo, loc);
			if (!getWord())
				return error("Missing opcode", lineNo, loc);
			op = opHALT;
			while (op < opRALim && strncmp(opCodeTab[op], word, 4))
				op++;
			if (strncmp(opCodeTab[op], word, 4))
				return error("Illegal opcode", lineNo,loc);
			switch(opClass(op))
			{
			case opclRR:
				if (!getNum() || num < 0 || num >= NO_REGS)
					return error("Bad first register", lineNo, loc);
				arg1 = num;
				if (!skipCh(','))
					return error("Missing comma", lineNo, loc);
				if (!getNum() || num < 0 || num >= NO_REGS)
					return error("Bad second register", lineNo, loc);
				arg2 = num;
				if (!skipCh(','))
					return error("Missing comma", lineNo, loc);
				if(!getNum() || num < 0 || num >= NO_REGS)
					return error("Bad third register", lineNo, loc);
				arg3 = num;
				break;
			case opclRM:
			case opclRA:
				if (!getNum() || num < 0 || num >= NO_REGS)
					return error("Bad first register", lineNo, loc);
				arg1 = num;
				if (!skipCh(','))
					return error("Missing comma", lineNo,loc);
				if (!getNum())
					return error("Bad displacement", lineNo,loc);
				arg2 = num;
				if (!skipCh('(') && !skipCh(','))
					return error("Missing LParen", lineNo, loc);
				if (!getNum() || num < 0 || num >= NO_REGS)
					return error("Bad second register", lineNo, loc);
				arg3 = num;
				break;
			}
			iMem[loc].iop = op;
			iMem[loc].iarg1 = arg1;
			iMem[loc].iarg2 = arg2;
			iMem[loc].iarg3 = arg3;
		}
	}
	return true;
}

STEPRESULT stepTM(void)
{
	INSTRUCTION currentinstruction;
	int pc;
	int r, s, t, m;
	int ok;

	pc = reg[PC_REG];
	if (pc < 0 || pc > IADDR_SIZE)
		return srIMEM_ERR;
	reg[PC_REG] = pc + 1;
	currentinstruction = iMem[pc];
	switch (opClass(currentinstruction.iop))
	{
	case opclRR:
		r = currentinstruction.iarg1;
		s = currentinstruction.iarg2;
		t = currentinstruction.iarg3;
		break;
	case opclRM:
		r = currentinstruction.iarg1;
		s = currentinstruction.iarg3;
		m = currentinstruction.iarg2 + reg[s];
		if (m < 0 || m > DADDR_SIZE)
			return srDMEM_ERR;
		break;
	case opclRA:
		r = currentinstruction.iarg1;
		s = currentinstruction.iarg3;
		m = currentinstruction.iarg2 + reg[s];
		break;
	}
	switch (currentinstruction.iop)
	{
	// RR指令
	case opHALT:
		printf("HALT: %1d,%1d,%1d\n", r, s, t);
		return srHALT;
		// break;
	case opIN:
		do {
			printf("Enter value for IN instruction: ");
			fflush(stdin);
			fflush(stdout);
			gets(in_Line);
			lineLen = strlen(in_Line);
			inCol = 0;
			ok = getNum();
			if (!ok)
				printf("Illegal value\n");
			else
				reg[r] = num;
		} while (!ok);
		break;
	case opOUT:  
		printf("OUT instruction prints: %d\n", reg[r]);
		break;
	case opADD:
		reg[r] = reg[s] + reg[t];
		break;
	case opSUB:
		reg[r] = reg[s] - reg[t];
		break;
	case opMUL:
		reg[r] = reg[s] * reg[t];
		break;
	case opDIV:
		if (reg[t] != 0)
			reg[r] = reg[s] / reg[t];
		else
			return srZERODIVIDE;
		break;
	// RM指令
	case opLD:
		reg[r] = dMem[m];
		break;
	case opST:
		dMem[m] = reg[r];
		break;
	// RA指令
	case opLDA:
		reg[r] = m;
		break;
	case opLDC:
		reg[r] = currentinstruction.iarg2;
		break;
	case opJLT:
		if (reg[r] < 0)
			reg[PC_REG] = m;
		break;
	case opJLE:
		if (reg[r] <= 0)
			reg[PC_REG] = m;
		break;
	case opJGT:
		if (reg[r] > 0)
			reg[PC_REG] = m;
		break;
	case opJGE:
		if (reg[r] >= 0)
			reg[PC_REG] = m;
		break;
	case opJEQ:
		if (reg[r] == 0)
			reg[PC_REG] = m;
		break;
	case opJNE:
		if (reg[r] != 0)
			reg[PC_REG] = m;
		break;
	}
	return srOKAY;
}

bool doCommand(void)
{
	char cmd;
	int stepcnt = 0, i;
	int printcnt;
	int stepResult;
	int regNo, loc;
	do {
		printf("Enter command: ");
		fflush(stdin);
		fflush(stdout);
		gets(in_Line);
		lineLen = strlen(in_Line);
		inCol = 0;
	}  while (!getWord());
	cmd = word[0];
	switch (cmd)
	{
	case 't': 
		traceflag = !traceflag;
		printf("Tracing now ");
		if (traceflag)
			printf("on.\n");
		else
			printf("off.\n");
		break;
	case 'h':
		printf("Commands are:\n");
		printf("   s(tep <n>      Execute n(default 1)TM instructions\n");
		printf("   g(o            Execute TM instructions until HALT\n");
		printf("   r(egs          Print the contents of the registers\n");
		printf("   i(Mem <b <n>>  Print n iMem locations starting at b\n");
		printf("   d(Mem <b <n>>  Print n dMem locations starting at b\n");
		printf("   t(race         Toggle instruction trace\n");
		printf("   p(rint         Toggle print of total instructions executed('go' only)\n");
		printf("   c(lear         Reset simulator for new execution of program\n");
		printf("   h(elp          Cause this list of commands to be printed\n");
		printf("   q(uit          Terminate the simulation\n");
		break;
	case 'p':
		icountflag = !icountflag;
		printf("Printing instruction count now ");
		if (icountflag)
			printf("on.\n");
		else
			printf("off.\n");
		break;
	case 's':
		if (atEOL())
			stepcnt = 1;
		else if (getNum())
			stepcnt = abs(num);
		else
			printf("Step count?\n");
		break;
	case 'g':
		stepcnt = 1;
		break;
	case 'r':
		for (i = 0; i < NO_REGS; i++)
		{
			printf("%1d: %4d    ", i, reg[i]);
			if (i % 4 == 3)
				printf("\n");
		}
		break;
	case 'i':
		printcnt = 1;
		if (getNum())
		{
			iloc = num;
			if (getNum())
				printcnt = num;
		}
		if (!atEOL())
			printf("Instruction locations?\n");
		else
		{
			while(iloc >= 0 && iloc < IADDR_SIZE && printcnt > 0)
			{
				writeInstruction(iloc);
				iloc++;
				printcnt--;
			}
		}
		break;
	case 'd':
		printcnt = 1;
		if (getNum ())
		{
			dloc = num;
			if (getNum())
				printcnt = num;
		} 
		if (!atEOL())
			printf("Data locations?\n");
		else 
		{
			while(dloc >= 0 && dloc < DADDR_SIZE && printcnt > 0)
			{
				printf("%5d: %5d\n", dloc, dMem[dloc]);
				dloc++;
				printcnt--;
			} 
		} 
		break;
	case 'c':
		iloc = 0;
		dloc = 0;
		stepcnt = 0;
		for (regNo = 0; regNo < NO_REGS; regNo++)
			reg[regNo] = 0;
		dMem[0] = DADDR_SIZE - 1;
		for (loc = 1; loc < DADDR_SIZE; loc++)
			dMem[loc] = 0;
		break;
	case 'q':
		return false;
		// break;
	default:
		printf("Command %c unknown.\n", cmd);
		break;
	}
	stepResult = srOKAY;
	if (stepcnt > 0)
	{
		if (cmd == 'g')
		{
			stepcnt = 0;
			while (stepResult == srOKAY)
			{
				iloc = reg[PC_REG];
				if (traceflag)
					writeInstruction(iloc);
				stepResult = stepTM();
				stepcnt++;
			}
		if (icountflag)
			printf("Number of instructions executed = %d\n", stepcnt);
		} 
		else 
		{
			while(stepcnt > 0 && stepResult == srOKAY)
			{
				iloc = reg[PC_REG];
				if (traceflag)
					writeInstruction(iloc);
				stepResult = stepTM();
				stepcnt--;
			} 
		}
		printf("%s\n",stepResultTab[stepResult]);
	}
	return true;
}

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		printf("usage: %s <filename>\n", argv[0]);
		exit(1);
	} 
	strcpy(pgmName,argv[1]);
	if(strchr(pgmName, '.')== NULL)
		strcat(pgmName, ".tm");
	pgm = fopen(pgmName, "r");
	if(pgm == NULL)
	{
		printf("file '%s' not found\n",pgmName);
		exit(1);
	} 
	// 读入程序
	if(!readInstructions())
		exit(1);
	// 切换输入文件到终端
	// reset(input);
	// read-eval-print
	printf("TM  simulation(enter h for help)...\n");
	do {
		done = !doCommand();
	} while(!done);
	printf("Simulation done.\n");
	return 0;
}
 
