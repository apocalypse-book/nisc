#include "include/nunsc.h"

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

size_t nun_tree_list_length_inner(NunStree *tree, size_t acc) {
    if (tree->kind == NUN_STREE_NIL) {
        return acc;
    }
    if (tree->kind != NUN_STREE_PAIR) {
        return 0;
    }

    return nun_tree_list_length_inner(tree->vpair.cdr, acc + 1);
}
    
size_t nun_tree_list_length(NunStree *tree) {
    return nun_tree_list_length_inner(tree, 0);
}

size_t nun_value_list_length(NunValue *value) {
    if (value->kind != NUN_VALUE_TREE) {
        return 0;
    }

    return nun_tree_list_length(value->vtree);
}
