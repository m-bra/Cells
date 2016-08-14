#include "Logger.hpp"
#include <glm/glm.hpp>
#include <cmath>
#include "physics.hpp"
#include <algorithm>
#include <vector>
#include "Iterator.hpp"

/// Apply a spring force between two bodies that either only repulses or attracts (controlled by boolean "repulsion")
/// repulsion == 1: only repulse. repulsion == 0: repulse && attach. (according to distance)
/// "distance" is the preferred distance between the two bodies.
/// for repulsion, all distances below the prefered result in correction force.
/// for repulsion, all distances above the prefered result in correction force.
/// correction force for a distance of exactly one is "base_force"
void apply_spring_force(Body *body0, Body *body1, 
        float distance, float base_force, int repulsion, 
        float time)
{   
    glm::vec2 sub = body1->pos - body0->pos;
    float dist = glm::length(sub);
    float stretch_factor = 
        dist - body0->radius() - body1->radius() - distance;
    if (repulsion)
        stretch_factor = fminf(stretch_factor, 0);    
    float force = base_force * stretch_factor;

    if (false && force != 0)
    {
	std::string tag;
	if (repulsion) tag = "[repulsion]";
	else tag = "[attachment]";
	LOG_DEBUG(tag, " dist: ", dist, " distance: ", distance,
		  " radii: ", body0->radius(), " ", body1->radius(),
		  " stretch_factor: ", stretch_factor, " force: ", force, " time: ", time);
    }
    
    glm::vec2 force_vec;
    if (dist > 0.1)
	force_vec = sub / dist * force;
    else
	force_vec = glm::vec2(1.f, 0.f) * force;
    body0->vel+= force_vec * (time / body0->mass);
    body1->vel-= force_vec * (time / body1->mass);
}

/// applies torque on both bodies to achieve the target delta angle
void apply_angle_force(Body *body0, Body *body1,
		       float target_delta_angle,
		       float force_per_error, float time)
{
    float error = (body1->angle - target_delta_angle) - body0->angle;
    float correction = error * force_per_error;
    body0->angle_vel+= correction * time / body0->mass;
    body1->angle_vel-= correction * time / body1->mass;
}

void apply_attachment_forces(PhysicsWorld *world, float time, float base_force)
{    
    world->attachments.iter().do_each(
	[&](Attachment *attachment)
	{	    
	    Body *body0 = &*attachment->bodies[0];
	    Body *body1 = &*attachment->bodies[1];
 	    apply_spring_force(
                body0, body1, 
                attachment->config.distance, 
                base_force * attachment->config.strength, 
                0, time);
	    if (!std::isnan(attachment->config.delta_angle))
		apply_angle_force(
		    body0, body1,
		    attachment->config.delta_angle,
		    base_force / 2, time);			 
	});
}


void apply_damping(PhysicsWorld *world, float decay_per_second, float time)
{
    world->bodies.iter().do_each([&](Body *body)
        {
	    body->vel*= pow(decay_per_second, time);
	    body->angle_vel*= pow(decay_per_second, time);		    
        });
}

void calc_body_room(PhysicsWorld *world, Body *body, int *room_x, int *room_y)
{
    *room_x = floorf(body->pos[0] / world->body_rooms.room_width);
    *room_y = floorf(body->pos[1] / world->body_rooms.room_height);
    if (*room_x < 0) *room_x = 0;
    if (*room_x >= ROOMS_X) *room_x = ROOMS_X - 1;
    if (*room_y < 0) *room_y = 0;
    if (*room_y >= ROOMS_Y) *room_y = ROOMS_Y - 1;
}

/// The given pointer to the body will be added to the BodyRooms.
/// Thus, when the given body is meant to already be inside BodyRooms,
/// it has to be stored there with the same pointer!
void update_body_room(PhysicsWorld *world, Body *body)
{
    BodyRooms *rooms = &world->body_rooms;
    
    int room_x, room_y;
    calc_body_room(world, body, &room_x, &room_y);

    if (room_x != body->room_x || room_y != body->room_y)
    {
	std::vector<Body *> &old_room = rooms->rooms[body->room_x][body->room_y];
	std::vector<Body *>::iterator i = std::find(old_room.begin(), old_room.end(), body);
        if (i != old_room.end())
	{
	    std::swap(*i, old_room.back());
	    old_room.pop_back();
	}

        rooms->rooms[room_x][room_y].push_back(body);

        body->room_x = room_x;
        body->room_y = room_y;
    }
}

void update_all_body_rooms(PhysicsWorld *world)
{
    world->bodies.iter().do_each([&](Body *body) {update_body_room(world, &*body);});
}

/// i=0: small x
/// 1: big x
/// 2: small y
/// 3: big y
float room_edge(float width, float height, int dir)
{
    return (dir % 2) * (width * (1 - dir/2) + height * (dir / 2));
}

/// given: two room coordinates and a direction
/// outputs: the neighbor of that room
/// meaning of dir: ->room_edge
void neighbor_room(int *room_x, int *room_y, int dir)
{
    *room_x+= (1 - dir/2) * (dir * 2 - 1);
    *room_y+= (dir/2) * ((dir - 2) * 2 - 1);
}

void apply_repulsion_forces(PhysicsWorld *world, float base_force, float time)
{
    BodyRooms *rooms = &world->body_rooms;
    for (int i = 0; i < ROOMS_X; ++i)
    for (int j = 0; j < ROOMS_Y; ++j)
    {
	std::vector<Body *> &bodies = rooms->rooms[i][j];
        for (std::vector<Body *>::iterator body = bodies.begin(); body != bodies.end(); ++body)
        {
            // Apply on all bodies in the same room
            for (std::vector<Body *>::iterator other = bodies.begin(); other != bodies.end(); ++other)
            {
                if (other == body)
                    continue;
                apply_spring_force(
                        *body, *other,
                        0, base_force, 1, time); 
            }

            // apply on bodies in neighboring rooms IF this body touches other rooms
            // dir=0: small x
            // 1: big x
            // 2: small y
            // 3: big y
            for (int dir = 0; dir < 4; ++dir)
            {
                float coord = (*body)->pos[dir / 2];
                float abs_room_edge = coord + 
                    room_edge(rooms->room_width, rooms->room_height, dir);
                if (fabsf(coord - abs_room_edge) < (*body)->radius())
                {
                    int other_room_x = i;
                    int other_room_y = j;
                    neighbor_room(&other_room_x, &other_room_y, dir);

                    if (other_room_x >= 0 && other_room_x < ROOMS_X && other_room_y >= 0 && other_room_y < ROOMS_Y)
                    {
			std::vector<Body *> &bodies_other_room =
                            rooms->rooms[other_room_x][other_room_y];
                        for (std::vector<Body *>::iterator other = bodies_other_room.begin(); other != bodies_other_room.end(); ++other)
                            apply_spring_force(
                                *body, *other,
                                0, base_force, 1, time);
                    }
                    
                }
            }
        }
    }
}

void ensure_inside_bounds(float left, float bottom, float right, float top, Body *body)
{
    if (body->pos[0] < left)
        body->pos[0] = left;
    if (body->pos[0] > right)
        body->pos[0] = right;
    if (body->pos[1] < bottom)
        body->pos[1] = bottom;
    if (body->pos[1] > top)
        body->pos[1] = top;
}

void apply_velocities(PhysicsWorld *world, float time)
{
    auto room_width = world->body_rooms.room_width;
    auto room_height = world->body_rooms.room_height;

    world->bodies.iter().do_each([&](Body *body)
        {
	    if (!body->fixed)
	    {
		body->pos+= body->vel * time;
		body->angle+= body->angle_vel * time;
		ensure_inside_bounds(0, 0, room_width * ROOMS_X, room_height * ROOMS_Y, &*body);             }
	});
}

void init_physics(PhysicsWorld *world)
{
    world->body_rooms.room_width = world->body_rooms.room_height = 5;
}

void update_physics(PhysicsWorld *world, float elapsed_time)
{
    float base_repulsion_force = 2 / 1;
    float base_attachment_force = 5 / 0.5;
    float decay_per_second = 0.3;

    apply_repulsion_forces(world, base_repulsion_force, elapsed_time);
    apply_attachment_forces(world, elapsed_time, base_attachment_force);
    apply_velocities(world, elapsed_time);
    apply_damping(world, decay_per_second, elapsed_time);
    
    update_all_body_rooms(world);
}

Optional<Attachment *> find_attachment(PhysicsWorld &world, Body *a, Body *b)
{   
    auto it = world.attachments.iter()
        .filter([&](Attachment *attachment)
	{
	    return (attachment->bodies[0] == a && attachment->bodies[1] == b)
	        || (attachment->bodies[0] == b && attachment->bodies[1] == a);
 	});

    Optional<Attachment *> needle = it.next();
    assert(it.next().empty);

    return needle;
} 

