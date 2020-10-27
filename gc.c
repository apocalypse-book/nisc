#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "include/nunsc.h"

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
