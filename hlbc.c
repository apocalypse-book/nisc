#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "include/nunsc.h"

void nun_new_hlbuilder(NunHlbuilder *dest, NunGc *gc) {
    dest->gc = gc;
    dest->regcnt = 0;
    dest->funref = -1;
    dest->insref = -1;
    dest->funs = 16;
    dest->func = 0;
    dest->funv = malloc(dest->funs * sizeof(NunHlfun));
    dest->funent = -1;
}

void nun_build_hlbuilder(NunHlprog *dest, NunHlbuilder *b) {
    dest->funref = -1;
    dest->insref = -1;
    dest->func = b->func;
    dest->funv = realloc(b->funv, b->func * sizeof(NunHlfun));
    dest->funent = b->funent;
}

void nun_hlb_entry(NunHlbuilder *b, int32_t funref) {
    b->funent = funref;
}

void nun_hlb_del_ins(NunHlbc *ins) {
    free(ins->argv);
}

int32_t nun_hlb_addfun(NunHlbuilder *b, const char *name) {
    b->funv[b->func].present = 1;
    b->funv[b->func].name = name;
    b->funv[b->func].inss = 16;
    b->funv[b->func].insc = 0;
    b->funv[b->func].insv = malloc(b->funv[b->func].inss * sizeof(NunHlbc));
    int32_t funref = b->func;
    ++b->func;
    return funref;
}

void nun_hlb_rmfun(NunHlbuilder *b, int32_t funref) {
    if ((size_t) funref > b->func) {
        fprintf(stderr,
                "nunsc:%s:%d: error: "
                "index out of bounds, "
                "got %d but length is %zu\n",
                __FILE__,
                __LINE__,
                (int) funref,
                b->func);
        exit(1);
    }

    NunHlfun *fun = b->funv + funref;
    if (!fun->present) {
        return;
    }

    for (size_t i = 0; i < fun->insc; i++) {
        nun_hlb_del_ins(fun->insv + i);
    }

    free(fun->insv);
}

static NunHlbc *nun_hlb_prepare_build(NunHlbuilder *b) {
    NunHlfun *fun = b->funv + b->funref;
    if ((size_t)  b->insref < fun->insc) {
        if (fun->insc == fun->inss) {
            fun->inss *= 2;
            fun->insv = realloc(fun->insv, fun->inss * sizeof(NunHlbc));
        }
        NunHlbc *ins = fun->insv + b->insref;
        size_t size = fun->insc - b->insref;
        memmove(ins + 1, ins, size);
    } else if ((size_t)  b->insref > fun->insc) {
        fprintf(stderr,
                "nunsc:%s:%d: error: "
                "index out of bounds, "
                "got %d but length is %zu\n",
                __FILE__,
                __LINE__,
                (int) b->insref,
                fun->insc);
        exit(1);
    }

    NunHlbc *result = fun->insv + b->insref;
    ++b->insref;
    ++fun->insc;
    return result;
}

static void nun_hlb_finish_build(NunHlarg *dest, NunHlbuilder *b) {
    dest->kind = NUN_HLBC_ARG_REGISTER;
    dest->ssreg = b->regcnt;
    ++b->regcnt;
}

static void nun_hlb_finish_build_void(NunHlbuilder *b) {
    (void) b;
}

void nun_hlb_build_call(NunHlarg *dest, NunHlbuilder *b, NunHlarg *argv, size_t argc) {
    NunHlbc *ins = nun_hlb_prepare_build(b);
    ins->opcode = NUN_HLBC_CALL;
    ins->flags = 0;
    ins->target = b->regcnt;
    ins->argc = argc;
    ins->argv = argv;
    nun_hlb_finish_build(dest, b);
}

void nun_hlb_build_add(NunHlarg *dest, NunHlbuilder *b, NunHlarg *lhs, NunHlarg *rhs) {
    NunHlbc *ins = nun_hlb_prepare_build(b);
    ins->opcode = NUN_HLBC_ADD;
    ins->flags = 0;
    ins->target = b->regcnt;
    ins->argc = 2;
    ins->argv = malloc(2 * sizeof(NunHlarg));
    ins->argv[0] = *lhs;
    ins->argv[1] = *rhs;
    nun_hlb_finish_build(dest, b);
}

void nun_hlb_build_sub(NunHlarg *dest, NunHlbuilder *b, NunHlarg *lhs, NunHlarg *rhs) {
    NunHlbc *ins = nun_hlb_prepare_build(b);
    ins->opcode = NUN_HLBC_SUB;
    ins->flags = 0;
    ins->target = b->regcnt;
    ins->argc = 2;
    ins->argv = malloc(2 * sizeof(NunHlarg));
    ins->argv[0] = *lhs;
    ins->argv[1] = *rhs;
    nun_hlb_finish_build(dest, b);
}

void nun_hlb_build_imul(NunHlarg *dest, NunHlbuilder *b, NunHlarg *lhs, NunHlarg *rhs) {
    NunHlbc *ins = nun_hlb_prepare_build(b);
    ins->opcode = NUN_HLBC_IMUL;
    ins->flags = 0;
    ins->target = b->regcnt;
    ins->argc = 2;
    ins->argv = malloc(2 * sizeof(NunHlarg));
    ins->argv[0] = *lhs;
    ins->argv[1] = *rhs;
    nun_hlb_finish_build(dest, b);
}

void nun_hlb_build_idiv(NunHlarg *dest, NunHlbuilder *b, NunHlarg *lhs, NunHlarg *rhs) {
    NunHlbc *ins = nun_hlb_prepare_build(b);
    ins->opcode = NUN_HLBC_IDIV;
    ins->flags = 0;
    ins->target = b->regcnt;
    ins->argc = 2;
    ins->argv = malloc(2 * sizeof(NunHlarg));
    ins->argv[0] = *lhs;
    ins->argv[1] = *rhs;
    nun_hlb_finish_build(dest, b);
}

void nun_hlb_build_irem(NunHlarg *dest, NunHlbuilder *b, NunHlarg *lhs, NunHlarg *rhs) {
    NunHlbc *ins = nun_hlb_prepare_build(b);
    ins->opcode = NUN_HLBC_IREM;
    ins->flags = 0;
    ins->target = b->regcnt;
    ins->argc = 2;
    ins->argv = malloc(2 * sizeof(NunHlarg));
    ins->argv[0] = *lhs;
    ins->argv[1] = *rhs;
    nun_hlb_finish_build(dest, b);
}

void nun_hlb_build_return(NunHlbuilder *b, NunHlarg *retval) {
    NunHlbc *ins = nun_hlb_prepare_build(b);
    ins->opcode = NUN_HLBC_RETURN;
    ins->flags = 0;
    ins->target = -1;
    ins->argc = 1;
    ins->argv = malloc(1 * sizeof(NunHlarg));
    ins->argv[0] = *retval;
    nun_hlb_finish_build_void(b);
}

#define PRELUDE_OP(ident, name, func)                   \
    void ident(NunHlbuilder *b) {                       \
        int32_t funref = nun_hlb_addfun(b, name);       \
        b->funref = funref;                             \
        b->insref = 0;                                  \
                                                        \
        NunHlarg lhs;                                   \
        lhs.kind = NUN_HLBC_ARG_PROPER;                 \
        lhs.ssarg = 0;                                  \
        NunHlarg rhs;                                   \
        rhs.kind = NUN_HLBC_ARG_PROPER;                 \
        rhs.ssarg = 1;                                  \
                                                        \
        NunHlarg retval;                                \
        func(&retval, b, &lhs, &rhs);                   \
                                                        \
        nun_hlb_build_return(b, &retval);               \
                                                        \
        b->funref = -1;                                 \
        b->insref = -1;                                 \
    }                                                   \

PRELUDE_OP(nun_hlb_prelude_add, "+", nun_hlb_build_add)
PRELUDE_OP(nun_hlb_prelude_sub, "-", nun_hlb_build_sub)
PRELUDE_OP(nun_hlb_prelude_imul, "*", nun_hlb_build_imul)
PRELUDE_OP(nun_hlb_prelude_idiv, "/", nun_hlb_build_idiv)
PRELUDE_OP(nun_hlb_prelude_irem, "%", nun_hlb_build_irem)

void nun_hlb_make_prelude(NunHlbuilder *b) {
    nun_hlb_prelude_add(b);
    nun_hlb_prelude_sub(b);
    nun_hlb_prelude_imul(b);
    nun_hlb_prelude_idiv(b);
    nun_hlb_prelude_irem(b);
}

static int nun_expr_to_hlbc(NunHlarg *dest, NunHlbuilder *b, NunValue *expr) {
    if (expr->kind == NUN_VALUE_TREE) {
        NunValue *origval = expr;
        NunStree *tmp = expr->vtree;
        NunStree *expr = tmp;
        switch (expr->kind) {
        case NUN_STREE_PAIR: {
            if (nun_list_eh(expr)) {
                NunStree *car = expr->vpair.car;
                NunStree *cdr = expr->vpair.cdr;
                switch (car->kind) {
                case NUN_STREE_ATOM: {
                    NunStree *args = cdr;
                    size_t capacity = 4;
                    NunHlarg *argv = malloc(capacity * sizeof(NunHlarg));
                    size_t argc = 0;
                    const char *funname;
                    if (car->flags & NUN_FLAG_INLINE) {
                        funname = car->vinline;
                    } else {
                        funname = car->vatom;
                    }
                    int32_t funref = -1;
                    for (size_t i = 0; i < b->func; i++) {
                        if (b->funv[i].present && strcmp(b->funv[i].name, funname) == 0) {
                            funref = i;
                            break;
                        }
                    }
                    if (funref < 0) {
                        fprintf(stderr,
                                "nunsc:%s:%d: error: undefined function: %s\n",
                                __FILE__,
                                __LINE__,
                                funname);
                        return 1;
                    }
                    argv[argc].kind = NUN_HLBC_ARG_VALUE;
                    argv[argc].value.kind = NUN_VALUE_INT;
                    argv[argc].value.flags = 0;
                    argv[argc].value.vint = funref;
                    ++argc;
                    for (NunStree *car = args->vpair.car;
                         args->kind != NUN_STREE_NIL;
                         args = args->vpair.cdr, car = args->vpair.car) {
                        if (argc == capacity) {
                            capacity *= 2;
                            argv = realloc(argv, capacity * sizeof(NunHlarg));
                        }
                        NunValue val;
                        nun_stree(&val, b->gc, car);
                        nun_expr_to_hlbc(argv + argc, b, &val);
                        ++argc;
                    }
                    nun_hlb_build_call(dest, b, argv, argc);
                    return 0;
                } break;
                default:
                    // TODO: all the special forms
                    return 0;
                }
            } else {
                // TODO: pair
                return 0;
            }
        }
        case NUN_STREE_VECTOR: {
            // TODO
            return 0;
        }
        default:
            dest->kind = NUN_HLBC_ARG_VALUE;
            dest->value = *origval;
            return 0;
        }
    } else {
        dest->kind = NUN_HLBC_ARG_VALUE;
        dest->value = *expr;
        return 0;
    }
}

int nun_to_hlbc(NunHlprog *dest, NunHlbuilder *b, NunValue *program, size_t proglen) {
    int status = 0;
    
    int32_t funref = nun_hlb_addfun(b, "main");
    b->funref = funref;
    b->insref = 0;
    nun_hlb_entry(b, funref);

    NunHlarg result;
    for (size_t i = 0; i < proglen; i++) {
        NunValue *expr = program + i;
        status |= nun_expr_to_hlbc(&result, b, expr);
    }
    if (!status) {
        nun_hlb_build_return(b, &result);
        nun_build_hlbuilder(dest, b);
    }
    return status;
}
