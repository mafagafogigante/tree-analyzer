#include <stdio.h>
#include <stdlib.h>

#include "BST.h"

#define DEFAULT_INDENT 2

/*
 * From this point onwards is defined the private interface of the Binary Search
 * Tree.
 */

/**
 * Allocates a new Binary Search Tree with the specified key and element in the
 * root node and null children.
 *
 * If memory allocation fails, NULL is returned.
 */
BST *create_node(Key key, Element element) {
  BST *tree;
  tree = malloc(sizeof(BST));
  if (tree != NULL) {
    tree->key = key;
    tree->data = element;
    tree->left = NULL;
    tree->right = NULL;
  }
  return tree;
}

/**
 * Removes the element with the smallest key from the tree and returns a
 * pointer to it.
 */
BST *detach_smallest(BST **root) {
  BST *iter = *root;
  BST *prev = *root;
  if (iter == NULL) {
    return NULL;
  }
  while (iter->left != NULL) {
    prev = iter;
    iter = iter->left;
  }
  /* If we did not walk, remove the tie from our parent. */
  prev->left = iter == prev ? NULL : iter->right;
  return iter;
}

/**
 * Removes the element with the greatest key from the tree and returns a
 * pointer to it.
 */
BST *detach_greatest(BST **root) {
  BST *iter = *root;
  BST *prev = *root;
  if (iter == NULL) {
    return NULL;
  }
  while (iter->right != NULL) {
    prev = iter;
    iter = iter->right;
  }
  /* If we did not walk, remove the tie from our parent. */
  prev->right = iter == prev ? NULL : iter->left;
  return iter;
}

void print_BST_indented(BST *tree, int level) {
  int i;
  if (tree->left != NULL) {
    print_BST_indented(tree->left, level + DEFAULT_INDENT);
  }
  for (i = 0; i < level; i++) {
    printf(" ");
  }
  print_key(tree->key);
  printf(" -> ");
  print_element(tree->data);
  printf("\n");
  if (tree->right != NULL) {
    print_BST_indented(tree->right, level + DEFAULT_INDENT);
  }
}

/*
 * From this point onwards is defined the public interface of the Binary Search
 * Tree.
 */

void BST_initialize(BST **root) { *root = NULL; }

void BST_free(BST **root) {
  BST *tree = *root;
  if (tree != NULL) {
    BST_free(&tree->left);
    BST_free(&tree->right);
    free(tree);
  }
  *root = NULL;
}

int BST_is_empty(BST *tree) { return tree == NULL; }

size_t BST_size(BST *tree) {
  size_t count = 0;
  if (tree != NULL) {
    count = 1;
    count += BST_size(tree->left);
    count += BST_size(tree->right);
  }
  return count;
}

static size_t BST_height(BST *tree) {
  size_t left_height;
  size_t right_height;
  size_t maximum_child_height;
  if (tree == NULL) {
    return 0;
  }
  left_height = BST_height(tree->left);
  right_height = BST_height(tree->right);
  if (left_height >= right_height) {
    maximum_child_height = left_height;
  } else {
    maximum_child_height = right_height;
  }
  return 1 + maximum_child_height;
}

static long BST_balance_factor(BST *tree) {
  long height_left;
  long height_right;
  long balance_here;
  long factor_here;
  long factor_left;
  long factor_right;
  if (tree == NULL) {
    return 0;
  }
  height_left = (long)BST_height(tree->left);
  height_right = (long)BST_height(tree->right);
  balance_here = labs(height_left - height_right);
  factor_left = BST_balance_factor(tree->left);
  factor_right = BST_balance_factor(tree->left);
  factor_here = balance_here;
  if (factor_left > factor_here) {
    factor_here = factor_left;
  }
  if (factor_left > factor_here) {
    factor_here = factor_right;
  }
  return factor_here;
}

void BST_update_report(Report *report, BST *tree) {
  report->nodes = BST_size(tree);
  report->height = BST_height(tree);
  report->factor = BST_balance_factor(tree);
}

/**
 * Returns whether or not the tree contains an element with the specified key.
 */
int BST_contains(Report *report, BST *tree, Key key) {
  return BST_get(report, tree, key) != NULL_ELEMENT;
}

/**
 * Returns the element associated with the specified key in the Binary Search
 * Tree or NULL_ELEMENT if the key is not present in the tree.
 */
Element BST_get(Report *report, BST *tree, Key key) {
  Element element = NULL_ELEMENT;
  while (tree != NULL) {
    report->comparisons++;
    if (tree->key == key) {
      element = tree->data;
      tree = NULL;
    } else if (key_less_than(key, tree->key)) {
      tree = tree->left;
    } else {
      tree = tree->right;
    }
  }
  return element;
}

/**
 * Inserts the (key, element) pair in the tree if it does not exist yet or
 * changes the element currently pointed by the key.
 *
 * This function is not recursive.
 */
void BST_insert(Report *report, BST **root, Key key, Element element) {
  while (*root != NULL) {
    report->comparisons++;
    if ((*root)->key == key) {
      /* Key collision, will replace element. */
      break;
    } else if (key_less_than(key, (*root)->key)) {
      root = &((*root)->left);
    } else {
      root = &((*root)->right);
    }
  }
  if (*root == NULL) {
    *root = create_node(key, element);
  } else {
    (*root)->data = element;
  }
}

void BST_remove(Report *report, BST **root, Key key) {
  BST *tree = *root;
  BST *left = NULL;
  BST *right = NULL;
  BST *replacement = NULL;
  while (tree != NULL) {
    report->comparisons++;
    if (tree->key == key) {
      if (tree->left != NULL) {
        replacement = detach_greatest(&tree->left);
      } else if (tree->right != NULL) {
        replacement = detach_smallest(&tree->right);
      }
      left = tree->left;
      right = tree->right;
      free(tree);
      if (replacement != NULL) {
        /* Update the replacement to fit its new position. */
        /* Ensure that we do not insert a cycle in the tree. */
        if (left != replacement) {
          replacement->left = left;
        } else {
          replacement->left = left->left;
        }
        if (right != replacement) {
          replacement->right = right;
        } else {
          replacement->right = right->right;
        }
      }
      *root = replacement;
      tree = NULL;
    } else if (key_less_than(key, tree->key)) {
      /* Propagate removal to the left child. */
      root = &tree->left;
      tree = *root;
    } else {
      /* Propagate removal to the right child. */
      root = &tree->right;
      tree = *root;
    }
  }
}

BST *lowest_common_ancestor(BST *tree, Key a, Key b) {
  if (tree == NULL) {
    return NULL;
  }
  if (tree->key != a && tree->key != b) {
    if (key_less_than(a, tree->key) && key_less_than(b, tree->key)) {
      return lowest_common_ancestor(tree->left, a, b);
    } else if (key_less_than(tree->key, a) && key_less_than(tree->key, b)) {
      return lowest_common_ancestor(tree->right, a, b);
    }
  }
  return tree;
}

void BST_print(BST *tree) {
  if (tree == NULL) {
    printf("Empty tree.\n");
  } else {
    print_BST_indented(tree, 0);
  }
}
