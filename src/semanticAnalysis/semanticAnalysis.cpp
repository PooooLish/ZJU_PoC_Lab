#include "semanticAnalysis/semanticAnalysis.h"

extern void yyerror(const char *s);

void reportError(const std::string& error_msg) {
    std::cerr << "Error: " << error_msg << "\n";
    exit(1); // Or handle errors appropriately
}
// Enter a new scope
void enterScope() {
    symbol_table_var.emplace_back();
}
// Exit a scope
void exitScope() {
    if (!symbol_table_var.empty()) {
        symbol_table_var.pop_back();
    }
}

// Semantic analysis function
int semanticAnalysis(NodePtr node) {
    static std::map<std::string, std::vector<std::vector<int>>> funcparam;
    static std::map<std::string, std::string> funcreturn;
    //static std::vector<std::map<std::string, std::vector<int>>> symbol_table_var;
    //0: int, 1: void;
    static std::vector<int> return_type;
    static int first = 0;
    static int func_block_flag = 0;
    if(first == 0){
        std::map<std::string, std::vector<int>> my_map;
        symbol_table_var.push_back(my_map);
        std::vector<std::vector<int>> params1 = {{}};
        funcparam["putint"] = params1;
        funcreturn["putint"] = "int";
        std::vector<std::vector<int>> params2 = {};
        funcparam["getint"] = params2;
        funcreturn["getint"] = "int";
        std::vector<std::vector<int>> params4 = {};
        funcparam["getarray"] = params4;
        funcreturn["getarray"] = "int";
        std::vector<std::vector<int>> params5 = {{}};
        funcparam["putch"] = params5;
        funcreturn["putch"] = "void";
        std::vector<std::vector<int>> params6 = {};
        funcparam["getch"] = params6;
        funcreturn["getch"] = "int";
        std::vector<std::vector<int>> params7 = {{}};
        funcparam["putarray"] = params7;
        funcreturn["putarray"] = "void";
        first = 1;
    }
    if (node == nullptr)
        return 0;

    switch (node->node_type) {
        case ND_CompUnit: {
            // 对于根节点 CompUnit，递归处理其子节点
            auto compUnit = node->as<CompUnit*>();
            for (auto child : compUnit->children) {
                int error = semanticAnalysis(child);
                if(error != 0){
                    return error;
                }
            }
            break;
        }
        case ND_Block: {
            // 处理代码块
            if(func_block_flag == 0){
                enterScope();
            }
            func_block_flag = 0;
            auto block = node->as<Block*>();
            // 在代码块的作用域中，可以定义新的变量
            int error = semanticAnalysis(block->itemList);
            if(error != 0){
                return error;
            }
            exitScope();
            break;
        }
        case ND_BlockItemList: {
            // 处理代码块中的语句列表
            auto blockItemList = node->as<BlockItemList*>();
            for (auto blockItem : blockItemList->children) {
                int error = semanticAnalysis(blockItem);
                if(error != 0){
                    return error;
                }
            }
            break;
        }
        case ND_BlockItem: {
            // 处理代码块中的语句
            auto blockItem = node->as<BlockItem*>();
            return semanticAnalysis(blockItem->item);
            break;
        }
        case ND_Stmt: {
            // 处理语句
            auto stmt = node->as<Stmt*>();
            return semanticAnalysis(stmt->stmtPtr);
            break;
        }
        case ND_IfStmt: {
            // 处理 if 语句
            auto ifStmt = node->as<IfStmt*>();
            int error = semanticAnalysis(ifStmt->condition);
            if(error != 0){
                return error;
            }
            enterScope();
            error = semanticAnalysis(ifStmt->then_stmt);
            if(error != 0){
                return error;
            }
            exitScope();
            if (ifStmt->else_stmt) {
                enterScope();
                error = semanticAnalysis(ifStmt->else_stmt);
                if(error != 0){
                    return error;
                }
                exitScope();
            }
            break;
        }
        case ND_ReturnStmt: {
            // 处理 return 语句
            auto returnStmt = node->as<ReturnStmt*>();
            // 检查 void 函数是否返回了值
            if (return_type.back() == TYPE_VOID && returnStmt->exp != nullptr) {
                std::cerr << "Error: void function should not return value\n";
                return 1;
            }
            // 检查 int 函数是否未返回值
            if (return_type.back() == TYPE_INT && returnStmt->exp == nullptr) {
                std::cerr << "Error: int function should return value\n";
                return 1;
            }
            // 执行表达式的语义分析
            if (returnStmt->exp) {
                int error = semanticAnalysis(returnStmt->exp);
                if(error != 0){
                    return error;
                }
                // 检查返回类型是否匹配
                bool typeMismatch = false;
                if (return_type.back() == TYPE_INT && !returnStmt->exp->is<IntExpr>()) {
                    typeMismatch = true;
                }
                if (typeMismatch) {
                    std::cerr << "Error: Return type mismatch.\n";
                    return 1;
                }
            }
            break;
        }
        case ND_WhileStmt: {
            // 处理 while 语句
            auto whileStmt = node->as<WhileStmt*>();
            int error = semanticAnalysis(whileStmt->condition);
            if(error != 0){
                return error;
            }
            enterScope();
            error = semanticAnalysis(whileStmt->body);
            if(error != 0){
                return error;
            }
            exitScope();
            break;
        }
        case ND_AssignStmt: {
            // 处理赋值语句
            auto assignStmt = node->as<AssignStmt*>();
            // 执行左值表达式的语义分析
            int error = semanticAnalysis(assignStmt->lhs);
            if(error != 0){
                return error;
            }
            
            // 执行右值表达式的语义分析
            error = semanticAnalysis(assignStmt->rhs);
            if(error != 0){
                return error;
            }
            if(assignStmt->rhs->as<Exp*>()->exp->node_type == ND_UnaryExp){
                if(assignStmt->rhs->as<Exp*>()->exp->as<UnaryExp*>()->ident_name != ""){
                    std::cerr << "Error: Function assignment is not allowed.\n";
                    if(funcreturn[assignStmt->rhs->as<Exp*>()->exp->as<UnaryExp*>()->ident_name] == "void"){
                        std::cerr << "Error: Function assignment is not allowed.\n";
                        return 1;
                    }
                }
            }
            
            // 检查左值和右值的类型是否匹配
            // if(assignStmt->lhs->as<Lval*>()->lvalexplist != nullptr){
            //     if(assignStmt->rhs->as<Exp*>()->exp->as<UnaryExp*>()->ident_name != ""){
            //         break;
            //     }
            //     if(assignStmt->rhs->as<Exp*>()->exp->as<UnaryExp*>()->primaryexp->as<PrimaryExp*>()->pri_exp->node_type == ND_IntegerLiteral){
            //         std::cerr << "Error: Array assignment is not allowed.\n";
            //         return 1;
            //     }
            // }
            break;
        }
        case ND_Decl: {
            // 处理声明语句
            auto declStmt = node->as<Decl*>();
            return semanticAnalysis(declStmt->vardecl);
            break;
        }
        case ND_InitVal: {
            // 处理初始化值
            auto initVal = node->as<InitVal*>();
            return semanticAnalysis(initVal->exp);
            break;
        }
        case ND_Exp: {
            // 处理表达式
            auto exp = node->as<Exp*>();
            return semanticAnalysis(exp->exp);
            break;
        }
        case ND_PrimaryExp: {
            // 处理基本表达式
            auto primaryExp = node->as<PrimaryExp*>();
            return semanticAnalysis(primaryExp->pri_exp);
            break;
        }
        case ND_VarDecl: {
            // 处理变量声明
            auto varDecl = node->as<VarDecl*>();
            return semanticAnalysis(varDecl->vardeflist);
            break;
        }
        case ND_VarDefList: {
            // 处理变量定义列表
            auto varDefList = node->as<VarDefList*>();
            for (auto varDef : varDefList->children) {
                int error = semanticAnalysis(varDef);
                if(error != 0){
                    return error;
                }
            }
            break;
        }
        case ND_VarDef: {
            // 处理变量定义
            auto varDef = node->as<VarDef*>();
            // 在此进行变量定义的语义分析，比如检查变量是否已经定义过等
            if (symbol_table_var.back().find(varDef->var_name) != symbol_table_var.back().end()) {

                // 如果变量已经定义过，则报错
                std::cerr << "Error: Variable '" << varDef->var_name << "' redefined\n";
                // 可以根据需要进行更详细的错误处理
                return 1;
            } 
            // 遍历 symbol_table_var 中的所有 map
            // 将变量和其维数添加到符号表中
            symbol_table_var.back()[varDef->var_name] = varDef->array_indices;
            // 继续处理变量定义的初始值
            return semanticAnalysis(varDef->init_value);
            break;
        }
        case ND_FuncFParams: {
            // 处理函数参数列表
            auto funcFParams = node->as<FuncFParams*>();
            if (!funcFParams) {
                std::cerr << "Error: Invalid function parameter list node.\n";
                return 1;
            }
            // 逐个分析函数参数
            for (auto funcFParam : funcFParams->children) {
                auto param = funcFParam->as<FuncFParam*>();
                if (!param) {
                    std::cerr << "Error: Invalid function parameter node.\n";
                    return 1;
                }
                // 执行参数的语义分析
                int error = semanticAnalysis(param);
                if (error != 0) {
                    return error;
                }
            }
            break;
        }
        case ND_FuncFParam: {
            // 处理函数参数
            auto funcFParam = node->as<FuncFParam*>();
            if (!funcFParam) {
                std::cerr << "Error: Invalid function parameter node.\n";
                return 1;
            }
            // 在此进行函数参数的语义分析，比如检查参数是否已经定义过等
            // 将参数和其维数添加到符号表中
            symbol_table_var.back()[funcFParam->param_name] = funcFParam->dimensions;
            break;
        }
        case ND_FuncDef: {
            auto funcDef = node->as<FuncDef*>();
            // 检查函数是否已经定义过
            if (funcparam.find(funcDef->func_name) != funcparam.end()) {
                std::cerr << "Error: Function '" << funcDef->func_name << "' redefined\n";
                return 1;
            }
            // 根据 funcDef->return_type 设置返回类型的代码
            int returnTypeCode;
            if (funcDef->return_type == "int") {
                returnTypeCode = 0; // 代码为 0 表示返回类型为 int
            } else if (funcDef->return_type == "void") {
                returnTypeCode = 1; // 代码为 1 表示返回类型为 void
            } else {
                std::cerr << "Error: Unsupported return type '" << funcDef->return_type << "'\n";
                return 1;
            }
            return_type.push_back(returnTypeCode);
            // 注册函数的参数和返回类型到符号表
            std::vector<std::vector<int>> paramDims;
            if (funcDef->params) {
                auto funcFParams = funcDef->params->as<FuncFParams*>();
                for (auto param : funcFParams->children) {
                    auto funcFParam = param->as<FuncFParam*>();
                    paramDims.push_back(funcFParam->dimensions); // 存储参数的维度信息
                }
            }
            funcparam[funcDef->func_name] = paramDims; // 保存参数维度信息
            funcreturn[funcDef->func_name] = funcDef->return_type; // 保存返回类型
            // 创建新的作用域并分析函数体
            enterScope();
            func_block_flag = 1;
            int error = semanticAnalysis(funcDef->params);
            if (error != 0) {
                return error;
            }
            error = semanticAnalysis(funcDef->body);
            if (error != 0) {
                return error;
            }
            // 从返回类型栈中移除当前函数的返回类型
            return_type.pop_back();
            break;
        }
        case ND_BinaryExp: {
            auto binaryExp = node->as<BinaryExp*>();
            if (!binaryExp) {
                std::cerr << "Error: Invalid binary expression node.\n";
                return 1;
            }
            // 递归地对左右子树进行语义分析
            int error = semanticAnalysis(binaryExp->lhs);
            if (error != 0) {
                return error;
            }
            error = semanticAnalysis(binaryExp->rhs);
            if (error != 0) {
                return error;
            }
            break;
        }
             
        case ND_LVal: {
            auto lval = node->as<Lval*>();
            if (!lval) {
                std::cerr << "Error: Invalid Lval node.\n";
                return 1;
            }
            // Check for variable definition in scope
            bool found = false;
            std::map<std::string, std::vector<int>> pick;
            for (auto it = symbol_table_var.rbegin(); it != symbol_table_var.rend() && !found; ++it) {
                if (it->find(lval->ident_name) != it->end()) {
                    pick=*it;
                    found = true;
                }
            }
            if (!found) {
                std::cerr << "Semantic Error : Variable '" << lval->ident_name << "' not defined.\n";
                return 1; // Stop further processing because of undefined variable
            }
            if(lval->lvalexplist != nullptr){
                if(pick[lval->ident_name].size() == 0){
                    std::cerr << "Error: Variable '" << lval->ident_name << "' is not an array\n";
                    return 1;
                }
            }
            
            else if(lval->lvalexplist == nullptr){
                if(pick[lval->ident_name].size() != 0){
                    std::cerr << "Error: Variable '" << lval->ident_name << "' is an array\n";
                    return 1;
                }
            }
            return semanticAnalysis(lval->lvalexplist);
            break;
        }
        case ND_LValExpList: {
            // 处理左值表达式列表
            auto lvalExpList = node->as<LValExpList*>();
            for (auto lval : lvalExpList->children) {
                int error = semanticAnalysis(lval);
                if (error != 0) {
                    return error;
                }
            }
            break;
        }
        case ND_UnaryExp: {
            // 处理函数调用表达式
            auto unaryExp = node->as<UnaryExp*>();
            if (!unaryExp) {
                std::cerr << "Error: Invalid Unary Expression node.\n";
                return 1;
            }
            if(unaryExp->ident_name != ""){
                
                if (unaryExp->funcrparams != nullptr) {
                    // 在此进行函数调用的语义分析，比如检查函数是否已经定义等
                    // 可以使用一个符号表来保存函数定义信息
                    if (funcparam.find(unaryExp->ident_name) == funcparam.end()) {
                        // 如果函数未定义，则报错
                        std::cerr << "Error: Function '" << unaryExp->ident_name << "' not defined\n";
                        return 1;
                    }
                    // 检查函数参数是否匹配
                    auto funcRParams = unaryExp->funcrparams->as<FuncRParams*>();
                    if (funcparam[unaryExp->ident_name].size() != funcRParams->children.size()) {
                        std::cerr << "Error: Function '" << unaryExp->ident_name << "' parameter number does not match\n";
                        return 1;
                    }
                    
                    for (size_t i = 0; i < funcparam[unaryExp->ident_name].size(); i++) {
                        auto exp = funcRParams->children[i]->as<Exp*>();
                        if(exp->exp->node_type==ND_BinaryExp){
                            int error = semanticAnalysis(exp->exp);
                            if(error != 0){
                                return error;
                            }
                            continue;
                        }
                        // 处理函数嵌套调用
                        if(exp->exp->as<UnaryExp*>()->ident_name != ""){
                            if (funcparam.find(exp->exp->as<UnaryExp*>()->ident_name) == funcparam.end()) {
                                // 如果函数未定义，则报错
                                std::cerr << "Error: Function '" << exp->exp->as<UnaryExp*>()->ident_name << "' not defined\n";
                                return 1;
                            }
                            // 检查函数参数是否匹配
                            // if (funcparam[exp->exp->as<UnaryExp*>()->ident_name].size() != 0) {
                            //     std::cout << funcparam[exp->exp->as<UnaryExp*>()->ident_name].size() << std::endl;
                            //     std::cerr << "Error: Function '" << exp->exp->as<UnaryExp*>()->ident_name << "' parameter number does not match\n";
                            //     return 1;
                            // }
                            int error = semanticAnalysis(exp->exp);
                            if(error != 0){
                                return error;
                            }
                            continue;
                        }
                        if(exp->exp->as<UnaryExp*>()->primaryexp->as<PrimaryExp*>()->pri_exp->node_type==ND_IntegerLiteral&&funcparam[unaryExp->ident_name][i].size()==0)
                        {
                            continue;
                        }
                        else{
                            if (!exp) {
                                std::cerr << "Error: Invalid or missing parameter at position " << i + 1 << " for function '" << unaryExp->ident_name << "'\n";
                                return 1;
                            }
                            // 如果参数是变量表达式，则进行定义检查
                            std::vector<int> pick;
                            auto lval = exp->exp->as<UnaryExp*>()->primaryexp->as<PrimaryExp*>()->pri_exp->as<Lval*>(); 
                            bool found = false;
                            for (auto it = symbol_table_var.rbegin(); it != symbol_table_var.rend() && !found; ++it) {
                                if (it->find(lval->ident_name) != it->end()) {
                                    // 用pick保存symbol_table_var中变量的维度信息
                                    pick = it->find(lval->ident_name)->second;
                                    found = true;
                                }
                            }
                            if (!found) {
                                std::cerr << "Error: Parameter '" << lval->ident_name << "' not defined at position " << i + 1 << " for function '" << unaryExp->ident_name << "'\n";
                                return 1;
                            }
                            bool givenArray = pick.size() != 0;
                            if(lval->lvalexplist!=nullptr){
                                givenArray = pick.size()-lval->lvalexplist->as<LValExpList*>()->children.size()!= 0;
                            }
                            if(funcparam[unaryExp->ident_name][i].size() == 0 && givenArray){
                                std::cerr << "Error: Function '" << unaryExp->ident_name << "' parameter " << i + 1 << " is not an array\n";
                                return 1;
                            }
                            if(funcparam[unaryExp->ident_name][i].size() != 0 && !givenArray){
                                std::cerr << "Error: Function '" << unaryExp->ident_name << "' parameter " << i + 1 << " is an array\n";
                                return 1;
                            }
                            if (givenArray) {
                                // 拼接维度信息，将exp的维度与函数参数的维度进行拼接

                                // 创建一个新的 vector
                                std::vector<int> newContainer;

                                // 获取需要拼接的两个 vector
                                std::vector<int> vec2 = funcparam[unaryExp->ident_name][i];
                                std::vector<int> vec1;
                                if(lval->lvalexplist!=nullptr){
                                    for(size_t i = 0; i < lval->lvalexplist->as<LValExpList*>()->children.size(); i++){
                                        vec1.push_back(lval->lvalexplist->as<LValExpList*>()->children[i]->as<Exp*>()->exp->as<UnaryExp*>()->primaryexp->as<PrimaryExp*>()->pri_exp->as<IntegerLiteral*>()->value);
                                    }
                                }
                                
                                // 使用 insert 方法将 vec1 和 vec2 的内容添加到 newContainer 中
                                newContainer.insert(newContainer.end(), vec1.begin(), vec1.end());
                                newContainer.insert(newContainer.end(), vec2.begin(), vec2.end());
                                
                                auto givenDims = newContainer;
                                auto expectedDims = pick;
                                if (givenDims.size() != expectedDims.size()) {
                                    std::cerr << "Error: Dimension mismatch for parameter " << i + 1 << " of function '" << unaryExp->ident_name << "'\n";
                                    return 1;
                                }
                                for (size_t j = 0; j < givenDims.size(); j++) {
                                    if (givenDims[j] > expectedDims[j]&&givenDims[j]!=-1) {
                                        std::cerr << "Error: Dimension size mismatch at dimension " << j + 1 << " of parameter " << i + 1 << " for function '" << unaryExp->ident_name << "'\n";
                                        return 1;
                                    }
                                }
                            }
                        }
                    }
                } 
                else {
                    // 在此进行函数调用的语义分析，比如检查函数是否已经定义等
                    // 可以使用一个符号表来保存函数定义信息
                    if (funcparam.find(unaryExp->ident_name) == funcparam.end()) {
                        // 如果函数未定义，则报错
                        std::cerr << "Error: Function '" << unaryExp->ident_name << "' not defined\n";
                        return 1;
                    }
                    // 检查函数参数是否匹配
                    if (funcparam[unaryExp->ident_name].size() != 0) {
                        std::cerr << "Error: Function '" << unaryExp->ident_name << "' parameter number does not match\n";
                        return 1;
                    }
                }
            }
            else if (unaryExp->primaryexp != nullptr) {
                return semanticAnalysis(unaryExp->primaryexp);
            } else if (unaryExp->operand != nullptr) {
                return semanticAnalysis(unaryExp->operand);
            }
            break;
        }
        
        default:
            // 其他节点类型暂不处理
            break;
    }
    return 0;
}

