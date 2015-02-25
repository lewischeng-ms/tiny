// *仅用于学习目的，特此声明！
// 文件: cgen.h
// 作者: 成立
// 日期: 2010/3/20
// 描述: TINY编译器的代码生成器接口

#ifndef _CGEN_H_
#define _CGEN_H_

// 通过遍历语法树产生代码文件
// 参数codefile指定代码文件的文件名，并被用于注释中的文件名
void codeGen(TreeNode* syntaxTree, char* codefile);

#endif