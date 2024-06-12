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

    Module _module;

    void traverse(NodePtr node);

    BasicBlock *translate_stmt(NodePtr stmt);

    Value *translate_expr(NodePtr expr);
};


#endif //_TRANSLATE_H_
