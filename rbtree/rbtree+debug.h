#include <stdio.h>

#include "rbtree.h"

void rbtree_print_dot_null(rbtree_node_t *key, int nullcount, FILE *stream, void (*PrintNodeFunc)(FILE *s, const rbtree_node_t *n));
void rbtree_print_dot_aux(rbtree_t *self, rbtree_node_t *node, FILE *stream, void (*PrintNodeFunc)(FILE *s, const rbtree_node_t *n));
void rbtree_print_dot(rbtree_t *self, FILE *stream, void (*PrintNodeFunc)(FILE *s, const rbtree_node_t *n));
