#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "include/nisc.h"

void nis_new_hlbuilder(NisHlbuilder *dest, NisGc *gc) {
    dest->gc = gc;
    dest->regcnt = 0;
    dest->funref = -1;
    dest->insref = -1;
    dest->funs = 16;
    dest->func = 0;
    dest->funv = malloc(dest->funs * sizeof(NisHlfun));
    dest->funent = -1;
}

void nis_build_hlbuilder(NisHlprog *dest, NisHlbuilder *b) {
    dest->funref = -1;
    dest->insref = -1;
    dest->func = b->func;
    dest->funv = realloc(b->funv, b->func * sizeof(NisHlfun));
    dest->funent = b->funent;
}

void nis_hlb_entry(NisHlbuilder *b, int32_t funref) {
    b->funent = funref;
}

void nis_hlb_del_ins(NisHlbc *ins) {
    free(ins->argv);
}

int32_t nis_hlb_addfun(NisHlbuilder *b, const char *name) {
    b->funv[b->func].present = 1;
    b->funv[b->func].name = name;
    b->funv[b->func].inss = 16;
    b->funv[b->func].insc = 0;
    b->funv[b->func].insv = malloc(b->funv[b->func].inss * sizeof(NisHlbc));
    int32_t funref = b->func;
    ++b->func;
    return funref;
}

void nis_hlb_rmfun(NisHlbuilder *b, int32_t funref) {
    if ((size_t) funref > b->func) {
        fprintf(stderr,
                "nisc:%s:%d: error: "
                "index out of bounds, "
                "got %d but length is %zu\n",
                __FILE__,
                __LINE__,
                (int) funref,
                b->func);
        exit(1);
    }

    NisHlfun *fun = b->funv + funref;
    if (!fun->present) {
        return;
    }

    for (size_t i = 0; i < fun->insc; i++) {
        nis_hlb_del_ins(fun->insv + i);
    }

    free(fun->insv);
}

static NisHlbc *nis_hlb_prepare_build(NisHlbuilder *b) {
    NisHlfun *fun = b->funv + b->funref;
    if ((size_t)  b->insref < fun->insc) {
        if (fun->insc == fun->inss) {
            fun->inss *= 2;
            fun->insv = realloc(fun->insv, fun->inss * sizeof(NisHlbc));
        }
        NisHlbc *ins = fun->insv + b->insref;
        size_t size = fun->insc - b->insref;
        memmove(ins + 1, ins, size);
    } else if ((size_t)  b->insref > fun->insc) {
        fprintf(stderr,
                "nisc:%s:%d: error: "
                "index out of bounds, "
                "got %d but length is %zu\n",
                __FILE__,
                __LINE__,
                (int) b->insref,
                fun->insc);
        exit(1);
    }

    NisHlbc *result = fun->insv + b->insref;
    ++b->insref;
    ++fun->insc;
    return result;
}

static void nis_hlb_finish_build(NisHlarg *dest, NisHlbuilder *b) {
    dest->kind = NIS_HLBC_ARG_REGISTER;
    dest->ssreg = b->regcnt;
    ++b->regcnt;
}

static void nis_hlb_finish_build_void(NisHlbuilder *b) {
    (void) b;
}

void nis_hlb_build_call(NisHlarg *dest, NisHlbuilder *b, NisHlarg *argv, size_t argc) {
    NisHlbc *ins = nis_hlb_prepare_build(b);
    ins->opcode = NIS_HLBC_CALL;
    ins->flags = 0;
    ins->target = b->regcnt;
    ins->argc = argc;
    ins->argv = argv;
    nis_hlb_finish_build(dest, b);
}

void nis_hlb_build_add(NisHlarg *dest, NisHlbuilder *b, NisHlarg *lhs, NisHlarg *rhs) {
    NisHlbc *ins = nis_hlb_prepare_build(b);
    ins->opcode = NIS_HLBC_ADD;
    ins->flags = 0;
    ins->target = b->regcnt;
    ins->argc = 2;
    ins->argv = malloc(2 * sizeof(NisHlarg));
    ins->argv[0] = *lhs;
    ins->argv[1] = *rhs;
    nis_hlb_finish_build(dest, b);
}

void nis_hlb_build_sub(NisHlarg *dest, NisHlbuilder *b, NisHlarg *lhs, NisHlarg *rhs) {
    NisHlbc *ins = nis_hlb_prepare_build(b);
    ins->opcode = NIS_HLBC_SUB;
    ins->flags = 0;
    ins->target = b->regcnt;
    ins->argc = 2;
    ins->argv = malloc(2 * sizeof(NisHlarg));
    ins->argv[0] = *lhs;
    ins->argv[1] = *rhs;
    nis_hlb_finish_build(dest, b);
}

void nis_hlb_build_imul(NisHlarg *dest, NisHlbuilder *b, NisHlarg *lhs, NisHlarg *rhs) {
    NisHlbc *ins = nis_hlb_prepare_build(b);
    ins->opcode = NIS_HLBC_IMUL;
    ins->flags = 0;
    ins->target = b->regcnt;
    ins->argc = 2;
    ins->argv = malloc(2 * sizeof(NisHlarg));
    ins->argv[0] = *lhs;
    ins->argv[1] = *rhs;
    nis_hlb_finish_build(dest, b);
}

void nis_hlb_build_idiv(NisHlarg *dest, NisHlbuilder *b, NisHlarg *lhs, NisHlarg *rhs) {
    NisHlbc *ins = nis_hlb_prepare_build(b);
    ins->opcode = NIS_HLBC_IDIV;
    ins->flags = 0;
    ins->target = b->regcnt;
    ins->argc = 2;
    ins->argv = malloc(2 * sizeof(NisHlarg));
    ins->argv[0] = *lhs;
    ins->argv[1] = *rhs;
    nis_hlb_finish_build(dest, b);
}

void nis_hlb_build_irem(NisHlarg *dest, NisHlbuilder *b, NisHlarg *lhs, NisHlarg *rhs) {
    NisHlbc *ins = nis_hlb_prepare_build(b);
    ins->opcode = NIS_HLBC_IREM;
    ins->flags = 0;
    ins->target = b->regcnt;
    ins->argc = 2;
    ins->argv = malloc(2 * sizeof(NisHlarg));
    ins->argv[0] = *lhs;
    ins->argv[1] = *rhs;
    nis_hlb_finish_build(dest, b);
}

void nis_hlb_build_return(NisHlbuilder *b, NisHlarg *retval) {
    NisHlbc *ins = nis_hlb_prepare_build(b);
    ins->opcode = NIS_HLBC_RETURN;
    ins->flags = 0;
    ins->target = -1;
    ins->argc = 1;
    ins->argv = malloc(1 * sizeof(NisHlarg));
    ins->argv[0] = *retval;
    nis_hlb_finish_build_void(b);
}

#define PRELUDE_OP(ident, name, func)                   \
    void ident(NisHlbuilder *b) {                       \
        int32_t funref = nis_hlb_addfun(b, name);       \
        b->funref = funref;                             \
        b->insref = 0;                                  \
                                                        \
        NisHlarg lhs;                                   \
        lhs.kind = NIS_HLBC_ARG_PROPER;                 \
        lhs.ssarg = 0;                                  \
        NisHlarg rhs;                                   \
        rhs.kind = NIS_HLBC_ARG_PROPER;                 \
        rhs.ssarg = 1;                                  \
                                                        \
        NisHlarg retval;                                \
        func(&retval, b, &lhs, &rhs);                   \
                                                        \
        nis_hlb_build_return(b, &retval);               \
                                                        \
        b->funref = -1;                                 \
        b->insref = -1;                                 \
    }                                                   \

PRELUDE_OP(nis_hlb_prelude_add, "+", nis_hlb_build_add)
PRELUDE_OP(nis_hlb_prelude_sub, "-", nis_hlb_build_sub)
PRELUDE_OP(nis_hlb_prelude_imul, "*", nis_hlb_build_imul)
PRELUDE_OP(nis_hlb_prelude_idiv, "/", nis_hlb_build_idiv)
PRELUDE_OP(nis_hlb_prelude_irem, "%", nis_hlb_build_irem)

void nis_hlb_make_prelude(NisHlbuilder *b) {
    nis_hlb_prelude_add(b);
    nis_hlb_prelude_sub(b);
    nis_hlb_prelude_imul(b);
    nis_hlb_prelude_idiv(b);
    nis_hlb_prelude_irem(b);
}

static int nis_expr_to_hlbc(NisHlarg *dest, NisHlbuilder *b, NisValue *expr) {
    if (expr->kind == NIS_VALUE_TREE) {
        NisValue *origval = expr;
        NisStree *tmp = expr->vtree;
        NisStree *expr = tmp;
        switch (expr->kind) {
        case NIS_STREE_PAIR: {
            if (nis_list_eh(expr)) {
                NisStree *car = expr->vpair.car;
                NisStree *cdr = expr->vpair.cdr;
                switch (car->kind) {
                case NIS_STREE_ATOM: {
                    NisStree *args = cdr;
                    size_t capacity = 4;
                    NisHlarg *argv = malloc(capacity * sizeof(NisHlarg));
                    size_t argc = 0;
                    const char *funname;
                    if (car->flags & NIS_FLAG_INLINE) {
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
                                "nisc:%s:%d: error: undefined function: %s\n",
                                __FILE__,
                                __LINE__,
                                funname);
                        return 1;
                    }
                    argv[argc].kind = NIS_HLBC_ARG_VALUE;
                    argv[argc].value.kind = NIS_VALUE_INT;
                    argv[argc].value.flags = 0;
                    argv[argc].value.vint = funref;
                    ++argc;
                    for (NisStree *car = args->vpair.car;
                         args->kind != NIS_STREE_NIL;
                         args = args->vpair.cdr, car = args->vpair.car) {
                        if (argc == capacity) {
                            capacity *= 2;
                            argv = realloc(argv, capacity * sizeof(NisHlarg));
                        }
                        NisValue val;
                        nis_stree(&val, b->gc, car);
                        nis_expr_to_hlbc(argv + argc, b, &val);
                        ++argc;
                    }
                    nis_hlb_build_call(dest, b, argv, argc);
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
        case NIS_STREE_VECTOR: {
            // TODO
            return 0;
        }
        default:
            dest->kind = NIS_HLBC_ARG_VALUE;
            dest->value = *origval;
            return 0;
        }
    } else {
        dest->kind = NIS_HLBC_ARG_VALUE;
        dest->value = *expr;
        return 0;
    }
}

int nis_to_hlbc(NisHlprog *dest, NisHlbuilder *b, NisValue *program, size_t proglen) {
    int status = 0;
    
    int32_t funref = nis_hlb_addfun(b, "main");
    b->funref = funref;
    b->insref = 0;
    nis_hlb_entry(b, funref);

    NisHlarg result;
    for (size_t i = 0; i < proglen; i++) {
        NisValue *expr = program + i;
        status |= nis_expr_to_hlbc(&result, b, expr);
    }
    if (!status) {
        nis_hlb_build_return(b, &result);
        nis_build_hlbuilder(dest, b);
    }
    return status;
}
