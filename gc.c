#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "include/nisc.h"

void nis_new_gc(NisGc *dest, size_t capacity) {
    dest->buffer = malloc(capacity);
    dest->len = sizeof(struct NisAllocFrame);
    dest->capacity = capacity;
    dest->prev = dest->buffer;
    dest->prev->next = NULL;
    dest->prev->len = capacity;

    dest->nil = nis_alloc(dest, sizeof(NisStree));
    dest->nil->kind = NIS_STREE_NIL;
    dest->nil->flags = 0;
    dest->nil->next = NULL;
    
    dest->t = nis_alloc(dest, sizeof(NisStree));
    dest->t->kind = NIS_STREE_TRUE;
    dest->t->flags = 0;
    dest->t->next = NULL;
    
    dest->f = nis_alloc(dest, sizeof(NisStree));
    dest->f->kind = NIS_STREE_FALSE;
    dest->f->flags = 0;
    dest->f->next = NULL;

    dest->last = NULL;
    dest->allocated = 0;
    dest->threshold = capacity / sizeof(NisStree) >> 3;
}

void nis_del_gc(NisGc *dest) {
    free(dest->buffer);
}

void *nis_alloc(NisGc *gc, size_t size) {
    if (size == 0) {
        return NULL;
    }
    
    size = nis_align_up(size, 8);
    if (gc->len + size >= gc->capacity) {
        fprintf(stderr, "nisc:%s:%d: error: out of memory\n", __FILE__, __LINE__);
        exit(1);
    }

    struct NisAllocFrame **prev = &gc->prev;
    struct NisAllocFrame *frame = gc->prev;
    while (frame) {
        if (frame->len >= size) {
            if (frame->len - size < sizeof(struct NisAllocFrame)) {
                void *ptr = frame;
                *prev = frame->next;
                return ptr;
            } else {
                void *ptr = frame;
                struct NisAllocFrame *nextptr = (void *) ((char *) ptr + size);
                struct NisAllocFrame *nextnext = frame->next;
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
    
    fprintf(stderr, "nisc:%s:%d: error: out of memory\n", __FILE__, __LINE__);
    exit(1);
}

void nis_dealloc(NisGc *gc, void *ptr, size_t size) {
    size = nis_align_up(size, 8);
    
    struct NisAllocFrame **prev = &gc->prev;
    struct NisAllocFrame *frame = gc->prev;
    while (frame) {
        if (frame == ptr) {
            fprintf(stderr, "nisc:%s:%d: error: double free\n", __FILE__, __LINE__);
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
    
    fprintf(stderr, "nisc:%s:%d: error: invalid free\n", __FILE__, __LINE__);
    exit(1);
}

void *nis_realloc(NisGc *gc, void *ptr, size_t oldsize, size_t newsize) {
    oldsize = nis_align_up(oldsize, 8);
    newsize = nis_align_up(newsize, 8);

    if (newsize == oldsize) {
        return ptr;
    } else if (newsize > oldsize) {
        struct NisAllocFrame *frame = gc->prev;
        while (frame) {
            struct NisAllocFrame *next = frame->next;
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
                    void *newptr = nis_alloc(gc, newsize);
                    memcpy(newptr, oldptr, oldsize);
                    nis_dealloc(gc, oldptr, oldsize);
                    return newptr;
                }
            }
            frame = next;
        }
    }
    
    fprintf(stderr, "nisc:%s:%d: error: invalid free\n", __FILE__, __LINE__);
    exit(1);
}

void nis_false(NisValue *dest, NisGc *gc) {
    (void) gc;
    dest->kind = NIS_VALUE_FALSE;
}

void nis_true(NisValue *dest, NisGc *gc) {
    (void) gc;
    dest->kind = NIS_VALUE_TRUE;
}

void nis_int(NisValue *dest, NisGc *gc, long value) {
    (void) gc;
    dest->kind = NIS_VALUE_INT;
    dest->vint = value;
}

void nis_float(NisValue *dest, NisGc *gc, double value) {
    (void) gc;
    dest->kind = NIS_VALUE_FLOAT;
    dest->vfloat = value;
}

void nis_stree(NisValue *dest, NisGc *gc, NisStree *value) {
    (void) gc;
    dest->kind = NIS_VALUE_TREE;
    dest->vtree = value;
}

void nis_nil(NisValue *dest, NisGc *gc) {
    dest->kind = NIS_VALUE_TREE;
    dest->vtree = gc->nil;
}

void nis_pair(NisValue *dest, NisGc *gc, NisValue *car, NisValue *cdr) {
    dest->kind = NIS_VALUE_TREE;
    NisStree *tree = nis_alloc(gc, sizeof(NisStree));
    tree->kind = NIS_STREE_PAIR;
    tree->flags = 0;
    tree->next = gc->last;
    tree->vpair.car = nis_value_to_stree(gc, car);
    tree->vpair.cdr = nis_value_to_stree(gc, cdr);
    dest->vtree = tree;
    gc->last = tree;
}

void nis_vector(NisValue *dest, NisGc *gc) {
    dest->kind = NIS_VALUE_TREE;
    NisStree *tree = nis_alloc(gc, sizeof(NisStree));
    tree->kind = NIS_STREE_PAIR;
    tree->flags = 0;
    tree->next = gc->last;
    tree->vvec.ptr = NULL;
    tree->vvec.len = 0;
    tree->vvec.cap = 0;
    dest->vtree = tree;
    gc->last = tree;
}

void nis_byte_vector(NisValue *dest, NisGc *gc) {
    dest->kind = NIS_VALUE_TREE;
    NisStree *tree = nis_alloc(gc, sizeof(NisStree));
    tree->kind = NIS_STREE_PAIR;
    tree->flags = 0;
    tree->next = gc->last;
    tree->vbvec.ptr = NULL;
    tree->vbvec.len = 0;
    tree->vbvec.cap = 0;
    dest->vtree = tree;
    gc->last = tree;
}

void nis_atom(NisValue *dest, NisGc *gc, const char *value) {
    size_t len = strlen(value);
    
    dest->kind = NIS_VALUE_TREE;
    NisStree *tree = nis_alloc(gc, sizeof(NisStree));
    if (len < 24) {
        tree->kind = NIS_STREE_ATOM;
        tree->flags = NIS_FLAG_INLINE;
        tree->next = gc->last;
        strcpy((char *) tree->vinline, value);
    } else {
        tree->kind = NIS_STREE_ATOM;
        tree->flags = NIS_FLAG_INLINE;
        tree->next = gc->last;
        tree->vatom = nis_alloc(gc, len + 1);
        strcpy((char *) tree->vatom, value);
    }
    dest->vtree = tree;
    gc->last = tree;
}

void nis_special(NisValue *dest, NisGc *gc, int value) {
    (void) gc;
    dest->kind = value;
}

NisStree *nis_value_to_stree(NisGc *gc, NisValue *value) {
    switch (value->kind) {
    case NIS_VALUE_TRUE:
        return gc->t;
    case NIS_VALUE_FALSE:
        return gc->f;
    case NIS_VALUE_INT: {
        NisStree *tree = nis_alloc(gc, sizeof(NisStree));
        tree->kind = NIS_STREE_INT;
        tree->flags = 0;
        tree->next = gc->last;
        tree->vint = value->vint;
        gc->last = tree;
        return tree;
    }
    case NIS_VALUE_FLOAT: {
        NisStree *tree = nis_alloc(gc, sizeof(NisStree));
        tree->kind = NIS_STREE_FLOAT;
        tree->flags = 0;
        tree->next = gc->last;
        tree->vfloat = value->vfloat;
        gc->last = tree;
        return tree;
    }
    case NIS_VALUE_TREE:
        return value->vtree;
    default:
        fprintf(stderr, "nisc:%s:%d: error: undefined value\n", __FILE__, __LINE__);
        exit(-1);
    }
}
