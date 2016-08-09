#include <vector>
#include <cstdlib>
#include <iostream>
#include "physics/physics.hpp"
#include "graphics/graphics.hpp"
#include "string.h"
#include "time.h"
#include "sleep.h"

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

void populate(std::vector<Body> &bodies, std::vector<Attachment> &attachments);

BodyRooms body_rooms;

int main(int argc, char **argv)
{
    // init glfw
    if (!glfwInit())
	return -1;
    std::atexit(glfwTerminate);
    
    GLFWwindow *window = glfwCreateWindow(640, 480, "float", 0, 0);
    if (!window)
	exit(-1);
    
    glfwMakeContextCurrent(window);

    // init graphics
    Graphics graphics;
    init_graphics(&graphics);
    glViewport(0, 0, 640, 480);
    
    // init physics
    std::vector<Body> bodies;
    std::vector<Attachment> attachments;

    BodyRooms body_rooms;
    body_rooms.room_width = 5;
    body_rooms.room_height = body_rooms.room_width;
    
    populate(bodies, attachments);
    for (Body &body: bodies)
        update_body_room(&body_rooms, &body);

    float base_repulsion_force = 2 / 0.1;
    float base_attachment_force = 1 / 0.2;
    double min_frame_time = 1 / 10.f;
    double frame_start = glfwGetTime() - min_frame_time;
    while (!glfwWindowShouldClose(window))
    {
	double elapsed_time = glfwGetTime() - frame_start;
	frame_start+= elapsed_time;
	std::cout << "Framestart: " << frame_start << "s; glfwGetTime: " << glfwGetTime() << "s";
	
	// physics
	apply_repulsion_forces(&body_rooms, base_repulsion_force, elapsed_time);
        apply_attachment_forces(attachments.data(), attachments.size(), elapsed_time, base_attachment_force);
        apply_velocities(&body_rooms, bodies.data(), bodies.size(), elapsed_time);

        for (Body &body: bodies)
            update_body_room(&body_rooms, &body);

	// graphics
	render(&graphics);
	
	glfwSwapBuffers(window);
	glfwPollEvents();

	double sleep_time = min_frame_time - (glfwGetTime() - frame_start);
	if (sleep_time > 0)
	    sleep(sleep_time);
	else if (sleep_time < -min_frame_time / 2)
	    std::cout << "we are lagging behind by " << -sleep_time << " seconds!!!\n";
    }
}

void populate(std::vector<Body> &bodies, std::vector<Attachment> &attachments)
{
    Body body0;
    body0.pos[0] = 0.5;
    body0.pos[1] = 0.5;
    body0.angle = 0;
    body0.vel[0] = body0.vel[1] = 0;
    body0.mass = 1;
    body0.mass_per_radius = 1;
    // set to wrong position to force update
    body0.room_x = body0.room_y = 10;
    bodies.push_back(body0);

    Body body1;
    body1.pos[0] = 3;
    body1.pos[1] = 3;
    body1.angle = 0;
    body1.vel[0] = body1.vel[1] = 0;
    body1.mass = 1;
    body1.mass_per_radius = 1;
    // set to wrong position to force update
    body1.room_x = body1.room_y = 10;
    bodies.push_back(body1);

    Body body2;
    body2.pos[0] = 4.5;
    body2.pos[1] = 3;
    body2.angle = 0;
    body2.vel[0] = body2.vel[1] = 0;
    body2.mass = 1;
    body2.mass_per_radius = 1;
    // set to wrong position to force update
    body2.room_x = body2.room_y = 10;
    bodies.push_back(body2);

    Attachment attachment;
    attachment.strength = 1;
    attachment.distance = .05;
    attachment.bodies[0] = &bodies[0];
    attachment.bodies[1] = &bodies[1];
    attachments.push_back(attachment);
}
