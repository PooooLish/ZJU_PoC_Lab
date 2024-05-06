#include <cstdint>
#include <type_traits>
#include <string>
#include <vector>

// 定义操作符类型枚举
enum OpType {
#define OpcodeDefine(x, s) x,
#include "common/common.def"
};

// 定义变量类型枚举
// enum CastType {
// #define CastTypeDefine(x, s) x,
// #include "common/common.def"
// };

// 定义节点类型枚举
enum NodeType {
#define TreeNodeDefine(x) x,
#include "common/common.def"
};

// 前置声明
struct Node;
using NodePtr = Node*;
struct Block;
using BlockPtr = Block*;
struct TreeExp;
using ExprPtr = TreeExp*;
struct TreeType;

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
};

// 表达式节点基类
struct TreeExp : public Node {
    TreeExp(NodeType type) : Node(type) {}
};

// 基本表达式节点
struct PrimaryExp : public TreeExp {

};

// 二元表达式节点
struct BinaryExp : public TreeExp {
    constexpr static NodeType this_type = ND_BinaryExp;
    OpType op;
    ExprPtr lhs, rhs;
    BinaryExp(OpType op, ExprPtr lhs, ExprPtr rhs)
        : TreeExp(this_type), op(op), lhs(lhs), rhs(rhs) {
    }
};

// 一元表达式节点
struct UnaryExp : public TreeExp {
    constexpr static NodeType this_type = ND_UnaryExp;
    OpType op;
    ExprPtr operand;
    UnaryExp(OpType op, ExprPtr operand)
        : TreeExp(this_type), op(op), operand(operand) {
    }
};

// 整数字面量节点
struct IntegerLiteral : public TreeExp {
    constexpr static NodeType this_type = ND_IntegerLiteral;
    int64_t value;
    IntegerLiteral(int64_t value) : TreeExp(this_type), value(value) {}
};

// 字符串字面量节点
// struct StringLiteral : public TreeExp {
//     constexpr static NodeType this_type = ND_StringLiteral;
//     std::string value;
//     ExprPtr operand;
//     StringLiteral(std::string value) : TreeExp(this_type), value(value) {}
//     StringLiteral(std::string value, ExprPtr operand) : TreeExp(this_type), value(value) ,operand(operand){}
// };

// 类型判断表达式节点
// struct TypeCast : public TreeExp {
//     constexpr static NodeType this_type = ND_TypeCast;
//     CastType type;
//     TypeCast(CastType type)
//         : TreeExp(this_type), type(type) {
//     }
// };

// type节点
struct BType : public TreeExp {
    constexpr static NodeType this_type = ND_BType;
    std::string type;
    BType(std::string type): TreeExp(this_type), type(type) {}
};

struct FuncType : public TreeExp {
    constexpr static NodeType this_type = ND_FuncType;
    std::string type;
    FuncType(std::string type): TreeExp(this_type), type(type) {}
};

// 根节点
struct CompUnit : public Node {
    constexpr static NodeType this_type = ND_CompUnit;
    std::vector<NodePtr> children;
    CompUnit() : Node(this_type) {}
};

// 函数定义节点
struct FuncDef : public Node {
    constexpr static NodeType this_type = ND_FuncDef;
    std::string func_name;
    std::vector<std::string> param_types;
    std::string return_type;
    BlockPtr body;
    FuncDef(std::string name, std::vector<std::string> params, std::string ret_type, BlockPtr block)
        : Node(this_type), func_name(name), param_types(params), return_type(ret_type), body(block) {}
};

struct FuncFParam : public Node {

};

struct FuncFParams : public Node {

};


// 块节点
struct Block : public Node {
    constexpr static NodeType this_type = ND_Block;
    std::vector<NodePtr> statements;
    Block() : Node(this_type) {}
};

struct BlockItem : public Node {

};

struct BlockItemList : public Node {

};

struct Stmt : public Node {

};

// 变量定义节点
struct VarDef : public Node {
    constexpr static NodeType this_type = ND_VarDef;
    std::string var_name;
    std::string var_type;
    ExprPtr init_value;
    std::vector<int> array_indices; // 添加数组索引
    VarDef(std::string name, std::string type, ExprPtr init)
        : Node(this_type), var_name(name), var_type(type), init_value(init) {}
};

// 变量声明节点
struct VarDecl : public Node {
    constexpr static NodeType this_type = ND_VarDecl;
    std::string btype;
    std::vector<VarDef> VarDefList;
    VarDecl(BType BType, std::vector<struct VarDef> init)
        : Node(this_type), btype(BType.type), VarDefList(init) {}
};

// 变量定义队列
std::vector<VarDef> VarDefList;

// 数组索引列表节点
struct ArrayIndexList : public Node {
    constexpr static NodeType this_type = ND_ArrayIndexList;
    std::vector<int> indices;
    ArrayIndexList() : Node(this_type) {}
};

// 标识符节点
struct Identifier : public Node {
    constexpr static NodeType this_type = ND_Identifier;
    std::string var_name;
    Identifier(std::string name) : Node(this_type), var_name(name) {}
};

// 打印表达式节点的可能辅助函数
void print_expr(ExprPtr exp, std::string prefix = "", std::string ident = "");
