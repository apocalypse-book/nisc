#include "include/nisc.h"

bool nis_tree_list_eh(NisStree *tree) {
    if (tree->kind == NIS_STREE_NIL) {
        return true;
    }
    if (tree->kind != NIS_STREE_PAIR) {
        return false;
    }

    return nis_tree_list_eh(tree->vpair.cdr);
}

bool nis_value_list_eh(NisValue *value) {
    if (value->kind != NIS_VALUE_TREE) {
        return false;
    }

    return nis_tree_list_eh(value->vtree);
}

size_t nis_tree_list_length_inner(NisStree *tree, size_t acc) {
    if (tree->kind == NIS_STREE_NIL) {
        return acc;
    }
    if (tree->kind != NIS_STREE_PAIR) {
        return 0;
    }

    return nis_tree_list_length_inner(tree->vpair.cdr, acc + 1);
}
    
size_t nis_tree_list_length(NisStree *tree) {
    return nis_tree_list_length_inner(tree, 0);
}

size_t nis_value_list_length(NisValue *value) {
    if (value->kind != NIS_VALUE_TREE) {
        return 0;
    }

    return nis_tree_list_length(value->vtree);
}
