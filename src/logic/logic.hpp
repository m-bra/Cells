#ifndef LOGIC_HPP_INCLUDED
#define LOGIC_HPP_INCLUDED

#include <physics/physics.hpp>
#include "Optional.hpp"
#include "slots.hpp"
#include <map>

#define MAX_CELLS 500
#define MAX

struct StemCell
{
    Optional<AttachmentConfig> optional_child_attachment;
    /// this vector holds the ids of the parental attachments are passed to the child
    /// the ids of the attachments are retained when passed onto the child, and if
    /// a new attachment is created, its id is the smallest number that is not already
    /// used for this child (thus ids are local, not global!)
    std::vector<int> passed_attachments;
    /// the children will be positioned left and right to the parental cell
    /// (with respect to the parent angle)
    /// this member specifies the orientations of the children RELATIVE to the parental cell
    float children_angles[2];
    struct CellType *children_types[2];
    float min_split_mass;
    /// How much of the parental mass the first child gets (the rest will be given to the other)
    /// relative statement; thus logically valid range is zero to one
    float child0_amount;
};

struct CellType
{
    enum
    {
	STEM_CELL,
    } tag;

    union
    {
	/// the stem cell will split into two new cells,	
	StemCell stem_cell;
    };
};

struct Cell
{
    CellType *type;
    Slot<Body> *body;
    /// how long this cell is living now (seconds)
    float life_time;
};

struct LogicWorld
{
    /// order matters.
    Slots<Cell, MAX_CELLS> cells;
    /// The cell types inside this vector reference each other.
    /// If a cell type is referenced by no cell and no other cell type
    /// then that cell type is leaked. garbage. dead.
    std::vector<CellType> cell_types;

    
};

#endif
