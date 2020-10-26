#ifndef NUNSC_H
#define NUNSC_H 1

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define NUN_FLAG_MARK 0x1
#define NUN_FLAG_INLINE 0x2

#define NUN_FLAG_WEAK 0x1

typedef struct NunToken NunToken;
typedef struct NunView NunView;
typedef struct NunValue NunValue;
typedef struct NunStree NunStree;
typedef struct NunPair NunPair;
typedef struct NunVector NunVector;
typedef struct NunByteVector NunByteVector;
typedef struct NunGc NunGc;

enum {
    NUN_TOKEN_NONE = 0,
    NUN_TOKEN_IDENT,
    NUN_TOKEN_CHAR,
    NUN_TOKEN_INT,
    NUN_TOKEN_FLOAT,
    NUN_TOKEN_DOT,
    NUN_TOKEN_SINGLE_QUOTE,
    NUN_TOKEN_U8PARENL,
    NUN_TOKEN_PARENL,
    NUN_TOKEN_PARENR,
    NUN_TOKEN_BACKTICK,
    NUN_TOKEN_COMMA,
    NUN_TOKEN_COMMA_AT,
    NUN_TOKEN_DOUBLE_QUOTE,
    NUN_TOKEN_BACKSLASH,
    NUN_TOKEN_SQUAREL,
    NUN_TOKEN_SQUARER,
    NUN_TOKEN_CURLYL,
    NUN_TOKEN_CURLYR,
    NUN_TOKEN_HASH,
    // subkind only
    NUN_TOKEN_INLINE,
    NUN_TOKEN_LABEL,
    NUN_TOKEN_REFERENCE,
};

struct NunView {
    // borrowed
    const char *ptr;
    size_t len;
};

struct NunToken {
    int kind;
    int subkind;
    NunView span;
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

struct NunTokens {
    NunToken *list;
    size_t len;
};

enum {
    NUN_STREE_FALSE = 0,
    NUN_STREE_TRUE = 1,
    NUN_STREE_INT,
    NUN_STREE_FLOAT,
    NUN_STREE_NIL,
    NUN_STREE_PAIR,
    NUN_STREE_VECTOR,
    NUN_STREE_BYTE_VECTOR,
    NUN_STREE_ATOM,
};

struct NunPair {
    NunStree *car;
    NunStree *cdr;
};

struct NunVector {
    NunStree **ptr;
    size_t len;
    size_t cap;
};

struct NunByteVector {
    unsigned char *ptr;
    size_t len;
    size_t cap;
};

struct NunStree {
    int kind;
    int flags;
    NunStree *next;
    NunView span;
    union {
        long vint;
        double vfloat;
        NunPair vpair;
        NunVector vvec;
        NunByteVector vbvec;
        const char *vatom;
        const char vinline[24];
    };
};

enum {
    NUN_VALUE_FALSE,
    NUN_VALUE_TRUE,
    NUN_VALUE_INT,
    NUN_VALUE_FLOAT,
    NUN_VALUE_TREE,
    // special forms
    NUN_VALUE_LAMBDA,
    NUN_VALUE_IF,
    NUN_VALUE_SET,
    NUN_VALUE_INCLUDE,
    NUN_VALUE_INCLUDE_CI,
    NUN_VALUE_COND,
    NUN_VALUE_CASE,
    NUN_VALUE_ELSE,
    NUN_VALUE_AND,
    NUN_VALUE_OR,
    NUN_VALUE_UNLESS,
    NUN_VALUE_COND_EXPAND,
    NUN_VALUE_LET,
    NUN_VALUE_LET_STAR,
    NUN_VALUE_LETREC,
    NUN_VALUE_LETREC_STAR,
    NUN_VALUE_LET_VALUES,
    NUN_VALUE_LET_VALUES_STAR,
    NUN_VALUE_BEGIN,
    NUN_VALUE_DO,
    NUN_VALUE_LET_LOOP,
    NUN_VALUE_DELAY,
    NUN_VALUE_DELAY_FORCE,
    NUN_VALUE_FORCE,
    NUN_VALUE_MAKE_PROMISE,
    NUN_VALUE_MAKE_PARAMETER,
    NUN_VALUE_PARAMETERIZE,
    NUN_VALUE_GUARD,
    NUN_VALUE_QUOTE,
    NUN_VALUE_QUASIQUOTE,
    NUN_VALUE_UNQUOTE,
    NUN_VALUE_UNQUOTE_SPLICING,
    NUN_VALUE_CASE_LAMBDA,
    NUN_VALUE_LET_SYNTAX,
    NUN_VALUE_LETREC_SYNTAX,
    NUN_VALUE_SYNTAX_RULES,
    NUN_VALUE_SYNTAX_ERROR,
};

struct NunValue {
    int kind;
    int flags;
    union {
        long vint;
        double vfloat;
        NunStree *vtree;
    };
};

struct NunAllocFrame {
    struct NunAllocFrame *next;
    size_t len;
};

struct NunGc {
    struct NunAllocFrame *prev;
    void *buffer;
    size_t len;
    size_t capacity;

    NunStree *nil;
    NunStree *t;
    NunStree *f;
    
    NunStree *last;
    size_t allocated;
    size_t threshold;
};

extern const char *TOKEN_STRINGS[];

int nun_lex(struct NunTokens *dest, const char *src, size_t len);
void nun_del_tokens(struct NunTokens *tokens);

static inline size_t nun_align_down(size_t arg, size_t align) {
    return arg & ~(align - 1);
}

static inline size_t nun_align_up(size_t arg, size_t align) {
    return nun_align_down(arg + align - 1, align);
}

void nun_new_gc(NunGc *dest, size_t capacity);
void nun_del_gc(NunGc *dest);

void *nun_alloc(NunGc *gc, size_t size);
void nun_dealloc(NunGc *gc, void *ptr, size_t size);
void *nun_realloc(NunGc *gc, void *ptr, size_t oldsize, size_t newsize);

void nun_false(NunValue *dest, NunGc *gc);
void nun_true(NunValue *dest, NunGc *gc);
void nun_int(NunValue *dest, NunGc *gc, long value);
void nun_float(NunValue *dest, NunGc *gc, double value);
void nun_stree(NunValue *dest, NunGc *gc, NunStree *value);
void nun_nil(NunValue *dest, NunGc *gc);
void nun_pair(NunValue *dest, NunGc *gc, NunValue *car, NunValue *cdr);
void nun_vector(NunValue *dest, NunGc *gc);
void nun_byte_vector(NunValue *dest, NunGc *gc);
void nun_atom(NunValue *dest, NunGc *gc, const char *value);
void nun_special(NunValue *dest, NunGc *gc, int value);

NunStree *nun_value_to_stree(NunGc *gc, NunValue *value);

int nun_parse(NunValue **dest, size_t *len, NunGc *gc, struct NunTokens *tokens);

size_t nun_display(char *dest, size_t len, NunValue *value);

bool nun_tree_list_eh(NunStree *tree);
bool nun_value_list_eh(NunValue *value);

#define nun_list_eh(x) _Generic((x),                                    \
                                NunStree *: nun_tree_list_eh,           \
                                NunValue *: nun_value_list_eh)(x)

static inline bool nun_tree_true_eh(NunStree *tree) {
    return tree->kind != NUN_STREE_FALSE;
}

static inline bool nun_value_true_eh(NunValue *value) {
    return value->kind != NUN_VALUE_FALSE;
}

#define nun_true_eh(x) _Generic((x),                                    \
                               NunStree *: nun_tree_list_eh,            \
                               NunValue *: nun_value_list_eh)(x)

#endif /* NUNSC_H */
