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

    std::unordered_map<std::string_view, Value* > symbol_table;

    const Module &getModule() const { return _module; }

private:
    Module _module;

    void traverse(NodePtr node);

    std::unordered_map<std::string_view, Value*> global_symbol_map;

    BasicBlock *translate_stmt(NodePtr node, BasicBlock *current_bb, std::unordered_map<std::string_view, Value*>* symbol_table);

    Value *translate_expr(NodePtr node, BasicBlock *current_bb, std::unordered_map<std::string_view, Value*>* symbol_table);
};


#endif //_TRANSLATE_H_
