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
                if (child->node_type == ND_Decl) { // 全局变量声明
                    std::cout<< "decl" << std::endl;

                    struct VarDecl* vardecl = child->as<Decl*>()->vardecl->as<VarDecl*>();
                    std::string_view btype = vardecl->btype;
                    struct VarDefList* vardeflist = vardecl->vardeflist->as<VarDefList*>();

                    for (auto child_1 : vardeflist->children) {
                        struct VarDef* vardef = child_1->as<VarDef*>();
                        std::string var_name = vardef->var_name;
                        NodePtr init_value = vardef->init_value;
                        std::vector<int> array_indices = vardef->array_indices;
                        std::cout<< "vardef_varname:"<< var_name << "array_indices:"<< array_indices.size() << std::endl;
                        std::size_t NumElements = 1;
                        for (auto child_2 : array_indices) {
                            NumElements = NumElements * child_2;
                        }

                        if (array_indices.size() != 0) {
                            Type *intType = Type::getIntegerTy();
                            Type *pointerType = PointerType::get(intType);
                            GlobalVariable *global_var = GlobalVariable::Create(pointerType, NumElements, false, var_name , &_module);
                        } else {
                            Type *intType = Type::getIntegerTy();
                            GlobalVariable *global_var = GlobalVariable::Create(intType, NumElements, false, var_name , &_module);
                        }

                    }


                } else if (child->node_type == ND_FuncDef) {  // 函数定义
                    std::cout<< "funcdef" << std::endl;
                    traverse(child);
                }


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
            NodePtr body = temp->body;


            // Function 返回类型
            std::cout << return_type << "  " << func_name << std::endl;
            Type *returnType;
            if (return_type == "int") {
                returnType = Type::getIntegerTy();
            } else if (return_type == "void") {
                returnType = Type::getUnitTy();
            }

            // Function 形参

            std::vector<Type *> paramTypes;
            struct FuncFParams* params;
            if (temp->params == nullptr){
                std::cout << "no params" << std::endl;
            } else {
                std::cout << "have params" << std::endl;
                params = temp->params->as<FuncFParams*>();
                std::cout << params->children.size() << std::endl;

                for (auto child :params->children) {
                    struct FuncFParam* param= child->as<FuncFParam*>();
                    std::string param_type = param->param_type;
                    std::string param_name = param->param_name;
                    std::vector<int> dimensions = param->dimensions; // 存储数组的每个维度的大小

                    std::cout << "dimensions size" << dimensions.size() << std::endl;

                    for (auto child_1 : dimensions){
                        std::cout << child_1 << std::endl;
                    }

                    if (dimensions.size() != 0) {
                        Type *intType = Type::getIntegerTy();
                        Type *pointerType = PointerType::get(intType);
                        paramTypes.push_back(pointerType);
                    } else {
                        Type *intType = Type::getIntegerTy();
                        paramTypes.push_back(intType);
                    }
                }
            }


            // 创建 FunctionType 实例
            FunctionType *funcType = FunctionType::get(returnType, paramTypes);
            Function *func = Function::Create(funcType, false, func_name , &_module);

//            for (Function::arg_iterator it = func->arg_begin(); it != func->arg_end(); ++it) {
//                Argument *arg = it;
//                std::cout << "Argument name: " << arg->getName() << ", type: " << arg->getType()->getTypeID() << std::endl;
//            }

            traverse(body);

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
