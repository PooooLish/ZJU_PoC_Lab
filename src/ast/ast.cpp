#include "ast/ast.h"

#include <fmt/core.h>
#include <cassert>

// 辅助函数，将操作符类型转换为字符串表示
// static std::string op_str(OpType op_type) {
//     switch (op_type) {
// #define OpcodeDefine(x, s)     \
//         case x: return s;
// #include "common/common.def"
//     default:
//         return "<unknown>";
//     }
// }

// 打印表达式的函数（要改的）
void print_expr(ExprPtr exp, std::string prefix, std::string ident) {
    // assert(exp != nullptr); // 断言：确保表达式指针不为空
    // fmt::print(prefix); // 打印前缀

    // // 如果表达式是二元表达式类型
    // if (auto *bin_op = exp->as<TreeBinaryExpr *>()) {
    //     fmt::print("BinOp \"{}\"\n", op_str(bin_op->op)); // 打印二元操作符类型
    //     // 递归打印左右子表达式
    //     print_expr(bin_op->lhs, ident + "├─ ", ident + "│  ");
    //     print_expr(bin_op->rhs, ident + "└─ ", ident + "   ");
    // }
    // // 如果表达式是一元表达式类型
    // if (auto *un_op = exp->as<TreeUnaryExpr *>()) {
    //     fmt::print("UnOp \"{}\"\n", op_str(un_op->op)); // 打印一元操作符类型
    //     // 递归打印操作数表达式
    //     print_expr(un_op->operand, ident + "└─ ", ident + "   ");
    // }
    // // 如果表达式是整数字面量类型
    // if (auto* lit = exp->as<TreeIntegerLiteral *>()) {
    //     fmt::print("Int {}\n", lit->value); // 直接打印整数值
    // }
}
