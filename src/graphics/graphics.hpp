#ifndef GRAPHICS_HPP_INCLUDED
#define GRAPHICS_HPP_INCLUDED

#include "GLL/GLL.hpp"
#include <functional>
#include <vector>
#include <glm/glm.hpp>

struct ProgramVars
{
    gll::Uniform mvp;
    gll::Attribute vertXYZ;
    gll::Attribute vertUV;
    gll::Uniform tex;
    gll::Uniform tex_off_y;
    gll::Uniform overlay_color;
};

struct Model
{
    GLuint vbo, vao;
};

struct Graphics
{
    ProgramVars program_vars;
    Model cell_model, attachment_model;
    GLuint cell_tex, attachment_tex;
    int cell_tex_rows;
};

void init_graphics(Graphics *graphics);
void render(Graphics *graphics, struct PhysicsWorld *physics, struct LogicWorld *logic,
	    glm::mat4 const &view);

#endif
