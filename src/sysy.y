%{
#include <stdio.h>
#include <ast/ast.h>
void yyerror(const char *s);
extern int yylex(void);

NodePtr root = new CompUnit();

#define TESTPRINT 0
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
CompUnit    : Decl                                      { root->as<CompUnit*>()->children.push_back($1);  if (TESTPRINT) { printf("[CompUnit->Decl]\n"); }}
            | FuncDef                                   { root->as<CompUnit*>()->children.push_back($1);  if (TESTPRINT) { printf("[CompUnit->FuncDef]\n"); }}
            | CompUnit Decl                             { root->as<CompUnit*>()->children.push_back($2);  if (TESTPRINT) { printf("[CompUnit->CompUnit Decl]\n"); } }
            | CompUnit FuncDef                          { root->as<CompUnit*>()->children.push_back($2);  if (TESTPRINT) { printf("[CompUnit->CompUnit FuncDef]\n"); } }
            ;

Decl        : VarDecl                                   { $$ = new Decl($1); if (TESTPRINT) { printf("[Decl->VarDecl]\n"); } }
            ;

BType       : INT                                       { $$ = new BType("int"); if (TESTPRINT) { printf("[BType->INT]\n"); }}
            | VOID                                      { $$ = new BType("void"); }
            ;

VarDecl     : BType VarDefList SEMICOLON                { $$ = new VarDecl(static_cast<BType*>($1),$2);  if (TESTPRINT) { printf("VarDecl->BType VarDef VarDefList SEMICLONO \n"); } }
            ;

VarDefList  : VarDefList COMMA VarDef                   { $$ = $1; $$->as<VarDefList*>()->children.push_back($3);  if (TESTPRINT) { printf("[VarDefList->VarDefList COMMA VarDef] \n"); }}
            | VarDef                                    { $$ = new VarDefList(); $$->as<VarDefList*>()->children.push_back($1);  if (TESTPRINT) { printf("[VarDefList->VarDef] \n"); }}
            ;

VarDef      : IDENT ArrayIndexList  { $$ = new VarDef($1, nullptr); $$->as<VarDef*>()->array_indices = $2->as<ArrayIndexList*>()->indices;  if (TESTPRINT) { printf("[VarDef->IDENT ArrayIndexList] \n"); }}
            | IDENT ASSIGN InitVal  { $$ = new VarDef($1, $3); if (TESTPRINT) { printf("[VarDef->IDENT ASSIGN InitVal] \n"); }}
            | IDENT                 { $$ = new VarDef($1, nullptr); if (TESTPRINT) { printf("[VarDef->IDENT] \n"); }}

ArrayIndexList 
            : ArrayIndexList LBRACKET INTCONST RBRACKET  { $$ = $1; $$->as<ArrayIndexList*>()->indices.push_back($3);  if (TESTPRINT) { printf("[ArrayIndexList : ArrayIndexList ArrayIndex] \n"); }}
            | LBRACKET INTCONST RBRACKET                 { $$ = new ArrayIndexList(); $$->as<ArrayIndexList*>()->indices.push_back($2); if (TESTPRINT) { printf("[VarDefList->VarDef] \n"); } }
            ;

InitVal     : Exp                                        { $$ = new InitVal($1); if (TESTPRINT) { printf("[InitVal->Exp] \n"); }}

FuncDef     : BType IDENT LPAREN RPAREN Block                       { $$ = new FuncDef($1->as<BType*>(),$2,nullptr,$5); }
            | BType IDENT LPAREN FuncFParams RPAREN Block           { $$ = new FuncDef($1->as<BType*>(),$2,$4,$6); }
            ;

FuncFParams : FuncFParams COMMA FuncFParam                  { $$ = $1; $$->as<FuncFParams*>()->children.push_back($3);  if (TESTPRINT) { printf("[FuncDef->FuncType IDENT LPAREN RPAREN Block] \n");} }
            | FuncFParam                                    { $$ = new FuncFParams(); $$->as<FuncFParams*>()->children.push_back($1); if (TESTPRINT) { printf("[FuncDef->FuncType IDENT LPAREN FuncFParams RPAREN Block] \n"); } }
            ;

FuncFParam  : BType IDENT                                   { $$ = new FuncFParam($1->as<BType*>(),$2,nullptr); if (TESTPRINT) { printf("[FuncFParams->FuncFParamList COMMA FuncFParam] \n"); } }
            | BType IDENT NumList                           { $$ = new FuncFParam($1->as<BType*>(),$2,$3->as<NumList*>()); if (TESTPRINT) { printf("[FuncFParams->FuncFParam] \n"); } }
            ;

NumList     : NumList LBRACKET INTCONST RBRACKET        { $$ = $1; $$->as<NumList*>()->children.push_back($3); if (TESTPRINT) { printf("[NumList->NumList LBRACKET INTCONST RBRACKET] \n"); }}
            | LBRACKET RBRACKET                         { $$ = new NumList();$$->as<NumList*>()->children.push_back(-1); if (TESTPRINT) { printf("[NumList->LBRACKET RBRACKET] \n"); }}
            ;

Block           : LBRACE RBRACE                         { $$ = new Block(nullptr);  if (TESTPRINT) { printf("[Block->LBRACE RBRACE] \n"); }}
                | LBRACE BlockItemList RBRACE           { $$ = new Block($2);  if (TESTPRINT) { printf("[Block->LBRACE BlockItemList RBRACE] \n"); } }
                ;

BlockItemList   : BlockItemList BlockItem               { $$ = $1; $$->as<BlockItemList*>()->children.push_back($2); if (TESTPRINT) { printf("[BlockItemList->BlockItemList BlockItem] \n"); } }
                | BlockItem                             { $$ = new BlockItemList(); $$->as<BlockItemList*>()->children.push_back($1); if (TESTPRINT) { printf("[BlockItemList->BlockItem] \n"); } }
                ;

BlockItem       : Decl                                  { $$ = new BlockItem($1); if (TESTPRINT) { printf("[BlockItem->Decl] \n"); } }
                | Stmt                                  { $$ = new BlockItem($1); if (TESTPRINT) { printf("[BlockItem->Stmt] \n"); } }
                ;

Stmt            : LVal ASSIGN Exp SEMICOLON                     { $$ = new Stmt(new AssignStmt($1,$3)); if (TESTPRINT) { printf("[Stmt->LVal ASSIGN Exp SEMICOLON] \n"); }}
                | Exp SEMICOLON                                 { $$ = new Stmt($1); if (TESTPRINT) { printf("[Stmt->Exp SEMICOLON] \n"); }}
                | Block                                         { $$ = new Stmt($1); if (TESTPRINT) { printf("[Stmt->Block] \n"); }}
                | IF LPAREN Exp RPAREN Stmt ELSE Stmt           { $$ = new Stmt(new IfStmt($3,$5,$7)); if (TESTPRINT) { printf("[Stmt->IF LPAREN Exp RPAREN Stmt ELSE Stmt] \n"); } }
                | IF LPAREN Exp RPAREN Stmt                     { $$ = new Stmt(new IfStmt($3,$5,nullptr)); if (TESTPRINT) { printf("[Stmt->IF LPAREN Exp RPAREN Stmt] \n"); } }
                | WHILE LPAREN Exp RPAREN Stmt                  { $$ = new Stmt(new WhileStmt($3,$5)); if (TESTPRINT) { printf("[Stmt->WHILE LPAREN Exp RPAREN Stmt] \n"); } }
                | BREAK SEMICOLON                               { $$ = new Stmt(new BreakStmt()); if (TESTPRINT) { printf("[Stmt->BREAK SEMICOLON] \n"); } }
                | CONTINUE SEMICOLON                            { $$ = new Stmt(new ContinueStmt()); if (TESTPRINT) { printf("[Stmt->CONTINUE SEMICOLON] \n"); } }
                | RETURN SEMICOLON                              { $$ = new Stmt(new ReturnStmt(nullptr));  if (TESTPRINT) { printf("[Stmt->RETURN SEMICOLON] \n");} }
                | RETURN Exp SEMICOLON                          { $$ = new Stmt(new ReturnStmt($2)); if (TESTPRINT) { printf("[Stmt->RETURN Exp SEMICOLON] \n");} }
                ;

Exp         : LOrExp                                { $$ = new Exp($1); if (TESTPRINT) { printf("[Exp->LOrExp] \n"); } }
            ;

LVal        : IDENT                                 { $$ = new Lval($1,nullptr); if (TESTPRINT) { printf("[LVal->IDENT] %s\n\n",$1); }}
            | IDENT LValExpList                     { $$ = new Lval($1,$2); if (TESTPRINT) { printf("[LVal->IDENT] %s\n\n",$1); }}
            ;

LValExpList : LValExpList LBRACKET Exp RBRACKET     { $$ = $1; $$->as<LValExpList*>()->children.push_back($3); if (TESTPRINT) { printf("[LValExpList->LValExpList LBRACKET Exp RBRACKET] \n"); }  }
            | LBRACKET Exp RBRACKET                 { $$ = new LValExpList(); $$->as<LValExpList*>()->children.push_back($2); if (TESTPRINT) { printf("[LValExpList->LBRACKET Exp RBRACKET] \n"); } }
            ;

PrimaryExp  : LPAREN Exp RPAREN                     { $$ = new PrimaryExp($2); if (TESTPRINT) { printf("[PrimaryExp->LPAREN Exp RPAREN] \n"); }}
            | LVal                                  { $$ = new PrimaryExp($1); if (TESTPRINT) { printf("[PrimaryExp->LVal] \n"); }}
            | Number                                { $$ = new PrimaryExp($1); if (TESTPRINT) { printf("[PrimaryExp->Number] \n"); }}
            ;

Number      : INTCONST                              { $$ = new IntegerLiteral($1); if (TESTPRINT) { printf("[Number->INTCONST] \n"); } }
            ;

UnaryExp    : PrimaryExp                            { $$ = new UnaryExp($1);  if (TESTPRINT) { printf("[UnaryExp->PrimaryExp] \n"); }}
            | IDENT LPAREN RPAREN                   { $$ = new UnaryExp($1,nullptr); if (TESTPRINT) { printf("[UnaryExp->IDENT LPAREN RPAREN] \n"); } }
            | IDENT LPAREN FuncRParams RPAREN       { $$ = new UnaryExp($1,$3); if (TESTPRINT) { printf("[UnaryExp->IDENT LPAREN FuncRParams RPAREN] \n"); }}
            | ADD UnaryExp                          { $$ = new UnaryExp(OpType::OP_Pos, $2); if (TESTPRINT) { printf("[UnaryExp->ADD UnaryExp] \n"); }}
            | SUB UnaryExp                          { $$ = new UnaryExp(OpType::OP_Neg, $2); if (TESTPRINT) { printf("[UnaryExp->SUB UnaryExp] \n"); }}
            | NOT UnaryExp                          { $$ = new UnaryExp(OpType::OP_Lnot, $2); if (TESTPRINT) { printf("[UnaryExp->NOT UnaryExp] \n"); }}
            ;

FuncRParams : FuncRParams COMMA Exp                  { $$ = $1; $$->as<FuncRParams*>()->children.push_back($3); if (TESTPRINT) { printf("[FuncRParams->FuncRParams COMMA Exp] \n"); }}
            | Exp                                    { $$ = new FuncRParams(); $$->as<FuncRParams*>()->children.push_back($1); if (TESTPRINT) { printf("[FuncRParams->Exp] \n"); }}
            ;

MulExp      : UnaryExp                              { $$ = $1; if (TESTPRINT) { printf("[MulExp->UnaryExp] \n"); } }
            | MulExp MUL UnaryExp                   { $$ = new BinaryExp(OpType::OP_Mul, $1, $3);  if (TESTPRINT) { printf("[MulExp->MulExp MUL UnaryExp] \n"); } }
            | MulExp DIV UnaryExp                   { $$ = new BinaryExp(OpType::OP_Div, $1, $3);  if (TESTPRINT) { printf("[MulExp->MulExp DIV UnaryExp] \n"); } }
            | MulExp MOD UnaryExp                   { $$ = new BinaryExp(OpType::OP_Mod, $1, $3);  if (TESTPRINT) { printf("[MulExp->MulExp MOD UnaryExp] \n"); } }
            ;

AddExp      : MulExp                                { $$ = $1;   if (TESTPRINT) { printf("[AddExp->MulExp] \n"); }  }
            | AddExp ADD MulExp                     { $$ = new BinaryExp(OpType::OP_Add, $1, $3);  if (TESTPRINT) { printf("[AddExp->AddExp ADD MulExp] \n"); } }
            | AddExp SUB MulExp                     { $$ = new BinaryExp(OpType::OP_Sub, $1, $3);  if (TESTPRINT) { printf("[AddExp->AddExp SUB MulExp] \n"); } }
            ;

RelExp      : AddExp                                { $$ = $1;  if (TESTPRINT) { printf("[RelExp->AddExp] \n"); } }
            | RelExp GT AddExp                      { $$ = new BinaryExp(OpType::OP_Gt, $1, $3); if (TESTPRINT) { printf("[RelExp->RelExp GT AddExp] \n"); } }
            | RelExp GE AddExp                      { $$ = new BinaryExp(OpType::OP_Ge, $1, $3); if (TESTPRINT) { printf("[RelExp->RelExp GE AddExp] \n"); } }
            | RelExp LT AddExp                      { $$ = new BinaryExp(OpType::OP_Lt, $1, $3); if (TESTPRINT) { printf("[RelExp->RelExp LT AddExp] \n"); } }
            | RelExp LE AddExp                      { $$ = new BinaryExp(OpType::OP_Le, $1, $3); if (TESTPRINT) { printf("[RelExp->RelExp LE AddExp] \n"); } }
            ;

EqExp       : RelExp                                { $$ = $1; if (TESTPRINT) { printf("[EqExp->RelExp] \n"); } }
            | EqExp EQ RelExp                       { $$ = new BinaryExp(OpType::OP_Eq, $1, $3); if (TESTPRINT) { printf("[EqExp->EqExp EQ RelExp] \n"); } }
            | EqExp NEQ RelExp                      { $$ = new BinaryExp(OpType::OP_Ne, $1, $3); if (TESTPRINT) { printf("[EqExp->EqExp NEQ RelExp] \n"); } }
            ;

LAndExp     : EqExp                                 { $$ = $1; if (TESTPRINT) { printf("[LAndExp->EqExp] \n"); } }
            | LAndExp AND EqExp                     { $$ = new BinaryExp(OpType::OP_Land, $1, $3); if (TESTPRINT) { printf("[LAndExp->LAndExp AND EqExp] \n"); } }
            ;

LOrExp      : LAndExp                               { $$ = $1; if (TESTPRINT) { printf("[LOrExp->LAndExp] \n"); }}
            | LOrExp OR LAndExp                     { $$ = new BinaryExp(OpType::OP_Lor, $1, $3); if (TESTPRINT) { printf("[LOrExp->LAndExp OR LAndExp] \n"); }}
            ;

%%

void yyerror(const char *s) {
    printf("error: %s\n", s);
}
