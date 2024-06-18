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

    std::unordered_map<std::string, Value*> func_symbol_table;

    std::unordered_map<std::string, Value*> global_symbol_map;

    const Module &getModule() const { return _module; }

    Module _module;

    void traverse(NodePtr node);

    void processGlobalDecl(NodePtr node);

    void processFuncDef(NodePtr node);

    BasicBlock *translate_stmt(NodePtr node, BasicBlock *current_bb, std::unordered_map<std::string, Value*>* symbol_table);

    Value *translate_expr(NodePtr node, BasicBlock *current_bb, std::unordered_map<std::string, Value*>* symbol_table);

    void printSymbolTable(const std::unordered_map<std::string, Value*>& symbol_table);

    void addValue(std::unordered_map<std::string, Value*>* symbol_table, std::string name, Value* value);

    Value* getValue(const std::unordered_map<std::string, Value*>* symbol_table, std::string name);

};


#endif //_TRANSLATE_H_
