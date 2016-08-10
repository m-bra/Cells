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

PhysicsWorld world;

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
    init_physics(&world);
    
    double min_frame_time = 1 / 10.f;
    double frame_start = glfwGetTime() - min_frame_time;
    while (!glfwWindowShouldClose(window))
    {
	double elapsed_time = glfwGetTime() - frame_start;
	frame_start+= elapsed_time;
	
	// physics
	update_physics(&world, elapsed_time);
	
	// graphics
	render(&graphics, &world);
	
	glfwSwapBuffers(window);
	glfwPollEvents();

	double sleep_time = min_frame_time - (glfwGetTime() - frame_start);
	if (sleep_time > 0)
	    sleep(sleep_time);
	else if (sleep_time < -min_frame_time / 2)
	    std::cout << "we are lagging behind by " << -sleep_time << " seconds!!!\n";
    }
}
