#include "ast/ast.h"

#include <fmt/core.h>
#include <cassert>

// 辅助函数，将操作符类型转换为字符串表示
static std::string op_str(OpType op_type) {
    switch (op_type) {
#define OpcodeDefine(x, s)     \
        case x: return s;
#include "common/common.def"
    default:
        return "<unknown>";
    }
}

// 打印表达式的函数（要改的）
void print_expr(CompUnit exp, std::string prefix, std::string ident) {
    
}
