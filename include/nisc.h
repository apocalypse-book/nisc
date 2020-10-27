#ifndef NISC_H
#define NISC_H 1

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "riscv.h"

#define NIS_FLAG_MARK 0x1
#define NIS_FLAG_INLINE 0x2

#define NIS_FLAG_WEAK 0x1

typedef struct NisToken NisToken;
typedef struct NisView NisView;
typedef struct NisValue NisValue;
typedef struct NisStree NisStree;
typedef struct NisPair NisPair;
typedef struct NisVector NisVector;
typedef struct NisByteVector NisByteVector;
typedef struct NisGc NisGc;
typedef struct NisHlbc NisHlbc;
typedef struct NisHlarg NisHlarg;
typedef struct NisHlfun NisHlfun;
typedef struct NisHlbuilder NisHlbuilder;
typedef struct NisHlprog NisHlprog;

enum {
    NIS_TOKEN_NONE = 0,
    NIS_TOKEN_IDENT,
    NIS_TOKEN_CHAR,
    NIS_TOKEN_INT,
    NIS_TOKEN_FLOAT,
    NIS_TOKEN_DOT,
    NIS_TOKEN_SINGLE_QUOTE,
    NIS_TOKEN_U8PARENL,
    NIS_TOKEN_PARENL,
    NIS_TOKEN_PARENR,
    NIS_TOKEN_BACKTICK,
    NIS_TOKEN_COMMA,
    NIS_TOKEN_COMMA_AT,
    NIS_TOKEN_DOUBLE_QUOTE,
    NIS_TOKEN_BACKSLASH,
    NIS_TOKEN_SQUAREL,
    NIS_TOKEN_SQUARER,
    NIS_TOKEN_CURLYL,
    NIS_TOKEN_CURLYR,
    NIS_TOKEN_HASH,
    // subkind only
    NIS_TOKEN_INLINE,
    NIS_TOKEN_LABEL,
    NIS_TOKEN_REFERENCE,
};

struct NisView {
    // borrowed
    const char *ptr;
    size_t len;
};

struct NisToken {
    int kind;
    int subkind;
    NisView span;
    union {
        bool vbool;
        long vint;
        double vfloat;
        uint32_t vchar;
        // owned
        char *vatom;
        char vinline[8];
    };
};

struct NisTokens {
    // owned
    NisToken *list;
    size_t len;
};

enum {
    NIS_STREE_FALSE = 0,
    NIS_STREE_TRUE = 1,
    NIS_STREE_INT,
    NIS_STREE_FLOAT,
    NIS_STREE_NIL,
    NIS_STREE_PAIR,
    NIS_STREE_VECTOR,
    NIS_STREE_BYTE_VECTOR,
    NIS_STREE_ATOM,
};

struct NisPair {
    // gc'ed
    NisStree *car;
    // gc'ed
    NisStree *cdr;
};

struct NisVector {
    // gc-owned
    NisStree **ptr;
    size_t len;
    size_t cap;
};

struct NisByteVector {
    // gc-owned
    unsigned char *ptr;
    size_t len;
    size_t cap;
};

struct NisStree {
    int kind;
    int flags;
    NisStree *next;
    NisView span;
    union {
        long vint;
        double vfloat;
        NisPair vpair;
        NisVector vvec;
        NisByteVector vbvec;
        // gc-owned
        const char *vatom;
        const char vinline[24];
    };
};

enum {
    NIS_VALUE_FALSE,
    NIS_VALUE_TRUE,
    NIS_VALUE_INT,
    NIS_VALUE_FLOAT,
    NIS_VALUE_TREE,
    // special forms
    NIS_VALUE_LAMBDA,
    NIS_VALUE_IF,
    NIS_VALUE_SET,
    NIS_VALUE_INCLUDE,
    NIS_VALUE_INCLUDE_CI,
    NIS_VALUE_COND,
    NIS_VALUE_CASE,
    NIS_VALUE_ELSE,
    NIS_VALUE_AND,
    NIS_VALUE_OR,
    NIS_VALUE_UNLESS,
    NIS_VALUE_COND_EXPAND,
    NIS_VALUE_LET,
    NIS_VALUE_LET_STAR,
    NIS_VALUE_LETREC,
    NIS_VALUE_LETREC_STAR,
    NIS_VALUE_LET_VALUES,
    NIS_VALUE_LET_VALUES_STAR,
    NIS_VALUE_BEGIN,
    NIS_VALUE_DO,
    NIS_VALUE_LET_LOOP,
    NIS_VALUE_DELAY,
    NIS_VALUE_DELAY_FORCE,
    NIS_VALUE_FORCE,
    NIS_VALUE_MAKE_PROMISE,
    NIS_VALUE_MAKE_PARAMETER,
    NIS_VALUE_PARAMETERIZE,
    NIS_VALUE_GUARD,
    NIS_VALUE_QUOTE,
    NIS_VALUE_QUASIQUOTE,
    NIS_VALUE_UNQUOTE,
    NIS_VALUE_UNQUOTE_SPLICING,
    NIS_VALUE_CASE_LAMBDA,
    NIS_VALUE_LET_SYNTAX,
    NIS_VALUE_LETREC_SYNTAX,
    NIS_VALUE_SYNTAX_RULES,
    NIS_VALUE_SYNTAX_ERROR,
};

struct NisValue {
    int kind;
    int flags;
    union {
        long vint;
        double vfloat;
        // gc'ed
        NisStree *vtree;
    };
};

struct NisAllocFrame {
    // borrowed
    struct NisAllocFrame *next;
    size_t len;
};

struct NisGc {
    // borrowed
    struct NisAllocFrame *prev;
    // owned
    void *buffer;
    size_t len;
    size_t capacity;

    // gc'ed
    NisStree *nil;
    NisStree *t;
    NisStree *f;
    
    // borrowed
    NisStree *last;
    size_t allocated;
    size_t threshold;
};

enum {
    NIS_HLBC_ARG_VALUE,
    NIS_HLBC_ARG_REGISTER,
    NIS_HLBC_ARG_PROPER,
};

struct NisHlarg {
    int kind;
    int flags;
    union {
        NisValue value;
        int32_t ssreg;
        int32_t ssarg;
    };
};

enum {
    // memory
    NIS_HLBC_ALLOCA,
    NIS_HLBC_LOAD,
    NIS_HLBC_STORE,
    NIS_HLBC_LOAD_U8,
    NIS_HLBC_LOAD_U16,
    NIS_HLBC_LOAD_U32,
    NIS_HLBC_LOAD_U64,
    NIS_HLBC_STORE_U8,
    NIS_HLBC_STORE_U16,
    NIS_HLBC_STORE_U32,
    NIS_HLBC_STORE_U64,
    // control flow
    NIS_HLBC_CALL,
    NIS_HLBC_RETURN,
    NIS_HLBC_BR,
    NIS_HLBC_COND_BR,
    NIS_HLBC_PHI,
    // comparison
    NIS_HLBC_CMP,
    NIS_HLBC_FCMP,
    // integer arithmetic
    NIS_HLBC_ADD,
    NIS_HLBC_SUB,
    NIS_HLBC_MUL,
    NIS_HLBC_IMUL,
    NIS_HLBC_DIV,
    NIS_HLBC_IDIV,
    NIS_HLBC_REM,
    NIS_HLBC_IREM,
    // bitwise arithmetic
    NIS_HLBC_XOR,
    NIS_HLBC_OR,
    NIS_HLBC_AND,
    NIS_HLBC_SHLL,
    NIS_HLBC_SHRL,
    NIS_HLBC_SHRA,
    // floating point arithmetic
    NIS_HLBC_FADD,
    NIS_HLBC_FSUB,
    NIS_HLBC_FMUL,
    NIS_HLBC_FDIV,
    NIS_HLBC_FREM,
    // lisp specific
    NIS_HLBC_CAR,
    NIS_HLBC_CDR,
    NIS_HLBC_CONS,
};

struct NisHlbc {
    int opcode;
    int flags;
    int32_t target;
    size_t argc;
    // owned
    NisHlarg *argv;
};

struct NisHlfun {
    bool present;
    const char *name;
    size_t inss;
    size_t insc;
    // owned
    NisHlbc *insv;
};

struct NisHlbuilder {
    NisGc *gc;
    int32_t regcnt;
    int32_t funref;
    int32_t insref;
    size_t funs;
    size_t func;
    // owned
    NisHlfun *funv;
    int32_t funent;
};

struct NisHlprog {
    int32_t funref;
    int32_t insref;
    size_t func;
    // owned
    NisHlfun *funv;
    int32_t funent;
};

extern const char *TOKEN_STRINGS[];

int nis_lex(struct NisTokens *dest, const char *src, size_t len);
void nis_del_tokens(struct NisTokens *tokens);

static inline size_t nis_align_down(size_t arg, size_t align) {
    return arg & ~(align - 1);
}

static inline size_t nis_align_up(size_t arg, size_t align) {
    return nis_align_down(arg + align - 1, align);
}

void nis_new_gc(NisGc *dest, size_t capacity);
void nis_del_gc(NisGc *dest);

void *nis_alloc(NisGc *gc, size_t size);
void nis_dealloc(NisGc *gc, void *ptr, size_t size);
void *nis_realloc(NisGc *gc, void *ptr, size_t oldsize, size_t newsize);

void nis_false(NisValue *dest, NisGc *gc);
void nis_true(NisValue *dest, NisGc *gc);
void nis_int(NisValue *dest, NisGc *gc, long value);
void nis_float(NisValue *dest, NisGc *gc, double value);
void nis_stree(NisValue *dest, NisGc *gc, NisStree *value);
void nis_nil(NisValue *dest, NisGc *gc);
void nis_pair(NisValue *dest, NisGc *gc, NisValue *car, NisValue *cdr);
void nis_vector(NisValue *dest, NisGc *gc);
void nis_byte_vector(NisValue *dest, NisGc *gc);
void nis_atom(NisValue *dest, NisGc *gc, const char *value);
void nis_special(NisValue *dest, NisGc *gc, int value);

NisStree *nis_value_to_stree(NisGc *gc, NisValue *value);

int nis_parse(NisValue **dest, size_t *len, NisGc *gc, struct NisTokens *tokens);

size_t nis_display(char *dest, size_t len, NisValue *value);

bool nis_tree_list_eh(NisStree *tree);
bool nis_value_list_eh(NisValue *value);
#define nis_list_eh(x) _Generic((x),                                    \
                                NisStree *: nis_tree_list_eh,           \
                                NisValue *: nis_value_list_eh)(x)

size_t nis_tree_list_length(NisStree *tree);
size_t nis_value_list_length(NisValue *value);
#define nis_list_length(x) _Generic((x),                                \
                                    NisStree *: nis_tree_list_length,   \
                                    NisValue *: nis_value_list_length)(x)

static inline bool nis_tree_true_eh(NisStree *tree) {
    return tree->kind != NIS_STREE_FALSE;
}

static inline bool nis_value_true_eh(NisValue *value) {
    return value->kind != NIS_VALUE_FALSE;
}
#define nis_true_eh(x) _Generic((x),                                    \
                               NisStree *: nis_tree_list_eh,            \
                               NisValue *: nis_value_list_eh)(x)

void nis_new_hlbuilder(NisHlbuilder *dest, NisGc *gc);
void nis_build_hlbuilder(NisHlprog *dest, NisHlbuilder *b);
void nis_hlb_entry(NisHlbuilder *b, int32_t funref);
int32_t nis_hlb_addfun(NisHlbuilder *b, const char *name);
void nis_hlb_rmfun(NisHlbuilder *b, int32_t funref);
void nis_hlb_del_ins(NisHlbc *ins);

void nis_hlb_build_call(NisHlarg *dest, NisHlbuilder *b, /* moved */ NisHlarg *argv, size_t argc);
void nis_hlb_build_add(NisHlarg *dest, NisHlbuilder *b, NisHlarg *lhs, NisHlarg *rhs);
void nis_hlb_build_sub(NisHlarg *dest, NisHlbuilder *b, NisHlarg *lhs, NisHlarg *rhs);
void nis_hlb_build_mul(NisHlarg *dest, NisHlbuilder *b, NisHlarg *lhs, NisHlarg *rhs);
void nis_hlb_build_imul(NisHlarg *dest, NisHlbuilder *b, NisHlarg *lhs, NisHlarg *rhs);
void nis_hlb_build_div(NisHlarg *dest, NisHlbuilder *b, NisHlarg *lhs, NisHlarg *rhs);
void nis_hlb_build_idiv(NisHlarg *dest, NisHlbuilder *b, NisHlarg *lhs, NisHlarg *rhs);
void nis_hlb_build_rem(NisHlarg *dest, NisHlbuilder *b, NisHlarg *lhs, NisHlarg *rhs);
void nis_hlb_build_irem(NisHlarg *dest, NisHlbuilder *b, NisHlarg *lhs, NisHlarg *rhs);
void nis_hlb_build_return(NisHlbuilder *b, NisHlarg *retval);

void nis_hlb_prelude_add(NisHlbuilder *b);
void nis_hlb_make_prelude(NisHlbuilder *b);

int nis_to_hlbc(NisHlprog *dest, NisHlbuilder *b, NisValue *program, size_t proglen);
size_t nis_hlbc_display(char *dest, size_t len, NisHlprog *prog);

#endif /* NISC_H */
