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

void print_expr(NodePtr exp, std::string prefix) {
    if (exp == nullptr) {
        return;
    }

    switch (exp->node_type) {
        case ND_CompUnit: {
            std::cout << "CompUnit" << exp->node_type << std::endl;
            auto comp_unit = exp->as<CompUnit*>();
            for (auto child : comp_unit->children) {
                print_expr(child,prefix + "  ");
                // std::cout << prefix.length() << std::endl;
            }
            break;
        }
        case ND_FuncDef: {

            std::cout << prefix << "FuncDef " << exp->as<FuncDef*>()->func_name << " '" << exp->as<FuncDef*>()->return_type << "(";
            auto params = exp->as<FuncDef*>()->params;
            if (params != nullptr) {
                print_expr(params, "");
            }
            std::cout << ")'" << std::endl;
            print_expr(exp->as<FuncDef*>()->body, prefix + "  ");
            break;
        }
        case ND_FuncFParams: {
            auto func_fparams = exp->as<FuncFParams*>();
            for (auto child : func_fparams->children) {
                print_expr(child, "");
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
                print_expr(numlist, "");
            }
            break;
        }
        case ND_Block: {
            std::cout << prefix << "Block" << std::endl;

            BlockItemList* list = exp->as<Block*>()->itemList->as<BlockItemList*>();
            for (auto child : list->children) {
                NodePtr ptr = child->as<BlockItem*>()->item;
                if (ptr->node_type == ND_Stmt){
                    print_expr(ptr, prefix + "  ");
                } else if (ptr->node_type == ND_Decl){
                    print_expr(ptr, prefix + "  ");
                }
            }
            break;
        }
        case ND_Stmt: {
            // std::cout << "Stmt" << std::endl;
            print_expr(exp->as<Stmt*>()->stmtPtr,prefix);
            break;
        }
        case ND_Decl: {
            // std::cout << "Decl" << std::endl;
            print_expr(exp->as<Decl*>()->vardecl,prefix);
            break;
        }
        case ND_VarDecl: {
            std::cout << prefix << "VarDecl" << std::endl;
            VarDefList* list= exp->as<VarDecl*>()->vardeflist->as<VarDefList*>();
            for (auto child : list->children) {
                if (child == nullptr) break;
                print_expr(child, prefix + "  ");
            }
            break;
        }
        case ND_VarDef: {
            std::cout << prefix <<"Ident " << exp->as<VarDef*>()->var_name << std::endl;
            NodePtr init_value = exp->as<VarDef*>()->init_value;
            print_expr(init_value,prefix + "  ");
            // if (init_value != nullptr) {
            //     NodePtr ptr = init_value->as<InitVal*>()->exp->as<Exp*>()->exp;
            //     print_expr(ptr, prefix);
            // } else {
            //     break;
            // }
            break;
        }
        case ND_InitVal: {
            std::cout << prefix <<"InitVal " << std::endl;
            // NodePtr ptr = exp->as<InitVal*>()->exp;
            // print_expr(exp ,prefix + "  ");
            break;
        }
        case ND_LVal: {
            std::cout << prefix << "Ident " << exp->as<Lval*>()->ident_name << std::endl;
            auto lvalexplist = exp->as<Lval*>()->lvalexplist;
            if (lvalexplist != nullptr) {
                print_expr(lvalexplist, prefix);
            }
            break;
        }
        case ND_LValExpList: {
            auto lval_exp_list = exp->as<LValExpList*>();
            for (auto child : lval_exp_list->children) {
                print_expr(child, "");
                if (child != lval_exp_list->children.back()) {
                    std::cout << ", ";
                }
            }
            break;
        }
        case ND_AssignStmt: {
            std::cout << prefix << "AssignStmt" << std::endl;
            print_expr(exp->as<AssignStmt*>()->lhs, prefix + "  ");
            print_expr(exp->as<AssignStmt*>()->rhs, prefix + "  ");
            break;
        }
        case ND_ReturnStmt: {
            std::cout << prefix << "ReturnStmt" << std::endl;
            print_expr(exp->as<ReturnStmt*>()->exp, prefix + "  ");
            break;
        }
        case ND_IfStmt: {
            std::cout << prefix << "IfStmt" << std::endl;
            print_expr(exp->as<IfStmt*>()->condition, prefix + "  ");
            print_expr(exp->as<IfStmt*>()->then_stmt, prefix + "  ");
            print_expr(exp->as<IfStmt*>()->else_stmt, prefix + "  ");
            break;
        }
        case ND_WhileStmt: {
            std::cout << prefix << "WhileStmt" << std::endl;
            print_expr(exp->as<WhileStmt*>()->condition, prefix + "  ");
            print_expr(exp->as<WhileStmt*>()->body, prefix + "  ");
            break;
        }
        case ND_BreakStmt: {
            std::cout << prefix << "BreakStmt" << std::endl;
            break;
        }
        case ND_ContinueStmt: {
            std::cout << prefix << "ContinueStmt" << std::endl;
            break;
        }
        case ND_IntegerLiteral: {
            std::cout << prefix << "IntConst " << exp->as<IntegerLiteral*>()->value << std::endl;
            break;
        }
        case ND_FuncRParams: {
            std::cout << "FuncRParams" << std::endl;
            for (auto child : exp->as<FuncRParams*>()->children) {
                print_expr(child, prefix + "  ");
            }
            break;
        }
        case ND_Exp: {
            print_expr(exp->as<Exp*>()->exp, prefix);
            break;
        }
        case ND_PrimaryExp: {
            NodePtr ptr = exp->as<PrimaryExp*>()->pri_exp;
            print_expr(ptr, prefix);
            break;
        }
        case ND_BinaryExp: {
            std::cout << prefix << "BinaryOp " << op_str(exp->as<BinaryExp*>()->op) << std::endl;
            print_expr(exp->as<BinaryExp*>()->lhs, prefix + "  ");
            print_expr(exp->as<BinaryExp*>()->rhs, prefix + "  ");
            break;
        }
        case ND_UnaryExp: {
            // std::cout << "UnaryExp " << std::endl;
            if (exp->as<UnaryExp*>()->primaryexp != nullptr) {
                print_expr(exp->as<UnaryExp*>()->primaryexp, prefix);
                break;
            } else if (exp->as<UnaryExp*>()->ident_name != "") {
                break;
            } else if (exp->as<UnaryExp*>()->operand != nullptr) {
                break;
            }

            break;
        }

        default:
            break;
    }

    return;
}

