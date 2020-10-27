#include <stdlib.h>
#include <string.h>
#include <tgmath.h>
#include "include/nisc.h"

struct DisplayParams {
    size_t count;
    size_t indent;
    const char *indent_char;
    const char *newline_char;
};

static size_t nis_display_inner_tree(char *dest, size_t len, struct DisplayParams *params, NisStree *value) {
    switch (value->kind) {
    case NIS_STREE_FALSE: {
        if (params->count + 2 <= len) {
            params->count += 2;
            dest[0] = '#';
            dest[1] = 'f';
            return 2;
        }
    } break;
    case NIS_STREE_TRUE: {
        if (params->count + 2 <= len) {
            params->count += 2;
            dest[0] = '#';
            dest[1] = 't';
            return 2;
        }
    } break;
    case NIS_STREE_INT: {
        int digits;
        if (value->vint) {
            digits = floor(log10(labs(value->vint)) + 1.0);
        } else {
            digits = 1;
        }
        if (params->count + digits <= len) {
            params->count += digits;
            long num = value->vint;
            if (num < 0) {
                num = -num;
                dest[0] = '-';
                ++dest;
                ++params->count;
            }
            for (int i = 1; i <= digits; i++) {
                dest[digits - i] = '0' + num % 10;
                num /= 10;
            }
            return digits;
        }
    } break;
    case NIS_STREE_FLOAT: {
        // TODO
    } break;
    case NIS_STREE_NIL: {
        if (params->count + 2 <= len) {
            dest[0] = '(';
            dest[1] = ')';
            params->count += 2;
            return 2;
        }
    } break;
    case NIS_STREE_PAIR: {
        if (nis_list_eh(value)) {
            size_t count = 0;
            if (params->count + 1 <= len) {
                dest[0] = '(';
                ++dest;
                ++params->count;
                ++count;
            }
            for (NisStree *list = value, *car = list->vpair.car;
                 list->kind != NIS_STREE_NIL;
                 list = list->vpair.cdr, car = list->vpair.car) {
                size_t c = nis_display_inner_tree(dest, len, params, car);
                dest += c;
                count += c;
                if (params->count + 1 <= len) {
                    dest[0] = ' ';
                    ++params->count;
                    ++dest;
                    ++count;
                }
            }
            if (count > 2) {
                dest[-1] = ')';
            } else if (count == 2 && params->count + 1 <= len) {
                dest[0] = ')';
                ++params->count;
                ++dest;
                ++count;
            }
            return count;
        } else {
            size_t count = 0;
            if (params->count + 1 <= len) {
                dest[0] = '(';
                ++dest;
                ++params->count;
                ++count;
            }
            size_t c = nis_display_inner_tree(dest, len, params, value->vpair.car);
            dest += c;
            count += c;
            if (params->count + 3 <= len) {
                dest[0] = ' ';
                dest[1] = '.';
                dest[2] = ' ';
                dest += 3;
                params->count += 3;
                count += 3;
            }
            c = nis_display_inner_tree(dest, len, params, value->vpair.cdr);
            dest += c;
            count += c;
            if (params->count + 1 <= len) {
                dest[0] = ')';
                ++dest;
                ++params->count;
                ++count;
            }
            return count;
        }
    } break;
    case NIS_STREE_VECTOR: {
        size_t count = 0;
        if (params->count + 2 <= len) {
            dest[0] = '#';
            dest[1] = '(';
            dest += 2;
            params->count += 2;
            count += 2;
        }
        for (size_t i = 0; i < value->vvec.len; i++) {
            size_t c = nis_display_inner_tree(dest, len, params, value->vvec.ptr[i]);
            dest += c;
            count += c;
            if (params->count + 1 <= len) {
                dest[0] = ' ';
                ++params->count;
                ++dest;
                ++count;
            }
        }
        if (count > 2) {
            dest[-1] = ')';
        } else if (count == 2 && params->count + 1 <= len) {
            dest[0] = ')';
            ++params->count;
            ++dest;
            ++count;
        }
        return count;
    } break;
    case NIS_STREE_BYTE_VECTOR: {
        size_t count = 0;
        if (params->count + 4 <= len) {
            dest[0] = '#';
            dest[1] = 'u';
            dest[2] = '8';
            dest[3] = '(';
            dest += 4;
            params->count += 4;
            count += 4;
        }
        for (size_t i = 0; i < value->vbvec.len; i++) {
            unsigned char byte = value->vbvec.ptr[i];
            int digits;
            if (value->vint) {
                digits = floor(log10(byte) + 1.0);
            } else {
                digits = 1;
            }
            if (params->count + digits <= len) {
                params->count += digits;
                for (int i = 1; i <= digits; i++) {
                    dest[digits - i] = '0' + byte % 10;
                    byte /= 10;
                }
                dest += digits;
                count += digits;
            }
            if (params->count + 1 <= len) {
                dest[0] = ' ';
                ++params->count;
                ++dest;
                ++count;
            }
        }
        if (count > 4) {
            dest[-1] = ')';
        } else if (count == 4 && params->count + 1 <= len) {
            dest[0] = ')';
            ++params->count;
            ++dest;
            ++count;
        }
        return count;
    } break;
    case NIS_STREE_ATOM: {
        const char *atom;
        if (value->flags & NIS_FLAG_INLINE) {
            atom = value->vinline;
        } else {
            atom = value->vatom;
        }
        size_t atomlen = strlen(atom);
        if (params->count + atomlen <= len) {
            memcpy(dest, atom, atomlen);
            params->count += atomlen;
            return atomlen;
        }
    } break;
    }
    return 0;
}

static void nis_display_inner(char *dest, size_t len, struct DisplayParams *params, NisValue *value) {
    switch (value->kind) {
    case NIS_VALUE_FALSE: {
        if (params->count + 2 <= len) {
            params->count += 2;
            dest[0] = '#';
            dest[1] = 'f';
        }
    } break;
    case NIS_VALUE_TRUE: {
        if (params->count + 2 <= len) {
            params->count += 2;
            dest[0] = '#';
            dest[1] = 't';
        }
    } break;
    case NIS_VALUE_INT: {
        int digits;
        if (value->vint) {
            digits = floor(log10(labs(value->vint)) + 1.0);
        } else {
            digits = 1;
        }
        if (params->count + digits <= len) {
            params->count += digits;
            long num = value->vint;
            int sign = 0;
            if (num < 0) {
                num = -num;
                dest[0] = '-';
                sign = 1;
                ++params->count;
            }
            for (int i = 1; i <= digits; i++) {
                dest[sign + digits - i] = '0' + num % 10;
                num /= 10;
            }
        }
    } break;
    case NIS_VALUE_FLOAT: {
        // TODO
    } break;
    case NIS_VALUE_TREE: {
        nis_display_inner_tree(dest, len, params, value->vtree);
    } break;
#define DISPLAY_SPECIAL(x) if (params->count + strlen(x) <= len) strcpy(dest, x);
    case NIS_VALUE_LAMBDA: {
        DISPLAY_SPECIAL("lambda")
    } break;
    case NIS_VALUE_IF: {
        DISPLAY_SPECIAL("if")
    } break;
    case NIS_VALUE_SET: {
        DISPLAY_SPECIAL("set!")
    } break;
    case NIS_VALUE_INCLUDE: {
        DISPLAY_SPECIAL("include")
    } break;
    case NIS_VALUE_INCLUDE_CI: {
        DISPLAY_SPECIAL("include-ci")
    } break;
    case NIS_VALUE_COND: {
        DISPLAY_SPECIAL("cond")
    } break;
    case NIS_VALUE_CASE: {
        DISPLAY_SPECIAL("case")
    } break;
    case NIS_VALUE_ELSE: {
        DISPLAY_SPECIAL("else")
    } break;
    case NIS_VALUE_AND: {
        DISPLAY_SPECIAL("and")
    } break;
    case NIS_VALUE_OR: {
        DISPLAY_SPECIAL("or")
    } break;
    case NIS_VALUE_UNLESS: {
        DISPLAY_SPECIAL("unless")
    } break;
    case NIS_VALUE_COND_EXPAND: {
        DISPLAY_SPECIAL("cond-expand")
    } break;
    case NIS_VALUE_LET: {
        DISPLAY_SPECIAL("let")
    } break;
    case NIS_VALUE_LET_STAR: {
        DISPLAY_SPECIAL("let*")
    } break;
    case NIS_VALUE_LETREC: {
        DISPLAY_SPECIAL("letrec")
    } break;
    case NIS_VALUE_LETREC_STAR: {
        DISPLAY_SPECIAL("letrec*")
    } break;
    case NIS_VALUE_LET_VALUES: {
        DISPLAY_SPECIAL("let-values")
    } break;
    case NIS_VALUE_LET_VALUES_STAR: {
        DISPLAY_SPECIAL("let*-values")
    } break;
    case NIS_VALUE_BEGIN: {
        DISPLAY_SPECIAL("begin")
    } break;
    case NIS_VALUE_DO: {
        DISPLAY_SPECIAL("do")
    } break;
    case NIS_VALUE_LET_LOOP: {
        DISPLAY_SPECIAL("let")
    } break;
    case NIS_VALUE_DELAY: {
        DISPLAY_SPECIAL("delay")
    } break;
    case NIS_VALUE_DELAY_FORCE: {
        DISPLAY_SPECIAL("delay-force")
    } break;
    case NIS_VALUE_FORCE: {
        DISPLAY_SPECIAL("force")
    } break;
    case NIS_VALUE_MAKE_PROMISE: {
        DISPLAY_SPECIAL("make-promise")
    } break;
    case NIS_VALUE_MAKE_PARAMETER: {
        DISPLAY_SPECIAL("make-parameter")
    } break;
    case NIS_VALUE_PARAMETERIZE: {
        DISPLAY_SPECIAL("parameterize")
    } break;
    case NIS_VALUE_GUARD: {
        DISPLAY_SPECIAL("guard")
    } break;
    case NIS_VALUE_QUOTE: {
        DISPLAY_SPECIAL("quote")
    } break;
    case NIS_VALUE_QUASIQUOTE: {
        DISPLAY_SPECIAL("quasiquote")
    } break;
    case NIS_VALUE_UNQUOTE: {
        DISPLAY_SPECIAL("unquote")
    } break;
    case NIS_VALUE_UNQUOTE_SPLICING: {
        DISPLAY_SPECIAL("unquote-splicing")
    } break;
    case NIS_VALUE_CASE_LAMBDA: {
        DISPLAY_SPECIAL("case-lambda")
    } break;
    case NIS_VALUE_LET_SYNTAX: {
        DISPLAY_SPECIAL("let-syntax")
    } break;
    case NIS_VALUE_LETREC_SYNTAX: {
        DISPLAY_SPECIAL("letrec-syntax")
    } break;
    case NIS_VALUE_SYNTAX_RULES: {
        DISPLAY_SPECIAL("syntax-rules")
    } break;
    case NIS_VALUE_SYNTAX_ERROR: {
        DISPLAY_SPECIAL("syntax-error")
    } break;
    }
#undef DISPLAY_SPECIAL
}

size_t nis_display(char *dest, size_t len, NisValue *value) {
    struct DisplayParams params = {
        .count = 0,
        .indent = 0,
        .indent_char = " ",
        .newline_char = "\n",
    };
    nis_display_inner(dest, len - 1, &params, value);
    dest[params.count] = 0;
    return params.count;
}

size_t nis_hlbc_display(char *dest, size_t len, NisHlprog *prog) {
#define DISPLAY_STR(x) if (count + strlen(x) <= len) {  \
        strcpy(dest + count, x);                        \
        count += strlen(x);                             \
    }
    --len;
    size_t count = 0;
    bool end;
    for (size_t i = 0; i < prog->func; i++) {
        end = false;
        NisHlfun *fun = prog->funv + i;
        DISPLAY_STR("(define (");
        DISPLAY_STR(fun->name);
        DISPLAY_STR(")");
        for (size_t j = 0; j < fun->insc; j++) {
            NisHlbc *ins = fun->insv + j;
            DISPLAY_STR("\n  (");
            switch (ins->opcode) {
            case NIS_HLBC_ALLOCA: {
                DISPLAY_STR("alloca");
            } break;
            case NIS_HLBC_LOAD: {
                DISPLAY_STR("load");
            } break;
            case NIS_HLBC_STORE: {
                DISPLAY_STR("store");
            } break;
            case NIS_HLBC_LOAD_U8: {
                DISPLAY_STR("load-u8");
            } break;
            case NIS_HLBC_LOAD_U16: {
                DISPLAY_STR("load-u16");
            } break;
            case NIS_HLBC_LOAD_U32: {
                DISPLAY_STR("load-u32");
            } break;
            case NIS_HLBC_LOAD_U64: {
                DISPLAY_STR("load-u64");
            } break;
            case NIS_HLBC_STORE_U8: {
                DISPLAY_STR("store-u8");
            } break;
            case NIS_HLBC_STORE_U16: {
                DISPLAY_STR("store-u16");
            } break;
            case NIS_HLBC_STORE_U32: {
                DISPLAY_STR("store-u32");
            } break;
            case NIS_HLBC_STORE_U64: {
                DISPLAY_STR("store-u64");
            } break;
            case NIS_HLBC_CALL: {
                DISPLAY_STR("call");
            } break;
            case NIS_HLBC_RETURN: {
                DISPLAY_STR("return");
            } break;
            case NIS_HLBC_BR: {
                DISPLAY_STR("br");
            } break;
            case NIS_HLBC_COND_BR: {
                DISPLAY_STR("br");
            } break;
            case NIS_HLBC_PHI: {
                DISPLAY_STR("phi");
            } break;
            case NIS_HLBC_CMP: {
                DISPLAY_STR("cmp");
            } break;
            case NIS_HLBC_FCMP: {
                DISPLAY_STR("fcmp");
            } break;
            case NIS_HLBC_ADD: {
                DISPLAY_STR("add");
            } break;
            case NIS_HLBC_SUB: {
                DISPLAY_STR("sub");
            } break;
            case NIS_HLBC_MUL: {
                DISPLAY_STR("mul");
            } break;
            case NIS_HLBC_IMUL: {
                DISPLAY_STR("imul");
            } break;
            case NIS_HLBC_DIV: {
                DISPLAY_STR("div");
            } break;
            case NIS_HLBC_IDIV: {
                DISPLAY_STR("idiv");
            } break;
            case NIS_HLBC_REM: {
                DISPLAY_STR("rem");
            } break;
            case NIS_HLBC_IREM: {
                DISPLAY_STR("irem");
            } break;
            case NIS_HLBC_XOR: {
                DISPLAY_STR("xor");
            } break;
            case NIS_HLBC_OR: {
                DISPLAY_STR("or");
            } break;
            case NIS_HLBC_AND: {
                DISPLAY_STR("and");
            } break;
            case NIS_HLBC_SHLL: {
                DISPLAY_STR("shll");
            } break;
            case NIS_HLBC_SHRL: {
                DISPLAY_STR("shrl");
            } break;
            case NIS_HLBC_SHRA: {
                DISPLAY_STR("shra");
            } break;
            case NIS_HLBC_FADD: {
                DISPLAY_STR("fadd");
            } break;
            case NIS_HLBC_FSUB: {
                DISPLAY_STR("fsub");
            } break;
            case NIS_HLBC_FMUL: {
                DISPLAY_STR("fmul");
            } break;
            case NIS_HLBC_FDIV: {
                DISPLAY_STR("fdiv");
            } break;
            case NIS_HLBC_FREM: {
                DISPLAY_STR("frem");
            } break;
            case NIS_HLBC_CAR: {
                DISPLAY_STR("car");
            } break;
            case NIS_HLBC_CDR: {
                DISPLAY_STR("cdr");
            } break;
            case NIS_HLBC_CONS: {
                DISPLAY_STR("cons");
            } break;
            }

            struct DisplayParams params = {
                .count = count,
                .indent = 2,
                .indent_char = " ",
                .newline_char = "\n",
            };

            if (ins->target >= 0) {
                DISPLAY_STR(" @");
                params.count = count;
                NisValue pseudo;
                pseudo.kind = NIS_VALUE_INT;
                pseudo.vint = ins->target;
                nis_display_inner(dest + count, len, &params, &pseudo);
                count = params.count;
            }
            
            for (size_t k = 0; k < ins->argc; k++) {
                DISPLAY_STR(" ");
                NisHlarg *arg = ins->argv + k;
                switch (arg->kind) {
                case NIS_HLBC_ARG_VALUE: {
                    params.count = count;
                    nis_display_inner(dest + count, len, &params, &arg->value);
                } break;
                case NIS_HLBC_ARG_REGISTER: {
                    DISPLAY_STR("%");
                    params.count = count;
                    NisValue pseudo;
                    pseudo.kind = NIS_VALUE_INT;
                    pseudo.vint = arg->ssreg;
                    nis_display_inner(dest + count, len, &params, &pseudo);
                } break;
                case NIS_HLBC_ARG_PROPER: {
                    DISPLAY_STR("%");
                    params.count = count;
                    NisValue pseudo;
                    pseudo.kind = NIS_VALUE_INT;
                    pseudo.vint = -1 - arg->ssarg;
                    nis_display_inner(dest + count, len, &params, &pseudo);
                } break;
                }
                count = params.count;
            }
            
            DISPLAY_STR(")");
        }
        size_t c1 = count;
        DISPLAY_STR(")\n\n");
        size_t c2 = count;
        if (c2 > c1) {
            end = true;
        }
    }
    if (end) {
        --count;
    }
    dest[count] = 0;
    return count;
#undef DISPLAY_STR
}
