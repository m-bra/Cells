
#include "logic.hpp"
#include "physics/physics.hpp"

void init_logic_world(LogicWorld *logic, PhysicsWorld *physics)
{
    
}

int get_empty_cell_slot(LogicWorld *world)
{
    for (int i = 0; i != (int) world->cells.size(); ++i)
    {
	if (world->cells[i].empty)
	{
	    return i;
	}
    }
    world->cells.resize(world->cells.size() + 1);
    world->cells[world->cells.size() - 1].empty = true;
    return world->cells.size() - 1;
}

void update_cell(LogicWorld *logic, PhysicsWorld physics, int cell_i, float time)
{
    assert(!logic->cells[cell_i].empty);
    Cell &cell = logic->cells[cell_i].value();
    cell.life_time+= time;
    
    switch (cell.type->tag)
    {
    STEM_CELL:
    {
	float const split_cool_down = 3;
		
	StemCell &stem_cell = cell.type->stem_cell;
	float parent_mass = cell.body->mass;
       	if (cell.life_time > split_cool_down && parent_mass > stem_cell.min_split_mass)
	{
	    float child0_mass = stem_cell.child0_amount * parent_mass;
	    float child1_mass = parent_mass - child0_mass;
	}
	break;    
    }
    }
}
