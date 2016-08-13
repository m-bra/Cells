
#include "logic.hpp"
#include "physics/physics.hpp"

void init_logic_world(LogicWorld *logic, PhysicsWorld *physics)
{
    
}

void kill_cell(Slot<Cell> *slot)
{
    assert(!slot->empty);
    
    // needed: shared_ptr
    slot->value().type = 0;

    for (Optional<LogicAttachment> &att: slot->value().attachments)
    {
	if (att.empty)
	    continue;

	// remove the other side of the logical attachment
	int C = 0;
	for (Optional<LogicAttachment> &other: att.value().other_cell->attachments)
	    if (other.value().other_cell == &slot->value())
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
    size_t occurences = !iter(a->attachments)
	.filter([&](Optional<LogicAttachment> const &la)
		{
		    if (la.empty)
			return false;
		    return la.value().other_cell == b;
		})
	.count();
    assert(occurences < 2);
    
    assert(iter(b->attachments)
	   .filter([&](Optional<LogicAttachment> const &la)
		   {
		       if (la.empty)
			   return false;
		       return la.value().other_cell == a;
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
    assert(find_attachment(*physics, &cell0.body(),
			             &cell1.body()
	                  ).empty);
    assert(!are_cells_logic_attached(&cell0, &cell1));
    
    Attachment att;
    att.config = config;
    att.bodies[0] = &cell0.body();
    att.bodies[1] = &cell1.body();
    Slot<Attachment> *att_slot = physics->attachments.add(att);

    LogicAttachment logatt;
    logatt.other_cell = &cell1;
    logatt.physics = att_slot;
    cell0.attachments.push_back(logatt);

    logatt.other_cell = &cell0;
    cell1.attachments.push_back(logatt);
}

void update_cell(LogicWorld *logic, PhysicsWorld *physics, Slot<Cell> *slot, float time)
{
    assert(!slot->empty);
    Cell &cell = slot->value();
    cell.life_time+= time;
    
    switch (cell.type->tag)
    {
    case CellType::STEM_CELL:
    {
	float const split_cool_down = 3;
		
	StemCell &stem_cell = cell.type->stem_cell;
	float parent_mass = cell.body().mass;
       	if (cell.life_time > split_cool_down && parent_mass > stem_cell.min_split_mass)
	{
	    Slot<Cell> *children[2];
    	    for (int i = 0; i != 2; ++i)
	    {
		Body child_body;
		child_body.angle = cell.body().angle + stem_cell.children_angles[i];
		child_body.angle_vel = 0;
		child_body.mass = abs((i - stem_cell.child0_amount) * parent_mass);
		child_body.mass_per_radius = 1;
		glm::vec2 dir = glm::vec2(cos(cell.body().angle + M_PI * (i * 2 - 1)),
		                          sin(cell.body().angle + M_PI * (i * 2 - 1)))
		                * child_body.radius();
		child_body.pos = cell.body().pos + dir;
		child_body.vel = glm::vec2();
		assert(BodyRooms::no_negative_rooms);
		child_body.room_x = child_body.room_y = -1;
		Slot<Body> *child_body_slot = physics->bodies.add(child_body);

		Cell child_cell;
		child_cell.type = stem_cell.children_types[i];
		child_cell.body_slot = child_body_slot;
		child_cell.life_time = 0;
		child_cell.attachments.reserve(stem_cell.passed_attachments[i].size() + 1);
		children[i] = logic->cells.add(child_cell);

		for (int passing_att: stem_cell.passed_attachments[i])
		{
		    if (passing_att >= (int)cell.attachments.size())
			continue;
		    Optional<LogicAttachment> &parent_att = cell.attachments[i];
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
	}
	break;    
    }
    }
}
