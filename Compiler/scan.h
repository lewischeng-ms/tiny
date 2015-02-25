// *仅用于学习目的，特此声明！
// 文件: scan.h
// 作者: 成立
// 日期: 2010/3/20
// 描述: TINY编译器的扫描器接口

#ifndef _SCAN_H_
#define _SCAN_H_

// 记号的最大长度
#define MAXTOKENLEN 40

// 存储每个记号的词素（lexeme，词素即该记号的字符构成）
extern char tokenString[MAXTOKENLEN + 1];

// 返回源文件中的下一个记号
TokenType getToken(void);

#endif