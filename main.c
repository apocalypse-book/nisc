#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tgmath.h>
#include <errno.h>
#include <sys/stat.h>
#include "include/nunsc.h"

const char *TOKEN_STRINGS[] = {
    [NUN_TOKEN_NONE] = "<none>",
    [NUN_TOKEN_IDENT] = "<identifer>",
    [NUN_TOKEN_CHAR] = "<character>",
    [NUN_TOKEN_INT] = "<integer>",
    [NUN_TOKEN_FLOAT] = "<real>",
    [NUN_TOKEN_DOT] = "`.`",
    [NUN_TOKEN_SINGLE_QUOTE] = "`'`",
    [NUN_TOKEN_U8PARENL] = "`u8(`",
    [NUN_TOKEN_PARENL] = "`(`",
    [NUN_TOKEN_PARENR] = "`)`",
    [NUN_TOKEN_BACKTICK] = "`\\``",
    [NUN_TOKEN_COMMA] = "`,`",
    [NUN_TOKEN_COMMA_AT] = "`,@`",
    [NUN_TOKEN_DOUBLE_QUOTE] = "`\"`",
    [NUN_TOKEN_BACKSLASH] = "`\\\\`",
    [NUN_TOKEN_SQUAREL] = "`[`",
    [NUN_TOKEN_SQUARER] = "`]`",
    [NUN_TOKEN_CURLYL] = "`{`",
    [NUN_TOKEN_CURLYR] = "`}`",
    [NUN_TOKEN_HASH] = "`#`",
    [NUN_TOKEN_INLINE] = "<identifier>",
    [NUN_TOKEN_LABEL] = "<integer>`=`",
    [NUN_TOKEN_REFERENCE] = "<integer>`#`",
    NULL,
};

enum {
    NUN_LEX_NORMAL,
    NUN_LEX_HASH,
    NUN_LEX_STRING,
};

static bool is_ident_begin(int ch) {
    switch (ch) {
    case '!': case '$': case '%': case '&': case '*':
    case '+': case '-': case '.': case '/': case ':':
    case '<': case '=': case '>': case '?': case '@':
    case '^': case '_': case '~':
    case 'a': case 'b': case 'c': case 'd': case 'e':
    case 'f': case 'g': case 'h': case 'i': case 'j':
    case 'k': case 'l': case 'm': case 'n': case 'o':
    case 'p': case 'q': case 'r': case 's': case 't':
    case 'u': case 'v': case 'w': case 'x': case 'y':
    case 'z':
    case 'A': case 'B': case 'C': case 'D': case 'E':
    case 'F': case 'G': case 'H': case 'I': case 'J':
    case 'K': case 'L': case 'M': case 'N': case 'O':
    case 'P': case 'Q': case 'R': case 'S': case 'T':
    case 'U': case 'V': case 'W': case 'X': case 'Y':
    case 'Z':
        return 1;
    default:
        return 0;
    }
}

static bool is_ident_cont(int ch) {
    switch (ch) {
    case '!': case '$': case '%': case '&': case '*':
    case '+': case '-': case '.': case '/': case ':':
    case '<': case '=': case '>': case '?': case '@':
    case '^': case '_': case '~':
    case 'a': case 'b': case 'c': case 'd': case 'e':
    case 'f': case 'g': case 'h': case 'i': case 'j':
    case 'k': case 'l': case 'm': case 'n': case 'o':
    case 'p': case 'q': case 'r': case 's': case 't':
    case 'u': case 'v': case 'w': case 'x': case 'y':
    case 'z':
    case 'A': case 'B': case 'C': case 'D': case 'E':
    case 'F': case 'G': case 'H': case 'I': case 'J':
    case 'K': case 'L': case 'M': case 'N': case 'O':
    case 'P': case 'Q': case 'R': case 'S': case 'T':
    case 'U': case 'V': case 'W': case 'X': case 'Y':
    case 'Z':
    case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9': case '0':
        return 1;
    default:
        return 0;
    }
}

int nun_lex(struct NunTokens *dest, const char *src, size_t len) {
    size_t cap = 16;
    dest->list = malloc(cap * sizeof(NunToken));
    dest->len = 0;

    int state = NUN_LEX_NORMAL;

    const char *ptr = src;
    for (size_t offset = 0; offset < len; ) {
        if (dest->len == cap) {
            cap *= 2;
            dest->list = realloc(dest->list, cap * sizeof(NunToken));
        }
        switch (state) {
        case NUN_LEX_NORMAL: {
            ptr = src + offset;
            char ch = src[offset++];
            switch (ch) {
            case '(': {
                dest->list[dest->len].kind = NUN_TOKEN_PARENL;
                dest->list[dest->len].span.ptr = ptr;
                dest->list[dest->len].span.len = 1;
                dest->len++;
            } break;
            case ')': {
                dest->list[dest->len].kind = NUN_TOKEN_PARENR;
                dest->list[dest->len].span.ptr = ptr;
                dest->list[dest->len].span.len = 1;
                dest->len++;
            } break;
            case '\'': {
                dest->list[dest->len].kind = NUN_TOKEN_SINGLE_QUOTE;
                dest->list[dest->len].span.ptr = ptr;
                dest->list[dest->len].span.len = 1;
                dest->len++;
            } break;
            case '`': {
                dest->list[dest->len].kind = NUN_TOKEN_BACKTICK;
                dest->list[dest->len].span.ptr = ptr;
                dest->list[dest->len].span.len = 1;
                dest->len++;
            } break;
            case ',': {
                if (src[offset] == '@') {
                    offset++;
                    dest->list[dest->len].kind = NUN_TOKEN_COMMA_AT;
                    dest->list[dest->len].span.ptr = ptr;
                    dest->list[dest->len].span.len = 2;
                } else {
                    dest->list[dest->len].kind = NUN_TOKEN_COMMA;
                    dest->list[dest->len].span.ptr = ptr;
                    dest->list[dest->len].span.len = 1;
                }
                dest->len++;
            } break;
            case '[': {
                dest->list[dest->len].kind = NUN_TOKEN_SQUAREL;
                dest->list[dest->len].span.ptr = ptr;
                dest->list[dest->len].span.len = 1;
                dest->len++;
            } break;
            case ']': {
                dest->list[dest->len].kind = NUN_TOKEN_SQUARER;
                dest->list[dest->len].span.ptr = ptr;
                dest->list[dest->len].span.len = 1;
                dest->len++;
            } break;
            case '{': {
                dest->list[dest->len].kind = NUN_TOKEN_CURLYL;
                dest->list[dest->len].span.ptr = ptr;
                dest->list[dest->len].span.len = 1;
                dest->len++;
            } break;
            case '}': {
                dest->list[dest->len].kind = NUN_TOKEN_CURLYR;
                dest->list[dest->len].span.ptr = ptr;
                dest->list[dest->len].span.len = 1;
                dest->len++;
            } break;
            case '\t': case '\n': case ' ': {
                // nothing
            } break;
            case '"': {
                // TODO
            } break;
            case '\\': {
                // TODO
            } break;
            case '1': case '2': case '3':
            case '4': case '5': case '6':
            case '7': case '8': case '9':
            case '0': {
                size_t len = 1;
                long num = ch - '0';
                for (ch = src[offset]; ch >= '0' && ch <= '9'; ch = src[offset++]) {
                    num *= 10;
                    num += ch;
                    ++len;
                }
                dest->list[dest->len].kind = NUN_TOKEN_INT;
                dest->list[dest->len].vint = num;
                dest->list[dest->len].span.ptr = ptr;
                dest->list[dest->len].span.len = len;
                dest->len++;
            } break;
            default: {
                if (is_ident_begin(ch)) {
                    size_t len = 1;
                    size_t cap = 0;
                    char vinline[8];
                    size_t idx = 0;
                    char *vatom = NULL;
                    vinline[idx++] = ch;
                    for (ch = src[offset]; is_ident_cont(ch); ch = src[offset++]) {
                        if (idx == 7) {
                            cap = 16;
                            vatom = malloc(cap * sizeof(char));
                            memcpy(vatom, vinline, 7);
                        }
                        if (idx >= 7) {
                            vatom[idx++] = ch;
                        } else {
                            vinline[idx++] = ch;
                        }
                        ++len;
                    }
                    
                    if (vatom) {
                        dest->list[dest->len].kind = NUN_TOKEN_IDENT;
                        dest->list[dest->len].subkind = NUN_TOKEN_NONE;
                        dest->list[dest->len].vatom = vatom;
                    } else {
                        vinline[len] = '\0';
                        if (strcmp(vinline, ".") == 0) {
                            dest->list[dest->len].kind = NUN_TOKEN_DOT;
                        } else {
                            dest->list[dest->len].kind = NUN_TOKEN_IDENT;
                            dest->list[dest->len].subkind = NUN_TOKEN_INLINE;
                            strncpy(dest->list[dest->len].vinline, vinline, 8);
                        }
                    }
                    dest->list[dest->len].span.ptr = ptr;
                    dest->list[dest->len].span.len = len;
                    dest->len++;
                } else {
                    fprintf(stderr, "nunsc:%s:%d: error: `%c` is not a valid character\n", __FILE__, __LINE__, ch);
                    return 1;
                }
            } break;
            }
        } break;
        case NUN_LEX_HASH: {
        } break;
        case NUN_LEX_STRING: {
        } break;
        }
    }
    return 0;
}

void nun_new_gc(NunGc *dest, size_t capacity) {
    dest->buffer = malloc(capacity);
    dest->len = sizeof(struct NunAllocFrame);
    dest->capacity = capacity;
    dest->prev = dest->buffer;
    dest->prev->next = NULL;
    dest->prev->len = capacity;

    dest->nil = nun_alloc(dest, sizeof(NunStree));
    dest->nil->kind = NUN_STREE_NIL;
    dest->nil->flags = 0;
    dest->nil->next = NULL;
    
    dest->t = nun_alloc(dest, sizeof(NunStree));
    dest->t->kind = NUN_STREE_TRUE;
    dest->t->flags = 0;
    dest->t->next = NULL;
    
    dest->f = nun_alloc(dest, sizeof(NunStree));
    dest->f->kind = NUN_STREE_FALSE;
    dest->f->flags = 0;
    dest->f->next = NULL;

    dest->last = NULL;
    dest->allocated = 0;
    dest->threshold = capacity / sizeof(NunStree) >> 3;
}

void nun_del_gc(NunGc *dest) {
    free(dest->buffer);
}

void *nun_alloc(NunGc *gc, size_t size) {
    if (size == 0) {
        return NULL;
    }
    
    size = nun_align_up(size, 8);
    if (gc->len + size >= gc->capacity) {
        fprintf(stderr, "nunsc:%s:%d: error: out of memory\n", __FILE__, __LINE__);
        exit(1);
    }

    struct NunAllocFrame **prev = &gc->prev;
    struct NunAllocFrame *frame = gc->prev;
    while (frame) {
        if (frame->len >= size) {
            if (frame->len - size < sizeof(struct NunAllocFrame)) {
                void *ptr = frame;
                *prev = frame->next;
                return ptr;
            } else {
                void *ptr = frame;
                struct NunAllocFrame *nextptr = (void *) ((char *) ptr + size);
                struct NunAllocFrame *nextnext = frame->next;
                size_t nextlen = frame->len - size;
                *prev = nextptr;
                frame = nextptr;
                frame->next = nextnext;
                frame->len = nextlen;
                return ptr;
            }
        }
        prev = &frame->next;
        frame = frame->next;
    }
    
    fprintf(stderr, "nunsc:%s:%d: error: out of memory\n", __FILE__, __LINE__);
    exit(1);
}

void nun_dealloc(NunGc *gc, void *ptr, size_t size) {
    size = nun_align_up(size, 8);
    
    struct NunAllocFrame **prev = &gc->prev;
    struct NunAllocFrame *frame = gc->prev;
    while (frame) {
        if (frame == ptr) {
            fprintf(stderr, "nunsc:%s:%d: error: double free\n", __FILE__, __LINE__);
            exit(1);
        } else if ((size_t) frame > (size_t) ptr) {
            frame = *prev;
            if ((char *) frame + frame->len == ptr) {
                frame->len += size;
            } else {
                frame->next = ptr;
                frame = frame->next;
                frame->len = size;
            }
            return;
        }
        prev = &frame->next;
        frame = frame->next;
    }
    
    fprintf(stderr, "nunsc:%s:%d: error: invalid free\n", __FILE__, __LINE__);
    exit(1);
}

void *nun_realloc(NunGc *gc, void *ptr, size_t oldsize, size_t newsize) {
    oldsize = nun_align_up(oldsize, 8);
    newsize = nun_align_up(newsize, 8);

    if (newsize == oldsize) {
        return ptr;
    } else if (newsize > oldsize) {
        struct NunAllocFrame *frame = gc->prev;
        while (frame) {
            struct NunAllocFrame *next = frame->next;
            if ((char *) ptr + oldsize == (char *) next) {
                if (oldsize + next->len >= newsize) {
                    if (oldsize + next->len == newsize) {
                        frame->next = next->next;
                    } else {
                        size_t len = next->len - (newsize - oldsize);
                        frame->next = next + (newsize - oldsize);
                        frame->next->next = next->next;
                        frame->next->len = len;
                    }
                    return ptr;
                } else {
                    void *oldptr = ptr;
                    void *newptr = nun_alloc(gc, newsize);
                    memcpy(newptr, oldptr, oldsize);
                    nun_dealloc(gc, oldptr, oldsize);
                    return newptr;
                }
            }
            frame = next;
        }
    }
    
    fprintf(stderr, "nunsc:%s:%d: error: invalid free\n", __FILE__, __LINE__);
    exit(1);
}

void nun_false(NunValue *dest, NunGc *gc) {
    (void) gc;
    dest->kind = NUN_VALUE_FALSE;
}

void nun_true(NunValue *dest, NunGc *gc) {
    (void) gc;
    dest->kind = NUN_VALUE_TRUE;
}

void nun_int(NunValue *dest, NunGc *gc, long value) {
    (void) gc;
    dest->kind = NUN_VALUE_INT;
    dest->vint = value;
}

void nun_float(NunValue *dest, NunGc *gc, double value) {
    (void) gc;
    dest->kind = NUN_VALUE_FLOAT;
    dest->vfloat = value;
}

void nun_stree(NunValue *dest, NunGc *gc, NunStree *value) {
    (void) gc;
    dest->kind = NUN_VALUE_TREE;
    dest->vtree = value;
}

void nun_nil(NunValue *dest, NunGc *gc) {
    dest->kind = NUN_VALUE_TREE;
    dest->vtree = gc->nil;
}

void nun_pair(NunValue *dest, NunGc *gc, NunValue *car, NunValue *cdr) {
    dest->kind = NUN_VALUE_TREE;
    NunStree *tree = nun_alloc(gc, sizeof(NunStree));
    tree->kind = NUN_STREE_PAIR;
    tree->flags = 0;
    tree->next = gc->last;
    tree->vpair.car = nun_value_to_stree(gc, car);
    tree->vpair.cdr = nun_value_to_stree(gc, cdr);
    dest->vtree = tree;
    gc->last = tree;
}

void nun_vector(NunValue *dest, NunGc *gc) {
    dest->kind = NUN_VALUE_TREE;
    NunStree *tree = nun_alloc(gc, sizeof(NunStree));
    tree->kind = NUN_STREE_PAIR;
    tree->flags = 0;
    tree->next = gc->last;
    tree->vvec.ptr = NULL;
    tree->vvec.len = 0;
    tree->vvec.cap = 0;
    dest->vtree = tree;
    gc->last = tree;
}

void nun_byte_vector(NunValue *dest, NunGc *gc) {
    dest->kind = NUN_VALUE_TREE;
    NunStree *tree = nun_alloc(gc, sizeof(NunStree));
    tree->kind = NUN_STREE_PAIR;
    tree->flags = 0;
    tree->next = gc->last;
    tree->vbvec.ptr = NULL;
    tree->vbvec.len = 0;
    tree->vbvec.cap = 0;
    dest->vtree = tree;
    gc->last = tree;
}

void nun_atom(NunValue *dest, NunGc *gc, const char *value) {
    size_t len = strlen(value);
    
    dest->kind = NUN_VALUE_TREE;
    NunStree *tree = nun_alloc(gc, sizeof(NunStree));
    if (len < 24) {
        tree->kind = NUN_STREE_ATOM;
        tree->flags = NUN_FLAG_INLINE;
        tree->next = gc->last;
        strcpy((char *) tree->vinline, value);
    } else {
        tree->kind = NUN_STREE_ATOM;
        tree->flags = NUN_FLAG_INLINE;
        tree->next = gc->last;
        tree->vatom = nun_alloc(gc, len + 1);
        strcpy((char *) tree->vatom, value);
    }
    dest->vtree = tree;
    gc->last = tree;
}

void nun_special(NunValue *dest, NunGc *gc, int value) {
    (void) gc;
    dest->kind = value;
}

NunStree *nun_value_to_stree(NunGc *gc, NunValue *value) {
    switch (value->kind) {
    case NUN_VALUE_TRUE:
        return gc->t;
    case NUN_VALUE_FALSE:
        return gc->f;
    case NUN_VALUE_INT: {
        NunStree *tree = nun_alloc(gc, sizeof(NunStree));
        tree->kind = NUN_STREE_INT;
        tree->flags = 0;
        tree->next = gc->last;
        tree->vint = value->vint;
        gc->last = tree;
        return tree;
    }
    case NUN_VALUE_FLOAT: {
        NunStree *tree = nun_alloc(gc, sizeof(NunStree));
        tree->kind = NUN_STREE_FLOAT;
        tree->flags = 0;
        tree->next = gc->last;
        tree->vfloat = value->vfloat;
        gc->last = tree;
        return tree;
    }
    case NUN_VALUE_TREE:
        return value->vtree;
    default:
        fprintf(stderr, "nunsc:%s:%d: error: undefined value\n", __FILE__, __LINE__);
        exit(-1);
    }
}

static int nun_parse_one(NunValue *dest, NunGc *gc, struct NunTokens *tokens, size_t *idx) {
    NunToken *token = &tokens->list[*idx];
    ++*idx;
    switch (token->kind) {
    case NUN_TOKEN_IDENT: {
        const char *atom;
        if (token->subkind == NUN_TOKEN_INLINE) {
            atom = token->vinline;
        } else {
            atom = token->vatom;
        }
        if (strcmp(atom, "lambda") == 0) {
            nun_special(dest, gc, NUN_VALUE_LAMBDA);
        } else if (strcmp(atom, "if") == 0) {
            nun_special(dest, gc, NUN_VALUE_IF);
        } else if (strcmp(atom, "set!") == 0) {
            nun_special(dest, gc, NUN_VALUE_SET);
        } else if (strcmp(atom, "include") == 0) {
            nun_special(dest, gc, NUN_VALUE_INCLUDE);
        } else if (strcmp(atom, "include-ci") == 0) {
            nun_special(dest, gc, NUN_VALUE_INCLUDE_CI);
        } else if (strcmp(atom, "cond") == 0) {
            nun_special(dest, gc, NUN_VALUE_COND);
        } else if (strcmp(atom, "case") == 0) {
            nun_special(dest, gc, NUN_VALUE_CASE);
        } else if (strcmp(atom, "else") == 0) {
            nun_special(dest, gc, NUN_VALUE_ELSE);
        } else if (strcmp(atom, "and") == 0) {
            nun_special(dest, gc, NUN_VALUE_AND);
        } else if (strcmp(atom, "or") == 0) {
            nun_special(dest, gc, NUN_VALUE_OR);
        } else if (strcmp(atom, "unless") == 0) {
            nun_special(dest, gc, NUN_VALUE_UNLESS);
        } else if (strcmp(atom, "cond-expand") == 0) {
            nun_special(dest, gc, NUN_VALUE_COND_EXPAND);
        } else if (strcmp(atom, "let") == 0) {
            nun_special(dest, gc, NUN_VALUE_LET);
        } else if (strcmp(atom, "let*") == 0) {
            nun_special(dest, gc, NUN_VALUE_LET_STAR);
        } else if (strcmp(atom, "letrec") == 0) {
            nun_special(dest, gc, NUN_VALUE_LETREC);
        } else if (strcmp(atom, "letrec*") == 0) {
            nun_special(dest, gc, NUN_VALUE_LETREC_STAR);
        } else if (strcmp(atom, "let-values") == 0) {
            nun_special(dest, gc, NUN_VALUE_LET_VALUES);
        } else if (strcmp(atom, "let*-values") == 0) {
            nun_special(dest, gc, NUN_VALUE_LET_VALUES_STAR);
        } else if (strcmp(atom, "begin") == 0) {
            nun_special(dest, gc, NUN_VALUE_BEGIN);
        } else if (strcmp(atom, "do") == 0) {
            nun_special(dest, gc, NUN_VALUE_DO);
        } else if (strcmp(atom, "delay") == 0) {
            nun_special(dest, gc, NUN_VALUE_DELAY);
        } else if (strcmp(atom, "delay-force") == 0) {
            nun_special(dest, gc, NUN_VALUE_DELAY_FORCE);
        } else if (strcmp(atom, "force") == 0) {
            nun_special(dest, gc, NUN_VALUE_FORCE);
        } else if (strcmp(atom, "make-promise") == 0) {
            nun_special(dest, gc, NUN_VALUE_MAKE_PROMISE);
        } else if (strcmp(atom, "make-parameter") == 0) {
            nun_special(dest, gc, NUN_VALUE_MAKE_PARAMETER);
        } else if (strcmp(atom, "parameterize") == 0) {
            nun_special(dest, gc, NUN_VALUE_PARAMETERIZE);
        } else if (strcmp(atom, "guard") == 0) {
            nun_special(dest, gc, NUN_VALUE_GUARD);
        } else if (strcmp(atom, "quasiquote") == 0) {
            nun_special(dest, gc, NUN_VALUE_QUASIQUOTE);
        } else if (strcmp(atom, "unquote") == 0) {
            nun_special(dest, gc, NUN_VALUE_UNQUOTE);
        } else if (strcmp(atom, "unquote-splicing") == 0) {
            nun_special(dest, gc, NUN_VALUE_UNQUOTE_SPLICING);
        } else if (strcmp(atom, "case-lambda") == 0) {
            nun_special(dest, gc, NUN_VALUE_CASE_LAMBDA);
        } else if (strcmp(atom, "let-syntax") == 0) {
            nun_special(dest, gc, NUN_VALUE_LET_SYNTAX);
        } else if (strcmp(atom, "letrec-syntax") == 0) {
            nun_special(dest, gc, NUN_VALUE_LETREC_SYNTAX);
        } else if (strcmp(atom, "syntax-rules") == 0) {
            nun_special(dest, gc, NUN_VALUE_SYNTAX_RULES);
        } else if (strcmp(atom, "syntax-error") == 0) {
            nun_special(dest, gc, NUN_VALUE_SYNTAX_ERROR);
        } else {
            nun_atom(dest, gc, atom);
            dest->vtree->span = token->span;
        }
        return 0;
    }
    case NUN_TOKEN_CHAR: {
        nun_int(dest, gc, token->vchar);
        return 0;
    }
    case NUN_TOKEN_INT: {
        nun_int(dest, gc, token->vint);
        return 0;
    }
    case NUN_TOKEN_FLOAT: {
        nun_float(dest, gc, token->vfloat);
        return 0;
    }
    case NUN_TOKEN_PARENL: {
        size_t len = token->span.len;

        NunToken *token2 = &tokens->list[*idx];
        
        if (token2->kind == NUN_TOKEN_PARENR) {
            ++*idx;
            nun_nil(dest, gc);
            return 0;
        }

        NunValue fst;
        nun_parse_one(&fst, gc, tokens, idx);
        
        token2 = &tokens->list[*idx];

        if (token2->kind == NUN_TOKEN_PARENR) {
            ++*idx;
            NunValue nil;
            nun_nil(&nil, gc);
            nun_pair(dest, gc, &fst, &nil);
            
            len += (token2->span.ptr - (token->span.ptr + len)) + token2->span.len;
            dest->vtree->span.ptr = token->span.ptr;
            dest->vtree->span.len = len;

            return 0;
        } else if (token2->kind == NUN_TOKEN_DOT) {
            ++*idx;
            NunValue snd;
            nun_parse_one(&snd, gc, tokens, idx);
            nun_pair(dest, gc, &fst, &snd);
            
            token2 = &tokens->list[*idx];

            if (token2->kind != NUN_TOKEN_PARENR) {
                fprintf(stderr,
                        "nunsc:%s:%d: error: unexpected token: %s\n",
                        __FILE__,
                        __LINE__,
                        TOKEN_STRINGS[token2->kind]);
                return 1;
            }
            
            len += (token2->span.ptr - (token->span.ptr + len)) + token2->span.len;
            dest->vtree->span.ptr = token->span.ptr;
            dest->vtree->span.len = len;
            
            return 0;
        }

        NunValue nil;
        nun_nil(&nil, gc);
        nun_pair(dest, gc, &fst, &nil);

        NunStree *list = dest->vtree;
        
        token2 = &tokens->list[*idx];

        while (token2->kind != NUN_TOKEN_PARENR) {
            if (token2->kind == NUN_TOKEN_DOT) {
                ++*idx;
                NunValue last;
                nun_parse_one(&last, gc, tokens, idx);
                list->vpair.cdr = nun_value_to_stree(gc, &last);
                break;
            } else {
                NunValue next;
                nun_parse_one(&next, gc, tokens, idx);
                NunValue newlist;
                nun_pair(&newlist, gc, &next, &nil);
                list->vpair.cdr = newlist.vtree;
                list = list->vpair.cdr;
            }
            token2 = &tokens->list[*idx];
        }
        
        token2 = &tokens->list[*idx];

        if (token2->kind != NUN_TOKEN_PARENR) {
            fprintf(stderr,
                    "nunsc:%s:%d: error: unexpected token: %s\n",
                    __FILE__,
                    __LINE__,
                    TOKEN_STRINGS[token2->kind]);
            return 1;
        }

        ++*idx;
        
        len += (token2->span.ptr - (token->span.ptr + len)) + token2->span.len;
        dest->vtree->span.ptr = token->span.ptr;
        dest->vtree->span.len = len;

        return 0;
    }
    case NUN_TOKEN_SINGLE_QUOTE: {
        NunValue expr;
        nun_parse_one(&expr, gc, tokens, idx);

        NunValue nil;
        nun_nil(&nil, gc);
        
        NunValue list;
        nun_pair(&list, gc, &expr, &nil);

        NunValue quote;
        nun_special(&quote, gc, NUN_VALUE_QUOTE);

        nun_pair(dest, gc, &quote, &list);
        return 0;
    }
    case NUN_TOKEN_BACKTICK: {
        NunValue expr;
        nun_parse_one(&expr, gc, tokens, idx);

        NunValue nil;
        nun_nil(&nil, gc);
        
        NunValue list;
        nun_pair(&list, gc, &expr, &nil);

        NunValue quote;
        nun_special(&quote, gc, NUN_VALUE_QUASIQUOTE);

        nun_pair(dest, gc, &quote, &list);
        return 0;
    }
    case NUN_TOKEN_COMMA: {
        NunValue expr;
        nun_parse_one(&expr, gc, tokens, idx);

        NunValue nil;
        nun_nil(&nil, gc);
        
        NunValue list;
        nun_pair(&list, gc, &expr, &nil);

        NunValue unquote;
        nun_special(&unquote, gc, NUN_VALUE_UNQUOTE);

        nun_pair(dest, gc, &unquote, &list);
        return 0;
    }
    case NUN_TOKEN_COMMA_AT: {
        NunValue expr;
        nun_parse_one(&expr, gc, tokens, idx);

        NunValue nil;
        nun_nil(&nil, gc);
        
        NunValue list;
        nun_pair(&list, gc, &expr, &nil);

        NunValue unquote;
        nun_special(&unquote, gc, NUN_VALUE_UNQUOTE_SPLICING);

        nun_pair(dest, gc, &unquote, &list);
        return 0;
    }
    default:
        fprintf(stderr,
                "nunsc:%s:%d: error: unexpected token: %s\n",
                __FILE__,
                __LINE__,
                TOKEN_STRINGS[token->kind]);
        return 1;
    }
}

int nun_parse(NunValue **dest, size_t *len, NunGc *gc, struct NunTokens *tokens) {
    int status = 0;
    size_t offset = 0;
    size_t capacity = 64;
    *dest = malloc(capacity * sizeof(NunValue));
    *len = 0;
    size_t i = 0;
    while (i < tokens->len) {
        if (offset == capacity) {
            capacity *= 2;
            *dest = realloc(*dest, capacity * sizeof(NunValue));
        }
        int s = nun_parse_one(*dest + offset++, gc, tokens, &i);
        if (!s) {
            ++*len;
        }
        status |= s;
    }
    return status;
}

struct DisplayParams {
    size_t count;
    size_t indent;
    const char *indent_char;
    const char *newline_char;
};

static size_t nun_display_inner_tree(char *dest, size_t len, struct DisplayParams *params, NunStree *value) {
    switch (value->kind) {
    case NUN_STREE_FALSE: {
        if (params->count + 2 <= len) {
            params->count += 2;
            dest[0] = '#';
            dest[1] = 'f';
            return 2;
        }
    } break;
    case NUN_STREE_TRUE: {
        if (params->count + 2 <= len) {
            params->count += 2;
            dest[0] = '#';
            dest[1] = 't';
            return 2;
        }
    } break;
    case NUN_STREE_INT: {
        int digits = floor(log10(value->vint) + 1.0);
        if (params->count + digits <= len) {
            params->count += digits;
            long num = value->vint;
            if (num < 0) {
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
    case NUN_STREE_FLOAT: {
        // TODO
    } break;
    case NUN_STREE_NIL: {
        if (params->count + 2 <= len) {
            dest[0] = '(';
            dest[1] = ')';
            params->count += 2;
            return 2;
        }
    } break;
    case NUN_STREE_PAIR: {
        if (nun_list_eh(value)) {
            size_t count = 0;
            if (params->count + 1 <= len) {
                dest[0] = '(';
                ++dest;
                ++params->count;
                ++count;
            }
            for (NunStree *list = value, *car = list->vpair.car;
                 list->kind != NUN_STREE_NIL;
                 list = list->vpair.cdr, car = list->vpair.car) {
                size_t c = nun_display_inner_tree(dest, len, params, car);
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
            size_t c = nun_display_inner_tree(dest, len, params, value->vpair.car);
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
            c = nun_display_inner_tree(dest, len, params, value->vpair.cdr);
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
    case NUN_STREE_VECTOR: {
        size_t count = 0;
        if (params->count + 2 <= len) {
            dest[0] = '#';
            dest[1] = '(';
            dest += 2;
            params->count += 2;
            count += 2;
        }
        for (size_t i = 0; i < value->vvec.len; i++) {
            size_t c = nun_display_inner_tree(dest, len, params, value->vvec.ptr[i]);
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
    case NUN_STREE_BYTE_VECTOR: {
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
            int digits = floor(log10(byte) + 1.0);
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
    case NUN_STREE_ATOM: {
        const char *atom;
        if (value->flags & NUN_FLAG_INLINE) {
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

static void nun_display_inner(char *dest, size_t len, struct DisplayParams *params, NunValue *value) {
    switch (value->kind) {
    case NUN_VALUE_FALSE: {
        if (params->count + 2 <= len) {
            params->count += 2;
            dest[0] = '#';
            dest[1] = 'f';
        }
    } break;
    case NUN_VALUE_TRUE: {
        if (params->count + 2 <= len) {
            params->count += 2;
            dest[0] = '#';
            dest[1] = 't';
        }
    } break;
    case NUN_VALUE_INT: {
        int digits = floor(log10(value->vint) + 1.0);
        if (params->count + digits <= len) {
            params->count += digits;
            long num = value->vint;
            int sign = 0;
            if (num < 0) {
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
    case NUN_VALUE_FLOAT: {
        // TODO
    } break;
    case NUN_VALUE_TREE: {
        nun_display_inner_tree(dest, len, params, value->vtree);
    } break;
#define DISPLAY_SPECIAL(x) if (params->count + strlen(x) <= len) strcpy(dest, x);
    case NUN_VALUE_LAMBDA: {
        DISPLAY_SPECIAL("lambda")
    } break;
    case NUN_VALUE_IF: {
        DISPLAY_SPECIAL("if")
    } break;
    case NUN_VALUE_SET: {
        DISPLAY_SPECIAL("set!")
    } break;
    case NUN_VALUE_INCLUDE: {
        DISPLAY_SPECIAL("include")
    } break;
    case NUN_VALUE_INCLUDE_CI: {
        DISPLAY_SPECIAL("include-ci")
    } break;
    case NUN_VALUE_COND: {
        DISPLAY_SPECIAL("cond")
    } break;
    case NUN_VALUE_CASE: {
        DISPLAY_SPECIAL("case")
    } break;
    case NUN_VALUE_ELSE: {
        DISPLAY_SPECIAL("else")
    } break;
    case NUN_VALUE_AND: {
        DISPLAY_SPECIAL("and")
    } break;
    case NUN_VALUE_OR: {
        DISPLAY_SPECIAL("or")
    } break;
    case NUN_VALUE_UNLESS: {
        DISPLAY_SPECIAL("unless")
    } break;
    case NUN_VALUE_COND_EXPAND: {
        DISPLAY_SPECIAL("cond-expand")
    } break;
    case NUN_VALUE_LET: {
        DISPLAY_SPECIAL("let")
    } break;
    case NUN_VALUE_LET_STAR: {
        DISPLAY_SPECIAL("let*")
    } break;
    case NUN_VALUE_LETREC: {
        DISPLAY_SPECIAL("letrec")
    } break;
    case NUN_VALUE_LETREC_STAR: {
        DISPLAY_SPECIAL("letrec*")
    } break;
    case NUN_VALUE_LET_VALUES: {
        DISPLAY_SPECIAL("let-values")
    } break;
    case NUN_VALUE_LET_VALUES_STAR: {
        DISPLAY_SPECIAL("let*-values")
    } break;
    case NUN_VALUE_BEGIN: {
        DISPLAY_SPECIAL("begin")
    } break;
    case NUN_VALUE_DO: {
        DISPLAY_SPECIAL("do")
    } break;
    case NUN_VALUE_LET_LOOP: {
        DISPLAY_SPECIAL("let")
    } break;
    case NUN_VALUE_DELAY: {
        DISPLAY_SPECIAL("delay")
    } break;
    case NUN_VALUE_DELAY_FORCE: {
        DISPLAY_SPECIAL("delay-force")
    } break;
    case NUN_VALUE_FORCE: {
        DISPLAY_SPECIAL("force")
    } break;
    case NUN_VALUE_MAKE_PROMISE: {
        DISPLAY_SPECIAL("make-promise")
    } break;
    case NUN_VALUE_MAKE_PARAMETER: {
        DISPLAY_SPECIAL("make-parameter")
    } break;
    case NUN_VALUE_PARAMETERIZE: {
        DISPLAY_SPECIAL("parameterize")
    } break;
    case NUN_VALUE_GUARD: {
        DISPLAY_SPECIAL("guard")
    } break;
    case NUN_VALUE_QUOTE: {
        DISPLAY_SPECIAL("quote")
    } break;
    case NUN_VALUE_QUASIQUOTE: {
        DISPLAY_SPECIAL("quasiquote")
    } break;
    case NUN_VALUE_UNQUOTE: {
        DISPLAY_SPECIAL("unquote")
    } break;
    case NUN_VALUE_UNQUOTE_SPLICING: {
        DISPLAY_SPECIAL("unquote-splicing")
    } break;
    case NUN_VALUE_CASE_LAMBDA: {
        DISPLAY_SPECIAL("case-lambda")
    } break;
    case NUN_VALUE_LET_SYNTAX: {
        DISPLAY_SPECIAL("let-syntax")
    } break;
    case NUN_VALUE_LETREC_SYNTAX: {
        DISPLAY_SPECIAL("letrec-syntax")
    } break;
    case NUN_VALUE_SYNTAX_RULES: {
        DISPLAY_SPECIAL("syntax-rules")
    } break;
    case NUN_VALUE_SYNTAX_ERROR: {
        DISPLAY_SPECIAL("syntax-error")
    } break;
    }
#undef DISPLAY_SPECIAL
}

size_t nun_display(char *dest, size_t len, NunValue *value) {
    struct DisplayParams params = {
        .count = 0,
        .indent = 0,
        .indent_char = " ",
        .newline_char = "\n",
    };
    nun_display_inner(dest, len - 1, &params, value);
    dest[params.count] = 0;
    return params.count;
}

bool nun_tree_list_eh(NunStree *tree) {
    if (tree->kind == NUN_STREE_NIL) {
        return true;
    }
    if (tree->kind != NUN_STREE_PAIR) {
        return false;
    }

    return nun_tree_list_eh(tree->vpair.cdr);
}

bool nun_value_list_eh(NunValue *value) {
    if (value->kind != NUN_VALUE_TREE) {
        return false;
    }

    return nun_tree_list_eh(value->vtree);
}

int main(int argc, const char **argv) {
    if (argc != 2) {
        fprintf(stderr, "nunsc:%s:%d: error: no input file\n", __FILE__, __LINE__);
        exit(1);
    }

    struct stat statbuf;
    int status = stat(argv[1], &statbuf);
    if (status < 0) {
        fprintf(stderr, "nunsc:%s:%d: error: %s\n", __FILE__, __LINE__, strerror(errno));
        exit(errno);
    }

    size_t len;
    const char *source;
    {
        FILE *fsrc = fopen(argv[1], "r");
        char *src = malloc(statbuf.st_size);
        len = fread(src, 1, statbuf.st_size, fsrc);
        if (len != (size_t) statbuf.st_size) {
            fprintf(stderr, "nunsc:%s:%d: error: %s\n", __FILE__, __LINE__, strerror(errno));
            exit(errno);
        }
        fclose(fsrc);
        source = src;
    }
    
    struct NunTokens tokens;

    if ((status = nun_lex(&tokens, source, len))) {
        exit(status);
    }

    NunGc gc;
    nun_new_gc(&gc, 1024 * 1024);

    NunValue *program;
    size_t proglen;
    if ((status = nun_parse(&program, &proglen, &gc, &tokens))) {
        exit(status);
    }

    for (size_t i = 0; i < proglen; i++) {
        const int cap = 1024;
        char buffer[cap];
        nun_display(buffer, cap, program + i);
        fprintf(stdout, "%.*s\n", cap, buffer);
    }

    nun_del_gc(&gc);
    
    return 0;
}
