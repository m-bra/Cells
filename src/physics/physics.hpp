#ifndef PHYSICS_H_INCLUDED
#define PHYSICS_H_INCLUDED

#include "linmath.h"
#include <vector>

#define ROOMS_X 50
#define ROOMS_Y 50

struct Body
{
   vec2 pos, vel;
   float angle;
   float mass;
   /// The "density" of the body.
   float mass_per_radius;
   int room_x, room_y;
};

struct Attachment 
{
    /// The distance between the radius'es of the body that should be kept
    float distance;
    /// Just a factor for the correction force
    float strength;
    Body *bodies[2];
};

typedef struct
{
    std::vector<Body *> rooms[ROOMS_X][ROOMS_Y];
    float room_width, room_height;
} BodyRooms;

static inline float body_radius(const Body *body)
{
    return body->mass / body->mass_per_radius;
}

/// Updates the velocities of the bodies to correct the attachments
/// by acting forces on them over time "time".
/// "base_force" is the force acted on the bodies in an attachment that has distance 1 and strength 1.
void apply_attachment_forces(Attachment *attachments, int attachment_count, float time, float base_force);
void apply_repulsion_forces(BodyRooms *rooms, float base_force, float time);
void apply_velocities(BodyRooms *rooms, Body *bodies, int body_count, float time);
void update_body_room(BodyRooms *rooms, Body *body);
void ensure_inside_bounds(float left, float right, float top, float bottom, Body *body);

#endif
