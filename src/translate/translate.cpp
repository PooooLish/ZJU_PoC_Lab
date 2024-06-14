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

//        case ND_Block: {
//            break;
//        }
//        case ND_BlockItemList: {
//            break;
//        }
//        case ND_BlockItem: {
//            break;
//        }
//        case ND_Stmt: {
//            break;
//        }
//        case ND_IfStmt: {
//            break;
//        }
//        case ND_ReturnStmt: {
//            break;
//        }
//        case ND_WhileStmt: {
//            break;
//        }
//        case ND_AssignStmt: {
//            break;
//        }
//        case ND_Decl: {
//            break;
//        }
//        case ND_InitVal: {
//            break;
//        }
//        case ND_Exp: {
//            break;
//        }
//        case ND_PrimaryExp: {
//            break;
//        }
//        case ND_VarDecl: {
//            break;
//        }
//        case ND_VarDefList: {
//            break;
//        }
//        case ND_VarDef: {
//            break;
//        }
//        case ND_FuncFParams: {
//            break;
//        }
//        case ND_FuncFParam: {
//            break;
//        }
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

            BasicBlock* entry_bb = BasicBlock::Create(func, nullptr);

            std::unordered_map<std::string_view, Value*> symbol_table;

            BasicBlock* exit_bb = translate_stmt(body, entry_bb, &symbol_table);

            if (exit_bb != nullptr) {
                exit_bb->insertInto(func, nullptr);
            }
            break;
        }

        default:
            break;
    }

    return;
}

BasicBlock *translate::translate_stmt(NodePtr node, BasicBlock *current_bb, std::unordered_map<std::string_view, Value*> symbol_table) {
    // Translate a statement node into an IR basic block
    // The actual implementation depends on your statement node structure
    // and the IR representation you are targeting

    // Example (pseudocode):
    BasicBlock *BB = BasicBlock::Create();
    // Translate the statement into IR instructions and add them to the block
    // ...

    switch (node->node_type){
        case ND_Block: {
            struct BlockItemList* item_list = node->as<Block*>()->itemList->as<BlockItemList*>();
            BasicBlock *exit_bb = current_bb;
            for (auto child : item_list->children) { // child为 block item ，Decl | Stmt
                exit_bb = translate_stmt(child, exit_bb, symbol_table); // 循环child ，每个child在上一个child的返回基本块处继续
            }
            return exit_bb;
            break;
        }

        case ND_BlockItem: {
//            NodePtr
            return translate_stmt(child, exit_bb, symbol_table);
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
//            VarDecl ID ：

//            entry_bb = get_function_entry_bb();
//            var_type = lookup_var_type(sym_table, ID);
//            alloca_instr = create_alloca(var_type, 1, entry_bb);
//            update(sym_table, ID, alloca_instr);
//            return current_bb;

//            VarDecl ID[size]
//            entry_bb = get_function_entry_bb();
//            var_type = lookup_var_type(sym_table, ID);
//            alloca_instr = create_alloca(var_type, size, entry_bb);
//            update(sym_table, ID, alloca_instr);
//            return current_bb;

            break;
        }

        default:
            break;
    }

    return BB;
}

Value *translate::translate_expr(NodePtr node, BasicBlock *current_bb, std::unordered_map<std::string_view, Value*> symbol_table) {
    // Translate an expression node into an IR value
    // The actual implementation depends on your expression node structure
    // and the IR representation you are targeting

    // Example (pseudocode):
    Value *value = nullptr;
    // Translate the expression into an IR value
    // ...

    switch (node->node_type) {
        case ND_BinaryExp: {
            auto binaryExp = node->as<BinaryExp*>();
            Value *lhs = translate_expr(binaryExp->lhs, current_bb, symbol_table);
            Value *rhs = translate_expr(binaryExp->rhs, current_bb, symbol_table);
            // Mapping from binary operator type to the corresponding creation function
            switch (binaryExp->op) {
                case Add:
                    value = BinaryInst::CreateAdd(lhs, rhs, lhs->getType(), current_bb);
                    break;
                case Sub:
                    value = BinaryInst::CreateSub(lhs, rhs, lhs->getType(), current_bb);
                    break;
                case Mul:
                    value = BinaryInst::CreateMul(lhs, rhs, lhs->getType(), current_bb);
                    break;
                case Div:
                    value = BinaryInst::CreateDiv(lhs, rhs, lhs->getType(), current_bb);
                    break;
                case Mod:
                    value = BinaryInst::CreateMod(lhs, rhs, lhs->getType(), current_bb);
                    break;
                case Eq:
                    value = BinaryInst::CreateEq(lhs, rhs, lhs->getType(), current_bb);
                    break;
                case Ne:
                    value = BinaryInst::CreateNe(lhs, rhs, lhs->getType(), current_bb);
                    break;
                case Lt:
                    value = BinaryInst::CreateLt(lhs, rhs, lhs->getType(), current_bb);
                    break;
                case Gt:
                    value = BinaryInst::CreateGt(lhs, rhs, lhs->getType(), current_bb);
                    break;
                case Le:
                    value = BinaryInst::CreateLe(lhs, rhs, lhs->getType(), current_bb);
                    break;
                case Ge:
                    value = BinaryInst::CreateGe(lhs, rhs, lhs->getType(), current_bb);
                    break;
                case And:
                    value = BinaryInst::CreateAnd(lhs, rhs, lhs->getType(), current_bb);
                    break;
                case Or:
                    value = BinaryInst::CreateOr(lhs, rhs, lhs->getType(), current_bb);
                    break;
                case Xor:
                    value = BinaryInst::CreateXor(lhs, rhs, lhs->getType(), current_bb);
                    break;
                // Add other binary operators as needed
                default:
                    assert(false && "Unknown binary operator");
            }
            break;
        }
        case ND_LVal: {
            auto lval = node->as<Lval*>();
            std::string_view name = lval->ident_name;
            
            if (lval->lvalexplist == nullptr) {
                // 如果没有下标访问，直接从符号表中获取值
                value = symbol_table[name];
            } else {
                // 有下标访问，需要计算数组元素地址
                std::vector<Value*> indices;
                
                // 获取数组的基地址
                Value *array = symbol_table[name];
                
                // 第一个索引是0，用于获取基地址
                indices.push_back(ConstantInt::get(Type::getInt32Ty(context), 0));
                
                for (auto child : lval->lvalexplist->as<LValExpList*>()->children) {
                    // 获取每个子节点的整数值
                    Value *index = translate_expr(child, current_bb, symbol_table);
                    indices.push_back(index);
                }
                
                // 生成GEP指令
                value = GetElementPtrInst::Create(array, indices, "", current_bb);
            }
            break;
        }

    }

    return value;
}
