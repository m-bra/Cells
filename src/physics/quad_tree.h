#ifndef QUADTREE_H_INCLUDED
#define QUADTREE_H_INCLUDED

#include "util/vector.h"
#include "util/util.h"
#include "math.h"

typedef struct Leaf
{
    Vector bodies;
} Leaf;

typedef struct Node
{
   int is_leaf;
   union
   {
       Leaf leaf;
       /// [0]: small x, small y
       /// [1]: big x, small y
       /// [2]: small x, big y
       /// [3]: big x, big y
       /// see quad_child_index()
       struct Node *children;
   };
} Node;

typedef struct QuadTree
{
    /// The center of this node is (0,0), the width and height the other members.
    Node node;
    float width;
    float height;
    /// The depth/level in the tree where the bodies reside.
    /// There must not exist any node that is below that level
    /// There must not exist any body that is above that level
    int body_level;
} QuadTree;

Leaf *leaf_at(QuadTree *quad_tree, float x, float y);
Leaf *create_leaf_at(QuadTree *quad_tree, float x, float y);

/// Returns the index for the quarter of the position
/// This index is used for Node::children
/// negative x && negative y => index 0
/// positive x && negative y => index 1
/// negative x && positive y => index 2
/// positive x && positive y => index 3
/// 
/// If you have a node at (nx, ny) and a position at (x, y) and want to know its quarter index, use: quad_child_index(x-nx, y-ny)
static inline int quad_child_index(float x, float y)
{
    return (x > 0) + 2 * (y > 0);
}

typedef struct Rect {
    float center_x, center_y, half_width, half_height;
} Rect;

static inline Rect quad_child_rect(Rect rect, int quad_child_index)
{
    int y_factor = (quad_child_index / 2) * 2 - 1;
    int x_factor = (quad_child_index % 2) * 2 - 1;
    rect.center_x+= x_factor * rect.half_width / 2;
    rect.center_y+= y_factor * rect.half_height / 2;
    rect.half_width/= 2;
    rect.half_height/= 2;
    return rect;
}

/// When viewing a whole quadtree in one specific level you get a grid of (theoretical) leaves
/// Each of this leaf in this grid has a unique coordinate in respect to that grid.
/// This function returns this coordinate of the leaf that contains the given point (x,y)
/// Note that the leaf does not have to physically exist in memory. Its a "theoretical" coordinate.
/// The level this function operates on is quadtree->body_level
void pos_to_leaf_coords(QuadTree *quadtree, float x, float y, int *cx, int *cy)
{
    float level_w = quadtree->width / powi(2, quadtree->body_level);
    float level_h = quadtree->height / powi(2, quadtree->body_level);
    *cx = floorf(x / level_w);
    *cy = floorf(y / level_h);
}

#endif
