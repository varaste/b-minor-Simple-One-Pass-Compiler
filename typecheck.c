#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "typecheck.h"
#include "param_list.h"
#include "type.h"
#include "expr.h"
#include "decl.h"
#include "stmt.h"

#include <stdarg.h>

extern int type_error;

void error_print(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    while (*fmt != '\0') {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 'T':
                    type_print(va_arg(args, Type*));
                    break;
                case 'E':
                    expr_print(va_arg(args, Expr*));
                    break;
                case 's':
                    printf("%s", va_arg(args, char*));
                    break;
                case '%':
                    putchar('%');
                    break;
                default:
                    printf("Error in 'error_print': reached unexpected character %c after %%.\n", *fmt);
                    exit(1);
                    break;
            }
        } else {
            putchar(*fmt);
        }

        fmt++;
    }

    va_end(args);
}

int type_equals(Type* a, Type* b) {
    if (!a || !b || a->kind != b->kind) {
        return 0;
    }

    switch (a->kind) {
        case TYPE_VOID:
        case TYPE_BOOLEAN:
        case TYPE_CHAR:
        case TYPE_INTEGER:
        case TYPE_STRING:
            return 1;
        case TYPE_ARRAY:
            return type_equals(a->subtype, b->subtype);
        case TYPE_FUNCTION:
            return type_equals(a->subtype, b->subtype)
                    && param_list_equals(a->params, b->params);
        default:
            printf("Compiler bug: enum case not handled.\n");
            assert(0);
    }
}

Type* type_copy(Type* t) {
    if (!t) return NULL;

    Type* new_t = type_create(t->kind);
    new_t->subtype = type_copy(t->subtype);

    return new_t;
}

int is_compile_time_constant(Expr* e) {
    int result = 1;
    switch (e->kind) {
        case EXPR_BOOLEAN_LITERAL:
        case EXPR_INTEGER_LITERAL:
        case EXPR_CHAR_LITERAL:
        case EXPR_STRING_LITERAL:
            break;
        case EXPR_INIT_LIST: {
            Expr* current = e->right;
            while (current != NULL) {
                if (!is_compile_time_constant(current->left)) {
                    result = 0;
                    break;
                }
                current = current->right;
            }
        } break;
        default:
            result = 0;
            break;
    }

    return result;
}

void decl_typecheck(Decl* d) {
    if (!d) return;

    // type check arrays
    if (d->type->kind == TYPE_ARRAY) {
        if (!d->value && !d->type->size_expr) {
            error_print(
                "Type Error: cannot infer size for array '%s'\
                \n\tA size expression or initial value must be included.\n\n",
                d->name
            );
            type_error = 1;
        }

        if (d->type->size_expr) {
            Type* t = expr_typecheck(d->type->size_expr);
            if (t->kind != TYPE_INTEGER) {
                error_print(
                    "Type Error: array size expression must be an integer.\
                    \n\tGot (%E), which is of type (%T)\n\n",
                    d->type->size_expr, t
                );
            }
        }
    }

    if (d->value) {
        Type* t = expr_typecheck(d->value);
        if (!type_equals(t, d->symbol->type)) {
            error_print(
                "Type error: cannot assign to a variable of a different type.\n\t\
                 Got declaration (%s: %T = %E), which is of type (%T) = (%T).\n",
                d->name, d->symbol->type, d->value, d->symbol->type, t
            );
            type_error = 1;
        }

        if (d->type->kind == TYPE_ARRAY && d->type->size_expr
            && !is_compile_time_constant(d->type->size_expr)
        ) {
            error_print(
                "Type error: array size expression must be a compile-time constant.\
                \n\tFound: (%E)\n",
                d->type->size_expr
            );
        }

        // only allow compile-time constants as initializer values for globals
        if (d->symbol->kind == SYMBOL_GLOBAL && !is_compile_time_constant(d->value)) {
            error_print(
                "Type error: global initializer values must be compile-time constants.\
                \n\tFound: (%E)\n\n",
                d->value
            );
            type_error = 1;
        }
    }

    if (d->code) {
        stmt_typecheck(d->code);
    }

    decl_typecheck(d->next);
}

void stmt_typecheck(Stmt* s) {
    if (!s) return;

    Type* t;
    switch (s->kind) {
        case STMT_DECL:
            decl_typecheck(s->decl);
            break;
        case STMT_EXPR:
            t = expr_typecheck(s->expr);
            //type_delete(t);
            break;
        case STMT_IF_ELSE:
            t = expr_typecheck(s->expr);
            if (t->kind != TYPE_BOOLEAN) {
                error_print("Type error: if statement condition must be a boolean.\n\tGot expression (%E), which is of type (%T).\n", s->expr, t);
                type_error = 1;
            }

            //type_delete(t);
            stmt_typecheck(s->body);
            stmt_typecheck(s->else_body);
            break;
        case STMT_FOR:
            t = expr_typecheck(s->expr);

            if (t == NULL || t->kind != TYPE_BOOLEAN) {
                error_print("Type error: for loop condition must be a boolean.\n\tGot expression (%E), which is of type (%T)\n", s->expr, t);
                type_error = 1;
            }

            //type_delete(t);
            stmt_typecheck(s->body);
            break;
        case STMT_PRINT:
            expr_typecheck(s->expr);
            break;
        case STMT_RETURN:
            expr_typecheck(s->expr);
            break;
        case STMT_BLOCK:
            stmt_typecheck(s->body);
            break;
    }

    stmt_typecheck(s->next);
}

Type* expr_typecheck(Expr* e) {
    if (!e) return 0;

    Type* lt = expr_typecheck(e->left);
    Type* rt = expr_typecheck(e->right);

    Type* result = NULL;

    switch (e->kind) {
        case EXPR_ADD:
        case EXPR_SUB:
        case EXPR_MUL:
        case EXPR_DIV:
        case EXPR_EXPONENT:
        case EXPR_MODULO:
            if (lt->kind != TYPE_INTEGER || rt->kind != TYPE_INTEGER) {
                error_print("Type error: arithmetic operations require integers. \n\tGot the expression (%E), which is of type (%T) + (%T)\n", e, lt, rt);
                type_error = 1;
            }
            result = type_create(TYPE_INTEGER);
            break;
        case EXPR_NAME:
            // e->symbol is NULL if e is being used before it is declared
            if (e->symbol == NULL) {
                // return arbitrary type so we can continue with checking for other type errors
                result = type_create(TYPE_INTEGER);
            } else {
                result = type_copy(e->symbol->type);
            }
            break;
        case EXPR_NEGATE:
            if (lt->kind != TYPE_INTEGER) {
                error_print("Type error: negate operator requries an integer.\n\tGot expression (%E), which is of type (%T).\n", lt);
                type_error = 1;
            }
            result = type_create(TYPE_INTEGER);
            break;
        case EXPR_LOGICAL_OR:
        case EXPR_LOGICAL_AND:
            if (lt->kind != TYPE_BOOLEAN || rt->kind != TYPE_BOOLEAN) {
                error_print("Type error: logical operators require boolean arguments.\n\tGot the expression (%E), which is of type (%T) %s (%T)\n", e, lt, e->kind == EXPR_LOGICAL_OR ? "||" : "&&", rt);
                type_error = 1;
            }
            result = type_create(TYPE_BOOLEAN);
            break;
        case EXPR_LOGICAL_NOT:
            if (lt->kind != TYPE_BOOLEAN) {
                error_print("Type error: logical negation requires a boolean.\n\tGot the expression (%E) which is of type (%T)\n", e, lt);
                type_error = 1;
            }
            result = type_create(TYPE_BOOLEAN);
            break;
        case EXPR_CMP_EQUAL:
        case EXPR_CMP_NOT_EQUAL:
            if (!type_equals(lt, rt)) {
                error_print("Type error: comparison operators may only be used on two values of the same type.\n\tGot the expression (%E), which is of the type (%T) %s (%T).\n", e, lt, e->kind == EXPR_CMP_EQUAL ? "==" : "!=", rt);
                type_error = 1;
            }

            if (lt->kind == TYPE_VOID ||
                lt->kind == TYPE_ARRAY ||
                lt->kind == TYPE_FUNCTION) {
                error_print("Type error: cannot compare values of non-atomic types. \n\tGot the expression (%E), which is of type (%T)\n", e, lt);
                type_error = 1;
            }
            result = type_create(TYPE_BOOLEAN);
            break;
        case EXPR_CMP_GT:
        case EXPR_CMP_GT_EQUAL:
        case EXPR_CMP_LT:
        case EXPR_CMP_LT_EQUAL:
            if (lt->kind != TYPE_INTEGER || rt->kind != TYPE_INTEGER) {
                error_print("Type error: cannot use relative comparison operators on non-integer types.\n\tGot expression (%E), with operands of type (%T), (%T)\n", e, lt, rt);
                type_error = 1;
            }
            result = type_create(TYPE_BOOLEAN);
            break;
        case EXPR_CHAR_LITERAL:
            result = type_create(TYPE_CHAR);
            break;
        case EXPR_STRING_LITERAL:
            result = type_create(TYPE_STRING);
            break;
        case EXPR_INTEGER_LITERAL:
            result = type_create(TYPE_INTEGER);
            break;
        case EXPR_BOOLEAN_LITERAL:
            result = type_create(TYPE_BOOLEAN);
            break;
        case EXPR_CALL:
            // subtype is the return type of the function being called
            result = type_copy(lt->subtype);
            break;
        case EXPR_INIT_LIST:
            result = type_create_array(type_copy(rt), 0);
            break;
        case EXPR_ARG:
            result = type_copy(lt);
            break;
        case EXPR_SUBSCRIPT:
            if (lt->kind == TYPE_ARRAY) {
                if (rt->kind != TYPE_INTEGER) {
                    error_print("Type error: array subscript must be an integer.\n\tGot expression (%E), which is of type (%T).\n", e->right, rt);
                    type_error = 1;
                }
                result = type_copy(lt->subtype);
            } else {
                error_print("Type error: subscript target is not an array.\n\tGot expression (%E), which is of type (%T)\n", e->left, lt);
                type_error = 1;
                result = type_copy(lt);
            }
            break;
        case EXPR_ASSIGN:
            if (lt->kind != rt->kind) {
                error_print("Type error: cannot assign to a variable of a different type.\n\tGot expression (%E), which is of type (%T) = (%T).\n", e, lt, rt);
                type_error = 1;
            }
            result = type_copy(lt);
            break;
        case EXPR_INCREMENT:
            if (lt->kind != TYPE_INTEGER) {
                error_print("Type error: cannot use increment operator on non-integer.\n\tGot expression (%E), which is of type (%T).\n", e, lt);
                type_error = 1;
            }
            result = type_create(TYPE_INTEGER);
            break;
        case EXPR_DECREMENT:
            if (lt->kind != TYPE_INTEGER) {
                error_print("Type error: cannot use decrement operator on non-integer.\n\tGot expression (%E), which is of type (%T).\n", e, lt);
                type_error = 1;
            }
            result = type_create(TYPE_INTEGER);
            break;
        default:
            printf("Compiler bug: enum case not handled.\n");
            assert(0);
    }
    e->type = result;

    //type_delete(lt);
    //type_delete(rt);

    assert(result != NULL);
    return result;
}
