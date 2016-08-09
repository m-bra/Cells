#include "linmath.h"
#include "math.h"
#include "physics.hpp"
#include "quad_tree.h"
#include <algorithm>
#include <vector>

/// Apply a spring force between two bodies that either only repulses or only attracts (controlled by boolean "repulsion")
/// "distance" is the preferred distance between the two bodies.
/// for repulsion, all distances below the prefered result in correction force.
/// for repulsion, all distances above the prefered result in correction force.
/// correction force for a distance of exactly one is "base_force"
void apply_spring_force(Body *body0, Body *body1, 
        float distance, float base_force, int repulsion, 
        float time)
{   
    vec2 sub;
    vec2_sub(sub, body1->pos, body0->pos);
    float dist = vec2_len(sub);
    float stretch_factor = 
        dist - body_radius(body0) - body_radius(body1) - distance;
    if (repulsion)
        stretch_factor = fminf(stretch_factor, 0);    
    else
        stretch_factor = fmaxf(stretch_factor, 0);
    float force = base_force * stretch_factor; 

    vec2 force_vec;
    vec2_scale(force_vec, sub, force / dist);

    vec2 vel_vec;
    vec2_scale(vel_vec, force_vec, time / body0->mass);
    vec2_add(body0->vel, body0->vel, vel_vec);

    vec2_scale(vel_vec, force_vec, time / body1->mass);
    vec2_sub(body1->vel, body1->vel, vel_vec);
}

void apply_attachment_forces(Attachment *attachments, int attachment_count, float time, float base_force)
{
    for (int i = 0; i < attachment_count; ++i)
    {
        Body *body0 = attachments[i].bodies[0];
        Body *body1 = attachments[i].bodies[1];
        apply_spring_force(
                body0, body1, 
                attachments[i].distance, 
                base_force * attachments[i].strength, 
                0, time);
    }
}

void body_room(BodyRooms *rooms, Body *body, int *room_x, int *room_y)
{
    *room_x = floorf(body->pos[0] / rooms->room_width);
    *room_y = floorf(body->pos[1] / rooms->room_height);
    if (*room_x < 0) *room_x = 0;
    if (*room_x >= ROOMS_X) *room_x = ROOMS_X - 1;
    if (*room_y < 0) *room_y = 0;
    if (*room_y >= ROOMS_Y) *room_y = ROOMS_Y - 1;
}

/// The given pointer to the body will be added to the BodyRooms.
/// Thus, when the given body is meant to already be inside BodyRooms,
/// it has to be stored there with the same pointer!
void update_body_room(BodyRooms *rooms, Body *body)
{
    int room_x, room_y;
    body_room(rooms, body, &room_x, &room_y);

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

void apply_repulsion_forces(BodyRooms *rooms, float base_force, float time)
{
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
                if (fabsf(coord - abs_room_edge) < body_radius(*body))
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

void apply_velocities(BodyRooms *rooms, Body *bodies, int body_count, float time)
{
    for (Body *body = bodies; body != &bodies[body_count]; ++body)
    {
        vec2 tmp;
        vec2_scale(tmp, body->vel, time);
        vec2_add(body->pos, body->pos, tmp);
        ensure_inside_bounds(0, 0, rooms->room_width * ROOMS_X, rooms->room_height * ROOMS_Y, body);
    }
}

void ensure_inside_bounds(float left, float right, float top, float bottom, Body *body)
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

