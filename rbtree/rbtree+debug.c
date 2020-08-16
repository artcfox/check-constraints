/*

  rbtree+debug.c

  Adds debugging methods based on code by Eli Bendersky to the
  Red-Black Tree implementation that generate .dot files which can be
  used to visualize the tree.

  To turn the .dot files into a .png file use the command:
    dot -Tpng input.dot -o output.png

  Copyright 2020 Matthew T. Pandina. All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY MATTHEW T. PANDINA "AS IS" AND ANY
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MATTHEW T. PANDINA OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
  USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  SUCH DAMAGE.

*/

#include "rbtree+debug.h"

void rbtree_print_dot_null(rbtree_node_t *key, int nullcount, FILE *stream)
{
  fprintf(stream, "    \"null%d\" [shape=point];\n", nullcount);
  fprintf(stream, "    \"%p\" -> \"null%d\";\n", key, nullcount);
}

void rbtree_print_dot_aux(rbtree_t *self, rbtree_node_t *node, FILE *stream)
{
  static int nullcount = 0;

  if (node->left != self->nil) {
    fprintf(stream, "    \"%p\" -> \"%p\";\n", node/*->key*/, node->left/*->key*/);
    rbtree_print_dot_aux(self, node->left, stream);
  }
  else
    rbtree_print_dot_null(node/*->key*/, nullcount++, stream);

  if (node->right != self->nil) {
    fprintf(stream, "    \"%p\" -> \"%p\";\n", node/*->key*/, node->right/*->key*/);
    rbtree_print_dot_aux(self, node->right, stream);
  }
  else
    rbtree_print_dot_null(node/*->key*/, nullcount++, stream);
}

void rbtree_print_dot(rbtree_t *self, FILE *stream)
{
  fprintf(stream, "digraph BST {\n");
  fprintf(stream, "    node [fontname=\"Arial\"];\n");

  if (self->root == self->nil)
    fprintf(stream, "\n");
  else if (self->root->right == self->nil && self->root->left == self->nil)
    fprintf(stream, "    \"%p\";\n", self->root/*->key*/);
  else
    rbtree_print_dot_aux(self, self->root, stream);

  fprintf(stream, "}\n");
}
