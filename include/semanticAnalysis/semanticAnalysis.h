#ifndef __semanticAnalysis_HH__
#define __semanticAnalysis_HH__

#include<string>
#include<vector>
#include<iostream>
#include<map>
#include<algorithm>
#include "ast/ast.h"


// 定义符号表的栈，其中每个元素是一个映射，从字符串映射到整数向量
static std::vector<std::map<std::string, std::vector<int>>> symbol_table_var;

void reportError(const std::string& error_msg);
// Enter a new scope
void enterScope();
// Exit a scope
void exitScope();

enum ExprType {
    TYPE_INT,
    TYPE_VOID,
    TYPE_UNKNOWN  // 处理未知类型或者错误处理
};

int semanticAnalysis(NodePtr node);
#endif