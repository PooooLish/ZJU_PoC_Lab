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
    _module.print(outFile, false);  // Assuming `print` method accepts an output stream and a boolean flag

    // Close the file
    outFile.close();

    std::cout << "Module printed to module_output.txt" << std::endl;
}

void translate::traverse(NodePtr node) {

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
            std::cout<< "init finish" << std::endl;
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
            BasicBlock* return_bb = BasicBlock::Create(func, nullptr);

            std::unordered_map<std::string_view, Value*> symbol_table;

            BasicBlock* exit_bb = translate_stmt(body, entry_bb, &symbol_table);

//            for (const auto& entry : symbol_table) {
//                std::cout << "Key: " << entry.first << std::endl;
//            }

//            if (exit_bb != nullptr) {
//                exit_bb->insertInto(func, nullptr);
//            }

            break;
        }

        default:
            break;
    }

    return;
}

BasicBlock *translate::translate_stmt(NodePtr node, BasicBlock *current_bb, std::unordered_map<std::string_view, Value*>* symbol_table) {

    BasicBlock *BB = BasicBlock::Create();

    switch (node->node_type){

        case ND_Block: {
            struct BlockItemList* item_list = node->as<Block*>()->itemList->as<BlockItemList*>();
            BasicBlock *exit_bb = current_bb;
            for (auto child : item_list->children) { // child为 block item
                NodePtr item = child->as<BlockItem*>()->item; // item 为 decl | stmt
                exit_bb = translate_stmt(item, current_bb, symbol_table); // 循环child ，每个child在上一个child的返回基本块处继续
                if (exit_bb == nullptr) break;
            }
            return exit_bb;
            break;
        }

        case ND_Stmt: {
            std::cout << "ND_Stmt" << std::endl;
            NodePtr stmtPtr = node->as<Stmt*>()->stmtPtr;
            BasicBlock *exit_bb = translate_stmt(stmtPtr, current_bb, symbol_table);
            std::cout << "ND_Stmt finish" << std::endl;
            return exit_bb;
            break;
        }

        case ND_AssignStmt: {
            // addr_value = lookup(sym_table, ID);
            // result_value = translate_expr(Expr, sym_table, current_bb);
            // create_store(result_value, addr_value, current_bb);

            std::cout << "ND_AssignStmt" << std::endl;

            NodePtr lhs = node->as<AssignStmt*>()->lhs;
            NodePtr rhs = node->as<AssignStmt*>()->rhs;
            Value *addr_value;
            if(symbol_table->find(lhs->as<Lval*>()->ident_name)!=symbol_table->end()){
                std::cout << "find" << std::endl;
                addr_value = symbol_table->find(lhs->as<Lval*>()->ident_name)->second;
            } else {
                std::cout << "not find" << std::endl;
                //symbol_table->insert({lhs->as<Lval*>()->ident_name, new AllocaInst(Type::getIntegerTy(), 1, current_bb)});
            }
            Value *result_value = translate_expr(rhs ,current_bb , symbol_table );
            std::cout << "ND_AssignStmt finish" << std::endl;
            StoreInst *store_inst= StoreInst::Create(result_value, addr_value, current_bb);
            std::cout << "ND_AssignStmt finish3" << std::endl;
            std::string var_name = lhs->as<Lval*>()->ident_name;
            (*symbol_table)[var_name] = store_inst;

            std::cout << "ND_AssignStmt finish" << std::endl;
            return current_bb;
            break;
        }
        case ND_Exp: {
            std::cout << "ND_Exp" << std::endl;
            NodePtr exp = node->as<Exp*>()->exp;
            translate_expr(exp, current_bb, symbol_table);
            std::cout << "ND_Exp finish" << std::endl;
            return current_bb;
            break;
        }
        case ND_IfStmt: { // two conditions
            std::cout << "ND_IfStmt" << std::endl;
            NodePtr condition = node->as<IfStmt*>()->condition;
            NodePtr then_stmt = node->as<IfStmt*>()->then_stmt;
            NodePtr else_stmt = node->as<IfStmt*>()->else_stmt;

            Function *parent_func = current_bb->getParent();
            BasicBlock *exit_bb = BasicBlock::Create(parent_func, &(*parent_func->end()));
            BasicBlock *true_bb = BasicBlock::Create(parent_func, &(*parent_func->end()));
            BasicBlock *false_bb = BasicBlock::Create(parent_func, &(*parent_func->end()));
            Value *cond_value = translate_expr(condition , current_bb, symbol_table);

            if (else_stmt == nullptr) { // If (Expr) Stmt

                BranchInst::Create(true_bb, exit_bb, cond_value, current_bb);
                BasicBlock* true_exit_bb = translate_stmt(then_stmt, true_bb, symbol_table);
                JumpInst::Create(exit_bb,true_exit_bb);

            } else { // If (Expr) Stmt1 Else Stmt2

                BranchInst::Create(true_bb, false_bb, cond_value, current_bb);
                BasicBlock* true_exit_bb = translate_stmt(then_stmt, true_bb, symbol_table);
                JumpInst::Create(exit_bb,true_exit_bb);
                BasicBlock* false_exit_bb = translate_stmt(else_stmt, false_bb, symbol_table);
                JumpInst::Create(exit_bb,false_exit_bb);

            }
            std::cout << "ND_ReturnStmt finish" << std::endl;
            return exit_bb;
            break;
        }

        case ND_WhileStmt: {
            std::cout << "ND_WhileStmt" << std::endl;

            Function *parent_func = current_bb->getParent();

            NodePtr condition = node->as<WhileStmt*>()->condition;
            NodePtr body = node->as<WhileStmt*>()->body;

            BasicBlock *entry_bb = BasicBlock::Create(parent_func, &(*parent_func->end()));
            BasicBlock *body_bb = BasicBlock::Create(parent_func, &(*parent_func->end()));
            BasicBlock *exit_bb = BasicBlock::Create(parent_func, &(*parent_func->end()));

            JumpInst::Create(entry_bb,current_bb);
            Value *cond_value = translate_expr(condition , entry_bb, symbol_table);
            BranchInst::Create(body_bb, exit_bb, cond_value, entry_bb);
            BasicBlock* body_exit_bb = translate_stmt(body, body_bb, symbol_table);
            JumpInst::Create(entry_bb,body_exit_bb);

            return exit_bb;
            break;
        }

        case ND_BreakStmt: {
            std::cout << "ND_BreakStmt" << std::endl;
            std::cout << "ND_BreakStmt finish" << std::endl;
            return BB;
            break;
        }

        case ND_ContinueStmt: {
            std::cout << "ND_ContinueStmt" << std::endl;
            std::cout << "ND_ContinueStmt finish" << std::endl;
            return BB;
            break;
        }

        case ND_ReturnStmt: {
            std::cout << "ND_ReturnStmt" << std::endl;

//            NodePtr exp = node->as<ReturnStmt*>()->exp;
//
//            Function* parent_func = current_bb->getParent();
//            BasicBlock* return_bb = &(*parent_func->end());
//
//            if (exp != nullptr){
//                Value* return_value = translate_expr(exp , current_bb, symbol_table);
//            }
//
//            JumpInst::Create(return_bb,current_bb);

            // return_addr = get_function_ret_value_addr();
            // return_value = translate_expr(Expr, sym_table, current_bb);
            // create_store(return_value, return_addr, current_bb);
            // create_jump(return_bb, current_bb);

            std::cout << "ND_ReturnStmt finish" << std::endl;

            return nullptr;
            break;
        }

        case ND_Decl: {

            std::cout << "ND_Decl" << std::endl;

            struct VarDecl* vardecl = node->as<Decl*>()->vardecl->as<VarDecl*>();
            std::string_view btype = vardecl->btype;
            struct VarDefList* vardeflist = vardecl->vardeflist->as<VarDefList*>();
            Type *intType = Type::getIntegerTy(); // change by btype

            for (auto child : vardeflist->children) {
                struct VarDef *vardef = child->as<VarDef *>();
                std::string var_name = vardef->var_name;
                NodePtr init_value = vardef->init_value;
                std::vector<int> array_indices = vardef->array_indices;
                std::size_t NumElements = 1;

                for (auto child_1: array_indices) {
                    NumElements = NumElements * child_1;
                }
                std::cout << "vardef_varname:" << var_name << " num_element:" << NumElements << std::endl;

                Function* parent_func = current_bb->getParent();
                BasicBlock* entry_bb = &parent_func->getEntryBlock();
                AllocaInst *alloc_inst;

                if (array_indices.size() != 0) {
                    auto* type = PointerType::get(intType);
                    alloc_inst = AllocaInst::Create(type, NumElements, entry_bb);
                } else {
                    alloc_inst = AllocaInst::Create(intType, NumElements , entry_bb);
                }

                (*symbol_table)[var_name] = alloc_inst;
                std::cout<< (*symbol_table)[var_name] << std::endl;
                if (init_value != nullptr) {
                    Value *value = translate_expr(init_value, current_bb, symbol_table);
//                    StoreInst *store_inst= StoreInst::Create(value, alloc_inst, current_bb);
//                    symbol_table[var_name] = store_inst;
                }
            }
            std::cout << "ND_Decl finish" << std::endl;
            return current_bb;
            break;
        }

        default:
            break;
    }

    return BB;
}

Value *translate::translate_expr(NodePtr node, BasicBlock *current_bb, std::unordered_map<std::string_view, Value*>*symbol_table) {
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
        case ND_UnaryExp: {
            auto unaryExp = node->as<UnaryExp*>();
            if(unaryExp->ident_name != "") {
                // 函数调用
                std::string_view name = unaryExp->ident_name;
                std::vector<Value*> args;
                if (unaryExp->funcrparams != nullptr) {
                    auto funcrparams = unaryExp->funcrparams->as<FuncRParams*>();
                    for (auto child : funcrparams->children) {
                        Value *arg = translate_expr(child, current_bb, symbol_table);
                        args.push_back(arg);
                    }
                }
                Function *callee = _module.getFunction(name);
                if (callee == nullptr) {
                    std::cerr << "Unknown function referenced" << std::endl;
                    return nullptr;
                }
                value = CallInst::Create(callee, args, current_bb);
            } else if (unaryExp->operand != nullptr) {
                // 一元操作符（-、!等）(unaryExp->op, unaryExp->operand
                Value *operand = translate_expr(unaryExp->operand, current_bb, symbol_table);
                if (unaryExp->op==Neg) {
                    Value* zero_value = ConstantInt::Create(0);
                    value = BinaryInst::CreateSub(zero_value, operand, operand->getType(), current_bb);
                }
            } else if (unaryExp->primaryexp != nullptr) {
                // primaryexp
                value = translate_expr(unaryExp->primaryexp, current_bb, symbol_table);
            }
            break;
        }
        case ND_PrimaryExp: {
            auto primaryExp = node->as<PrimaryExp*>();
            value = translate_expr(primaryExp->pri_exp, current_bb, symbol_table);
            break;
        }
        case ND_IntegerLiteral: {
            auto intLiteral = node->as<IntegerLiteral*>();
            value = ConstantInt::Create(intLiteral->value);
            break;
        }
        case ND_LVal: {
            auto lval = node->as<Lval*>();
            std::string_view name = lval->ident_name;
            if (lval->lvalexplist == nullptr) {
                // 如果没有下标访问，直接从符号表中获取值
                auto it = symbol_table->find(name);
                if (it != symbol_table->end()) {
                    value = it->second;
                    // 使用 var_value 执行后续操作
                }
                // } else {
                //     // 有下标访问，需要计算数组元素地址
                //     // element type
                //     array_type = lookup_var_type(sym_table, ID);
                //     elem_type = get_elem_type(array_type);
                //     // address of the first element in the array,
                //     // which is actually the stack address represented
                //     // by a 'alloca' instruction or a global variable.
                //     addr_value = lookup(sym_table, ID);
                //     // indices
                //     indices = [];
                //     for idx in Idx1..IdxN:
                //     indices += translate_expr(idx, sym_table, current_bb);
                //     // bounds
                //     bounds = get_bounds(array_type);
                //     return create_load(create_offset(
                //     elem_type,
                //     addr_value,
                //     indices,
                //     bounds
                //     ));
                //     std::vector<Value*> indices;
                //     // 获取数组的基地址
                //     Value *array = symbol_table[name];
                //     for (auto child : lval->lvalexplist->as<LValExpList*>()->children) {
                //         // 获取每个子节点的整数值
                //         Value *index = translate_expr(child, current_bb, symbol_table);
                //         indices.push_back(index);
                //     }
                //     Value *offset = OffsetInst::Create(array->getType(),array, indices, current_bb);
                //     value = LoadInst::Create(array, indices, current_bb);
            }
            break;
        }

        case ND_Exp: {
            auto exp = node->as<Exp*>();
            value = translate_expr(exp->exp, current_bb, symbol_table);
            std::cout << "ND_AssignStmt finish2" << std::endl;
            break;
        }

    }
    return value;
}
