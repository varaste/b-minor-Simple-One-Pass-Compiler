#include <stdio.h>
#include <stdlib.h>

#include "expr.h"
#include "decl.h"
#include "stmt.h"
#include "type.h"
#include "typecheck.h"
#include "param_list.h"
#include "scope.h"
#include "x64_codegen.h"

#include "hash_table.h"

#include "parser.h"

#define DEBUG 0

extern int yyparse();
extern Decl* parser_result;

extern int yylex();
extern FILE* yyin;

int scope_error = 0;
int type_error = 0;

struct hash_table* scope_stack[SCOPE_STACK_MAX];

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Require one argument: filename.\n");
        return EXIT_FAILURE;
    }

    char* filename = argv[1];
    yyin = fopen(filename, "r");
    if (!yyin) {
        printf("Could not open file '%s'.", filename);
        return EXIT_FAILURE;
    }

#if 0
    // print out token values
    printf("token values are:\n");
    enum yytokentype t;
    while ((t = yylex())) {
        printf("%d\n", t);
    }

    return EXIT_SUCCESS;
#endif

    // parsing
    if (yyparse() != 0) {
        printf("Parse failed!\n");
        return EXIT_FAILURE;
    }

    // re-outputting
    printf("Parse successful!\n");
    printf("Result is:\n");
    if (parser_result) {
        decl_print(parser_result);
    } else {
        printf("null\n");
    }

    // resolving symbols
    scope_stack[0] = hash_table_create(0, 0);
    decl_resolve(parser_result);
    if (scope_error != 0) {
        printf("Error(s) encountered when resolving symbols. Exiting...\n");
        exit(1);
    }
    hash_table_delete(scope_stack[0]);

    // typechecking
    decl_typecheck(parser_result);
    if (type_error != 0) {
        printf("Error(s) encountered when typechecking. Exiting...\n");
        exit(1);
    }

    // codegen
    codegen(parser_result, "output.s");

    decl_delete(parser_result);
    fclose(yyin);
    return EXIT_SUCCESS;
}
