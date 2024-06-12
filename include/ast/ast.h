#include <cstdint>
#include <type_traits>
#include <string>
#include <vector>
#include <iostream>

// 定义操作符类型枚举
enum OpType {
#define OpcodeDefine(x, s) x,
#include "common/common.def"
};
// 定义变量类型枚举
enum CastType {
#define CastTypeDefine(x) x,
#include "common/common.def"
};
// 定义节点类型枚举
enum NodeType {
#define TreeNodeDefine(x) x,
#include "common/common.def"
};

// 前置声明
struct Node;
using NodePtr = Node*;

// 节点基类
struct Node {
    NodeType node_type;
    Node(NodeType type) : node_type(type) {}
    // 检查节点是否为特定类型
    template <typename T> bool is() {
        return node_type == std::remove_pointer_t<T>::this_type;
    }
    // 将节点转换为特定类型（安全转换）
    template <typename T> T as() {
        if (is<T>())
            return static_cast<T>(this);
        return nullptr;
    }
    // 将节点转换为特定类型（不安全转换）
    template <typename T> T as_unchecked() { return static_cast<T>(this); }
    
    virtual ~Node() = default;
};

struct Exp :public Node {
    constexpr static NodeType this_type = ND_Exp; 
    NodePtr exp;
    Exp(NodePtr exp) : Node(this_type), exp(exp) {}
    std::vector<int> dimensions;  // 存储数组的维度信息，非数组表达式为空

     // 判断表达式是否为整数表达式
    virtual bool isIntExpr() const  { return false; }
    // 判断表达式是否为数组表达式，基于维度信息
    virtual bool isArrayExpr() const  { return !dimensions.empty(); }

    // 获取数组的维度信息
    const std::vector<int>& getArrayDimensions() const {
        return dimensions;
    }
    // 设置表达式的数组维度信息
    void setArrayDimensions(const std::vector<int>& dims) {
        dimensions = dims;
    }
};

struct IntExpr : public Exp {
    IntExpr(NodePtr e = nullptr) : Exp(e) {}
    
    bool isIntExpr() const override { return true; }
};
struct ArrayExpr : public Exp {
    ArrayExpr(NodePtr e = nullptr) : Exp(e) {}
    
    bool isArrayExpr() const override { return true; }
};

// 基本表达式节点
struct PrimaryExp : public Node {
    constexpr static NodeType this_type = ND_PrimaryExp;
    NodePtr pri_exp;
    PrimaryExp(NodePtr pri_exp) : Node(this_type) , pri_exp(pri_exp) {}
};

// 一元表达式节点
struct UnaryExp : public Node {
    constexpr static NodeType this_type = ND_UnaryExp;
    OpType op;
    std::string ident_name = "";
    NodePtr funcrparams = nullptr;
    NodePtr primaryexp = nullptr;
    NodePtr operand = nullptr;
    UnaryExp(NodePtr primaryexp) 
        : Node(this_type),primaryexp(primaryexp) {}
    UnaryExp(std::string ident_name, NodePtr funcrparams) 
        : Node(this_type), ident_name(ident_name), funcrparams(funcrparams) {}
    UnaryExp(OpType op, NodePtr operand)
        : Node(this_type), op(op), operand(operand) {}
};

// 二元表达式节点
struct BinaryExp : public Node {
    constexpr static NodeType this_type = ND_BinaryExp;
    OpType op;
    NodePtr lhs, rhs;
    BinaryExp(OpType op, NodePtr lhs, NodePtr rhs)
        : Node(this_type), op(op), lhs(lhs), rhs(rhs) {
    }
};

struct FuncRParams : public Node {
    constexpr static NodeType this_type = ND_FuncRParams;
    std::vector<NodePtr> children;
    FuncRParams() : Node(this_type) {}
};

// 整数字面量节点
struct IntegerLiteral : public Node {
    constexpr static NodeType this_type = ND_IntegerLiteral;
    int64_t value;
    IntegerLiteral(int64_t value) : Node(this_type), value(value) {}
};

struct BType : public Node {
    constexpr static NodeType this_type = ND_BType;
    std::string type;
    BType(std::string type): Node(this_type), type(type) {}
};

// 根节点
struct CompUnit : public Node {
    constexpr static NodeType this_type = ND_CompUnit;
    std::vector<NodePtr> children;
    CompUnit() : Node(this_type) {}
};

// 声明
struct Decl : public Node {
    constexpr static NodeType this_type = ND_Decl;
    NodePtr vardecl;
    Decl(NodePtr decl) : Node(this_type),vardecl(decl) {}
};

struct InitVal : public Node {
    constexpr static NodeType this_type = ND_InitVal;
    NodePtr exp;
    InitVal(NodePtr exp) : Node(this_type), exp(exp) {}
};

// 变量定义节点
struct VarDef : public Node {
    constexpr static NodeType this_type = ND_VarDef;
    std::string var_name;
    NodePtr init_value;
    std::vector<int> array_indices; // 添加数组索引
    VarDef(std::string name, NodePtr init)
        : Node(this_type), var_name(name), init_value(init) {}
};

// 变量定义队列
struct VarDefList : public Node {   
    constexpr static NodeType this_type = ND_VarDefList;
    std::vector<NodePtr> children;
    VarDefList() : Node(this_type) {}
};

// 变量声明节点
struct VarDecl : public Node {
    constexpr static NodeType this_type = ND_VarDecl;
    std::string btype;
    // std::vector<VarDef*> vardeflist;
    NodePtr vardeflist;
    VarDecl(BType* BType, NodePtr VarDefList)
        : Node(this_type), btype(BType->type), vardeflist(VarDefList) {}
};

// 数组索引列表节点
struct ArrayIndexList : public Node {
    constexpr static NodeType this_type = ND_ArrayIndexList;
    std::vector<int> indices;
    ArrayIndexList() : Node(this_type) {}
};

// 参数维度
struct NumList : public Node {
    constexpr static NodeType this_type = ND_NumList;
    std::vector<int> children;
    NumList() : Node(this_type) {}
};

// 函数形参
struct FuncFParam : public Node {
    constexpr static NodeType this_type = ND_FuncFParam;
    std::string param_type;
    std::string param_name;
    std::vector<int> dimensions; // 存储数组的每个维度的大小
    bool is_array; // 标记是否为数组
    NumList* numlist;

    FuncFParam(BType* paramtype, std::string name, NumList* list)
        : Node(this_type), param_type(paramtype->type), param_name(name), numlist(list){
        if (list != nullptr) {
            // 直接使用 NumList 中的 children 初始化 dimensions
            this->is_array = true;
            this->dimensions = list->children; 
        } else {
            this->is_array = false;
        }
    }
};

// 函数形参列表
struct FuncFParams : public Node {
    constexpr static NodeType this_type = ND_FuncFParams;
    std::vector<NodePtr> children;
    FuncFParams() : Node(this_type) {}
};

// 函数定义
struct FuncDef : public Node {
    constexpr static NodeType this_type = ND_FuncDef;
    std::string return_type;
    std::string func_name;
    NodePtr params;
    NodePtr body;

    FuncDef(BType* functype, std::string name , NodePtr params, NodePtr block)
        : Node(this_type), return_type(functype->type), func_name(name), params(params), body(block) {}
};

// 块节点
struct Block : public Node {
    constexpr static NodeType this_type = ND_Block;
    NodePtr itemList;
    Block(NodePtr list) : Node(this_type), itemList(list) {}
};

struct BlockItemList : public Node {
    constexpr static NodeType this_type = ND_BlockItemList;
    std::vector<NodePtr> children;
    BlockItemList() : Node(this_type) {}
};

struct BlockItem : public Node {
    constexpr static NodeType this_type = ND_BlockItem;
    NodePtr item;
    BlockItem(NodePtr stmt_decl) : Node(this_type) , item(stmt_decl) {}
};

// Stmt
struct Stmt : public Node {
    constexpr static NodeType this_type = ND_Stmt;
    NodePtr stmtPtr;
    Stmt(NodePtr stmtptr): Node(this_type), stmtPtr(stmtptr) {}
};

struct AssignStmt : public Node {
    constexpr static NodeType this_type = ND_AssignStmt;
    NodePtr lhs;
    NodePtr rhs;
    AssignStmt(NodePtr left, NodePtr right)
        : Node(this_type), lhs(left), rhs(right) {}
};

struct ReturnStmt : public Node {
    constexpr static NodeType this_type = ND_ReturnStmt;
    NodePtr exp;
    ReturnStmt(NodePtr expression)
        : Node(this_type), exp(expression) {}
};

struct IfStmt : public Node {
    constexpr static NodeType this_type = ND_IfStmt;
    NodePtr condition;
    NodePtr then_stmt;
    NodePtr else_stmt;
    IfStmt(NodePtr cond, NodePtr thenStmt, NodePtr elseStmt)
        : Node(this_type), condition(cond), then_stmt(thenStmt), else_stmt(elseStmt) {}
};

struct WhileStmt : public Node {
    constexpr static NodeType this_type = ND_WhileStmt;
    NodePtr condition;
    NodePtr body;
    WhileStmt(NodePtr cond, NodePtr stmt)
        : Node(this_type), condition(cond), body(stmt) {}
};

struct BreakStmt : public Node {
    constexpr static NodeType this_type = ND_BreakStmt;
    BreakStmt() : Node(this_type) {}
};

struct ContinueStmt : public Node {
    constexpr static NodeType this_type = ND_ContinueStmt;
    ContinueStmt() : Node(this_type) {}
};

// 左值表达式
struct Lval : public Node {
    constexpr static NodeType this_type = ND_LVal;
    std::string ident_name;
    NodePtr lvalexplist;
    Lval(std::string name , NodePtr lvalexplist) 
        : Node(this_type), ident_name(name), lvalexplist(lvalexplist) {}
};

// 左值表达式rhs
struct LValExpList : public Node {
    constexpr static NodeType this_type = ND_LValExpList;
    std::vector<NodePtr> children;
    LValExpList() : Node(this_type) {}
};

void print_expr(NodePtr exp, std::string prefix = "") ;