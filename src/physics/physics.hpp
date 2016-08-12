#ifndef PHYSICS_H_INCLUDED
#define PHYSICS_H_INCLUDED

#include <glm/glm.hpp>
#include <vector>
#include <algorithm>
#include "Optional.hpp"
#include "Iterator.hpp"
#include "slots.hpp"

#define ROOMS_X 50
#define ROOMS_Y 50
#define MAX_BODIES 500
#define MAX_ATTACHMENTS 500

struct Body
{
    glm::vec2 pos, vel;
    float angle, angle_vel;
    float mass;
    /// The "density" of the body.
    float mass_per_radius;
    int room_x, room_y;
    /// back-reference to all the attachments that reference this body.
    std::vector<struct Attachment *> attachments;

    float radius()
    {
	return mass / mass_per_radius;
    }
};

struct AttachmentConfig
{
    /// The distance between the radius'es of the body that should be kept
    float distance;
    /// The angle between the angles of the bodies that should be kept, NaN = ignore angle
    /// angle from bodies[0] to bodies[1]
    float delta_angle;
    /// Just a factor for the correction force
    float strength;
};

/// The attachment is above the bodies: The bodies no nothing about attachments,
/// only the attachment knows about the bodies.
struct Attachment
{
    AttachmentConfig config;
    Body *bodies[2];
};

/// Space partitioning. Only references Bodies, does not own them.
struct BodyRooms
{
    std::vector<Body *> rooms[ROOMS_X][ROOMS_Y];
    float room_width, room_height;
};

Optional<Attachment *> find_attachment(struct PhysicsWorld &world, Body *a, Body *b);

struct PhysicsWorld
{
    /// Order matters. (elements are referenced)
    Slots<Body, MAX_BODIES> bodies;
    /// Order matters. (elements are referenced)
    Slots<Attachment, MAX_ATTACHMENTS> attachments;
    BodyRooms body_rooms;
};

void init_physics(PhysicsWorld *world);
void update_physics(PhysicsWorld *world, float elapsed_time);

#endif
