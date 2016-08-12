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

    float radius(Body const &body)
    {
	return body.mass / body.mass_per_radius;
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

struct BodyId
{
    Optional<Body> *_ptr;
};

struct AttachmentId
{
    Optional<Attachment> *_ptr;
};

template <typename T>
struct OptNotEmpty
{ bool operator()(Optional<T> *opt) {return !opt->empty;}};
template <typename T>
struct UnwrapOpt
{   typedef T *Result;
    T *operator()(Optional<T> *opt) {return &opt->value();}};

template <typename T>
using SlotIterator = Iterator<
    MapIterator<UnwrapOpt<T>, FilterIterator<OptNotEmpty<T>, RawIterator<Optional<T>>>>>;

struct PhysicsWorld
{
    /// Order matters.
    std::vector<Optional<Body>> bodies;
    /// Order matters.
    std::vector<Optional<Attachment>> attachments;
    BodyRooms body_rooms;

    Body &body_of(int i)
    {
	assert(!bodies[i].empty);
	return bodies[i].value();
    }

    SlotIterator<Body> iter_bodies()
    {
	return iter(bodies)
	    .filter(OptNotEmpty<Body>())
	    .map(UnwrapOpt<Body>());
    }

    SlotIterator<Attachment> iter_attachments()
    {
	return iter(attachments)
	    .filter(OptNotEmpty<Attachment>())
	    .map(UnwrapOpt<Attachment>());
    }

    BodyId add_body(Body const &body)
    {
	for (size_t i = 0; i != bodies.size(); ++i)
	    if (bodies[i].empty)
	    {
		bodies[i] = Optional<Body>(body);
		return {&bodies[i]};
	    }
	bodies.push_back(Optional<Body>(body));
	return {&bodies.back()};
    }

    bool from_this_world(BodyId body)
    {
	return body._ptr > bodies.data() && body._ptr < bodies.data() + bodies.size();
    }

    Body const &kill_body(BodyId body)
    {
	assert(from_this_world(body));
	body._ptr->empty = true;
	return body._ptr->value();
    }

    bool attached(BodyId a, BodyId b)
    {
	size_t count = iter_attachments()
	    .filter([&](Attachment *attachment)
	    {
       	        return attachment->bodies[0] == &a._ptr->assert_value()
		    && attachment->bodies[1] == &b._ptr->assert_value();
	    })
	    .count();
	assert(count < 2);
	return count == 1;
    }

    bool attached(AttachmentId id)
    {
	return !id._ptr->empty;
    }

    Optional<AttachmentId> attach(BodyId a, BodyId b, AttachmentConfig const &config)
    {
	assert(from_this_world(a) && from_this_world(b));
	if (attached(a, b))
	    return Optional<AttachmentId>();
	
	Attachment attachment;
	attachment.config = config;
	attachment.bodies[0] = &a._ptr->assert_value();
	attachment.bodies[1] = &b._ptr->assert_value();
	attachments.push_back(attachment);
	return {&attachments.back()};
    }

    
};

#warning deprecated
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
