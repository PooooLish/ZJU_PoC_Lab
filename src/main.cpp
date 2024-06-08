#include "semanticAnalysis/semanticAnalysis.h"
//#include "ast/ast.h"
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
        print_expr(root,"");
        fmt::print("\nSuccess\n");
    }
    int result = semanticAnalysis(root);
    return result;
}