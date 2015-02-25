// *仅用于学习目的，特此声明！
// 文件: analyze.h
// 作者: 成立
// 日期: 2010/3/20
// 描述: TINY编译器的语义分析器接口

#ifndef _ANALYZE_H_
#define _ANALYZE_H_

// 前序遍历语法树并构造符号表
void buildSymtab(TreeNode*);

// 后序遍历语法树并执行类型检查
void typeCheck(TreeNode*);

#endif