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

// print_expr是用于打印ast树的函数
// 如：当我输入的文件内容如下时
// int factorial(int n) {
//     if (n == 0)
//         return 1;
//     return n * factorial(n - 1);
// }
// int main() {
//     int n = getint();
//     int result = factorial(n);
//     putint(result);
//     return 0;
// }
// 通过print_expr可以生成这样一个ast树：
//  CompUnit
//  ├─ FuncDef factorial 'int(int)'
//  │  ├─ FuncFParam n 'int'
//  │  └─ Block
//  │     ├─ IfStmt
//  │     │  ├─ RelationOp ==
//  │     │  │  ├─ Ident n
//  │     │  │  └─ IntConst 0
//  │     │  └─ ReturnStmt
//  │     │     └─ IntConst 1
//  │     └─ ReturnStmt
//  │        └─ BinaryOp *
//  │           ├─ Ident n
//  │           └─ Call factorial
//  │              └─ BinaryOp -
//  │                 ├─ Ident n
//  │                 └─ IntConst 1
//  └─ FuncDef main 'int()'
//     └─ Block
//        ├─ VarDecl
//        │  └─ Ident n
//        │     └─ Call getint
//        ├─ VarDecl
//        │  └─ Ident result
//        │     └─ Call factorial
//        │        └─ Ident n
//        ├─ ExpStmt
//        │  └─ Call putint
//        │     └─ Ident result
//        └─ ReturnStmt
//           └─ IntConst 0
void print_expr(NodePtr exp, std::string prefix, std::string ident) {
    if (exp == nullptr) {
        return;
    }
    std::cout << prefix;
    if (ident != "") {
        std::cout << ident << " ";
    }

    switch (exp->node_type) {
        case ND_CompUnit: {
            std::cout << "CompUnit" << std::endl;
            auto comp_unit = exp->as<CompUnit*>();
            for (auto child : comp_unit->children) {
                print_expr(child, prefix + "|", "--"); 
            }
            break;
        }
        case ND_FuncDef: {
            std::cout << "FuncDef " << exp->as<FuncDef*>()->func_name << " '" << exp->as<FuncDef*>()->return_type << "(";
            auto params = exp->as<FuncDef*>()->params;
            if (params != nullptr) {
                print_expr(params, "", "");
            }
            std::cout << ")'" << std::endl;
            print_expr(exp->as<FuncDef*>()->body, prefix + "   |", "--");
            break;
        }
        case ND_FuncFParams: {
            auto func_fparams = exp->as<FuncFParams*>();
            for (auto child : func_fparams->children) {
                print_expr(child, "", "");
                if (child != func_fparams->children.back()) {
                    std::cout << ", ";
                }
            }
            break;
        }
        case ND_FuncFParam: {
            std::cout << exp->as<FuncFParam*>()->param_name << " '" << exp->as<FuncFParam*>()->param_type << "'";
            auto numlist = exp->as<FuncFParam*>()->numlist;
            if (numlist != nullptr) {
                print_expr(numlist, "", "");
            }
            break;
        }
        case ND_Block: {
            std::cout << "Block" << std::endl;

            BlockItemList* list = exp->as<Block*>()->itemList->as<BlockItemList*>();
            for (auto child : list->children) {
                NodePtr ptr = child->as<BlockItem*>()->item;
                // print_expr(child, prefix, "");
                if (ptr->node_type == ND_Stmt)
                    print_expr(ptr->as<Stmt*>()->stmtPtr, prefix + "   |", "--");
                else if (ptr->node_type == ND_Decl)
                    print_expr(ptr->as<Decl*>()->vardecl, prefix + "   |", "--");
            }
            break;
        }
        case ND_VarDecl: {
            std::cout << "VarDecl" << std::endl;
            VarDefList* list= exp->as<VarDecl*>()->vardeflist->as<VarDefList*>();
            for (auto child : list->children) {
                std::cout << list->children.size() << std::endl;
                print_expr(child, prefix + "   |", "--");
            }
            break;
        }
        case ND_VarDef: {
            std::cout << "Ident " << exp->as<VarDef*>()->var_name << std::endl;
            auto init_value = exp->as<VarDef*>()->init_value;
            if (init_value != nullptr) {
                NodePtr ptr = init_value->as<InitVal*>()->exp->as<Exp*>()->exp;
                print_expr(ptr, prefix, "--");
            } else {

            }
            break;
        }
        case ND_LVal: {
            std::cout << "LVal " << exp->as<Lval*>()->ident_name << std::endl;
            auto lvalexplist = exp->as<Lval*>()->lvalexplist;
            if (lvalexplist != nullptr) {
                print_expr(lvalexplist, prefix + "│  ", "");
            }
            break;
        }
        case ND_LValExpList: {
            auto lval_exp_list = exp->as<LValExpList*>();
            for (auto child : lval_exp_list->children) {
                print_expr(child, "", "");
                if (child != lval_exp_list->children.back()) {
                    std::cout << ", ";
                }
            }
            break;
        }
        case ND_AssignStmt: {
            std::cout << "AssignStmt" << std::endl;
            print_expr(exp->as<AssignStmt*>()->lhs, prefix + "│  ", "├─ ");
            print_expr(exp->as<AssignStmt*>()->rhs, prefix + "│  ", "└─ ");
            break;
        }
        case ND_ReturnStmt: {
            std::cout << "ReturnStmt" << std::endl;
            print_expr(exp->as<ReturnStmt*>()->exp, prefix + "   ", "");
            break;
        }
        case ND_IfStmt: {
            std::cout << "IfStmt" << std::endl;
            print_expr(exp->as<IfStmt*>()->condition, prefix + "   ", "├─ ");
            print_expr(exp->as<IfStmt*>()->then_stmt, prefix + "   ", "├─ ");
            print_expr(exp->as<IfStmt*>()->else_stmt, prefix + "   ", "└─ ");
            break;
        }
        case ND_WhileStmt: {
            std::cout << "WhileStmt" << std::endl;
            print_expr(exp->as<WhileStmt*>()->condition, prefix + "│  ", "├─ ");
            print_expr(exp->as<WhileStmt*>()->body, prefix + "│  ", "└─ ");
            break;
        }
        case ND_BreakStmt: {
            std::cout << "BreakStmt" << std::endl;
            break;
        }
        case ND_ContinueStmt: {
            std::cout << "ContinueStmt" << std::endl;
            break;
        }
        case ND_IntegerLiteral: {
            std::cout << "IntConst " << exp->as<IntegerLiteral*>()->value << std::endl;
            break;
        }
        case ND_BType: {
            std::cout << "BType " << exp->as<BType*>()->type << std::endl;
            break;
        }
        case ND_FuncRParams: {
            std::cout << "FuncRParams" << std::endl;
            for (auto child : exp->as<FuncRParams*>()->children) {
                print_expr(child, prefix + "│  ", "");
            }
            break;
        }
        case ND_BinaryExp: {
            std::cout << "BinaryOp " << op_str(exp->as<BinaryExp*>()->op) << std::endl;
            print_expr(exp->as<BinaryExp*>()->lhs, prefix + "   ", "├─ ");
            print_expr(exp->as<BinaryExp*>()->rhs, prefix + "   ", "└─ ");
            break;
        }
        case ND_UnaryExp: {
            // std::cout << "UnaryExp " << exp->as<UnaryExp*>()->op << std::endl;
            
            auto primaryexp = exp->as<UnaryExp*>()->primaryexp;
            if (primaryexp != nullptr) {
                // std::cout << "1" << std::endl;
                NodePtr ptr = primaryexp->as<PrimaryExp*>()->pri_exp;
                if (ptr->node_type == ND_LVal){
                    std::cout << "ND_LVal" << std::endl;
                    print_expr(ptr, prefix + "  |", "--");
                } else if (ptr->node_type == ND_Exp) {
                    std::cout << "ND_Exp" << std::endl;
                    print_expr(ptr, prefix + "  |", "--");
                } else if (ptr->node_type == ND_IntegerLiteral) {
                    // std::cout << "ND_IntegerLiteral" << std::endl;
                    // print_expr(ptr, prefix , "");
                    std::cout << "IntConst " << ptr->as<IntegerLiteral*>()->value <<std::endl;
                    
                }
                // std::cout << prefix + "└─" << "IntConst" << 
                // print_expr(primaryexp, prefix + "│  ", "├─ ");
                break;
            }
            
            auto funcrparams = exp->as<UnaryExp*>()->funcrparams;
            if (funcrparams != nullptr) {
                std::cout << "2" << std::endl;
                print_expr(funcrparams, prefix + "│  ", "├─ ");
                break;
            }
            
            auto operand = exp->as<UnaryExp*>()->operand;
            if (operand != nullptr) {
                std::cout << "3" << std::endl;
                print_expr(operand, prefix + "│  ", "└─ ");
                break;
            }
            
        }
        default:
            break;
    }
}

