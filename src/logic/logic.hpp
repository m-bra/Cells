#ifndef LOGIC_HPP_INCLUDED
#define LOGIC_HPP_INCLUDED

#include <physics/physics.hpp>
#include "Optional.hpp"
#include "slots.hpp"
#include <map>
#include <memory>
#include <new>
#include <utility>

size_t constexpr MAX_CELLS = MAX_BODIES;
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
    std::vector<size_t> passed_attachments[2];
    /// the children will be positioned left and right to the parental cell
    /// (with respect to the parent angle)
    /// this member specifies the orientations of the children RELATIVE to the parental cell
    float children_angles[2] = {0, 0};
    Slot<struct CellType> *children_types[2];
    float min_split_mass = 0.7;
    /// How much of the parental mass the first child gets (the rest will be given to the other)
    /// relative statement; thus logically valid range is zero to one
    float child0_amount = 0.5;
};

struct MuscleInput
{
    /// the attachment to read the charge from
    size_t input_attachment;
    /// the attachment that is subject of modification of its distance
    size_t output_attachment;
    /// multiplier.
    float weight;
};

struct MuscleCellType
{
    /// The attachment to listen to.
    /// If the charge of the attached cell is near one, this MuscleCellType will fix the body.
    /// If the charge of the attached cell is near zero, this MuscleCellType will release.
    Optional<size_t> fix_input_attachment;
    /// this vector specifies which input charge attachments control which output attachments
    std::vector<MuscleInput> control_inputs; 
};

struct NeuronInput
{
    size_t attachment;
    float weight;
};

struct NeuronCellType
{
    enum Function
    {
	SIGMOID,
	STEP,
	LINEAR,
    } function = STEP;
    float threshold = 0.5;
    std::vector<NeuronInput> inputs;

    float update_offset = 0;
};

struct CellType
{
    enum CellTypeTag
    {
	STEM_CELL,
	MUSCLE_CELL,
	NEURON_CELL,
    } _tag;

    union
    {
	/// the stem cell will split into two new cells,	
	StemCell stem_cell;
	MuscleCellType muscle_cell;
	NeuronCellType neuron_cell;
    };

    CellType(): CellType(STEM_CELL) {}

    CellType(CellTypeTag a_tag)
    {
	_tag = a_tag;
	switch (_tag)
	{
	case STEM_CELL:
	    new (&stem_cell) StemCell();
	    break;
	case MUSCLE_CELL:
	    new (&muscle_cell) MuscleCellType();
	    break;
	case NEURON_CELL:
	    new (&neuron_cell) NeuronCellType();
	    break;
	}
    }

    ~CellType()
    {
	switch (_tag)
	{
	case STEM_CELL:
	    stem_cell.~StemCell();
	    break;
	case MUSCLE_CELL:
	    muscle_cell.~MuscleCellType();
	    break;
	case NEURON_CELL:
	    neuron_cell.~NeuronCellType();
	    break;
	}
    }

    CellType(CellType const &src)
    {
	_tag = src._tag;
	switch (_tag)
	{
	case STEM_CELL:
	    new (&stem_cell) StemCell(src.stem_cell);
	    break;
	case MUSCLE_CELL:
	    new (&muscle_cell) MuscleCellType(src.muscle_cell);
	    break;
	case NEURON_CELL:
	    new (&neuron_cell) NeuronCellType(src.neuron_cell);
	    break;
	}
    }

    CellType &operator =(CellType &&rhs)
    {
	this->~CellType();
	new (this) CellType(rhs);
	return *this;
    }
};

/// One-sided attachment reference.
struct LogicAttachment
{
    struct Cell *other_cell;
    Slot<Attachment> *physics;
};

struct Cell
{
    Slot<CellType> *type_slot;
    Slot<Body> *body_slot;
    // order matters. the attachment indices are used by stem_cell for attachment propagation
    // however, to be able to remove an attachment, the elements are optionals
    std::vector<Optional<LogicAttachment>> attachments;
    /// how long this cell is living now (seconds)
    float life_time = 0;
    /// Used to communicate (and, for neurons, compute) with other cells.
    /// Cells can read the charge of attached cells and thus read information.
    float charge = 0;

    float neuron_next_update = 0;

    Body &body()
    {
	return body_slot->assert_value();
    }

    Body const &body() const
    {
	return body_slot->assert_value();
    }

    CellType &type()
    {
	return type_slot->assert_value();
    }

    CellType const &type() const
    {
	return type_slot->assert_value();
    }

    Optional<LogicAttachment> attachment(size_t i)
    {
	if (i >= attachments.size())
	    return Optional<LogicAttachment>();
	return attachments[i];
    }
};

struct LogicWorld
{
    Slots<Cell, MAX_CELLS> cells;
    Slots<CellType, MAX_CELL_TYPES> cell_types;
};

void init_logic_world(LogicWorld *logic, PhysicsWorld *physics);
void update_logic(LogicWorld *logic, PhysicsWorld *physics, float time);

#endif
