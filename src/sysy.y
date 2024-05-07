%{
#include <stdio.h>
#include <ast/ast.h>
void yyerror(const char *s);
extern int yylex(void);

NodePtr root = new CompUnit();

#define SYNTAXTESTPRINT 0
%}

/// types
%union {
    int ival;
    char *ident;
    NodePtr node;
    OpType op;
}

%token <ival> INTCONST
%token <ident> IDENT

%token ADD SUB MUL DIV ASSIGN GT GE LT LE EQ NEQ MOD AND OR NOT 
%token IF ELSE WHILE BREAK CONTINUE RETURN TRUE FALSE VOID INT
%token LPAREN RPAREN LBRACKET RBRACKET LBRACE RBRACE COMMA SEMICOLON

%type <node> CompUnit Decl BType VarDecl VarDefList VarDef ArrayIndexList InitVal
%type <node> FuncDef FuncFParams FuncFParam NumList Block BlockItemList BlockItem
%type <node> Stmt Exp LVal LValExpList PrimaryExp Number UnaryExp UnaryOp FuncRParams
%type <node> MulExp AddExp RelExp EqExp LOrExp LAndExp
 
%left AND OR
%left ASSIGN GT GE LT LE EQ NEQ
%left ADD SUB 
%left MUL DIV MOD
%left NOT

%%
CompUnit    : Decl                                      { root->as<CompUnit*>()->children.push_back($1);  if (SYNTAXTESTPRINT) { printf("[CompUnit->Decl]\n"); }}
            | FuncDef                                   { root->as<CompUnit*>()->children.push_back($1);  if (SYNTAXTESTPRINT) { printf("[CompUnit->FuncDef]\n"); }}
            | CompUnit Decl                             { root->as<CompUnit*>()->children.push_back($2);  if (SYNTAXTESTPRINT) { printf("[CompUnit->CompUnit Decl]\n"); } }
            | CompUnit FuncDef                          { root->as<CompUnit*>()->children.push_back($2);  if (SYNTAXTESTPRINT) { printf("[CompUnit->CompUnit FuncDef]\n"); } }
            ;

Decl        : VarDecl                                   { $$ = new Decl($1); if (SYNTAXTESTPRINT) { printf("[Decl->VarDecl]\n"); } }
            ;

BType       : INT                                       { $$ = new BType("int"); if (SYNTAXTESTPRINT) { printf("[BType->INT]\n"); }}
            | VOID                                      { $$ = new BType("void"); }
            ;

VarDecl     : BType VarDefList SEMICOLON                { $$ = new VarDecl(static_cast<BType*>($1),static_cast<VarDefList*>($2));  if (SYNTAXTESTPRINT) { printf("VarDecl->BType VarDef VarDefList SEMICLONO \n"); } }
            ;

VarDefList  : VarDefList COMMA VarDef                   { $$ = $1; $$->as<VarDefList*>()->children.push_back($3->as<VarDef*>());  if (SYNTAXTESTPRINT) { printf("[VarDefList->VarDefList COMMA VarDef] \n"); }}
            | VarDef                                    { $$ = new VarDefList(); $$->as<VarDefList*>()->children.push_back($1->as<VarDef*>());  if (SYNTAXTESTPRINT) { printf("[VarDefList->VarDef] \n"); }}
            ;

VarDef      : IDENT ArrayIndexList  { $$ = new VarDef($1, nullptr); $$->as<VarDef*>()->array_indices = $2->as<ArrayIndexList*>()->indices;  if (SYNTAXTESTPRINT) { printf("[VarDef->IDENT ArrayIndexList] \n"); }}
            | IDENT ASSIGN InitVal  { $$ = new VarDef($1, $3); if (SYNTAXTESTPRINT) { printf("[VarDef->IDENT ASSIGN InitVal] \n"); }}
            | IDENT                 { $$ = new VarDef($1, nullptr); if (SYNTAXTESTPRINT) { printf("[VarDef->IDENT] \n"); }}

ArrayIndexList 
            : ArrayIndexList LBRACKET INTCONST RBRACKET  { $$ = $1; $$->as<ArrayIndexList*>()->indices.push_back($3);  if (SYNTAXTESTPRINT) { printf("[ArrayIndexList : ArrayIndexList ArrayIndex] \n"); }}
            | LBRACKET INTCONST RBRACKET                 { $$ = new ArrayIndexList(); $$->as<ArrayIndexList*>()->indices.push_back($2); }
            ;

InitVal     : Exp                                       { $$ = new InitVal($1); }

FuncDef     : BType IDENT LPAREN RPAREN Block                       { $$ = new FuncDef($1->as<BType*>(),$2,nullptr,$5); }
            | BType IDENT LPAREN FuncFParams RPAREN Block           { $$ = new FuncDef($1->as<BType*>(),$2,$4,$6); }
            ;

FuncFParams : FuncFParams COMMA FuncFParam                  { $$ = $1; $$->as<FuncFParams*>()->children.push_back($3->as<FuncFParam*>()); }
            | FuncFParam                                    { $$ = new FuncFParams(); $$->as<FuncFParams*>()->children.push_back($1->as<FuncFParam*>()); }
            ;

FuncFParam  : BType IDENT                                   { $$ = new FuncFParam($1->as<BType*>(),$2,nullptr); }
            | BType IDENT NumList                           { $$ = new FuncFParam($1->as<BType*>(),$2,$3->as<NumList*>()); }
            ;

NumList     : NumList LBRACKET INTCONST RBRACKET        { $$ = $1; $$->as<NumList*>()->children.push_back($3); }
            | LBRACKET RBRACKET                         { $$ = new NumList(); }
            ;

Block           : LBRACE RBRACE                         { $$ = new Block(nullptr); }
                | LBRACE BlockItemList RBRACE           { $$ = new Block($2); }
                ;

BlockItemList   : BlockItemList BlockItem               { $$ = $1; $$->as<BlockItemList*>()->children.push_back($1->as<BlockItem*>()); }
                | BlockItem                             { $$ = new BlockItemList(); $$->as<BlockItemList*>()->children.push_back($1->as<BlockItem*>());}
                ;

BlockItem       : Decl                                  { $$ = new BlockItem($1); }
                | Stmt                                  { $$ = new BlockItem($1); }
                ;

Stmt            : LVal ASSIGN Exp SEMICOLON                     { $$ = new Stmt(new AssignStmt($1,$3)); }
                | Exp SEMICOLON                                 { $$ = new Stmt($1); }
                | Block                                         { $$ = new Stmt($1); }
                | IF LPAREN Exp RPAREN Stmt ELSE Stmt           { $$ = new Stmt(new IfStmt($3,$5,$7)); }
                | IF LPAREN Exp RPAREN Stmt                     { $$ = new Stmt(new IfStmt($3,$5,nullptr)); }
                | WHILE LPAREN Exp RPAREN Stmt                  { $$ = new Stmt(new WhileStmt($3,$5)); }
                | BREAK SEMICOLON                               { $$ = new Stmt(new BreakStmt()); }
                | CONTINUE SEMICOLON                            { $$ = new Stmt(new ContinueStmt()); }
                | RETURN SEMICOLON                              { $$ = new Stmt(new ReturnStmt(nullptr)); }
                | RETURN Exp SEMICOLON                          { $$ = new Stmt(new ReturnStmt($2)); }
                ;

Exp         : LOrExp                                { $$ = $1; }
            ;

LVal        : IDENT                                 { $$ = new Lval($1,nullptr); }
            | IDENT LValExpList                     { $$ = new Lval($1,$2); }
            ;

LValExpList : LValExpList LBRACKET Exp RBRACKET     { $$ = $1; $$->as<LValExpList*>()->children.push_back($3); }
            | LBRACKET Exp RBRACKET                 { $$ = new LValExpList(); $$->as<LValExpList*>()->children.push_back($2); }
            ;

PrimaryExp  : LPAREN Exp RPAREN                     { $$ = new PrimaryExp($2); }
            | LVal                                  { $$ = new PrimaryExp($1); }
            | Number                                { $$ = new PrimaryExp($1); }
            ;

Number      : INTCONST                              { $$ = new IntegerLiteral($1); }
            ;

UnaryExp    : PrimaryExp                            { $$ = new UnaryExp($1); }
            | IDENT LPAREN RPAREN                   { $$ = new UnaryExp($1,nullptr); }
            | IDENT LPAREN FuncRParams RPAREN       { $$ = new UnaryExp($1,$3); }
            | ADD UnaryExp                          { $$ = new UnaryExp(OpType::OP_Pos, $2); }
            | SUB UnaryExp                          { $$ = new UnaryExp(OpType::OP_Neg, $2); }
            | NOT UnaryExp                          { $$ = new UnaryExp(OpType::OP_Lnot, $2); }
            ;

FuncRParams : FuncRParams COMMA Exp                  { $$ = $1; $$->as<FuncRParams*>()->children.push_back($3); }
            | Exp                                    { $$ = new FuncRParams(); $$->as<FuncRParams*>()->children.push_back($1); }
            ;

MulExp      : UnaryExp                              { $$ = $1; if (SYNTAXTESTPRINT) { printf("[MulExp->UnaryExp] \n"); } }
            | MulExp MUL UnaryExp                   { $$ = new BinaryExp(OpType::OP_Mul, $1, $3);  if (SYNTAXTESTPRINT) { printf("[MulExp->MulExp MUL UnaryExp] \n"); } }
            | MulExp DIV UnaryExp                   { $$ = new BinaryExp(OpType::OP_Div, $1, $3);  if (SYNTAXTESTPRINT) { printf("[MulExp->MulExp DIV UnaryExp] \n"); } }
            | MulExp MOD UnaryExp                   { $$ = new BinaryExp(OpType::OP_Mod, $1, $3);  if (SYNTAXTESTPRINT) { printf("[MulExp->MulExp MOD UnaryExp] \n"); } }
            ;

AddExp      : MulExp                                { $$ = $1;   if (SYNTAXTESTPRINT) { printf("[AddExp->MulExp] \n"); }  }
            | AddExp ADD MulExp                     { $$ = new BinaryExp(OpType::OP_Mul, $1, $3);  if (SYNTAXTESTPRINT) { printf("[AddExp->AddExp ADD MulExp] \n"); } }
            | AddExp SUB MulExp                     { $$ = new BinaryExp(OpType::OP_Mul, $1, $3);  if (SYNTAXTESTPRINT) { printf("[AddExp->AddExp SUB MulExp] \n"); } }
            ;

RelExp      : AddExp                                { $$ = $1;  if (SYNTAXTESTPRINT) { printf("[RelExp->AddExp] \n"); } }
            | RelExp GT AddExp                      { $$ = new BinaryExp(OpType::OP_Gt, $1, $3); if (SYNTAXTESTPRINT) { printf("[RelExp->RelExp GT AddExp] \n"); } }
            | RelExp GE AddExp                      { $$ = new BinaryExp(OpType::OP_Ge, $1, $3); if (SYNTAXTESTPRINT) { printf("[RelExp->RelExp GE AddExp] \n"); } }
            | RelExp LT AddExp                      { $$ = new BinaryExp(OpType::OP_Lt, $1, $3); if (SYNTAXTESTPRINT) { printf("[RelExp->RelExp LT AddExp] \n"); } }
            | RelExp LE AddExp                      { $$ = new BinaryExp(OpType::OP_Le, $1, $3); if (SYNTAXTESTPRINT) { printf("[RelExp->RelExp LE AddExp] \n"); } }
            ;

EqExp       : RelExp                                { $$ = $1; if (SYNTAXTESTPRINT) { printf("[EqExp->RelExp] \n"); } }
            | EqExp EQ RelExp                       { $$ = new BinaryExp(OpType::OP_Eq, $1, $3); if (SYNTAXTESTPRINT) { printf("[EqExp->EqExp EQ RelExp] \n"); } }
            | EqExp NEQ RelExp                      { $$ = new BinaryExp(OpType::OP_Ne, $1, $3); if (SYNTAXTESTPRINT) { printf("[EqExp->EqExp NEQ RelExp] \n"); } }
            ;

LAndExp     : EqExp                                 { $$ = $1; if (SYNTAXTESTPRINT) { printf("[LAndExp->EqExp] \n"); } }
            | LAndExp AND EqExp                     { $$ = new BinaryExp(OpType::OP_Land, $1, $3); if (SYNTAXTESTPRINT) { printf("[LAndExp->LAndExp AND EqExp] \n"); } }
            ;

LOrExp      : LAndExp                               { $$ = $1; if (SYNTAXTESTPRINT) { printf("[LOrExp->LAndExp] \n"); }}
            | LOrExp OR LAndExp                     { $$ = new BinaryExp(OpType::OP_Lor, $1, $3); if (SYNTAXTESTPRINT) { printf("[LOrExp->LAndExp OR LAndExp] \n"); }}
            ;

%%

void yyerror(const char *s) {
    printf("error: %s\n", s);
}
