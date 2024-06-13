//
// Created by MaHong on 2024/6/12.
//

#ifndef _TRANSLATE_H_
#define _TRANSLATE_H_

#include "ast/ast.h"
#include "ir/ir.h"
#include <unordered_map>
#include <vector>

class translate {
public:
    explicit translate(NodePtr root);

    ~translate() = default;

    const Module &getModule() const { return _module; }

private:
    Module _module;

    void traverse(NodePtr node);

    BasicBlock *translate_stmt(NodePtr stmt, Function *func, std::unordered_map<std::string_view, Value*> symbol_table);

    BasicBlock *translate_stmt(NodePtr stmt, BasicBlock *current_bb, std::unordered_map<std::string_view, Value*> symbol_table);

    Value *translate_expr(NodePtr expr, BasicBlock *current_bb, std::unordered_map<std::string_view, Value*> symbol_table);
};


#endif //_TRANSLATE_H_
