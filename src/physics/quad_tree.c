#include "quad_tree.h"
#include "assert.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"


/// x, y: position to get
/// (x, y) assumed to be inside node rect
Leaf *rec_leaf_at(Node *node, Rect rect, float x, float y, int *depth)
{
    if (node->is_leaf)
        return node->leaf;
    else
    {
        int quarter = quad_child_index(x-rect.center_x, y-rect.center_y);
        ++*depth;
        return rec_leaf_at(&node->children[quarter], quad_child_rect(rect, quarter), x, y, depth, creation_level);
    }
}

/// x, y: position to create
/// (x, y) assumed to be inside node rect
///
/// creation_level: the level until which new leaves have to be created if the position is not deep enough
Leaf *rec_create_leaf_at(Node *node, Rect rect, float x, float y, int *depth, int creation_level)
{
    if (node->is_leaf)
    {
        if (*depth == creation_level)
            return &node->leaf; 
        else if (*depth < creation_level)
        {
            // if capacity == 0, we do not have to free() any data
            assert(node->leaf.bodies.capacity == 0);
            node->is_leaf = 0;
            node->children = calloc(4, sizeof(Node));
            node->children[0].is_leaf = 1;
            node->children[1].is_leaf = 1;
            node->children[2].is_leaf = 1;
            node->children[3].is_leaf = 1;

            int quarter = quad_child_index(x-rect.center_x, y-rect.center_y);
            ++*depth;
            return rec_create_leaf_at(&node->children[quarter], quad_child_rect(rect, quarter), x, y, depth, creation_level);
        }
        else
        {
            printf("ERROR: Quadtree: rec_create_leaf_at: Went below level to create leaf!");
            *depth = -1;
            return 0;
        }
    }
    else
    {
        int quarter = quad_child_index(x-rect.center_x, y-rect.center_y);
        ++*depth;
        return rec_create_leaf_at(&node->children[quarter], quad_child_rect(rect, quarter), x, y, depth, creation_level);
    }
}

Leaf *leaf_at(QuadTree *quad_tree, float x, float y)
{
    float hw = quad_tree->width / 2;
    float hh = quad_tree->height / 2;
    
    if (x < -hw || x > hw || y < -hh || y > hh)
        return 0;
    else {
        int depth; 
        Rect root_rect;
        root_rect.center_x = 0;
        root_rect.center_y = 0;
        root_rect.half_width = hw;
        root_rect.half_height = hh;

        Leaf *leaf = rec_leaf_at(&quad_tree->node, root_rect, x, y, &depth);
        assert(depth <= quad_tree->body_level);
        if (depth == quad_tree->body_level)
            return leaf;
        else
            return 0;
    }
}

Leaf *create_leaf_at(QuadTree *quad_tree, float x, float y)
{
    float hw = quad_tree->width / 2;
    float hh = quad_tree->height / 2;

    if (x > -hw && x < hw && y > -hh && y < hh)
    {
        int depth; 
        Rect root_rect;
        root_rect.center_x = 0;
        root_rect.center_y = 0;
        root_rect.half_width = hw;
        root_rect.half_height = hh;

        Leaf *leaf = rec_create_leaf_at(&quad_tree->node, root_rect, x, y, &depth, quad_tree->body_level);
        assert(depth == quad_tree->body_level);
        assert(leaf);
        return leaf;
    }
    else
    {
        assert(quad_tree->node.is_leaf);
        Node old_nodes[4];
        memcpy(old_nodes, quad_tree->node.children, 4 * sizeof (Node));
        quad_tree->width = 2 * 2 * hw;
        quad_tree->height = 2 * 2 * hh;

        Node *child = &quad_tree->node.children[0];
        child->is_leaf = 0;
        child->children = calloc(4, sizeof (Node));
        child->children[0].is_leaf = 1;
        child->children[1].is_leaf = 1;
        child->children[2].is_leaf = 1;
        child->children[3] = old_nodes[0];
        
        child = &quad_tree->node.children[1];
        child->is_leaf = 0;
        child->children = calloc(4, sizeof (Node));
        child->children[0].is_leaf = 1;
        child->children[1].is_leaf = 1;
        child->children[2] = old_nodes[1];
        child->children[3].is_leaf = 1;

        child = &quad_tree->node.children[2];
        child->is_leaf = 0;
        child->children = calloc(4, sizeof (Node));
        child->children[0].is_leaf = 1;
        child->children[1] = old_nodes[2];
        child->children[2].is_leaf = 1;
        child->children[3].is_leaf = 1;

        child = &quad_tree->node.children[3];
        child->is_leaf = 0;
        child->children = calloc(4, sizeof (Node));
        child->children[0] = old_nodes[3];
        child->children[1].is_leaf = 1;
        child->children[2].is_leaf = 1;
        child->children[3].is_leaf = 1;
        return create_leaf_at(quad_tree, x, y);
    }
}
