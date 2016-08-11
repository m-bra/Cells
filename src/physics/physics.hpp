#ifndef PHYSICS_H_INCLUDED
#define PHYSICS_H_INCLUDED

#include <glm/glm.hpp>
#include <vector>
#include "Optional.hpp"
#include "Iterator.hpp"

#define ROOMS_X 50
#define ROOMS_Y 50

struct Body
{
    glm::vec2 pos, vel;
    float angle, angle_vel;
    float mass;
    /// The "density" of the body.
    float mass_per_radius;
    int room_x, room_y;
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

struct Attachment
{
    AttachmentConfig config;
    Body *bodies[2];
};

struct BodyRooms
{
    std::vector<Body *> rooms[ROOMS_X][ROOMS_Y];
    float room_width, room_height;
};

struct PhysicsWorld
{
    /// Order matters.
    std::vector<Optional<Body>> bodies;
    /// Order matters.
    std::vector<Optional<Attachment>> attachments;
    BodyRooms body_rooms;
};

inline float body_radius(Body const &body)
{
    return body.mass / body.mass_per_radius;
}

inline Body &body_of(PhysicsWorld &world, int i)
{
    assert(!world.bodies[i].empty);
    return world.bodies[i].value;
}

template <typename T>
struct OptNotEmpty
{   typedef bool Result;
    bool operator()(Optional<T> &body) {return !body.empty;}};
template <typename T>
struct UnwrapOpt
{   typedef T Result;
    T operator()(Optional<T> body) {return body.value;}};

template <typename T>
using SlotIterator = Iterator<
    MapIterator<FilterIterator<RawIterator<Optional<T>>, OptNotEmpty<T>>, UnwrapOpt<T>>>;

inline SlotIterator<Body> iter_bodies(PhysicsWorld &world)
{
    return iter(world.bodies)
	.filter(OptNotEmpty<Body>())
	.map(UnwrapOpt<Body>());
}

inline SlotIterator<Attachment> iter_attachments(PhysicsWorld &world)
{
    return iter(world.attachments)
	.filter(OptNotEmpty<Attachment>())
	.map(UnwrapOpt<Attachment>());
}

void init_physics(PhysicsWorld *world);
void update_physics(PhysicsWorld *world, float elapsed_time);

#endif
