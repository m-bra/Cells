
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
		
		for (int passing_att: stem_cell.passed_attachments[i])
		{
		    if (passing_att >= (int)cell.attachments.size())
			continue;
		    Optional<LogicAttachment> &parent_att = cell.attachments[i];
		    if (parent_att.empty)
			continue;
		    Attachment phyatt;
		    phyatt.config = parent_att.value().physics->value().config;
		    phyatt.bodies[0] = &child_body_slot->value();
		    phyatt.bodies[1] = &parent_att.value().other_cell->body();
		    Slot<Attachment> *att_slot = physics->attachments.add(phyatt);

		    LogicAttachment child_att;
		    child_att.other_cell = parent_att.value().other_cell;
		    child_att.physics = att_slot;
		    child_cell.attachments.push_back(Optional<LogicAttachment>(child_att));
		}
		children[i] = logic->cells.add(child_cell);
	    }

	    if (!stem_cell.optional_child_attachment.empty)
	    {
		Attachment phyatt;
		phyatt.config = stem_cell.optional_child_attachment.value();
		phyatt.bodies[0] = &children[0]->value().body();
		phyatt.bodies[1] = &children[1]->value().body();
		Slot<Attachment> *att_slot = physics->attachments.add(phyatt);

	        for (int i = 0; i != 2; ++i)
		{
		    LogicAttachment att;
		    att.other_cell = &children[1 - i]->value();
		    att.physics = att_slot;
		    children[i]->value().attachments.push_back(Optional<LogicAttachment>(att));
		}
	    }

	    // martyr mother commits suicide for her children :'(
	    kill_cell(slot);
	}
	break;    
    }
    }
}
