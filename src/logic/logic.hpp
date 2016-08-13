#ifndef LOGIC_HPP_INCLUDED
#define LOGIC_HPP_INCLUDED

#include <physics/physics.hpp>
#include "Optional.hpp"
#include "slots.hpp"
#include <map>
#include <memory>

#define MAX_CELLS 500
#define MAX_LOGIC_ATTACHMENTS 500
#define MAX_CELL_TYPES 50

struct StemCell
{
    Optional<AttachmentConfig> optional_child_attachment;
    /// this vector holds the indices of the parental attachments are passed to the child
    /// the indices of the attachments are retained when passed onto the child, and if
    /// a new attachment is created (->optional_child_attachment),
    /// its index is the smallest number that is not already
    /// used for this child (thus ids are local, not global!)
    /// Now, there are two vectors, for each child separately.
    std::vector<int> passed_attachments[2];
    /// the children will be positioned left and right to the parental cell
    /// (with respect to the parent angle)
    /// this member specifies the orientations of the children RELATIVE to the parental cell
    float children_angles[2];
    std::shared_ptr<struct CellType> children_types[2];
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

/// One-sided attachment reference.
struct LogicAttachment
{
    struct Cell *other_cell;
    Slot<Attachment> *physics;
};

struct Cell
{
    std::shared_ptr<CellType> type;
    Slot<Body> *body_slot;
    // order matters. the attachment indices are used by stem_cell for attachment propagation
    // however, to be able to remove an attachment, the elements are optionals
    std::vector<Optional<LogicAttachment>> attachments;
    /// how long this cell is living now (seconds)
    float life_time;

    Body &body()
    {
	return body_slot->value();
    }
};

struct LogicWorld
{
    Slots<Cell, MAX_CELLS> cells;
};

#endif
