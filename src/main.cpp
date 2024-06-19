#include "semanticAnalysis/semanticAnalysis.h"
#include "translate/translate.h"
#include <fmt/core.h>
#include <iostream>

extern int yyparse();
extern FILE* yyin;
extern NodePtr root;

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_file>" << std::endl;
        return 1;
    }

    yyin = fopen(argv[1], "r");
    if (!yyin) {
        std::cerr << "Error opening input file: " << argv[1] << std::endl;
        return 1;
    }

    int parse_result = yyparse();
    fclose(yyin);

    if (parse_result != 0) {
        return 1;
    }

    if (root) {
        fmt::print("Lexer and Parser success\n");
    } else {
        fmt::print("Lexer and Parser fail\n");
        return 1;
    }

    int semantic_result = semanticAnalysis(root);
    if (semantic_result == 0) {
        fmt::print("Semantic analysis success\n");
    } else {
        fmt::print("Semantic analysis fail\n");
        return 1;
    }

    // Create a translate instance and traverse the AST
    translate translator(root, argv[2]);

    return 0;
}
