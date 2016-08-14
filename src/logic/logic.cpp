#include "Logger.hpp"
#include "logic.hpp"
#include "physics/physics.hpp"
#include <cmath>

#define VAR(x) std::string(indent, ' ') << #x << ": " << (x) << "\n"

void print_type(CellType const &type, int indent)
{
    switch (type._tag)
    {
    case CellType::STEM_CELL:
    {
	std::string tag_str = "STEM_CELL";
	StemCell const &stem = type.stem_cell;
	bool attach_children = !stem.optional_child_attachment.empty;
	
	std::cout << VAR(tag_str) << VAR(attach_children)
		  << VAR(stem.min_split_mass) << VAR(stem.child0_amount)
		  << VAR(stem.children_types[0]) << VAR(stem.children_types[1]);
	for (int c = 0; c != 2; ++c)
	for (size_t i = 0; i != stem.passed_attachments[c].size(); ++i)
	    std::cout << std::string(indent, ' ') << "pass attachment "
		      << stem.passed_attachments[c][i] << " to child " << c << "\n";
	std::cout << VAR(stem.children_angles[0]) << VAR(stem.children_angles[1]);
	break;
    }
    case CellType::MUSCLE_CELL:
    {
	std::string tag_str = "MUSCLE_CELL";
	MuscleCellType const &muscle = type.muscle_cell;

	std::cout << VAR(tag_str);
	if (muscle.fix_input_attachment.empty)
	    std::cout << std::string(indent, ' ') << "no fix\n";
	else
	{
	    std::cout << std::string(indent, ' ') << "fix@"
		      << muscle.fix_input_attachment.value() << "\n";
	}

	for (size_t i = 0; i != muscle.control_inputs.size(); ++i)
	    std::cout << std::string(indent, ' ') << "attachment_distance@"
		      << muscle.control_inputs[i].output_attachment << " = "
		      << muscle.control_inputs[i].weight << " * charge@"
		      << muscle.control_inputs[i].input_attachment << "\n";
	break;
    }
    case CellType::NEURON_CELL:
    {
	std::string tag_str = "NEURON_CELL";
	NeuronCellType const &neuron = type.neuron_cell;

	std::string function_str;
	switch (neuron.function)
	{
	case NeuronCellType::STEP:
	    function_str = "STEP";
	    break;
	case NeuronCellType::SIGMOID:
	    function_str = "SIGMOID";
	    break;
	case NeuronCellType::LINEAR:
	    function_str = "LINEAR";
	    break;
	}

	std::cout << VAR(tag_str) << VAR(function_str) << VAR(neuron.threshold) << VAR(neuron.update_offset);

	for (size_t i = 0; i != neuron.inputs.size(); ++i)
	    std::cout << std::string(indent, ' ') << "input: charge@"
		      << neuron.inputs[i].attachment << " * " << neuron.inputs[i].weight << "\n";
	
	break;
    }
    }
}

void print_cell(Cell const &cell, int indent)
{   
    Body const &body = cell.body();
    std::cout << VAR(body.angle) << VAR(body.pos.x) << VAR(body.pos.y)
	      << VAR(body.room_x) << VAR(body.room_y)
	      << VAR(body.vel.x) << VAR(body.vel.y) << VAR(body.angle_vel)
	      << VAR(body.mass) << VAR(body.radius()) << VAR(body.fixed)
	      << VAR(cell.life_time) << VAR(cell.charge) << VAR(cell.neuron_next_update)
	      << VAR(cell.type_slot);
    for (size_t i = 0; i != cell.attachments.size(); ++i)
	if (!cell.attachments[i].empty)
	    std::cout << std::string(indent, ' ') << "attached to: "
		      << cell.attachments[i].value().other_cell << " @ " << i << "\n";
}

#undef VAR

void print_logic(LogicWorld *logic)
{
    logic->cell_types.iter().do_each([](CellType *type)
				{
				    std::cout << "CellType " << type << ":\n";
				    print_type(*type, 4);
				});
    logic->cells.iter().do_each([](Cell *cell)
				{
				    std::cout << "Cell " << cell << ":\n";
				    print_cell(*cell, 4);
				});
}

void init_logic_world(LogicWorld *logic, PhysicsWorld *physics)
{
    AttachmentConfig child_att = AttachmentConfig();
    child_att.delta_angle = std::nan("");
    child_att.distance = 0;
    child_att.strength = 1;
    
    CellType muscle_type(CellType::MUSCLE_CELL);
    {
	MuscleInput input = MuscleInput();
	input.weight = 2;
	input.input_attachment = 2;
	input.output_attachment = 1;
	muscle_type.muscle_cell.control_inputs.push_back(input);

	input.weight = 4;
	input.input_attachment = 2;
	input.output_attachment = 2;
	muscle_type.muscle_cell.control_inputs.push_back(input);
    }
    muscle_type.muscle_cell.fix_input_attachment = Optional<size_t>(1);
    Slot<CellType> *muscle_type_slot = logic->cell_types.add(muscle_type);

    CellType pass_neuron_type(CellType::NEURON_CELL);
    pass_neuron_type.neuron_cell.function = NeuronCellType::STEP;
    pass_neuron_type.neuron_cell.threshold = 0.9;
    pass_neuron_type.neuron_cell.update_offset = 2.5;
    {
	NeuronInput input = NeuronInput();
	input.attachment = 1;
	input.weight = 1;
	pass_neuron_type.neuron_cell.inputs.push_back(input);
    }
    Slot<CellType> *pass_neuron_type_slot = logic->cell_types.add(pass_neuron_type);

    CellType negate_neuron_type(CellType::NEURON_CELL);
    negate_neuron_type.neuron_cell.function = NeuronCellType::LINEAR;
    negate_neuron_type.neuron_cell.threshold = -1;
    {
	NeuronInput input = NeuronInput();
	input.attachment = 1;
	input.weight = -0.3;
	negate_neuron_type.neuron_cell.inputs.push_back(input);
    }
    Slot<CellType> *negate_neuron_type_slot = logic->cell_types.add(negate_neuron_type);

    CellType neurons_stem_cell_type(CellType::STEM_CELL);
    {
	StemCell &stem = neurons_stem_cell_type.stem_cell;
	stem.optional_child_attachment = optional(child_att);
	stem.passed_attachments[0].push_back(0);
	stem.passed_attachments[1].push_back(0);
	stem.children_types[0] = pass_neuron_type_slot;
	stem.children_types[1] = negate_neuron_type_slot;
    }
    Slot<CellType> *neurons_stem_cell_type_slot = logic->cell_types.add(neurons_stem_cell_type);
    
    CellType orig_type(CellType::STEM_CELL);
    {
	StemCell &stem_type = orig_type.stem_cell;
	stem_type.optional_child_attachment = Optional<AttachmentConfig>(child_att);
	stem_type.child0_amount = 1.0/4.0;
	stem_type.children_types[0] = muscle_type_slot;
	stem_type.children_types[1] = neurons_stem_cell_type_slot;
	stem_type.children_angles[1] = M_PI / 2;
    }
    Slot<CellType> *orig_type_slot = logic->cell_types.add(orig_type);

    Body first_body = Body();
    first_body.mass = 4;
    first_body.pos = glm::vec2(20, 20);
    Slot<Body> *first_body_slot = physics->bodies.add(first_body);
    
    Cell first_cell = Cell();
    first_cell.type_slot = orig_type_slot;
    first_cell.body_slot = first_body_slot;
    first_cell.life_time = 0;
    logic->cells.add(first_cell);
}

void kill_cell(Slot<Cell> *slot)
{
    assert(!slot->empty);
    
    // needed: shared_ptr
    slot->value().type_slot = 0;

    for (Optional<LogicAttachment> &att: slot->value().attachments)
    {
	if (att.empty)
	    continue;

	// remove the other side of the logical attachment
	int C = 0;
	for (Optional<LogicAttachment> &other: att.value().other_cell->attachments)
	    if (!other.empty && other.value().other_cell == &slot->value())
	    {
		other.empty = true;
		// the pun was worth it
		C++;	
	    }
	assert(C == 1);
	
	att.value().physics->empty = true;
    }
	

    slot->value().body_slot->empty = true;
    slot->empty = true;
}

bool are_cells_logic_attached(Cell *a, Cell *b)
{
    size_t occurences = iter(a->attachments)
	.filter([&](Optional<LogicAttachment> *const &la)
		{
		    if (la->empty)
			return false;
		    return la->value().other_cell == b;
		})
	.count();
    assert(occurences < 2);
    
    assert(iter(b->attachments)
	   .filter([&](Optional<LogicAttachment> *const &la)
		   {
		       if (la->empty)
			   return false;
		       return la->value().other_cell == a;
		   })
	   .count() == occurences);

    return occurences == 1;
}

void attach_cells(LogicWorld *, PhysicsWorld *physics,
		  Cell *cell0_ptr, Cell *cell1_ptr,
                  AttachmentConfig const &config)
{
    Cell &cell0 = *cell0_ptr;
    Cell &cell1 = *cell1_ptr;
    assert(cell0_ptr != cell1_ptr);
    assert(&cell0.body() != &cell1.body());
    assert(find_attachment(*physics, &cell0.body(),
			             &cell1.body()
	                  ).empty);
    assert(!are_cells_logic_attached(&cell0, &cell1));
    
    Attachment att = Attachment();
    att.config = config;
    att.bodies[0] = &cell0.body();
    att.bodies[1] = &cell1.body();
    Slot<Attachment> *att_slot = physics->attachments.add(att);

    LogicAttachment logatt = LogicAttachment();
    logatt.other_cell = &cell1;
    logatt.physics = att_slot;
    cell0.attachments.push_back(logatt);

    logatt.other_cell = &cell0;
    cell1.attachments.push_back(logatt);
}

void on_cell_create(LogicWorld *, PhysicsWorld *, Cell *cell)
{

    
    switch (cell->type()._tag)
    {
    case CellType::NEURON_CELL:
	cell->neuron_next_update = cell->type().neuron_cell.update_offset;
	break;
    default:
	break;
    }
}

void update_cell(LogicWorld *logic, PhysicsWorld *physics, Slot<Cell> *slot, float time)
{
    Cell &cell = slot->assert_value();
    cell.life_time+= time;
    CellType &cell_type = cell.type();
    
    switch (cell_type._tag)
    {
    case CellType::STEM_CELL:
    {
	float const split_cool_down = 3;
		
	StemCell &stem_cell = cell_type.stem_cell;
	float parent_mass = cell.body().mass;
       	if (cell.life_time > split_cool_down && parent_mass > stem_cell.min_split_mass)
	{
	    Slot<Cell> *children[2];
    	    for (int i = 0; i != 2; ++i)
	    {
		Body child_body = Body();
		child_body.angle = cell.body().angle + stem_cell.children_angles[i];
		child_body.angle_vel = 0;
		child_body.mass = abs((i - stem_cell.child0_amount) * parent_mass);
       		child_body.mass_per_radius = 1;
		glm::vec2 dir = glm::vec2(cos(cell.body().angle + 0.5 * M_PI * (i * 2 - 1)),
		                          sin(cell.body().angle + 0.5 * M_PI * (i * 2 - 1)))
		                * child_body.radius()
		                * 0.1f; // so that the cells have to repulse first, cool effect 
		child_body.pos = cell.body().pos + dir;
		child_body.vel = glm::vec2();
		assert(BodyRooms::no_negative_rooms);
		child_body.room_x = child_body.room_y = -1;
		Slot<Body> *child_body_slot = physics->bodies.add(child_body);

		Cell child_cell = Cell();
		child_cell.type_slot = stem_cell.children_types[i];
		child_cell.body_slot = child_body_slot;
		child_cell.life_time = 0;
		child_cell.attachments.reserve(stem_cell.passed_attachments[i].size() + 1);
		children[i] = logic->cells.add(child_cell);

		for (size_t passing_att: stem_cell.passed_attachments[i])
		{
		    if (passing_att >= cell.attachments.size())
			continue;
		    Optional<LogicAttachment> &parent_att = cell.attachments[passing_att];
		    if (parent_att.empty)
			continue;

		    attach_cells(logic, physics,
				 &children[i]->value(), parent_att.value().other_cell,
				 parent_att.value().physics->assert_value().config);
		}
	    }

	    if (!stem_cell.optional_child_attachment.empty)
		attach_cells(logic, physics, &children[0]->value(), &children[1]->value(),
			     stem_cell.optional_child_attachment.value());

	    // martyr mother commits suicide for her children :'(
	    kill_cell(slot);

	    on_cell_create(logic, physics, &children[0]->value());
	    on_cell_create(logic, physics, &children[1]->value());
	}
	break;    
    }
    case CellType::MUSCLE_CELL:
    {
	if (cell.charge != 0)
	    LOG_DEBUG("why tf am I charged?!?!");
	MuscleCellType &muscle = cell_type.muscle_cell;
	if (!muscle.fix_input_attachment.empty)
	    cell.attachment(muscle.fix_input_attachment.value())
	        .do_value([&](LogicAttachment &la)
	        {
	            cell.body().fixed = la.other_cell->charge > 0.5;
	        });
	iter(muscle.control_inputs)
	    .filter([&](MuscleInput *input)
		    {
			return !cell.attachment(input->input_attachment).empty
			    && !cell.attachment(input->output_attachment).empty;
		    })
	    .do_each([&](MuscleInput *input)
	    {
		Attachment &output = cell.attachment(input->output_attachment).value()
		    .physics->assert_value();
		output.config.distance = cell.attachment(input->input_attachment).value()
		    .other_cell->charge * input->weight;
		static float last_distance = -1;
		if (output.config.distance != last_distance)
		{
		    last_distance = output.config.distance;
		    // LOG_DEBUG("set distance to ", output.config.distance);
		}
	    });
	break;
    }
    case CellType::NEURON_CELL:
    {
        if (cell.body().fixed)
	    LOG_DEBUG("why tf am I fixed?!?!?!");
	cell.neuron_next_update-= time;
	if (cell.neuron_next_update < 0)
	{
	    cell.neuron_next_update = 1;

	    NeuronCellType &neuron = cell_type.neuron_cell;
	    float weighted_input = iter(neuron.inputs)
		.fold(0, [&](float acc, NeuronInput const* input)
		      {
			  if (input->attachment >= cell.attachments.size())
			      return acc;
			  return acc + input->weight *
			       cell.attachments[input->attachment]
			       .map([](LogicAttachment const &attachment)
			       {
				   return attachment.other_cell->charge;
			       }).value_or(0);    
		      });
	    weighted_input = 0;
	    for (size_t i = 0; i != neuron.inputs.size(); ++i)
	    {
		if (neuron.inputs[i].attachment >= cell.attachments.size())
		    continue;
		if (cell.attachments[neuron.inputs[i].attachment].empty)
		    continue;
		weighted_input+= neuron.inputs[i].weight
		    * cell.attachments[neuron.inputs[i].attachment].value().other_cell->charge;
	    }
	    switch (neuron.function)
	    {
	    case NeuronCellType::Function::STEP:
		cell.charge = weighted_input > neuron.threshold ? 1 : 0;
		break;
	    case NeuronCellType::Function::SIGMOID:
		cell.charge = 1 / (1 + pow(M_E, -(weighted_input - neuron.threshold)));
		break;
	    case NeuronCellType::Function::LINEAR:
		cell.charge = (weighted_input - neuron.threshold);
		break;
	    }
	}
	break;
    }
    }
}

void update_logic(LogicWorld *logic, PhysicsWorld *physics, float time)
{
    logic->cells.iter_nonempty_slots().do_each([&](Slot<Cell> *slot)
    {
        update_cell(logic, physics, slot, time);
    });
}
