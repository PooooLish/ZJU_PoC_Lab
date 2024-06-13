#include "translate/translate.h"
#include "ast/ast.h"
#include <fstream>
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <string_view>

translate::translate(NodePtr root) {
    // Initialize the module (if any initialization is needed)
    // Start the translation process by traversing the AST starting from the root
    traverse(root);

    // Open the output file
    std::ofstream outFile("module_output.txt");
    if (!outFile.is_open()) {
        std::cerr << "Error opening file for writing" << std::endl;
        return;
    }

    // Print the module content to the file
    _module.print(outFile, true);  // Assuming `print` method accepts an output stream and a boolean flag

    // Close the file
    outFile.close();

    std::cout << "Module printed to module_output.txt" << std::endl;
}

void translate::traverse(NodePtr node) {
    // Traverse the AST and translate each statement and expression
    // This is a recursive function that visits each node in the AST
    // For each node, you need to determine the type and call the appropriate translation function

    switch (node->node_type) {
        case ND_CompUnit: {
            auto compUnit = node->as<CompUnit*>();
            for (auto child : compUnit->children) {
                if (child->node_type == ND_Decl) {
                    std::cout<< "decl" <<std::endl;
                } else if (child->node_type == ND_FuncDef) {
                    std::cout<< "funcdef" <<std::endl;
                }

                traverse(child);
            }
            break;
        }
        case ND_Block: {
            break;
        }
        case ND_BlockItemList: {
            break;
        }
        case ND_BlockItem: {
            break;
        }
        case ND_Stmt: {
            break;
        }
        case ND_IfStmt: {
            break;
        }
        case ND_ReturnStmt: {
            break;
        }
        case ND_WhileStmt: {
            break;
        }
        case ND_AssignStmt: {
            break;
        }
        case ND_Decl: {
            traverse(node->as<Decl*>()->vardecl);
            break;
        }
        case ND_InitVal: {
            break;
        }
        case ND_Exp: {
            break;
        }
        case ND_PrimaryExp: {
            break;
        }
        case ND_VarDecl: {



            break;
        }
        case ND_VarDefList: {
            break;
        }
        case ND_VarDef: {
            break;
        }
        case ND_FuncFParams: {
            break;
        }
        case ND_FuncFParam: {
            break;
        }
        case ND_FuncDef: {

            struct FuncDef* temp = node->as<FuncDef*>();

            std::string_view return_type = temp->return_type;
            std::string_view func_name = temp->func_name;
            NodePtr params = temp->params;
            NodePtr body = temp->body;

            std::cout << return_type << "  " << func_name << std::endl;

            Type *returnType;

            if (return_type == "int") {
                returnType = Type::getIntegerTy();
            } else if (return_type == "void") {
                returnType = Type::getUnitTy();
            }

            // 定义参数类型列表，每个参数都是 int
            std::vector<Type *> paramTypes = { Type::getIntegerTy(), Type::getIntegerTy() };

            // 创建 FunctionType 实例
            FunctionType *funcType = FunctionType::get(returnType, paramTypes);

            Function *func = Function::Create(funcType, false, func_name , &_module);

//            std::cout << "Function return type ID: " << funcType->getReturnType()->getTypeID() << std::endl;
//            std::cout << "Number of parameters: " << funcType->getNumParams() << std::endl;
//
//            for (unsigned i = 0; i < funcType->getNumParams(); ++i) {
//                std::cout << "Parameter " << i << " type ID: " << funcType->getParamType(i)->getTypeID() << std::endl;
//            }

            break;
        }
        case ND_BinaryExp: {
            break;
        }
        case ND_LVal: {
            break;
        }
        case ND_LValExpList: {
            break;
        }
        case ND_UnaryExp: {
            break;
        }
        default:
            break;
    }

    return;
}

BasicBlock *translate::translate_stmt(NodePtr stmt, Function *func, std::unordered_map<std::string_view, Value*> symbol_table) {
    // Translate a statement node into an IR basic block
    // The actual implementation depends on your statement node structure
    // and the IR representation you are targeting

    // Example (pseudocode):
    BasicBlock *BB = BasicBlock::Create();
    // Translate the statement into IR instructions and add them to the block
    // ...

    return BB;
}

BasicBlock *translate::translate_stmt(NodePtr stmt, BasicBlock *current_bb, std::unordered_map<std::string_view, Value*> symbol_table) {
    // Translate a statement node into an IR basic block
    // The actual implementation depends on your statement node structure
    // and the IR representation you are targeting

    // Example (pseudocode):
    BasicBlock *BB = BasicBlock::Create();
    // Translate the statement into IR instructions and add them to the block
    // ...

    return BB;
}

Value *translate::translate_expr(NodePtr expr, BasicBlock *current_bb, std::unordered_map<std::string_view, Value*> symbol_table) {
    // Translate an expression node into an IR value
    // The actual implementation depends on your expression node structure
    // and the IR representation you are targeting

    // Example (pseudocode):
    Value *value = nullptr;
    // Translate the expression into an IR value
    // ...

    return value;
}
