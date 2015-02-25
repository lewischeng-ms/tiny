// *仅用于学习目的，特此声明！
// 文件: util.h
// 作者: 成立
// 日期: 2010/3/20
// 描述: TINY编译器的实用函数

#ifndef _UTIL_H_
#define _UTIL_H_

// 输出记号及其词素至列表文件
void printToken(TokenType, const char*);

// 为语法树创建新的语句节点
TreeNode* newStmtNode(StmtKind);

// 为语法树创建新的表达式节点
TreeNode* newExpNode(ExpKind);

// 分配并创建某现有字符串的一份拷贝
char* copyString(char*);

// 输出语法树至列表文件（利用缩进表达子树）
void printTree(TreeNode*);

#endif