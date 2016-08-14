#ifndef PHYSICS_H_INCLUDED
#define PHYSICS_H_INCLUDED

#include <glm/glm.hpp>
#include <vector>
#include "Optional.hpp"
#include "Iterator.hpp"
#include "slots.hpp"

#define ROOMS_X 20
#define ROOMS_Y 20
constexpr size_t MAX_BODIES = 500;
constexpr size_t MAX_ATTACHMENTS = MAX_BODIES;

struct Body
{
    glm::vec2 pos, vel;
    float angle = 0, angle_vel = 0;
    float mass = 1;
    /// The "density" of the body.
    float mass_per_radius = 1;
    int room_x = -1, room_y = 1;
    bool fixed = false;

    float radius() const
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
    /// Guarantee to the user that every room has a positive coordinate
    /// This makes it possible for the user to use negative coords for
    /// placeholders for "missing coord"
    static constexpr bool no_negative_rooms = true;
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
void calc_body_room(PhysicsWorld *world, Body *body, int *room_x, int *room_y);

#endif
