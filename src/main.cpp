#include "Logger.hpp"
#include <vector>
#include <cstdlib>
#include <iostream>
#include "physics/physics.hpp"
#include "graphics/graphics.hpp"
#include "logic/logic.hpp"
#include "string.h"
#include "time.h"
#include "sleep.h"
#include <functional>
#include "input_utils.hpp"
#include <iomanip>

using namespace input_utils;

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

PhysicsWorld physics;
LogicWorld logic;

bool mouse_down = false;
glm::mat4 view;
glm::vec2 last_cursor;
ViewConfig viewconfig;
int w = 800, h = 600;

int main(int argc, char **argv)
{
    // init glfw
    if (!glfwInit())
	return -1;
    std::atexit(glfwTerminate);

    glfwWindowHint(GLFW_SAMPLES, 4);
    GLFWwindow *window = glfwCreateWindow(w, h, "float", 0, 0);
    if (!window)
	exit(-1);

    view = glm::mat4();
    view[0] = glm::vec4(0.1, 0, 0, 0);
    view[1] = glm::vec4(0, 0.1, 0, 0);
    view[2] = glm::vec4(0, 0, 1, 0);
    view[3] = glm::vec4(-2, -2, 0, 1);

    viewconfig.trans_per_mouse_move = 3 / (float)w;
    viewconfig.zoom_per_scroll = 1.1;

    glfwSetKeyCallback(window, [](GLFWwindow *, int key, int scancode, int action, int mods){});
    glfwSetMouseButtonCallback(window, [](GLFWwindow *, int key, int action, int mods)
			       {
				   if (key == GLFW_MOUSE_BUTTON_LEFT)				      				        mouse_down = action == GLFW_PRESS;
			       });
    glfwSetCursorPosCallback(window, [](GLFWwindow *, double x, double y)
			     {
				 glm::vec2 cursor(x, y);
				 
				 if (mouse_down)
				 {
				     glm::vec2 motion = cursor - last_cursor;
				     motion.y = -motion.y;
				     mouse_move(view, motion, viewconfig);
				 }
				     
				 last_cursor = cursor;
			     });
    glfwSetScrollCallback(window, [](GLFWwindow *, double, double y)
			  {
			      glm::vec2 wsize(w, h);
			      glm::vec2 convcurs = last_cursor / wsize * 2.f - glm::vec2(1, 1);
			      convcurs.y = -convcurs.y;
			      scroll(view, y, convcurs, viewconfig);
			  });
    
    glfwMakeContextCurrent(window);
    
    // init physics
    init_physics(&physics);

    // init logic
    init_logic_world(&logic, &physics);

    // init graphics
    Graphics graphics;
    init_graphics(&graphics, &physics);
    glViewport(0, 0, w, h);
    
    double min_frame_time = 1 / 30.f;
    double frame_start = glfwGetTime() - min_frame_time;
    while (!glfwWindowShouldClose(window))
    {	
	double elapsed_time = glfwGetTime() - frame_start;
	frame_start+= elapsed_time;

	// for debugging
	elapsed_time = min_frame_time;

	update_logic(&logic, &physics, elapsed_time);
	update_physics(&physics, elapsed_time);
	render(&graphics, &physics, &logic, view);
	
	glfwSwapBuffers(window);
	glfwPollEvents();

	double sleep_time = min_frame_time - (glfwGetTime() - frame_start);
	if (sleep_time > 0)
	    sleep(sleep_time);
	else if (sleep_time < -min_frame_time / 2)
	    std::cout << "we are lagging behind by " << -sleep_time << " seconds!!!\n";
    }
}
