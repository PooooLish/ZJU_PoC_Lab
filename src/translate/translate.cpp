#include "translate/translate.h"
#include "ast/ast.h"
#include <fstream>
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <string>

using SymbolTable = std::unordered_map<std::string, Value*>;

void translate::addAddr(std::unordered_map<std::string, Value*>* symbol_table, std::string name, Value* value) {
    (*symbol_table)[name] = value;
    std::cout << "Add value: " << name << std::endl;
    return;
}

Value* translate::getAddr(const std::unordered_map<std::string, Value*>* symbol_table, std::string name) {
    auto it = symbol_table->find(name);
    if (it != symbol_table->end()) {
        return it->second;
    }
    std::cout << "Not find value: " << name << std::endl;
    return nullptr;
}

translate::translate(NodePtr root, const std::string& output_file) {

    Type *intType = Type::getIntegerTy();
    Type *pointerType = PointerType::get(intType);

    // putint(int x)
    std::vector<Type *> putintParamTypes = { intType };
    FunctionType *putintType = FunctionType::get(Type::getUnitTy(), putintParamTypes);
    Function *putintFunc = Function::Create(putintType, true, "putint", &_module);
    putintFunc->getArg(0)->setName("x");

    // putch(int x)
    std::vector<Type *> putchParamTypes = { intType };
    FunctionType *putchType = FunctionType::get(Type::getUnitTy(), putchParamTypes);
    Function *putchFunc = Function::Create(putchType, true, "putch", &_module);
    putchFunc->getArg(0)->setName("x");

    // putarray(int n, pointer arr)
    std::vector<Type *> putarrayParamTypes = { intType , pointerType};
    FunctionType *putarrayType = FunctionType::get(Type::getUnitTy(), putarrayParamTypes);
    Function *putarrayFunc = Function::Create(putarrayType, true, "putarray", &_module);
    putarrayFunc->getArg(0)->setName("n");
    putarrayFunc->getArg(1)->setName("arr");

    // getint()
    std::vector<Type *> getintParamTypes;
    FunctionType *getintType = FunctionType::get(intType, getintParamTypes);
    Function *getintFunc = Function::Create(getintType, true, "getint", &_module);

    // getch()
    std::vector<Type *> getchParamTypes;
    FunctionType *getchType = FunctionType::get(intType, getchParamTypes);
    Function *getchFunc = Function::Create(getchType, true, "getch", &_module);

    // getarray(int n, pointer arr)
    std::vector<Type *> getarrayParamTypes = { intType , pointerType};
    FunctionType *getarrayType = FunctionType::get(Type::getUnitTy(), getarrayParamTypes);
    Function *getarrayFunc = Function::Create(getarrayType, true, "getarray", &_module);
    getarrayFunc->getArg(0)->setName("n");
    getarrayFunc->getArg(1)->setName("arr");

    traverse(root);

    std::ofstream outFile(output_file);
    if (!outFile.is_open()) {
        std::cerr << "Error opening file for writing: " << output_file << std::endl;
        return;
    }
    _module.print(outFile, false);
    outFile.close();
    std::cout << "Module printed to " << output_file << std::endl;
}

void translate::traverse(NodePtr node) {
    switch (node->node_type) {
        case ND_CompUnit: {
            auto compUnit = node->as<CompUnit*>();

            for (auto child : compUnit->children) {
                if (child->node_type == ND_Decl) { // 全局变量声明
                    std::cout<< "decl" << std::endl;
                    traverse(child);
                }
            }
            for (auto child : compUnit->children){
                if (child->node_type == ND_FuncDef) {  // 函数定义
                    std::cout<< "funcdef" << std::endl;
                    traverse(child);
                }
            }
            std::cout<< "init finish" << std::endl;
            break;
        }
        case ND_Decl: {
            global_decl_list.push_back(node);
            processGlobalDecl(node);
            for (const auto& pair : _module.getGlobalVariableMap()) {
                std::cout << pair.first << " : " << pair.second << std::endl;
            }
            break;
        }
        case ND_FuncDef: {
            processFuncDef(node);
            break;
        }

        default:
            break;
    }
    return;
}

void translate::processGlobalDecl(NodePtr node){
    struct VarDecl* vardecl = node->as<Decl*>()->vardecl->as<VarDecl*>();
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
            GlobalVariable *global_var = GlobalVariable::Create(pointerType, NumElements, true, var_name , &_module);
        } else {
            Type *intType = Type::getIntegerTy();
            GlobalVariable *global_var = GlobalVariable::Create(intType, NumElements, true, var_name , &_module);
        }
    }
    for (const auto& pair : _module.getGlobalVariableMap()) {
        std::cout << pair.first << " : " << pair.second << std::endl;
    }
}

void translate::processFuncDef(NodePtr node){
    struct FuncDef* temp = node->as<FuncDef*>();

    std::string return_type = temp->return_type;
    std::string func_name = temp->func_name;
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

    std::cout << "params: " << func->getNumParams() <<std::endl;

    func_symbol_table.clear();
    SymbolTable* symbol_table = &func_symbol_table;

    entry_bb = BasicBlock::Create(func, nullptr);
    entry_bb->setName("Entry");

    AllocaInst* return_alloc_inst;

    
    if(returnType == Type::getIntegerTy())
        std::cout << "int" <<std::endl;
    else if(returnType == Type::getUnitTy())
        std::cout << "void" <<std::endl;
    else
        std::cout << "other" <<std::endl;

    if(returnType == Type::getIntegerTy()){
        return_alloc_inst = AllocaInst::Create(returnType, 1, entry_bb);
        return_alloc_inst->setName("ret.addr");
        addAddr(symbol_table ,"ret.addr" , return_alloc_inst);




    }
   
    
    

    if (func_name == "main"){
        std::cout << "in main func" <<std::endl;
        for (auto global_decl: global_decl_list) {
            translate_stmt(global_decl, entry_bb, symbol_table);
        }
        std::cout << "decl global var finish" <<std::endl;
    }
    

    return_bb = BasicBlock::Create(func, nullptr);
    return_bb->setName("Ret");
    LoadInst* return_value_inst;
    if(returnType == Type::getIntegerTy()){
        return_value_inst = LoadInst::Create(return_alloc_inst, return_bb);
        return_value_inst->setName("ret.val");
        RetInst* ret_inst;
        ret_inst = RetInst::Create(return_value_inst, return_bb);
    }
    if (temp->params != nullptr){
        
        params = temp->params->as<FuncFParams*>();
        AllocaInst *alloc_inst;
        int param_num = 0;

        for (auto child :params->children) {
            struct FuncFParam* param= child->as<FuncFParam*>();
            std::string param_type = param->param_type;
            std::string param_name = param->param_name;
            std::vector<int> dimensions = param->dimensions; // 存储数组的每个维度的大小
            std::vector<std::optional<std::size_t>> arr_bounds;
            Type *intType = Type::getIntegerTy();

            for(auto child_1: dimensions){
                if( child_1 != -1){
                    arr_bounds.push_back(child_1);
                    std::cout << "arr_bounds: " << child_1 << std::endl;
                }
                else
                    arr_bounds.push_back(std::nullopt);
            }
            arr_bounds_table[param_name] = arr_bounds;

            std::size_t NumElements = 1; // 形参元素个数

            for (auto child_1: dimensions) {
                NumElements = NumElements * child_1;
            }

            auto* type = dimensions.size() == 0 ? intType : PointerType::get(intType);
            type = intType;
            std::cout << "param_name1111111111111111111111111:" << type << std::endl;
            alloc_inst = AllocaInst::Create(type, NumElements, entry_bb);
            alloc_inst->setName(param_name);
            addAddr(symbol_table , param_name , alloc_inst);
            StoreInst::Create(func->getArg(param_num), alloc_inst, entry_bb);

            param_num++;
        }
    }

    translate_stmt(body, entry_bb, symbol_table);

    std::cout << "printSymbolTable: "<<std::endl;
    printSymbolTable(*symbol_table);

}

BasicBlock *translate::translate_stmt(NodePtr node, BasicBlock *current_bb, std::unordered_map<std::string, Value*>* symbol_table) {

    BasicBlock *BB = BasicBlock::Create();

    if (current_bb == nullptr) return nullptr;

    switch (node->node_type){

        case ND_Block: {
            struct BlockItemList* item_list = node->as<Block*>()->itemList->as<BlockItemList*>();
            BasicBlock *exit_bb = current_bb;
            for (auto child : item_list->children) { // child为 block item
                BasicBlock* temp = exit_bb;
                NodePtr item = child->as<BlockItem*>()->item; // item 为 decl | stmt
                exit_bb = translate_stmt(item, exit_bb, symbol_table); // 循环child ，每个child在上一个child的返回基本块处继续
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

            std::cout << "ND_AssignStmt" << std::endl;

            NodePtr lhs = node->as<AssignStmt*>()->lhs;
            NodePtr rhs = node->as<AssignStmt*>()->rhs;
            std::string ident_name = lhs->as<Lval*>()->ident_name;
            std::cout << "ident_name: "<< ident_name;

            Value *addr_value = getAddr(symbol_table, ident_name);
            if (lhs->as<Lval*>()->lvalexplist != nullptr) {
                std::vector<Value*> indices;
                for (auto child : lhs->as<Lval*>()->lvalexplist->as<LValExpList*>()->children) {
                    Value *index = translate_expr(child, current_bb, symbol_table);
                    indices.push_back(index);
                }
                addr_value = OffsetInst::Create(Type::getIntegerTy(),addr_value, indices, arr_bounds_table[ident_name], current_bb);
            }
            Value *result_value = translate_expr(rhs ,current_bb , symbol_table);

            StoreInst::Create(result_value, addr_value, current_bb);

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

            BasicBlock *exit_bb = BasicBlock::Create();;
            BasicBlock *true_bb;
            BasicBlock *false_bb;

            std::cout << "111" << std::endl;

            if (else_stmt == nullptr) {
                // If (Expr) Stmt

                true_bb = BasicBlock::Create(parent_func, return_bb);
                Value *cond_value = translate_expr(condition , current_bb, symbol_table);
                BranchInst::Create(true_bb, exit_bb, cond_value, current_bb);

                BasicBlock* true_exit_bb = translate_stmt(then_stmt, true_bb, symbol_table);
                if (true_exit_bb == nullptr){
                    std::cout << "If (Expr) Stmt include 'return'" << std::endl;
                } else {
                    JumpInst::Create(exit_bb,true_exit_bb);
                }

            } else {
                // If (Expr) Stmt1 Else Stmt2
                std::cout << "222" << std::endl;

                true_bb = BasicBlock::Create(parent_func, return_bb);
                Value *cond_value = translate_expr(condition , current_bb, symbol_table);
                false_bb = BasicBlock::Create(parent_func, return_bb);
                BranchInst::Create(true_bb, false_bb, cond_value, current_bb);

                std::cout << "333" << std::endl;

                BasicBlock* true_exit_bb = translate_stmt(then_stmt, true_bb, symbol_table);
                if (true_exit_bb == nullptr){
                    std::cout << "If (Expr) Stmt1 Else Stmt2 include 'return'" << std::endl;
                } else {
                    JumpInst::Create(exit_bb,true_exit_bb);
                }

                BasicBlock* false_exit_bb = translate_stmt(else_stmt, false_bb, symbol_table);
                if (false_exit_bb == nullptr){
                    std::cout << "If (Expr) Stmt1 Else Stmt2 include 'return'" << std::endl;
                } else {
                    JumpInst::Create(exit_bb,false_exit_bb);
                }

            }
            exit_bb->insertInto(parent_func, return_bb);


            std::cout << "ND_IfStmt finish" << std::endl;
            return exit_bb;

            break;
        }

        case ND_WhileStmt: {
            std::cout << "ND_WhileStmt" << std::endl;

            Function *parent_func = current_bb->getParent();

            NodePtr condition = node->as<WhileStmt*>()->condition;
            NodePtr body = node->as<WhileStmt*>()->body;

            BasicBlock *while_exit_bb = BasicBlock::Create(parent_func, return_bb);
            BasicBlock *while_entry_bb = BasicBlock::Create(parent_func, while_exit_bb);
            BasicBlock *while_body_bb = BasicBlock::Create(parent_func, while_exit_bb);

            JumpInst::Create(while_entry_bb,current_bb);
            Value *cond_value = translate_expr(condition , while_entry_bb, symbol_table);
            BranchInst::Create(while_body_bb, while_exit_bb, cond_value, while_entry_bb);
            BasicBlock* body_exit_bb = translate_stmt(body, while_body_bb, symbol_table);
            if (body_exit_bb == nullptr){
                std::cout << "While (Expr) Stmt include 'return'" << std::endl;
            } else {
                JumpInst::Create(while_entry_bb,body_exit_bb);
            }

            return while_exit_bb;
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

            NodePtr exp = node->as<ReturnStmt*>()->exp;
            Function* parent_func = current_bb->getParent();
            Value* return_addr = getAddr(symbol_table, "ret.addr");
            Value* return_value;
            if (exp != nullptr){
                return_value = translate_expr(exp , current_bb, symbol_table);
                StoreInst::Create(return_value, return_addr, current_bb);
            }

            JumpInst::Create(return_bb,current_bb);

            std::cout << "ND_ReturnStmt finish" << std::endl;

            return nullptr;
            break;
        }

        case ND_Decl: {

            std::cout << "ND_Decl" << std::endl;

            struct VarDecl* vardecl = node->as<Decl*>()->vardecl->as<VarDecl*>();
            std::string btype = vardecl->btype;
            struct VarDefList* vardeflist = vardecl->vardeflist->as<VarDefList*>();
            Type *intType = Type::getIntegerTy(); // change by btype

            for (auto child : vardeflist->children) {
                struct VarDef *vardef = child->as<VarDef *>();
                std::string var_name = vardef->var_name;
                NodePtr init_value = vardef->init_value;
                std::vector<int> array_indices = vardef->array_indices; // 可能存在的多维度
                std::size_t NumElements = 1; // 元素个数

                std::vector<std::optional<std::size_t>> arr_bounds;
                Type *intType = Type::getIntegerTy();

                for(auto child_1: array_indices){
                    if( child_1 != -1){
                        arr_bounds.push_back(child_1);
                        std::cout << "arr_bounds: " << child_1 << std::endl;
                    }
                    else
                        arr_bounds.push_back(std::nullopt);
                }
                arr_bounds_table[var_name] = arr_bounds;

                for (auto child_1: array_indices) {
                    NumElements = NumElements * child_1;
                }
                std::cout << "varname:" << var_name << " num_element:" << NumElements << std::endl;

                Function* parent_func = current_bb->getParent();
                BasicBlock* entry_bb = &parent_func->getEntryBlock();
                AllocaInst *alloc_inst;

                auto* type = array_indices.size() == 0 ? intType : PointerType::get(intType);
                type = intType;
                if (current_bb == entry_bb){
                    alloc_inst = AllocaInst::Create(type, NumElements, entry_bb);
                }
                else {
                    alloc_inst = AllocaInst::Create(type, NumElements ,entry_bb->getTerminator());
                }
                alloc_inst->setName(var_name);

                addAddr(symbol_table, var_name, alloc_inst);

                if (init_value != nullptr) {
                    std::cout << "have init_value" << std::endl;
                    NodePtr init_value_exp = init_value->as<InitVal*>()->exp;
                    Value *result_value = translate_expr(init_value_exp ,current_bb , symbol_table);
                    StoreInst::Create(result_value, alloc_inst, current_bb);
                }
            }
            std::cout << "ND_Decl finish" << std::endl;
            return current_bb;
            break;
        }

        default:
            break;
    }

    return nullptr;
}

Value *translate::translate_expr(NodePtr node, BasicBlock *current_bb, std::unordered_map<std::string, Value*>* symbol_table) {
    Value *value = nullptr;
    
    switch (node->node_type) {
        case ND_BinaryExp: {
            
            auto binaryExp = node->as<BinaryExp*>();
            Value *lhs;
            Value *rhs;
            Value *lhs_addr = translate_expr(binaryExp->lhs, current_bb, symbol_table);
            if(lhs_addr->getType() != Type::getIntegerTy()) {
                lhs = LoadInst::Create(lhs_addr, current_bb);
            } else {
                lhs = lhs_addr;
            }
            Value *rhs_addr = translate_expr(binaryExp->rhs, current_bb, symbol_table);
            if(rhs_addr->getType() != Type::getIntegerTy()) {
                rhs = LoadInst::Create(rhs_addr, current_bb);
            } else {
                rhs = rhs_addr;
            }

            // Mapping from binary operator type to the corresponding creation function
            switch (binaryExp->op) {
                case Add:
                    std::cout << "Add" << std::endl;
                    value = BinaryInst::CreateAdd(lhs, rhs, lhs->getType(), current_bb);
                    break;
                case Sub:
                    std::cout << "Sub" << std::endl;
                    value = BinaryInst::CreateSub(lhs, rhs, lhs->getType(), current_bb);
                    break;
                case Mul:
                    std::cout << "Mul" << std::endl;
                    value = BinaryInst::CreateMul(lhs, rhs, lhs->getType(), current_bb);
                    break;
                case Div:
                    std::cout << "Div" << std::endl;
                    value = BinaryInst::CreateDiv(lhs, rhs, lhs->getType(), current_bb);
                    break;
                case Mod:
                    std::cout << "Mod" << std::endl;
                    value = BinaryInst::CreateMod(lhs, rhs, lhs->getType(), current_bb);
                    break;
                case Eq:
                    std::cout << "Eq" << std::endl;
                    value = BinaryInst::CreateEq(lhs, rhs, lhs->getType(), current_bb);
                    break;
                case Ne:
                    std::cout << "Ne" << std::endl;
                    value = BinaryInst::CreateNe(lhs, rhs, lhs->getType(), current_bb);
                    break;
                case Lt:
                    std::cout << "Lt" << std::endl;
                    value = BinaryInst::CreateLt(lhs, rhs, lhs->getType(), current_bb);
                    break;
                case Gt:
                    std::cout << "Gt" << std::endl;
                    value = BinaryInst::CreateGt(lhs, rhs, lhs->getType(), current_bb);
                    break;
                case Le:
                    std::cout << "Le" << std::endl;
                    value = BinaryInst::CreateLe(lhs, rhs, lhs->getType(), current_bb);
                    break;
                case Ge:
                    std::cout << "Ge" << std::endl;
                    value = BinaryInst::CreateGe(lhs, rhs, lhs->getType(), current_bb);
                    break;
                case And:
                    std::cout << "And" << std::endl;
                    value = BinaryInst::CreateAnd(lhs, rhs, lhs->getType(), current_bb);
                    break;
                case Or:
                    std::cout << "Or" << std::endl;
                    value = BinaryInst::CreateOr(lhs, rhs, lhs->getType(), current_bb);
                    break;
                case Xor:
                    std::cout << "Xor" << std::endl;
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
                std::string name = unaryExp->ident_name;
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
                if(operand->getType() != Type::getIntegerTy()) {
                    operand = LoadInst::Create(operand, current_bb);
                }
                std::cout<<"11111111"<<std::endl;
                if (unaryExp->op==Neg) {
                    Value* zero_value = ConstantInt::Create(0);
                    value = BinaryInst::CreateSub(zero_value, operand, operand->getType(), current_bb);
                }else if(unaryExp->op==Lnot) {
                    Value* zero_value = ConstantInt::Create(0);
                    value = BinaryInst::CreateEq(operand, zero_value, operand->getType(), current_bb);
                }else {
                    value = operand;
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
            ConstantInt *value1 = ConstantInt::Create(intLiteral->value);
            std::cout <<"       " <<value1->getType() << std::endl;
            value = value1;
            break;
        }
        case ND_LVal: {
            auto lval = node->as<Lval*>();
            std::string name = lval->ident_name;
            if (lval->lvalexplist == nullptr) {
                // 如果没有下标访问，直接从符号表中获取值
                auto it = symbol_table->find(name);
                if (it != symbol_table->end()) {
                    value = LoadInst::Create(it->second, current_bb);
                    // 使用 var_value 执行后续操作
                } 
            }
            else {
                std::vector<Value*> indices;
                // 获取数组的基地址
                Value *addr_value = (*symbol_table)[name];
                for (auto child : lval->lvalexplist->as<LValExpList*>()->children) {
                    // 获取每个子节点的整数值
                    Value *index = translate_expr(child, current_bb, symbol_table);
                    indices.push_back(index);
                }
                std::cout << Type::getIntegerTy()<< std::endl;
                std::cout << Type::getUnitTy()<< std::endl;
                std::cout << "addr_value->getType() " << addr_value->getType()<< std::endl;
                if(addr_value->getType() != Type::getIntegerTy()) {
                    std::cout << "addr_value->"<< std::endl;
                    addr_value = OffsetInst::Create(Type::getIntegerTy(),addr_value, indices, arr_bounds_table[name], current_bb);  
                    std::cout << "addr_value->"<< std::endl;
                    value = LoadInst::Create(addr_value, current_bb);
                    // 使用 var_value 执行后续操作
                } else{
                    Value *offset = OffsetInst::Create(Type::getIntegerTy(),addr_value, indices, arr_bounds_table[name], current_bb);  
                    value = LoadInst::Create(offset, current_bb);
                }
                
            }
            break;
        }

        case ND_Exp: {
            auto exp = node->as<Exp*>();
            value = translate_expr(exp->exp, current_bb, symbol_table);
            break;
        }

    }
    return value;
}

void translate::printSymbolTable(const std::unordered_map<std::string, Value*>& symbol_table) {
    for (const auto& entry : symbol_table) {
        std::cout << "Name: " << entry.first << " | Address: " << entry.second << std::endl;
    }
}