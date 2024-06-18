#include "semanticAnalysis/semanticAnalysis.h"
#include "translate/translate.h"
#include <fmt/core.h>

extern int yyparse();

extern FILE* yyin;

extern NodePtr root;

int main(int argc, char **argv) {
    yyin = fopen(argv[1], "r");
    int parse_result = yyparse();
    fclose(yyin);

    if (parse_result != 0) {
        return 1;
    }

    if (root) {
        fmt::print("Lexer and Parser success\n");
    } else{
        fmt::print("Lexer and Parser fail\n");
        return 1;
    }

    int semantic_result = semanticAnalysis(root);
    if (semantic_result == 0) {
        fmt::print("Semantic analysis success\n");
    } else{
        fmt::print("Semantic analysis fail\n");
        return 1;
    }

    // Create a translate instance and traverse the AST
    translate translator(root);

    //补充
    return 0;
}